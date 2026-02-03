#pragma once

#include "AudioIOTypesFwd.h"
#include "audio_io/src/AudioIOMacros.h"

namespace audio_io {

using AudioCallback = void (*)(AudioBuffer buffer, void *context);

audio_io::hAudioSession setupAudioSession(const audio_io::Config &userConfig,
                                          audio_io::AudioCallback userCallback,
                                          void *userContext);

PLATFORM_START(startAudioSession);
PLATFORM_STOP(stopAudioSession);
PLATFORM_CLEANUP(cleanupAudioSession);

} // namespace audio_io
