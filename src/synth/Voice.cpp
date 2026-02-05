#include "Voice.h"
#include "Oscillator.h"
#include "utils/NoteEventQueue.h"
#include "utils/SynthUtils.h"

namespace Synth {
Voice::Voice(OscillatorType oscType, float sampleRate)
    : mOscillator(oscType, sampleRate), mEnvelope(sampleRate) {}

void Voice::noteOn(const utils::NoteEvent &event) {
  mOscillator.setFrequency(SynthUtils::midiToFrequency(event.note));
  mIsActive = true;

  mEnvelope.noteOn();
}

void Voice::noteOff() { mEnvelope.noteOff(); }

float Voice::process() {
  if (!mIsActive)
    return 0.0f;

  float oscValue = mOscillator.process();
  float envValue = mEnvelope.process();

  if (mEnvelope.isComplete())
    mIsActive = false;

  return oscValue * envValue;
}

bool Voice::isAvailable() const { return !mIsActive; }
bool Voice::isReleasing() const { return mEnvelope.isReleasing(); }

void Voice::setOscillatorType(OscillatorType oscType) {
  mOscillator.setOscillatorType(oscType);
}

void Voice::setSampleRate(float sampleRate) {
  mOscillator.setSampleRate(sampleRate);
  mEnvelope.setSampleRate(sampleRate);
}

} // namespace Synth
