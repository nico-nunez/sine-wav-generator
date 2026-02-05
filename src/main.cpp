#include "synth/Engine.h"

#include <audio_io/AudioIO.h>
#include <csignal>

// Test state
struct PlaybackContext {
  utils::NoteEventQueue *eventQueue;
  Synth::Engine *engine;
};

void audioCallback(audio_io::AudioBuffer buffer, void *context) {
  auto *ctx = static_cast<PlaybackContext *>(context);

  utils::NoteEvent event;
  while (ctx->eventQueue->pop(event)) {
    ctx->engine->processEvent(event);
  }

  ctx->engine->processBlock(buffer.channelPtrs, buffer.numChannels,
                            buffer.numFrames);
}

#include "utils/NoteEventQueue.h"
#include "utils/RawTerminal.h"

int main() {
  utils::NoteEventQueue eventQueue{};

  // eventQueue.printQueue();

  constexpr float SAMPLE_RATE = 48000.0f;

  Synth::Engine engine{SAMPLE_RATE, Synth::OscillatorType::Square};

  // /* Generate waveform samples
  //  * Audio is just an array of values representing air pressure over time
  //  * Values: floats (0.0 - 1.0)
  //  */

  PlaybackContext audioContext;
  audioContext.engine = &engine;
  audioContext.eventQueue = &eventQueue;

  // 2. Setup audio_io
  audio_io::Config config{};
  config.sampleRate = static_cast<uint32_t>(SAMPLE_RATE);

  auto session =
      audio_io::setupAudioSession(config, audioCallback, &audioContext);

  audio_io::startAudioSession(session);

  utils::enableRawTerminal();
  utils::captureKeyboardInputs(eventQueue);

  audio_io::stopAudioSession(session);
  audio_io::cleanupAudioSession(session);

  return 0;
}
