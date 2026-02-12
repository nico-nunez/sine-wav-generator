#ifndef SYNTH_ENGINE_H
#define SYNTH_ENGINE_H

#include "Oscillator.h"
#include "Voice.h"
#include "platform_io/NoteEventQueue.h"

namespace Synth {

class Engine {
public:
  static constexpr int MAX_VOICES{10};

  Engine(const float sampleRate = DEFAULT_SAMPLE_RATE,
         const OscillatorType oscType = OscillatorType::Sine);

  OscillatorType getOscillatorType() const;
  void setOscillatorType(const OscillatorType oscType);

  float getDrive() const;
  void setDrive(float driveValue);

  void processEvent(const platform_io::NoteEvent &event);

  void processBlock(float **outputBuffer, size_t numChannels, size_t numFrames);
  void processBlockFast(float **outputBuffer, size_t numChannels,
                        size_t numFrames);

private:
  float mSampleRate;

  OscillatorType mOscillatorType{};
  Voice mVoices[MAX_VOICES];

  float mDrive{0};
  float mInvNormDrive{0};

  float maxReleaseTime{};

  void updateMaxReleaseTime();
};

} // namespace Synth

#endif
