#include "dsp/Modulation.h"
#include "dsp/Waveforms.h"
#include <cmath>

namespace dsp::modulation {
// Generate LFO waveform at given phase
float processLFO(float phase, waveforms::WaveformType type, float pulseWidth) {
  // LFOs often centered around 0 with range -1 to +1
  switch (type) {
  case waveforms::WaveformType::Sine:
    return waveforms::sine(phase);
  case waveforms::WaveformType::Saw:
    return waveforms::sawNaive(phase);
  case waveforms::WaveformType::Square:
    return waveforms::squareNaive(phase, pulseWidth);
  case waveforms::WaveformType::Triangle:
    return waveforms::triangle(phase);
  }
}

// TODO(nico): spend more time understanding these...
// Modulation curves (for velocity, mod wheel, etc.)
float exponentialCurve(float linear) {
  // Maps 0-1 linearly to 0-1 exponentially
  return (std::exp(linear * 4.0f) - 1.0f) / (std::exp(4.0f) - 1.0f);
}

float logarithmicCurve(float linear) {
  // Maps 0-1 linearly to 0-1 logarithmically
  return std::log(1.0f + linear * (std::exp(1.0f) - 1.0f));
}

} // namespace dsp::modulation
