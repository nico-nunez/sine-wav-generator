#pragma once

#include "platform/NoteEventQueue.h"

namespace input_io {
struct NoteEventQueue;

void enableRawTerminal();
void disableRawTerminal();

platform::MIDINote asciiToMidi(char key);
void captureKeyboardInputs(platform::NoteEventQueue &eventQueue);
} // namespace input_io
