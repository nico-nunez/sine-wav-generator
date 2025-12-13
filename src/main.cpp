#include "synth/Oscillator.h"
#include "utils/AudioUtils.h"
#include "utils/WavWriter.h"

#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

int main() {
  // Audio parameters
  const int SAMPLE_RATE = 44100;  // CD quality: 44,100 samples per second
  const int DURATION_SECONDS = 4; // Length of audio

  // Generate 3 notes(hz) -> C4, E4-flat, G4 (Cminor triad)
  // AudioUtils::Sequence monoSequence{
  //     Synth::OscillatorGroup{
  //         Synth::Oscillator(AudioUtils::getHertzFromSemitoneOffset(-9))},
  //     Synth::OscillatorGroup{
  //         Synth::Oscillator(AudioUtils::getHertzFromSemitoneOffset(-6))},
  //     Synth::OscillatorGroup{
  //         Synth::Oscillator(AudioUtils::getHertzFromSemitoneOffset(-2))}};

  // Generate 3 notes(hz) -> C4, E4-flat, G4 (Cminor triad)
  AudioUtils::Sequence polySequence{Synth::OscillatorGroup{
      Synth::Oscillator(AudioUtils::getHertzFromSemitoneOffset(-9)),
      Synth::Oscillator(AudioUtils::getHertzFromSemitoneOffset(-6)),
      Synth::Oscillator(AudioUtils::getHertzFromSemitoneOffset(-2))}};

  // Ensure sequence length can be converted to int
  // TODO: add runtime check
  assert(polySequence.size() <= INT_MAX);

  // Calculate total number of samples
  const int32_t TOTAL_SAMPLES =
      SAMPLE_RATE * DURATION_SECONDS * static_cast<int>(polySequence.size());

  // Generate sine wave samples
  // Audio is just an array of numbers representing air pressure over time
  std::vector<int16_t> samples;

  // TODO: add runtime check
  assert(TOTAL_SAMPLES > 0);
  samples.reserve(static_cast<size_t>(TOTAL_SAMPLES));

  std::ofstream wavFile{WavWriter::createWavFile()};

  // Generate sample values (aka play oscillators)
  AudioUtils::renderSequence(samples, polySequence, DURATION_SECONDS);

  if (!wavFile) {
    std::cerr << "Error: Could not create output.wav\n";
    return 1;
  }

  std::cout << "Writing WAV file...\n";

  WavWriter::writeWavMetadata(wavFile, TOTAL_SAMPLES, SAMPLE_RATE);

  // --- DATA CHUNK ---
  // Contains the actual audio samples
  WavWriter::writeString(wavFile, "data", 4);

  // Data chunk size (number of samples * bytes per sample)
  WavWriter::writeInt32(wavFile, TOTAL_SAMPLES * 2);

  // Write all the audio samples
  for (const auto &sample : samples) {
    WavWriter::writeInt16(wavFile, sample);
  }

  wavFile.close();

  std::cout << "Success! Created output.wav\n";
  std::cout << "Play it with any audio player to hear your sine wave.\n";

  return 0;
}
