#pragma once

#include "audio_io/include/audio_io/AudioIOTypesFwd.h"
#include "audio_io/src/AudioIOMacros.h"

#include <AudioToolbox/AudioToolbox.h>
#include <cstdint>

namespace CoreAudioAdapter {

AudioStreamBasicDescription configToASBD(const audio_io::Config &config);

PLATFORM_SETUP(coreAudioSetup);
PLATFORM_START(coreAudioStart);
PLATFORM_STOP(coreAudioStop);
PLATFORM_CLEANUP(coreAudioCleanup);

} // namespace CoreAudioAdapter
