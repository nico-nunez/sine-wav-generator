#pragma once

#include "NoteEventQueue.h"
#include "audio_io/AudioIOTypes.h"
#include <cstdint>

namespace platform {

// --- Constants ---
inline constexpr uint32_t DEFAULT_SAMPLE_RATE = 48000;
inline constexpr uint32_t DEFAULT_FRAMES = 512;
inline constexpr uint16_t DEFAULT_CHANNELS = 2;

enum class BufferFormat {
  NonInterleaved, // channels in separate arrays [LLLL] [RRRR]
  Interleaved,    // channels interwoven in single array [LRLRLRLR]
};

struct Config {
  uint32_t sampleRate = DEFAULT_SAMPLE_RATE;
  uint32_t numFrames = DEFAULT_FRAMES;
  uint16_t numChannels = DEFAULT_CHANNELS;
  BufferFormat bufferFormat = BufferFormat::NonInterleaved;
};

using AudioConfig = audio_io::Config;

typedef void (*EventHandler)(NoteEvent noteEvent, void *userContext);
typedef void (*BufferHandler)(float **outputBuffer, size_t numChannels,
                              size_t numFrames, void *userContext);

void setupAudioProcess(AudioConfig userConfig, EventHandler eventHandler,
                       BufferHandler bufferHandler, void *userContext);

} // namespace platform
