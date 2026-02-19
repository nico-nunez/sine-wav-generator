#include "ParamBindings.h"

#include "Engine.h"
#include "Envelope.h"

#include <cmath>
#include <cstring>

namespace synth::param_bindings {

// Anonymous Helpers
namespace {

ParamBinding makeParamBinding(bool *ptr) {
  ParamBinding binding;
  binding.boolPtr = ptr;
  binding.type = BOOL;
  binding.min = 0.0f;
  binding.max = 1.0f;
  return binding;
}

ParamBinding makeParamBinding(int8_t *ptr, int min, int max) {
  ParamBinding binding;
  binding.int8Ptr = ptr;
  binding.type = INT8;
  binding.min = static_cast<float>(min);
  binding.max = static_cast<float>(max);
  return binding;
}

ParamBinding makeParamBinding(float *ptr, float min, float max) {
  ParamBinding binding;
  binding.floatPtr = ptr;
  binding.type = FLOAT;
  binding.min = min;
  binding.max = max;
  return binding;
}

ParamBinding makeParamBinding(WaveformType *ptr, int min, int max) {
  ParamBinding binding;
  binding.waveformPtr = ptr;
  binding.type = WAVEFORM;
  binding.min = static_cast<float>(min);
  binding.max = static_cast<float>(max);
  return binding;
}

void bindOscillator(ParamBinding *bindings, ParamID baseId,
                    oscillator::Oscillator &osc) {
  bindings[baseId + 0] = makeParamBinding(&osc.waveform, 0, 3);
  bindings[baseId + 1] = makeParamBinding(&osc.mixLevel, 0.0f, 4.0f);
  bindings[baseId + 2] = makeParamBinding(&osc.detuneAmount, -100.0f, 100.0f);
  bindings[baseId + 3] = makeParamBinding(&osc.octaveOffset, -2, 2);
  bindings[baseId + 4] = makeParamBinding(&osc.enabled);
}

// Bind one envelope's 4 params starting at baseId
void bindEnvelope(ParamBinding *bindings, ParamID baseId,
                  envelope::Envelope &env) {
  bindings[baseId + 0] = makeParamBinding(&env.attackMs, 0.0f, 10000.0f);
  bindings[baseId + 1] = makeParamBinding(&env.decayMs, 0.0f, 10000.0f);
  bindings[baseId + 2] = makeParamBinding(&env.sustainLevel, 0.0f, 1.0f);
  bindings[baseId + 3] = makeParamBinding(&env.releaseMs, 0.0f, 10000.0f);
}

void onParamUpdate(Engine &engine, ParamID id) {
  switch (id) {
  case AMP_ENV_ATTACK:
  case AMP_ENV_DECAY:
  case AMP_ENV_RELEASE:
    envelope::updateIncrements(engine.voicePool.ampEnv, engine.sampleRate);
    break;

    /* TODO(nico): Add filter/mod envelopes:
     * case FILTER_ENV_ATTACK:
     * case FILTER_ENV_DECAY:
     * case FILTER_ENV_RELEASE:
     *   envelope::updateIncrements(engine.voicePool.filterEnv,
     *   engine.sampleRate); break;
     */

    // No special handling needed for other params like
    // Oscillator pitch params - no active voice updates (avoid clicks)
  default:
    break;
  }
}

} // namespace

// ==== APIs ====
void initParamBindings(Engine &engine) {
  // Oscillators - 5 params each, enum layout must match!
  bindOscillator(engine.paramBindings, OSC1_WAVEFORM, engine.voicePool.osc1);
  bindOscillator(engine.paramBindings, OSC2_WAVEFORM, engine.voicePool.osc2);
  bindOscillator(engine.paramBindings, OSC3_WAVEFORM, engine.voicePool.osc3);
  bindOscillator(engine.paramBindings, SUB_OSC_WAVEFORM,
                 engine.voicePool.subOsc);

  // Envelopes
  bindEnvelope(engine.paramBindings, AMP_ENV_ATTACK, engine.voicePool.ampEnv);

  // Voice Pool
  engine.paramBindings[MASTER_GAIN] =
      makeParamBinding(&engine.voicePool.masterGain, 0.0f, 2.0f);
}

// ==== Param Getter/Setter ====

// Get param value by ID (normalized value)
// Retreives current denormalized and returns normalized value
float getParamValueByID(const Engine &engine, ParamID id,
                        ParamValueFormat valueFormat) {
  if (id < 0 || id >= PARAM_COUNT) {
    return 0.0f;
  }

  const ParamBinding &binding = engine.paramBindings[id];
  float value = 0.0f;

  // Read the current value based on type
  switch (binding.type) {
  case FLOAT:
    value = *binding.floatPtr;
    break;

  case INT8:
    value = static_cast<float>(*binding.int8Ptr);
    break;

  case BOOL:
    value = *binding.boolPtr ? 1.0f : 0.0f;
    break;

  case WAVEFORM:
    value = static_cast<float>(static_cast<int>(*binding.waveformPtr));
    break;
  }

  if (valueFormat == ParamValueFormat::DENORMALIZED)
    return value;

  // Noramlize
  float range = binding.max - binding.min;
  if (range > 0.0f)
    return (value - binding.min) / range;

  return 0.0f;
}

// Set param value by ID
// Expects normalized values, denormalizes, and updates value
void setParamValueByID(Engine &engine, ParamID id, float value,
                       ParamValueFormat valueFormat) {
  if (id < 0 || id >= PARAM_COUNT) {
    return;
  }

  ParamBinding &binding = engine.paramBindings[id];

  // Denormalize
  if (valueFormat == ParamValueFormat::NORMALIZED) {
    if (value < 0.0f)
      value = 0.0f;
    if (value > 1.0f)
      value = 1.0f;

    value = binding.min + (value * (binding.max - binding.min));
  }

  switch (binding.type) {
  case FLOAT:
    *binding.floatPtr = value;
    break;

  case INT8:
    *binding.int8Ptr = static_cast<int8_t>(std::round(value));
    break;

  case BOOL:
    *binding.boolPtr = value >= 0.5f;
    break;

  case WAVEFORM:
    *binding.waveformPtr =
        static_cast<WaveformType>(static_cast<int>(std::round(value)));
    break;
  }

  // Handle post-update logic for params with derived values (i.e. Envelopes)
  onParamUpdate(engine, id);
}

// String → ParamID (for parsing 'set' commands)
ParamID findParamByName(const char *name) {
  for (const auto &mapping : PARAM_NAMES) {
    if (strcmp(mapping.name, name) == 0) {
      return mapping.id;
    }
  }
  return PARAM_COUNT; // Invalid/not found
}

// ParamID → String (for 'get' commands, help text, errors)
const char *getParamName(ParamID id) {
  for (const auto &mapping : PARAM_NAMES) {
    if (mapping.id == id) {
      return mapping.name;
    }
  }
  return nullptr;
}

} // namespace synth::param_bindings
