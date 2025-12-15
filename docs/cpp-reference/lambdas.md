# Lambdas and std::function

## Table of Contents
- [What is a Lambda?](#what-is-a-lambda)
- [Basic Lambda Syntax](#basic-lambda-syntax)
- [Captures](#captures)
- [Lambda Return Types](#lambda-return-types)
- [std::function - The Lambda Container](#stdfunction---the-lambda-container)
- [Lambdas vs Function Pointers](#lambdas-vs-function-pointers)
- [Real-World Example: Audio Processing](#real-world-example-audio-processing)
- [Performance Considerations](#performance-considerations)
- [Quick Rules](#quick-rules)

## What is a Lambda?

A **lambda** is an anonymous (unnamed) function you can define inline. Think of it as a function literal.

**Coming from TypeScript:**

```typescript
// TypeScript - arrow functions
const square = (x: number) => x * x;

setTimeout(() => {
  console.log('Hello');
}, 1000);

const numbers = [1, 2, 3];
numbers.map(n => n * 2);
```

C++ lambdas are similar - inline, anonymous functions.

### Why Use Lambdas?

- **Inline callbacks** - no need to define separate function
- **Capture local variables** - closures (function pointers can't do this!)
- **Cleaner code** - keep logic close to where it's used

## Basic Lambda Syntax

### Minimal Lambda

```cpp
// Lambda that takes int, returns int
auto square = [](int x) { return x * x; };

int result = square(5);  // 25
```

### Syntax Breakdown

```cpp
[capture](parameters) { body }
  ^         ^           ^
  |         |           |
  |         |           +-- Function body
  |         +-------------- Parameters (like regular function)
  +------------------------ Capture clause (variables from outer scope)
```

### Full Form

```cpp
[capture](parameters) -> returnType { body }
```

The `-> returnType` is optional - compiler usually deduces it.

## Captures

**Captures** let lambdas use variables from the surrounding scope - this is what makes them powerful.

### Capture Modes

```cpp
int x = 10;
int y = 20;

// [=] - Capture all by value (copy)
auto byValue = [=]() { return x + y; };  // x and y are copied

// [&] - Capture all by reference
auto byRef = [&]() { x = 100; y = 200; };  // x and y can be modified

// Specific captures
auto specific = [x, &y]() { y = x + 5; };  // x by value, y by reference

// Capture nothing
auto noCapture = []() { return 42; };
```

### Capture Examples

```cpp
float gain = 0.5f;
float offset = 0.1f;

// Capture gain by value
auto process1 = [gain](float sample) {
  return sample * gain;  // gain is copied into lambda
};

// Capture gain by reference
auto process2 = [&gain](float sample) {
  return sample * gain;  // gain is referenced (sees changes)
};

// Capture multiple
auto process3 = [gain, offset](float sample) {
  return sample * gain + offset;
};

// Capture all by value
auto process4 = [=](float sample) {
  return sample * gain + offset;  // All local vars captured
};
```

### Capture by Value vs Reference

```cpp
int counter = 0;

// By value - copies counter
auto incrementCopy = [counter]() mutable {
  counter++;  // Modifies the COPY, not original
  return counter;
};

incrementCopy();  // Returns 1
incrementCopy();  // Returns 2
std::cout << counter;  // Still 0 - original unchanged

// By reference - refers to counter
auto incrementRef = [&counter]() {
  counter++;  // Modifies the ORIGINAL
  return counter;
};

incrementRef();  // Returns 1
incrementRef();  // Returns 2
std::cout << counter;  // Now 2 - original modified
```

**Note:** The `mutable` keyword allows modifying captured-by-value variables inside the lambda.

### Capture Pitfalls

**Dangling references:**

```cpp
std::function<int()> makeCounter() {
  int count = 0;
  return [&count]() { return count++; };  // BAD! count goes out of scope
}

auto counter = makeCounter();
counter();  // UNDEFINED BEHAVIOR - count is destroyed
```

**Fix:** Capture by value or use shared state.

```cpp
std::function<int()> makeCounter() {
  int count = 0;
  return [count]() mutable { return count++; };  // OK - count is copied
}
```

## Lambda Return Types

### Auto Deduction (Default)

```cpp
// Return type deduced as float
auto lambda = [](float x) { return x * 2.0f; };
```

### Explicit Return Type

Use `-> type` when:
- Multiple return statements with different types
- Compiler can't deduce (rare)
- You want to be explicit

```cpp
// Explicit return type
auto lambda = [](float x) -> float {
  if (x < 0.0f) return 0.0f;
  return x * 2.0f;
};

// Return type needed for conversions
auto toInt = [](float x) -> int {
  return static_cast<int>(x);
};
```

## std::function - The Lambda Container

`std::function` is a wrapper that can hold **any callable**: lambdas, function pointers, functors.

### Include and Syntax

```cpp
#include <functional>

std::function<returnType(paramTypes)> name;
```

### Examples

```cpp
// std::function holding a lambda
std::function<float(float)> waveform = [](float phase) {
  return std::sin(phase);
};

float result = waveform(1.0f);

// Can reassign to different lambda
waveform = [](float phase) {
  return phase < M_PI ? 1.0f : -1.0f;  // Square wave
};

// Can hold function pointer
float sineWave(float phase) { return std::sin(phase); }
waveform = sineWave;  // Works!

// Can hold capturing lambda
float gain = 0.5f;
waveform = [gain](float phase) {
  return std::sin(phase) * gain;  // Captures gain
};
```

### std::function as Class Member

```cpp
class Oscillator {
public:
  using WaveformFunc = std::function<float(float)>;

  void setWaveform(WaveformFunc func) {
    m_waveformFunc = func;
  }

  float getNextSample() {
    float sample = m_waveformFunc(m_phase);
    incrementPhase();
    return sample;
  }

private:
  float m_phase = 0.0f;
  WaveformFunc m_waveformFunc = [](float phase) { return std::sin(phase); };  // Default
};

// Usage:
Oscillator osc;

// Set to lambda
osc.setWaveform([](float phase) {
  return phase < M_PI ? 1.0f : -1.0f;
});

// Set to lambda with capture
float pulseWidth = 0.7f;
osc.setWaveform([pulseWidth](float phase) {
  float threshold = 2.0f * M_PI * pulseWidth;
  return phase < threshold ? 1.0f : -1.0f;
});

// Set to function pointer
osc.setWaveform(sineWave);
```

### std::function vs Function Pointers

| Feature | Function Pointer | std::function |
|---------|-----------------|---------------|
| **Syntax** | `float (*)(float)` | `std::function<float(float)>` |
| **Can capture?** | No | Yes (lambdas) |
| **Overhead** | Zero | Small (type erasure) |
| **Flexibility** | Functions only | Functions, lambdas, functors |
| **Readability** | Ugly | Clean |

```cpp
// Function pointer - can't capture
float (*funcPtr)(float) = [](float x) { return x * 2.0f; };  // OK - no capture

float gain = 0.5f;
funcPtr = [gain](float x) { return x * gain; };  // ERROR - can't capture

// std::function - can capture
std::function<float(float)> func = [gain](float x) { return x * gain; };  // OK
```

## Lambdas vs Function Pointers

### When to Use Lambdas

**Use lambdas when:**
- You need to capture local variables
- You want inline, one-off callbacks
- Readability matters more than micro-optimization

```cpp
// Lambda with capture - clean and readable
float threshold = 0.5f;
auto clamp = [threshold](float value) {
  return value > threshold ? threshold : value;
};

processBuffer(buffer, size, clamp);
```

### When to Use Function Pointers

**Use function pointers when:**
- No need to capture variables
- Performance is critical (zero overhead)
- Interfacing with C APIs

```cpp
// Function pointer - no overhead
float sineWave(float phase) { return std::sin(phase); }
float (*waveFunc)(float) = sineWave;
```

### Conversion to Function Pointer

**Non-capturing lambdas** can convert to function pointers:

```cpp
// Non-capturing lambda -> converts to function pointer
float (*funcPtr)(float) = [](float x) { return x * 2.0f; };  // OK

// Capturing lambda -> cannot convert
float gain = 0.5f;
float (*funcPtr2)(float) = [gain](float x) { return x * gain; };  // ERROR
```

## Real-World Example: Audio Processing

### Flexible Effect Chain Using std::function

```cpp
class EffectChain {
public:
  using EffectFunc = std::function<float(float)>;

  void addEffect(EffectFunc effect) {
    m_effects.push_back(effect);
  }

  void process(float* buffer, int size) {
    for (int i = 0; i < size; ++i) {
      float sample = buffer[i];
      for (auto& effect : m_effects) {
        sample = effect(sample);  // Apply each effect
      }
      buffer[i] = sample;
    }
  }

private:
  std::vector<EffectFunc> m_effects;
};

// Usage:
EffectChain chain;

// Add simple gain
float gain = 0.5f;
chain.addEffect([gain](float sample) {
  return sample * gain;
});

// Add hard clip
float threshold = 0.8f;
chain.addEffect([threshold](float sample) {
  if (sample > threshold) return threshold;
  if (sample < -threshold) return -threshold;
  return sample;
});

// Add DC offset
float offset = 0.1f;
chain.addEffect([offset](float sample) {
  return sample + offset;
});

// Process buffer
float buffer[512];
chain.process(buffer, 512);
```

### Oscillator with Custom Waveform

```cpp
class Oscillator {
public:
  void setWaveform(std::function<float(float)> func) {
    m_waveformFunc = func;
  }

  void setFrequency(float freq) {
    m_frequency = freq;
    calculatePhaseIncrement();
  }

  float getNextSample() {
    float sample = m_waveformFunc(m_phase);
    m_phase += m_phaseIncrement;
    if (m_phase >= TWO_PI) m_phase -= TWO_PI;
    return sample;
  }

private:
  float m_phase = 0.0f;
  float m_phaseIncrement = 0.0f;
  float m_frequency = 440.0f;
  float m_sampleRate = 44100.0f;
  std::function<float(float)> m_waveformFunc;

  void calculatePhaseIncrement() {
    m_phaseIncrement = TWO_PI * m_frequency / m_sampleRate;
  }
};

// Usage with different waveforms:
Oscillator osc;

// Sine wave
osc.setWaveform([](float phase) { return std::sin(phase); });

// Square wave with adjustable pulse width
float pulseWidth = 0.5f;  // 50% duty cycle
osc.setWaveform([pulseWidth](float phase) {
  return phase < (TWO_PI * pulseWidth) ? 1.0f : -1.0f;
});

// Custom wave with harmonics
osc.setWaveform([](float phase) {
  return std::sin(phase) * 0.5f +           // Fundamental
         std::sin(phase * 2.0f) * 0.25f +   // 2nd harmonic
         std::sin(phase * 3.0f) * 0.125f;   // 3rd harmonic
});
```

## Performance Considerations

### Overhead Comparison

```cpp
// 1. Direct function call - fastest (inlineable)
float sine(float x) { return std::sin(x); }
sine(1.0f);  // ~0 overhead

// 2. Function pointer - fast (direct call, but not inlineable)
float (*funcPtr)(float) = sine;
funcPtr(1.0f);  // ~0 overhead

// 3. Non-capturing lambda (can convert to function pointer)
auto lambda = [](float x) { return std::sin(x); };
lambda(1.0f);  // ~0 overhead (likely inlined)

// 4. std::function - small overhead
std::function<float(float)> func = sine;
func(1.0f);  // Small overhead (type erasure, virtual call)
```

### When Overhead Matters

**Per-sample processing (44.1kHz):**
```cpp
// Avoid std::function in tight loops
for (int i = 0; i < 1000000; ++i) {
  buffer[i] = m_waveformFunc(phase);  // Millions of calls - overhead adds up
}
```

**Per-buffer processing (86 times/sec @ 512 samples):**
```cpp
// std::function overhead negligible here
processBuffer(buffer, 512, [gain](float s) { return s * gain; });  // Fine
```

### Optimization Tips

1. **Use `auto`** when possible - avoids std::function wrapper
```cpp
auto lambda = [](float x) { return x * 2.0f; };  // Type is lambda, not std::function
```

2. **Store std::function** only when you need type erasure
```cpp
// If type is always the same, use auto
auto process = [gain](float x) { return x * gain; };

// If type varies, use std::function
std::function<float(float)> process = /* different lambdas */;
```

3. **Mark lambdas `inline`** for single-use callbacks
```cpp
processBuffer(buffer, size, [gain](float s) { return s * gain; });  // Likely inlined
```

## Quick Rules

1. **Syntax:** `[capture](params) { body }`
2. **Capture by value:** `[x]` or `[=]` (copy)
3. **Capture by reference:** `[&x]` or `[&]` (reference)
4. **Modify captured value:** Add `mutable` keyword
5. **Explicit return type:** `[](params) -> type { body }`
6. **std::function** holds lambdas, function pointers, functors
7. **Non-capturing lambdas** can convert to function pointers
8. **Capturing lambdas** cannot convert to function pointers

**When to use:**
- Inline callbacks and event handlers
- Need to capture local variables
- Flexible, readable code over raw performance

**When to avoid:**
- Per-sample processing (use function pointers or direct calls)
- C API interop that needs function pointers
- When zero overhead is critical
