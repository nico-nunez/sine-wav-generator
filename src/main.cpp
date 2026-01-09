#include "synth/Engine.h"
#include "utils/SynthUtils.h"
#include "utils/WavWriter.h"

#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

int main() {
  // Audio parameters
  constexpr int SAMPLE_RATE = 44100;    // CD quality: 44,100 samples per second
  constexpr int DURATION_SECONDS = 4.0; // Length of audio

  Synth::NoteSequence noteSequence{
      {"C4", "D#4", "G4"}, {"F4", "G#4", "C5"}, {"G4", "A#5", "D5"}};
  // Synth::NoteSequence noteSequence{{"C4"}, {"D#4"},
  // {"G4"}};

  Synth::NoteEventSequence noteEventSequence{};
  noteEventSequence.reserve(noteSequence.size());

  for (const std::vector<std::string> &noteGroup : noteSequence) {
    noteEventSequence.push_back(
        SynthUtils::createEventGroupFromNotes(noteGroup));
  }

  // Generate waveform samples
  // Audio is just an array of values representing air pressure over time
  // Values: floats (0.0 - 1.0)
  Synth::Engine synthEngine{SAMPLE_RATE, Synth::OscillatorType::Square};

  std::vector<float> audioBuffer{
      synthEngine.process(noteEventSequence, DURATION_SECONDS)};

  int32_t fileSampleRate{static_cast<int32_t>(SAMPLE_RATE)};

  WavWriter::writeWavFile("output.wav", audioBuffer, fileSampleRate);

  return 0;
}
