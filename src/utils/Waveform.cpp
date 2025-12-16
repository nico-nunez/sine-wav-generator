#include "utils/Waveform.h"
#include <cmath>
#include <cstdlib>

namespace Waveforms {

constexpr float TWO_PI = 2 * static_cast<float>(M_PI);

float sine(float phase) { return std::sin(phase * TWO_PI); }

float square(float phase) { return phase < 0.5f ? 1.0f : -1.0f; }

float saw(float phase) { return 2.0f * phase - 1.0f; }

float triangle(float phase) { return 4.0f * std::abs(phase - 0.5f) - 1.0f; }

// NOTE:  using normalized phase results in a cool distorted sound
float sineNormalized(float phase) { return std::sin(phase); }

} // namespace Waveforms
