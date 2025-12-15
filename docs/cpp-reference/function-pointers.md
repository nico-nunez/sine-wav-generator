# Function Pointers

## Table of Contents
- [What is a Function Pointer?](#what-is-a-function-pointer)
- [Basic Syntax](#basic-syntax)
- [Function Pointers as Parameters](#function-pointers-as-parameters)
- [Member Function Pointers](#member-function-pointers)
- [Typedef for Readability](#typedef-for-readability)
- [Real-World Example: Waveform Strategy](#real-world-example-waveform-strategy)
- [Function Pointers vs Other Approaches](#function-pointers-vs-other-approaches)
- [Common Use Cases in Audio Programming](#common-use-cases-in-audio-programming)
- [Quick Rules](#quick-rules)

## What is a Function Pointer?

A **function pointer** is a variable that stores the address of a function. You can call the function through the pointer.

**Why use them?**
- Pass functions as parameters to other functions
- Store different function implementations and swap them at runtime
- Callback mechanisms (common in audio APIs, VST plugins, etc.)

**Analogy from TypeScript:**

```typescript
// TypeScript
type WaveformFunc = (phase: number) => number;

const sine: WaveformFunc = (phase) => Math.sin(phase);
const square: WaveformFunc = (phase) => phase < Math.PI ? 1 : -1;

let currentWaveform = sine;  // Store function in variable
currentWaveform(0.5);        // Call it
currentWaveform = square;    // Swap it
```

Function pointers are C++'s way of doing this.

## Basic Syntax

### Declaration and Assignment

```cpp
// Regular function
float sineWave(float phase) {
  return std::sin(phase);
}

// Function pointer declaration
float (*waveformFunc)(float);  // Pointer to function that takes float, returns float

// Assign function to pointer
waveformFunc = sineWave;  // No () - we want the address, not to call it

// Or equivalently:
waveformFunc = &sineWave;  // Explicit address-of operator

// Call through pointer
float result = waveformFunc(0.5f);  // Just like calling sineWave(0.5f)
```

### Syntax Breakdown

```cpp
float (*waveformFunc)(float);
  ^      ^     ^         ^
  |      |     |         |
  |      |     |         +-- Parameter type(s)
  |      |     +------------ Pointer name
  |      +------------------ Indicates it's a pointer
  +------------------------- Return type
```

**The parentheses matter:**

```cpp
float (*funcPtr)(float);   // Pointer to function - CORRECT
float *funcPtr(float);     // Function returning pointer - DIFFERENT!
```

### Multiple Functions, Same Signature

```cpp
float sineWave(float phase) {
  return std::sin(phase);
}

float squareWave(float phase) {
  return phase < M_PI ? 1.0f : -1.0f;
}

float triangleWave(float phase) {
  return 2.0f * std::abs(phase / M_PI - 1.0f) - 1.0f;
}

// One pointer can point to any of them
float (*waveFunc)(float);

waveFunc = sineWave;
float s = waveFunc(1.0f);    // Calls sineWave

waveFunc = squareWave;
float sq = waveFunc(1.0f);   // Calls squareWave

waveFunc = triangleWave;
float t = waveFunc(1.0f);    // Calls triangleWave
```

**Key point:** All functions must have the **same signature** (same parameter types and return type).

## Function Pointers as Parameters

Pass behavior to other functions - the foundation of callbacks and strategies.

### Simple Example

```cpp
// Function that takes a function pointer as parameter
void processSamples(float* buffer, int size, float (*waveform)(float)) {
  float phase = 0.0f;
  for (int i = 0; i < size; ++i) {
    buffer[i] = waveform(phase);  // Call the passed function
    phase += 0.1f;
  }
}

// Usage:
float buffer[100];
processSamples(buffer, 100, sineWave);     // Uses sine
processSamples(buffer, 100, squareWave);   // Uses square
```

### Audio Callback Example

```cpp
// Typical audio API callback signature
typedef void (*AudioCallback)(float* input, float* output, int numSamples, void* userData);

void myProcessCallback(float* input, float* output, int numSamples, void* userData) {
  // Process audio
  for (int i = 0; i < numSamples; ++i) {
    output[i] = input[i] * 0.5f;  // Simple gain
  }
}

// Register callback with audio system
audioSystem.setCallback(myProcessCallback);
```

## Member Function Pointers

Pointers to member functions have different syntax (and are more complex).

### Syntax

```cpp
class Oscillator {
public:
  float sineWave(float phase) { return std::sin(phase); }
  float squareWave(float phase) { return phase < M_PI ? 1.0f : -1.0f; }
};

// Member function pointer declaration
float (Oscillator::*waveformMethod)(float);  // Note: Oscillator::*

// Assign
waveformMethod = &Oscillator::sineWave;  // Must use & for member functions

// Call requires an object instance
Oscillator osc;
float result = (osc.*waveformMethod)(0.5f);  // .* operator
```

**Pointer-to-member syntax is awkward** - this is why many prefer:
- Regular (non-member) function pointers
- std::function (see lambdas.md)
- Virtual functions for member polymorphism

## Typedef for Readability

Function pointer syntax gets ugly fast. Use `typedef` or `using` for clarity.

### Using typedef

```cpp
// Without typedef - hard to read
void processSamples(float* buffer, int size, float (*waveform)(float));

// With typedef
typedef float (*WaveformFunc)(float);

void processSamples(float* buffer, int size, WaveformFunc waveform);  // Much clearer!
```

### Using 'using' (Modern C++11+)

```cpp
// Modern C++ approach - even clearer
using WaveformFunc = float (*)(float);

void processSamples(float* buffer, int size, WaveformFunc waveform);
```

### Multiple Parameters

```cpp
// Typedef for complex signatures
typedef void (*ProcessCallback)(float* input, float* output, int size, void* userData);

// Or with 'using':
using ProcessCallback = void (*)(float* input, float* output, int size, void* userData);

// Now easy to use:
void setCallback(ProcessCallback callback);
```

## Real-World Example: Waveform Strategy

Using function pointers to implement the Strategy pattern for oscillator waveforms.

### Waveform Functions

```cpp
// waveforms.h
namespace Waveforms {
  float sine(float phase);
  float square(float phase);
  float triangle(float phase);
  float saw(float phase);
}

// waveforms.cpp
namespace Waveforms {
  float sine(float phase) {
    return std::sin(phase);
  }

  float square(float phase) {
    return phase < M_PI ? 1.0f : -1.0f;
  }

  float triangle(float phase) {
    float normalized = phase / (2.0f * M_PI);  // 0.0 to 1.0
    return 4.0f * std::abs(normalized - 0.5f) - 1.0f;
  }

  float saw(float phase) {
    return 2.0f * (phase / (2.0f * M_PI)) - 1.0f;
  }
}
```

### Oscillator Using Function Pointer

```cpp
// Oscillator.h
class Oscillator {
public:
  using WaveformFunc = float (*)(float);  // Typedef for clarity

  Oscillator(float freq, float sampleRate = 44100.0f);

  void setWaveform(WaveformFunc func);
  float getNextSampleValue();

private:
  float m_phase = 0.0f;
  float m_phaseIncrement;
  WaveformFunc m_waveformFunc = Waveforms::sine;  // Default to sine

  void incrementPhase();
};

// Oscillator.cpp
void Oscillator::setWaveform(WaveformFunc func) {
  m_waveformFunc = func;
}

float Oscillator::getNextSampleValue() {
  float sample = m_waveformFunc(m_phase);  // Call through pointer
  incrementPhase();
  return sample;
}

// Usage:
Oscillator osc(440.0f);
osc.setWaveform(Waveforms::sine);
osc.setWaveform(Waveforms::square);
osc.setWaveform(Waveforms::saw);
```

**Benefits:**
- Clean separation: Oscillator handles phase, waveforms are just math
- Easy to add new waveforms without modifying Oscillator class
- No virtual function overhead
- Can swap waveforms at runtime

## Function Pointers vs Other Approaches

### vs Virtual Functions

```cpp
// Virtual functions
class Oscillator {
public:
  virtual float generate(float phase) = 0;  // vtable lookup
};

// Function pointers
float (*generate)(float);  // Direct function call (can be inlined)
```

**Function pointers:**
- ✓ Lighter weight (no vtable overhead)
- ✓ Can point to any function (not just class members)
- ✗ Can't capture state (no member variables)
- ✗ Less type-safe than virtual functions

**Virtual functions:**
- ✓ Encapsulate state with behavior
- ✓ Clear class hierarchy
- ✗ Slight overhead (vtable lookup)
- ✗ Must use inheritance

### vs std::function

```cpp
// Function pointer - raw and fast
float (*func)(float);

// std::function - flexible but heavier
std::function<float(float)> func;
```

**Function pointers:**
- ✓ Zero overhead (direct function call)
- ✓ Simple and explicit
- ✗ Can't capture variables (no closures)
- ✗ Ugly syntax for complex signatures

**std::function (see lambdas.md):**
- ✓ Can hold lambdas with captures
- ✓ Cleaner syntax
- ✗ Small overhead (type erasure)
- ✗ Slightly more memory

### vs Switch Statement

```cpp
// Switch statement
enum Waveform { Sine, Square, Triangle };
float generate(Waveform type, float phase) {
  switch (type) {
    case Sine: return std::sin(phase);
    case Square: return phase < M_PI ? 1.0f : -1.0f;
    // ...
  }
}

// Function pointer
float (*generate)(float) = sineWave;  // Swap without conditionals
```

**Function pointers:**
- ✓ No runtime branching
- ✓ Open/Closed Principle (add waveforms without modifying code)
- ✗ Slightly more complex setup

**Switch:**
- ✓ Simple and explicit
- ✓ All logic in one place
- ✗ Must modify code to add cases
- ✗ Branch per call

## Common Use Cases in Audio Programming

### 1. Audio Callbacks

```cpp
// VST/Audio plugin callback signature
typedef void (*ProcessCallback)(float** inputs, float** outputs, int numSamples);

void registerProcessor(ProcessCallback callback);
```

### 2. Waveform Generation

```cpp
// Oscillator waveform strategies
using WaveformFunc = float (*)(float phase);
oscillator.setWaveform(Waveforms::saw);
```

### 3. DSP Algorithm Selection

```cpp
// Different resampling algorithms
using ResampleFunc = void (*)(float* input, float* output, int inSize, int outSize);

void resample(float* in, float* out, int inSize, int outSize, ResampleFunc algorithm) {
  algorithm(in, out, inSize, outSize);
}

resample(in, out, 100, 200, linearInterpolation);
resample(in, out, 100, 200, cubicInterpolation);
```

### 4. Filter Callbacks

```cpp
// Custom filter processing
using FilterFunc = float (*)(float input, void* state);

class CustomFilter {
public:
  void setProcessFunc(FilterFunc func) { m_processFunc = func; }

  float process(float input) {
    return m_processFunc(input, &m_state);
  }

private:
  FilterFunc m_processFunc;
  FilterState m_state;
};
```

## Quick Rules

1. **Syntax:** `returnType (*ptrName)(paramTypes)`
2. **Assign:** `ptr = functionName` or `ptr = &functionName`
3. **Call:** `ptr(args)` - just like a regular function
4. **Use typedef/using** to make complex signatures readable
5. **All functions must match signature** exactly
6. **Can't capture state** - use std::function or virtual functions if you need state
7. **Zero overhead** - direct function call, can be inlined
8. **Common in C APIs** - audio callbacks, plugin interfaces, etc.

**When to use:**
- Callbacks (audio processing, event handlers)
- Strategy pattern without inheritance overhead
- C API interop
- Performance-critical code where std::function overhead matters

**When to avoid:**
- Need to capture variables → use lambdas/std::function
- Need per-instance state → use virtual functions
- Simple cases with few options → use switch/if
