#ifndef SYNTH_OSCILLATOR_H
#define SYNTH_OSCILLATOR_H

#include <vector>

namespace Synth {
class Oscillator {
public:
  Oscillator(float freq, float sampleRate = 44100.0f);

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

  void incrementPhase();
  void calculatePhaseIncrement();
};

using OscillatorGroup = std::vector<Oscillator>;

} // namespace Synth

#endif
