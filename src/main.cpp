#include "audio_api/NoteEventQueue.h"
#include "audio_api/RawTerminal.h"
#include "synth/Engine.h"

#include <audio_io/AudioIO.h>
#include <csignal>

// Test state
struct PlaybackContext {
  audio_api::NoteEventQueue *eventQueue;
  Synth::Engine *engine;
};

void audioCallback(audio_io::AudioBuffer buffer, void *context) {
  auto *ctx = static_cast<PlaybackContext *>(context);

  audio_api::NoteEvent event;
  while (ctx->eventQueue->pop(event)) {
    ctx->engine->processEvent(event);
  }

  ctx->engine->processBlock(buffer.channelPtrs, buffer.numChannels,
                            buffer.numFrames);
}

int main() {
  audio_api::NoteEventQueue eventQueue{};

  constexpr float SAMPLE_RATE = 48000.0f;

  Synth::Engine engine{SAMPLE_RATE, Synth::OscillatorType::Square};

  PlaybackContext audioContext;
  audioContext.engine = &engine;
  audioContext.eventQueue = &eventQueue;

  // 2. Setup audio_io
  audio_io::Config config{};
  config.sampleRate = static_cast<uint32_t>(SAMPLE_RATE);

  auto session =
      audio_io::setupAudioSession(config, audioCallback, &audioContext);

  audio_io::startAudioSession(session);

  audio_api::enableRawTerminal();
  audio_api::captureKeyboardInputs(eventQueue);

  audio_io::stopAudioSession(session);
  audio_io::cleanupAudioSession(session);

  return 0;
}
