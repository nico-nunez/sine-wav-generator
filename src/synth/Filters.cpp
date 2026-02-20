#include "Filters.h"

#include "dsp/Math.h"

#include <cmath>

namespace synth::filters {

// ==== Internal Helpers ====
namespace {

float computeEffectiveCutoff(float baseCutoff, float envAmount,
                             float filterEnvVal) {
  // Octave-based modulation: musically linear, consistent across the frequency
  // range envAmount in octaves (e.g. +3.0 = three octaves above baseCutoff at
  // full envelope)
  float octaveShift = envAmount * filterEnvVal;
  return baseCutoff * std::exp2f(octaveShift);
}

} // namespace

// ==== SVF Helpers ====
void enableSVFilter(SVFilter &filter, bool enable) {
  if (enable && !filter.enabled) {
    // Reset all voice states
    for (uint32_t i = 0; i < MAX_VOICES; i++) {
      filter.voiceStates[i] = dsp::filters::SVFState{};
    }
  }
  filter.enabled = enable;
}

void initSVFilter(SVFilter &filter, size_t voiceIndex) {
  filter.voiceStates[voiceIndex] = SVFState{};
}

void updateSVFCoefficients(SVFilter &filter, float invSampleRate) {
  float Q = 0.5f + filter.resonance * 20.0f;
  filter.coeffs =
      dsp::filters::computeSVFCoeffs(filter.cutoff, Q, invSampleRate);
}

float processSVFilter(SVFilter &filter, float input, uint32_t voiceIndex,
                      float filterEnvVal, float sampleRate) {
  if (!filter.enabled)
    return input;

  float modCutoff =
      computeEffectiveCutoff(filter.cutoff, filter.envAmount, filterEnvVal);

  // Use cached coeffs when envelope is idle, recompute when active
  SVFCoeffs c =
      (filterEnvVal > 0.001f)
          ? dsp::filters::computeSVFCoeffs(
                modCutoff, 0.5f + filter.resonance * 20.0f, sampleRate)
          : filter.coeffs;

  SVFOutputs out =
      dsp::filters::processSVF(input, c, filter.voiceStates[voiceIndex]);

  switch (filter.mode) {
  case SVFMode::LP:
    return out.lp;
  case SVFMode::HP:
    return out.hp;
  case SVFMode::BP:
    return out.bp;
  case SVFMode::Notch:
    return out.lp + out.hp;
  }
  return out.lp;
}

// ==== Ladder Helpers ====
void enableLadderFilter(LadderFilter &filter, bool enable) {
  if (enable && !filter.enabled) {
    for (uint32_t i = 0; i < MAX_VOICES; i++) {
      filter.voiceStates[i] = dsp::filters::LadderState{};
    }
  }
  filter.enabled = enable;
}

void initLadderFilter(LadderFilter &filter, size_t voiceIndex) {
  filter.voiceStates[voiceIndex] = LadderState{};
}

void updateLadderCoefficient(LadderFilter &filter, float invSampleRate) {
  filter.coeff =
      2.0f * std::sin(dsp::math::PI_F * filter.cutoff * invSampleRate);
}

float processLadderFilter(LadderFilter &filter, float input,
                          uint32_t voiceIndex, float filterEnvVal,
                          float invSampleRate) {
  if (!filter.enabled)
    return input;

  float modCutoff =
      computeEffectiveCutoff(filter.cutoff, filter.envAmount, filterEnvVal);
  float coeff =
      (filterEnvVal > 0.001f)
          ? 2.0f * std::sin(dsp::math::PI_F * modCutoff * invSampleRate)
          : filter.coeff;

  float res = filter.resonance * 4.0f; // map 0–1 to Ladder's 0–4 range

  return (filter.drive > 1.001f)
             ? dsp::filters::processLadderNonlinear(
                   input, coeff, res, filter.drive,
                   filter.voiceStates[voiceIndex])
             : dsp::filters::processLadder(input, coeff, res,
                                           filter.voiceStates[voiceIndex]);
}
} // namespace synth::filters
