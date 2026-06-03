/**
 * ConcurrentIndexer.h
 *
 * Thread-safe concurrent document indexer.
 *
 * Problem with the original codebase:
 *  - Trienode::insert() and listnode::add() are not thread-safe.
 *    Concurrent writers will corrupt the linked-list / trie structure.
 *  - Mymap::insert() / setlength() write to shared arrays – also unsafe.
 *
 * Strategy used here (production-correct):
 *
 *  Phase 1 – Parallel file I/O + tokenisation (embarrassingly parallel)
 *     Each worker thread reads its assigned document lines, tokenises them
 *     into a private per-document word list, and stores the result in a
 *     pre-allocated staging array.  No shared mutable state is touched.
 *
 *  Phase 2 – Serial merge into Mymap + Trie  (single writer, safe)
 *     After all workers complete, the main thread calls merge() which
 *     iterates the staging array and calls the original insert() / add()
 *     methods as before.  This keeps the original data structures
 *     completely unchanged.
 *
 * Why not shard the Trie?
 *  Sharding a linked-list trie is complex and error-prone.  The bottleneck
 *  in this engine is I/O + tokenisation, not trie insertion.  Parallelising
 *  the heavy part (I/O) and keeping the serial merge is the correct
 *  engineering trade-off here.
 *
 * Why not fine-grained locking on the Trie?
 *  The original Trienode uses raw pointers and C-style linked lists.
 *  Retrofitting fine-grained locks without modifying those files would
 *  require intrusive changes.  The staged approach achieves the same
 *  parallelism without touching existing files.
 */

#ifndef CONCURRENT_INDEXER_H
#define CONCURRENT_INDEXER_H

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "ThreadPool.h"
#include "Map.hpp"
#include "Trie.hpp"

/* =========================================================================
 * StagedDocument – holds the pre-parsed content for one document
 * ========================================================================= */
struct StagedDocument
{
    int                      doc_id   = -1;
    std::string              raw_text;           /* stripped line text         */
    std::vector<std::string> tokens;             /* individual words           */
    bool                     valid    = false;   /* false → skip on merge      */
};

/* =========================================================================
 * IndexStats – lightweight timing / diagnostic counters
 * ========================================================================= */
struct IndexStats
{
    double   io_parse_ms   = 0.0;  /* wall time for Phase 1 (parallel I/O)  */
    double   merge_ms      = 0.0;  /* wall time for Phase 2 (serial merge)   */
    double   total_ms      = 0.0;  /* end-to-end                             */
    int      doc_count     = 0;
    long     total_tokens  = 0;
    unsigned thread_count  = 0;
};

/* =========================================================================
 * ConcurrentIndexer
 * ========================================================================= */
class ConcurrentIndexer
{
public:
    /* ------------------------------------------------------------------
     * @param pool        Shared thread pool (caller owns lifetime)
     * @param chunk_size  Documents per task dispatched to the pool.
     *                    Tune for your dataset; 64 is a good default.
     * ------------------------------------------------------------------ */
    explicit ConcurrentIndexer(ThreadPool &pool, int chunk_size = 64)
        : pool_(pool), chunk_size_(chunk_size)
    {}

    /* ------------------------------------------------------------------ */
    /*  Primary entry point                                                 */
    /* ------------------------------------------------------------------ */

    /**
     * Build the full index from a pre-loaded file.
     *
     * @param lines       Raw lines read from the dataset file.
     *                    lines[i] is the full file line (includes leading id).
     * @param map         Mymap already constructed with correct size/buffersize.
     * @param trie        Trienode root (empty).
     * @param stats_out   Optional: filled with timing information.
     * @returns           true on success, false if any document was malformed.
     */
    bool build_index(const std::vector<std::string> &lines,
                     Mymap                          *map,
                     TrieNode                       *trie,
                     IndexStats                     *stats_out = nullptr)
    {
        const int n = static_cast<int>(lines.size());
        if (n == 0) return true;

        auto t_start = std::chrono::steady_clock::now();

        /* ---- Phase 1: allocate staging array ---- */
        staged_.assign(n, StagedDocument{});

        /* ---- Phase 1: dispatch parallel parse tasks ---- */
        std::vector<std::future<bool>> futures;
        futures.reserve((n + chunk_size_ - 1) / chunk_size_);

        for (int start = 0; start < n; start += chunk_size_)
        {
            int end = std::min(start + chunk_size_, n);
            futures.emplace_back(
                pool_.submit(&ConcurrentIndexer::parse_chunk,
                             this,
                             std::cref(lines), start, end));
        }

        /* ---- Phase 1: collect results ---- */
        bool all_ok = true;
        for (auto &f : futures)
            if (!f.get()) all_ok = false;

        auto t_phase1 = std::chrono::steady_clock::now();

        /* ---- Phase 2: serial merge into Mymap + Trie ---- */
        bool merge_ok = merge_into_structures(map, trie, stats_out);

        auto t_end = std::chrono::steady_clock::now();

        /* ---- Fill stats ---- */
        if (stats_out)
        {
            using ms = std::chrono::duration<double, std::milli>;
            stats_out->io_parse_ms  = ms(t_phase1 - t_start).count();
            stats_out->merge_ms     = ms(t_end    - t_phase1).count();
            stats_out->total_ms     = ms(t_end    - t_start).count();
            stats_out->doc_count    = n;
            stats_out->thread_count = static_cast<unsigned>(pool_.thread_count());

            long tok = 0;
            for (auto &sd : staged_) tok += static_cast<long>(sd.tokens.size());
            stats_out->total_tokens = tok;
        }

        staged_.clear();   /* release staging memory */
        return all_ok && merge_ok;
    }

