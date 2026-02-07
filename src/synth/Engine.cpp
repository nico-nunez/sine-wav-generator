#include "synth/Engine.h"
#include "platform_io/NoteEventQueue.h"
#include "synth/Oscillator.h"
#include "synth/Voice.h"

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
  // NOTE(nico): Non-Interleaved for now
  for (size_t frame = 0; frame < numFrames; frame++) {
    float sampleValue = 0;
    for (auto &voice : mVoices) {
      if (voice.isAvailable())
        continue;

      sampleValue += voice.process() * DEFAULT_AMPLITUDE;
    }
    for (size_t ch = 0; ch < numChannels; ch++) {
      outputBuffer[ch][frame] = sampleValue;
    }
  }
}

} // namespace Synth
