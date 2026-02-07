#include "platform/AudioProcessor.h"
#include "platform/NoteEventQueue.h"
#include "synth/Engine.h"

#include <audio_io/AudioIO.h>
#include <csignal>

static void processEvent(platform::NoteEvent event, void *myContext) {
  auto engine = static_cast<Synth::Engine *>(myContext);
  engine->processEvent(event);
}

static void processBlock(float **outputBuffer, size_t numChannels,
                         size_t numFrames, void *myContext) {
  auto engine = static_cast<Synth::Engine *>(myContext);
  engine->processBlock(outputBuffer, numChannels, numFrames);
}

int main() {
  constexpr float SAMPLE_RATE = 48000.0f;

  // 1. Setup synth engine
  Synth::Engine engine{SAMPLE_RATE, Synth::OscillatorType::Sine};

  // 2. Setup audio_io
  platform::AudioConfig config{};
  config.sampleRate = static_cast<uint32_t>(SAMPLE_RATE);

  platform::setupAudioProcess(config, processEvent, processBlock, &engine);
  return 0;
}
