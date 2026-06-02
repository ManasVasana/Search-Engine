cmake_minimum_required(VERSION 3.10)
project(SearchEngine)

set(CMAKE_CXX_STANDARD 11)

include_directories(header)

add_executable(searchengine src/Searchengine.cpp
src/Document_store.cpp
src/Trie.cpp
src/Score.cpp
src/Listnode.cpp
src/Search.cpp
src/Maxheap.cpp
src/Map.cpp)
