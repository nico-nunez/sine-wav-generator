#ifndef SYNTH_ENVELOPE_H
#define SYNTH_ENVELOPE_H

#include "synth/Oscillator.h"

namespace Synth {
class Envelope {
public:
  Envelope(float sampleRate = Synth::DEFAULT_SAMPLE_RATE);

  /*
   * ADSR Configuration:
   * Attack, Decay, Release in milliseconds (float)
   * Sustain is amplitude (0.0 - 1.0)
   */

  // Stage 1
  void setAttack(float ms);
  float getAttack() const;

  // Stage 2
  void setDecay(float ms);
  float getDecay() const;

  // Stage 3
  void setSustain(float level);
  float getSustain() const;

  // Stage 4
  void setRelease(float ms);
  float getRelease() const;

  void setSampleRate(float sampleRate);
  float getSampleRate() const;

  // Envelope Control
  void trigger();        // Start attack stage
  void release();        // Start release stage
  float getNextSample(); // Get current amplitude (0.0 - 1.0)
  bool isDone() const;   // Has envelope stages completed

private:
  enum class State { Idle, Attack, Decay, Sustain, Release };

  // Time in milliseconds
  float mAttackMs;
  float mDecayMs;
  float mReleaseMs;

  // Increments per sample: 1.0f / numSamples (numSamples are derived from time)
  float mAttackIncrement;
  float mDecayIncrement;
  float mReleaseIncrement;

  // Percentage as float (0.0 - 1.0)
  float mSustainLevel;

  float mSampleRate;

  // Runtime State
  State mState = State::Idle;
  float mStateProgress = 0;
  float mReleaseStartLevel; // Current level when release stage begins
                            // usually same as sustain level but could be
                            // value in the middle of Attack or Decay stages

  // Helpers
  float getCurrentAmplitude() const;
  float convertMsToIncrement(float ms) const;
  void updateIncrements(); // Recalculates ADR increment values

  // Stage calculations
  float calculateAttack() const;
  float calculateDecay() const;
  float calculateRelease() const;
};
} // namespace Synth

#endif // !SYNTH_ENVELOPE_H
