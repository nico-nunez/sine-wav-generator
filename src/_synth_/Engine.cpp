#include "Engine.h"
#include "_synth_/VoicePool.h"
#include <cstdio>

namespace synth {
Engine createEngine(const EngineConfig &config) {
  Engine engine{};
  voices::updateVoicePoolConfig(engine.voicePool, config);

  return engine;
}

void Engine::processEvent(const platform_io::NoteEvent &event) {
  if (!event.midiNote)
    return;

  if (event.type == platform_io::NoteEventType::NoteOff) {
    voices::releaseVoice(voicePool, event.midiNote);
  } else {
    voices::handleNoteOn(voicePool, event.midiNote, event.velocity, noteCount++,
                         sampleRate);
  }
}

void Engine::processBlock(float **outputBuffer, size_t numChannels,
                          size_t numFrames) {
  voices::processVoices(voicePool, poolBuffer, numFrames);

  for (size_t frame = 0; frame < numFrames; frame++) {
    for (size_t ch = 0; ch < numChannels; ch++) {
      outputBuffer[ch][frame] = poolBuffer[frame];
    }
  }
}

} // namespace synth
