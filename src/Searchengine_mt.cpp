/**
 * Searchengine_mt.cpp
 *
 * Multithreaded main entry point for the Search Engine.
 *
 * Differences from the original Searchengine.cpp:
 *
 *   1. Indexing is parallelised via ConcurrentIndexer + ThreadPool.
 *      On an N-core machine the I/O + tokenisation phase uses N threads,
 *      reducing indexing time roughly proportionally.
 *
 *   2. Query processing uses ConcurrentQueryProcessor which dispatches
 *      Trie lookups for multi-term queries in parallel and supports a
 *      batch-query mode.
 *
 *   3. The REPL supports two additional commands:
 *        /threads [N]   â€“ report (or change) the thread count
 *        /stats         â€“ print last indexing timing report
 *        /batch         â€“ enter batch query mode (one query per line,
 *                         empty line to exit; all queries run in parallel)
 *
 *   4. All original commands (/search, /df, /tf, /exit) remain unchanged
 *      and produce identical output.
 *
 * Build (matching ManasVasana's CMakeLists structure):
 *
 *   cmake_minimum_required(VERSION 3.10)
 *   project(SearchEngine)
 *   set(CMAKE_CXX_STANDARD 14)
 *   find_package(Threads REQUIRED)
 *   include_directories(header)
 *   add_executable(searchengine_mt
 *       src/Searchengine_mt.cpp
 *       src/Document_store.cpp   # kept as-is
 *       src/Trie.cpp             # kept as-is
 *       src/Score.cpp            # kept as-is
 *       src/Listnode.cpp         # kept as-is
 *       src/Search.cpp           # kept as-is (used for /df, /tf)
 *       src/Maxheap.cpp          # kept as-is
 *       src/Map.cpp              # kept as-is
 *   )
 *   target_link_libraries(searchengine_mt Threads::Threads)
 *
 * NOTE: header-only files ThreadPool.h, ConcurrentIndexer.h,
 *       ConcurrentQueryProcessor.h, and ConcurrentReadinput.h are placed in
 *       the header/ directory.  No existing file is modified.
 *
 * Usage:
 *   ./searchengine_mt -d <dataset> -k <top_k> [-t <num_threads>]
 */

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

/* ---- Original headers (unchanged) ---- */
#include "Map.hpp"
#include "Trie.hpp"
#include "Score.hpp"
#include "Maxheap.hpp"
#include "Search.hpp"        /* for df(), tf() â€“ reused unchanged   */

/* ---- New multithreading headers ---- */
#include "ThreadPool.h"
#include "ConcurrentIndexer.h"
#include "ConcurrentReadinput.h"
#include "ConcurrentQueryProcessor.h"

/* =========================================================================
 * Helpers
 * ========================================================================= */

/** Split a string on whitespace into tokens. */
static std::vector<std::string> tokenise(const std::string &s)
{
    std::vector<std::string> out;
    std::istringstream iss(s);
    std::string tok;
    while (iss >> tok) out.push_back(tok);
    return out;
}

/** Print the IndexStats timing summary to stdout. */
static void print_stats(const IndexStats &st)
{
    std::cout << "\n========== Indexing Statistics ==========\n";
    std::cout << "  Documents indexed : " << st.doc_count     << "\n";
    std::cout << "  Total tokens      : " << st.total_tokens  << "\n";
    std::cout << "  Worker threads    : " << st.thread_count  << "\n";
    std::cout << "  Phase 1 (I/O+parse): " << st.io_parse_ms  << " ms\n";
    std::cout << "  Phase 2 (merge)    : " << st.merge_ms     << " ms\n";
    std::cout << "  Total             : " << st.total_ms      << " ms\n";
    std::cout << "=========================================\n\n";
}

/** Pretty-print a single QueryResult hit. */
static void print_hit(int rank, const QueryResult::Hit &h, Mymap *map)
{
    /* Reproduce the original score/id formatting from Search.cpp */
    std::cout << "(" << h.doc_id;

    /* right-pad id to 5 chars */
    int digits = 1;
    int tmp    = h.doc_id;
    while (tmp / 10 != 0) { tmp /= 10; ++digits; }
    for (int sp = digits; sp < 5; ++sp) std::cout << ' ';

    std::printf(")[%10.6f] ", h.score);
    std::cout << map->getDocument(h.doc_id) << "\n";
    (void)rank;
}

