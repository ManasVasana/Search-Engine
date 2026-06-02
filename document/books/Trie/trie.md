# Trie Data Structure - Concepts Documentation

This document explains the **concepts and theory** behind the Trie (prefix tree) data structure used in our search engine. For detailed code explanation, see `working.md`.

---

## Table of Contents

1. [What is a Trie?](#1-what-is-a-trie)
2. [Why Use a Trie for Search Engines?](#2-why-use-a-trie-for-search-engines)
3. [Trie Structure - Nodes and Pointers](#3-trie-structure---nodes-and-pointers)
4. [Sibling-Child Representation](#4-sibling-child-representation)
5. [Recursive Insert Algorithm](#5-recursive-insert-algorithm)
6. [Why char value Instead of char* word](#6-why-char-value-instead-of-char-word)
7. [Future Integration with Listnode](#7-future-integration-with-listnode)
8. [Time and Space Complexity](#8-time-and-space-complexity)

---

## 1. What is a Trie?

### Definition

A **Trie** (pronounced "try") is a tree-like data structure used for efficient storage and retrieval of strings. The name comes from "re**trie**val".

### Key Characteristics

- Each node represents a single character
- Root node is empty (value = -1 in our implementation)
- Words are formed by paths from root to leaf nodes
- Common prefixes share the same path

### Visual Example

```
Storing words: "hello", "help", "world"

           ROOT(-1)
          /        \
        h           w
        |           |
        e           o
        |           |
        l           r
       / \          |
      l   p         l
      |   |         |
      o  [✓]        d
      |            [✓]
     [✓]

[✓] = End of word marker (listnode will be attached here)
```

---

## 2. Why Use a Trie for Search Engines?

### Advantages

**1. Fast Prefix Search**
```cpp
// Find all words starting with "hel"
// Only traverse h → e → l
// Much faster than checking every word
```

**2. Efficient Storage**
- Words with common prefixes share nodes
- "hello" and "help" share "h", "e", "l"
- Saves memory compared to storing full strings

**3. Document Frequency (DF)**
- Each word endpoint stores list of documents containing it
- Quick lookup: "How many documents have 'hello'?"

**4. Term Frequency (TF)**
- Each listnode tracks: documentID and occurrence count
- Quick lookup: "How many times does 'hello' appear in doc 5?"

### Comparison with Alternatives

| Data Structure | Insert | Search | Prefix Search | Memory |
|---------------|--------|--------|---------------|--------|
| Array of strings | O(1) | O(n) | O(n·m) | High |
| Hash Table | O(1) | O(1) | O(n·m) | Medium |
| **Trie** | **O(m)** | **O(m)** | **O(m)** | **Optimal** |

*n = number of words, m = length of word*

---

## 3. Trie Structure - Nodes and Pointers

### TrieNode Structure

```cpp
class TrieNode {
    char value;          // Character stored in this node
    TrieNode* sibling;   // Next character at same level
    TrieNode* child;     // Next character in sequence
    // listnode* list;   // Documents containing this word (future)
};
```

### Three Pointer System

**1. value** - The character stored
- `-1` for root node (special marker)
- 'a'-'z', 'A'-'Z', etc. for actual characters

**2. sibling** - Horizontal navigation
- Points to alternative characters at same position
- Handles branching: "hello" and "help" have 'l' and 'p' as siblings

**3. child** - Vertical navigation
- Points to next character in word
- Follows the word sequence

**Visual Representation:**
```
        h → w  (siblings: different starting letters)
        ↓   ↓
        e   o  
        ↓   ↓
        l   r
       ↓ ↘  ↓
       l   p l  (siblings: "hello" vs "help")
       ↓   ↓ ↓
       o  [✓]d
       ↓     ↓
      [✓]   [✓]
```

---

## 4. Sibling-Child Representation

### Why Not Use Array of Children?

**Alternative 1: Array of 26 pointers (a-z)**
```cpp
TrieNode* children[26];  // ❌ Wastes memory for sparse data
```

**Problems:**
- Each node allocates 26 pointers (208 bytes on 64-bit)
- Most pointers are NULL (wasted space)
- Only works for lowercase English letters

**Alternative 2: HashMap**
```cpp
map<char, TrieNode*> children;  // ❌ Overhead for small datasets
```

**Problems:**
- Hash table overhead
- More complex memory management

**Our Solution: Sibling List** ✅
```cpp
TrieNode* sibling;  // Only allocates what's needed
TrieNode* child;    // Minimal memory usage
```

**Benefits:**
- Only creates nodes for characters that exist
- Works with any character set (not just a-z)
- Minimal memory overhead
- Simple pointer traversal

### How Siblings Work

**Storing "cat", "car", "dog":**

```
        c → d  (siblings)
        ↓   ↓
        a   o
        ↓   ↓
      t → r g  (siblings)
      ↓   ↓ ↓
     [✓] [✓][✓]
```

**Traversal Example:**
```
Insert "car":
1. Start at root, look for 'c'
   - Found 'c' → go to child
2. At 'a', look for 'a'
   - Found 'a' → go to child
3. At 't', look for 'r'
   - 't' != 'r' → go to sibling
   - Found 'r' → go to child
4. End of word → mark complete
```

---

## 5. Recursive Insert Algorithm

### How Recursive Insert Works

**Function Signature:**
```cpp
void insert(char* token, int id);
```

**Base Cases and Logic:**

**Case 1: Empty node (value == -1)**
```cpp
if(value == -1) {
    value = token[0];  // Store first character
    // Continue with rest of word
}
```

**Case 2: Character matches**
```cpp
if(value == token[0]) {
    if(strlen(token) == 1) {
        // End of word - attach listnode (future)
        return;
    } else {
        // Continue with child
        child->insert(token+1, id);
    }
}
```

**Case 3: Character doesn't match**
```cpp
else {
    // Try sibling
    sibling->insert(token, id);
}
```

### Visual Execution

**Inserting "hello" into empty trie:**

```
Step 1: Root receives "hello"
        value = -1 → Set to 'h'
        
Step 2: 'h' receives "ello"
        Create child, call child->insert("ello")
        
Step 3: New node receives "ello"
        value = -1 → Set to 'e'
        
Step 4: 'e' receives "llo"
        Create child, call child->insert("llo")
        
... continues until all characters inserted
```

**Inserting "help" when "hello" exists:**

```
Existing:  h → e → l → l → o

Insert "help":
1. 'h' matches → go to child with "elp"
2. 'e' matches → go to child with "lp"
3. 'l' matches → go to child with "p"
4. 'l' doesn't match 'p' → create sibling

Result:    h → e → l → l → o
                       ↓
                       p
```

### Why Recursion?

**Advantages:**
✅ Clean, elegant code  
✅ Naturally follows tree structure  
✅ Easy to understand and maintain  
✅ Matches the problem structure  

**Without Recursion (Iterative):**
```cpp
// Much more complex with manual stack management
void insert_iterative(char* token, int id) {
    TrieNode* current = this;
    // Complex loop with many conditions...
}
```

---

## 6. Why char value Instead of char* word

### Design Choice

Our implementation stores **one character per node** (`char value`) instead of whole strings (`char* word`).

**Our Design:**
```cpp
class TrieNode {
    char value;  // ✅ Single character
};
```

**Alternative Design:**
```cpp
class TrieNode {
    char* word;  // ❌ Entire string
};
```

### Reasons for Single Character

**1. Memory Efficiency**
```
Word: "hello"

Single char approach:
  5 nodes × 1 char = 5 bytes

String approach:
  Could store "hello" at end: 6 bytes
  But loses prefix sharing advantage
```

**2. Prefix Sharing**
```
Words: "hello", "help"

Single char: Share h→e→l (3 nodes)
             Split at l→l and l→p

String: Each word stored separately
        No sharing possible
```

**3. True Trie Structure**
- Classic Trie definition: one character per node
- Enables all Trie algorithms and optimizations
- Standard implementation pattern

**4. Flexible Operations**
```cpp
// Easy to implement:
- Prefix search: traverse partial path
- Autocomplete: return all children from prefix
- Spell check: find similar paths
```

### Alternative: Compressed Trie (PATRICIA)

**Compressed Trie** stores strings in nodes:
```
Normal Trie:     h → e → l → l → o
Compressed:      hell → o
```

**Trade-offs:**
- ✅ Fewer nodes
- ❌ More complex insertion
- ❌ Harder to search partial prefixes

We chose **standard Trie** for simplicity and learning value.

---

## 7. Future Integration with Listnode

### Current State (Incomplete)

```cpp
class TrieNode {
    char value;
    TrieNode* sibling;
    TrieNode* child;
    // listnode* list;  // ← COMMENTED OUT (not yet implemented)
};
```

### Why Listnode is Needed

When a word is complete, we need to track:
1. **Which documents** contain this word (document IDs)
2. **How many times** the word appears in each document (frequency)

### Complete Implementation (Future)

```cpp
class TrieNode {
    char value;
    TrieNode* sibling;
    TrieNode* child;
    listnode* list;  // ✅ Will be used for word completion
};

void insert(char* token, int id) {
    if(value == -1 || value == token[0]) {
        value = token[0];
        if(strlen(token) == 1) {
            // Word complete!
            if(list == NULL) {
                list = new listnode(id);  // First occurrence
            }
            list->add(id);  // Track document and frequency
            return;
        }
        // ... continue with child
    }
}
```

### How It Will Work

**Example: Inserting "hello" in different documents**

```
Document 1: "hello world hello earth"
→ insert("hello", 1) twice

Document 2: "hello"
→ insert("hello", 2) once

Trie structure:
h → e → l → l → o [list]
                    ↓
                  listnode
                  {id:1, times:2} → {id:2, times:1} → NULL
```

**Query Operations:**
```cpp
// Term Frequency: How many times "hello" in doc 1?
trie->tfsearchword(1, "hello") → returns 2

// Document Frequency: How many docs contain "hello"?
trie->dsearchword("hello") → returns 2 (doc1 and doc2)
```

### Why Not Implemented Yet?

**Current Phase:** Building foundation
- ✅ Trie structure complete
- ✅ Insert basic characters
- ✅ Memory management working

**Next Phase:** Add search functionality
- ⏳ Uncomment listnode integration
- ⏳ Implement tfsearchword()
- ⏳ Implement dsearchword()
- ⏳ Add BM25 scoring

---

## 8. Time and Space Complexity

### Time Complexity

**Insert Operation:**
- **Best Case:** O(m) where m = length of word
- **Worst Case:** O(m) 
- **Average:** O(m)

Each character requires one recursive call. No searching through arrays.

**Search Operation (Future):**
- **Time:** O(m) where m = length of word
- Just follow the path, one character at a time

**Prefix Search (Future):**
- **Time:** O(m + k) where m = prefix length, k = number of results
- Find prefix: O(m)
- List all completions: O(k)

### Space Complexity

**Per Node:**
```cpp
sizeof(TrieNode) = 
    1 byte (char value) +
    8 bytes (TrieNode* sibling) +
    8 bytes (TrieNode* child) +
    8 bytes (listnode* list - future) +
    padding
= ~32 bytes per node
```

**Total Space:**
- **Best Case:** O(n) where n = total characters in all unique words
- **Worst Case:** O(n × 26) if every word is completely unique
- **Average:** O(n) due to prefix sharing

**Example:**
```
Words: "hello", "help", "world" (14 total chars)
Shared: h→e→l (3 chars)
Unique: l→o, p, w→o→r→l→d (9 chars)
Total nodes: 12 (instead of 14 without sharing)
```

### Comparison with Hash Table

| Metric | Trie | Hash Table |
|--------|------|------------|
| Insert | O(m) | O(1) |
| Search | O(m) | O(1) |
| Prefix Search | O(m+k) | O(n·m) |
| Memory | Optimal with sharing | Higher |
| Order | Lexicographic | Random |

**When to use Trie:**
✅ Prefix searches (autocomplete)  
✅ Lexicographic order matters  
✅ Words share common prefixes  
✅ Need document frequency tracking  

**When to use Hash Table:**
✅ Only exact match searches  
✅ No prefix queries needed  
✅ Constant time critical  

---

## Summary

### Key Concepts

1. **Trie** - Tree structure for efficient string storage and retrieval
2. **One Character Per Node** - Classic Trie design for prefix sharing
3. **Sibling-Child Pointers** - Memory-efficient representation
4. **Recursive Insert** - Clean, natural algorithm
5. **Listnode Integration** - Future feature for search functionality
6. **O(m) Operations** - Fast insert and search

### Why Trie for Search Engine?

✅ Fast prefix search (autocomplete)  
✅ Efficient document frequency queries  
✅ Natural term frequency tracking  
✅ Memory-efficient prefix sharing  
✅ Foundation for BM25 ranking algorithm  

### Current Status

✅ Basic Trie structure implemented  
✅ Insert operation working  
✅ Memory management safe  
⏳ Listnode integration pending  
⏳ Search operations pending  
⏳ BM25 scoring pending  

---

**Document Version**: 1.0  
**Last Updated**: December 26, 2025  
**Author**: High-Performance Search Engine Project  
**Repository**: github.com/adarshpheonix2810/high-performance-search-engine-cpp
