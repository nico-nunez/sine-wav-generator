#pragma once

#include "VoicePool.h"

#include "dsp/Waveforms.h"

#include "platform_io/AudioProcessor.h"
#include "platform_io/NoteEventQueue.h"

#include <cstdint>

namespace synth {
using NoteEvent = platform_io::NoteEvent;

using VoicePool = voices::VoicePool;
using VoiceConfig = voices::VoicePoolConfig;
using WaveformType = dsp::waveforms::WaveformType;

struct EngineConfig : VoiceConfig {
  float sampleRate = platform_io::DEFAULT_SAMPLE_RATE;
  uint32_t numFrames = platform_io::DEFAULT_FRAMES;
};

struct Engine {
  static constexpr uint32_t NUM_FRAMES = platform_io::DEFAULT_FRAMES;
  float sampleRate = platform_io::DEFAULT_SAMPLE_RATE;

  VoicePool voicePool;

  // TODO(nico): this probably needs to live on heap
  // since the number of frames won't be known at compile time
  float poolBuffer[NUM_FRAMES];

  uint32_t noteCount = 0;

  void processEvent(const NoteEvent &event);
  void processBlock(float **outputBuffer, size_t numChannels, size_t numFrames);
};

Engine createEngine(const EngineConfig &config);

} // namespace synth
