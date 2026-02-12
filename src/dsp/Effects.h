#pragma once

namespace dsp::effects {
float hardClip(float sample, float threshold);

float denormalizeDrive(float drive);

// Should pass NORMALIZED _drive_ value. However,
// will return whatever _drive_ is (Normalized or Denormalized)
float calcInvDrive(float drive);

// NOTE(nico): _drive_ and _invDrive_ should be denormalized
// but _mix_ still normalized
float softClip(float sample, float drive, float invDrive, float mix = 1.0);
float softClipFast(float sample);

// NOTE(nico): _drive_ should be denormalized
// _bias_ should be normalized
float tapeSimulation(float sample, float drive, float bias);

/* DC Block is a high-pass filter that removes DC offset (constant bias) from
 * the audio signal.
 *
 * Why you need it:
 * - Saturation creates DC - asymmetric clipping shifts the average value
 * - Oscillator drift - numerical errors can accumulate over time
 * - Speaker protection - DC can damage speakers (wasted power, cone movement)
 *
 * The coefficient (default 0.995):
 * - Higher (0.999) = removes ONLY DC, preserves sub-bass
 * - Lower (0.99) = more aggressive, removes more low frequencies
 * - 0.995 is a good default - cutoff around 3-5 Hz
 */
float dcBlock(float sample, float &state, float coefficient = 0.995f);
} // namespace dsp::effects
