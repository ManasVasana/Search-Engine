#ifndef SEARCHENGINE_HPP
#define SEARCHENGINE_HPP

#include <iostream>
#include <cstring>
#include <cstdlib>
#include "Document_store.hpp"
#include "Map.hpp"
#include "Trie.hpp"
#include "Search.hpp"

// Function declaration
int inputmanager(char* input, TrieNode* trie, Mymap* mymap, int k);

#endif