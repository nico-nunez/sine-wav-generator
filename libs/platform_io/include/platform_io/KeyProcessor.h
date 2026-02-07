#pragma once

#include "NoteEventQueue.h"

namespace platform_io {
int startKeyInputCapture(NoteEventQueue &eventQueue);

MIDINote asciiToMidi(char key);
} // namespace platform_io
