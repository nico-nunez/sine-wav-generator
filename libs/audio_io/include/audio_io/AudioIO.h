#pragma once

#include "AudioIOTypesFwd.h"
#include "audio_io/src/AudioIOMacros.h"

namespace audio_io {

using AudioCallback = void (*)(AudioBuffer buffer, void *context);

audio_io::hAudioSession setupSession(const audio_io::Config &userConfig,
                                     audio_io::AudioCallback userCallback,
                                     void *userContext);

PLATFORM_START(startAudio);
PLATFORM_STOP(stopAudio);
PLATFORM_CLEANUP(cleanupAudio);

} // namespace audio_io
