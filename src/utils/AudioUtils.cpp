#include "AudioUtils.h"
#include "synth/Oscillator.h"

#include <cassert>
#include <climits>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

namespace AudioUtils {
void generateSineValues(std::vector<int16_t> &samples, double frequency,
                        const int duration, const int sampleRate) {
  // Volume (0.0 to 1.0, keeping it at 50% to avoid clipping)
  const double AMPLITUDE = 0.5;
  const int32_t numSamples = sampleRate * duration;

  std::cout << "Generating " << duration << " seconds of " << frequency
            << "HZ sine wave...\n";

  for (int32_t i = 0; i < numSamples; ++i) {
    // The sine wave formula: sin(2 * π * frequency * time)
    // time = i / SAMPLE_RATE (current sample number divided by samples per
    // second)
    double time = static_cast<double>(i) / sampleRate;
    double value = AMPLITUDE * std::sin(2.0 * M_PI * frequency * time);

    // Convert from floating point (-1.0 to 1.0) to 16-bit integer (-32768 to
    // 32767) This is how audio is stored in WAV files
    int16_t sample = static_cast<int16_t>(value * 32767.0);
    samples.push_back(sample);
  }
}

/* Semitone ratio is 2^(1/12). This is multiplied by the number
 * of semitones from A4 for the note that's to be calculated.
 * This value (ratio * semitones) is multiplied by 440hz (A4)
 * to get the value of the desired note in hertz.
 *
 * Example: C4 = 440hz * (2^((1/12) * -9)), where C4 is -9 semitones from A4
 */
float getHertzFromSemitoneOffset(int semitones) {
  return 440.0f * static_cast<float>(std::pow(2.0, (1.0 / 12.0) * semitones));
}

void renderSequence(std::vector<int16_t> &samples, Sequence &sequence,
                    int duration, float amplitude) {
  /*
   * A "Sequence" is a vector of "OscillatorGroups"
   *"OscillatorGroup" is a vector of "Oscillators"
   * sequence[i] == WHEN to play/render (order)
   * sequence[i][j] == WHAT to play/render (oscillators)
   */
  for (Synth::OscillatorGroup &oscillators : sequence) {

    // Ensure it's not empty and not too large to be converted to int
    // TODO: add runtime checks
    assert(oscillators.size() > 0);
    assert(oscillators.size() < INT_MAX);

    int numOscillators = static_cast<int>(oscillators.size());

    // Making assumption that all oscillators have the same sample rate
    const int32_t totalSamples =
        static_cast<int>(oscillators[0].getSampleRate()) * duration;

    // Reduce the amplitude based on number of oscillators. Example:
    // 3 oscillators at N amplitude is louder than 1 oscillator at N amplitude
    const float relativeAmplitude = amplitude * (1.0f / numOscillators);

    for (int32_t i = 0; i < totalSamples; ++i) {
      float value{0.0f};
      for (auto &osc : oscillators) {
        value += osc.getNextSampleValue();
      }

      // Convert from floating point (-1.0 to 1.0) to 16-bit integer (-32768
      // to 32767) This is how audio is stored in WAV files
      int16_t sample = static_cast<int16_t>(relativeAmplitude * value *
                                            static_cast<float>(INT16_MAX));
      samples.push_back(sample);
    }
  }
}

} // namespace AudioUtils
