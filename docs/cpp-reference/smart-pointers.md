# Smart Pointers

## Table of Contents
- [What Are Smart Pointers?](#what-are-smart-pointers)
- [Ownership Semantics](#ownership-semantics)
- [std::unique_ptr - Single Ownership](#stdunique_ptr---single-ownership)
- [std::shared_ptr - Shared Ownership](#stdshared_ptr---shared-ownership)
- [std::weak_ptr - Non-Owning Observer](#stdweak_ptr---non-owning-observer)
- [Performance Comparison](#performance-comparison)
- [Converting from Raw Pointers](#converting-from-raw-pointers)
- [Common Patterns](#common-patterns)
- [When to Use Each](#when-to-use-each)
- [Quick Rules](#quick-rules)

## What Are Smart Pointers?

**Smart pointers** are wrapper classes that manage raw pointers automatically. They call `delete` for you when the pointer is no longer needed.

### The Problem with Raw Pointers

```cpp
void processAudio() {
  Oscillator* osc = new Oscillator(440.0f);

  if (errorCondition) {
    return;  // MEMORY LEAK - forgot to delete!
  }

  osc->process();
  delete osc;  // Must remember to delete
}
```

### The Solution: Smart Pointers

```cpp
void processAudio() {
  std::unique_ptr<Oscillator> osc = std::make_unique<Oscillator>(440.0f);

  if (errorCondition) {
    return;  // OK - automatically deleted when out of scope
  }

  osc->process();
  // Automatically deleted here when function ends
}
```

**Key benefits:**
- Automatic memory management (no manual `delete`)
- Exception-safe (deleted even if exception thrown)
- Clear ownership semantics
- Zero or minimal overhead

**Include:**
```cpp
#include <memory>  // For smart pointers
```

## Ownership Semantics

**Ownership** = who is responsible for deleting an object.

### The Fundamental Rule

**Only `delete` what was created with `new`, and only delete it ONCE.**

```cpp
// Heap - can delete
Oscillator* heap = new Oscillator(440.0f);
delete heap;  // OK - created with new

// Stack - CANNOT delete
Oscillator stack(440.0f);
delete &stack;  // UNDEFINED BEHAVIOR - stack object!
```

### Ownership with Raw Pointers (Unclear!)

```cpp
// Who deletes this?
void process(Oscillator* osc) {
  osc->getNextSample();
  // Should I delete? Not clear from signature!
}

// Who deletes this?
Oscillator* createOscillator() {
  return new Oscillator(440.0f);  // Caller must delete, but not obvious
}

// Convention (not enforced):
// - Parameter pointer = non-owning (don't delete)
// - Return pointer = owning (you must delete)
// But easy to forget!
```

### Ownership with Smart Pointers (Clear!)

```cpp
// Clearly non-owning - just observing
void process(Oscillator* osc) {  // Raw pointer = non-owning
  osc->getNextSample();
}

// Or even clearer:
void process(const Oscillator& osc) {  // Reference = non-owning
  osc.getNextSample();
}

// Clearly transfers ownership
std::unique_ptr<Oscillator> createOscillator() {
  return std::make_unique<Oscillator>(440.0f);  // Ownership transferred to caller
}

// Clearly shares ownership
std::shared_ptr<Oscillator> getSharedOscillator() {
  return m_oscillator;  // Shared ownership - ref count increases
}
```

### Modern C++ Ownership Guidelines

1. **Use `std::unique_ptr`** for single ownership (most common)
2. **Use `std::shared_ptr`** for shared ownership (less common)
3. **Use raw pointers or references** for non-owning access
4. **Avoid manual `new`/`delete`** in application code

```cpp
class Synth {
public:
  // Owns the oscillator
  void setOscillator(std::unique_ptr<Oscillator> osc) {
    m_oscillator = std::move(osc);  // Takes ownership
  }

  // Non-owning access
  void process(const Oscillator& osc) {
    // Just using it, not owning it
  }

  // Non-owning access (nullable)
  void processIfExists(Oscillator* osc) {
    if (osc) {
      // Just using it, not owning it
    }
  }

private:
  std::unique_ptr<Oscillator> m_oscillator;  // Owns
};
```

## std::unique_ptr - Single Ownership

**One owner, automatic deletion, zero overhead.**

### Basic Usage

```cpp
#include <memory>

// Create with make_unique (preferred)
std::unique_ptr<Oscillator> osc = std::make_unique<Oscillator>(440.0f);

// Use just like a raw pointer
osc->setFrequency(880.0f);
float sample = osc->getNextSample();

// Access raw pointer if needed
Oscillator* raw = osc.get();

// Automatically deleted when osc goes out of scope
```

### Why make_unique?

```cpp
// Good - one allocation
auto osc = std::make_unique<Oscillator>(440.0f);

// Less good - two operations (new, then wrap in unique_ptr)
std::unique_ptr<Oscillator> osc(new Oscillator(440.0f));

// Also exception-safe with multiple arguments:
riskyFunction(std::make_unique<Oscillator>(440.0f),  // Safe
              std::make_unique<Filter>(1000.0f));     // Safe

// Unsafe with raw new (if first succeeds but second throws, leak!)
riskyFunction(std::unique_ptr<Oscillator>(new Oscillator(440.0f)),
              std::unique_ptr<Filter>(new Filter(1000.0f)));
```

### Ownership Transfer (Move)

`std::unique_ptr` cannot be copied - only moved.

```cpp
auto osc1 = std::make_unique<Oscillator>(440.0f);

// auto osc2 = osc1;  // ERROR - can't copy unique_ptr

auto osc2 = std::move(osc1);  // OK - transfers ownership
// osc1 is now nullptr
// osc2 owns the Oscillator
```

### Returning unique_ptr

```cpp
std::unique_ptr<Oscillator> createOscillator(std::string type) {
  if (type == "sine") {
    return std::make_unique<SineOscillator>(440.0f);
  } else if (type == "square") {
    return std::make_unique<SquareOscillator>(440.0f);
  }
  return nullptr;  // No oscillator
}

// Caller receives ownership
auto osc = createOscillator("sine");
if (osc) {  // Check if nullptr
  osc->process();
}
// Automatically deleted when osc goes out of scope
```

### Passing unique_ptr to Functions

```cpp
// 1. Transfer ownership (move)
void takeOwnership(std::unique_ptr<Oscillator> osc) {
  // Function now owns osc
  // Will be deleted when function ends
}

auto osc = std::make_unique<Oscillator>(440.0f);
takeOwnership(std::move(osc));  // Must use std::move
// osc is now nullptr

// 2. Non-owning access (just use it, don't take ownership)
void useOscillator(Oscillator* osc) {  // Raw pointer for non-owning
  osc->process();
  // Doesn't delete - caller still owns
}

auto osc = std::make_unique<Oscillator>(440.0f);
useOscillator(osc.get());  // Get raw pointer
// osc still valid, still owned by caller

// 3. Non-owning access (reference - preferred)
void useOscillator(const Oscillator& osc) {  // Reference for non-owning
  osc.getFrequency();
}

useOscillator(*osc);  // Dereference to get reference
```

### unique_ptr with Arrays

```cpp
// For arrays, use std::vector instead (preferred)
std::vector<float> buffer(1024);

// But if you must use unique_ptr for arrays:
std::unique_ptr<float[]> buffer = std::make_unique<float[]>(1024);
buffer[0] = 1.0f;  // Array access
// Automatically calls delete[] (not delete)
```

### Releasing Ownership

```cpp
auto smart = std::make_unique<Oscillator>(440.0f);

// Transfer to raw pointer (lose automatic deletion!)
Oscillator* raw = smart.release();  // smart is now nullptr
delete raw;  // Must manually delete now

// Usually only needed for C API interop
```

## std::shared_ptr - Shared Ownership

**Multiple owners, reference counting, small overhead.**

### Basic Usage

```cpp
#include <memory>

// Create with make_shared (preferred)
std::shared_ptr<Oscillator> osc1 = std::make_shared<Oscillator>(440.0f);

// Copy creates another owner (ref count = 2)
std::shared_ptr<Oscillator> osc2 = osc1;

// Both point to same object
osc1->setFrequency(880.0f);
std::cout << osc2->getFrequency();  // 880.0f

// Object deleted when LAST owner goes out of scope
{
  auto osc3 = osc1;  // ref count = 3
}  // ref count = 2

// osc1 and osc2 still valid
```

### Reference Counting

```cpp
auto osc = std::make_shared<Oscillator>(440.0f);

std::cout << osc.use_count();  // 1 owner

{
  auto copy1 = osc;  // ref count = 2
  auto copy2 = osc;  // ref count = 3

  std::cout << osc.use_count();  // 3
}  // copy1 and copy2 destroyed, ref count = 1

std::cout << osc.use_count();  // 1
```

### When to Use shared_ptr

```cpp
// Multiple objects need access to same resource
class Voice {
public:
  Voice(std::shared_ptr<Wavetable> wavetable)
    : m_wavetable(wavetable) {}  // Shares ownership

private:
  std::shared_ptr<Wavetable> m_wavetable;
};

// Multiple voices share the same wavetable
auto wavetable = std::make_shared<Wavetable>();
Voice voice1(wavetable);  // ref count = 2
Voice voice2(wavetable);  // ref count = 3
Voice voice3(wavetable);  // ref count = 4
// Wavetable deleted when all voices are destroyed

// Callback storage
std::vector<std::shared_ptr<Listener>> listeners;
listeners.push_back(listenerPtr);  // Shared ownership
```

### Performance Cost

```cpp
// Each copy increments/decrements atomic counter
for (int i = 0; i < 1000000; ++i) {
  auto copy = sharedPtr;  // Atomic increment - small cost
}  // Atomic decrement on each destruction

// In tight loops, prefer references:
void process(const std::shared_ptr<Oscillator>& osc) {  // No ref count change
  osc->getNextSample();
}
```

## std::weak_ptr - Non-Owning Observer

**Observes `shared_ptr` without affecting ref count. Avoids circular references.**

### The Circular Reference Problem

```cpp
// BAD - circular reference (memory leak!)
class Node {
  std::shared_ptr<Node> parent;    // Parent owns child
  std::shared_ptr<Node> child;     // Child owns parent
  // Neither can be deleted - ref count never reaches 0!
};

// GOOD - break cycle with weak_ptr
class Node {
  std::weak_ptr<Node> parent;      // Doesn't own parent
  std::shared_ptr<Node> child;     // Owns child
};
```

### Using weak_ptr

```cpp
auto shared = std::make_shared<Oscillator>(440.0f);
std::weak_ptr<Oscillator> weak = shared;  // Observe, don't own

std::cout << weak.use_count();  // 1 (only shared owns it)

// Must lock to access (converts to shared_ptr temporarily)
if (auto locked = weak.lock()) {  // Returns shared_ptr if still alive
  locked->process();
} else {
  std::cout << "Object was deleted";
}

// After shared destroyed:
shared.reset();  // Delete the object
if (auto locked = weak.lock()) {
  // Won't enter - object is gone
}
```

### Common Use Case: Caching

```cpp
class ResourceCache {
public:
  std::shared_ptr<Wavetable> getWavetable(std::string name) {
    if (auto cached = m_cache[name].lock()) {
      return cached;  // Still in memory
    }

    // Load and cache
    auto wavetable = std::make_shared<Wavetable>(name);
    m_cache[name] = wavetable;  // weak_ptr - doesn't prevent deletion
    return wavetable;
  }

private:
  std::map<std::string, std::weak_ptr<Wavetable>> m_cache;
  // Cached resources can be deleted when not in use
};
```

## Performance Comparison

### Memory Overhead

```cpp
// Raw pointer: sizeof(void*) = 8 bytes on 64-bit
Oscillator* raw;

// unique_ptr: sizeof(void*) = 8 bytes (same as raw!)
std::unique_ptr<Oscillator> unique;

// shared_ptr: sizeof(void*) * 2 = 16 bytes (pointer + control block pointer)
std::shared_ptr<Oscillator> shared;

// Control block overhead:
// - 2 atomic counters (shared count + weak count) = ~8 bytes
// - Virtual destructor pointer = ~8 bytes
// Total: ~24 bytes overhead per shared object
```

### Runtime Performance

| Operation | unique_ptr | shared_ptr | Raw Pointer |
|-----------|------------|------------|-------------|
| **Dereference** | Same as raw | Same as raw | Baseline |
| **Copy** | N/A (move only) | Atomic increment | Simple copy |
| **Destroy** | Destructor call | Atomic decrement + check | Manual delete |
| **Overhead** | **Zero** | **Small** (atomic ops) | Zero |

### Benchmark Example

```cpp
// Negligible difference in most code
void process(std::unique_ptr<Oscillator>& osc) {
  for (int i = 0; i < 1000; ++i) {
    buffer[i] = osc->getNextSample();  // Same as raw pointer
  }
}

// Copying shared_ptr in tight loop - avoid this
void bad(std::shared_ptr<Oscillator> osc) {  // Copy on every call!
  // Atomic increment + decrement per call
}

// Better - pass by reference
void good(const std::shared_ptr<Oscillator>& osc) {  // No copy
  // No ref count change
}

// Best - use raw pointer or reference for non-owning access
void best(Oscillator& osc) {  // Zero overhead
  osc.getNextSample();
}
```

## Converting from Raw Pointers

### Before (Raw Pointers)

```cpp
class Synth {
public:
  Synth() {
    m_osc = new Oscillator(440.0f);
  }

  ~Synth() {
    delete m_osc;  // Must remember!
  }

  // Can't copy safely (double delete!)
  Synth(const Synth&) = delete;
  Synth& operator=(const Synth&) = delete;

private:
  Oscillator* m_osc;
};
```

### After (Smart Pointers)

```cpp
class Synth {
public:
  Synth() {
    m_osc = std::make_unique<Oscillator>(440.0f);
  }

  // No destructor needed - automatic cleanup!
  // Compiler-generated copy/move work correctly

private:
  std::unique_ptr<Oscillator> m_osc;
};
```

### Polymorphism with Smart Pointers

```cpp
// Before
std::vector<Oscillator*> oscillators;
oscillators.push_back(new SineOscillator());
oscillators.push_back(new SquareOscillator());

for (auto* osc : oscillators) {
  delete osc;  // Must remember to delete each one
}
oscillators.clear();

// After
std::vector<std::unique_ptr<Oscillator>> oscillators;
oscillators.push_back(std::make_unique<SineOscillator>());
oscillators.push_back(std::make_unique<SquareOscillator>());

// Automatically deleted when vector is destroyed or cleared!
```

## Common Patterns

### Factory Pattern

```cpp
// Returns ownership to caller
std::unique_ptr<Oscillator> createOscillator(std::string type) {
  if (type == "sine") return std::make_unique<SineOscillator>();
  if (type == "square") return std::make_unique<SquareOscillator>();
  if (type == "saw") return std::make_unique<SawOscillator>();
  return nullptr;
}

auto osc = createOscillator("sine");
if (osc) {
  osc->process();
}
```

### PIMPL (Pointer to Implementation)

```cpp
// Header - hide implementation details
class Synth {
public:
  Synth();
  ~Synth();  // Must declare (unique_ptr needs complete type to delete)

  void process();

private:
  class Impl;  // Forward declaration
  std::unique_ptr<Impl> m_impl;  // Hides implementation
};

// Implementation
class Synth::Impl {
  // Complex implementation details
  std::vector<Oscillator> oscillators;
  Filter filter;
};

Synth::Synth() : m_impl(std::make_unique<Impl>()) {}
Synth::~Synth() = default;  // Must define in .cpp where Impl is complete
```

### Resource Management

```cpp
class AudioEngine {
public:
  void setOscillator(std::unique_ptr<Oscillator> osc) {
    m_oscillator = std::move(osc);  // Take ownership
  }

  Oscillator* getOscillator() {  // Non-owning access
    return m_oscillator.get();
  }

  std::unique_ptr<Oscillator> releaseOscillator() {  // Transfer ownership out
    return std::move(m_oscillator);  // Caller now owns it
  }

private:
  std::unique_ptr<Oscillator> m_oscillator;
};
```

## When to Use Each

### Use `std::unique_ptr` when:
- ✓ Single, clear ownership (95% of cases)
- ✓ Replacing raw `new`/`delete`
- ✓ Class members that own resources
- ✓ Factory functions returning ownership
- ✓ Zero overhead required

```cpp
std::unique_ptr<Oscillator> m_oscillator;  // Synth owns oscillator
```

### Use `std::shared_ptr` when:
- ✓ Multiple owners needed (rare)
- ✓ Shared resources (wavetables, sample buffers)
- ✓ Callbacks that outlive the creator
- ✓ Object lifetime unclear

```cpp
std::shared_ptr<Wavetable> m_wavetable;  // Multiple voices share
```

### Use `std::weak_ptr` when:
- ✓ Observing without owning
- ✓ Breaking circular references
- ✓ Caching (don't prevent deletion)

```cpp
std::weak_ptr<Node> m_parent;  // Observe parent, don't own
```

### Use raw pointers when:
- ✓ Non-owning access (function parameters)
- ✓ Nullable references
- ✓ C API interop
- ✗ NOT for ownership

```cpp
void process(Oscillator* osc) {  // Just using it, not owning
  if (osc) osc->process();
}
```

### Use references when:
- ✓ Non-owning, non-null access (preferred)
- ✓ Function parameters

```cpp
void process(const Oscillator& osc) {  // Non-owning, guaranteed non-null
  osc.getFrequency();
}
```

## Quick Rules

1. **Default to `std::unique_ptr`** for ownership
2. **Use `std::make_unique`** and `std::make_shared`** (not raw `new`)
3. **`unique_ptr` has zero overhead** - use freely
4. **`shared_ptr` has small overhead** - use when needed
5. **Pass by reference** for non-owning access (not by value)
6. **Raw pointers = non-owning** in modern C++
7. **Avoid manual `new`/`delete`** in application code
8. **`std::move` transfers ownership** from `unique_ptr`
9. **Can't copy `unique_ptr`** - only move
10. **Use `weak_ptr`** to break circular references

**Modern C++ guideline:** If you write `delete` in application code, something is wrong. Smart pointers handle it automatically.

**Performance tip:** In real-time audio code, create smart pointers during setup/initialization, then use raw pointers or references during processing for zero overhead.