/* =========================================================================
 * REPL command handlers
 * ========================================================================= */

/** /search <word1> [word2 â€¦] */
static void cmd_search(const std::vector<std::string> &tokens,
                       ConcurrentQueryProcessor       &qproc,
                       Mymap                          *map,
                       int                             k)
{
    if (tokens.size() < 2)
    {
        std::cout << "Usage: /search <word> [word2 â€¦]\n";
        return;
    }

    std::vector<std::string> query_terms(tokens.begin() + 1, tokens.end());

    auto t0 = std::chrono::steady_clock::now();
    QueryResult result = qproc.search(query_terms, k);
    auto t1 = std::chrono::steady_clock::now();

    if (!result.ok || result.hits.empty())
    {
        std::cout << "No results found.\n";
        return;
    }

    for (int i = 0; i < static_cast<int>(result.hits.size()); ++i)
        print_hit(i + 1, result.hits[static_cast<size_t>(i)], map);

    double ms = std::chrono::duration<double,std::milli>(t1 - t0).count();
    std::printf("\n[Query completed in %.3f ms]\n\n", ms);
}

/** /df [word] â€“ delegates to original df() */
static void cmd_df(char *raw_input, TrieNode *trie)
{
    /* df() consumes tokens from strtok state already initialised
     * by the caller; we re-tokenise from raw_input ourselves. */
    (void)raw_input;
    (void)trie;

    /* NOTE: df() uses strtok(NULL, â€¦) which expects strtok context
     * to be set up by the caller.  We call it exactly as the original
     * inputmanager() did. */
    df(trie);
}

/** /tf <docid> <word> â€“ delegates to original tf() */
static int cmd_tf(char *token, TrieNode *trie)
{
    return tf(token, trie);
}

/** /batch â€“ enter concurrent batch query mode */
static void cmd_batch(ConcurrentQueryProcessor &qproc,
                      Mymap                    *map,
                      int                       k)
{
    std::cout << "Batch mode: enter one query per line (empty line to finish).\n";

    std::vector<std::vector<std::string>> queries;
    std::string line;
    while (std::getline(std::cin, line))
    {
        if (line.empty()) break;
        auto toks = tokenise(line);
        if (!toks.empty() && toks[0] == "/search")
            toks.erase(toks.begin());
        if (!toks.empty())
            queries.push_back(std::move(toks));
    }

    if (queries.empty())
    {
        std::cout << "No queries entered.\n";
        return;
    }

    std::cout << "Running " << queries.size()
              << " queries concurrentlyâ€¦\n";

    auto t0 = std::chrono::steady_clock::now();
    std::vector<QueryResult> results = qproc.batch_search(queries, k);
    auto t1 = std::chrono::steady_clock::now();

    for (size_t qi = 0; qi < queries.size(); ++qi)
    {
        std::cout << "\n--- Query " << (qi+1) << ": ";
        for (auto &w : queries[qi]) std::cout << w << ' ';
        std::cout << "---\n";

        const QueryResult &r = results[qi];
        if (!r.ok || r.hits.empty())
        {
            std::cout << "  (no results)\n";
            continue;
        }
        for (int i = 0; i < static_cast<int>(r.hits.size()); ++i)
            print_hit(i + 1, r.hits[static_cast<size_t>(i)], map);
    }

    double ms = std::chrono::duration<double,std::milli>(t1 - t0).count();
    std::printf("\n[Batch of %zu queries completed in %.3f ms]\n\n",
                queries.size(), ms);
}

/* =========================================================================
 * inputmanager_mt â€“ handles one line of REPL input
 * ========================================================================= */

/**
 * @returns  1  success
 *          -1  bad input
 *           2  /exit requested
 */
