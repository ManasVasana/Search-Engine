# Map Class - C++ Concepts Documentation

This document explains the **C++ concepts and features** used in the `Mymap` class. For detailed code explanation, see `working.md`.

---

## Table of Contents

1. [Header Guards (#ifndef, #define, #endif)](#1-header-guards-ifndef-define-endif)
2. [new vs malloc](#2-new-vs-malloc)
3. [delete vs free](#3-delete-vs-free)
4. [Pointers to Pointers (char**)](#4-pointers-to-pointers-char)
5. [nullptr](#5-nullptr)
6. [strcpy() Function](#6-strcpy-function)
7. [strlen() Function](#7-strlen-function)
8. [Member Initializer List](#8-member-initializer-list)
9. [const Keyword](#9-const-keyword)
10. [Scope Resolution Operator (::)](#10-scope-resolution-operator)
11. [const Correctness in Methods](#11-const-correctness-in-methods)
12. [Why doc_lengths Stores Word Count](#12-why-doc_lengths-stores-word-count)

---

## 1. Header Guards (#ifndef, #define, #endif)

### What Are Header Guards?

Header guards are preprocessor directives that **prevent multiple inclusion** of the same header file in a single compilation unit.

### Syntax

```cpp
#ifndef HEADER_NAME_HPP
#define HEADER_NAME_HPP

// Header content here

#endif
```

### Why We Need Them?

**Problem without header guards:**

```cpp
// File1.hpp
class MyClass {
    int x;
};

// File2.hpp
#include "File1.hpp"
class AnotherClass {
    MyClass obj;
};

// main.cpp
#include "File1.hpp"  // MyClass defined
#include "File2.hpp"  // File1.hpp included again! 
                      // MyClass defined AGAIN! 
                      // ERROR: Redefinition!
```

**Solution with header guards:**

```cpp
// File1.hpp
#ifndef FILE1_HPP    // If not defined
#define FILE1_HPP    // Define it

class MyClass {
    int x;
};

#endif  // End if

// Now multiple includes are safe:
#include "File1.hpp"  // First time: content included
#include "File1.hpp"  // Second time: skipped (already defined)
```

---

### How It Works

**Visual Execution:**

```
First inclusion:
1. Compiler sees #ifndef FILE1_HPP
2. FILE1_HPP is NOT defined â†’ condition TRUE
3. Execute #define FILE1_HPP â†’ Now it's defined
4. Include all content until #endif
5. Compiler continues

Second inclusion:
1. Compiler sees #ifndef FILE1_HPP
2. FILE1_HPP IS defined â†’ condition FALSE
3. Skip everything until #endif
4. Nothing included (prevented duplicate)
```

---

### In Our Map.hpp

```cpp
#ifndef MAP_HPP
#define MAP_HPP

class Mymap {
    // Class definition
};

#endif
```

**Naming Convention:**
- Use uppercase
- Match filename: `Map.hpp` â†’ `MAP_HPP`
- Add `_HPP` or `_H` suffix
- Avoid name conflicts

---

### Alternative: #pragma once

**Modern alternative:**

```cpp
#pragma once

class Mymap {
    // Class definition
};
```

**Comparison:**

| Feature | #ifndef guards | #pragma once |
|---------|---------------|--------------|
| Standard | âœ… C++ standard | âš ï¸ Non-standard (widely supported) |
| Portability | âœ… Works everywhere | âš ï¸ Most compilers support |
| Syntax | Verbose (3 lines) | Concise (1 line) |
| Speed | Slightly slower | Slightly faster |
| Recommended | âœ… For portable code | âœ… For modern projects |

---

### Common Mistakes

**âŒ Forgetting #endif:**
```cpp
#ifndef MAP_HPP
#define MAP_HPP

class Mymap { };
// Missing #endif â†’ Compilation error!
```

**âŒ Wrong naming:**
```cpp
// Map.hpp
#ifndef FILE_HPP  // âŒ Doesn't match filename
#define FILE_HPP

class Mymap { };

#endif
```

**âŒ Using same name in different files:**
```cpp
// File1.hpp
#ifndef HEADER_HPP  // âŒ Generic name
#define HEADER_HPP
// ...

// File2.hpp
#ifndef HEADER_HPP  // âŒ Same name! Conflict!
#define HEADER_HPP
// ...
```

---

## 2. new vs malloc

### What is new?

`new` is a **C++ operator** for dynamic memory allocation that:
- Allocates memory on the heap
- Calls constructor
- Returns typed pointer
- Type-safe

### What is malloc?

`malloc()` is a **C function** for dynamic memory allocation that:
- Allocates memory on the heap
- Does NOT call constructor
- Returns `void*` (must cast)
- Not type-safe

---

### Syntax Comparison

```cpp
// Using new (C++)
int* ptr = new int;           // Single integer
int* arr = new int[10];       // Array of 10 integers
MyClass* obj = new MyClass(); // Object with constructor

// Using malloc (C)
int* ptr = (int*)malloc(sizeof(int));              // Single integer
int* arr = (int*)malloc(10 * sizeof(int));         // Array
MyClass* obj = (MyClass*)malloc(sizeof(MyClass));  // Object (NO constructor!)
```

---

### Key Differences

| Feature | `new` | `malloc()` |
|---------|-------|------------|
| **Language** | C++ | C |
| **Type** | Operator | Function |
| **Return type** | Typed pointer | `void*` (requires cast) |
| **Constructor** | âœ… Calls constructor | âŒ No constructor |
| **Type safety** | âœ… Yes | âŒ No |
| **Size calculation** | âœ… Automatic | âš ï¸ Manual (`sizeof`) |
| **On failure** | Throws exception | Returns `NULL` |
| **Deallocation** | `delete` / `delete[]` | `free()` |

---

### Examples

**Allocating single integer:**

```cpp
// C++ way
int* num = new int(42);
delete num;

// C way
int* num = (int*)malloc(sizeof(int));
*num = 42;
free(num);
```

**Allocating array:**

```cpp
// C++ way
int* arr = new int[100];
delete[] arr;  // Note: delete[] for arrays

// C way
int* arr = (int*)malloc(100 * sizeof(int));
free(arr);
```

**Allocating object:**

```cpp
class Person {
public:
    Person() { cout << "Constructor called!" << endl; }
};

// C++ way
Person* p = new Person();  // Output: "Constructor called!"
delete p;

// C way
Person* p = (Person*)malloc(sizeof(Person));  // No output! Constructor NOT called!
free(p);
```

---

### Why Use new in C++?

**Type Safety:**
```cpp
int* ptr = new int;  // âœ… Type clear

int* ptr = malloc(sizeof(int));  // âŒ Error! Type mismatch
int* ptr = (int*)malloc(sizeof(int));  // âš ï¸ Need cast
```

**Automatic Size:**
```cpp
MyClass* obj = new MyClass;  // âœ… Compiler knows size

MyClass* obj = (MyClass*)malloc(sizeof(MyClass));  // âš ï¸ Manual size
```

**Constructor Called:**
```cpp
class Database {
    Database() { connect(); }  // Important initialization
};

Database* db = new Database();  // âœ… connect() is called
Database* db = (Database*)malloc(sizeof(Database));  // âŒ connect() NOT called!
```

---

### When to Use Each?

**Use `new`:**
- âœ… Writing C++ code
- âœ… Need constructors/destructors
- âœ… Want type safety
- âœ… Working with classes/objects

**Use `malloc`:**
- âš ï¸ Interfacing with C libraries
- âš ï¸ Need `realloc()` (C++ has no equivalent)
- âš ï¸ Working in pure C

**Recommendation:** In C++, **always use `new`/`delete`**

---

## 3. delete vs free

### What is delete?

`delete` is a **C++ operator** that:
- Calls destructor
- Deallocates memory
- Type-aware

### What is free?

`free()` is a **C function** that:
- Does NOT call destructor
- Deallocates memory only
- Type-unaware

---

### Syntax

```cpp
// Single object
MyClass* obj = new MyClass();
delete obj;

// Array
int* arr = new int[100];
delete[] arr;  // Note: delete[] for arrays!

// C style
int* ptr = (int*)malloc(sizeof(int));
free(ptr);
```

---

### Key Rule: Match Pairs!

| Allocate with | Deallocate with |
|---------------|-----------------|
| `new` | `delete` |
| `new[]` | `delete[]` |
| `malloc()` | `free()` |

**âŒ NEVER MIX THEM:**

```cpp
// âŒ WRONG: Mixing new with free
int* ptr = new int;
free(ptr);  // ðŸ’¥ Undefined behavior!

// âŒ WRONG: Mixing malloc with delete
int* ptr = (int*)malloc(sizeof(int));
delete ptr;  // ðŸ’¥ Undefined behavior!

// âŒ WRONG: delete instead of delete[]
int* arr = new int[100];
delete arr;  // ðŸ’¥ Memory leak! Only first element freed

// âœ… CORRECT: Matching pairs
int* ptr = new int;
delete ptr;

int* arr = new int[100];
delete[] arr;

int* ptr = (int*)malloc(sizeof(int));
free(ptr);
```

---

### Why Matching Matters?

**Example with destructor:**

```cpp
class FileHandler {
    FILE* file;
public:
    FileHandler() { file = fopen("data.txt", "r"); }
    ~FileHandler() { fclose(file); }  // Important cleanup!
};

FileHandler* fh = new FileHandler();

delete fh;  // âœ… Destructor called, file closed
free(fh);   // âŒ Destructor NOT called, file still open! LEAK!
```

---

### delete vs delete[]

**delete** - For single objects:
```cpp
int* num = new int;
delete num;  // Correct
```

**delete[]** - For arrays:
```cpp
int* arr = new int[100];
delete[] arr;  // Correct - frees entire array
```

**What happens if you mix them?**

```cpp
int* arr = new int[100];
delete arr;  // âŒ Only first element freed! 99 elements leaked!
```

---

## 4. Pointers to Pointers (char**)

### Understanding Pointer Levels

**Level 0: Regular Variable**
```cpp
int num = 42;
```

**Level 1: Pointer to Variable**
```cpp
int* ptr = &num;  // Points to integer
```

**Level 2: Pointer to Pointer**
```cpp
int** ptrptr = &ptr;  // Points to pointer
```

---

### Why char**?

`char**` is an **array of strings**!

**Visual:**

```
char* (single string):
str â†’ "Hello"

char** (array of strings):
docs â†’ [ptr0] â†’ "Hello"
       [ptr1] â†’ "World"
       [ptr2] â†’ "C++"
```

---

### Syntax Breakdown

```cpp
char** documents;
```

**Reading right-to-left:**
- `documents` = variable name
- `*` = pointer to
- `char*` = pointer to char (string)
- `char**` = pointer to pointer to char (array of strings)

---

### Memory Layout

```cpp
char** docs = new char*[3];
docs[0] = new char[10];
docs[1] = new char[20];
docs[2] = new char[15];

strcpy(docs[0], "First");
strcpy(docs[1], "Second");
strcpy(docs[2], "Third");
```

**Memory:**
```
Stack:
  docs (char**) = [address to heap]

Heap:
  [ptr0][ptr1][ptr2]  â† Array of 3 pointers
    â†“     â†“     â†“
  "First" "Second" "Third"  â† Actual strings
```

---

### Access Methods

```cpp
char** docs;

docs[0]     // Pointer to first string
docs[0][0]  // First character of first string ('F')
*docs       // Same as docs[0]
**docs      // Same as docs[0][0]
```

---

### Why Not char[][]?

**2D Array (Fixed):**
```cpp
char docs[100][500];  // 100 strings, 500 chars each
// Memory: 50,000 bytes (always!)
```

**Array of Pointers (Variable):**
```cpp
char** docs = new char*[100];
docs[0] = new char[10];   // Short string
docs[1] = new char[200];  // Long string
// Memory: Only what you need!
```

---

## 5. nullptr

### What is nullptr?

`nullptr` is a **C++11 keyword** representing a null pointer constant. It's the modern way to represent "no object" or "points to nothing".

### Old Way vs New Way

```cpp
// Old C-style (before C++11)
char* ptr = NULL;     // NULL is actually 0
int* num = 0;         // Using 0 directly

// Modern C++ (C++11 and later)
char* ptr = nullptr;  // Type-safe null pointer
int* num = nullptr;
```

---

### Why nullptr is Better?

**Problem with NULL:**

```cpp
void func(int x) { cout << "int" << endl; }
void func(char* p) { cout << "pointer" << endl; }

func(NULL);    // âš ï¸ Calls func(int)! NULL is 0!
func(nullptr); // âœ… Calls func(char*)! Clear intent!
```

**Type Safety:**

```cpp
int x = NULL;     // âš ï¸ Works! NULL is integer 0
int y = nullptr;  // âŒ Error! nullptr is pointer-only
```

---

### Usage

```cpp
// Initialization
char* str = nullptr;

// Checking if pointer is null
if(str == nullptr) {
    cout << "Pointer is null" << endl;
}

// Safe to delete
delete[] str;  // Safe! delete nullptr does nothing
```

---

### nullptr vs NULL vs 0

| Value | Type | C++ Version | Recommended |
|-------|------|-------------|-------------|
| `0` | `int` | All | âŒ Ambiguous |
| `NULL` | Macro (`0`) | All | âš ï¸ C-style |
| `nullptr` | `std::nullptr_t` | C++11+ | âœ… Modern C++ |

---

## 6. strcpy() Function

### What is strcpy()?

`strcpy()` copies a string from source to destination, including the null terminator.

### Syntax

```cpp
char* strcpy(char* dest, const char* src);
```

### Parameters

- **dest**: Destination buffer (must be large enough!)
- **src**: Source string to copy
- **Return**: Pointer to dest

---

### How It Works

```cpp
char dest[20];
strcpy(dest, "Hello");
```

**Visual:**

```
Before:
src  â†’ ['H']['e']['l']['l']['o']['\0']
dest â†’ [?][?][?][?][?][?]...

After:
dest â†’ ['H']['e']['l']['l']['o']['\0']...
```

**Process:**
1. Copies 'H' to dest[0]
2. Copies 'e' to dest[1]
3. Copies 'l' to dest[2]
4. Copies 'l' to dest[3]
5. Copies 'o' to dest[4]
6. Copies '\0' to dest[5]
7. Stops

---

### Important Rules

**1. Destination must be large enough:**

```cpp
char dest[5];
strcpy(dest, "Hello World");  // âŒ Buffer overflow! ðŸ’¥
```

**2. Source must be null-terminated:**

```cpp
char src[5] = {'H','e','l','l','o'};  // No '\0'!
strcpy(dest, src);  // âŒ Undefined behavior!
```

**3. Strings can overlap in special functions:**

```cpp
// strcpy - no overlap allowed
strcpy(str, str+1);  // âŒ Undefined!

// memmove - overlap okay
memmove(str, str+1, strlen(str));  // âœ… OK
```

---

### Safe Alternative: strncpy

```cpp
char dest[10];
strncpy(dest, "Hello World", sizeof(dest)-1);
dest[sizeof(dest)-1] = '\0';  // Ensure null termination
```

---

## 7. strlen() Function

### What is strlen()?

`strlen()` returns the **length** of a C-style string (number of characters before null terminator).

### Syntax

```cpp
size_t strlen(const char* str);
```

### Return Value

Number of characters (NOT including '\0')

---

### Examples

```cpp
strlen("Hello")     // Returns 5
strlen("")          // Returns 0
strlen("Hi\0There") // Returns 2 (stops at first \0)
```

**Visual:**

```
String: "Hello"
Memory: ['H']['e']['l']['l']['o']['\0']
         0    1    2    3    4    5

strlen("Hello") = 5 (does NOT count \0)
```

---

### Important Points

**1. Does NOT include null terminator:**

```cpp
char* str = "Hello";
int len = strlen(str);  // len = 5

// To allocate: need len + 1
char* copy = new char[len + 1];  // +1 for '\0'
```

**2. Stops at first '\0':**

```cpp
char str[] = {'H','i','\0','!'','\0'};
strlen(str);  // Returns 2 (stops at first \0)
```

**3. Must be null-terminated:**

```cpp
char arr[5] = {'H','e','l','l','o'};  // No '\0'!
strlen(arr);  // âŒ Undefined! Keeps reading past array!
```

---

### vs sizeof

```cpp
char str[] = "Hello";

strlen(str);  // 5 (characters in string)
sizeof(str);  // 6 (bytes in array, including \0)
```

---

## 8. Member Initializer List

### What Is It?

A way to **initialize member variables before the constructor body executes**.

### Syntax

```cpp
ClassName::ClassName(parameters) : member1(value1), member2(value2) {
    // Constructor body
}
```

---

### Example

```cpp
class Person {
    string name;
    int age;
public:
    // With initializer list
    Person(string n, int a) : name(n), age(a) {
        // Variables already initialized
    }
    
    // Without initializer list
    Person(string n, int a) {
        name = n;  // Assignment, not initialization
        age = a;
    }
};
```

---

### Why Use It?

**1. More Efficient**

```cpp
// With initializer list (efficient)
Person(string n) : name(n) {
    // name constructed once with value "n"
}

// Without (less efficient)
Person(string n) {
    name = n;
    // name constructed with empty value
    // then assigned "n" (two operations!)
}
```

**2. Required for const members**

```cpp
class Box {
    const int id;  // const must be initialized
public:
    // âœ… Must use initializer list
    Box(int i) : id(i) { }
    
    // âŒ Error! Can't assign const
    Box(int i) {
        id = i;  // Error!
    }
};
```

**3. Required for references**

```cpp
class Wrapper {
    int& ref;
public:
    // âœ… Must use initializer list
    Wrapper(int& r) : ref(r) { }
    
    // âŒ Error! Reference must be initialized
    Wrapper(int& r) {
        ref = r;  // Error!
    }
};
```

---

### Order of Initialization

**Members are initialized in DECLARATION order, not list order:**

```cpp
class Test {
    int a;
    int b;
public:
    // List order: b, then a
    Test() : b(5), a(b) {  // âš ï¸ Problem!
        // But initialization happens: a first, then b
        // a tries to use uninitialized b!
    }
};
```

---

## 9. const Keyword

### What Does const Mean?

`const` = "constant" = **cannot be modified after initialization**

### Different Uses

**1. Const variable:**
```cpp
const int max = 100;
max = 200;  // âŒ Error!
```

**2. Const pointer to data:**
```cpp
const char* str = "Hello";
str[0] = 'h';  // âŒ Error! Can't modify data
str = "World"; // âœ… OK! Can change pointer
```

**3. Const pointer:**
```cpp
char* const str = "Hello";
str[0] = 'h';  // âœ… OK! Can modify data
str = "World"; // âŒ Error! Can't change pointer
```

**4. Const method:**
```cpp
class MyClass {
    int x;
public:
    int getX() const {
        return x;     // âœ… OK! Reading
        x = 10;       // âŒ Error! Can't modify
    }
};
```

---

### const Methods

```cpp
int getSize() const { return size; }
```

**Guarantees:**
- Won't modify object
- Can be called on const objects
- Compiler enforces

**Example:**

```cpp
void process(const Mymap& map) {
    int s = map.get_size();  // âœ… OK! get_size() is const
    map.insert("text", 0);   // âŒ Error! insert() not const
}
```

---

### const Return Values

```cpp
const int getValue() const {
    return value;
}
```

**First const:** Return value is const (usually unnecessary for basic types)  
**Second const:** Method won't modify object  

---

## 10. Scope Resolution Operator (::)

### What is :: ?

The **scope resolution operator** accesses members of a class or namespace from outside their scope.

### Uses

**1. Define class methods outside class:**

```cpp
// In header (Map.hpp)
class Mymap {
    Mymap(int size);  // Declaration
};

// In implementation (Map.cpp)
Mymap::Mymap(int size) {  // Definition
    // :: says "this Mymap belongs to Mymap class"
}
```

**2. Access namespace members:**

```cpp
std::cout << "Hello";  // std is namespace, cout is member
```

**3. Access global variable:**

```cpp
int x = 10;  // Global

void func() {
    int x = 20;  // Local
    cout << x << endl;    // Prints 20 (local)
    cout << ::x << endl;  // Prints 10 (global)
}
```

---

### Why Needed?

**Without ::**

```cpp
// This doesn't work:
Mymap(int size) {  // âŒ Compiler doesn't know which class!
}
```

**With ::**

```cpp
Mymap::Mymap(int size) {  // âœ… Clear: Mymap's constructor
}
```

---

### Multiple Classes

```cpp
class Class1 {
    void method();
};

class Class2 {
    void method();
};

void Class1::method() {  // Class1's method
    cout << "Class1" << endl;
}

void Class2::method() {  // Class2's method
    cout << "Class2" << endl;
}
```

---

## Summary

### Key C++ Concepts

1. **Header Guards** - Prevent multiple inclusion (#ifndef, #define, #endif)
2. **new/delete** - C++ dynamic memory (better than malloc/free)
3. **nullptr** - Modern null pointer (C++11+)
4. **char*** - Pointer to pointer (arrays of strings)
5. **strcpy/strlen** - C string functions
6. **Initializer List** - Efficient member initialization
7. **const** - Immutability and const-correctness
8. **::** - Scope resolution operator

### Best Practices

âœ… Use header guards in all .hpp files  
âœ… Use `new`/`delete` in C++ (not malloc/free)  
âœ… Use `nullptr` instead of NULL  
âœ… Match allocation: `new` with `delete`, `new[]` with `delete[]`  
âœ… Use const for methods that don't modify state  
âœ… Use member initializer lists  
âœ… Always check pointer != nullptr before use  

---

## 11. const Correctness in Methods

### What is const Correctness?

**const correctness** is a C++ programming practice where you mark methods and return types as `const` when they don't modify object state or data. This helps the compiler enforce immutability and prevents accidental modifications.

### Syntax for const Methods

```cpp
class MyClass {
    int value;
public:
    int getValue() const {  // const method - doesn't modify object
        return value;
    }
    
    void setValue(int v) {  // Non-const method - modifies object
        value = v;
    }
};
```

### Why Use const Methods?

1. **Compiler Enforcement** - Prevents accidental modification
2. **Interface Contract** - Shows users this method is safe (read-only)
3. **const Objects** - Allows method calls on const objects
4. **Optimization** - Compiler can optimize better with const

### Example from Map.hpp

**Before (Incorrect):**
```cpp
char* getDocument(int i){
    return documents[i];
}
```

**Problems:**
❌ Non-const method - can't call on const Mymap objects  
❌ Returns non-const pointer - caller can modify the document  

**After (Correct):**
```cpp
const char* getDocument(int i) const {
    return documents[i];
}
```

**Benefits:**
✅ `const` method - can be called on const Mymap objects  
✅ Returns `const char*` - caller cannot modify the document  
✅ Enforces read-only access to document data  

### Visual Explanation

```cpp
// Without const correctness:
Mymap map(3, 100);
char* doc = map.getDocument(0);
doc[0] = 'X';  // ❌ Oops! Modified the document accidentally!

// With const correctness:
Mymap map(3, 100);
const char* doc = map.getDocument(0);
doc[0] = 'X';  // ✅ Compile ERROR! Cannot modify const data
```

### const Object Usage

```cpp
void processMap(const Mymap& map) {
    // Only const methods can be called on const objects
    
    const char* doc = map.getDocument(0);  // ✅ OK - const method
    int size = map.get_size();              // ✅ OK - const method
    
    // map.insert("text", 0);               // ❌ ERROR - non-const method
}
```

### Return Type const

| Return Type | Meaning | Can Modify? |
|-------------|---------|-------------|
| `char*` | Mutable pointer | Yes ❌ |
| `const char*` | Pointer to const data | No ✅ |
| `char* const` | Const pointer to mutable data | Data yes, pointer no |
| `const char* const` | Const pointer to const data | Nothing ✅ |

### Best Practices

✅ Mark all getters as `const` methods  
✅ Return `const` pointers/references for read-only data  
✅ Use const references in function parameters: `void func(const Mymap& map)`  
✅ Declare objects as const when they won't change: `const Mymap map(3, 100);`  

---

## 12. Why doc_lengths Stores Word Count

### Background

The `doc_lengths` array in the Mymap class stores important information about each document. Originally it stored character count, but was changed to store **word count** for a specific algorithmic reason.

### What is BM25?

**BM25** (Best Matching 25) is a ranking algorithm used in search engines to score documents based on relevance to a query. It's one of the most widely used algorithms in information retrieval.

### The avgdl (Average Document Length) Requirement

BM25 algorithm needs to calculate the **average document length** across all documents:

```cpp
avgdl = (total words in all documents) / (number of documents)
```

**Why word count, not character count?**
- BM25 measures relevance by **term frequency** (how many times words appear)
- Longer documents naturally have more words, which can bias scoring
- BM25 normalizes scores using avgdl to be fair to shorter documents

### Example Calculation

```
Document 1: "hello world" → 2 words
Document 2: "hello world from earth" → 4 words  
Document 3: "world" → 1 word

avgdl = (2 + 4 + 1) / 3 = 2.33 words
```

### How It Works in Our Code

**In split() function (document_store.cpp):**
```cpp
void split(char* temp, int id, TrieNode* trie, Mymap* mymap){
    char* token;
    token = strtok(temp, " \t");
    int i = 0;                    // Word counter
    while(token != NULL){
        i++;                      // Count each word
        trie->insert(token, id);
        token = strtok(NULL, " \t");
    }
    mymap->setlength(id, i);      // Store word count
}
```

**In Map class:**
```cpp
void setlength(int id, int length){
    doc_lengths[id] = length;  // length = word count
}

int getlength(int id){
    return doc_lengths[id];    // Returns word count
}
```

### Why the Change?

**Original Implementation (Wrong):**
```cpp
// In Map.cpp insert():
doc_lengths[i] = len;  // ❌ Stored character count
```

**Problem:**
- Character count includes spaces, punctuation
- Doesn't reflect actual term frequency
- Can't calculate proper avgdl for BM25

**Fixed Implementation (Correct):**
```cpp
// In Map.cpp insert():
// doc_lengths will be set by split() with word count

// split() calls:
mymap->setlength(id, wordCount);  // ✅ Stores word count
```

**Benefit:**
✅ Each document's word count is stored  
✅ Can calculate avgdl for BM25 algorithm  
✅ Proper normalization in search ranking  

### Future Usage

When search functionality is implemented:

```cpp
void search(char* query, Trienode* trie, Mymap* map) {
    // Calculate average document length
    double avgdl = 0;
    for(int i = 0; i < map->get_size(); i++){
        avgdl += (double)map->getlength(i);  // Sum all word counts
    }
    avgdl /= (double)map->get_size();        // Divide by doc count
    
    // Use avgdl in BM25 scoring formula
    // score = IDF * (tf * (k1 + 1)) / (tf + k1 * (1 - b + b * docLength / avgdl))
}
```

### Key Takeaway

**Character count** → Used for memory allocation  
**Word count** → Used for search relevance scoring (BM25)  

By storing word count in `doc_lengths`, we prepare the data structure for efficient BM25 implementation in future search functionality.

---

## Summary

### Key C++ Concepts

1. **Header Guards** - Prevent multiple inclusion (#ifndef, #define, #endif)
2. **new/delete** - C++ dynamic memory (better than malloc/free)
3. **nullptr** - Modern null pointer (C++11+)
4. **char*** - Pointer to pointer (arrays of strings)
5. **strcpy/strlen** - C string functions
6. **Initializer List** - Efficient member initialization
7. **const** - Immutability and const-correctness
8. **::** - Scope resolution operator
9. **const Methods** - Read-only member functions
10. **BM25 avgdl** - Why we store word count not character count

### Best Practices

✅ Use header guards in all .hpp files  
✅ Use `new`/`delete` in C++ (not malloc/free)  
✅ Use `nullptr` instead of NULL  
✅ Match allocation: `new` with `delete`, `new[]` with `delete[]`  
✅ Use const for methods that don't modify state  
✅ Use member initializer lists  
✅ Always check pointer != nullptr before use  
✅ Return const pointers for read-only data access  
✅ Store word count for BM25 algorithm compatibility  

---

**Document Version**: 1.1  
**Last Updated**: December 26, 2025  
**Changes**: Added const correctness and BM25 word count explanation  
**Repository**: github.com/adarshpheonix2810/high-performance-search-engine-cpp
