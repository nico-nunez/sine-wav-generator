#ifndef SYNTH_UTILS_NOTE_TABLE_H
#define SYNTH_UTILS_NOTE_TABLE_H

namespace SynthUtils {

// Pre-calcuate all frequency values if performance is absolutely critical
class NoteTable {
public:
  NoteTable();

  float getFrequency(int midiNote) const;

private:
  static constexpr int ROOT_MIDI{69};       // A4
  static constexpr float ROOT_FREQ{440.0f}; // A4

  static constexpr int MIDI_NOTE_COUNT{128};

  float mFreqquencyTable[MIDI_NOTE_COUNT];

  // Helper methods
  void generateNoteTable();
};

} // namespace SynthUtils

#endif // !SYNTH_UTILS_NOTE_TABLE_H
