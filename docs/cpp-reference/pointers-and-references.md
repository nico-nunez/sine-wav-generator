# Pointers and References

## Table of Contents
- [What is a Pointer?](#what-is-a-pointer)
- [Pointer Syntax](#pointer-syntax)
- [new and delete - Heap Allocation](#new-and-delete---heap-allocation)
- [Stack vs Heap - When to Use Each](#stack-vs-heap---when-to-use-each)
- [Arrow vs Dot Operators](#arrow-vs-dot-operators)
- [Scope Resolution Operator](#scope-resolution-operator)
- [References](#references)
- [Pointers vs References](#pointers-vs-references)
- [nullptr](#nullptr)
- [Common Pointer Pitfalls](#common-pointer-pitfalls)
- [Quick Rules](#quick-rules)

## What is a Pointer?

A **pointer** is a variable that stores a memory address.

```cpp
int x = 42;        // Regular variable - stores the value 42
int* ptr = &x;     // Pointer - stores the ADDRESS of x

std::cout << x;    // Prints: 42
std::cout << ptr;  // Prints: 0x7fff5fbff8ac (memory address)
std::cout << *ptr; // Prints: 42 (dereference - get value at address)
```

**Analogy:** A pointer is like a street address - it tells you WHERE something is, not WHAT it is.

## Pointer Syntax

### Declaration

```cpp
int* ptr;      // Pointer to int
float* fptr;   // Pointer to float
Oscillator* osc;  // Pointer to Oscillator

// All of these are equivalent (style choice):
int* ptr;      // Most common
int *ptr;
int * ptr;
```

### Getting an Address (`&`)

The `&` operator gets the address of a variable:

```cpp
int x = 42;
int* ptr = &x;  // ptr now points to x

// Read as: "ptr is a pointer to int, assigned the address of x"
```

### Dereferencing (`*`)

The `*` operator accesses the value at the address:

```cpp
int x = 42;
int* ptr = &x;

std::cout << *ptr;  // 42 - dereference to get value
*ptr = 100;         // Change value at address
std::cout << x;     // 100 - x has been modified through pointer
```

### The `*` symbol has two meanings:

```cpp
int* ptr;      // Declaration - "ptr is a pointer to int"
int value = *ptr;  // Dereference - "get the value ptr points to"
```

### The `&` symbol has two meanings:

```cpp
int& ref = x;   // Declaration - "ref is a reference to x"
int* ptr = &x;  // Address-of - "get the address of x"
```

Context determines which meaning!

## new and delete - Heap Allocation

### Stack Allocation (No `new`)

```cpp
int x = 42;  // Lives on the stack
Oscillator osc(440.0f);  // Lives on the stack

// Automatically destroyed when out of scope
{
  Oscillator osc(440.0f);
  // use osc...
}  // osc destroyed here automatically
```

**Characteristics:**
- Fast allocation
- Automatic cleanup
- Limited size (~1-8 MB stack)
- Lifetime tied to scope

### Heap Allocation (With `new`)

```cpp
int* ptr = new int(42);  // Lives on the heap
Oscillator* osc = new Oscillator(440.0f);  // Lives on the heap

// Must manually delete
delete ptr;
delete osc;

// Or use smart pointers (modern C++)
std::unique_ptr<Oscillator> osc = std::make_unique<Oscillator>(440.0f);
// Automatically deleted when unique_ptr goes out of scope
```

**Characteristics:**
- Slower allocation
- Manual cleanup required (or use smart pointers)
- Large available space (GBs)
- Lifetime until you `delete` it

### Array Allocation

```cpp
// Stack array
int arr[100];  // Size must be known at compile time

// Heap array
int* arr = new int[size];  // Size can be runtime variable
delete[] arr;  // Note: delete[] for arrays, not delete

// Modern C++ - use std::vector instead
std::vector<int> arr(size);  // Automatic cleanup
```

**Critical:** Use `delete[]` for arrays, `delete` for single objects!

```cpp
int* single = new int(42);
delete single;  // Correct

int* array = new int[10];
delete[] array;  // Correct - note the []

// WRONG:
delete array;    // Memory leak!
delete[] single; // Undefined behavior!
```

### Memory Leaks

```cpp
// BAD - memory leak
void bad() {
  int* ptr = new int(42);
  // forgot to delete - memory leaked!
}

// GOOD - deleted
void good() {
  int* ptr = new int(42);
  delete ptr;  // Memory freed
}

// BEST - automatic cleanup
void best() {
  std::unique_ptr<int> ptr = std::make_unique<int>(42);
  // Automatically deleted when out of scope
}
```

**Modern C++ rule:** Prefer smart pointers (`std::unique_ptr`, `std::shared_ptr`) over raw `new`/`delete`.

## Stack vs Heap - When to Use Each

### Use Stack (No Pointer) When:

```cpp
// 1. You know the type and lifetime
void processAudio() {
  Oscillator osc(440.0f);  // Stack - used only in this function
  // use osc...
}  // Automatically cleaned up

// 2. Size is known at compile time
int buffer[512];

// 3. Small objects
struct Point { float x, y; };
Point p{1.0f, 2.0f};
```

### Use Heap (Pointer) When:

```cpp
// 1. Need polymorphism (see polymorphism.md)
std::vector<Oscillator*> oscillators;
oscillators.push_back(new SineOscillator());
oscillators.push_back(new SquareOscillator());

// 2. Lifetime extends beyond current scope
Oscillator* createOscillator() {
  return new Oscillator(440.0f);  // Must return pointer - outlives function
}

// 3. Large objects
float* bigBuffer = new float[10000000];  // Too big for stack

// 4. Size not known at compile time
int size = getUserInput();
float* buffer = new float[size];  // Runtime size
```

### Your Current Oscillator - No Pointer Needed!

```cpp
// If you're using it like this, no pointer needed:
Oscillator osc(440.0f);
for (int i = 0; i < numSamples; ++i) {
  buffer[i] = osc.getNextSampleValue();
}
// Perfect! Stack allocation is simpler and safer.
```

## Arrow vs Dot Operators

### Dot (`.`) - For Objects

```cpp
Oscillator osc(440.0f);  // Object on stack
osc.setFrequency(880.0f);  // Use dot
float freq = osc.getFrequency();
```

### Arrow (`->`) - For Pointers

```cpp
Oscillator* ptr = new Oscillator(440.0f);  // Pointer to heap object
ptr->setFrequency(880.0f);  // Use arrow
float freq = ptr->getFrequency();
delete ptr;
```

### Arrow is Shorthand for Dereference + Dot

```cpp
Oscillator* ptr = new Oscillator(440.0f);

ptr->setFrequency(880.0f);     // Arrow operator
(*ptr).setFrequency(880.0f);   // Same thing - dereference then dot

// Arrow is much cleaner!
```

### Quick Reference

```cpp
// Stack object - use dot
Oscillator osc;
osc.method();

// Heap pointer - use arrow
Oscillator* ptr = new Oscillator();
ptr->method();

// Reference - use dot (acts like object)
Oscillator& ref = osc;
ref.method();
```

## Scope Resolution Operator

The `::` operator is completely different - it's for accessing class/namespace members, NOT instance members.

### Namespace Access

```cpp
std::vector<int> v;  // std::vector from std namespace
std::sin(1.0f);      // std::sin from std namespace

Synth::Oscillator osc;  // Oscillator from Synth namespace
```

### Static Member Access

```cpp
class Oscillator {
public:
  static float getMaxFrequency() { return 20000.0f; }
};

float max = Oscillator::getMaxFrequency();  // Call static method
// No instance needed - :: accesses class member
```

### Implementation Scope

```cpp
// In .cpp file
float Oscillator::getFrequency() const {  // "getFrequency that belongs to Oscillator"
  return m_frequency;
}
```

### Why NOT `foo::method()`?

```cpp
Oscillator osc;
osc::getFrequency();  // ERROR - osc is a variable, not a class/namespace

// Correct:
osc.getFrequency();   // Dot - for instances
Oscillator::staticMethod();  // :: - for class/namespace
```

## References

A **reference** is an alias - another name for an existing variable.

### Basic Syntax

```cpp
int x = 42;
int& ref = x;  // ref is a reference to x (another name for x)

ref = 100;     // Changes x through ref
std::cout << x;  // 100 - x was modified

// ref and x are THE SAME variable
```

### Must Be Initialized

```cpp
int& ref;  // ERROR - reference must be initialized
int& ref = x;  // OK
```

### Cannot Be Reseated

```cpp
int x = 42;
int y = 100;

int& ref = x;  // ref refers to x
ref = y;       // Sets x = y (doesn't make ref refer to y!)

std::cout << x;  // 100 - x was changed
std::cout << ref;  // 100 - ref still refers to x
```

### Common Use: Function Parameters

```cpp
// Pass by value - makes a copy
void modify(Oscillator osc) {
  osc.setFrequency(880.0f);  // Modifies the COPY
}

// Pass by reference - no copy, can modify original
void modify(Oscillator& osc) {
  osc.setFrequency(880.0f);  // Modifies the ORIGINAL
}

// Pass by const reference - no copy, read-only
void print(const Oscillator& osc) {
  std::cout << osc.getFrequency();  // Read only, no copy
}
```

## Pointers vs References

| Feature | Pointer | Reference |
|---------|---------|-----------|
| **Nullable** | Yes (`nullptr`) | No (must refer to something) |
| **Reassignable** | Yes | No (always refers to same variable) |
| **Syntax** | `*` and `->` | Like regular variable (`.`) |
| **Can be array** | Yes (`int* arr`) | No |
| **Common use** | Dynamic memory, polymorphism | Function parameters |

### When to Use Each

**Use references for:**
- Function parameters (avoid copies)
- Return values (avoid copies)
- Aliases in local scope

```cpp
void process(const Oscillator& osc) {  // Reference - no copy
  // ...
}

Oscillator& getOscillator() {  // Return reference to existing object
  return m_oscillator;
}
```

**Use pointers for:**
- Dynamic memory (`new`/`delete`)
- Polymorphism (base class pointer)
- Optional values (can be `nullptr`)
- Need to reassign

```cpp
Oscillator* osc = new Oscillator();  // Dynamic allocation
delete osc;

std::vector<Oscillator*> oscillators;  // Polymorphism

Oscillator* findOscillator() {  // Can return nullptr if not found
  if (found) return ptr;
  return nullptr;
}
```

## nullptr

`nullptr` is the modern C++ way to represent "no pointer" (replaces old `NULL` or `0`).

```cpp
int* ptr = nullptr;  // Pointer to nothing

if (ptr == nullptr) {
  std::cout << "No pointer";
}

// Dereferencing nullptr is undefined behavior!
*ptr = 42;  // CRASH!
```

### Checking Before Use

```cpp
Oscillator* findOscillator(int id) {
  if (found) return ptr;
  return nullptr;  // Not found
}

Oscillator* osc = findOscillator(42);
if (osc != nullptr) {  // Always check!
  osc->process();
} else {
  std::cout << "Not found";
}

// Or shorter:
if (osc) {  // nullptr is false
  osc->process();
}
```

## Common Pointer Pitfalls

### 1. Dangling Pointer

```cpp
// BAD - pointer to destroyed object
int* getDanglingPointer() {
  int x = 42;
  return &x;  // x destroyed when function returns!
}

int* ptr = getDanglingPointer();
*ptr = 100;  // UNDEFINED BEHAVIOR - x is gone!
```

### 2. Memory Leak

```cpp
// BAD - forgot to delete
void leak() {
  int* ptr = new int(42);
  // forgot delete - memory leaked!
}

// GOOD - deleted
void good() {
  int* ptr = new int(42);
  delete ptr;
}

// BEST - automatic cleanup
void best() {
  std::unique_ptr<int> ptr = std::make_unique<int>(42);
}
```

### 3. Double Delete

```cpp
// BAD - deleting twice
int* ptr = new int(42);
delete ptr;
delete ptr;  // UNDEFINED BEHAVIOR!

// Solution: set to nullptr after delete
delete ptr;
ptr = nullptr;
delete ptr;  // Safe - deleting nullptr is no-op
```

### 4. Using After Delete

```cpp
// BAD - use after delete
int* ptr = new int(42);
delete ptr;
*ptr = 100;  // UNDEFINED BEHAVIOR!

// Solution: don't use after delete, or set to nullptr
delete ptr;
ptr = nullptr;
if (ptr) {  // Won't enter - ptr is nullptr
  *ptr = 100;
}
```

### 5. Wrong delete for Arrays

```cpp
// BAD - wrong delete
int* arr = new int[10];
delete arr;  // Should be delete[]

// GOOD
int* arr = new int[10];
delete[] arr;

// BEST - use std::vector
std::vector<int> arr(10);  // Automatic cleanup
```

## Quick Rules

1. **`*` in declaration** - "pointer to"
2. **`*` in code** - "dereference" (get value at address)
3. **`&` in declaration** - "reference to"
4. **`&` in code** - "address of"
5. **`.` for objects** - `osc.method()`
6. **`->` for pointers** - `ptr->method()`
7. **`::` for class/namespace** - `Oscillator::staticMethod()`
8. **`new`** allocates on heap - must `delete`
9. **`delete`** for single objects, **`delete[]`** for arrays
10. **Stack is default** - only use heap when you need it
11. **Prefer smart pointers** over raw `new`/`delete`
12. **Always check** for `nullptr` before dereferencing

**Modern C++ tip:** Avoid raw pointers for ownership. Use:
- `std::unique_ptr` for single ownership
- `std::shared_ptr` for shared ownership
- Raw pointers only for non-owning references
