#ifndef SYNTH_OSCILLATOR_H
#define SYNTH_OSCILLATOR_H

#include "utils/Waveform.h"
#include <vector>

namespace Synth {
class Oscillator {
public:
  Oscillator(float freq, float sampleRate = 44100.0f,
             Waveforms::WaveformFunc waveformFunc = Waveforms::sine);

  void setFrequency(float freq);
  float getFrequency() const;

  void setSampleRate(float sampleRate);
  float getSampleRate() const;

  float getNextSampleValue();

private:
  float m_frequency;
  float m_phase = 0.0f;
  float m_phaseIncrement;
  float m_sampleRate;

  Waveforms::WaveformFunc m_waveformFunc = nullptr;

  void incrementPhase();
  void calculatePhaseIncrement();
};

using OscillatorGroup = std::vector<Oscillator>;

} // namespace Synth

#endif
