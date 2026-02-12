#include "Utils.h"
#include "Types.h"

#include <cmath>

namespace synth::utils {

// ==== MIDI/Frequency Conversion Helpers ====
float semitoneToFrequency(int semitones) {
  // Pre-caluated value of 2^(1/12)
  static constexpr float SEMITONE_RATIO = 1.059463094f;

  // Cast `semitones` to float since std::pow is optimized for floats(32-bit)
  return ROOT_NOTE_FREQ * static_cast<float>(std::pow(
                              SEMITONE_RATIO, static_cast<float>(semitones)));
}

float midiToFrequency(int midiValue) {
  return semitoneToFrequency(midiValue - ROOT_NOTE_MIDI);
}

// ==== DB Conversion Helpers ====
float dBtoLinear(float dB) { return std::pow(10.0f, dB / 20.0f); }

float linearToDb(float linear) {
  if (linear <= 0.0f)
    return -INFINITY;
  return 20.0f * std::log10f(linear);
}

} // namespace synth::utils
