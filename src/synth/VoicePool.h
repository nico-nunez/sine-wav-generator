#pragma once

#include "Envelope.h"
#include "Filters.h"
#include "Oscillator.h"
#include "Types.h"

#include "dsp/Waveforms.h"
#include "synth/ModMatrix.h"

#include <cstddef>
#include <cstdint>

namespace synth::voices {
using WaveformType = dsp::waveforms::WaveformType;

using Envelope = envelope::Envelope;

using OscConfig = oscillator::OscConfig;
using Oscillator = oscillator::Oscillator;

using ModMatrix = mod_matrix::ModMatrix;

static constexpr OscConfig SUB_OSC_DEFAULT = {WaveformType::Sine, 0.5f, -2,
                                              0.0f, true};

struct VoicePoolConfig {
  OscConfig osc1{};
  OscConfig osc2{};
  OscConfig osc3{};
  OscConfig subOsc{SUB_OSC_DEFAULT};

  float masterGain = 1.0f;
  float sampleRate = 48000.0f;
};

// VoicePool - top-level container (universal synth)
struct VoicePool {
  // ==== Oscillators (3 main + sub oscillator) ====
  Oscillator osc1;
  Oscillator osc2;
  Oscillator osc3;
  Oscillator subOsc = oscillator::createOscillator(SUB_OSC_DEFAULT);

  // Reduce gain for multiple oscillators
  // TODO(nico): this needs to be tide to number of active oscs
  float oscMixGain = 1.0f / 4.0;

  // TODO(nico) ==== Noise Generator ====
  // NoiseGenerator noise;

  ModMatrix modMatrix;

  // ==== Envelopes ====
  Envelope ampEnv;    // Amplitude envelope
  Envelope filterEnv; // Filter modulation
  Envelope modEnv;    // General-purpose modulation

  // ==== Filters ====
  filters::SVFilter svf;
  filters::LadderFilter ladder;

  // TODO(nico)
  // // ====  LFOs (3 for modulation) ====
  // LFO lfo1;
  // LFO lfo2;
  // LFO lfo3;

  // TODO(nico)
  // // ==== Effects ====
  // Saturator saturator;

  float masterGain = 1.0f; // range [0.0 - 2.0]
                           // range [-inf - +6DB]

  // ==== Voice metadata ====
  uint8_t midiNotes[MAX_VOICES];    // Which MIDI note (0-127)
  float velocities[MAX_VOICES];     // Note-on velocity (0.0-1.0)
  uint32_t noteOnTimes[MAX_VOICES]; // NoteOn counter ( 1 is older than 2)
  uint8_t isActive[MAX_VOICES];     // 1 = active, 0 = free

  float sampleRate;
  float invSampleRate;

  // ==== Active voice tracking ====
  uint32_t activeCount = 0;
  uint32_t activeIndices[MAX_VOICES]; // Dense array of active indices
};

// updating existing Engine member
void updateVoicePoolConfig(VoicePool &pool, const VoicePoolConfig &config);

// Find free or oldest voice index for voice Initialization
uint32_t allocateVoiceIndex(VoicePool &pool);

// Initial voice state for noteOn event
void initializeVoice(VoicePool &pool, uint32_t index, uint8_t midiNote,
                     float velocity, uint32_t noteOnTime, float sampleRate);

// Trigger envelope release for voice playing midiNote
void releaseVoice(VoicePool &pool, uint8_t midiNote);

// Add newly active voice (noteOn)
void addActiveIndex(VoicePool &pool, uint32_t voiceIndex);

// Remove an inactive voice (noteOff)
void removeInactiveIndex(VoicePool &pool, uint32_t voiceIndex);

void processVoices(VoicePool &pool, float *output, size_t numSamples);

void handleNoteOn(VoicePool &pool, uint8_t midiNote, float velocity,
                  uint32_t noteOnTime, float sampleRate);

} // namespace synth::voices
