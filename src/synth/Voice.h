#ifndef SYNTH_VOICE_H
#define SYNTH_VOICE_H

namespace Synth {
class Voice {
public:
  Voice(float freq, float sampleRate = 44100.0f);

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

} // namespace Synth

#endif
