#include "synth/Envelope.h"
#include <stdexcept>

namespace Synth {
Envelope::Envelope(float sampleRate) : mSampleRate(sampleRate) {
  setAttack(10.0f);
  setDecay(100.0f);
  setSustain(0.7f);
  setRelease(200.0f);
}

// Attack (Stage 1)
void Envelope::setAttack(float ms) {
  if (ms < 0)
    throw std::runtime_error("Invalid Value: must be positive");

  mAttackMs = ms;
  mAttackIncrement = convertMsToIncrement(ms);
}
float Envelope::getAttack() const { return mAttackMs; }

// Decay (Stage 2)
void Envelope::setDecay(float ms) {
  if (ms < 0)
    throw std::runtime_error("Invalid Value: must be positive");

  mDecayMs = ms;
  mDecayIncrement = convertMsToIncrement(ms);
}
float Envelope::getDecay() const { return mDecayMs; }

// Sustain (Stage 3)
void Envelope::setSustain(float value) {
  if (value < 0 || value > 1)
    throw std::runtime_error("Invalid Value: must be >=0.0 and <=1.0");
  mSustainLevel = value;
}
float Envelope::getSustain() const { return mSustainLevel; }

// Release (Stage 4)
void Envelope::setRelease(float ms) {
  if (ms < 0)
    throw std::runtime_error("Invalid Value: must be positive");

  mReleaseMs = ms;
  mReleaseIncrement = convertMsToIncrement(ms);
}
float Envelope::getRelease() const { return mReleaseMs; }

// Sample Rate
void Envelope::setSampleRate(float sampleRate) {
  if (sampleRate < 0)
    throw std::runtime_error("Invalid Value: must be positive");

  mSampleRate = sampleRate;
  updateIncrements();
}
float Envelope::getSampleRate() const { return mSampleRate; }

// Envelope Control Methods
void Envelope::trigger() {
  mStateProgress = 0;
  mState = State::Attack;
}

void Envelope::release() {
  mReleaseStartLevel = getCurrentAmplitude();
  mState = State::Release;
  mStateProgress = 0;
}

float Envelope::getNextSample() {
  float amplitude = 0.0f;

  switch (mState) {

  case State::Attack:
    amplitude = calculateAttack();
    mStateProgress += mAttackIncrement;

    if (mStateProgress >= 1.0f) {
      mState = State::Decay;
      mStateProgress = 0;
    }
    break;

  case State::Decay:
    amplitude = calculateDecay();
    mStateProgress += mDecayIncrement;

    if (mStateProgress >= 1.0f) {
      mState = State::Sustain;
      mStateProgress = 0;
    }
    break;

  case State::Sustain:
    amplitude = mSustainLevel;
    break;

  case State::Release:
    amplitude = calculateRelease();
    mStateProgress += mReleaseIncrement;

    if (mStateProgress >= 1.0f) {
      mState = State::Idle;
      mStateProgress = 0;
    }
    break;

  case State::Idle:
    amplitude = 0.0f;
    break;
  }

  return amplitude;
}

bool Envelope::isDone() const { return mState == State::Idle; }

// Helper Methods
float Envelope::getCurrentAmplitude() const {
  switch (mState) {
  case State::Attack:
    return calculateAttack();
  case State::Decay:
    return calculateDecay();
  case State::Sustain:
    return mSustainLevel;
  case State::Release:
    return calculateRelease();
  case State::Idle:
    return 0.0f;
  default:
    return 0.0f;
  }
}

float Envelope::convertMsToIncrement(float ms) const {
  float totalStateSamples = (ms / 1000.f) * mSampleRate;

  if (totalStateSamples == 0)
    return 0;

  return 1.0f / totalStateSamples;
}

void Envelope::updateIncrements() {
  setAttack(mAttackMs);
  setDecay(mDecayMs);
  setRelease(mReleaseMs);
}

// Amplitude Calculations
float Envelope::calculateAttack() const {
  if (mAttackIncrement == 0)
    return 1.0f;

  return mStateProgress;
}

float Envelope::calculateDecay() const {
  if (mDecayIncrement == 0)
    return 1.0f;

  float decayRange = 1.0f - mSustainLevel;
  return 1.0f - (mStateProgress * decayRange);
}

float Envelope::calculateRelease() const {
  if (mReleaseIncrement == 0)
    return 1.0f;
  return mReleaseStartLevel * (1.0f - mStateProgress);
}

} // namespace Synth
