#pragma once

#include "platform_io/NoteEventQueue.h"

namespace device_io {
struct NoteEventQueue;

void enableRawTerminal();
void disableRawTerminal();

void captureKeyboardInputs(platform_io::NoteEventQueue &eventQueue);
} // namespace device_io
