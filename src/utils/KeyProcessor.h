#pragma once

#include <cstdint>

namespace synth_io {
struct NoteEventQueue;
struct SynthSession;
} // namespace synth_io

namespace synth::utils {
using hSynthSession = synth_io::SynthSession *;

int startKeyInputCapture(hSynthSession);

uint8_t asciiToMidi(char key);
} // namespace synth::utils
