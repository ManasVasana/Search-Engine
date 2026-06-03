/**
 * ConcurrentQueryProcessor.h
 *
 * Thread-safe concurrent multi-query processor.
 *
 * The original Search.cpp is single-threaded: one query at a time.
 * This layer adds:
 *
 *   1. Parallel term lookup  – when a /search query has multiple words,
 *      each word's Trie traversal is dispatched to the thread pool
 *      concurrently.  The Trie is read-only after indexing, so no locks
 *      are required for reads.
 *
 *   2. Parallel batch queries – when multiple queries arrive at once
 *      (e.g. from a pipeline or test harness) they are processed in
 *      parallel, each producing its own independent result set.
 *
 *   3. IDF pre-computation cache – IDF values for every term are computed
 *      in parallel after indexing completes.  Subsequent queries hit the
 *      cache instead of recomputing log().
 *
 * Thread-safety contract:
 *   - The Trie and Mymap are READ-ONLY after build_index() completes.
 *     Concurrent reads of read-only data structures are safe without locks.
 *   - The IDF cache is populated once (under a mutex) then read-only.
 *   - Each query creates its own Scorelist, Maxheap, etc. – fully private.
 *
 * Public API mirrors the original Search.h functions so the main file
 * can opt-in to concurrent processing with minimal changes.
 */

#ifndef CONCURRENT_QUERY_PROCESSOR_H
#define CONCURRENT_QUERY_PROCESSOR_H

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "ThreadPool.h"
#include "Map.hpp"
#include "Trie.hpp"
#include "Score.hpp"
#include "Maxheap.hpp"

/* =========================================================================
 * QueryResult – the result of one /search query
 * ========================================================================= */
struct QueryResult
{
    struct Hit
    {
        int    doc_id;
        double score;
    };
    std::vector<Hit> hits;   /* sorted descending by score, capped at k     */
    bool             ok = false;
};

/* =========================================================================
 * ConcurrentQueryProcessor
 * ========================================================================= */
class ConcurrentQueryProcessor
{
public:
    /* BM25 tuning constants (same defaults as original Search.cpp) */
    static constexpr float K1 = 1.2f;
    static constexpr float B  = 0.75f;

    /* Maximum terms per query (same limit as original) */
    static constexpr int MAX_QUERY_TERMS = 10;

    /* ------------------------------------------------------------------ */
    /*  Construction                                                        */
    /* ------------------------------------------------------------------ */

    /**
     * @param pool   Shared thread pool.
     * @param trie   Fully-built, read-only Trie.
     * @param map    Fully-built, read-only Mymap.
     */
    ConcurrentQueryProcessor(ThreadPool &pool,
                             TrieNode   *trie,
                             Mymap      *map)
        : pool_(pool), trie_(trie), map_(map),
          avg_dl_(0.0), idf_cache_ready_(false)
    {
        precompute_avg_dl();
    }

    /* ------------------------------------------------------------------ */
    /*  Post-index setup                                                    */
    /* ------------------------------------------------------------------ */

    /**
     * Pre-compute IDF values for all terms that appear in the current
     * query batch.  Call this once after build_index() completes.
     *
     * For an interactive REPL this is optional (IDF is computed on demand),
     * but for batch workloads it avoids repeated log() calls per query.
     *
     * @param terms  List of terms to pre-compute IDF for.
     */
    void precompute_idf(const std::vector<std::string> &terms)
    {
        std::vector<std::future<std::pair<std::string,double>>> futures;
        futures.reserve(terms.size());

        for (const auto &term : terms)
        {
            futures.emplace_back(
                pool_.submit([this, term]() -> std::pair<std::string,double> {
                    return { term, compute_idf(term) };
                }));
        }

        std::lock_guard<std::mutex> lk(idf_cache_mutex_);
        for (auto &f : futures)
        {
            auto kv = f.get();
            idf_cache_[kv.first] = kv.second;
        }
        idf_cache_ready_ = true;
    }

    /* ------------------------------------------------------------------ */
    /*  Single query (replaces search() in Search.cpp)                     */
    /* ------------------------------------------------------------------ */

