#include "synth/Engine.h"
#include "audio_api/NoteEventQueue.h"
#include "synth/Oscillator.h"
#include "synth/Voice.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace Synth {
// Set to 0.6 instead of 1.0 for additional headroom to avoid clipping
constexpr float DEFAULT_AMPLITUDE{0.6f};

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

void Engine::processEvent(const audio_api::NoteEvent &event) {
  bool processed = false;

  // Turn off any/all activeVoices
  for (auto &voice : mVoices) {
    if (!voice.isAvailable() && !voice.isReleasing())
      voice.noteOff();

    if (!processed && voice.isAvailable()) {
      voice.noteOn(event);
      processed = true;
    }
  }
}

void Engine::processBlock(float **outputBuffer, size_t numChannels,
                          size_t numFrames) {
  uint32_t activeVoices = 1;

  // Share overall gain across each voice to avoid clipping
  const float gainPerVoice =
      DEFAULT_AMPLITUDE / static_cast<float>(activeVoices);

  // NOTE(nico): Non-Interleaved for now
  for (size_t frame = 0; frame < numFrames; frame++) {
    float sampleValue = 0;
    for (auto &voice : mVoices) {
      if (voice.isAvailable())
        continue;

      sampleValue += voice.process() * gainPerVoice;
    }
    for (size_t ch = 0; ch < numChannels; ch++) {
      outputBuffer[ch][frame] = sampleValue;
    }
  }
}

} // namespace Synth