    /* ------------------------------------------------------------------ */
    /*  Accessors                                                           */
    /* ------------------------------------------------------------------ */

    void set_chunk_size(int cs) { chunk_size_ = std::max(1, cs); }
    int  chunk_size()     const { return chunk_size_; }

private:
    /* ================================================================== */
    /*  Phase 1: parse_chunk (runs on a pool worker thread)               */
    /* ================================================================== */

    /**
     * Parse lines[start .. end) into staged_[start .. end).
     * No shared mutable state is written – staged_[i] is owned by
     * exactly one task (non-overlapping ranges).
     */
    bool parse_chunk(const std::vector<std::string> &lines,
                     int start, int end)
    {
        bool ok = true;
        for (int i = start; i < end; ++i)
        {
            StagedDocument &sd = staged_[static_cast<size_t>(i)];
            sd.doc_id = i;

            const std::string &line = lines[static_cast<size_t>(i)];
            if (line.empty())
            {
                sd.valid = false;
                ok = false;
                continue;
            }

            /* ---- Use the line as raw text (no leading ID expected) ---- */
            /* Strip leading and trailing whitespace */
            size_t first = line.find_first_not_of(" \t");
            if (first == std::string::npos)
            {
                sd.valid = false;
                ok = false;
                continue;
            }
            size_t last = line.find_last_not_of(" \t\r\n");
            sd.raw_text = line.substr(first, last - first + 1);

            /* ---- Tokenise ---- */
            sd.tokens.clear();
            std::istringstream tiss(sd.raw_text);
            std::string tok;
            while (tiss >> tok)
                sd.tokens.push_back(std::move(tok));

            sd.valid = !sd.tokens.empty();
        }
        return ok;
    }

    /* ================================================================== */
    /*  Phase 2: merge_into_structures (runs on the main thread only)     */
    /* ================================================================== */

    bool merge_into_structures(Mymap    *map,
                               TrieNode *trie,
                               IndexStats * /*unused*/)
    {
        bool ok = true;
        for (auto &sd : staged_)
        {
            if (!sd.valid)
            {
                ok = false;
                continue;
            }

            /* ---- Insert raw text into Mymap ---- */
            /* Mymap::insert() expects a mutable C-string with the full
             * "id text" format.  We reconstruct it. */
            std::string full_line =
                std::to_string(sd.doc_id) + " " + sd.raw_text;

            /* Mymap::insert uses strtok which mutates the buffer */
            char *buf = new char[full_line.size() + 1];
            std::memcpy(buf, full_line.c_str(), full_line.size() + 1);

            if (map->insert(buf, sd.doc_id) == -1)
            {
                delete[] buf;
                ok = false;
                continue;
            }
            delete[] buf;

            /* ---- Insert word-count into map ---- */
            map->setlength(sd.doc_id, static_cast<int>(sd.tokens.size()));

            /* ---- Insert tokens into Trie ---- */
            for (const auto &word : sd.tokens)
            {
                /* Trienode::insert also uses strtok-style char* */
                char *wbuf = new char[word.size() + 1];
                std::memcpy(wbuf, word.c_str(), word.size() + 1);
                trie->insert(wbuf, sd.doc_id);
                delete[] wbuf;
            }
        }
        return ok;
    }

    /* ================================================================== */
    /*  Data members                                                        */
    /* ================================================================== */

    ThreadPool              &pool_;
    int                      chunk_size_;

    /* Staging array – one entry per document.
     * Sized once in build_index(), written in Phase 1 (parallel, disjoint),
     * read in Phase 2 (serial).  No locks needed. */
    std::vector<StagedDocument> staged_;
};

#endif /* CONCURRENT_INDEXER_H */