    /**
     * Execute one /search query synchronously.
     *
     * Internally, Trie lookups for each term are dispatched in parallel.
     *
     * @param query_terms  Up to MAX_QUERY_TERMS words.
     * @param k            Number of top results to return.
     * @returns            QueryResult with hits sorted by score descending.
     */
    QueryResult search(const std::vector<std::string> &query_terms, int k)
    {
        QueryResult result;
        if (query_terms.empty() || k <= 0)
            return result;

        const int n_terms = std::min(static_cast<int>(query_terms.size()),
                                     MAX_QUERY_TERMS);

        /* ---- Step 1: Parallel Trie lookup + IDF computation ---- */

        std::vector<TermData>                        term_data(n_terms);
        std::vector<std::future<void>>               futures(n_terms);

        for (int i = 0; i < n_terms; ++i)
        {
            term_data[i].word      = query_terms[static_cast<size_t>(i)];
            term_data[i].scorelist = new Scorelist();

            futures[i] = pool_.submit(
                [this, i, &term_data]() {
                    TermData &td = term_data[i];

                    /* IDF (cache-friendly path) */
                    td.idf = get_idf(td.word);

                    /* Trie traversal – read-only, safe to run concurrently */
                    char *wbuf = new char[td.word.size() + 1];
                    std::memcpy(wbuf, td.word.c_str(), td.word.size() + 1);
                    trie_->search(wbuf, 0, td.scorelist);
                    delete[] wbuf;
                });
        }

        /* Wait for all term lookups to complete */
        for (auto &f : futures) f.get();

        /* ---- Step 2: Merge Scorelists into a unified candidate set ----
         *
         * We build a set of unique document IDs from all term Scorelists.
         * This is done serially because Scorelist is a linked list that
         * was written by each worker thread independently (each thread has
         * its own Scorelist object – no sharing).
         */
        /* Use a hash set for O(1) dedup */
        std::unordered_map<int,bool> candidate_set;
        for (int i = 0; i < n_terms; ++i)
        {
            Scorelist *sl = term_data[i].scorelist;
            while (sl != nullptr && sl->get_id() != -1)
            {
                candidate_set[sl->get_id()] = true;
                sl = sl->get_next();
            }
        }

        /* ---- Step 3: BM25 scoring – parallel over documents ---- */
        /*
         * Each document's score can be computed independently.
         * We dispatch a parallel score computation if there are enough
         * candidates to justify the overhead (threshold: >64 docs).
         */
        const int n_candidates = static_cast<int>(candidate_set.size());
        std::vector<int> candidates;
        candidates.reserve(static_cast<size_t>(n_candidates));
        for (auto &kv : candidate_set)
            candidates.push_back(kv.first);

        std::vector<double> scores(static_cast<size_t>(n_candidates), 0.0);

        if (n_candidates > 64)
        {
            /* Parallel scoring */
            const int chunk = 32;
            std::vector<std::future<void>> score_futures;
            score_futures.reserve(static_cast<size_t>((n_candidates + chunk - 1) / chunk));

            for (int start = 0; start < n_candidates; start += chunk)
            {
                int end = std::min(start + chunk, n_candidates);
                score_futures.emplace_back(
                    pool_.submit(
                        [this, start, end, n_terms,
                         &candidates, &scores, &term_data]() {
                            for (int ci = start; ci < end; ++ci)
                            {
                                int doc = candidates[static_cast<size_t>(ci)];
                                scores[static_cast<size_t>(ci)] =
                                    bm25_score(doc, n_terms, term_data.data());
                            }
                        }));
            }
            for (auto &sf : score_futures) sf.get();
        }
        else
        {
            /* Serial scoring for small candidate sets */
            for (int ci = 0; ci < n_candidates; ++ci)
            {
                int doc = candidates[static_cast<size_t>(ci)];
                scores[static_cast<size_t>(ci)] =
                    bm25_score(doc, n_terms, term_data.data());
            }
        }

        /* ---- Step 4: Top-k extraction using original Maxheap ---- */
        Maxheap heap(k);
        for (int ci = 0; ci < n_candidates; ++ci)
            heap.insert(scores[static_cast<size_t>(ci)],
                        candidates[static_cast<size_t>(ci)]);

        int actual_k = std::min(k, n_candidates);
        result.hits.reserve(static_cast<size_t>(actual_k));
        for (int l = 0; l < actual_k; ++l)
        {
            int    id  = heap.get_id();
            if (id == -1) break;
            double sc  = heap.remove();
            result.hits.push_back({id, sc});
        }

        result.ok = true;

        /* ---- Cleanup ---- */
        for (int i = 0; i < n_terms; ++i)
            delete term_data[i].scorelist;

        return result;
    }

