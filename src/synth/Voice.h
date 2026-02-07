#ifndef SYNTH_VOICE_H
#define SYNTH_VOICE_H

#include "platform_io/NoteEventQueue.h"
#include "synth/Envelope.h"
#include "synth/Oscillator.h"

namespace Synth {
struct NoteEvent;

class Voice {
public:
  Voice(OscillatorType oscType = OscillatorType::Saw,
        float sampleRate = DEFAULT_SAMPLE_RATE);

  void noteOn(const platform_io::NoteEvent &event);
  void noteOff();

  float process();

  bool isAvailable() const;
  bool isReleasing() const;

  void setOscillatorType(OscillatorType oscType);
  void setSampleRate(float sampleRate);

  bool shouldStop(platform_io::MIDINote midiNote);

private:
  Oscillator mOscillator;
  Envelope mEnvelope;
  platform_io::MIDINote mMidiNote;
  bool mIsActive = false;
};

} // namespace Synth

#endif // !SYNTH_VOICE_H
