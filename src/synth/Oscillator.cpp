#include "Oscillator.h"
#include "utils/Waveform.h"
#include <cmath>

namespace Synth {

Oscillator::Oscillator(float freq, float sampleRate,
                       Waveforms::WaveformFunc waveFunc)
    : m_frequency(freq), m_sampleRate(sampleRate), m_waveformFunc(waveFunc) {
  calculatePhaseIncrement();
}

// Change in frequency requires recalculation of phase increment
// Thus member is private and updated via method
void Oscillator::setFrequency(float freq) {
  m_frequency = freq;
  calculatePhaseIncrement();
}
float Oscillator::getFrequency() const { return m_frequency; }

// Change in sample rate requires recalculation of phase increment
// Thus member is private and updated via method
void Oscillator::setSampleRate(float sampleRate) {
  m_sampleRate = sampleRate;
  calculatePhaseIncrement();
}
float Oscillator::getSampleRate() const { return m_sampleRate; }

// Calculate in advance in order to increment the phase each step.
// More efficient than time-based calculation on every increment.
// Introduces (neglibile) drift due to continous add operations using float
void Oscillator::calculatePhaseIncrement() {
  m_phaseIncrement = m_frequency / m_sampleRate; // normalized
}

// Increament after each sample
void Oscillator::incrementPhase() {
  m_phase += m_phaseIncrement;

  // Wrap phase to prevent float precision issues
  // due to limited number of significant digits (~7)
  if (m_phase >= 1.0f)
    m_phase -= 1.0f;
}

float Oscillator::getNextSampleValue() {
  incrementPhase();
  return m_waveformFunc(m_phase);
}

} // namespace Synth
