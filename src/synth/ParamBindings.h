#pragma once

#include "synth/Oscillator.h"
#include <cstddef>

namespace synth {
struct Engine;
}

namespace synth::param_bindings {
using WaveformType = oscillator::WaveformType;

enum ParamID {
  OSC1_WAVEFORM,
  OSC1_MIX_LEVEL,
  OSC1_DETUNE_AMOUNT,
  OSC1_OCTAVE_OFFSET,
  OSC1_ENABLED,
  OSC2_WAVEFORM,
  OSC2_MIX_LEVEL,
  OSC2_DETUNE_AMOUNT,
  OSC2_OCTAVE_OFFSET,
  OSC2_ENABLED,
  OSC3_WAVEFORM,
  OSC3_MIX_LEVEL,
  OSC3_DETUNE_AMOUNT,
  OSC3_OCTAVE_OFFSET,
  OSC3_ENABLED,
  SUB_OSC_WAVEFORM,
  SUB_OSC_MIX_LEVEL,
  SUB_OSC_DETUNE_AMOUNT,
  SUB_OSC_OCTAVE_OFFSET,
  SUB_OSC_ENABLED,
  AMP_ENV_ATTACK,
  AMP_ENV_DECAY,
  AMP_ENV_SUSTAIN_LEVEL,
  AMP_ENV_RELEASE,
  MASTER_GAIN,
  PARAM_COUNT,
};

enum ParamValueFormat {
  NORMALIZED,
  DENORMALIZED,
};

enum ParamStorageType {
  FLOAT,
  INT8,
  BOOL,
  WAVEFORM // For WaveformType enum
};

struct ParamBinding {
  union {
    float *floatPtr;
    int8_t *int8Ptr;
    bool *boolPtr;
    WaveformType *waveformPtr;
  };
  ParamStorageType type;
  float min, max;
};

// ==== Param Parsing ====
struct ParamNameMapping {
  const char *name;
  ParamID id;
};

constexpr ParamNameMapping PARAM_NAMES[] = {
    {"osc1.waveform", OSC1_WAVEFORM},
    {"osc1.mixLevel", OSC1_MIX_LEVEL},
    {"osc1.detune", OSC1_DETUNE_AMOUNT},
    {"osc1.octave", OSC1_OCTAVE_OFFSET},
    {"osc1.enabled", OSC1_ENABLED},

    {"osc2.waveform", OSC2_WAVEFORM},
    {"osc2.mixLevel", OSC2_MIX_LEVEL},
    {"osc2.detune", OSC2_DETUNE_AMOUNT},
    {"osc2.octave", OSC2_OCTAVE_OFFSET},
    {"osc2.enabled", OSC2_ENABLED},

    {"osc3.waveform", OSC3_WAVEFORM},
    {"osc3.mixLevel", OSC3_MIX_LEVEL},
    {"osc3.detune", OSC3_DETUNE_AMOUNT},
    {"osc3.octave", OSC3_OCTAVE_OFFSET},
    {"osc3.enabled", OSC3_ENABLED},

    {"subOsc.waveform", SUB_OSC_WAVEFORM},
    {"subOsc.mixLevel", SUB_OSC_MIX_LEVEL},
    {"subOsc.detune", SUB_OSC_DETUNE_AMOUNT},
    {"subOsc.octave", SUB_OSC_OCTAVE_OFFSET},
    {"subOsc.enabled", SUB_OSC_ENABLED},

    {"ampEnv.attack", AMP_ENV_ATTACK},
    {"ampEnv.decay", AMP_ENV_DECAY},
    {"ampEnv.sustain", AMP_ENV_SUSTAIN_LEVEL},
    {"ampEnv.release", AMP_ENV_RELEASE},

    {"master.gain", MASTER_GAIN},
};

inline constexpr size_t PARAM_NAME_COUNT =
    sizeof(PARAM_NAMES) / sizeof(PARAM_NAMES[0]);

// ==== API Methods ====
void initParamBindings(synth::Engine &engine);

float getParamValueByID(
    const Engine &engine, ParamID id,
    ParamValueFormat valueFormat = ParamValueFormat::DENORMALIZED);

void setParamValueByID(
    Engine &engine, ParamID id, float value,
    ParamValueFormat valueFormat = ParamValueFormat::DENORMALIZED);

// String parsing helpers
ParamID findParamByName(const char *name);
const char *getParamName(ParamID id);

} // namespace synth::param_bindings
