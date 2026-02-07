#include "Voice.h"
#include "Oscillator.h"
#include "platform/NoteEventQueue.h"
#include "utils/SynthUtils.h"

namespace Synth {
Voice::Voice(OscillatorType oscType, float sampleRate)
    : mOscillator(oscType, sampleRate), mEnvelope(sampleRate) {}

void Voice::noteOn(const platform::NoteEvent &event) {
  mOscillator.setFrequency(SynthUtils::midiToFrequency(event.midiNote));
  mIsActive = true;
  mMidiNote = event.midiNote;

  mEnvelope.noteOn();
}

void Voice::noteOff() { mEnvelope.noteOff(); }

bool Voice::shouldStop(platform::MIDINote midiNote) {
  return mIsActive && !isReleasing() && mMidiNote == midiNote;
}

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
