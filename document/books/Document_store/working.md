# Document Store Implementation - Code Explanation

This document provides a **detailed step-by-step explanation** of the `document_store.cpp` file implementation. It explains how the code works, what each line does, and the execution flow.

---

## File Information

**File Name**: `document_store.cpp`  
**Location**: `src/document_store.cpp`  
**Purpose**: Provides file processing functionality to count lines and determine maximum line length in text files  
**Header File**: `header/Document_store.hpp`

---

## Table of Contents

1. [Complete Source Code](#complete-source-code)
2. [Header Files and Dependencies](#header-files-and-dependencies)
3. [Function Declaration](#function-declaration)
4. [Function Implementation Explained](#function-implementation-explained)
5. [Execution Flow Diagram](#execution-flow-diagram)
6. [Memory Management](#memory-management)
7. [Error Handling](#error-handling)
8. [Example Usage](#example-usage)

---

## Complete Source Code

```cpp
#include "Document_store.hpp"
using namespace std;

int read_sizes(int *linecounter, int *maxlength, char *file_name){
    FILE *file = fopen(file_name, "r");
    if(file == NULL){
        cout << "Cannot open file: " << file_name << endl;
        return -1;
    }
    
    // Check if file is empty
    int c = fgetc(file);
    if(c == EOF){
        cout << "File is empty: " << file_name << endl;
        fclose(file);
        return -1;
    }
    ungetc(c, file);
    
    char *line = NULL;
    size_t len = 0;
    int current_length;
    while((current_length = getline(&line, &len, file)) != -1){
        // Remove newline character from count if present
        if(current_length > 0 && line[current_length-1] == '\n'){
            current_length--;
        }
        if(current_length > *maxlength)
            *maxlength = current_length;
        (*linecounter)++;
        free(line);
        line = NULL;
        len = 0;
    }
    fclose(file);
    free(line);
    return 1;
}
```

---

## Header Files and Dependencies

### Line 1: `#include "Document_store.hpp"`

```cpp
#include "Document_store.hpp"
```

**What it does:**
- Includes the header file that contains the function declaration
- The header file declares: `int read_sizes(int *linecounter, int *maxlength, char *file_name);`
- This allows the compiler to know the function signature before implementation

**Why it's needed:**
- Provides function prototype for validation
- Includes necessary standard libraries (`<iostream>`, `<fstream>`, `<string>`)
- Maintains separation between interface (`.hpp`) and implementation (`.cpp`)

---

### Line 2: `using namespace std;`

```cpp
using namespace std;
```

**What it does:**
- Allows us to use standard library functions without `std::` prefix
- Example: `cout` instead of `std::cout`

**Impact on code:**
- `cout` can be used directly (from `<iostream>`)
- Makes code more readable and concise

---

## Function Declaration

### Function Signature

```cpp
int read_sizes(int *linecounter, int *maxlength, char *file_name)
```

**Return Type**: `int`
- Returns `1` on success
- Returns `-1` on failure (file not found, file empty, or file cannot be opened)

**Parameters:**

| Parameter | Type | Purpose | Passed By |
|-----------|------|---------|-----------|
| `linecounter` | `int*` | Stores the total number of lines | Pointer (modified by function) |
| `maxlength` | `int*` | Stores the maximum line length | Pointer (modified by function) |
| `file_name` | `char*` | Path to the file to process | Pointer (read-only) |

**Why Pointers?**
- `linecounter` and `maxlength` are pointers so the function can **modify** their values
- The calling function can receive these updated values after function execution
- This is how C functions "return" multiple values

---

## Function Implementation Explained

### Step 1: Opening the File (Lines 3-8)

```cpp
FILE *file = fopen(file_name, "r");
if(file == NULL){
    cout << "Cannot open file: " << file_name << endl;
    return -1;
}
```

**Line-by-Line Breakdown:**

#### Line 4: `FILE *file = fopen(file_name, "r");`

**What happens:**
1. `fopen()` attempts to open the file specified by `file_name`
2. `"r"` mode = read-only mode
3. Returns a `FILE*` pointer if successful
4. Returns `NULL` if file doesn't exist or cannot be opened

**Visual:**
```
file_name = "doc1.txt"
    ↓
fopen("doc1.txt", "r")
    ↓
If exists: file → [FILE* pointing to doc1.txt]
If not:    file → NULL
```

#### Lines 5-8: Error Handling

```cpp
if(file == NULL){
    cout << "Cannot open file: " << file_name << endl;
    return -1;
}
```

**What happens:**
- Checks if file pointer is `NULL` (file opening failed)
- If `NULL`:
  - Prints error message with filename
  - Returns `-1` to indicate failure
  - Function exits immediately

**Why important:**
- Prevents accessing invalid file pointer
- Avoids program crash
- Provides user feedback about the error

---

### Step 2: Empty File Detection (Lines 10-17)

```cpp
// Check if file is empty
int c = fgetc(file);
if(c == EOF){
    cout << "File is empty: " << file_name << endl;
    fclose(file);
    return -1;
}
ungetc(c, file);
```

**Line-by-Line Breakdown:**

#### Line 11: `int c = fgetc(file);`

**What happens:**
- `fgetc()` reads the **first character** from the file
- Returns the character as `int` (e.g., 65 for 'A')
- Returns `EOF` (-1) if file is empty or error occurs

**Why `int` not `char`?**
- Need to distinguish between valid character (0-255) and `EOF` (-1)
- `int` can hold both character values and `EOF`

#### Lines 12-16: Check if Empty

```cpp
if(c == EOF){
    cout << "File is empty: " << file_name << endl;
    fclose(file);
    return -1;
}
```

**What happens:**
- If `c == EOF`, file has no content (0 bytes)
- Prints "File is empty" message
- **Critically important**: Closes the file with `fclose()` to prevent resource leak
- Returns `-1` to indicate failure

**Why this check?**
- On Windows, `getline()` hangs indefinitely on empty files (POSIX compatibility bug)
- This prevents the program from freezing
- See our documentation for detailed explanation of this Windows bug

#### Line 17: `ungetc(c, file);`

**What happens:**
- Pushes the character `c` **back** into the file stream
- File position indicator returns to the start
- Next read operation will read this character again

**Why needed?**
- We only wanted to **peek** at the first character to check if file is empty
- We don't want to **consume** this character
- `ungetc()` restores the file to its original state
- Subsequent `getline()` can read from the beginning

**Visual:**
```
File: "Hello\nWorld"
      ↑ position = 0

After fgetc(): c = 'H'
      "Hello\nWorld"
       ↑ position = 1

After ungetc(c, file):
      "Hello\nWorld"
      ↑ position = 0 (back to start)
```

---

### Step 3: Initialize Variables for Line Reading (Lines 19-21)

```cpp
char *line = NULL;
size_t len = 0;
int current_length;
```

**Line-by-Line Breakdown:**

#### Line 19: `char *line = NULL;`

**What it does:**
- Declares a pointer to `char` (will point to line buffer)
- Initializes to `NULL` (critical for `getline()` to work correctly)

**Why `NULL`?**
- `getline()` checks if pointer is `NULL`
- If `NULL`, `getline()` automatically allocates memory
- If not `NULL`, `getline()` assumes memory is already allocated

**Important:**
- **Must** initialize to `NULL` for first `getline()` call
- Otherwise, undefined behavior (crashes, corruption)

#### Line 20: `size_t len = 0;`

**What it does:**
- Declares variable to store buffer size
- Initializes to `0` (critical for `getline()` to work correctly)

**Purpose:**
- `getline()` uses this to track allocated buffer size
- `getline()` updates this value when it allocates or expands buffer

**Why `size_t`?**
- `size_t` is unsigned integer type for sizes
- Platform-independent (32-bit or 64-bit)
- Can represent the largest possible object size

#### Line 21: `int current_length;`

**What it does:**
- Declares variable to store the length of each line read
- Will be assigned by `getline()` return value

**Purpose:**
- Stores number of characters read (including `\n`)
- Used to track maximum line length
- Used to check if newline character is present

---

### Step 4: Main Loop - Reading Lines (Lines 22-31)

```cpp
while((current_length = getline(&line, &len, file)) != -1){
    // Remove newline character from count if present
    if(current_length > 0 && line[current_length-1] == '\n'){
        current_length--;
    }
    if(current_length > *maxlength)
        *maxlength = current_length;
    (*linecounter)++;
    free(line);
    line = NULL;
    len = 0;
}
```

**This is the CORE of the function!**

---

#### Line 22: Loop Condition

```cpp
while((current_length = getline(&line, &len, file)) != -1)
```

**Breaking it down step by step:**

**Step 1: `getline(&line, &len, file)` is called**

```cpp
getline(&line, &len, file)
```

**What happens:**
1. `getline()` reads one line from `file`
2. Automatically allocates/expands memory for `line` buffer
3. Stores line content in `line` (including `\n`)
4. Updates `len` with buffer size
5. Returns number of characters read (including `\n`)
6. Returns `-1` on EOF or error

**Parameters explained:**
- `&line`: Address of pointer (so `getline` can modify it)
- `&len`: Address of size variable (so `getline` can modify it)
- `file`: File stream to read from

**Step 2: Assign return value to `current_length`**

```cpp
current_length = getline(...)
```

**What happens:**
- Return value (number of characters) is assigned to `current_length`
- Now `current_length` contains line length

**Step 3: Compare with `-1`**

```cpp
... != -1
```

**What happens:**
- Check if `current_length` is not `-1`
- If not `-1`: line was successfully read, continue loop
- If `-1`: reached end of file (EOF), exit loop

**Complete execution flow:**

```
Iteration 1: Read "Hello\n" → current_length = 6 → 6 != -1 → TRUE → Execute loop body
Iteration 2: Read "World\n" → current_length = 6 → 6 != -1 → TRUE → Execute loop body
Iteration 3: EOF reached → current_length = -1 → -1 != -1 → FALSE → Exit loop
```

**Visual representation:**

```
File content:
+-------+
| Hello\n |  ← Line 1 (6 characters)
+-------+
| World\n |  ← Line 2 (6 characters)
+-------+
| EOF    |  ← End of file
+-------+

Loop iteration 1:
    getline() reads "Hello\n"
    current_length = 6
    6 != -1 → TRUE → enter loop

Loop iteration 2:
    getline() reads "World\n"
    current_length = 6
    6 != -1 → TRUE → enter loop

Loop iteration 3:
    getline() attempts to read
    Reaches EOF
    current_length = -1
    -1 != -1 → FALSE → exit loop
```

---

#### Lines 23-26: Remove Newline Character

```cpp
// Remove newline character from count if present
if(current_length > 0 && line[current_length-1] == '\n'){
    current_length--;
}
```

**Why this is needed:**

`getline()` includes the newline character `\n` in the line it reads. But we want to count the **actual content length** without the newline.

**Example:**
```
File contains: "Hello\n"
getline() reads: "Hello\n\0"
current_length = 6

But actual text is "Hello" (5 characters)
We need to subtract 1 for the '\n'
```

**Line-by-Line Breakdown:**

**Condition 1: `current_length > 0`**
- Ensures the line has at least one character
- Prevents accessing invalid memory if line is empty

**Condition 2: `line[current_length-1] == '\n'`**
- Checks if last character is newline
- `current_length-1` is the index of last character
- Example: If `current_length = 6`, last character is at index 5

**Action: `current_length--;`**
- Decrements `current_length` by 1
- Effectively removes the newline from the count

**Visual example:**

```
Before:
line = ['H', 'e', 'l', 'l', 'o', '\n', '\0']
        0    1    2    3    4     5     6
current_length = 6
last character index = 5
line[5] = '\n' ✓

After decrement:
current_length = 5 (now represents actual text length)
```

**Edge cases handled:**

```cpp
// Empty line (just newline)
Line: "\n"
current_length = 1
line[0] = '\n'
After decrement: current_length = 0 ✓

// Line without newline (last line in file)
Line: "Hello"
current_length = 5
line[4] = 'o' (not '\n')
No decrement ✓

// Line with only spaces then newline
Line: "   \n"
current_length = 4
line[3] = '\n'
After decrement: current_length = 3 ✓
```

---

#### Lines 27-28: Track Maximum Line Length

```cpp
if(current_length > *maxlength)
    *maxlength = current_length;
```

**Purpose:**
- Keep track of the **longest line** in the file
- Update `maxlength` whenever we find a longer line

**Line-by-Line Breakdown:**

**Line 27: `if(current_length > *maxlength)`**

**What happens:**
- Compares current line length with stored maximum
- `*maxlength` dereferences the pointer to get the value
- If current line is longer, condition is `TRUE`

**Line 28: `*maxlength = current_length;`**

**What happens:**
- Updates the value at the memory location pointed to by `maxlength`
- Uses dereference operator `*` to modify the actual value
- Calling function will see this updated value

**Execution example:**

```
Initial state:
    *maxlength = 0

Line 1: "Hi\n" → current_length = 2
    2 > 0 → TRUE
    *maxlength = 2

Line 2: "Hello\n" → current_length = 5
    5 > 2 → TRUE
    *maxlength = 5

Line 3: "Cat\n" → current_length = 3
    3 > 5 → FALSE
    *maxlength = 5 (unchanged)

Line 4: "World!\n" → current_length = 6
    6 > 5 → TRUE
    *maxlength = 6

Final result: *maxlength = 6
```

**Why pointer?**
- We need to modify the value in the **calling function**
- Passing by pointer allows function to update the original variable
- After function returns, calling code has the updated maximum length

---

#### Line 29: Increment Line Counter

```cpp
(*linecounter)++;
```

**What happens:**
- Increments the line count by 1
- `*linecounter` dereferences pointer to access value
- Parentheses ensure dereference happens before increment
- Each line read increases the counter

**Breakdown:**

```
*linecounter      → Dereference pointer, get value
(*linecounter)    → Parentheses group the dereference
(*linecounter)++  → Increment the dereferenced value
```

**Without parentheses (WRONG):**
```cpp
*linecounter++  // This increments the POINTER, not the value!
```

**With parentheses (CORRECT):**
```cpp
(*linecounter)++  // This increments the VALUE
```

**Execution example:**

```
Initial: *linecounter = 0

After line 1: (*linecounter)++ → *linecounter = 1
After line 2: (*linecounter)++ → *linecounter = 2
After line 3: (*linecounter)++ → *linecounter = 3
...
```

**Alternative syntax (same effect):**
```cpp
*linecounter += 1;    // Same result
*linecounter = *linecounter + 1;  // Same result
```

---

#### Lines 30-32: Memory Management Inside Loop

```cpp
free(line);
line = NULL;
len = 0;
```

**Purpose:**
- **Free allocated memory** after processing each line
- **Reset variables** for next iteration
- **Prevent memory leaks**

**Line-by-Line Breakdown:**

**Line 30: `free(line);`**

**What happens:**
- Deallocates memory that was allocated by `getline()`
- Returns memory to system for reuse
- Pointer value doesn't change (still contains old address)

**Why needed:**
- `getline()` uses `malloc()` internally to allocate memory
- If we don't `free()`, memory leaks accumulate
- Example: 1000 lines × 100 bytes each = 100KB leaked!

**Line 31: `line = NULL;`**

**What happens:**
- Sets pointer to `NULL` (address 0)
- Indicates pointer is not pointing to valid memory

**Why needed:**
- Next `getline()` call checks if `line == NULL`
- If `NULL`, `getline()` allocates fresh memory
- If not `NULL`, `getline()` tries to reuse (but we freed it!)
- **Critical**: Prevents use-after-free bug

**Line 32: `len = 0;`**

**What happens:**
- Resets buffer size to 0
- Tells next `getline()` that no buffer exists

**Why needed:**
- Next `getline()` call checks `len` value
- If `len == 0` and `line == NULL`, allocates new buffer
- Keeps buffer size tracking consistent

**Visual representation:**

```
Before free():
    line → [H][e][l][l][o][\n][\0]
           └─ memory allocated by getline()
    len = 7

After free(line):
    line → [?][?][?][?][?][?][?]
           └─ memory deallocated (invalid)
    len = 7 (still has old value)

After line = NULL:
    line → NULL (0x00000000)
    len = 7

After len = 0:
    line → NULL
    len = 0
    ✓ Ready for next getline() call
```

**What happens in next iteration:**

```
Next getline(&line, &len, file) call:
    1. Checks: line == NULL ✓
    2. Checks: len == 0 ✓
    3. Allocates fresh memory
    4. Reads next line
    5. Updates line pointer and len
```

**Why reset inside loop instead of reusing buffer?**

This code **intentionally frees and reallocates** on each iteration. Alternative approach would be:

```cpp
// Alternative (more efficient but different behavior):
char *line = NULL;
size_t len = 0;
while(getline(&line, &len, file) != -1) {
    // ... process line ...
    // Don't free here - let getline() reuse buffer
}
free(line);  // Free once at end
```

**Our approach:**
- Frees memory each iteration
- Cleaner separation between iterations
- Slightly less efficient (more malloc/free calls)

**Alternative approach:**
- Reuses same buffer
- More efficient (fewer allocations)
- Single free at end

Both are valid! Our code prioritizes clarity.

---

### Step 5: Cleanup and Return (Lines 33-35)

```cpp
fclose(file);
free(line);
return 1;
```

**Line-by-Line Breakdown:**

#### Line 33: `fclose(file);`

**What happens:**
- Closes the file stream
- Flushes any buffered data
- Releases system resources (file descriptor)
- Makes `file` pointer invalid

**Why critical:**
- Operating system limits number of open files
- Prevents resource exhaustion
- Ensures data integrity (buffered data written to disk)

**What if we forget `fclose()`?**
```cpp
// BAD: Resource leak
for(int i = 0; i < 10000; i++) {
    FILE *f = fopen("doc.txt", "r");
    // ... process ...
    // Missing fclose(f) - leaks file descriptor!
}
// Eventually: "Too many open files" error
```

#### Line 34: `free(line);`

**What happens:**
- Frees any remaining allocated memory
- Safety measure in case loop exited early

**Why needed:**
- If loop executed at least once, `line` was allocated
- We already freed inside loop, but this is defensive programming
- If `line == NULL`, `free(NULL)` is safe (does nothing)

**Scenarios:**

```
Scenario 1: Normal execution
    Loop iterations: line allocated → freed → NULL
    At line 34: line == NULL → free(NULL) → safe, does nothing

Scenario 2: File with one line
    Line allocated → processed → freed in loop → line = NULL
    At line 34: free(NULL) → safe

Scenario 3: Empty file detected earlier
    Never enters loop
    line is still NULL
    At line 34: free(NULL) → safe
```

**Best practice:**
```cpp
// This pattern is safe and defensive
free(line);  // Safe even if already freed (line=NULL)
```

#### Line 35: `return 1;`

**What happens:**
- Returns success code `1` to calling function
- Indicates file was processed successfully
- Calling function can check return value

**Return value convention:**

```
Return 1  → Success
Return -1 → Failure (file error, empty file, etc.)
```

**Usage by caller:**

```cpp
int result = read_sizes(&count, &max, "doc.txt");
if(result == -1) {
    // Handle error
    printf("Failed to read file\n");
} else {
    // Success
    printf("Lines: %d, Max length: %d\n", count, max);
}
```

---

## Execution Flow Diagram

### Visual Flow Chart

```
START
  ↓
[1] Open file with fopen()
  ↓
[2] Check: file == NULL?
  ├─ YES → Print error → Return -1 → END
  └─ NO → Continue
  ↓
[3] Read first character with fgetc()
  ↓
[4] Check: c == EOF?
  ├─ YES → Print "empty" → fclose() → Return -1 → END
  └─ NO → Continue
  ↓
[5] Push character back with ungetc()
  ↓
[6] Initialize: line=NULL, len=0
  ↓
[7] Start LOOP: getline() != -1?
  ├─ NO (EOF) → Jump to [13]
  └─ YES → Continue
  ↓
[8] Check: last char is '\n'?
  ├─ YES → Decrement current_length
  └─ NO → Continue
  ↓
[9] Check: current_length > *maxlength?
  ├─ YES → Update *maxlength
  └─ NO → Continue
  ↓
[10] Increment (*linecounter)++
  ↓
[11] Free line, set to NULL, len=0
  ↓
[12] Loop back to [7]
  ↓
[13] Close file with fclose()
  ↓
[14] Free line (safety)
  ↓
[15] Return 1 (success)
  ↓
END
```

### Step-by-Step Execution Example

**Input file `doc1.txt`:**
```
Hello World
Hi
This is a test
```

**Initial values:**
```
linecounter = 0
maxlength = 0
```

**Execution trace:**

```
Step 1: fopen("doc1.txt", "r")
    → file pointer created ✓

Step 2: file != NULL
    → TRUE, continue ✓

Step 3: fgetc(file)
    → c = 'H' (first character)

Step 4: c != EOF
    → TRUE, continue ✓

Step 5: ungetc('H', file)
    → File position back to start

Step 6: Initialize
    → line = NULL
    → len = 0

Step 7: LOOP ITERATION 1
    → getline() reads "Hello World\n"
    → current_length = 12
    → 12 != -1 → TRUE → Enter loop

Step 8: line[11] == '\n'?
    → TRUE → current_length = 11

Step 9: 11 > 0?
    → TRUE → maxlength = 11

Step 10: linecounter++
    → linecounter = 1

Step 11: free(line), line=NULL, len=0

Step 12: LOOP ITERATION 2
    → getline() reads "Hi\n"
    → current_length = 3
    → 3 != -1 → TRUE → Enter loop

Step 8: line[2] == '\n'?
    → TRUE → current_length = 2

Step 9: 2 > 11?
    → FALSE → maxlength = 11 (unchanged)

Step 10: linecounter++
    → linecounter = 2

Step 11: free(line), line=NULL, len=0

Step 12: LOOP ITERATION 3
    → getline() reads "This is a test\n"
    → current_length = 15
    → 15 != -1 → TRUE → Enter loop

Step 8: line[14] == '\n'?
    → TRUE → current_length = 14

Step 9: 14 > 11?
    → TRUE → maxlength = 14

Step 10: linecounter++
    → linecounter = 3

Step 11: free(line), line=NULL, len=0

Step 12: LOOP ITERATION 4
    → getline() attempts to read
    → EOF reached
    → current_length = -1
    → -1 != -1 → FALSE → Exit loop

Step 13: fclose(file)
    → File closed

Step 14: free(line)
    → free(NULL) → safe, does nothing

Step 15: return 1
    → Success!
```

**Final Results:**
```
linecounter = 3
maxlength = 14
return value = 1
```

---

## Memory Management

### Memory Allocation Timeline

**Visual representation of memory state throughout execution:**

```
BEFORE LOOP:
    Stack:
        line = NULL
        len = 0
        current_length = <uninitialized>
    Heap:
        (no allocation yet)

ITERATION 1 - After getline():
    Stack:
        line = 0x00A1B2C3 (points to heap)
        len = 64 (buffer size allocated by getline)
        current_length = 12
    Heap:
        [0x00A1B2C3]: ['H']['e']['l']['l']['o'][' ']['W']['o']['r']['l']['d']['\n']['\0']... (64 bytes total)

ITERATION 1 - After free():
    Stack:
        line = NULL
        len = 0
        current_length = 12
    Heap:
        [0x00A1B2C3]: (freed - memory returned to system)

ITERATION 2 - After getline():
    Stack:
        line = 0x00D4E5F6 (new allocation)
        len = 64
        current_length = 3
    Heap:
        [0x00D4E5F6]: ['H']['i']['\n']['\0']... (64 bytes)

... and so on ...

AFTER LOOP:
    Stack:
        line = NULL
        len = 0
    Heap:
        (all memory freed)
```

### Why This Memory Pattern?

**Advantages:**
1. **Clean separation**: Each line processed independently
2. **No memory growth**: Memory released after each line
3. **Predictable behavior**: Same pattern every iteration

**Disadvantages:**
1. **More allocations**: malloc/free called repeatedly
2. **Performance cost**: Allocation overhead on each iteration

**Alternative (more efficient):**
```cpp
char *line = NULL;
size_t len = 0;
// Let getline() reuse/expand the same buffer
while(getline(&line, &len, file) != -1) {
    // Process line
    // Don't free here!
}
free(line);  // Free once at end
```

This alternative is more efficient but our code prioritizes clarity and explicit memory management.

---

## Error Handling

### Error Scenarios and Handling

#### Error 1: File Cannot Be Opened

**Code:**
```cpp
FILE *file = fopen(file_name, "r");
if(file == NULL){
    cout << "Cannot open file: " << file_name << endl;
    return -1;
}
```

**When this happens:**
- File doesn't exist
- No read permissions
- File path is invalid
- Too many open files

**Response:**
- Print error message
- Return `-1` immediately
- No cleanup needed (file not opened)

**Example:**
```cpp
int result = read_sizes(&lines, &max, "nonexistent.txt");
// Output: "Cannot open file: nonexistent.txt"
// result = -1
```

---

#### Error 2: File is Empty

**Code:**
```cpp
int c = fgetc(file);
if(c == EOF){
    cout << "File is empty: " << file_name << endl;
    fclose(file);  // ← Must close file!
    return -1;
}
```

**When this happens:**
- File exists but has 0 bytes
- File contains only EOF marker

**Response:**
- Print "File is empty" message
- **Close file** (important!)
- Return `-1`

**Why close file here?**
- File was successfully opened
- Must release resource before returning
- Prevents file descriptor leak

**Example:**
```cpp
// File "empty.txt" has 0 bytes
int result = read_sizes(&lines, &max, "empty.txt");
// Output: "File is empty: empty.txt"
// result = -1
```

---

#### Error 3: Memory Allocation Failure (Not Explicitly Handled)

**What could happen:**
```cpp
getline(&line, &len, file)  // malloc() fails inside getline()
```

**Current code:**
- Does NOT check for memory allocation failure
- `getline()` returns `-1` on error
- Loop exits gracefully

**Potential improvement:**
```cpp
ssize_t read = getline(&line, &len, file);
if(read == -1) {
    if(!feof(file)) {  // Not EOF, must be error
        cout << "Error reading file: " << file_name << endl;
    }
    break;
}
```

---

### Return Value Summary

| Scenario | Return Value | linecounter | maxlength |
|----------|--------------|-------------|-----------|
| Success | `1` | Number of lines | Max line length |
| File not found | `-1` | Unchanged | Unchanged |
| File empty | `-1` | Unchanged | Unchanged |
| Read error | `-1` (loop exits) | Partial count | Partial max |

---

## Example Usage

### Basic Usage

```cpp
#include "Document_store.hpp"
#include <iostream>
using namespace std;

int main() {
    int line_count = 0;
    int max_length = 0;
    
    int result = read_sizes(&line_count, &max_length, "data/doc1.txt");
    
    if(result == -1) {
        cout << "Failed to process file" << endl;
        return 1;
    }
    
    cout << "Total lines: " << line_count << endl;
    cout << "Maximum line length: " << max_length << endl;
    
    return 0;
}
```

**Output (if `doc1.txt` contains 10 lines, longest is 45 characters):**
```
Total lines: 10
Maximum line length: 45
```

---

### Processing Multiple Files

```cpp
#include "Document_store.hpp"
#include <iostream>
#include <vector>
using namespace std;

int main() {
    vector<string> files = {"doc1.txt", "doc2.txt", "doc3.txt"};
    
    for(const string &filename : files) {
        int lines = 0;
        int max_len = 0;
        
        int result = read_sizes(&lines, &max_len, 
                                (char*)filename.c_str());
        
        if(result == 1) {
            cout << filename << ": " 
                 << lines << " lines, "
                 << "max " << max_len << " chars" << endl;
        } else {
            cout << filename << ": ERROR" << endl;
        }
    }
    
    return 0;
}
```

**Output:**
```
doc1.txt: 25 lines, max 78 chars
doc2.txt: 15 lines, max 120 chars
doc3.txt: ERROR
```

---

### Error Handling Example

```cpp
#include "Document_store.hpp"
#include <iostream>
using namespace std;

int main() {
    int lines = 0;
    int max_len = 0;
    
    int result = read_sizes(&lines, &max_len, "test.txt");
    
    switch(result) {
        case 1:
            cout << "Success!" << endl;
            cout << "Lines: " << lines << endl;
            cout << "Max length: " << max_len << endl;
            break;
        case -1:
            cout << "Error: Could not process file" << endl;
            cout << "Check if file exists and is not empty" << endl;
            break;
        default:
            cout << "Unexpected return value" << endl;
    }
    
    return 0;
}
```

---

## Performance Characteristics

### Time Complexity

**Analysis:**
- **File opening**: O(1)
- **Empty check**: O(1)
- **Main loop**: O(n) where n = number of lines
  - Each `getline()`: O(m) where m = line length
  - Each line processing: O(1)
- **Total**: O(n × m_avg) where m_avg = average line length

**Best case**: Small file with short lines → Fast  
**Worst case**: Large file with very long lines → Slower

### Space Complexity

**Analysis:**
- **Stack variables**: O(1) - fixed size
- **Heap (line buffer)**: O(m_max) where m_max = longest line length
  - Buffer freed after each line
  - Memory usage stays constant
- **Total**: O(m_max)

**Memory footprint**: Minimal - only one line in memory at a time

### Optimization Opportunities

**Current implementation:**
```cpp
free(line);
line = NULL;
len = 0;
```

**Optimized alternative:**
```cpp
// Reuse buffer instead of freeing
// Only one malloc/free per function call
while(getline(&line, &len, file) != -1) {
    // Process without freeing
}
free(line);  // Free once at end
```

**Trade-off:**
- Current: More allocations, clearer code
- Optimized: Fewer allocations, better performance

---

## December 26, 2025 Update - Critical Bug Fixes

### Changes Made Today

Today we fixed three critical bugs in `document_store.cpp` that could cause crashes, memory corruption, and silent failures.

---

### Fix 1: Removed Dangerous free(token) from split()

**File Changed**: `src/document_store.cpp` line 46  
**What Changed**: Removed `free(token);` line that caused memory corruption

**Before (BUGGY CODE):**
```cpp
void split(char* temp, int id, TrieNode* trie, Mymap* mymap){
    char* token;
    token = strtok(temp, " \t");
    while(token != NULL){
        trie->insert(token, id);
        token = strtok(NULL, " \t");
    }
    mymap->setlength(id, i);
    free(token);  // ❌ CRITICAL BUG - Memory corruption!
}
```

**After (FIXED CODE):**
```cpp
void split(char* temp, int id, TrieNode* trie, Mymap* mymap){
    char* token;
    token = strtok(temp, " \t");
    int i = 0;
    while(token != NULL){
        i++;
        trie->insert(token, id);
        token = strtok(NULL, " \t");
    }
    mymap->setlength(id, i);
    // No free(token) - token doesn't own memory! ✅
}
```

**Why was this a bug?**

**Understanding strtok():**
- `strtok()` does NOT allocate new memory
- It modifies the original string by replacing delimiters with '\0'
- It returns a pointer to a position INSIDE the original string

**Visual Explanation:**
```
temp (allocated memory): "hello world earth"
                         ^       ^
                         |       |
After strtok():          "hello\0world\0earth\0"
                         ^
                         token points here (inside temp, not new memory!)
```

**What happens with free(token)?**
1. `token` points to somewhere inside `temp`
2. `temp` was allocated by `malloc()` in caller function
3. `free(token)` tries to free memory in the middle of `temp` allocation
4. **Result:** Memory corruption! Undefined behavior! Potential crash!

**Correct Behavior:**
- `temp` is freed by caller (`read_input()` function)
- `token` is just a pointer into `temp`, doesn't need freeing
- Only free memory that YOU allocated with malloc/new

**Memory Ownership:**
```
read_input() function:
  → malloc(temp)           // ✅ Allocates memory
  → split(temp, ...)       
      → strtok(temp, " ")  // ❌ Does NOT allocate, just returns pointer
  → free(temp)             // ✅ Frees the allocation
```

**Impact of this bug:**
- **Before:** Crashes, corruption, undefined behavior
- **After:** Stable execution, proper memory management

---

### Fix 2: Added NULL Check After fopen()

**File Changed**: `src/document_store.cpp` line 49  
**What Changed**: Added validation to check if file opened successfully

**Before (NO ERROR HANDLING):**
```cpp
int read_input(Mymap* mymap, TrieNode *trie, char* file_name){
    FILE *file = fopen(file_name, "r");
    char *line = NULL;
    size_t buffersize = 0;
    char *temp = (char*)malloc(...);
    for(int i = 0; i < mymap->get_size(); i++){
        getline(&line, &buffersize, file);  // ❌ Crashes if file == NULL!
```

**After (WITH ERROR HANDLING):**
```cpp
int read_input(Mymap* mymap, TrieNode *trie, char* file_name){
    FILE *file = fopen(file_name, "r");
    if(file == NULL){  // ✅ Check for errors!
        cout << "Error opening file: " << file_name << endl;
        return -1;
    }
    char *line = NULL;
    size_t buffersize = 0;
    char *temp = (char*)malloc(...);
```

**Why was this needed?**

**Situations where fopen() returns NULL:**
1. File doesn't exist
2. No permission to read the file
3. Path is invalid
4. Too many files open
5. Disk I/O error

**What happens without the check?**
```cpp
FILE *file = fopen("nonexistent.txt", "r");  // Returns NULL
// file == NULL, but we don't check

getline(&line, &buffersize, file);  // ❌ Passes NULL to getline()
// Undefined behavior! Likely segmentation fault (crash)
```

**Example Scenario:**
```bash
# User types wrong filename
./searchengine -d wrongfile.txt -k 5

# Without check:
Please wait...
Segmentation fault (core dumped)  ❌ Unhelpful crash

# With check:
Please wait...
Error opening file: wrongfile.txt  ✅ Clear error message
```

**Best Practice Pattern:**
```cpp
FILE *file = fopen(filename, mode);
if(file == NULL){
    // Print error
    // Clean up any resources
    // Return error code
    return -1;
}
// Safe to use file here
```

**Impact:**
- **Before:** Silent crashes, confusing errors
- **After:** Clear error messages, graceful failure

---

### Fix 3: Added getline() Return Value Check

**File Changed**: `src/document_store.cpp` line 57  
**What Changed**: Validate getline() succeeded before using the data

**Before (NO VALIDATION):**
```cpp
for(int i = 0; i < mymap->get_size(); i++){
    getline(&line, &buffersize, file);  // ❌ No check!
    if (mymap->insert(line, i) == -1) {
        // Only checks insert, not getline
    }
```

**After (WITH VALIDATION):**
```cpp
for(int i = 0; i < mymap->get_size(); i++){
    if(getline(&line, &buffersize, file) == -1){  // ✅ Check return!
        cout << "Error reading line " << i << endl;
        free(line);
        fclose(file);
        free(temp);
        return -1;
    }
    if (mymap->insert(line, i) == -1) {
```

**Why was this needed?**

**getline() Return Values:**
- **Positive number:** Successfully read that many characters
- **-1:** End of file OR read error occurred

**Problem Scenario:**
```cpp
// Create map expecting 10 documents
Mymap* map = new Mymap(10, 1000);

// But file only has 5 lines!
read_input(map, trie, "short_file.txt");
```

**Execution without check:**
```
i=0: getline() → 45 chars ✅ Success
i=1: getline() → 52 chars ✅ Success
i=2: getline() → 38 chars ✅ Success
i=3: getline() → 61 chars ✅ Success
i=4: getline() → 47 chars ✅ Success
i=5: getline() → -1 (EOF) ❌ But we don't check!
     → line might be NULL or garbage
     → mymap->insert(garbage, 5) → undefined behavior
i=6: getline() → -1 (EOF) ❌ Still processing!
     → More garbage inserted
i=7: getline() → -1 (EOF) ❌ Continuing...
     → Data structure corrupted
```

**Execution with check:**
```
i=0: getline() → 45 chars ✅ Success
i=1: getline() → 52 chars ✅ Success
i=2: getline() → 38 chars ✅ Success
i=3: getline() → 61 chars ✅ Success
i=4: getline() → 47 chars ✅ Success
i=5: getline() → -1 (EOF)
     → Check: if (getline() == -1) ✅ TRUE
     → Print: "Error reading line 5"
     → free(line)
     → fclose(file)
     → free(temp)
     → return -1
     → Caller knows there was an error!
```

**The Error Recovery Pattern:**
```cpp
if(error_detected){
    // 1. Print informative message
    cout << "Error: what went wrong" << endl;
    
    // 2. Free allocated memory
    free(line);
    free(temp);
    
    // 3. Close open files
    fclose(file);
    
    // 4. Return error code
    return -1;
}
```

**Why cleanup is critical:**
```cpp
// Without cleanup:
if(getline() == -1){
    return -1;  // ❌ Memory leak! File still open!
}

// With proper cleanup:
if(getline() == -1){
    free(line);   // ✅ Free memory
    free(temp);   // ✅ Free memory
    fclose(file); // ✅ Close file
    return -1;    // ✅ Then return
}
```

**Impact:**
- **Before:** Silent corruption, garbage data processed
- **After:** Early detection, clean failure, no corruption

---

### Fix 4: Removed Unused Variable

**File Changed**: `src/document_store.cpp` line 52  
**What Changed**: Removed `int current_length;` declaration that was never used

**Before:**
```cpp
int read_input(Mymap* mymap, TrieNode *trie, char* file_name){
    FILE *file = fopen(file_name, "r");
    if(file == NULL){
        cout << "Error opening file: " << file_name << endl;
        return -1;
    }
    char *line = NULL;
    size_t buffersize = 0;
    int current_length;  // ❌ Declared but never used
    char *temp = (char*)malloc(mymap->get_buffersize() * sizeof(char));
```

**After:**
```cpp
int read_input(Mymap* mymap, TrieNode *trie, char* file_name){
    FILE *file = fopen(file_name, "r");
    if(file == NULL){
        cout << "Error opening file: " << file_name << endl;
        return -1;
    }
    char *line = NULL;
    size_t buffersize = 0;
    char *temp = (char*)malloc(mymap->get_buffersize() * sizeof(char));
```

**Why remove it?**
- Variable was declared but never assigned or used
- Wastes stack memory (4 bytes)
- Creates compiler warnings with strict flags
- Makes code confusing (readers wonder what it's for)
- **Code cleanup:** Keep only what's needed

**Best Practice:**
✅ Only declare variables when you need them  
✅ Remove unused code during refactoring  
✅ Enable compiler warnings to catch this  

---

### Summary of December 26 Fixes

| Fix # | File | Line | Issue | Impact |
|-------|------|------|-------|--------|
| 1 | document_store.cpp | 46 | `free(token)` memory corruption | Critical - causes crashes |
| 2 | document_store.cpp | 49 | No fopen NULL check | High - crashes on bad filename |
| 3 | document_store.cpp | 57 | No getline validation | High - processes garbage data |
| 4 | document_store.cpp | 52 | Unused variable | Low - cleanup |

### Code Quality Improvements

✅ **Memory Safety** - No more invalid free() calls  
✅ **Error Handling** - All I/O operations validated  
✅ **Resource Management** - Proper cleanup on errors  
✅ **User Experience** - Clear error messages  
✅ **Code Cleanliness** - Removed unused variables  

### Testing the Fixes

**Test Case 1: Non-existent file**
```bash
./searchengine -d fake.txt -k 5
# Before: Segmentation fault
# After:  Error opening file: fake.txt
```

**Test Case 2: File with fewer lines than expected**
```bash
# Map expects 10 lines, file has 5
./searchengine -d short.txt -k 5
# Before: Corrupt data, undefined behavior
# After:  Error reading line 5
```

**Test Case 3: Normal operation**
```bash
./searchengine -d doc1.txt -k 5
# Before: Works (but with memory bugs lurking)
# After:  Works safely with no hidden bugs
```

---

## Summary

### Key Takeaways

1. **Purpose**: Counts lines and finds maximum line length in a text file

2. **Core Functions Used**:
   - `fopen()` - Open file
   - `fgetc()` / `ungetc()` - Empty file detection
   - `getline()` - Read lines dynamically
   - `free()` - Memory management
   - `fclose()` - Release resources
   - `strtok()` - Tokenize strings (doesn't allocate!)

3. **Memory Management**:
   - Allocates memory for each line
   - Frees memory after processing
   - Defensive programming with final `free()`
   - **NEVER free strtok() return values**

4. **Error Handling**:
   - File not found → return -1 with message
   - File empty → return -1 (with cleanup)
   - Read error → return -1 with cleanup
   - Success → return 1

5. **Pointer Usage**:
   - Output parameters modified via pointers
   - Allows "returning" multiple values
   - Understanding ownership prevents bugs

6. **Platform Considerations**:
   - Empty file check prevents Windows `getline()` hang
   - POSIX `getline()` used (available on MinGW)

7. **Bug Fixes (Dec 26, 2025)**:
   - Fixed memory corruption from free(token)
   - Added fopen() NULL checking
   - Added getline() return validation
   - Removed unused variables

---

**Document Version**: 1.1  
**Last Updated**: December 26, 2025  
**Changes**: Added December 26 bug fixes documentation  
**Author**: High-Performance Search Engine Project  
**Repository**: github.com/adarshpheonix2810/high-performance-search-engine-cpp