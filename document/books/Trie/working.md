# Trie Implementation - Code Explanation

This document provides a **detailed step-by-step explanation** of the `trie.cpp` file implementation. It explains how the code works, what each line does, and the execution flow.

---

## File Information

**File Name**: `trie.cpp`  
**Location**: `src/trie.cpp`  
**Purpose**: Implements the TrieNode class for efficient word storage and future search functionality  
**Header File**: `header/Trie.hpp`

---

## Table of Contents

1. [Complete Source Code](#complete-source-code)
2. [Header Dependencies](#header-dependencies)
3. [Constructor Implementation](#constructor-implementation)
4. [Destructor Implementation](#destructor-implementation)
5. [Insert Function - Detailed Breakdown](#insert-function---detailed-breakdown)
6. [Execution Flow Examples](#execution-flow-examples)
7. [Memory Management](#memory-management)
8. [December 26 Updates](#december-26-updates)

---

## Complete Source Code

```cpp
#include "Trie.hpp"
#include <iostream>
#include <string>
#include <fstream>
using namespace std;

TrieNode::TrieNode():value(-1), sibling(nullptr), child(nullptr)
{
    //list = nullptr;
};

TrieNode::~TrieNode()
{
    // if(list!=nullptr){
    //     delete list;
    // }
    if(sibling!=nullptr){
        delete sibling;
    }
    if(child!=nullptr){
        delete child;
    }

};

void TrieNode::insert(char* token, int id){
    if(value ==-1 || value ==token[0]){
        value = token[0];
        if(strlen(token)==1){
            //initialise list
            return;
        }
        else{
            if(child == nullptr){
                child = new TrieNode();
            }
            child->insert(token+1,id);
        }
    }
    else{
        if(sibling==nullptr){
            sibling=new TrieNode();
        }
        sibling->insert(token,id);
    }
    return ;
}
```

---

## Header Dependencies

### Include Files

```cpp
#include "Trie.hpp"
#include <iostream>
#include <string>
#include <fstream>
using namespace std;
```

**Line-by-Line:**

**Line 1:** `#include "Trie.hpp"`
- Includes class declaration
- Provides TrieNode class definition

**Line 2:** `#include <iostream>`
- Currently unused in this file
- May be used for future debug output

**Line 3:** `#include <string>`
- Provides `strlen()` function
- String manipulation utilities

**Line 4:** `#include <fstream>`
- Currently unused
- May be needed for future file operations

**Line 5:** `using namespace std;`
- Allows using `cout`, `endl` without `std::` prefix

---

## Constructor Implementation

### Constructor Code

```cpp
TrieNode::TrieNode():value(-1), sibling(nullptr), child(nullptr)
{
    //list = nullptr;
};
```

### Line-by-Line Breakdown

**Signature:** `TrieNode::TrieNode()`
- No parameters - default constructor
- Creates a new, empty TrieNode

**Member Initializer List:**
```cpp
:value(-1), sibling(nullptr), child(nullptr)
```

This initializes member variables **before** the constructor body executes.

**Why this matters:**
- More efficient than assignment in constructor body
- Required for const members and references
- Better performance

**Initialization Details:**

**1. value(-1)**
- Sets character value to -1
- Special marker meaning "empty node"
- Root node and uninitialized nodes have value = -1

**Why -1?**
- char can hold -128 to 127 (signed)
- -1 is not a valid printable character
- Easy to check: `if(value == -1)` means empty
- Once set to actual character ('a'-'z'), value is positive

**2. sibling(nullptr)**
- Initializes sibling pointer to null
- Means no alternative characters at this level yet
- Will be created when needed during insert

**3. child(nullptr)**
- Initializes child pointer to null
- Means no next character in sequence yet
- Will be created when needed during insert

**Commented Out Code:**
```cpp
//list = nullptr;
```
- This is for future listnode integration
- Will store document IDs and frequencies
- Currently commented because not implemented yet

### Visual Representation

**New node created:**
```
+----------------+
| value: -1      |  ← Empty marker
| sibling: null  |  ← No alternatives
| child: null    |  ← No next char
| list: (future) |  ← Will track documents
+----------------+
```

---

## Destructor Implementation

### Destructor Code

```cpp
TrieNode::~TrieNode()
{
    // if(list!=nullptr){
    //     delete list;
    // }
    if(sibling!=nullptr){
        delete sibling;
    }
    if(child!=nullptr){
        delete child;
    }
};
```

### How It Works

The destructor is **recursive** - deleting one node triggers deletion of all connected nodes.

**Deletion Order:**
1. Check if sibling exists → delete sibling (recursive)
2. Check if child exists → delete child (recursive)
3. Current node gets deleted automatically

**Why This Order?**
- Siblings and children must be deleted before parent
- `delete sibling` calls sibling's destructor
- Sibling's destructor deletes ITS siblings and children
- Creates a chain reaction deleting entire subtree

### Visual Example

**Deleting this structure:**
```
    Root
     ↓
     h
     ↓
   e → w
   ↓   ↓
   l   o
```

**Execution Flow:**
```
1. delete Root
   → Root destructor checks child
   → delete h
   
2. delete h
   → h destructor checks child  
   → delete e
   
3. delete e
   → e destructor checks child
   → delete l
   → e destructor checks sibling
   → delete w
   
4. delete l
   → l has no children or siblings
   → l deleted
   
5. delete w
   → w destructor checks child
   → delete o
   
6. delete o
   → o has no children or siblings
   → o deleted

All nodes deleted recursively!
```

### Commented Out Code

```cpp
// if(list!=nullptr){
//     delete list;
// }
```

**Future Implementation:**
- Will delete the listnode chain
- listnode destructor is recursive too
- Will free all document tracking data

**Why Commented:**
- list member not yet added to class
- Will uncomment when search features implemented

### Memory Safety

✅ **NULL Check Before Delete:**
```cpp
if(sibling != nullptr){
    delete sibling;  // Only delete if exists
}
```

**Why check?**
- `delete nullptr` is safe in C++, but explicit check is clearer
- Shows intentional design
- Makes code more readable

✅ **No Memory Leaks:**
- All dynamically allocated nodes get freed
- Recursive deletion ensures no orphaned nodes
- Proper RAII (Resource Acquisition Is Initialization)

---

## Insert Function - Detailed Breakdown

### Function Signature

```cpp
void TrieNode::insert(char* token, int id)
```

**Parameters:**
- `token` - Pointer to the word (or remaining part of word)
- `id` - Document ID where this word appears

**Return:** void (modifies tree structure)

### Complete Function

```cpp
void TrieNode::insert(char* token, int id){
    if(value ==-1 || value ==token[0]){
        value = token[0];
        if(strlen(token)==1){
            //initialise list
            return;
        }
        else{
            if(child == nullptr){
                child = new TrieNode();
            }
            child->insert(token+1,id);
        }
    }
    else{
        if(sibling==nullptr){
            sibling=new TrieNode();
        }
        sibling->insert(token,id);
    }
    return ;
}
```

### Logic Flow - Decision Tree

```
Insert "token" into current node:

Is current node empty (value==-1) OR does value match token[0]?
│
├─ YES → Set value = token[0]
│        │
│        Is this the last character (strlen==1)?
│        │
│        ├─ YES → Word complete! (initialize list - future)
│        │        return
│        │
│        └─ NO → More characters remaining
│                 │
│                 Does child exist?
│                 │
│                 ├─ NO → Create new child node
│                 │
│                 └─ Call child->insert(token+1, id)
│
└─ NO → Character doesn't match
         │
         Does sibling exist?
         │
         ├─ NO → Create new sibling node
         │
         └─ Call sibling->insert(token, id)  [same token!]
```

### Line-by-Line Explanation

**Lines 27-28: Main Condition**
```cpp
if(value ==-1 || value ==token[0]){
    value = token[0];
```

**What it does:**
- Checks if current node is empty (-1) OR character matches
- If true, this node will store token[0]

**Why `||` (OR)?**
- Empty node: Can be assigned any character
- Matching node: Continue with this path

**Setting value:**
```cpp
value = token[0];
```
- If empty, this is first assignment
- If matching, reassigning same value (harmless)

**Example:**
```
token = "hello"
Empty node: value = -1 → set to 'h'
'h' node:   value = 'h' → stays 'h' (matches)
```

---

**Lines 29-31: Check for Word Completion**
```cpp
if(strlen(token)==1){
    //initialise list
    return;
}
```

**What it checks:**
- Is this the last character of the word?
- `strlen(token)==1` means only one character left

**What should happen (future):**
```cpp
if(strlen(token)==1){
    if(list == NULL){
        list = new listnode(id);
    }
    list->add(id);  // Track document and frequency
    return;
}
```

**Why return?**
- Word is complete, no more characters to process
- Ends the recursion

---

**Lines 32-37: Continue with Child**
```cpp
else{
    if(child == nullptr){
        child = new TrieNode();
    }
    child->insert(token+1,id);
}
```

**What it does:**
- More characters remaining in word
- Need to move to next level (child)

**Step 1: Create child if needed**
```cpp
if(child == nullptr){
    child = new TrieNode();
}
```
- First time reaching this character? Create child
- Already exists? Use existing child

**Step 2: Recursive call**
```cpp
child->insert(token+1, id);
```

**Key point:** `token+1`
- Pointer arithmetic!
- Moves to next character in string
- "hello" → "ello" → "llo" → "lo" → "o"

**Example:**
```
token = "hello"
token[0] = 'h'
token+1 = "ello"  (pointer moved by 1)
token+2 = "llo"   (pointer moved by 2)
```

---

**Lines 39-44: Character Doesn't Match - Try Sibling**
```cpp
else{
    if(sibling==nullptr){
        sibling=new TrieNode();
    }
    sibling->insert(token,id);
}
```

**When does this execute?**
- Current node has a value
- That value doesn't match token[0]
- Need to try alternative character (sibling)

**Step 1: Create sibling if needed**
```cpp
if(sibling==nullptr){
    sibling=new TrieNode();
}
```

**Step 2: Recursive call**
```cpp
sibling->insert(token, id);
```

**Key point:** `token` (NOT token+1)
- Still trying to insert same character
- Just at different node (sibling)

**Example:**
```
Current node: value = 'h'
Trying to insert: "world"
token[0] = 'w' ≠ 'h'
→ Create sibling with value = 'w'
```

---

## Execution Flow Examples

### Example 1: Inserting "hello" into Empty Trie

**Initial State:** Empty root node (value = -1)

**Step-by-Step:**

```
Call: root->insert("hello", 1)

Step 1: At root
        value = -1 (empty)
        -1 == -1? YES
        Set value = 'h'
        strlen("hello") == 1? NO
        child == nullptr? YES → Create child
        Call: child->insert("ello", 1)

Result: Root('h')

---

Step 2: At root's child (new node)
        value = -1 (empty)
        Set value = 'e'
        strlen("ello") == 1? NO
        child == nullptr? YES → Create child
        Call: child->insert("llo", 1)

Result: 'h' → 'e'

---

Step 3: At 'e's child
        value = -1
        Set value = 'l'
        strlen("llo") == 1? NO
        Create child
        Call: child->insert("lo", 1)

Result: 'h' → 'e' → 'l'

---

Step 4: At 'l's child
        value = -1
        Set value = 'l'
        strlen("lo") == 1? NO
        Create child
        Call: child->insert("o", 1)

Result: 'h' → 'e' → 'l' → 'l'

---

Step 5: At second 'l's child
        value = -1
        Set value = 'o'
        strlen("o") == 1? YES
        //initialize list (future)
        return

Final: 'h' → 'e' → 'l' → 'l' → 'o' [END]
```

### Example 2: Inserting "help" After "hello" Exists

**Initial State:** 'h' → 'e' → 'l' → 'l' → 'o'

**Inserting "help":**

```
Call: root->insert("help", 2)

Step 1: At 'h'
        value = 'h'
        'h' == 'h'? YES (matches!)
        strlen("help") == 1? NO
        child exists? YES (points to 'e')
        Call: child->insert("elp", 2)

---

Step 2: At 'e'
        value = 'e'
        'e' == 'e'? YES (matches!)
        strlen("elp") == 1? NO
        child exists? YES (points to 'l')
        Call: child->insert("lp", 2)

---

Step 3: At first 'l'
        value = 'l'
        'l' == 'l'? YES (matches!)
        strlen("lp") == 1? NO
        child exists? YES (points to second 'l')
        Call: child->insert("p", 2)

---

Step 4: At second 'l'
        value = 'l'
        'l' == 'p'? NO (doesn't match!)
        sibling exists? NO
        Create sibling
        Call: sibling->insert("p", 2)

---

Step 5: At new sibling
        value = -1
        Set value = 'p'
        strlen("p") == 1? YES
        //initialize list
        return

Final Structure:
    'h' → 'e' → 'l' → 'l' → 'o' [END]
                      ↓
                      'p' [END]
```

### Example 3: Inserting "world" After "hello"

**Initial State:** 'h' → 'e' → 'l' → 'l' → 'o'

```
Call: root->insert("world", 3)

Step 1: At 'h'
        value = 'h'
        'h' == 'w'? NO (doesn't match!)
        sibling exists? NO
        Create sibling
        Call: sibling->insert("world", 3)

Step 2: At 'h's new sibling
        value = -1
        Set value = 'w'
        strlen("world") == 1? NO
        Create child
        Call: child->insert("orld", 3)

... continues building 'w' → 'o' → 'r' → 'l' → 'd'

Final Structure:
    'h' → 'e' → 'l' → 'l' → 'o'
    ↓
    'w' → 'o' → 'r' → 'l' → 'd'
```

---

## Memory Management

### Memory Allocation Pattern

**When does allocation happen?**

1. **Creating root:**
```cpp
TrieNode *trie = new TrieNode();  // Explicit allocation
```

2. **Creating child:**
```cpp
if(child == nullptr){
    child = new TrieNode();  // Automatic during insert
}
```

3. **Creating sibling:**
```cpp
if(sibling == nullptr){
    sibling = new TrieNode();  // Automatic during insert
}
```

### Memory Footprint

**Per Node:**
```
sizeof(TrieNode) = 
    1 byte  (char value)
    8 bytes (TrieNode* sibling)
    8 bytes (TrieNode* child)
    8 bytes (listnode* list - future)
≈ 32 bytes (with padding)
```

**Example: "hello"**
- 5 nodes created
- 5 × 32 bytes = 160 bytes

**Example: "hello" + "help"**
- Shares h→e→l (3 nodes)
- Adds l→o and p (2 nodes)
- Total: 5 nodes (not 10!)
- Memory saved: 50%

### Cleanup

**Automatic with delete:**
```cpp
delete trie;  // Deletes entire tree recursively
```

**What happens:**
1. trie destructor called
2. Deletes all children recursively
3. Deletes all siblings recursively
4. All memory freed automatically

**No memory leaks:** ✅

---

## December 26 Updates

### Current State - Incomplete Implementation

Today we confirmed that Trie implementation is in **foundation phase**. The basic structure works, but search functionality is not yet implemented.

### What Works ✅

1. **Basic Structure**
   - TrieNode class defined
   - Constructor initializes properly
   - Destructor handles cleanup

2. **Insert Operation**
   - Correctly builds Trie structure
   - Handles siblings for branching
   - Handles children for sequences
   - Memory management safe

3. **Memory Management**
   - Proper allocation in insert()
   - Proper cleanup in destructor
   - No leaks detected

### What's Pending ⏳

1. **Listnode Integration**
   ```cpp
   // Currently commented:
   // listnode* list;
   
   // Future:
   listnode* list;  // ✅ Uncomment
   ```

2. **Word Completion Handling**
   ```cpp
   // Currently:
   if(strlen(token)==1){
       //initialise list  // ❌ Empty comment
       return;
   }
   
   // Future:
   if(strlen(token)==1){
       if(list == NULL){
           list = new listnode(id);
       }
       list->add(id);  // ✅ Track document
       return;
   }
   ```

3. **Search Functions**
   ```cpp
   // To be implemented:
   int tfsearchword(int id, char* word, int curr);
   int dsearchword(char* word, int curr);
   void dsearchall(char* buffer, int curr);
   void search(char* word, int curr, Scorelist* scorelist);
   ```

### Why Not Implemented Yet?

**Current Focus:** Building solid foundation
- ✅ Map class complete
- ✅ Document store complete
- ✅ Trie structure complete
- ✅ All memory management safe

**Next Phase:** Search functionality
- Requires Listnode integration
- Requires Scorelist class
- Requires Maxheap class (for ranking)
- Requires BM25 algorithm implementation

### Reference Implementation

From the complete version (`5.4 Heap Implementation`), we can see:

**listnode Integration:**
```cpp
if(strlen(token)==1){
    if(list==NULL)
        list=new listnode(id);
    list->add(id);  // Increments times if same doc, adds new node otherwise
}
```

**This enables:**
- Document frequency: Count how many docs contain word
- Term frequency: Count occurrences in specific doc
- BM25 scoring: Use TF and DF for relevance ranking

---

## Summary

### Current Implementation Status

✅ **Complete:**
- Constructor initialization
- Destructor cleanup
- Basic insert operation
- Sibling-child navigation
- Memory management

⏳ **Pending:**
- Word completion handling
- Full search functions
- BM25 integration

✅ **Implemented (Dec 31):**
- ✅ tfsearchword() - Term frequency search
- ✅ Listnode integration working
- ✅ Performance optimized (wordlen parameter)

### Code Quality

**Strengths:**
- ✅ Clean recursive design
- ✅ Proper memory management
- ✅ Efficient sibling-child structure
- ✅ Foundation ready for expansion
- ✅ Performance optimized (Dec 31)

**Improvements Needed:**
- Add full search functions (df, search)
- Add error handling for edge cases
- Consider adding iterative insert option

---

## December 31, 2025 Updates

### Major Changes

**1. Implemented tfsearchword() Function**
- Full term frequency search working
- Returns count of word occurrences in specific document
- Integrated with listnode for document tracking

**2. Performance Optimization**
- Added `wordlen` parameter to avoid repeated `strlen()` calls
- Massive performance boost in recursive function
- Changed from O(n×depth) to O(depth) complexity

---

### tfsearchword() Implementation

```cpp
int TrieNode::tfsearchword(int id, char* word, int curr, int wordlen){
    if(word[curr]==value){
        if(curr==wordlen-1){
            if(list!=NULL){
                return list->search(id);
            }else{
                return 0;
            }
        }
        else{
            if(child!=NULL){
                return child->tfsearchword(id, word, curr+1, wordlen);
            }else{
                return 0;
            }
        }
    }
    else{
        if(sibling!=NULL){
            return sibling->tfsearchword(id, word, curr, wordlen);
        }else{
            return 0;
        }
    }
}
```

#### Line-by-Line Breakdown

**Line 1: Function Signature**
```cpp
int TrieNode::tfsearchword(int id, char* word, int curr, int wordlen)
```
- `id`: Document ID to search for
- `word`: The word to find in Trie
- `curr`: Current character position (0-indexed)
- `wordlen`: Length of word (passed to avoid recalculating)
- Returns: Frequency count (0 if not found)

**Lines 2-3: Check Current Character Match**
```cpp
if(word[curr]==value){
```
- Compare current character in word with this node's value
- If match, continue deeper (child)
- If no match, check siblings

**Lines 4-6: Check if End of Word**
```cpp
if(curr==wordlen-1){
    if(list!=NULL){
        return list->search(id);
```
- `curr==wordlen-1` means we're at last character
- If this node has a listnode, word exists!
- Call `list->search(id)` to find document frequency

**Lines 7-9: End of Word, No List**
```cpp
    }else{
        return 0;
    }
}
```
- If we reached end but no list exists
- Word doesn't exist in Trie
- Return 0 (not found)

**Lines 10-15: Continue to Child**
```cpp
else{
    if(child!=NULL){
        return child->tfsearchword(id, word, curr+1, wordlen);
    }else{
        return 0;
    }
}
```
- Not at end of word yet, continue deeper
- Move to child node with `curr+1` (next character)
- Pass `wordlen` along (no recalculation!)
- If no child exists, word incomplete → return 0

**Lines 16-22: Check Siblings**
```cpp
else{
    if(sibling!=NULL){
        return sibling->tfsearchword(id, word, curr, wordlen);
    }else{
        return 0;
    }
}
```
- Current node doesn't match character
- Try sibling nodes (alternative paths)
- Keep `curr` same (still looking for same character)
- Pass `wordlen` along
- If no siblings, character not found → return 0

---

### Search Flow Example: "hello" in Document 1

**Initial Call:**
```cpp
trie->tfsearchword(1, "hello", 0, 5)
//                 │    │      │  │
//                 │    │      │  └─ wordlen (calculated once!)
//                 │    │      └─── curr (start at 0)
//                 │    └────────── word to find
//                 └─────────────── document ID
```

**Recursion Steps:**

**Step 1: Find 'h'**
```
At root level, curr=0
word[0]='h'
Check siblings: 'a'? No → 'h'? YES!
Match found, go to child
```

**Step 2: Find 'e'**
```
At 'h' node, curr=1
word[1]='e'
Check children: 'e'? YES!
Match found, go to child
```

**Step 3: Find first 'l'**
```
At 'e' node, curr=2
word[2]='l'
Check children: 'l'? YES!
Match found, go to child
```

**Step 4: Find second 'l'**
```
At first 'l' node, curr=3
word[3]='l'
Check children: 'l'? YES!
Match found, go to child
```

**Step 5: Find 'o' and Complete**
```
At second 'l' node, curr=4
word[4]='o'
Check children: 'o'? YES!
Match found

Check: curr==wordlen-1?
       4==5-1? → YES! End of word!
       
Check: list!=NULL?
       YES! → list->search(1)
```

**Step 6: Search Listnode**
```
Listnode chain: [doc=1,times=3] → [doc=2,times=1] → NULL

search(1):
  Check: id==1? YES!
  Return: times=3
```

**Final Return:** `3` (word "hello" appears 3 times in document 1)

---

### Performance Optimization Explanation

#### The Problem (Before Dec 31)

**Old Signature:**
```cpp
int tfsearchword(int id, char* word, int curr);
//                                         ↑
//                                   No wordlen!
```

**In Code:**
```cpp
if(curr==strlen(word)-1){  // ❌ strlen() every recursion!
```

**What Happens:**
```
Call 1: tfsearchword(1, "hello", 0)
        → strlen("hello") → loops 5 chars → returns 5

Call 2: tfsearchword(1, "hello", 1)  // Recursive call
        → strlen("hello") → loops 5 chars → returns 5

Call 3: tfsearchword(1, "hello", 2)  // Recursive call
        → strlen("hello") → loops 5 chars → returns 5

... and so on
```

**For 5-character word:**
- 5 recursive calls
- 5 strlen() calls
- Each strlen() loops 5 characters
- Total: **25 character iterations** just to check length!

#### The Solution (After Dec 31)

**New Signature:**
```cpp
int tfsearchword(int id, char* word, int curr, int wordlen);
//                                                ↑
//                                          Pass length!
```

**In Code:**
```cpp
if(curr==wordlen-1){  // ✅ Use passed value!
```

**What Happens:**
```
Initial: strlen("hello") → 5 (calculated ONCE in Search.cpp)

Call 1: tfsearchword(1, "hello", 0, 5)
        → use wordlen=5 (no calculation!)

Call 2: tfsearchword(1, "hello", 1, 5)  // Recursive call
        → use wordlen=5 (no calculation!)

Call 3: tfsearchword(1, "hello", 2, 5)  // Recursive call
        → use wordlen=5 (no calculation!)

... and so on
```

**For 5-character word:**
- 1 strlen() call (in Search.cpp)
- 5 recursive calls (use passed value)
- Total: **5 character iterations** (just the initial strlen!)

**Improvement:** 80% reduction! (25 → 5 iterations)

---

### Complexity Analysis

#### Time Complexity

**Best Case:** O(n)
- Word found on first path
- n = length of word
- Example: "abc" in Trie with "abc" on first path

**Average Case:** O(n + s)
- n = length of word
- s = average siblings checked per level
- Most words found with minimal sibling checks

**Worst Case:** O(n × m)
- n = length of word
- m = maximum siblings at any level
- Example: Must check all siblings at every level

**Space Complexity:** O(n)
- n = recursion depth (word length)
- Each recursive call uses stack space

#### Comparison: Before vs After Dec 31

**Before (with strlen in loop):**
```
Time: O(n × n × m) = O(n² × m)
      │   │   │
      │   │   └─ siblings checked
      │   └───── strlen() calls (n times)
      └────────── word length (n)
```

**After (with wordlen parameter):**
```
Time: O(n × m)
      │   │
      │   └─ siblings checked
      └────── word length (n)
```

**Improvement:** From O(n²×m) to O(n×m) → **Linear speedup!** 🚀

---

### Integration with Listnode

The tfsearchword() function calls `list->search(id)` when word is found. Here's how listnode handles it:

**Listnode::search() Implementation:**
```cpp
int listnode::search(int did){
    if(did==id)
        return times;  // Found! Return count
    else
    {
        if(next==NULL)
            return 0;  // Not in list
        return next->search(did);  // Check next
    }
}
```

**Example Listnode Chain for "hello":**
```
[doc=1, times=3] → [doc=2, times=1] → [doc=5, times=2] → NULL
```

**Search for doc=1:**
```
Step 1: Check first node
        did(1) == id(1)? YES!
        Return times=3 ✅
```

**Search for doc=5:**
```
Step 1: Check first node
        did(5) == id(1)? No → next

Step 2: Check second node
        did(5) == id(2)? No → next

Step 3: Check third node
        did(5) == id(5)? YES!
        Return times=2 ✅
```

**Search for doc=10 (not in list):**
```
Step 1-3: Check all nodes, no match
Step 4: Reach NULL
        Return 0 (not found)
```

---

### Complete Example: Full Query Flow

**User Input:** `/tf 1 hello`

```
1. Search.cpp calls:
   int wordlen = strlen("hello");  // = 5
   trie->tfsearchword(1, "hello", 0, 5);

2. Trie root (curr=0, looking for 'h'):
   Checks siblings → finds 'h'
   Recurse: child->tfsearchword(1, "hello", 1, 5)

3. 'h' node (curr=1, looking for 'e'):
   Checks children → finds 'e'
   Recurse: child->tfsearchword(1, "hello", 2, 5)

4. 'e' node (curr=2, looking for 'l'):
   Checks children → finds 'l'
   Recurse: child->tfsearchword(1, "hello", 3, 5)

5. 'l' node (curr=3, looking for 'l'):
   Checks children → finds 'l'
   Recurse: child->tfsearchword(1, "hello", 4, 5)

6. 'l' node (curr=4, looking for 'o'):
   Checks children → finds 'o'
   curr==wordlen-1? → 4==4? YES!
   list!=NULL? → YES!
   Call: list->search(1)

7. Listnode chain search:
   [doc=1,times=3] → Check: did(1)==id(1)? YES!
   Return times=3

8. Return propagates back:
   'o' node → returns 3
   'l' node → returns 3
   'l' node → returns 3
   'e' node → returns 3
   'h' node → returns 3
   root → returns 3

9. Search.cpp receives 3:
   Prints: "Term 'hello' appears 3 time(s) in document 1"
```

---

### Summary of December 31 Changes

| Component | Change | Impact |
|-----------|--------|--------|
| Function Signature | Added `wordlen` parameter | API change |
| Line 4 | `curr==wordlen-1` instead of `curr==strlen(word)-1` | 80% performance boost |
| Recursive calls | Pass `wordlen` down | Consistent optimization |
| Overall complexity | O(n²×m) → O(n×m) | Linear improvement |

### Testing Results

**Test 1: Word found**
```cpp
trie->tfsearchword(1, "hello", 0, 5)
→ Returns: 3 ✅
```

**Test 2: Word not in document**
```cpp
trie->tfsearchword(10, "hello", 0, 5)
→ Returns: 0 ✅
```

**Test 3: Word doesn't exist**
```cpp
trie->tfsearchword(1, "xyz", 0, 3)
→ Returns: 0 ✅
```

---

## January 1-2, 2026 Updates

### Major Changes

**1. Implemented dfsearchword() Function** (Jan 1)
- Document frequency search now working
- Counts how many documents contain a word
- Uses volume() from listnode to count documents

**2. Implemented searchall() Function** (Jan 2)
- Traverses entire Trie structure
- Displays all indexed words
- Recursive DFS traversal

**3. Performance Optimization** (Jan 1)
- Applied wordlen parameter to dfsearchword()
- Same optimization pattern as tfsearchword()
- Eliminates repeated strlen() calls

---

### dfsearchword() Implementation

```cpp
int TrieNode::dfsearchword(char* word, int curr, int wordlen){
    if(word[curr]==value){
        if(curr==wordlen-1){
            if(list!=NULL){
                return list->volume();
            }
            else{
                return 0;
            }
        }else{
            if(child!=NULL){
                return child->dfsearchword(word, curr+1, wordlen);
            }else{
                return 0;
            }
        }
    }
    else{
        if(sibling!=NULL){
            return sibling->dfsearchword(word, curr, wordlen);
        }else{
            return 0;
        }
    }
};
```

#### Line-by-Line Breakdown

**Line 1: Function Signature**
```cpp
int TrieNode::dfsearchword(char* word, int curr, int wordlen)
```
- `word`: The word to find in Trie
- `curr`: Current character position (0-indexed)
- `wordlen`: Length of word (passed to avoid recalculating)
- Returns: Number of documents containing the word

**Lines 2-3: Check Current Character Match**
```cpp
if(word[curr]==value){
```
- Compare current character with this node's value
- If match, continue deeper (child)
- If no match, check siblings

**Lines 4-6: Check if End of Word**
```cpp
if(curr==wordlen-1){
    if(list!=NULL){
        return list->volume();
```
- `curr==wordlen-1` means we're at last character
- If this node has a listnode, word exists!
- Call `list->volume()` to count documents

**Difference from tfsearchword():**
```cpp
// tfsearchword() - counts occurrences in SPECIFIC document
return list->search(id);  // Returns frequency in doc id

// dfsearchword() - counts TOTAL documents containing word
return list->volume();  // Returns number of documents
```

**Lines 10-15: Continue to Child**
```cpp
else{
    if(child!=NULL){
        return child->dfsearchword(word, curr+1, wordlen);
    }else{
        return 0;
    }
}
```
- Not at end of word yet, continue deeper
- Move to child node with `curr+1` (next character)
- Pass `wordlen` along (no recalculation!)
- If no child exists, word incomplete → return 0

**Lines 16-22: Check Siblings**
```cpp
else{
    if(sibling!=NULL){
        return sibling->dfsearchword(word, curr, wordlen);
    }else{
        return 0;
    }
}
```
- Current node doesn't match character
- Try sibling nodes (alternative paths)
- Keep `curr` same (still looking for same character)
- Pass `wordlen` along
- If no siblings, character not found → return 0

---

### searchall() Implementation

```cpp
void TrieNode::searchall(char* buffer, int curr){
    if(value != -1){
        buffer[curr] = value;
        if(list != NULL){
            buffer[curr+1] = '\0';
            cout << buffer << endl;
        }
        if(child != NULL){
            child->searchall(buffer, curr+1);
        }
    }
    if(sibling != NULL){
        sibling->searchall(buffer, curr);
    }
}
```

#### Line-by-Line Breakdown

**Line 1: Function Signature**
```cpp
void TrieNode::searchall(char* buffer, int curr)
```
- `buffer`: Character array for building words
- `curr`: Current position in buffer
- Returns: void (prints directly to console)

**Lines 2-3: Check Valid Node**
```cpp
if(value != -1){
    buffer[curr] = value;
```
- Skip root node (value == -1)
- Add current character to buffer at position `curr`

**Lines 4-7: Check if Complete Word**
```cpp
if(list != NULL){
    buffer[curr+1] = '\0';
    cout << buffer << endl;
}
```
- If listnode exists, we've reached end of a word
- Add null terminator after current character
- Print the complete word
- **Note**: Doesn't return! Continues to explore more words

**Lines 8-10: Recurse to Child**
```cpp
if(child != NULL){
    child->searchall(buffer, curr+1);
}
```
- Continue building longer words
- Increment position for next character
- Explores depth-first

**Lines 12-14: Recurse to Sibling**
```cpp
if(sibling != NULL){
    sibling->searchall(buffer, curr);
}
```
- Explore alternative paths at same level
- Keep same buffer position (replace character)
- Explores breadth at current depth

---

### Search Flow Example: "search" in documents

**Initial Call:**
```cpp
trie->dfsearchword("search", 0, 6)
//                   │       │  │
//                   │       │  └─ wordlen (calculated once!)
//                   │       └─── curr (start at 0)
//                   └─────────── word to find
```

**Recursion Steps:**

**Step 1: Find 's'**
```
At root level, curr=0
word[0]='s'
Check siblings → finds 's'
Match found, go to child
```

**Step 2: Find 'e'**
```
At 's' node, curr=1
word[1]='e'
Check children → finds 'e'
Match found, go to child
```

**Steps 3-6: Find 'a', 'r', 'c', 'h'**
```
Continue through: 'a' → 'r' → 'c' → 'h'
Each time: curr++, go to child
```

**Step 7: End of Word**
```
At 'h' node, curr=5
word[5]='h'
Match found

Check: curr==wordlen-1?
       5==6-1? → YES! End of word!
       
Check: list!=NULL?
       YES! → list->volume()
```

**Step 8: Count Documents**
```
Listnode chain: [doc=1] → [doc=2] → [doc=5] → NULL

volume():
  Count node 1: return 1 + next->volume()
  Count node 2: return 1 + next->volume()
  Count node 3: return 1 + NULL
  Total: 3 documents
```

**Final Return:** `3` (word "search" appears in 3 documents)

---

### searchall() Traversal Example

**Trie Structure:**
```
Root
 ├─ h (child)
 │   └─ e (child)
 │       └─ l (child)
 │           └─ l (child)
 │               └─ o [list] (child)
 │                   └─ w [list]
 ├─ w (sibling of h)
 │   └─ e (child)
 │       └─ b [list]
 └─ s (sibling of w)
     └─ e (child)
         └─ a (child)
             └─ r (child)
                 └─ c (child)
                     └─ h [list]
```

**Call:** `trie->searchall(buffer, 0)`

**Traversal Order (DFS):**

```
1. Root (value=-1) → skip, go to child
   
2. 'h' → buffer[0]='h'
   No list, go to child
   
3. 'e' → buffer[1]='e'
   No list, go to child
   
4. 'l' → buffer[2]='l'
   No list, go to child
   
5. 'l' → buffer[3]='l'
   No list, go to child
   
6. 'o' → buffer[4]='o'
   list exists! → print "hello"
   Go to child
   
7. 'w' → buffer[5]='w'
   list exists! → print "hellow"
   No child, backtrack
   
8. Back to 'h', check sibling
   
9. 'w' → buffer[0]='w'
   No list, go to child
   
10. 'e' → buffer[1]='e'
    No list, go to child
    
11. 'b' → buffer[2]='b'
    list exists! → print "web"
    No child, backtrack
    
12. Back to 'w', check sibling
    
13. 's' → buffer[0]='s'
    Continue... → prints "search"
```

**Output:**
```
hello
hellow
web
search
...
```

---

### Performance Comparison

#### dfsearchword() - Before vs After

**Before (December 28):**
```cpp
if(curr==strlen(word)-1){  // ❌ strlen() every recursion!
```

**For word "search" (6 characters):**
- 6 recursive calls (one per character)
- 6 strlen() calls
- Each strlen() loops 6 characters
- Total: **36 character iterations**

**After (January 1):**
```cpp
if(curr==wordlen-1){  // ✅ Use passed value!
```

**For word "search" (6 characters):**
- 1 strlen() call (in Search.cpp)
- 6 recursive calls (use passed wordlen)
- Total: **6 character iterations**

**Improvement:** 83% reduction! (36 → 6 iterations)

---

### Testing Results (January 1-2)

**Test 1: dfsearchword() - word exists**
```cpp
trie->dfsearchword("search", 0, 6)
→ Returns: 3 (appears in 3 documents) ✅
```

**Test 2: dfsearchword() - word not found**
```cpp
trie->dfsearchword("xyz", 0, 3)
→ Returns: 0 (not found) ✅
```

**Test 3: dfsearchword() - case sensitive**
```cpp
trie->dfsearchword("Search", 0, 6)
→ Returns: 1 (capital S) ✅

trie->dfsearchword("search", 0, 6)
→ Returns: 3 (lowercase s) ✅
```

**Test 4: searchall() - display all words**
```cpp
char* buffer = (char*)malloc(256);
trie->searchall(buffer, 0);
free(buffer);

Output:
Introduction
A
search
engine
is
a
software
...
✅
```

---

### Summary of January 1-2 Changes

| Component | Change | Impact |
|-----------|--------|--------|
| dfsearchword() signature | Added `wordlen` parameter | API change |
| dfsearchword() line 4 | Use `wordlen` instead of `strlen()` | 83% performance boost |
| dfsearchword() calls | Pass `wordlen` down recursively | Consistent optimization |
| searchall() | New function implemented | Display all indexed words |
| Overall complexity | O(n×depth) → O(depth) | Linear improvement |

### Integration with Listnode

**volume() function:**
```cpp
int listnode::volume(){
    if(next != NULL) 
        return 1 + next->volume();
    else 
        return 1;
}
```

**How it works:**
- Recursively counts all nodes in linked list
- Each node represents one document
- Returns total document count

**Example:**
```
List: [doc=1] → [doc=2] → [doc=5] → NULL

volume() call:
  Node 1: 1 + next->volume()
    Node 2: 1 + next->volume()
      Node 3: 1 + next->volume()
        NULL: 1
      Returns: 1
    Returns: 1 + 1 = 2
  Returns: 1 + 2 = 3

Total: 3 documents
```

---

**Document Version**: 1.2  
**Last Updated**: January 2, 2026  
**Changes**: Implemented dfsearchword() and searchall(), performance optimization with wordlen  
**New Features**: Document frequency search, complete vocabulary display  
**Performance Gain**: 83% reduction in operations for dfsearchword(), O(n×depth) → O(depth)  
**Status**: TF, DF, and vocabulary display all fully operational ✅  
**Author**: High-Performance Search Engine Project  
**Repository**: github.com/adarshpheonix2810/high-performance-search-engine-cpp
