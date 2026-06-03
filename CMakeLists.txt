cmake_minimum_required(VERSION 3.10)
project(SearchEngine)

# -----------------------------------------------------------------------
# Language standard
# C++14 is required for std::make_unique; C++11 would work with raw new.
# -----------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------
# Compiler flags
# -----------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -O2)
endif()

# -----------------------------------------------------------------------
# Threading
# -----------------------------------------------------------------------
find_package(Threads REQUIRED)

# -----------------------------------------------------------------------
# Include paths
# -----------------------------------------------------------------------
include_directories(header)

# -----------------------------------------------------------------------
# Common sources (original, unchanged)
# -----------------------------------------------------------------------
set(ORIGINAL_SOURCES
    src/Searchengine.cpp
    src/Document_store.cpp
    src/Trie.cpp
    src/Score.cpp
    src/Listnode.cpp
    src/Search.cpp
    src/Maxheap.cpp
    src/Map.cpp
)

# -----------------------------------------------------------------------
# Multithreaded sources
# New files only; all existing src/*.cpp files are reused unchanged.
# -----------------------------------------------------------------------
set(MT_SOURCES
    src/Searchengine_mt.cpp   # NEW – replaces Searchengine.cpp as main()
    src/Document_store.cpp    # original, unchanged
    src/Trie.cpp              # original, unchanged
    src/Score.cpp             # original, unchanged
    src/Listnode.cpp          # original, unchanged
    src/Search.cpp            # original, unchanged (df/tf reused)
    src/Maxheap.cpp           # original, unchanged
    src/Map.cpp               # original, unchanged
)

# -----------------------------------------------------------------------
# Target: searchengine  (original, single-threaded)
# -----------------------------------------------------------------------
add_executable(searchengine ${ORIGINAL_SOURCES})
target_link_libraries(searchengine Threads::Threads)

# -----------------------------------------------------------------------
# Target: searchengine_mt  (new, multithreaded)
# Header-only MT layer lives in header/:
#   ThreadPool.h
#   ConcurrentIndexer.h
#   ConcurrentQueryProcessor.h
#   ConcurrentReadinput.h
# -----------------------------------------------------------------------
add_executable(searchengine_mt ${MT_SOURCES})
target_link_libraries(searchengine_mt Threads::Threads)

# -----------------------------------------------------------------------
# Usage summary
# -----------------------------------------------------------------------
message(STATUS "")
message(STATUS "Build targets:")
message(STATUS "  searchengine     – original single-threaded binary")
message(STATUS "  searchengine_mt  – new multithreaded binary")
message(STATUS "")
message(STATUS "Run (single-threaded):")
message(STATUS "  ./searchengine -d data/dataset.txt -k 5")
message(STATUS "")
message(STATUS "Run (multithreaded, auto-detect cores):")
message(STATUS "  ./searchengine_mt -d data/dataset.txt -k 5")
message(STATUS "")
message(STATUS "Run (multithreaded, explicit thread count):")
message(STATUS "  ./searchengine_mt -d data/dataset.txt -k 5 -t 8")
message(STATUS "")
