#include "synth/Envelope.h"
#include <stdexcept>

namespace Synth {
Envelope::Envelope(float sampleRate, const Settings &settings)
    : mSampleRate(sampleRate), mSettings(settings) {
  updateIncrements();
}

// Attack (Stage 1)
void Envelope::setAttack(float ms) {
  if (ms < 0)
    throw std::runtime_error("Invalid Value: must be positive");

  mSettings.attack = ms;
  mAttackIncrement = convertMsToIncrement(ms);
}
float Envelope::getAttack() const { return mSettings.attack; }

// Decay (Stage 2)
void Envelope::setDecay(float ms) {
  if (ms < 0)
    throw std::runtime_error("Invalid Value: must be positive");

  mSettings.decay = ms;
  mDecayIncrement = convertMsToIncrement(ms);
}
float Envelope::getDecay() const { return mSettings.decay; }

// Sustain (Stage 3)
void Envelope::setSustain(float value) {
  if (value < 0 || value > 1)
    throw std::runtime_error("Invalid Value: must be >=0.0 and <=1.0");
  mSettings.sustain = value;
}
float Envelope::getSustain() const { return mSettings.sustain; }

// Release (Stage 4)
void Envelope::setRelease(float ms) {
  if (ms < 0)
    throw std::runtime_error("Invalid Value: must be positive");

  mSettings.release = ms;
  mReleaseIncrement = convertMsToIncrement(ms);
}
float Envelope::getRelease() const { return mSettings.release; }

// Sample Rate
void Envelope::setSampleRate(float sampleRate) {
  if (sampleRate < 0)
    throw std::runtime_error("Invalid Value: must be positive");

  mSampleRate = sampleRate;
  updateIncrements();
}
float Envelope::getSampleRate() const { return mSampleRate; }

// Envelope Control Methods
void Envelope::noteOn() {
  mStateProgress = 0;
  mState = State::Attack;
}

void Envelope::noteOff() {
  mReleaseStartLevel = getCurrentAmplitude();
  mState = State::Release;
  mStateProgress = 0;
}

float Envelope::process() {
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
    amplitude = mSettings.sustain;
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

bool Envelope::isComplete() const { return mState == State::Idle; }

// Helper Methods
float Envelope::getCurrentAmplitude() const {
  switch (mState) {
  case State::Attack:
    return calculateAttack();
  case State::Decay:
    return calculateDecay();
  case State::Sustain:
    return mSettings.sustain;
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
  setAttack(mSettings.attack);
  setDecay(mSettings.decay);
  setRelease(mSettings.release);
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

  float decayRange = 1.0f - mSettings.sustain;
  return 1.0f - (mStateProgress * decayRange);
}

float Envelope::calculateRelease() const {
  if (mReleaseIncrement == 0)
    return 1.0f;
  return mReleaseStartLevel * (1.0f - mStateProgress);
}

} // namespace Synth