    /* ------------------------------------------------------------------ */
    /*  Batch query (parallel over multiple independent queries)           */
    /* ------------------------------------------------------------------ */

    /**
     * Execute multiple queries concurrently.
     *
     * @param queries  Vector of query-term lists.
     * @param k        Top-k for every query.
     * @returns        Vector of QueryResult, one per input query, in order.
     */
    std::vector<QueryResult> batch_search(
        const std::vector<std::vector<std::string>> &queries, int k)
    {
        std::vector<std::future<QueryResult>> futures;
        futures.reserve(queries.size());

        for (const auto &qt : queries)
        {
            futures.emplace_back(
                pool_.submit([this, qt, k]() -> QueryResult {
                    return this->search(qt, k);
                }));
        }

        std::vector<QueryResult> results;
        results.reserve(queries.size());
        for (auto &f : futures)
            results.push_back(f.get());

        return results;
    }

    /* ------------------------------------------------------------------ */
    /*  Accessors                                                           */
    /* ------------------------------------------------------------------ */

    double avg_dl() const { return avg_dl_; }

private:
    /* ================================================================== */
    /*  Helpers                                                             */
    /* ================================================================== */

    /* Struct exposed to lambdas in score computation */
    struct TermData
    {
        std::string  word;
        double       idf;
        Scorelist   *scorelist;
    };

    void precompute_avg_dl()
    {
        const int n = map_->get_size();
        double sum  = 0.0;
        for (int i = 0; i < n; ++i)
            sum += static_cast<double>(map_->getlength(i));
        avg_dl_ = (n > 0) ? (sum / static_cast<double>(n)) : 1.0;
    }

    double compute_idf(const std::string &word) const
    {
        char *wbuf = new char[word.size() + 1];
        std::memcpy(wbuf, word.c_str(), word.size() + 1);
        double df = static_cast<double>(
                trie_->dfsearchword(wbuf, 0,
                                     static_cast<int>(word.size())));
        delete[] wbuf;

        const double N = static_cast<double>(map_->get_size());
        /* Robertson-Jones IDF (original formula from Search.cpp) */
        return std::log10((N - df + 0.5) / (df + 0.5));
    }

    double get_idf(const std::string &word) const
    {
        if (idf_cache_ready_)
        {
            std::lock_guard<std::mutex> lk(idf_cache_mutex_);
            auto it = idf_cache_.find(word);
            if (it != idf_cache_.end())
                return it->second;
        }
        return compute_idf(word);
    }

    double bm25_score(int doc_id, int n_terms,
                      const TermData *term_data) const
    {
        double score = 0.0;
        const double dl = static_cast<double>(map_->getlength(doc_id));

        for (int t = 0; t < n_terms; ++t)
        {
            char *wbuf = new char[term_data[t].word.size() + 1];
            std::memcpy(wbuf, term_data[t].word.c_str(),
                        term_data[t].word.size() + 1);
            double tf = static_cast<double>(
                trie_->tfsearchword(doc_id, wbuf, 0,
                                     static_cast<int>(term_data[t].word.size())));
            delete[] wbuf;

            const double denom =
                tf + K1 * (1.0 - B + B * (dl / avg_dl_));

            score += term_data[t].idf *
                     (tf * (K1 + 1.0)) /
                     (denom > 0.0 ? denom : 1e-12);
        }
        return score;
    }

    /* ================================================================== */
    /*  Data members                                                        */
    /* ================================================================== */

    ThreadPool &pool_;
    TrieNode   *trie_;   /* read-only after indexing                        */
    Mymap      *map_;    /* read-only after indexing                        */

    double      avg_dl_;

    mutable std::mutex                          idf_cache_mutex_;
    mutable std::unordered_map<std::string,double> idf_cache_;
    bool                                        idf_cache_ready_;
};

#endif /* CONCURRENT_QUERY_PROCESSOR_H */
