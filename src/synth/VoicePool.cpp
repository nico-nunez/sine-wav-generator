#include "VoicePool.h"
#include "Envelope.h"
#include "Oscillator.h"
#include "Types.h"

#include "dsp/Effects.h"

#include <cstddef>
#include <cstdint>

namespace synth::voices {

VoicePool initVoicePool(const VoicePoolConfig &config) {
  VoicePool pool{};

  oscillator::updateConfig(pool.osc1, config.osc1);
  oscillator::updateConfig(pool.osc2, config.osc2);
  oscillator::updateConfig(pool.osc3, config.osc3);
  oscillator::updateConfig(pool.subOsc, config.subOsc);

  pool.masterGain = config.masterGain;

  return pool;
}

void updateVoicePoolConfig(VoicePool &pool, const VoicePoolConfig &config) {
  oscillator::updateConfig(pool.osc1, config.osc1);
  oscillator::updateConfig(pool.osc2, config.osc2);
  oscillator::updateConfig(pool.osc3, config.osc3);
  oscillator::updateConfig(pool.subOsc, config.subOsc);

  pool.masterGain = config.masterGain;
}

// Find free or oldest voice index for voice Initialization
uint32_t allocateVoiceIndex(VoicePool &pool) {
  uint32_t oldestIndex = MAX_VOICES; // out of range
  uint32_t oldestNoteOnTime = UINT32_MAX;

  for (uint32_t i = 0; i < MAX_VOICES; i++) {
    if (!pool.isActive[i]) {
      return i; // return available voice index
    } else {
      if (pool.noteOnTimes[i] < oldestNoteOnTime) {
        oldestNoteOnTime = pool.noteOnTimes[i];
        oldestIndex = i;
      }
    }
  }

  // Need to cleanup otherwise it'll play twice
  // since it'll be added again after initializing voice
  removeInactiveIndex(pool, oldestIndex);

  return oldestIndex;
}

// Trigger envelope release for voice playing midiNote
void releaseVoice(VoicePool &pool, uint8_t midiNote);

// Find oldest active voice (for stealing)
int findOldestVoice(const VoicePool &pool);

void addActiveIndex(VoicePool &pool, uint32_t voiceIndex) {
  pool.activeIndices[pool.activeCount] = voiceIndex;
  pool.activeCount++;
}

void removeInactiveIndex(VoicePool &pool, uint32_t voiceIndex) {
  uint32_t removeIndex = pool.activeCount; // out of range of active

  for (uint32_t i = 0; i < pool.activeCount; i++) {
    if (pool.activeIndices[i] == voiceIndex) {
      removeIndex = i;
      break;
    }
  }

  if (removeIndex == pool.activeCount)
    return;

  // Swap current inactive with most recent active
  pool.activeCount--;
  pool.activeIndices[removeIndex] = pool.activeIndices[pool.activeCount];
  pool.isActive[voiceIndex] = 0;
}

void initializeVoice(VoicePool &pool, uint32_t voiceIndex, uint8_t midiNote,
                     float velocity, uint32_t noteOnTime, float sampleRate) {
  // === Set Metadata ===
  pool.isActive[voiceIndex] = 1;
  pool.midiNotes[voiceIndex] = midiNote;
  pool.noteOnTimes[voiceIndex] = noteOnTime;
  pool.velocities[voiceIndex] = velocity / 127.0f;

  // ==== Initialize Oscillator 1 ====
  oscillator::initOscillator(pool.osc1, voiceIndex, midiNote, sampleRate);

  // ==== Initialize Oscillator 2 ====
  oscillator::initOscillator(pool.osc2, voiceIndex, midiNote, sampleRate);

  // ==== Initialize Oscillator 3 ====
  oscillator::initOscillator(pool.osc3, voiceIndex, midiNote, sampleRate);

  // ==== Initialize Sub Oscillator ====
  oscillator::initOscillator(pool.subOsc, voiceIndex, midiNote, sampleRate);

  // ==== Initialize Envelopes ====
  // Amp envelope
  envelope::initEnvelope(pool.ampEnv, voiceIndex, sampleRate);

  // Filter envelope
  envelope::initEnvelope(pool.filterEnv, voiceIndex, sampleRate);

  // Mod envelope
  envelope::initEnvelope(pool.modEnv, voiceIndex, sampleRate);
}

uint32_t findVoiceRetrigger(VoicePool &pool, uint8_t midiNote) {
  for (uint32_t i = 0; i < pool.activeCount; i++) {
    uint32_t voiceIndex = pool.activeIndices[i];
    if (pool.midiNotes[voiceIndex] == midiNote &&
        pool.ampEnv.states[voiceIndex] != envelope::EnvelopeStatus::Idle) {
      return voiceIndex;
    }
  }
  return MAX_VOICES;
}

uint32_t findVoiceRelease(VoicePool &pool, uint8_t midiNote) {
  for (uint32_t i = 0; i < pool.activeCount; i++) {
    uint32_t voiceIndex = pool.activeIndices[i];
    if (pool.midiNotes[voiceIndex] == midiNote &&
        pool.ampEnv.states[voiceIndex] != envelope::EnvelopeStatus::Release &&
        pool.ampEnv.states[voiceIndex] != envelope::EnvelopeStatus::Idle) {

      return voiceIndex;
    }
  }
  return MAX_VOICES;
}

bool isValidActiveIndex(uint32_t index) { return index < MAX_VOICES; }

void releaseVoice(VoicePool &pool, uint8_t midiNote) {
  uint32_t voiceIndex = findVoiceRelease(pool, midiNote);

  if (!isValidActiveIndex(voiceIndex))
    return;

  if (pool.midiNotes[voiceIndex] == midiNote &&
      pool.ampEnv.states[voiceIndex] != envelope::EnvelopeStatus::Release &&
      pool.ampEnv.states[voiceIndex] != envelope::EnvelopeStatus::Idle) {

    envelope::triggerRelease(pool.ampEnv, voiceIndex);
    envelope::triggerRelease(pool.filterEnv, voiceIndex);
    envelope::triggerRelease(pool.modEnv, voiceIndex);
  }
}

void processVoices(VoicePool &pool, float *output, size_t numSamples) {
  for (uint32_t s = 0; s < numSamples; s++) {
    float sample = 0.0f;

    // Iterating backwards to more easily deal with swapping voices
    // that have/will become Idle/inactive after processing
    for (uint32_t i = pool.activeCount; i > 0; i--) {
      uint32_t voiceIndex = pool.activeIndices[i - 1];

      // ==== PROCESS OSCILLATORS ====
      float osc1 = oscillator::processOscillator(pool.osc1, voiceIndex);
      float osc2 = oscillator::processOscillator(pool.osc2, voiceIndex);
      float osc3 = oscillator::processOscillator(pool.osc3, voiceIndex);
      float subOsc = oscillator::processOscillator(pool.subOsc, voiceIndex);

      float mixedOsc = (osc1 + osc2 + osc3 + subOsc) * pool.oscMixGain;

      // ==== PROCESS ENVELOPES ====
      float ampEnv = envelope::processEnvelope(pool.ampEnv, voiceIndex);
      // float filterEnv = processEnvelope(pool.filterEnv, voiceIndex);

      // TODO(nico): Implement Filter
      // ==== Process filters (with envelope modulation) ====
      // float filtered = processFilter(pool.filter1, mixedOsc, idx, filterEnv);
      // filtered = processFilter(pool.filter2, filtered, idx, 0.0f);
      float filtered = mixedOsc;

      // TODO(nico): Implement Saturator
      // ==== Apply saturation ====
      // filtered = processSaturator(pool.saturator, filtered);

      // Check if amplitude envelope completed - remove immediately
      if (pool.ampEnv.states[voiceIndex] == envelope::EnvelopeStatus::Idle) {
        removeInactiveIndex(pool, voiceIndex);
        // No index adjustment needed - iterating backwards
      }

      sample += filtered * ampEnv * pool.velocities[voiceIndex] * VOICE_GAIN;
    }

    // TEMP(nico): Basic soft clip for now.
    // Mainly for protection and not as an effect
    output[s] = dsp::effects::softClipFast(sample * pool.masterGain);
  }
}

// Handle NoteOn Events
void handleNoteOn(VoicePool &pool, uint8_t midiNote, float velocity,
                  uint32_t noteOnTime, float sampleRate) {
  uint32_t voiceIndex = allocateVoiceIndex(pool);

  initializeVoice(pool, voiceIndex, midiNote, velocity, noteOnTime, sampleRate);

  addActiveIndex(pool, voiceIndex);
}
} // namespace synth::voices
