#include "synth/Engine.h"
#include "synth/Oscillator.h"
#include "synth/Voice.h"
#include "utils/NoteEventQueue.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <vector>

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

void Engine::processEvent(const utils::NoteEvent &event) {
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

std::vector<float> Engine::process(const NoteEventSequence &evtSequence,
                                   float stepDuration) {

  int samplesPerStep{static_cast<int>(stepDuration * mSampleRate)};
  int totalSamples{samplesPerStep * static_cast<int>(evtSequence.size())};

  std::vector<float> buffer{};
  buffer.reserve(static_cast<size_t>(totalSamples));

  float releaseMs = 200.0f; // hardcoded for now.  will be dynamic later

  int releaseSamples = static_cast<int>((releaseMs / 1000.0f) * mSampleRate);

  int lastSampleIndex = samplesPerStep - 1; // adjust for 0 index
  int noteOffSample = lastSampleIndex - releaseSamples;

  // Sequence of note(s)
  for (const auto &noteEvtGroup : evtSequence) {
    int activeVoices{0};

    size_t voiceIndex{0};
    for (auto &noteEvent : noteEvtGroup) {
      if (voiceIndex < mVoices.size() && mVoices[voiceIndex].isAvailable()) {
        mVoices[voiceIndex].noteOn(noteEvent);
        ++voiceIndex;
        ++activeVoices;
      }
    }

    // Render per sample note(s) values
    for (int i = 0; i < samplesPerStep; ++i) {
      float sampleValue{0.0f}; // amplitude (0.0 - 1.0)

      if (i == noteOffSample) {
        for (auto &voice : mVoices) {
          // Only "turn off" active voices
          if (!voice.isAvailable())
            voice.noteOff();
        }
      }

      // Share overall gain across each voice to avoid clipping
      const float gainPerVoice =
          DEFAULT_AMPLITUDE / static_cast<float>(activeVoices);

      for (auto &voice : mVoices) {
        sampleValue += voice.process() * gainPerVoice;
      }

      buffer.push_back(sampleValue);
    }
  }

  return buffer;
}

// Helper methods
// `void Engine::updateMaxReleaseTime() {
// `  float maxRelease{};
// `
// `  for (const auto &voice : mVoices) {
// `    // TODO
// `  }
// `}

} // namespace Synth
