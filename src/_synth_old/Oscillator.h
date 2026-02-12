#ifndef SYNTH_OSCILLATOR_H
#define SYNTH_OSCILLATOR_H

#include "Waveform.h"

namespace Synth {
constexpr float DEFAULT_SAMPLE_RATE = 44100.0f;
constexpr float DEFAULT_FREQUENCY = 440.0f; // A4

enum class OscillatorType {
  Sine,
  Saw,
  Square,
  Triangle,
};

class Oscillator {
public:
  Oscillator(OscillatorType oscType = OscillatorType::Sine,
             float sampleRate = DEFAULT_SAMPLE_RATE);

  void setOscillatorType(OscillatorType oscType);

  void setFrequency(float freq);
  float getFrequency() const;

  void setSampleRate(float sampleRate);
  float getSampleRate() const;

  float process();

private:
  OscillatorType mWaveform;
  Waveforms::WaveformFunc mWaveformFunc = nullptr;

  float mPhase = 0.0f;
  float mPhaseIncrement;

  float mFrequency = DEFAULT_FREQUENCY; // A4

  float mSampleRate;

  void incrementPhase();
  void calcPhaseIncrement();
};

} // namespace Synth

#endif
