#ifndef WAVEFORM_UTILS
#define WAVEFORM_UTILS

#include <cmath>

namespace Waveforms {

inline constexpr float TWO_PI = 2 * static_cast<float>(M_PI);

using WaveformFunc = float (*)(float);

float sine(float phase);
float square(float phase);
float triangle(float phase);
float saw(float phase);

float sineNormalized(float phase);
} // namespace Waveforms

#endif
