#pragma once

#include "synth/Filters.h"
#include "synth/Oscillator.h"
#include <cstddef>

namespace synth {
struct Engine;
}

namespace synth::param_bindings {
using SVFMode = filters::SVFMode;
using WaveformType = oscillator::WaveformType;

enum ParamID {
  // Oscillator 1
  OSC1_WAVEFORM,
  OSC1_MIX_LEVEL,
  OSC1_DETUNE_AMOUNT,
  OSC1_OCTAVE_OFFSET,
  OSC1_ENABLED,

  // Oscillator 2
  OSC2_WAVEFORM,
  OSC2_MIX_LEVEL,
  OSC2_DETUNE_AMOUNT,
  OSC2_OCTAVE_OFFSET,
  OSC2_ENABLED,

  // Oscillator 3
  OSC3_WAVEFORM,
  OSC3_MIX_LEVEL,
  OSC3_DETUNE_AMOUNT,
  OSC3_OCTAVE_OFFSET,
  OSC3_ENABLED,

  // Sub Oscillator
  SUB_OSC_WAVEFORM,
  SUB_OSC_MIX_LEVEL,
  SUB_OSC_DETUNE_AMOUNT,
  SUB_OSC_OCTAVE_OFFSET,
  SUB_OSC_ENABLED,

  // Amp Envelope
  AMP_ENV_ATTACK,
  AMP_ENV_DECAY,
  AMP_ENV_SUSTAIN_LEVEL,
  AMP_ENV_RELEASE,

  // Filter Envelope
  FILTER_ENV_ATTACK,
  FILTER_ENV_DECAY,
  FILTER_ENV_SUSTAIN_LEVEL,
  FILTER_ENV_RELEASE,

  // SVF Filter
  SVF_ENABLED,
  SVF_MODE,
  SVF_CUTOFF,
  SVF_RESONANCE,
  SVF_ENV_AMOUNT,

  // Ladder Filter
  LADDER_ENABLED,
  LADDER_CUTOFF,
  LADDER_RESONANCE,
  LADDER_DRIVE,
  LADDER_ENV_AMOUNT,

  MASTER_GAIN,
  PARAM_COUNT,
};

enum ParamValueFormat {
  NORMALIZED,
  DENORMALIZED,
};

enum ParamStorageType { FLOAT, INT8, BOOL, WAVEFORM, FILTER_MODE };

struct ParamBinding {
  union {
    float *floatPtr;
    int8_t *int8Ptr;
    bool *boolPtr;
    SVFMode *svfModePtr;
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

// Used to find input param names
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

    {"svf.enabled", SVF_ENABLED},
    {"svf.mode", SVF_MODE},
    {"svf.cutoff", SVF_CUTOFF},
    {"svf.resonance", SVF_RESONANCE},
    {"svf.envAmount", SVF_ENV_AMOUNT},

    {"ladder.enabled", LADDER_ENABLED},
    {"ladder.cutoff", LADDER_CUTOFF},
    {"ladder.resonance", LADDER_RESONANCE},
    {"ladder.drive", LADDER_DRIVE},
    {"ladder.envAmount", LADDER_ENV_AMOUNT},

    {"filterEnv.attack", FILTER_ENV_ATTACK},
    {"filterEnv.decay", FILTER_ENV_DECAY},
    {"filterEnv.sustain", FILTER_ENV_SUSTAIN_LEVEL},
    {"filterEnv.release", FILTER_ENV_RELEASE},

    {"master.gain", MASTER_GAIN},
};
inline constexpr size_t PARAM_NAME_COUNT =
    sizeof(PARAM_NAMES) / sizeof(PARAM_NAMES[0]);

// Used to check if input param value should be WaveformType
constexpr ParamNameMapping WAVEFORM_PARAMS[] = {
    {"osc1.waveform", OSC1_WAVEFORM},
    {"osc2.waveform", OSC2_WAVEFORM},
    {"osc3.waveform", OSC3_WAVEFORM},
    {"subOsc.waveform", SUB_OSC_WAVEFORM},
};
inline constexpr size_t WAVEFORM_PARAM_COUNT =
    sizeof(WAVEFORM_PARAMS) / sizeof(WAVEFORM_PARAMS[0]);

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

// Helpers for parsing waveform type param values
bool isWaveFormParam(const char *paramName);
WaveformType getWaveformType(const char *inputValue);

} // namespace synth::param_bindings
