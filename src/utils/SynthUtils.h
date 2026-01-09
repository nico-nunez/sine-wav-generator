#ifndef SYNTH_UTILS
#define SYNTH_UTILS

#include "synth/Engine.h"
#include "synth/Oscillator.h"
#include "utils/Waveform.h"

#include <string>
#include <vector>

namespace SynthUtils {
// Root note == A4
inline constexpr int ROOT_NOTE_MIDI{69};
inline constexpr float ROOT_NOTE_FREQ{440.0f};

void generateSineValues(std::vector<int16_t> &samples, double frequency,
                        const int duration, const int sampleRate);

float semitoneToFrequency(int semitones);

float midiToFrequency(int midiValue);

int noteNameToMidi(std::string_view noteName);

float noteNameToFrequency(std::string_view noteName);

Synth::NoteEvent createNoteEvent(float frequency);

Synth::NoteEventGroup
createEventGroupFromNotes(const std::vector<std::string> &notes);

} // namespace SynthUtils

#endif
