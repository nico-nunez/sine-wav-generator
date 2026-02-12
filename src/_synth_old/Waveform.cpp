#include "Waveform.h"

#include <cmath>
#include <cstdlib>

namespace Waveforms {

float polyBlep(float t) {
  if (t > 0.0f) {
    // We just passed the discontinuity (0 < t < 1)
    return t * t - 2.0f * t + 1.0f; // Polynomial: t² - 2t + 1
  } else {
    // We're approaching the discontinuity (-1 < t < 0)
    return t * t + 2.0f * t + 1.0f; // Polynomial: t² + 2t + 1
  }
}

float sine(float phase, float) { return std::sin(phase * TWO_PI); }

// Naive Square (aliasing at higher harmonics)
float square(float phase, float) { return phase < 0.5f ? 1.0f : -1.0f; }

// PolyBLep Square (reduces aliasing at higher haromincs)
float squarePolyBlep(float phase, float phaseIncrement) {
  float value = phase < 0.5f ? 1.0f : -1.0f;

  // Discontinuity at phase = 0.0 (rising edge)
  if (phase < phaseIncrement) {
    float t = phase / phaseIncrement;
    value += polyBlep(t); // Note: += for rising edge
  } else if (phase > 1.0f - phaseIncrement) {
    float t = (phase - 1.0f) / phaseIncrement;
    value += polyBlep(t);
  }

  // Discontinuity at phase = 0.5 (falling edge)
  if (phase > 0.5f && phase < 0.5f + phaseIncrement) {
    float t = (phase - 0.5f) / phaseIncrement;
    value -= polyBlep(t); // Note: -= for falling edge
  } else if (phase > 0.5f - phaseIncrement && phase < 0.5f) {
    float t = (phase - 0.5f) / phaseIncrement;
    value -= polyBlep(t);
  }

  return value;
}

// Naive Saw (aliasing at higher harmonics)
float saw(float phase, float) { return 2.0f * phase - 1.0f; }

// PolyBLep Saw (reduces aliaasing at higher harmonics)
float sawPolyBlep(float phase, float phaseIncrement) {
  float value = 2.0f * phase - 1.0f; // Naive saw

  // Correct the discontinuity at phase wraparound
  if (phase < phaseIncrement) {
    // Just wrapped, t = how far past the discontinuity
    float t = phase / phaseIncrement;
    value -= polyBlep(t);
  } else if (phase > 1.0f - phaseIncrement) {
    // About to wrap, t = how far before the discontinuity (negative)
    float t = (phase - 1.0f) / phaseIncrement;
    value -= polyBlep(t);
  }

  return value;
}

float triangle(float phase, float) {
  return 4.0f * std::abs(phase - 0.5f) - 1.0f;
}

// NOTE:  using normalized phase results in a cool distorted sound
float sineNormalized(float phase, float) { return std::sin(phase); }

} // namespace Waveforms
