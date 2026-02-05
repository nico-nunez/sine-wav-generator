#ifndef SYNTH_ENGINE_H
#define SYNTH_ENGINE_H

#include "synth/Oscillator.h"
#include "synth/Voice.h"
#include "utils/NoteEventQueue.h"
#include <array>
#include <string>
#include <vector>

namespace Synth {

using NoteEventGroup = std::vector<utils::NoteEvent>;
using NoteEventSequence = std::vector<NoteEventGroup>;
using NoteSequence = std::vector<std::vector<std::string>>;

class Engine {
public:
  static constexpr int MAX_VOICES{10};

  Engine(const float sampleRate = DEFAULT_SAMPLE_RATE,
         const OscillatorType oscType = OscillatorType::Sine);

  void setOscillatorType(const OscillatorType oscType);
  void getOscillatorType() const;

  void processEvent(const utils::NoteEvent &event);

  // TODO(nico): Implement this!!!
  void processBlock(float **outputBuffer, size_t numChannels, size_t numFrames);

  // TODO(nico): Remove this once processBlock is implemented
  std::vector<float> process(const NoteEventSequence &evtSequence,
                             float totalDuration);

private:
  float mSampleRate;

  OscillatorType mOscillatorType{};
  std::array<Voice, MAX_VOICES> mVoices;

  float maxReleaseTime{};

  void updateMaxReleaseTime();
};

} // namespace Synth

#endif
