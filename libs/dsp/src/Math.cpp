#include "dsp/Math.h"

#include <cstdint>
#include <cstring>

namespace dsp::math {
float fastExp2(float x) {
  int32_t xi = static_cast<int32_t>(x);
  float xf = x - static_cast<float>(xi);

  float p =
      1.0f + xf * (0.6931472f +
                   xf * (0.2402265f + xf * (0.0555041f + xf * 0.0096181f)));

  int32_t bits;
  std::memcpy(&bits, &p, 4);

  bits += xi << 23;

  std::memcpy(&p, &bits, 4);
  return p;
}

float semitonesToFreqRatio(float x) { return fastExp2(x / 12); }

} // namespace dsp::math
