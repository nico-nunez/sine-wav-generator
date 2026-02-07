#include "platform/AudioProcessor.h"
#include "platform/KeyProcessor.h"
#include "platform/NoteEventQueue.h"

#include "audio_io/AudioIO.h"
#include "audio_io/AudioIOTypes.h"

namespace platform {
// Test state
struct AudioContext {
  NoteEventQueue *eventQueue;
  EventHandler processEvent;
  BufferHandler processBlock;
  void *userContext;
};

static void audioCallback(audio_io::AudioBuffer buffer, void *context) {
  auto *ctx = static_cast<AudioContext *>(context);

  NoteEvent event;
  while (ctx->eventQueue->pop(event)) {
    ctx->processEvent(event, ctx->userContext);
  }

  ctx->processBlock(buffer.channelPtrs, buffer.numChannels, buffer.numFrames,
                    ctx->userContext);
}

void setupAudioProcess(AudioConfig userConfig, EventHandler eventHandler,
                       BufferHandler bufferHandler, void *userContext) {

  NoteEventQueue eventQueue{};

  AudioContext callbackContext{&eventQueue, eventHandler, bufferHandler,
                               userContext};

  // 2. Setup audio_io
  audio_io::Config config{};
  config.sampleRate = userConfig.sampleRate;
  config.numChannels = userConfig.numChannels;
  config.numFrames = userConfig.numFrames;
  config.bufferFormat = userConfig.bufferFormat;

  auto session =
      audio_io::setupAudioSession(config, audioCallback, &callbackContext);

  audio_io::startAudioSession(session);

  // audio_io::captureKeyEvents(eventQueue);
  startKeyInputCapture(eventQueue);

  audio_io::stopAudioSession(session);
  audio_io::cleanupAudioSession(session);
};

} // namespace platform
