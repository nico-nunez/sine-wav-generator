#include "ModMatrix.h"
#include "synth/Types.h"

#include <cstdint>
#include <cstring>

namespace synth::mod_matrix {
bool addRoute(ModMatrix &matrix, ModSrc src, ModDest dest, float amount) {
  if (matrix.count >= MAX_MOD_ROUTES)
    return false;

  matrix.routes[matrix.count] = {src, dest, amount};
  matrix.count++;

  return true;
}

bool addRoute(ModMatrix &matrix, const ModRoute &route) {
  if (matrix.count >= MAX_MOD_ROUTES)
    return false;

  matrix.routes[matrix.count] = route;
  matrix.count++;

  return true;
}

bool removeRoute(ModMatrix &matrix, uint8_t index) {
  if (index >= matrix.count || matrix.count < 1)
    return false;

  // TODO(nico): does order matter?  don't think it does...
  matrix.count--;

  matrix.routes[index] = matrix.routes[matrix.count];

  matrix.routes[matrix.count].src = ModSrc::NoSrc;
  matrix.routes[matrix.count].dest = ModDest::NoDest;
  matrix.routes[matrix.count].amount = 0.0f;

  return true;
}

// TODO(nico): is zeroing out necessary?
bool clear(ModMatrix &matrix) {
  for (auto &route : matrix.routes) {
    route.src = ModSrc::NoSrc;
    route.dest = ModDest::NoDest;
    route.amount = 0.0f;
  }

  matrix.count = 0;
  return true;
}

// ==== Parsing Helpers ====
ModSrc modSrcFromString(const char *input) {
  for (auto &mapping : modSrcMappings) {
    if (strcasecmp(mapping.name, input) == 0)
      return mapping.src;
  }

  return ModSrc::NoSrc;
}

ModDest modDestFromString(const char *input) {
  for (auto &mapping : modDestMappings) {
    if (strcasecmp(mapping.name, input) == 0)
      return mapping.dest;
  }
  return ModDest::NoDest;
};

void clearModDestSteps(ModMatrix &matrix) {
  for (uint8_t d = 0; d < ModDest::DEST_COUNT; d++) {
    for (uint8_t v = 0; v < MAX_VOICES; v++) {
      matrix.destStepValues[d][v] = 0;
    }
  }
}
void setModDestStep(ModMatrix &matrix, ModDest dest, uint32_t voiceIndex,
                    float invNumSamples) {
  matrix.destStepValues[dest][voiceIndex] =
      (matrix.destValues[dest][voiceIndex] -
       matrix.prevDestValues[dest][voiceIndex]) *
      invNumSamples;
}

} // namespace synth::mod_matrix
