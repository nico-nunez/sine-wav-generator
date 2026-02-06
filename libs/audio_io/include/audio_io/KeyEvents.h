#pragma once

#include "audio_api/NoteEventQueue.h"

namespace audio_io {
int setupKeyCapture();
int captureKeyEvents(audio_api::NoteEventQueue &eventQueue);
} // namespace audio_io
