#ifndef SYNTH_VOICE_H
#define SYNTH_VOICE_H

#include "platform/NoteEventQueue.h"
#include "synth/Envelope.h"
#include "synth/Oscillator.h"

namespace Synth {
struct NoteEvent;

class Voice {
public:
  Voice(OscillatorType oscType = OscillatorType::Saw,
        float sampleRate = DEFAULT_SAMPLE_RATE);

  void noteOn(const platform::NoteEvent &event);
  void noteOff();

  float process();

  bool isAvailable() const;
  bool isReleasing() const;

  void setOscillatorType(OscillatorType oscType);
  void setSampleRate(float sampleRate);

  bool shouldStop(platform::MIDINote midiNote);

private:
  Oscillator mOscillator;
  Envelope mEnvelope;
  platform::MIDINote mMidiNote;
  bool mIsActive = false;
};

} // namespace Synth

#endif // !SYNTH_VOICE_H
