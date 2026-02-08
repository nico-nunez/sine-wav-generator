#include "synth/Engine.h"
#include "platform_io/NoteEventQueue.h"
#include "synth/Oscillator.h"
#include "synth/Voice.h"

#include <cmath>
#include <cstddef>
#include <cstdio>

namespace Synth {
// Set to 0.6 instead of 1.0 for additional headroom to avoid clipping
constexpr float DEFAULT_AMPLITUDE{0.3f};

Engine::Engine(const float sampleRate, const OscillatorType oscType)
    : mSampleRate(sampleRate), mOscillatorType(oscType) {
  // Set sample rate and oscillator types for all voices
  for (auto &voice : mVoices) {
    voice.setSampleRate(sampleRate);
    voice.setOscillatorType(oscType);
  }
}

void Engine::setOscillatorType(const OscillatorType oscType) {
  mOscillatorType = oscType;

  for (Voice &voice : mVoices) {
    voice.setOscillatorType(mOscillatorType);
  }
}

void Engine::processEvent(const platform_io::NoteEvent &event) {
  if (!event.midiNote)
    return;

  if (event.type == platform_io::NoteEventType::NoteOff) {
    // Find and turn off Voice playing the note
    for (auto &voice : mVoices) {
      if (voice.shouldStop(event.midiNote)) {
        voice.noteOff();
      }
    }
  } else {
    // TODO(nico-nunez): implement Voice stealing
    for (auto &voice : mVoices) {
      if (voice.isAvailable()) {
        voice.noteOn(event);
        break;
      }
    }
  }
}

void Engine::processBlock(float **outputBuffer, size_t numChannels,
                          size_t numFrames) {
  float invNorm = mDrive >= 1 ? 1.0f / std::tanh(mDrive) : 0;

  // NOTE(nico): Non-Interleaved for now
  for (size_t frame = 0; frame < numFrames; frame++) {
    float sampleValue = 0;
    for (auto &voice : mVoices) {
      if (voice.isAvailable())
        continue;

      sampleValue += voice.process() * DEFAULT_AMPLITUDE;
    }

    float finalSample = invNorm > 0 ? std::tanh(sampleValue * mDrive) * invNorm
                                    : std::tanh(sampleValue);

    for (size_t ch = 0; ch < numChannels; ch++) {
      outputBuffer[ch][frame] = finalSample;
    }
  }
}

// Polynomial approximation of tanh
float tanhFast(float x) {
  return (x * (27.0f + x * x)) / (27.0f + 9.0f * x * x);
}

// void Engine::processBlockFast(float **outputBuffer, size_t numChannels,
//                                    size_t numFrames) {
//   float normalization = tanhFast(mDrive);
//   float invNormalization = 1.0f / normalization;
//
//   // NOTE(nico): Non-Interleaved for now
//   for (size_t frame = 0; frame < numFrames; frame++) {
//     float sampleValue = 0;
//     for (auto &voice : mVoices) {
//       if (voice.isAvailable())
//         continue;
//
//       sampleValue += voice.process() * DEFAULT_AMPLITUDE;
//     }
//
//     float drivenSample = sampleValue * mDrive;
//     float clipped = tanhFast(drivenSample);
//     float finalSample = clipped * invNormalization;
//
//     for (size_t ch = 0; ch < numChannels; ch++) {
//       outputBuffer[ch][frame] = finalSample;
//     }
//   }
// }
} // namespace Synth
