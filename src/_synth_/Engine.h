#pragma once

#include "_synth_/VoicePool.h"
#include "platform_io/AudioProcessor.h"
#include <cstdint>

namespace synth {
struct Engine {
  static constexpr int32_t NUM_FRAMES = platform_io::DEFAULT_FRAMES;
  float sampleRate = platform_io::DEFAULT_SAMPLE_RATE;

  voices::VoicePool voicePool;
  float poolBuffer[NUM_FRAMES];

  uint32_t noteCount = 0;

  void processEvent(const platform_io::NoteEvent &event);
  void processBlock(float **outputBuffer, size_t numChannels, size_t numFrames);
};
} // namespace synth
