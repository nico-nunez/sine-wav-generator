#include "Oscillator.h"
#include "synth/Waveform.h"
#include <stdexcept>

namespace Synth {

Oscillator::Oscillator(OscillatorType oscType, float sampleRate)
    : mWaveform(oscType), mSampleRate(sampleRate) {
  setOscillatorType(oscType);
}

void Oscillator::setFrequency(float freq) {
  mPhase = 0;
  mFrequency = freq;
  calcPhaseIncrement();
}

float Oscillator::getFrequency() const { return mFrequency; }

void Oscillator::setSampleRate(float sampleRate) {
  mSampleRate = sampleRate;
  calcPhaseIncrement();
}

float Oscillator::getSampleRate() const { return mSampleRate; }

// Calculate in advance in order to increment the phase each step.
// More efficient than time-based calculation on every increment.
// Introduces (neglibile) drift due to continous add operations using float
void Oscillator::calcPhaseIncrement() {
  mPhaseIncrement = mFrequency / mSampleRate; // normalized
}

// Increament after each sample
void Oscillator::incrementPhase() {
  mPhase += mPhaseIncrement;

  // Wrap phase to prevent float precision issues
  // due to limited number of significant digits (~7)
  if (mPhase >= 1.0f)
    mPhase -= 1.0f;
}

float Oscillator::process() {
  incrementPhase();
  return mWaveformFunc(mPhase, mPhaseIncrement);
}

void Oscillator::setOscillatorType(OscillatorType oscType) {
  switch (oscType) {
  case OscillatorType::Sine:
    mWaveformFunc = Waveforms::sine;
    break;
  case OscillatorType::Saw:
    mWaveformFunc = Waveforms::sawPolyBlep;
    break;
  case OscillatorType::Square:
    mWaveformFunc = Waveforms::squarePolyBlep;
    break;
  case OscillatorType::Triangle:
    mWaveformFunc = Waveforms::triangle;
    break;
  default:
    throw std::runtime_error("Invalid oscillator type");
  }

  mPhase = 0;
  mWaveform = oscType;
  calcPhaseIncrement();
}

} // namespace Synth