static int inputmanager_mt(char                    *input,
                           TrieNode                *trie,
                           Mymap                   *map,
                           int                      k,
                           ConcurrentQueryProcessor &qproc,
                           ThreadPool              &pool,
                           const IndexStats        &stats)
{
    /* Tokenise for command dispatch */
    std::string line(input);
    auto tokens = tokenise(line);
    if (tokens.empty()) return -1;

    const std::string &cmd = tokens[0];

    /* ---- /search ---- */
    if (cmd == "/search")
    {
        cmd_search(tokens, qproc, map, k);
        return 1;
    }

    /* ---- /df ---- */
    if (cmd == "/df")
    {
        /* df() relies on strtok state; set it up from raw input */
        strtok(input, " \t\n");   /* consumes "/df" */
        cmd_df(input, trie);
        return 1;
    }

    /* ---- /tf ---- */
    if (cmd == "/tf")
    {
        char *tok = strtok(input, " \t\n");   /* "/tf" */
        if (cmd_tf(tok, trie) == -1)
            return -1;
        return 1;
    }

    /* ---- /batch ---- */
    if (cmd == "/batch")
    {
        cmd_batch(qproc, map, k);
        return 1;
    }

    /* ---- /stats ---- */
    if (cmd == "/stats")
    {
        print_stats(stats);
        return 1;
    }

    /* ---- /threads [N] ---- */
    if (cmd == "/threads")
    {
        if (tokens.size() >= 2)
        {
            /* Informational only â€“ the pool size is fixed at construction.
             * A production system would support dynamic resizing; for this
             * engine we just report the current count. */
            std::cout << "Note: thread pool size is fixed at construction.\n";
        }
        std::cout << "Active worker threads: "
                  << pool.thread_count() << "\n";
        return 1;
    }

    /* ---- /exit ---- */
    if (cmd == "/exit")
    {
        std::cout << "Exitingâ€¦\n";
        return 2;
    }

    return -1;
}

/* =========================================================================
 * main
 * ========================================================================= */

int main(int argc, char **argv)
{
    /* ---- Argument parsing ---- */
    if (argc < 5 ||
        std::strcmp(argv[1], "-d") != 0 ||
        std::strcmp(argv[3], "-k") != 0)
    {
        std::cerr << "Usage: " << argv[0]
                  << " -d <dataset> -k <top_k> [-t <threads>]\n";
        return -1;
    }

    const char *docfile    = argv[2];
    const int   k          = std::atoi(argv[4]);

    /* Optional -t <threads> argument */
    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 1;

    for (int i = 5; i < argc - 1; ++i)
    {
        if (std::strcmp(argv[i], "-t") == 0)
        {
            int t = std::atoi(argv[i + 1]);
            if (t > 0) num_threads = static_cast<unsigned int>(t);
            break;
        }
    }

    std::cout << "Please wait â€“ indexing with " << num_threads
              << " thread(s)â€¦\n";

    /* ---- Build thread pool ---- */
    ThreadPool pool(num_threads);

    /* ---- First pass: count lines and max line length ---- */
    int linecounter = 0, maxlength = -1;
    if (read_sizes_concurrent(&linecounter, &maxlength,
                              const_cast<char *>(docfile)) == -1)
        return -1;

    /* ---- Allocate Mymap and Trie ---- */
    std::unique_ptr<Mymap>    mymap(new Mymap(linecounter, maxlength));
    std::unique_ptr<TrieNode> trie(new TrieNode());

    /* ---- Second pass: concurrent indexing ---- */
    IndexStats stats;
    if (read_input_concurrent(mymap.get(), trie.get(),
                              docfile, pool, &stats) == -1)
    {
        std::cerr << "Indexing failed.\n";
        return -1;
    }

    print_stats(stats);
    std::cout << "Database Ready\n\n";
    std::cout << "Commands: /search <terms>  /df [word]  /tf <id> <word>\n"
              << "          /batch           /stats      /threads  /exit\n\n";

    /* ---- Build concurrent query processor ---- */
    ConcurrentQueryProcessor qproc(pool, trie.get(), mymap.get());

    /* ---- REPL ---- */
    std::string input_line;

    while (true)
    {
        std::cout << "> ";
        std::cout.flush();

        if (!std::getline(std::cin, input_line))
            break;   /* EOF */

        std::vector<char> input_buf(input_line.begin(), input_line.end());
        input_buf.push_back('\0');

        int ret = inputmanager_mt(input_buf.data(), trie.get(), mymap.get(), k,
                                  qproc, pool, stats);
        if (ret == -1)
            std::cout << "Unknown command or bad input.\n";
        else if (ret == 2)
            break;
    }

    return 0;
}

