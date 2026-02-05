#ifndef SYNTH_ENGINE_H
#define SYNTH_ENGINE_H

#include "audio_api/NoteEventQueue.h"
#include "synth/Oscillator.h"
#include "synth/Voice.h"

namespace Synth {

class Engine {
public:
  static constexpr int MAX_VOICES{10};

  Engine(const float sampleRate = DEFAULT_SAMPLE_RATE,
         const OscillatorType oscType = OscillatorType::Sine);

  void setOscillatorType(const OscillatorType oscType);
  void getOscillatorType() const;

  void processEvent(const audio_api::NoteEvent &event);

  void processBlock(float **outputBuffer, size_t numChannels, size_t numFrames);

private:
  float mSampleRate;

  OscillatorType mOscillatorType{};
  Voice mVoices[MAX_VOICES];

  float maxReleaseTime{};

  void updateMaxReleaseTime();
};

} // namespace Synth

#endif
