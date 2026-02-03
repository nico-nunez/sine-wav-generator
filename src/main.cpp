#include "synth/Engine.h"
#include "utils/SynthUtils.h"
// #include "utils/WavWriter.h"

#include <atomic>
#include <audio_io/include/audio_io/AudioIO.h>
#include <chrono>
#include <csignal>
#include <thread>

// Test state
struct PlaybackState {
  std::vector<float> buffer;
  std::atomic<size_t> position{0};
};

void audioCallback(audio_io::AudioBuffer buffer, void *context) {
  auto *state = static_cast<PlaybackState *>(context);

  for (uint32_t frame = 0; frame < buffer.numFrames; ++frame) {
    float sample = 0.0f;

    size_t pos = state->position.load();
    if (pos < state->buffer.size()) {
      sample = state->buffer[pos];
      state->position.fetch_add(1);
    }

    // Write mono to all channels
    if (buffer.format == audio_io::BufferFormat::NonInterleaved) {
      for (uint32_t ch = 0; ch < buffer.numChannels; ++ch) {
        buffer.channelPtrs[ch][frame] = sample;
      }
    } else {
      for (uint32_t ch = 0; ch < buffer.numChannels; ++ch) {
        buffer.interleavedPtr[frame * buffer.numChannels + ch] = sample;
      }
    }
  }
}

int main() {
  // 1. Pre-render audio using existing Engine
  constexpr float SAMPLE_RATE = 48000.0f;
  constexpr float NOTE_DURATION = 2.0f;
  Synth::Engine engine(SAMPLE_RATE);

  Synth::NoteSequence noteSequence{
      {"C4", "D#4", "G4"}, {"F4", "G#4", "C5"}, {"G4", "A#5", "D5"}};

  Synth::NoteEventSequence noteEventSequence{};
  noteEventSequence.reserve(noteSequence.size());

  for (const std::vector<std::string> &noteGroup : noteSequence) {
    noteEventSequence.push_back(
        SynthUtils::createEventGroupFromNotes(noteGroup));
  }

  /* Generate waveform samples
   * Audio is just an array of values representing air pressure over time
   * Values: floats (0.0 - 1.0)
   */
  Synth::Engine synthEngine{SAMPLE_RATE, Synth::OscillatorType::Square};

  PlaybackState state;
  state.buffer = engine.process(noteEventSequence, NOTE_DURATION);

  // 2. Setup audio_io
  audio_io::Config config{};
  config.sampleRate = static_cast<uint32_t>(SAMPLE_RATE);

  auto session = audio_io::setupAudioSession(config, audioCallback, &state);
  audio_io::startAudioSession(session);

  // 3. Wait until done or ctrl+c
  while (state.position.load() < state.buffer.size()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  audio_io::stopAudioSession(session);
  audio_io::cleanupAudioSession(session);

  //   std::vector<float> audioBuffer{
  //       synthEngine.process(noteEventSequence, DURATION_SECONDS)};
  //
  //   int32_t fileSampleRate{static_cast<int32_t>(SAMPLE_RATE)};
  //
  //   WavWriter::writeWavFile("output.wav", audioBuffer, fileSampleRate);

  return 0;
}
