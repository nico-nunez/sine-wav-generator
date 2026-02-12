#include "Oscillator.h"
#include "_synth_/Utils.h"
#include <cmath>
#include <cstdint>

namespace synth::oscillator {

void initOscillator(Oscillator &osc, uint32_t voiceIndex, uint8_t midiNote,
                    float sampleRate) {

  float freq = utils::midiToFrequency(midiNote) *
               std::pow(2.0f, static_cast<float>(osc.octaveOffset)) *
               std::pow(2.0f, osc.detuneAmount / 1200.0f);

  osc.phases[voiceIndex] = 0.0f;
  osc.phaseIncrements[voiceIndex] = freq / sampleRate;
}

// void recalcActiveVoices(Oscillator &osc, const uint8_t midiNotes[],
//                         const uint8_t isActive[], float sampleRate) {
//   // TODO(nico-nunez): coming soon....
// }

// TODO(nico): are the following even necessary
// given it's up to the caller to recalculate?
void setWaveformType(Oscillator &osc, WaveformType newType) {
  osc.waveform = newType;
  // NOTE(nico): don't reset active voices or update anything else
  // Caller can do this if they want but shouldn't on active voices (clicks)
}

void setMixLevel(Oscillator &osc, float newLevel) {
  // Ensure values falls within valid range (0 - 4.0)
  if (newLevel < 0.0f) {
    newLevel = 0.0f;
  } else if (newLevel > 4.0f) {
    newLevel = 4.0f;
  }

  osc.mixLevel = newLevel;
}

void setOctiveOffset(Oscillator &osc, int8_t newOffest) {
  osc.octaveOffset = newOffest;
  // NOTE(nico): recalculation needs to be triggerd by caller to update active
  // voices
}

void setDetuneAmount(Oscillator &osc, float newDetuneAmount) {
  osc.detuneAmount = newDetuneAmount;
  // NOTE(nico): recalculation needs to be triggerd by caller to update active
  // voices
}

void toggleEnabled(Oscillator &osc, bool isEnabled) { osc.enabled = isEnabled; }

void incrementPhase(Oscillator &osc, uint32_t voiceIndex) {
  osc.phases[voiceIndex] += osc.phaseIncrements[voiceIndex];

  // Wrap index
  if (osc.phases[voiceIndex] >= 1.0f)
    osc.phases[voiceIndex] -= 1.0f;
}

float processOscillator(Oscillator &osc, uint32_t voiceIndex) {
  float sample =
      dsp::waveforms::processWaveform(osc.waveform, osc.phases[voiceIndex],
                                      osc.phaseIncrements[voiceIndex]) *
      osc.mixLevel;

  incrementPhase(osc, voiceIndex);

  return sample;
}

} // namespace synth::oscillator
