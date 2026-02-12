#ifndef SYNTH_ENVELOPE_H
#define SYNTH_ENVELOPE_H

#include "Oscillator.h"

namespace Synth {
class Envelope {
public:
  struct Settings {
    float attack;  // milliseconds
    float decay;   // milliseconds
    float sustain; // percentage (0.0 - 1.0)
    float release; // milliseconds

    /*
     * Constructor needs to be defined in order to use default values within
     * Envelope constructor.  Default memember initialization values are NOT
     * "available" until entire class has been parsed.
     *
     * Basically, without the constructor, the default (member) values aren't
     * known to the compiler
     */
    Settings(float a = 50.0f, float d = 100.0f, float s = 0.7f,
             float r = 200.0f)
        : attack(a), decay(d), sustain(s), release(r) {}
  };

  Envelope(float sampleRate = Synth::DEFAULT_SAMPLE_RATE,
           const Settings &settings = {});

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
  void noteOn();            // Start attack stage (start envelope)
  void noteOff();           // Start release stage (finish envelope)
  float process();          // Get current amplitude (0.0 - 1.0)
  bool isComplete() const;  // Has envelope stages completed
  bool isReleasing() const; // Has envelope stages completed

private:
  enum class State { Idle, Attack, Decay, Sustain, Release };

  float mSampleRate;
  Settings mSettings;

  // Increments per sample: 1.0f / numSamples (numSamples are derived from time)
  float mAttackIncrement;
  float mDecayIncrement;
  float mReleaseIncrement;

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
