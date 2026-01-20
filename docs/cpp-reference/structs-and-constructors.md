# Structs and Constructors in C++

## Table of Contents
- [struct vs class](#struct-vs-class)
- [Convention](#convention)
- [C++ Terminology](#c-terminology)
- [Constructors](#constructors)
- [Default Constructor](#default-constructor)
- [Methods in Structs](#methods-in-structs)
- [Rule of Zero/Five](#rule-of-zerofive)
- [Common Gotchas](#common-gotchas)
- [Quick Reference](#quick-reference)

## struct vs class

**The only difference**: default access level
- `struct`: members are **public** by default
- `class`: members are **private** by default

Everything else is identical - both can have constructors, methods, inheritance, etc.

```cpp
struct MyStruct {
  int x;  // public by default
};

class MyClass {
  int x;  // private by default
};
```

## Convention

- **struct**: Plain data structures, mostly public members
- **class**: When you need encapsulation/private implementation

## C++ Terminology

In C++, use these terms:
- **Member variable** / **Data member** = variables in a class/struct (not "property")
- **Member function** / **Method** = functions in a class/struct
- **Member** = general term for both

```cpp
struct Oscillator {
  float frequency;  // Member variable / data member

  void reset() {    // Member function / method
    frequency = 0.0f;
  }
};
```

## Constructors

Constructors are special functions that initialize objects. They have the same name as the struct/class and no return type.

### Basic Constructor

```cpp
struct Oscillator {
  float frequency;
  float phaseValue;

  // Constructor
  Oscillator(float freq) {
    frequency = freq;
    phaseValue = 0.0f;
  }
};

// Usage
Oscillator myOsc(440.0f);
```

### Member Initializer List (Preferred)

The `: member(value), ...` syntax after the parameter list:

```cpp
struct Oscillator {
  float frequency;
  float phaseValue;
  float phaseIncrement;

  Oscillator(float freq, float sampleRate)
    : frequency(freq),           // Initialize frequency
      phaseValue(0.0f),          // Initialize phaseValue
      phaseIncrement(freq / sampleRate)  // Calculate and initialize
  {
    // Body can be empty or contain additional logic
  }
};
```

**Why use initializer lists?**
- More efficient (direct initialization vs assignment)
- Required for `const` members and references
- Clearer intent

**Important**: Members are initialized in **declaration order**, not the order in the initializer list.

### Multiple Constructors

You can have multiple constructors (overloading):

```cpp
struct Oscillator {
  float frequency;
  float phaseValue;

  // Default constructor
  Oscillator() : frequency(440.0f), phaseValue(0.0f) {}

  // Constructor with frequency
  Oscillator(float freq) : frequency(freq), phaseValue(0.0f) {}

  // Constructor with both
  Oscillator(float freq, float phase) : frequency(freq), phaseValue(phase) {}
};

Oscillator osc1;              // Uses default constructor
Oscillator osc2(220.0f);      // Uses second constructor
Oscillator osc3(330.0f, 0.5f); // Uses third constructor
```

### Optional Constructor Parameters

Use default parameter values for optional parameters:

```cpp
struct Oscillator {
  float frequency;
  float phaseValue;
  float phaseIncrement;

  // sampleRate has a default value
  Oscillator(float freq, float sampleRate = 48000.0f)
    : frequency(freq),
      phaseValue(0.0f),
      phaseIncrement(freq / sampleRate)
  {}
};

Oscillator osc1(440.0f);           // Uses default sampleRate (48000.0f)
Oscillator osc2(440.0f, 44100.0f); // Overrides sampleRate
```

**Rules:**
- Default parameters must be at the **end** of the parameter list
- Can't have required params after optional ones

```cpp
// Good
Oscillator(float freq, float sr = 48000.0f, float phase = 0.0f);

// Bad - required param after optional
Oscillator(float freq = 440.0f, float sr);  // Error!
```

**Gotcha:** You can't skip middle parameters:

```cpp
Oscillator osc(220.0f, /* can't skip sampleRate */, 0.8f);  // Not possible!
```

If you need that flexibility, use multiple constructor overloads instead.

### Default Member Initializers (C++11+)

You can provide default values directly on member variables:

```cpp
struct Oscillator {
  float frequency = 440.0f;
  float phaseValue = 0.0f;
  float phaseIncrement = 0.0f;

  // Constructor only needs to set what's different
  Oscillator(float freq, float sampleRate)
    : frequency(freq),
      phaseIncrement(freq / sampleRate)
  {
    // phaseValue uses default (0.0f)
  }
};
```

**Default constructor gets initialized values automatically:**

```cpp
struct Oscillator {
  float frequency = 440.0f;
  float phaseValue = 0.0f;
};

Oscillator osc;  // frequency is 440.0f, phaseValue is 0.0f
```

**When to use default member initializers vs constructor:**
- **Simple constants** → Default member initializers (`float gain = 1.0f;`)
- **Zero/null values** → Default member initializers (`float phase = 0.0f;`)
- **Calculated from parameters** → Constructor initializer list
- **Multiple constructors with same defaults** → Default member initializers

**Best of both:**

```cpp
struct Oscillator {
  // Defaults for simple values
  float phaseValue = 0.0f;
  float gain = 1.0f;

  // Set in constructor
  float frequency;
  float sampleRate;
  float phaseIncrement;

  Oscillator(float freq, float sr)
    : frequency(freq),
      sampleRate(sr),
      phaseIncrement(freq / sr)
  {
    // phaseValue and gain use their defaults
  }
};
```

## Default Constructor

If you don't write any constructors, C++ provides a default constructor that does nothing (leaves members uninitialized for primitive types).

```cpp
struct Point {
  float x;
  float y;
};

Point p;  // x and y are UNINITIALIZED (garbage values)
```

**Gotcha**: If you write *any* constructor, the default constructor is no longer auto-generated.

```cpp
struct Point {
  float x;
  float y;

  Point(float x_, float y_) : x(x_), y(y_) {}
};

Point p;  // ERROR! No default constructor available
Point p(1.0f, 2.0f);  // OK
```

To keep it, explicitly define it:

```cpp
struct Point {
  float x;
  float y;

  Point() : x(0.0f), y(0.0f) {}  // Default constructor
  Point(float x_, float y_) : x(x_), y(y_) {}
};
```

Or use `= default`:

```cpp
struct Point {
  float x = 0.0f;
  float y = 0.0f;

  Point() = default;  // Use compiler-generated default constructor
  Point(float x_, float y_) : x(x_), y(y_) {}
};
```

## Methods in Structs

Structs can have methods just like classes:

```cpp
struct Oscillator {
  float frequency;
  float phaseValue;

  Oscillator(float freq) : frequency(freq), phaseValue(0.0f) {}

  // Method
  void reset() {
    phaseValue = 0.0f;
  }

  // Method that returns a value
  float getCurrentPhase() const {  // 'const' means it doesn't modify the object
    return phaseValue;
  }
};

Oscillator osc(440.0f);
osc.reset();
float phase = osc.getCurrentPhase();
```

## Rule of Zero/Five

C++ has **five special member functions** that control how objects are created, copied, moved, and destroyed. Understanding when to use them is critical.

### The Five Special Member Functions

```cpp
struct Example {
    // 1. Destructor
    ~Example() { /* cleanup */ }

    // 2. Copy constructor
    Example(const Example& other) { /* copy */ }

    // 3. Copy assignment operator
    Example& operator=(const Example& other) { /* copy assign */ return *this; }

    // 4. Move constructor (C++11)
    Example(Example&& other) noexcept { /* move */ }

    // 5. Move assignment operator (C++11)
    Example& operator=(Example&& other) noexcept { /* move assign */ return *this; }
};
```

### Rule of Zero (Preferred)

**Don't declare ANY of the five special member functions. Let the compiler generate them.**

```cpp
// Rule of Zero - Simple and correct
struct Settings {
    float attack;
    float decay;
    float sustain;
    float release;

    // No special member functions declared
    // Compiler generates correct copy/move/destructor
};

Settings s1;
Settings s2 = s1;       // Compiler-generated copy works perfectly
Settings s3 = std::move(s1);  // Compiler-generated move works perfectly
```

**When to use Rule of Zero:**
- ✅ Simple data structures (POD types)
- ✅ All members have value semantics
- ✅ No raw pointers, file handles, or other resources
- ✅ **95% of the time** - this is the default choice

**Why it works:** Modern C++ types (std::vector, std::unique_ptr, etc.) already handle their own copying/moving correctly. Compiler-generated functions do the right thing automatically.

### Rule of Five (When You Need It)

**If you declare ANY of the five, declare ALL five (or explicitly delete/default them).**

```cpp
// Rule of Five - Managing a resource
class AudioBuffer {
    float* mData;
    size_t mSize;

public:
    // Constructor
    AudioBuffer(size_t size) : mSize(size), mData(new float[size]) {}

    // 1. Destructor - MUST clean up resource
    ~AudioBuffer() {
        delete[] mData;
    }

    // 2. Copy constructor - deep copy
    AudioBuffer(const AudioBuffer& other)
        : mSize(other.mSize)
        , mData(new float[other.mSize])
    {
        std::copy(other.mData, other.mData + mSize, mData);
    }

    // 3. Copy assignment
    AudioBuffer& operator=(const AudioBuffer& other) {
        if (this != &other) {
            delete[] mData;  // Clean up old resource
            mSize = other.mSize;
            mData = new float[mSize];
            std::copy(other.mData, other.mData + mSize, mData);
        }
        return *this;
    }

    // 4. Move constructor - steal resource
    AudioBuffer(AudioBuffer&& other) noexcept
        : mSize(other.mSize)
        , mData(other.mData)
    {
        other.mData = nullptr;  // Leave other in valid state
        other.mSize = 0;
    }

    // 5. Move assignment
    AudioBuffer& operator=(AudioBuffer&& other) noexcept {
        if (this != &other) {
            delete[] mData;  // Clean up old resource
            mData = other.mData;
            mSize = other.mSize;
            other.mData = nullptr;  // Leave other in valid state
            other.mSize = 0;
        }
        return *this;
    }
};
```

**When to use Rule of Five:**
- You manage a **resource** (raw pointer, file handle, socket, etc.)
- You need custom cleanup in the destructor
- You need deep copies (copying pointed-to data, not just the pointer)

**Better approach:** Use Rule of Zero by wrapping resources in RAII types:

```cpp
// Better - Rule of Zero with smart pointers
class AudioBuffer {
    std::unique_ptr<float[]> mData;
    size_t mSize;

public:
    AudioBuffer(size_t size)
        : mSize(size)
        , mData(std::make_unique<float[]>(size))
    {}

    // No special member functions needed!
    // unique_ptr handles everything correctly
};
```

### Using `= default` and `= delete`

Explicitly control which operations are available:

```cpp
struct NonCopyable {
    int value;

    NonCopyable() = default;

    // Delete copy operations
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;

    // Keep move operations (compiler-generated)
    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;
};

NonCopyable a;
NonCopyable b = a;           // ❌ Error - copying deleted
NonCopyable c = std::move(a); // ✅ OK - moving allowed
```

**Use `= default` to document intent:**

```cpp
struct Settings {
    float attack;
    float decay;

    // Explicitly say "yes, use compiler-generated versions"
    Settings() = default;
    Settings(const Settings&) = default;
    Settings& operator=(const Settings&) = default;
    Settings(Settings&&) noexcept = default;
    Settings& operator=(Settings&&) noexcept = default;
    ~Settings() = default;
};
```

This shows you've **thought about** these operations (vs just not mentioning them).

### Debugging: Instrumenting Copy/Move

**Temporary technique** for learning - add print statements to see when copies/moves happen:

```cpp
struct Settings {
    float attack, decay, sustain, release;

    Settings() = default;

    Settings(const Settings& other)
        : attack(other.attack), decay(other.decay)
        , sustain(other.sustain), release(other.release)
    {
        std::cout << "[Settings] COPIED\n";
    }

    Settings& operator=(const Settings& other) {
        attack = other.attack;
        decay = other.decay;
        sustain = other.sustain;
        release = other.release;
        std::cout << "[Settings] COPY assigned\n";
        return *this;
    }

    Settings(Settings&& other) noexcept
        : attack(other.attack), decay(other.decay)
        , sustain(other.sustain), release(other.release)
    {
        std::cout << "[Settings] MOVED\n";
    }

    Settings& operator=(Settings&& other) noexcept {
        attack = other.attack;
        decay = other.decay;
        sustain = other.sustain;
        release = other.release;
        std::cout << "[Settings] MOVE assigned\n";
        return *this;
    }
};
```

**After learning:** Delete all five functions and return to Rule of Zero.

### The Deprecation Warning

If you declare a copy constructor but **not** a copy assignment operator (or vice versa), you'll get a deprecation warning:

```cpp
struct Bad {
    Bad(const Bad& other) { }  // Declared copy ctor
    // Missing copy assignment operator!
    // Compiler generates it, but warns it's deprecated
};
```

**Fix:** Either delete the copy constructor (Rule of Zero) or add all five (Rule of Five).

### Key Takeaways

1. **Default to Rule of Zero** - let the compiler do the work
2. **Only use Rule of Five** when managing resources directly
3. **If you need Rule of Five, consider using RAII types instead** (unique_ptr, vector, etc.)
4. **Never declare just some** of the five - it's all or nothing
5. **Use `= delete`** to explicitly disable operations
6. **Use `= default`** to document that you've thought about it

### Quick Decision Tree

```
Do you manage a raw resource (new/delete, file handle, etc.)?
├─ NO  → Rule of Zero (default behavior)
└─ YES → Can you use unique_ptr/vector/RAII wrapper?
         ├─ YES → Rule of Zero with RAII wrapper
         └─ NO  → Rule of Five (declare all 5)
```

**Audio Development Context:**
- Most audio DSP classes → Rule of Zero
- Simple data structures (Settings, Parameters) → Rule of Zero
- Buffer management → Use `std::vector` (Rule of Zero)
- Resource handles (audio streams, file I/O) → RAII wrapper or Rule of Five

## Common Gotchas

1. **Uninitialized members**: Always initialize primitive types (int, float, etc.)
2. **Declaration order matters**: Initializer list should match declaration order
3. **No default constructor after custom constructor**: Explicitly add one if needed
4. **Forgot return type?**: Constructors have NO return type (not even `void`)

## Quick Reference

```cpp
struct Example {
  // Members with defaults
  int value = 0;
  float data = 1.0f;

  // Default constructor
  Example() = default;

  // Custom constructor with initializer list
  Example(int v, float d) : value(v), data(d) {}

  // Method
  void doSomething() {
    // ...
  }

  // Const method (doesn't modify object)
  int getValue() const {
    return value;
  }
};
```
