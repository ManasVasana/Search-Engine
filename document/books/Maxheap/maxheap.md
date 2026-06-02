# Maxheap - Conceptual Documentation

This document explains **WHAT** a max heap is, **WHY** we use it, and **ALTERNATIVES**. For implementation details, see `working.md`.

---

## Table of Contents

1. [What is a Max Heap?](#1-what-is-a-max-heap)
2. [Why Use Max Heap for Search Results?](#2-why-use-max-heap-for-search-results)
3. [Heap vs Other Data Structures](#3-heap-vs-other-data-structures)
4. [malloc vs new](#4-malloc-vs-new)
5. [Time Complexity Analysis](#5-time-complexity-analysis)
6. [Design Decisions Q&A](#6-design-decisions-qa)

---

## 1. What is a Max Heap?

### Simple Definition

A **max heap** is a **complete binary tree** where every parent node has a value **greater than or equal to** its children.

### Visual Representation

```
        9 (root = maximum)
       / \
      7   8
     / \
    3   5

Heap property: parent >= children
9 >= 7, 9 >= 8
7 >= 3, 7 >= 5
8 has no children
```

### Array Storage

**Binary tree stored in array:**
```
Index:  0   1   2   3   4
Value: [9] [7] [8] [3] [5]

Relationships:
  Parent of index i:     (i-1)/2
  Left child of i:       2*i + 1
  Right child of i:      2*i + 2

Example:
  Node at index 1 (value=7):
    Parent: (1-1)/2 = 0 (value=9) ✅
    Left:   2*1+1 = 3 (value=3) ✅
    Right:  2*1+2 = 4 (value=5) ✅
```

### Heap Properties

1. **Shape property**: Complete binary tree (filled left-to-right)
2. **Heap property**: Parent >= children (max heap)
3. **Root property**: Maximum element always at root (index 0)

### Operations

**Insert:**
1. Add element at end
2. Bubble up until heap property restored
3. Time: O(log n)

**Remove (extract max):**
1. Save root (maximum)
2. Replace root with last element
3. Bubble down until heap property restored
4. Time: O(log n)

**Peek (get max):**
- Return root value
- Time: O(1)

---

## 2. Why Use Max Heap for Search Results?

### Problem: Top-K Document Ranking

**Scenario:**
```
Search query: "machine learning"
100 documents match

User wants: Top 5 documents (k=5)

Naive approach:
  1. Score all 100 documents
  2. Sort all 100 by score
  3. Take first 5
  
Time: O(n log n) where n=100
```

### Max Heap Approach

```
Heap approach:
  1. Create heap of size k=5
  2. For each document:
     - Calculate score
     - If heap not full, insert
     - If heap full and score > minimum:
       - Remove minimum
       - Insert new score
  3. Extract k elements from heap
  
Time: O(n log k) where n=100, k=5
```

### Why This Is Better

**Comparison:**

| Approach | Time | Space | Notes |
|----------|------|-------|-------|
| **Sort all** | O(n log n) | O(n) | 100 log 100 ≈ 664 ops |
| **Heap (k=5)** | O(n log k) | O(k) | 100 log 5 ≈ 232 ops |
| **Savings** | **2.86x faster!** | **20x less space** | k=5 vs n=100 |

**Real-world example:**
```
1,000,000 documents, want top 10:

Sort: 1M log 1M = 19,931,568 operations
Heap: 1M log 10 = 3,321,928 operations

Heap is 6x faster! ✅
```

### Additional Benefits

1. **Space efficient**: Only store k items, not all n
2. **Incremental**: Can stop processing early if needed
3. **Online**: Can add documents dynamically
4. **Simple**: Clear API (insert, remove, peek)

---

## 3. Heap vs Other Data Structures

### Q: Why Not Just Sort?

**Sorting approach:**
```cpp
struct Result {
    double score;
    int docId;
};
Result results[100];
// Fill results...
sort(results, results+100, [](Result a, Result b){
    return a.score > b.score;
});
// Take first k
```

**Disadvantages:**
❌ Must store ALL n documents
❌ O(n log n) time (slower for large n)
❌ Must process all documents before returning results
❌ Wastes memory on low-scoring documents

**Max heap approach:**
```cpp
Maxheap heap(k);
for each document:
    heap.insert(score, docId);  // Automatically keeps top-k
```

**Advantages:**
✅ Only store k documents
✅ O(n log k) time (faster when k << n)
✅ Results available immediately
✅ Minimal memory footprint

### Q: Why Not Use std::priority_queue?

**C++ STL priority_queue:**
```cpp
#include <queue>
std::priority_queue<pair<double, int>> pq;
pq.push({score, docId});
```

**Advantages:**
✅ Standard library (well-tested)
✅ Generic (works with any type)
✅ Exception-safe
✅ Modern C++ style

**Why we DON'T use it:**
❌ **Project constraint**: No STL allowed
❌ **Educational goal**: Learn by implementing
❌ **Simplicity**: Custom version has only features we need
❌ **Control**: Full control over memory and behavior

**In production code:** Use std::priority_queue!

### Q: Why Not Use Min Heap?

**Our max heap:**
```
Root = maximum score
Extract → Get highest score first
```

**Min heap alternative:**
```
Root = minimum score
Need to extract n-k times to get top-k
```

**Problem with min heap for top-k:**
```
Want top 5 from 100 documents

Min heap approach:
  1. Insert all 100
  2. Remove 95 smallest
  3. Remaining 5 are top-k
  
Operations: 100 inserts + 95 removes = 195 ops

Max heap approach:
  1. Maintain heap of size 5
  2. Only replace if new > minimum
  
Operations: ~100 comparisons, ~20 inserts = 120 ops
```

**Max heap is simpler for top-k!**

### Q: Why Not Use Array + Linear Search?

**Array approach:**
```cpp
double scores[5];
int ids[5];
int count = 0;

// Insert new score
if(count < 5){
    scores[count] = score;
    ids[count] = docId;
    count++;
}
else{
    // Find minimum
    int minIdx = 0;
    for(int i=1; i<5; i++){
        if(scores[i] < scores[minIdx])
            minIdx = i;
    }
    // Replace if new > min
    if(score > scores[minIdx]){
        scores[minIdx] = score;
        ids[minIdx] = docId;
    }
}
```

**Disadvantages:**
❌ O(k) to find minimum each time
❌ O(k) to extract in sorted order
❌ Total: O(n*k) insertion + O(k²) extraction

**Heap approach:**
❌ O(log k) to insert
❌ O(log k) to extract
❌ Total: O(n log k) + O(k log k)

**Example (n=100, k=5):**
```
Array: 100*5 + 5*5 = 525 operations
Heap:  100*2.3 + 5*2.3 = 241 operations

Heap is 2x faster! ✅
```

---

## 4. malloc vs new

### Q: Why Does Maxheap Use malloc Instead of new?

**Our implementation:**
```cpp
heap = (double*)malloc(maxnumofscores * sizeof(double));
ids = (int*)malloc(maxnumofscores * sizeof(int));
```

**Why malloc?**
1. **Raw arrays**: Just need contiguous memory blocks
2. **C-style arrays**: Plain Old Data (POD) types
3. **Performance**: Slightly faster (no constructor calls)
4. **Legacy code**: Original implementation style

**Why new would be better:**
```cpp
heap = new double[maxnumofscores];
ids = new int[maxnumofscores];
```

**Advantages of new:**
✅ Type-safe (no casting)
✅ Automatically initializes (for classes)
✅ Modern C++ style
✅ Pairs with delete[] (clearer intent)

### Detailed Comparison

| Feature | malloc/free | new/delete |
|---------|-------------|------------|
| **Language** | C | C++ |
| **Type safety** | Manual cast | Automatic |
| **Constructors** | No | Yes |
| **Destructors** | No | Yes |
| **Syntax** | malloc(size) | new Type[n] |
| **Cleanup** | free(ptr) | delete[] ptr |
| **Failure** | Returns NULL | Throws exception |
| **Usage** | POD types | All types |

### When to Use Each

**Use malloc/free:**
- Plain Old Data (int, double, char)
- C library integration
- Performance-critical raw buffers
- Legacy C code

**Use new/delete:**
- C++ objects (with constructors/destructors)
- Modern C++ code
- Exception handling needed
- Better type safety wanted

**Our case:**
- Storing doubles and ints (POD)
- malloc acceptable ✅
- new would be more idiomatic ✅✅

### Modern C++ Alternative

**Best practice (C++11+):**
```cpp
#include <vector>
std::vector<double> heap;
std::vector<int> ids;

heap.resize(maxnumofscores);
ids.resize(maxnumofscores);

// Automatic cleanup, exception-safe, modern!
```

**Why we don't use it:**
❌ No STL constraint
✅ Learning exercise

---

## 5. Time Complexity Analysis

### Operations Overview

| Operation | Best Case | Average Case | Worst Case | Space |
|-----------|-----------|--------------|------------|-------|
| **Insert** | O(1) | O(log k) | O(log k) | O(k) |
| **Remove** | O(1) | O(log k) | O(log k) | O(k) |
| **Peek** | O(1) | O(1) | O(1) | O(k) |
| **Build heap** | - | O(k) | O(k) | O(k) |

### Insert Analysis

**Best case: O(1)**
```
Insert 1 into heap [5, 3, 2]
New element is smallest → Stays at bottom
No swaps needed
```

**Average case: O(log k)**
```
Heap height: h = log₂(k)
Average swaps: h/2 ≈ log(k)/2
Example k=8: Height=3, avg swaps≈1.5
```

**Worst case: O(log k)**
```
Insert 10 into heap [5, 3, 2]
New element is largest → Bubbles to root
Swaps: log₂(k)
Example k=8: 3 swaps maximum
```

### Remove Analysis

**Always O(log k)**
```
1. Copy root → O(1)
2. Replace root with last → O(1)
3. Bubble down → O(log k)
Total: O(log k)
```

**Bubble down swaps:**
```
Heap height: log₂(k)
Worst case: Element sinks to bottom
Swaps: log₂(k)
```

### Overall Top-K Algorithm

**Given n documents, find top k:**

```cpp
Maxheap heap(k);
for(int i=0; i<n; i++){         // n iterations
    heap.insert(score, id);      // O(log k)
}
for(int i=0; i<k; i++){         // k iterations
    int id = heap.get_id();
    heap.remove();               // O(log k)
}
```

**Time complexity:**
```
Insertion phase: n × O(log k) = O(n log k)
Extraction phase: k × O(log k) = O(k log k)
Total: O(n log k) + O(k log k) = O(n log k)
  (since k << n typically)
```

**Space complexity:**
```
Heap storage: O(k)
Two arrays (scores + ids): 2k × size
Example k=5: ~80 bytes total
```

### Comparison with Alternatives

**For n=1000 documents, k=10 results:**

| Method | Time Complexity | Operations | Space |
|--------|----------------|------------|-------|
| **Full sort** | O(n log n) | 9,965 | O(n) |
| **Partial sort** | O(n log k) | 3,321 | O(n) |
| **Max heap** | O(n log k) | 3,321 | O(k) |
| **Linear scan** | O(nk) | 10,000 | O(k) |

**Winner: Max heap** (fast + space-efficient)

---

## 6. Design Decisions Q&A

### Q: Why Fixed Size Heap (k) Instead of Growing?

**Our design:**
```cpp
Maxheap(int k)  // Fixed capacity
```

**Alternative: Growing heap**
```cpp
Maxheap()  // Unlimited capacity, grows as needed
```

**Why fixed size:**
1. **Top-k problem**: Only need k results
2. **Memory bounded**: Know exact memory usage
3. **Simpler code**: No resizing logic
4. **Performance**: No reallocation overhead
5. **Clear intent**: "Give me top 5" → heap of 5

**Trade-off:** Less flexible, but perfect for our use case.

### Q: Why Store Both Scores and IDs?

**Our approach:**
```cpp
double* heap;   // Scores
int* ids;       // Document IDs
```

**Alternative: Store pairs**
```cpp
struct ScorePair {
    double score;
    int id;
};
ScorePair* heap;
```

**Our choice:**
✅ Separate arrays → Better cache locality when comparing scores
✅ Simpler swap logic (swap each independently)
✅ Clearer intent (scores drive ordering, IDs are payload)

**Alternative choice:**
✅ Single allocation
✅ Data bundled together
✅ More object-oriented

**Both valid!** We chose separate for simplicity.

### Q: Why Recursive Destructor for Scorelist but Not Heap?

**Scorelist (linked list):**
```cpp
~Scorelist(){
    if(next!=NULL) delete(next);  // Recursive
}
```

**Maxheap (arrays):**
```cpp
~Maxheap(){
    free(heap);  // Direct
    free(ids);
}
```

**Reason:**
- Linked list: Chain of objects → Must delete each
- Arrays: Single contiguous block → Single free

**Different data structures → Different cleanup strategies**

### Q: Why Not Heapify (Build Heap Efficiently)?

**Heapify algorithm:**
```cpp
// Build heap from array in O(n) time
for(int i=n/2-1; i>=0; i--){
    bubble_down(i);
}
```

**Why we don't use it:**
1. We insert incrementally (one score at a time)
2. Don't have all scores upfront
3. O(n log k) insertion fine for our use case
4. Simpler to understand and maintain

**When heapify is useful:**
- Have array of elements to turn into heap
- Batch insertion (all at once)
- O(n) vs O(n log n) matters

**Our use case: Incremental insertion → No heapify needed**

### Q: What Happens on Tie Scores?

**Scenario:**
```
Document 3: score = 5.0
Document 7: score = 5.0
Both have same score!
```

**Our behavior:**
- Heap compares scores only
- Ties broken by insertion order
- First inserted stays higher in heap

**Alternative approaches:**
1. **Secondary sort by docId**
   ```cpp
   if(score1 == score2)
       return id1 < id2;
   ```

2. **Stable heap**
   - Track insertion time
   - Maintain FIFO for ties

**Our choice:** Simplest (no tie-breaking)
**Rationale:** Rare in practice (BM25 scores rarely identical)

---

## Summary

### Max Heap Characteristics

**Structure:**
- Complete binary tree (array representation)
- Parent >= children (max heap property)
- Root = maximum element

**Time Complexity:**
- Insert: O(log k)
- Remove: O(log k)
- Peek: O(1)
- Top-k from n: O(n log k)

**Space Complexity:**
- O(k) - only store top k results

**Advantages:**
✅ Fast top-k retrieval
✅ Memory efficient
✅ Simple API
✅ Industry standard for ranking

**Design Choices:**
- Fixed size k (bounded memory)
- malloc for raw arrays (C-style)
- Separate score/ID arrays (cache-friendly)
- No heapify (incremental insertion)

**Use Cases:**
- Search result ranking
- Top-N queries
- Priority queues
- Event scheduling
- Graph algorithms (Dijkstra)

**Alternatives:**
- std::priority_queue (if STL allowed)
- Sorting (if k ≈ n)
- Linear scan (if k very small)

---

**Document Version**: 1.0  
**Created**: January 2, 2026  
**Purpose**: Explain conceptual design of max heap for search ranking  
**Bugs Fixed**: Uninitialized memory (Jan 2), added get_count() method  
**Author**: High-Performance Search Engine Project
---

## 8. January 3, 2026 - Additional Concepts

### Q: What is initialization in variable declaration?

**Answer**: Initialization means giving a variable its first value when it's created.

**Uninitialized variable** (dangerous):
```cpp
int x;           // Uninitialized - contains garbage value
cout << x;       // Undefined behavior - could be any number!
```

**Initialized variable** (safe):
```cpp
int x = 0;       // Initialized to 0
int y = 42;      // Initialized to 42
cout << x;       // Always prints 0 ✅
```

**Our bug (Fixed Jan 3)**: In `minindex()`, we had wrong initialization:
```cpp
// ❌ BAD - Initialized to wrong value
int min = 1;     // Should be 'low', not hardcoded 1

// ✅ GOOD - Initialized correctly
int min = low;   // Starts at beginning of search range
```

**Why initialization matters**:
- **Prevents garbage values**: Variables start with known value
- **Correctness**: Algorithm works as intended
- **Debugging**: Easier to find bugs with predictable starting values

---

### Q: What is a magic number and why avoid it?

**Answer**: A magic number is a hardcoded value in code without explanation.

**Example with magic number** (bad):
```cpp
double minscore = 1000000.0;  // ❌ What is 1000000? Why this value?
```

**Problems**:
1. No clear meaning - why 1 million?
2. Might be wrong if actual values exceed it
3. Hard to maintain - what if range changes?

**Better approaches**:

1. **Use actual value from data**:
```cpp
double minscore = heap[low];  // ✅ Use actual minimum starting point
```

2. **Use named constant**:
```cpp
const double MAX_SCORE = 1000000.0;  // ✅ Named and explained
double minscore = MAX_SCORE;
```

3. **Use system constant**:
```cpp
#include <limits>
double minscore = std::numeric_limits<double>::max();  // ✅ Proper max value
```

**Our fix (Jan 3)**:
```cpp
// BEFORE - Magic number
double minscore = 1000000.0;  // ❌ Arbitrary, could be wrong

// AFTER - Actual value
double minscore = heap[low];  // ✅ Real value from heap
```

**Benefits**:
- No arbitrary assumptions
- Works correctly for all possible score ranges
- Self-documenting code

---

### Q: What is a loop invariant?

**Answer**: A loop invariant is a condition that remains true before and after each loop iteration.

**Example**:
```cpp
int min = low;              // Invariant starts: min is in range [low, high)
double minscore = heap[low];

for(int i = low; i < high; i++){
    // Loop invariant: min holds index of smallest element seen so far
    if(heap[i] < minscore){
        min = i;
        minscore = heap[i];
    }
    // Invariant still true after iteration
}
// After loop: min holds index of smallest element in entire range
```

**Why it matters**:
- **Correctness**: Proves algorithm works
- **Debugging**: If invariant breaks, you know where bug is
- **Understanding**: Explains what loop accomplishes

**Our invariant (minindex function)**:
- **Before loop**: `min = low`, so min is valid index in [low, high)
- **During loop**: min always holds index of smallest element found so far
- **After loop**: min holds index of smallest element in entire range

**Bug (Fixed Jan 3)**: Starting with `min = 1` violated the invariant because 1 might not be in range [low, high).

---

### Q: What does "index out of range" mean?

**Answer**: Trying to access an array element that doesn't exist.

**Example**:
```cpp
int arr[5] = {10, 20, 30, 40, 50};
// Valid indices: 0, 1, 2, 3, 4

int x = arr[2];   // ✅ Valid - index 2 exists
int y = arr[10];  // ❌ Out of range - index 10 doesn't exist!
                  // Could crash or return garbage
```

**Common causes**:
1. Off-by-one errors
2. Using wrong variable as index
3. Not checking bounds before access

**Our bug (Fixed Jan 3)**:

In `minindex(low, high)`, if we return index outside [low, high):
```cpp
// Buggy: min starts at 1
int min = 1;
// If called with minindex(3, 5), searching range [3, 4]
// But min=1 is outside this range!
// Returns 1 when it should return 3 or 4
```

**Fix**: Start min at low, so it's always in valid range.

**Prevention**:
```cpp
// Always validate before accessing array
if(index >= 0 && index < arraySize){
    value = array[index];  // ✅ Safe
}
```

---

### Q: What is encapsulation in OOP?

**Answer**: Encapsulation means hiding internal data and exposing only what's needed through methods.

**Bad encapsulation** (direct access):
```cpp
class Heap{
public:
    double* heap;  // ❌ Public - anyone can access directly!
};

Heap h;
double score = h.heap[0];  // ❌ Direct access breaks encapsulation
h.heap[0] = 999;           // ❌ Can corrupt internal state!
```

**Good encapsulation** (methods):
```cpp
class Heap{
private:
    double* heap;  // ✅ Private - hidden from outside
    
public:
    double get_score(){  // ✅ Controlled access through method
        return heap[0];
    }
};

Heap h;
double score = h.get_score();  // ✅ Can only read, not modify
```

**Benefits**:
1. **Safety**: Can't accidentally corrupt internal data
2. **Validation**: Method can check validity before returning
3. **Flexibility**: Can change internal implementation without breaking code

**Our addition (Jan 3)**:
```cpp
// Added to Maxheap class
double get_score(){
    return heap[0];  // Safe way to get top score
}
```

Instead of letting Search.cpp directly access `heap[0]`, we provide a method.

---

### Q: What is bounds checking and why is it important?

**Answer**: Bounds checking validates that an index is within valid array range before accessing.

**Without bounds checking** (dangerous):
```cpp
while(heap[index] > heap[(index-1)/2]){  // ❌ What if index=0?
    swapscore(index, (index-1)/2);       // Access heap[-1]! Crash!
    index = (index-1)/2;
}
```

**With bounds checking** (safe):
```cpp
while(index > 0 && heap[index] > heap[(index-1)/2]){  // ✅ Check index > 0 first
    swapscore(index, (index-1)/2);
    index = (index-1)/2;
}
```

**Why necessary**:
1. **Prevent crashes**: No segmentation faults
2. **Data integrity**: Don't read/write garbage memory
3. **Correctness**: Algorithm behaves as intended
4. **Security**: Prevent buffer overflow attacks

**Types of bounds checks**:

1. **Lower bound check**:
```cpp
if(index >= 0){  // Not negative
    access array
}
```

2. **Upper bound check**:
```cpp
if(index < size){  // Not past end
    access array
}
```

3. **Combined check**:
```cpp
if(index >= 0 && index < size){  // Within valid range
    access array
}
```

**Our fix (Jan 3)**: Added `index > 0` check before accessing parent node to prevent accessing invalid negative index.

---

### Q: What is the difference between runtime check and compile-time check?

**Answer**: Runtime checks happen during program execution, compile-time checks happen during compilation.

**Compile-time check** (compiler catches):
```cpp
int arr[5];
int x = arr[10];  // ⚠️ Some compilers warn (but not error)

const int SIZE = 5;
arr[SIZE] = 10;   // ⚠️ Might warn about out of bounds
```

**Runtime check** (you must add explicitly):
```cpp
int arr[5];
int index = getUserInput();  // Could be anything!

// ✅ Runtime check
if(index >= 0 && index < 5){
    arr[index] = 10;  // Safe
} else {
    cout << "Index out of range!" << endl;
}
```

**Our case (Jan 3)**: Added runtime checks:
```cpp
// Runtime check - validated during execution
if(docId == -1 || docId >= map->get_size()){
    continue;  // Skip invalid document
}
```

**When to use each**:
- **Compile-time**: Type checking, const correctness, template errors
- **Runtime**: User input validation, dynamic conditions, array bounds

**Best**: Catch as much as possible at compile-time (faster, guaranteed), but add runtime checks for dynamic data.

---

### Q: What is defensive programming?

**Answer**: Defensive programming means writing code that handles unexpected situations gracefully.

**Non-defensive** (assumes everything is perfect):
```cpp
void process(int* arr, int size){
    for(int i=0; i<size; i++){
        arr[i] = i;  // ❌ What if arr is NULL? Crash!
    }
}
```

**Defensive** (handles edge cases):
```cpp
void process(int* arr, int size){
    if(arr == NULL){  // ✅ Check for NULL
        return;
    }
    if(size < 0){     // ✅ Validate size
        return;
    }
    for(int i=0; i<size; i++){
        arr[i] = i;   // Now safe
    }
}
```

**Defensive practices**:

1. **Null checks**:
```cpp
if(pointer != NULL){
    use pointer
}
```

2. **Range validation**:
```cpp
if(index >= 0 && index < size){
    access array
}
```

3. **Input validation**:
```cpp
if(input is valid){
    process input
} else {
    error message
}
```

4. **Graceful degradation**:
```cpp
if(malloc failed){
    use fallback method
} else {
    use optimal method
}
```

**Our defensive programming (Jan 3)**:

1. Null check for malloc:
```cpp
if(line == NULL){
    // Fallback - print without title
}
```

2. Bounds check for docId:
```cpp
if(docId >= map->get_size()){
    continue;  // Skip invalid
}
```

3. Heap count check:
```cpp
if(heap->get_count() == 0){
    break;  // No more results
}
```

**Benefits**:
- Program doesn't crash on unexpected input
- Clear error messages help debugging
- More robust and reliable code

---

**Document Version**: 1.1  
**Last Updated**: January 3, 2026  
**New Concepts**: Variable initialization, magic numbers, loop invariants, index out of range, encapsulation, bounds checking, runtime vs compile-time checks, defensive programming  
**Purpose**: Explain concepts related to January 3 bug fixes  
**Author**: High-Performance Search Engine Project