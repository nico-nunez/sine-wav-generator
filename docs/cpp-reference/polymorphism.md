# Polymorphism and Inheritance

## Table of Contents
- [What is Polymorphism?](#what-is-polymorphism)
- [Virtual Functions](#virtual-functions)
- [Pure Virtual Functions (Abstract Classes)](#pure-virtual-functions-abstract-classes)
- [The override Keyword](#the-override-keyword)
- [How Virtual Functions Work (vtables)](#how-virtual-functions-work-vtables)
- [When to Use Polymorphism](#when-to-use-polymorphism)
- [When NOT to Use Polymorphism](#when-not-to-use-polymorphism)
- [Performance Considerations](#performance-considerations)
- [FAQ - Common Questions](#faq---common-questions)
- [Quick Rules](#quick-rules)

## What is Polymorphism?

**Polymorphism** = "many forms" - the ability to treat different types uniformly through a common interface.

In C++, this means a base class pointer/reference can call the correct derived class method at runtime.

```cpp
class Oscillator {
public:
  virtual float getNextSample() { return 0.0f; }  // virtual = "can be overridden"
};

class SineOscillator : public Oscillator {
public:
  float getNextSample() override { return std::sin(m_phase); }
};

class SquareOscillator : public Oscillator {
public:
  float getNextSample() override { return m_phase < M_PI ? 1.0f : -1.0f; }
};

// Polymorphism in action:
Oscillator* osc = new SineOscillator();
osc->getNextSample();  // Calls SineOscillator::getNextSample()

osc = new SquareOscillator();
osc->getNextSample();  // Calls SquareOscillator::getNextSample()
```

**Without `virtual`**, the base class version would always be called (decided at compile-time by pointer type).

**With `virtual`**, the derived class version is called (decided at runtime by actual object type).

## Virtual Functions

A virtual function is a member function that can be overridden in derived classes.

### Basic Syntax

```cpp
class Base {
public:
  virtual void doSomething() {
    std::cout << "Base version\n";
  }
};

class Derived : public Base {
public:
  void doSomething() override {  // override is optional but recommended
    std::cout << "Derived version\n";
  }
};

// Usage:
Base* ptr = new Derived();
ptr->doSomething();  // Prints "Derived version" - runtime polymorphism
```

### Key Points

- Declared with `virtual` keyword in base class
- Can have implementation (default behavior)
- Derived classes can override them
- Called based on **actual object type**, not pointer/reference type

### Example: Audio Processing

```cpp
class AudioEffect {
public:
  virtual void process(float* buffer, int numSamples) {
    // Default: do nothing (bypass)
  }

  virtual ~AudioEffect() {}  // Virtual destructor (important!)
};

class Reverb : public AudioEffect {
public:
  void process(float* buffer, int numSamples) override {
    // Reverb processing logic
  }
};

class Distortion : public AudioEffect {
public:
  void process(float* buffer, int numSamples) override {
    // Distortion processing logic
  }
};

// Polymorphic usage:
std::vector<AudioEffect*> effectChain;
effectChain.push_back(new Distortion());
effectChain.push_back(new Reverb());

for (auto* effect : effectChain) {
  effect->process(buffer, size);  // Calls correct derived version
}
```

### Virtual Destructors

**Critical rule:** If a class has virtual functions, it **must** have a virtual destructor.

```cpp
class Base {
public:
  virtual void foo() {}
  virtual ~Base() {}  // Virtual destructor - REQUIRED
};

class Derived : public Base {
public:
  Derived() { m_data = new float[1024]; }
  ~Derived() { delete[] m_data; }
private:
  float* m_data;
};

// Why it matters:
Base* ptr = new Derived();
delete ptr;  // Without virtual destructor, only Base::~Base() is called!
             // Derived::~Derived() is skipped - MEMORY LEAK
             // With virtual destructor, both destructors are called correctly
```

## Pure Virtual Functions (Abstract Classes)

A **pure virtual function** has `= 0` and **no implementation** - derived classes **must** override it.

### Syntax

```cpp
class Oscillator {  // Abstract class - cannot instantiate
public:
  virtual float getNextSample() = 0;  // Pure virtual - MUST be overridden

  virtual ~Oscillator() {}
};

// Cannot do this:
Oscillator osc;  // ERROR - cannot instantiate abstract class

// Must derive and implement:
class SineOscillator : public Oscillator {
public:
  float getNextSample() override {  // MUST implement
    return std::sin(m_phase);
  }
private:
  float m_phase;
};

// Now we can use it:
Oscillator* osc = new SineOscillator();  // OK - pointer to abstract type
osc->getNextSample();
```

### When to Use Pure Virtual

**Use pure virtual when:**
- There's no sensible default implementation
- You want to **force** derived classes to implement the method
- You're defining an **interface** that subclasses must follow

**Example: Plugin Interface**

```cpp
class AudioPlugin {  // Abstract interface
public:
  virtual void process(float* input, float* output, int numSamples) = 0;
  virtual const char* getName() const = 0;
  virtual void setParameter(int index, float value) = 0;

  virtual ~AudioPlugin() {}
};

// Every plugin MUST implement these methods
class ReverbPlugin : public AudioPlugin {
public:
  void process(float* input, float* output, int numSamples) override {
    // Reverb implementation
  }

  const char* getName() const override {
    return "Reverb";
  }

  void setParameter(int index, float value) override {
    // Handle parameters
  }
};
```

### Mixing Virtual and Pure Virtual

You can have both in the same class:

```cpp
class Oscillator {
public:
  // Pure virtual - MUST override
  virtual float generateWaveform(float phase) = 0;

  // Regular virtual - CAN override, has default
  virtual void reset() {
    m_phase = 0.0f;
  }

protected:
  float m_phase = 0.0f;
};
```

## The override Keyword

The `override` keyword (C++11+) explicitly marks a function as overriding a base class virtual function.

### Why Use It?

**Catches errors at compile time:**

```cpp
class Base {
public:
  virtual void process(float value) {}
};

class Derived : public Base {
public:
  // Typo in parameter type - accidentally creates NEW function
  void process(int value) {}  // Compiles, but doesn't override!
};

// With override:
class Derived : public Base {
public:
  void process(int value) override {}  // ERROR - not overriding anything
                                       // Compiler catches the mistake!
};
```

**Makes intent clear:**

```cpp
class SineOscillator : public Oscillator {
public:
  float getNextSample() override {  // Clear: this overrides base class method
    return std::sin(m_phase);
  }
};
```

**Best practice:** Always use `override` when overriding virtual functions.

## How Virtual Functions Work (vtables)

Virtual functions use a **vtable** (virtual function table) - a lookup table created by the compiler.

### Conceptual Model

```cpp
class Animal {
public:
  virtual void speak() { cout << "..."; }
};

class Dog : public Animal {
public:
  void speak() override { cout << "Woof!"; }
};
```

**Behind the scenes:**

```
Animal vtable:
  [0]: Animal::speak

Dog vtable:
  [0]: Dog::speak

Dog object in memory:
  [vptr] -> Dog vtable
  [member data...]
```

Each object with virtual functions has a hidden **vptr** (virtual pointer) pointing to its class's vtable.

**When you call a virtual function:**

1. Dereference the object's vptr to find the vtable
2. Look up the function pointer in the vtable
3. Call the function through that pointer

### Why It Matters

**Performance cost:**
- One extra pointer dereference per virtual function call
- Small memory overhead (one vptr per object)
- Usually negligible, but matters in tight loops

**Example:**

```cpp
// Non-virtual: direct call (compile-time)
float result = oscillator.generateSample();  // ~0 overhead

// Virtual: vtable lookup (runtime)
float result = oscillator->generateSample();  // ~1 pointer dereference
```

For audio processing at 44.1kHz, this is usually fine. For per-sample processing in inner loops, consider alternatives (see "When NOT to Use" below).

## When to Use Polymorphism

**Good use cases:**

### 1. Plugin Systems

```cpp
class AudioEffect {
public:
  virtual void process(float* buffer, int size) = 0;
};

// Load different effects at runtime
std::vector<AudioEffect*> effects;
```

### 2. Per-Waveform State/Logic

```cpp
class WavetableOscillator : public Oscillator {
public:
  void loadWavetable(float* data, int size) { /* ... */ }
private:
  float* m_wavetable;  // Unique to this type
  int m_tableSize;
};

class PWMOscillator : public Oscillator {
public:
  void setPulseWidth(float width) { /* ... */ }
private:
  float m_pulseWidth;  // Unique to this type
};
```

### 3. Framework Callbacks

```cpp
class MidiListener {
public:
  virtual void onNoteOn(int note, int velocity) = 0;
  virtual void onNoteOff(int note) = 0;
};
```

## When NOT to Use Polymorphism

**Avoid for:**

### 1. Simple Stateless Functions

```cpp
// DON'T use inheritance for simple math functions
float sineWave(float phase) { return std::sin(phase); }
float squareWave(float phase) { return phase < M_PI ? 1.0f : -1.0f; }

// Use function pointers instead (see function-pointers.md)
```

### 2. Performance-Critical Inner Loops

```cpp
// Avoid virtual calls in per-sample loops
for (int i = 0; i < 1000000; ++i) {
  buffer[i] = oscillator->getNextSample();  // Virtual call overhead
}

// Prefer strategies that can be inlined or use function pointers
```

### 3. When You Don't Need Runtime Flexibility

```cpp
// If you know the type at compile time, don't use polymorphism
SineOscillator osc;  // Direct type - no need for base class pointer
```

## Performance Considerations

### Virtual Function Overhead

**Typical cost:**
- One pointer dereference (~1-2ns on modern CPUs)
- Prevents inlining (compiler can't optimize across virtual calls)

**When it matters:**
- Per-sample processing in tight loops (44.1kHz = 22μs per sample)
- Cache-sensitive code (vtable lookups can miss cache)

**When it doesn't matter:**
- Per-buffer processing (once per 512 samples, etc.)
- Initialization/setup code
- User interaction handling

### Example: Audio Context

```cpp
// Per-buffer: virtual is fine
class AudioProcessor {
public:
  virtual void processBlock(float* buffer, int size) = 0;  // Called 86 times/sec @ 512 samples
};

// Per-sample: avoid virtual
class Oscillator {
  // DON'T do this in tight loop:
  for (int i = 0; i < size; ++i) {
    buffer[i] = generateSample();  // Virtual call - 44100 times/sec
  }
};
```

## FAQ - Common Questions

### Q: Why `Base* ptr = new Derived()` instead of just `Derived ptr = new Derived()`?

**A: You only need the base pointer when you're using polymorphism - when the type isn't known until runtime.**

**Key insight:** The base pointer is NOT about creating the object - it's about HOW and WHERE you'll use it.

#### Isolated Use - No Pointer Needed:

```cpp
// You know the exact type - use it directly
SineOscillator osc(440.0f);
osc.setFrequency(880.0f);
float sample = osc.getNextSample();
// Perfect! No pointer, no Base class needed.
```

#### Polymorphic Use - Base Pointer Needed:

```cpp
// 1. Mixed types in container
std::vector<Oscillator*> oscillators;  // Base* - holds any derived type
oscillators.push_back(new SineOscillator());
oscillators.push_back(new SquareOscillator());
oscillators.push_back(new SawOscillator());

for (auto* osc : oscillators) {
  osc->getNextSample();  // Calls correct version for each type
}

// 2. Function that accepts "any Oscillator"
void processOscillator(Oscillator* osc) {  // Base* parameter
  osc->getNextSample();
}

processOscillator(new SineOscillator());    // Can pass any derived type
processOscillator(new SquareOscillator());

// 3. Factory - type determined at runtime
Oscillator* createOscillator(std::string type) {
  if (type == "sine") return new SineOscillator();
  if (type == "square") return new SquareOscillator();
  return nullptr;
}

std::string userChoice = getUserInput();
Oscillator* osc = createOscillator(userChoice);  // Don't know type until runtime!
```

**When you need `Base*`:**
- ✓ Storing different derived types in the same container
- ✓ Function parameters that accept "any derived type"
- ✓ Return types where the exact type isn't known until runtime
- ✓ Writing code that works with "any X" without knowing which X

**When you DON'T need `Base*`:**
- ✗ You know the exact type when writing the code
- ✗ Using the object in isolation
- ✗ No need for runtime type flexibility

### Q: Can Derived exist without a pointer or Base class?

**A: Absolutely yes!**

```cpp
// Perfectly valid - use Derived directly
SineOscillator osc(440.0f);
osc.setFrequency(880.0f);
float sample = osc.getNextSample();

// No Base*, no pointer, no problem!
```

Inheritance means "SineOscillator HAS everything Oscillator has, plus more." You can use SineOscillator directly without ever mentioning the base class.

**You only need the base class pointer when you want polymorphism** - when you're writing code that doesn't know the exact type until runtime.

### Q: Is `new` required because it's derived?

**A: No! `new` is about stack vs heap allocation, not about inheritance.**

```cpp
// Stack - no new (automatic cleanup)
SineOscillator osc(440.0f);  // Derived class on stack - totally fine
// Automatically destroyed when out of scope

// Heap - with new (manual cleanup or smart pointers)
SineOscillator* osc = new SineOscillator(440.0f);
delete osc;  // Must delete
```

The polymorphism examples use `new` because they need **pointers** (for the base class pointer), not because of inheritance. See [pointers-and-references.md](pointers-and-references.md) for details on `new`/`delete`.

### Q: Is `->` required? Why not `.` or `::`?

**A: The operator depends on what you're accessing:**

```cpp
// . (dot) - for objects
SineOscillator osc;
osc.getNextSample();  // Object on stack - use dot

// -> (arrow) - for pointers
SineOscillator* ptr = new SineOscillator();
ptr->getNextSample();  // Pointer - use arrow

// Arrow is shorthand for dereference + dot:
(*ptr).getNextSample();  // Same as ptr->getNextSample()

// :: (scope resolution) - for class/namespace, NOT instances
Oscillator::staticMethod();  // Class member
std::sin(1.0f);  // Namespace

// NOT valid:
osc::getNextSample();  // ERROR - osc is an instance, not a class/namespace
```

See [pointers-and-references.md](pointers-and-references.md) for more on `->` vs `.` vs `::`.

### Q: Is `override` required for pure virtual functions?

**A: No, but strongly recommended.**

```cpp
class Base {
public:
  virtual float process(float input) = 0;  // Pure virtual
};

// Without override - works but risky
class Derived : public Base {
public:
  float process(float input) {  // No override - still works
    return input * 2.0f;
  }
};

// With override - catches mistakes
class Derived : public Base {
public:
  float process(float input) override {  // Recommended!
    return input * 2.0f;
  }
};
```

**What happens if you don't implement a pure virtual?**

```cpp
class Derived : public Base {
  // Forgot to implement process()
};

Derived d;  // COMPILE ERROR: Derived is still abstract
```

**Why use `override` even though it's not required?**

Catches errors:

```cpp
class Derived : public Base {
public:
  // Typo: int instead of float
  float process(int input) override {  // COMPILE ERROR - not overriding anything!
    return input * 2.0f;
  }
};
```

Without `override`, this would compile but create a new function instead of overriding the base version, making Derived still abstract!

### Q: How does the compiler not know it's Derived when I write `class Derived : public Base`?

**A: The compiler DOES know when you use the object directly. Polymorphism is about when YOU don't know the type when writing the code.**

**Compile-time - you know the type:**

```cpp
SineOscillator osc;  // You know it's SineOscillator
osc.getNextSample();  // Compiler knows exactly which function to call
// No polymorphism needed - direct function call
```

**Runtime - you DON'T know the type:**

```cpp
std::string userChoice = getUserInput();  // Unknown until program runs

Oscillator* osc;  // Don't know which type yet!
if (userChoice == "sine") {
  osc = new SineOscillator();
} else if (userChoice == "square") {
  osc = new SquareOscillator();
}

osc->getNextSample();  // Compiler can't know which type at compile time
                       // Must use vtable to figure it out at runtime
```

**Polymorphism is for writing code that works with types you don't know when writing the code** - only the running program knows.

## Quick Rules

1. **Use `virtual`** when derived classes need to override behavior
2. **Use `= 0` (pure virtual)** when there's no sensible base implementation
3. **Always use `override`** when overriding to catch errors
4. **Always have virtual destructor** if class has any virtual functions
5. **Avoid virtual calls** in per-sample loops for performance
6. **Consider alternatives** (function pointers, templates) for simple cases
7. **Use polymorphism** for plugin systems, runtime flexibility, and per-type state
8. **Base pointers are for polymorphic interfaces** - you don't need them when using derived types directly

**Coming from TypeScript:** Virtual functions are similar to abstract methods in TypeScript classes, but C++ requires explicit `virtual` keyword and has performance implications you need to consider.
