#include "NoteTable.h"

#include <cmath>
#include <stdexcept>
#include <string>

namespace SynthUtils {

NoteTable::NoteTable() { generateNoteTable(); };

float NoteTable::getFrequency(int midiNote) const {
  if (midiNote < 0 || midiNote >= MIDI_NOTE_COUNT)
    throw std::out_of_range("MIDI note out of range (0-127): " +
                            std::to_string(midiNote));

  return mFreqquencyTable[midiNote];
}

// Helper methods
void NoteTable::generateNoteTable() {
  for (int i = 0; i < MIDI_NOTE_COUNT; i++) {
    mFreqquencyTable[i] = ROOT_FREQ * std::pow(2.0f, (i - 69) / 12.0f);
  }
}

} // namespace SynthUtils
