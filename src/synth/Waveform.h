#ifndef WAVEFORM_UTILS
#define WAVEFORM_UTILS

#include <cmath>

namespace Waveforms {

inline constexpr float TWO_PI = 2 * static_cast<float>(M_PI);

using WaveformFunc = float (*)(float phase, float phaseIncrement);

float sine(float phase, float);
float square(float phase, float phaseIncrement);
float squarePolyBlep(float phase, float phaseIncrement);
float saw(float phase, float phaseIncrement);
float sawPolyBlep(float phase, float phaseIncrement);
float triangle(float phase, float phaseIncrement);

float sineNormalized(float phase, float);
} // namespace Waveforms

#endif
