# Classes in C++

## Table of Contents
- [class vs struct](#class-vs-struct)
- [Basic Class](#basic-class)
- [Access Specifiers](#access-specifiers)
- [Naming Convention: m_ Prefix](#naming-convention-m_-prefix)
- [Constructors and Destructors](#constructors-and-destructors)
- [Overloading](#overloading)
- [Default Values](#default-values)
- [Header vs Implementation](#header-vs-implementation)
- [const Methods](#const-methods)
- [Getters and Setters](#getters-and-setters)
- [Inline Methods](#inline-methods)
- [this Pointer](#this-pointer)
- [Static Members](#static-members)
- [Namespaces](#namespaces)
- [Common Patterns](#common-patterns)
- [Quick Rules](#quick-rules)

## class vs struct

The **only** difference: default access level
- `class`: members are **private** by default
- `struct`: members are **public** by default

Everything else is identical.

**When to use which:**
- **class**: When you need encapsulation and private implementation details (most of the time)
- **struct**: For simple data structures with mostly public members

```cpp
// struct - Simple data, mostly public
struct Point {
  float x;
  float y;
};

// class - Encapsulation, private implementation
class Synthesizer {
public:
  Synthesizer(float sampleRate);
  void setFrequency(float freq);

private:
  float m_sampleRate;
  float m_frequency;
};
```

## Basic Class

```cpp
class Synthesizer {
public:
  Synthesizer(float sampleRate);
  void processBlock(float* output, int numSamples);
  void setFrequency(float freq);

private:
  float m_sampleRate;
  float m_frequency;
  float m_phase;
};
```

## Access Specifiers

Control who can access members:

```cpp
class MyClass {
public:
  // Accessible from anywhere
  void publicMethod();
  int publicData;

private:
  // Only accessible inside this class
  void privateMethod();
  int m_privateData;

protected:
  // Accessible in this class and derived classes
  void protectedMethod();
  int m_protectedData;
};
```

### When to Use Each

**public** - The interface
- Constructors
- Methods that users of your class need to call
- Rarely: data members (usually provide methods instead)

**private** - Implementation details (default for `class`)
- Internal state/data members
- Helper methods users don't need to know about
- Anything you might want to change later

**protected** - Inheritance
- Rarely used (only needed for inheritance)
- Members that derived classes need but users don't

### Why Use private?

**1. Encapsulation** - Hide implementation so you can change it later:

```cpp
class AudioBuffer {
public:
  float getSample(int index);  // Interface stays the same

private:
  float* m_data;  // Could change to std::vector later without breaking user code
};
```

**2. Validation** - Control how data is set:

```cpp
class Oscillator {
public:
  void setFrequency(float freq) {
    if (freq > 0.0f && freq < 20000.0f) {  // Validate!
      m_frequency = freq;
    }
  }

private:
  float m_frequency;  // Can't be set to invalid values from outside
};
```

**3. Maintain Invariants** - Keep object in valid state:

```cpp
class Voice {
public:
  void updatePhase() {
    m_phase += m_phaseIncrement;
    if (m_phase >= 1.0f) {
      m_phase -= 1.0f;  // Always keep phase 0.0-1.0
    }
  }

private:
  float m_phase;           // If public, user could break invariant
  float m_phaseIncrement;
};
```

**4. Clear Interface** - Users only see what matters:

```cpp
class Synthesizer {
public:
  // Clean, simple interface
  void processBlock(float* output, int numSamples);
  void setFrequency(float freq);

private:
  // Users don't need to see this complexity
  float m_sampleRate;
  float m_phase;
  float m_frequency;

  void updatePhase();
  void calculatePhaseIncrement();
  float generateSample();
};
```

### Bad vs Good Examples

**Bad - Everything public:**

```cpp
struct Synthesizer {
  float phase;
  float frequency;
  float sampleRate;
};

Synthesizer synth;
synth.phase = 999.0f;  // Oops! Invalid value
synth.frequency = -100.0f;  // Oops! Negative frequency
```

**Good - Private with controlled access:**

```cpp
class Synthesizer {
public:
  Synthesizer(float sampleRate) : m_sampleRate(sampleRate), m_phase(0.0f) {}

  void setFrequency(float freq) {
    if (freq > 0.0f) {  // Validation
      m_frequency = freq;
    }
  }

  void reset() {
    m_phase = 0.0f;  // Always valid
  }

private:
  float m_sampleRate;
  float m_frequency;
  float m_phase;  // Can't be set to invalid values
};
```

### Rule of Thumb

**Start with everything private, only make public what's necessary.**

1. **Always private**: Internal state, implementation details
2. **Usually private**: Helper methods
3. **Public**: Constructors, interface methods
4. **Avoid public data**: Use getters/setters for controlled access

## Naming Convention: m_ Prefix

Common convention: prefix private member variables with `m_` (for "member"):

```cpp
class Synthesizer {
public:
  void setFrequency(float frequency) {
    m_frequency = frequency;  // m_ prefix = member variable
  }

private:
  float m_sampleRate;   // Clear it's a member variable
  float m_frequency;
  float m_phase;
};
```

**Why?**
- Distinguishes members from local variables
- Makes code more readable
- Not required, just a common style

## Constructors and Destructors

### Constructor

Initializes the object:

```cpp
class Synthesizer {
public:
  // Constructor with member initializer list
  Synthesizer(float sampleRate)
    : m_sampleRate(sampleRate),
      m_frequency(440.0f),
      m_phase(0.0f)
  {}

private:
  float m_sampleRate;
  float m_frequency;
  float m_phase;
};

Synthesizer synth(48000.0f);  // Calls constructor
```

### Destructor

Cleans up when object is destroyed:

```cpp
class AudioBuffer {
public:
  AudioBuffer(int size) {
    m_data = new float[size];  // Allocate memory
    m_size = size;
  }

  ~AudioBuffer() {  // Destructor (note the ~)
    delete[] m_data;  // Free memory
  }

private:
  float* m_data;
  int m_size;
};

// Object destroyed automatically when out of scope
{
  AudioBuffer buffer(1024);  // Constructor called
  // Use buffer...
}  // Destructor called automatically here
```

**Destructor rules:**
- Same name as class with `~` prefix
- No parameters, no return type
- Called automatically when object is destroyed
- Use for cleanup (free memory, close files, etc.)

### Where Constructor Logic Goes

You can define constructors in the header or implementation file:

**Option 1: Inline in header (simple initialization only)**
```cpp
// Voice.h
class Voice {
public:
  Voice(float freq) : m_frequency(freq), m_phase(0.0f) {}  // Simple, all in initializer list
};
```

**Option 2: Declaration in header, definition in .cpp (preferred for consistency)**
```cpp
// Voice.h
class Voice {
public:
  Voice(float freq, float sampleRate = 44100.0f);  // Declare only
};

// Voice.cpp
Voice::Voice(float freq, float sampleRate)
  : m_frequency(freq),
    m_sampleRate(sampleRate)
{
  // Any initialization logic here
  calculatePhaseIncrement();
  validateFrequency();
}
```

**Best practice recommendation:** Always declare in `.h` and define in `.cpp`, even for simple constructors. This keeps all initialization logic in one predictable place, making it easier to find and maintain as complexity grows. While inline definitions in headers work fine for trivial cases, consistency is more valuable than the minor convenience.

## Overloading

You can have multiple functions/methods with the same name but different parameters:

```cpp
class Voice {
public:
  // Different number of parameters
  Voice(float freq);
  Voice(float freq, float sampleRate);

  // Different parameter types
  void setFrequency(float freq);
  void setFrequency(int midiNote);

private:
  float m_frequency;
  float m_sampleRate;
};

// Usage:
Voice v1(440.0f);              // Calls first constructor
Voice v2(440.0f, 48000.0f);    // Calls second constructor

v1.setFrequency(220.0f);       // Calls float version
v1.setFrequency(69);           // Calls int version (MIDI A4)
```

### How It Works

The compiler picks the right version based on the arguments you pass:

```cpp
// Implementation
Voice::Voice(float freq)
  : m_frequency(freq), m_sampleRate(44100.0f) {}

Voice::Voice(float freq, float sampleRate)
  : m_frequency(freq), m_sampleRate(sampleRate) {}

void Voice::setFrequency(float freq) {
  m_frequency = freq;
}

void Voice::setFrequency(int midiNote) {
  m_frequency = 440.0f * std::pow(2.0f, (midiNote - 69) / 12.0f);
}
```

### Rules

- Must differ in number or types of parameters
- Return type alone is NOT enough to distinguish
- Can overload constructors, methods, and regular functions

```cpp
// Valid overloads
void process(float value);
void process(float value, int sampleRate);
void process(int value);

// Invalid - only return type differs
float getValue();
int getValue();  // ✗ ERROR
```

### Common Use Cases

**Constructors with different initialization:**
```cpp
AudioBuffer(int size);                    // Empty buffer
AudioBuffer(int size, float* data);       // Pre-filled buffer
```

**Methods that accept different input types:**
```cpp
void setGain(float linearGain);
void setGain(int decibels);
```

## Default Values

There are two ways to give member variables default values:

### Option 1: In-Class Initialization (Preferred)

Define defaults directly in the class declaration:

```cpp
class Voice {
public:
  Voice(float sampleRate) : m_sampleRate(sampleRate) {}

private:
  float m_sampleRate;          // Set by constructor
  float m_frequency = 440.0f;  // Default value
  float m_phase = 0.0f;        // Default value
};
```

**Advantages:**
- Clear and concise - defaults are visible at a glance
- Less repetition if you have multiple constructors
- Modern C++ (C++11+) standard approach

### Option 2: Member Initializer List

Set defaults in the constructor:

```cpp
class Voice {
public:
  Voice(float sampleRate)
    : m_sampleRate(sampleRate),
      m_frequency(440.0f),
      m_phase(0.0f)
  {}

private:
  float m_sampleRate;
  float m_frequency;
  float m_phase;
};
```

**When to use:**
- When the default depends on constructor parameters
- In older C++ codebases (pre-C++11)

### Combining Both

You can mix approaches - in-class defaults can be overridden in the initializer list:

```cpp
class Voice {
public:
  // Default constructor uses in-class defaults
  Voice(float sampleRate) : m_sampleRate(sampleRate) {}

  // This constructor overrides the frequency default
  Voice(float sampleRate, float freq)
    : m_sampleRate(sampleRate),
      m_frequency(freq)  // Overrides the 440.0f default
  {}

private:
  float m_sampleRate;
  float m_frequency = 440.0f;  // Default for when not specified
  float m_phase = 0.0f;
};
```

### Default Function Parameters

You can also use default parameters in constructors:

```cpp
class Voice {
public:
  // sampleRate has a default value
  Voice(float freq, float sampleRate = 44100.0f)
    : m_frequency(freq), m_sampleRate(sampleRate) {}

private:
  float m_frequency;
  float m_sampleRate;
};

// Usage:
Voice v1(440.0f, 48000.0f);  // Explicit sample rate
Voice v2(440.0f);            // Uses default 44100.0f
```

**Rules:**
- Defaults must come last: `func(int required, int optional = 10)` ✓
- **Must be in header declaration, not .cpp definition** - compiler needs to see defaults at call site
- Can have multiple: `func(int a = 1, int b = 2, int c = 3)`

```cpp
// Voice.h
Voice(float freq, float sampleRate = 44100.0f);  // ✓ Default here

// Voice.cpp
Voice::Voice(float freq, float sampleRate)  // ✓ No default here
  : m_frequency(freq), m_sampleRate(sampleRate) {}
```

**Default parameter vs overloaded constructors:**

For simple cases, default parameters are cleaner:
```cpp
// Simpler - one constructor
Voice(float freq, float sampleRate = 44100.0f);
```

Use overloaded constructors when they do different initialization:
```cpp
// Use when constructors have different logic
Voice(float freq) : m_frequency(freq) {
  // Do extra setup for simple case
}

Voice(float freq, float sampleRate)
  : m_frequency(freq), m_sampleRate(sampleRate) {
  // Different initialization logic
}
```

### Best Practice

**Use in-class initialization for true default values.** Use the initializer list for values that must be set by the constructor or depend on constructor parameters.

```cpp
class Synthesizer {
public:
  Synthesizer(float sampleRate)
    : m_sampleRate(sampleRate)  // Required parameter
  {}

private:
  float m_sampleRate;           // No default - must be provided
  float m_frequency = 440.0f;   // Sensible default
  float m_phase = 0.0f;         // Always starts at zero
  float m_gain = 1.0f;          // Default unity gain
};
```

## Header vs Implementation

### Header (.h) - Interface

```cpp
#pragma once

class Synthesizer {
public:
  Synthesizer(float sampleRate);
  void processBlock(float* output, int numSamples);
  void setFrequency(float freq);
  float getFrequency() const;

private:
  float m_sampleRate;
  float m_frequency;
  float m_phase;

  void updatePhase();  // Private helper method
};
```

### Implementation (.cpp) - Definitions

```cpp
#include "Synthesizer.h"
#include <cmath>

Synthesizer::Synthesizer(float sampleRate)
  : m_sampleRate(sampleRate),
    m_frequency(440.0f),
    m_phase(0.0f)
{}

void Synthesizer::processBlock(float* output, int numSamples) {
  for (int i = 0; i < numSamples; ++i) {
    output[i] = std::sin(m_phase * 2.0f * M_PI);
    updatePhase();
  }
}

void Synthesizer::setFrequency(float freq) {
  m_frequency = freq;
}

float Synthesizer::getFrequency() const {
  return m_frequency;
}

void Synthesizer::updatePhase() {
  m_phase += m_frequency / m_sampleRate;
  if (m_phase >= 1.0f) {
    m_phase -= 1.0f;
  }
}
```

**The `::` operator**: Scope resolution - `Synthesizer::processBlock` means "the `processBlock` method that belongs to `Synthesizer`"

## const Methods

Methods that don't modify the object should be marked `const`:

```cpp
class Synthesizer {
public:
  // const method - promises not to modify member variables
  float getFrequency() const {
    return m_frequency;
  }

  // Non-const method - can modify members
  void setFrequency(float freq) {
    m_frequency = freq;
  }

private:
  float m_frequency;
};

const Synthesizer synth(440.0f);
float freq = synth.getFrequency();  // OK - const method
synth.setFrequency(220.0f);         // ERROR - can't call non-const method on const object
```

**What `const` means:** The `const` at the end of a method signature is a promise to the compiler that this method will not modify any member variables. This allows:
- Better compiler optimization
- Calling the method on `const` objects
- Clear documentation of intent - "this is read-only"

**Best practice:** Mark all methods that don't modify state as `const`. Getters should be `const`, setters should not.

## Getters and Setters

Common pattern for controlled access to private data:

```cpp
class Voice {
public:
  // Getter - const method
  float getFrequency() const {
    return m_frequency;
  }

  // Setter - validates input
  void setFrequency(float freq) {
    if (freq > 0.0f && freq < 20000.0f) {
      m_frequency = freq;
    }
  }

private:
  float m_frequency = 440.0f;
};
```

**Why not make members public?**
- Can validate input
- Can change internal representation later
- Can add logic (logging, notifications, etc.)

## Inline Methods

Short methods can be defined in the header:

```cpp
class Voice {
public:
  float getFrequency() const {
    return m_frequency;  // Simple getter - inline is fine
  }

  void setFrequency(float freq);  // Complex logic - define in .cpp

private:
  float m_frequency;
};
```

Methods defined inside the class declaration are implicitly `inline`.

## this Pointer

Every member function has access to `this`, a pointer to the current object:

```cpp
class Voice {
public:
  void setFrequency(float frequency) {
    this->m_frequency = frequency;  // Explicit use of 'this'
    m_frequency = frequency;         // Same thing, 'this->' is implicit
  }

  Voice& reset() {
    m_phase = 0.0f;
    return *this;  // Return reference to self for chaining
  }

private:
  float m_frequency;
  float m_phase;
};

Voice v(440.0f);
v.reset().setFrequency(220.0f);  // Method chaining
```

## Static Members

Members that belong to the class itself, not instances:

```cpp
class Synthesizer {
public:
  static float getMaxFrequency() {
    return s_maxFrequency;
  }

  static void setMaxFrequency(float freq) {
    s_maxFrequency = freq;
  }

private:
  static float s_maxFrequency;  // Shared by all instances
  float m_frequency;             // Each instance has its own
};

// Define static member (in .cpp file)
float Synthesizer::s_maxFrequency = 20000.0f;

// Call without an instance
float max = Synthesizer::getMaxFrequency();
```

**Common convention:** Prefix static members with `s_`

## Namespaces

Namespaces group related code and prevent name collisions:

```cpp
// Voice.h
namespace Synth {
  class Voice {
  public:
    Voice(float freq);
    void process();

  private:
    float m_frequency;
  };
}

// Envelope.h
namespace Synth {
  class Envelope {
  public:
    float getValue() const;

  private:
    float m_value;
  };
}

// Usage
Synth::Voice voice(440.0f);
Synth::Envelope env;
```

### Key Points

**Namespaces are "open"** - you can add to them from any file:
```cpp
// All files with `namespace Synth { }` contribute to the same namespace
```

**Mirror directory structure** for organization:
```cpp
src/synth/Voice.h     → namespace Synth { class Voice }
src/synth/Envelope.h  → namespace Synth { class Envelope }
src/utils/Math.h      → namespace Utils { class Math }
```

**Don't over-nest** - keep it simple:
```cpp
// Good
namespace Synth {
  class Voice { };
}
// Used as: Synth::Voice

// Avoid - too verbose
namespace Synth {
  namespace Voice {
    class Oscillator { };
  }
}
// Used as: Synth::Voice::Oscillator (getting long)
```

**Using declarations** (use sparingly):
```cpp
// In .cpp files only (never in headers!)
using Synth::Voice;

Voice v(440.0f);  // No need for Synth:: prefix
```

## Common Patterns

### Simple Data Class

```cpp
class Point {
public:
  Point(float x, float y) : m_x(x), m_y(y) {}

  float getX() const { return m_x; }
  float getY() const { return m_y; }

  void setX(float x) { m_x = x; }
  void setY(float y) { m_y = y; }

private:
  float m_x;
  float m_y;
};
```

### Class with Resource Management

```cpp
class AudioBuffer {
public:
  AudioBuffer(int size)
    : m_size(size) {
    m_data = new float[size];
  }

  ~AudioBuffer() {
    delete[] m_data;
  }

  float* getData() { return m_data; }
  int getSize() const { return m_size; }

private:
  float* m_data;
  int m_size;
};
```

### Class with Private Helper Methods

```cpp
class Synthesizer {
public:
  Synthesizer(float sampleRate);
  void processBlock(float* output, int numSamples);

private:
  float m_sampleRate;
  float m_phase;

  // Private helpers
  void updatePhase();
  float generateSample();
};
```

## Quick Rules

1. **Use `class`** when you need private members (most of the time)
2. **Use `struct`** for simple data structures with mostly public members
3. **Mark member variables private** and provide getters/setters if needed
4. **Use `m_` prefix** for private member variables (common convention)
5. **Mark methods `const`** if they don't modify state
6. **Split header/implementation** for complex classes
7. **Always use `#pragma once`** in headers
8. **Use namespaces** to organize related classes and prevent name collisions
