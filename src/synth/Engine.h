#ifndef SYNTH_ENGINE_H
#define SYNTH_ENGINE_H

#include "synth/Oscillator.h"
#include "synth/Voice.h"
#include <array>
#include <string>
#include <vector>

namespace Synth {

struct NoteEvent {
  float frequency;

  NoteEvent(float freq) : frequency(freq) {}
};

using NoteEventGroup = std::vector<NoteEvent>;
using NoteEventSequence = std::vector<NoteEventGroup>;
using NoteSequence = std::vector<std::vector<std::string>>;

class Engine {
public:
  static constexpr int MAX_VOICES{10};

  Engine(const float sampleRate = DEFAULT_SAMPLE_RATE,
         const OscillatorType oscType = OscillatorType::Sine);

  void setOscillatorType(const OscillatorType oscType);
  void getOscillatorType() const;

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
