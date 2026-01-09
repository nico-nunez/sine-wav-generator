#include "SynthUtils.h"
#include "synth/Engine.h"

#include <cctype>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace SynthUtils {

/* Semitone ratio is 2^(1/12). This is multiplied by the number
 * of semitones from A4 for the note that's to be calculated.
 * This value (ratio * semitones) is multiplied by 440hz (A4)
 * to get the value of the desired note in hertz.
 *
 * Example: C4 = 440hz * (2^((1/12) * -9)), where C4 is -9 semitones from A4
 */

float semitoneToFrequency(int semitones) {
  // Pre-caluated value of 2^(1/12)
  static constexpr float SEMITONE_RATIO = 1.059463094f;

  // Cast `semitones` to float since std::pow is optimized for floats(32-bit)
  return ROOT_NOTE_FREQ * static_cast<float>(std::pow(
                              SEMITONE_RATIO, static_cast<float>(semitones)));
}

float midiToFrequency(int midiValue) {
  return semitoneToFrequency(midiValue - ROOT_NOTE_MIDI);
}

int noteNameToMidi(std::string_view noteName) {
  if (noteName.empty())
    throw std::invalid_argument("Empty note name");

  // Map note characters to semitone offsets from 'C'
  static const std::unordered_map<char, int> noteOffests{
      {'C', 0}, {'D', 2}, {'E', 4}, {'F', 5}, {'G', 7}, {'A', 9}, {'B', 11}};

  size_t index = 0;

  // Get first character then increment index
  char noteLetter = static_cast<char>(std::toupper(noteName[index++]));

  // Ensure valid note (A-G)
  if (noteOffests.find(noteLetter) == noteOffests.end()) {
    throw std::invalid_argument("Invalid note letter: " +
                                std::string(1, noteLetter));
  }

  int noteSemitone = noteOffests.at(noteLetter);

  // Check if sharp ('#') or flat ('b')
  if (index < noteName.length()) {
    if (noteName[index] == '#') {
      noteSemitone++;
      index++;
    } else if (noteName[index] == 'b') {
      noteSemitone--;
      index++;
    }
  }

  // Get octive value (e.g., 0, 1, 2, etc.)
  if (index >= noteName.length())
    throw std::invalid_argument("Missing octave number");

  int noteOctive = noteName[index] - '0';
  if (noteOctive < 0 || noteOctive > 9)
    throw std::invalid_argument("Note octave out of range (0-9): " +
                                std::to_string(noteOctive));

  int midiNote = (noteOctive + 1) * 12 + noteSemitone;
  if (midiNote < 0 || midiNote > 127)
    throw std::invalid_argument("MIDI Note out of range (0-127): " +
                                std::to_string(midiNote));
  return midiNote;
}

float noteNameToFrequency(std::string_view noteName) {
  int midiNote{noteNameToMidi(noteName)};
  return midiToFrequency(midiNote);
}

Synth::NoteEvent createNoteEvent(const float frequency) {
  return Synth::NoteEvent{frequency};
}

Synth::NoteEventGroup
createEventGroupFromNotes(const std::vector<std::string> &notes) {
  Synth::NoteEventGroup eventGroup{};
  eventGroup.reserve(notes.size());

  for (const std::string &note : notes) {
    float noteFrequency{SynthUtils::noteNameToFrequency(note)};
    eventGroup.emplace_back(noteFrequency);
  }
  return eventGroup;
}

} // namespace SynthUtils
