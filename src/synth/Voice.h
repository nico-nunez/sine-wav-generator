#ifndef SYNTH_VOICE_H
#define SYNTH_VOICE_H

#include "audio_api/NoteEventQueue.h"
#include "synth/Envelope.h"
#include "synth/Oscillator.h"

namespace Synth {
struct NoteEvent;

class Voice {
public:
  Voice(OscillatorType oscType = OscillatorType::Sine,
        float sampleRate = DEFAULT_SAMPLE_RATE);

  void noteOn(const audio_api::NoteEvent &event);
  void noteOff();

  float process();

  bool isAvailable() const;
  bool isReleasing() const;

  void setOscillatorType(OscillatorType oscType);
  void setSampleRate(float sampleRate);

private:
  Oscillator mOscillator;
  Envelope mEnvelope;
  bool mIsActive = false;
};

} // namespace Synth

#endif // !SYNTH_VOICE_H
