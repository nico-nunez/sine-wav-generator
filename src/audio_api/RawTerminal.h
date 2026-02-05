#pragma once

#include <cstdint>

namespace audio_api {
struct NoteEventQueue;

void enableRawTerminal();
void disableRawTerminal();

uint8_t asciiToMidi(char key);
void captureKeyboardInputs(NoteEventQueue &eventQueue);
} // namespace audio_api
