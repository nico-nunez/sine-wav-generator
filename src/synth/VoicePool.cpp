#include "VoicePool.h"
#include "Envelope.h"
#include "Oscillator.h"
#include "Types.h"

#include "synth/Filters.h"
#include "synth/ModMatrix.h"

#include "dsp/Effects.h"
#include "dsp/Math.h"

#include <cstddef>
#include <cstdint>

namespace synth::voices {
using ModSrc = mod_matrix::ModSrc;
using ModDest = mod_matrix::ModDest;
using ModDest2D = mod_matrix::ModDest2D;
using ModRoute = mod_matrix::ModRoute;

// ===== Internal Helpers ====
namespace {
float interpolatePitchInc(Oscillator &osc, ModMatrix &matrix, ModDest dest,
                          uint32_t voiceIndex, uint32_t sampleNum) {
  float pitchMod =
      matrix.prevDestValues[dest][voiceIndex] +
      (matrix.destStepValues[dest][voiceIndex] * static_cast<float>(sampleNum));

  return osc.phaseIncrements[voiceIndex] *
         dsp::math::semitonesToFreqRatio(pitchMod);
}

} // namespace

VoicePool initVoicePool(const VoicePoolConfig &config) {
  VoicePool pool{};

  pool.sampleRate = config.sampleRate;
  pool.invSampleRate = 1.0f / config.sampleRate;

  pool.masterGain = config.masterGain;

  oscillator::updateConfig(pool.osc1, config.osc1);
  oscillator::updateConfig(pool.osc2, config.osc2);
  oscillator::updateConfig(pool.osc3, config.osc3);
  oscillator::updateConfig(pool.subOsc, config.subOsc);

  filters::updateSVFCoefficients(pool.svf, pool.invSampleRate);
  filters::updateLadderCoefficient(pool.ladder, pool.invSampleRate);

  mod_matrix::addRoute(pool.modMatrix, ModSrc::FilterEnv, ModDest::SVFCutoff,
                       0.0f);
  mod_matrix::addRoute(pool.modMatrix, ModSrc::FilterEnv, ModDest::LadderCutoff,
                       0.0f);
  return pool;
}

void updateVoicePoolConfig(VoicePool &pool, const VoicePoolConfig &config) {
  pool.sampleRate = config.sampleRate;
  pool.invSampleRate = 1.0f / config.sampleRate;

  pool.masterGain = config.masterGain;

  oscillator::updateConfig(pool.osc1, config.osc1);
  oscillator::updateConfig(pool.osc2, config.osc2);
  oscillator::updateConfig(pool.osc3, config.osc3);
  oscillator::updateConfig(pool.subOsc, config.subOsc);

  filters::updateSVFCoefficients(pool.svf, pool.invSampleRate);
  filters::updateLadderCoefficient(pool.ladder, pool.invSampleRate);
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

// TODO(nico): this is confusing.  there must be a better way
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
  // ==== Set Metadata ====
  pool.isActive[voiceIndex] = 1;
  pool.midiNotes[voiceIndex] = midiNote;
  pool.noteOnTimes[voiceIndex] = noteOnTime;
  pool.velocities[voiceIndex] = velocity / 127.0f;

  pool.sampleRate = sampleRate;
  pool.invSampleRate = 1.0f / sampleRate;

  // ==== Reset Modulation Destination Values ====
  for (int d = 0; d < ModDest::DEST_COUNT; d++) {
    pool.modMatrix.prevDestValues[d][voiceIndex] = 0.0f;
    pool.modMatrix.destValues[d][voiceIndex] = 0.0f;
  }

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

  // ==== Initialize Filter States ====
  filters::initSVFilter(pool.svf, voiceIndex);
  filters::initLadderFilter(pool.ladder, voiceIndex);
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
  float invNumSamples = 1.0f / static_cast<float>(numSamples);

  mod_matrix::clearModDestSteps(pool.modMatrix);

  /* ==== Pre-pass: once per block, once per active voice ====
   * Advance block-rate envelopes (filterEnv, modEnv).
   * ampEnv is NOT advanced here; it runs per-sample in the hot loop below.
   * =====================================================================
   */
  for (uint32_t i = pool.activeCount; i > 0; i--) {
    uint32_t voiceIndex = pool.activeIndices[i - 1];

    // Set initial modulation source values
    float modSrcs[ModSrc::SRC_COUNT] = {};

    modSrcs[ModSrc::AmpEnv] =
        pool.ampEnv.levels[voiceIndex]; // processed in main loop

    modSrcs[ModSrc::FilterEnv] =
        envelope::processEnvelope(pool.filterEnv, voiceIndex);

    modSrcs[ModSrc::ModEnv] =
        envelope::processEnvelope(pool.modEnv, voiceIndex);

    modSrcs[ModSrc::Velocity] = pool.velocities[voiceIndex];

    // Accumulate mod destinations
    float modDests[ModDest::DEST_COUNT] = {};

    for (uint8_t r = 0; r < pool.modMatrix.count; r++) {
      const ModRoute &route = pool.modMatrix.routes[r];

      if (route.src == ModSrc::NoSrc || route.dest == ModDest::NoDest)
        continue;

      modDests[route.dest] += modSrcs[route.src] * route.amount;
    }

    // Set modulation destination values
    for (int d = 0; d < ModDest::DEST_COUNT; d++)
      pool.modMatrix.destValues[d][voiceIndex] = modDests[d];

    // Interpolation setup for fast destinations (pitch)
    mod_matrix::setModDestStep(pool.modMatrix, ModDest::Osc1Pitch, voiceIndex,
                               invNumSamples);
    mod_matrix::setModDestStep(pool.modMatrix, ModDest::Osc2Pitch, voiceIndex,
                               invNumSamples);
    mod_matrix::setModDestStep(pool.modMatrix, ModDest::Osc3Pitch, voiceIndex,
                               invNumSamples);
    mod_matrix::setModDestStep(pool.modMatrix, ModDest::SubOscPitch, voiceIndex,
                               invNumSamples);
  }

  // ==== Main Loop: Calculate each sample value (process block) ====
  for (uint32_t s = 0; s < numSamples; s++) {
    float sample = 0.0f;

    // Iterating backwards to more easily deal with swapping voices
    // that have/will become Idle/inactive after processing
    for (uint32_t i = pool.activeCount; i > 0; i--) {
      uint32_t voiceIndex = pool.activeIndices[i - 1];

      // Process Oscillators (with Interpolation)
      float osc1PhaseInc = interpolatePitchInc(
          pool.osc1, pool.modMatrix, ModDest::Osc1Pitch, voiceIndex, s);
      float osc1 =
          oscillator::processOscillator(pool.osc1, voiceIndex, osc1PhaseInc);

      float osc2PhaseInc = interpolatePitchInc(
          pool.osc2, pool.modMatrix, ModDest::Osc2Pitch, voiceIndex, s);
      float osc2 =
          oscillator::processOscillator(pool.osc2, voiceIndex, osc2PhaseInc);

      float osc3PhaseInc = interpolatePitchInc(
          pool.osc3, pool.modMatrix, ModDest::Osc3Pitch, voiceIndex, s);
      float osc3 =
          oscillator::processOscillator(pool.osc3, voiceIndex, osc3PhaseInc);

      float subOscPhaseInc = interpolatePitchInc(
          pool.subOsc, pool.modMatrix, ModDest::SubOscPitch, voiceIndex, s);
      float subOsc = oscillator::processOscillator(pool.subOsc, voiceIndex,
                                                   subOscPhaseInc);

      // Mix (sum) Oscillators
      float mixedOscs = (osc1 + osc2 + osc3 + subOsc) * pool.oscMixGain;

      // Process Fitlers (svf & ladder)
      float filtered = filters::processSVFilter(
          pool.svf, mixedOscs, voiceIndex,
          pool.modMatrix.destValues[ModDest::SVFCutoff][voiceIndex],
          pool.invSampleRate);

      filtered = filters::processLadderFilter(
          pool.ladder, filtered, voiceIndex,
          pool.modMatrix.destValues[ModDest::LadderCutoff][voiceIndex],
          pool.invSampleRate);

      // TODO(nico): Implement Saturator
      // ==== Apply saturation ====
      // filtered = processSaturator(pool.saturator, filtered);

      // Process Amp Envelope
      float ampEnv = envelope::processEnvelope(pool.ampEnv, voiceIndex);

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

  /* ==== Post-block: Update prevDestValues with current value ====
   * Will be referenced at the Pre-pass of the next block
   * Active voices only
   * ==============================================================
   */
  for (uint32_t i = 0; i < pool.activeCount; i++) {
    uint32_t v = pool.activeIndices[i];
    pool.modMatrix.prevDestValues[ModDest::Osc1Pitch][v] =
        pool.modMatrix.destValues[ModDest::Osc1Pitch][v];

    pool.modMatrix.prevDestValues[ModDest::Osc2Pitch][v] =
        pool.modMatrix.destValues[ModDest::Osc2Pitch][v];

    pool.modMatrix.prevDestValues[ModDest::Osc3Pitch][v] =
        pool.modMatrix.destValues[ModDest::Osc3Pitch][v];

    pool.modMatrix.prevDestValues[ModDest::SubOscPitch][v] =
        pool.modMatrix.destValues[ModDest::SubOscPitch][v];
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
