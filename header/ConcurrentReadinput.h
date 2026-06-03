/**
 * ConcurrentReadinput.h
 *
 * Drop-in concurrent replacement for Readinput.h / Readinput.cpp.
 *
 * The original read_input() reads every line sequentially, then for each
 * line calls Mymap::insert() and Trienode::insert().  For large datasets
 * this is the dominant bottleneck.
 *
 * This header provides:
 *
 *   read_sizes_concurrent()   – same semantics as original read_sizes(),
 *                               reads the file once to count lines / max len.
 *
 *   read_input_concurrent()   – uses ConcurrentIndexer internally:
 *                               (a) reads all lines into a std::vector<string>
 *                               (b) dispatches parallel parse/tokenise tasks
 *                               (c) serially merges into Mymap + Trie
 *
 * Both functions are backward-compatible with the original signatures
 * (return int: -1 on error, 1 on success).
 */

#ifndef CONCURRENT_READINPUT_H
#define CONCURRENT_READINPUT_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Map.hpp"
#include "Trie.hpp"
#include "ThreadPool.h"
#include "ConcurrentIndexer.h"

/* -------------------------------------------------------------------------
 * read_sizes_concurrent
 *
 * Identical semantics to the original read_sizes().
 * Counts lines and tracks the maximum line length in a single sequential
 * pass.  File I/O is inherently sequential; no parallelism is added here.
 * ------------------------------------------------------------------------- */
inline int read_sizes_concurrent(int        *linecounter,
                                 int        *maxlength,
                                 const char *docfile)
{
    std::ifstream file(docfile);
    if (!file)
    {
        std::cerr << "[ConcurrentReadinput] Error opening file: "
                  << docfile << "\n";
        return -1;
    }

    std::string line;
    while (std::getline(file, line))
    {
        int currlength = static_cast<int>(line.size());
        if (*maxlength < currlength) *maxlength = currlength;
        (*linecounter)++;
    }

    if (*linecounter == 0 || *maxlength < 6)
    {
        std::cerr << "[ConcurrentReadinput] Document is empty or too short.\n";
        return -1;
    }
    return 1;
}

/* -------------------------------------------------------------------------
 * read_input_concurrent
 *
 * Concurrent replacement for the original read_input().
 *
 * @param map       Pre-constructed Mymap.
 * @param trie      Empty Trienode root.
 * @param docfile   Path to the dataset file.
 * @param pool      ThreadPool to use for parallel parsing.
 * @param stats     Optional: filled with indexing timing stats.
 * @param chunk_sz  Documents per task (default 64).
 * @returns         1 on success, -1 on error.
 * ------------------------------------------------------------------------- */
inline int read_input_concurrent(Mymap       *map,
                                 TrieNode    *trie,
                                 const char  *docfile,
                                 ThreadPool  &pool,
                                 IndexStats  *stats     = nullptr,
                                 int          chunk_sz  = 64)
{
    /* ---- Step 1: Load all lines into memory ---- */
    std::ifstream file(docfile);
    if (!file)
    {
        std::cerr << "[ConcurrentReadinput] Error opening file: "
                  << docfile << "\n";
        return -1;
    }

    const int n = map->get_size();
    std::vector<std::string> lines;
    lines.reserve(static_cast<size_t>(n));

    std::string rawline;
    while (std::getline(file, rawline))
    {
        lines.emplace_back(rawline);
    }

    if (static_cast<int>(lines.size()) != n)
    {
        std::cerr << "[ConcurrentReadinput] Line count mismatch: expected "
                  << n << ", got " << lines.size() << "\n";
        return -1;
    }

    /* ---- Step 2: Delegate to ConcurrentIndexer ---- */
    ConcurrentIndexer indexer(pool, chunk_sz);
    bool ok = indexer.build_index(lines, map, trie, stats);

    if (!ok)
    {
        std::cerr << "[ConcurrentReadinput] One or more documents were "
                     "malformed; partial index built.\n";
        return -1;
    }
    return 1;
}

#endif /* CONCURRENT_READINPUT_H */
