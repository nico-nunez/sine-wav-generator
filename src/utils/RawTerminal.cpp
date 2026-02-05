#include "RawTerminal.h"
#include "utils/NoteEventQueue.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

namespace utils {
static struct termios orig_termios;

void enableRawTerminal() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawTerminal);

  struct termios raw = orig_termios;
  raw.c_iflag &= ~((unsigned long)BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~((unsigned long)OPOST);
  raw.c_lflag &= ~((unsigned long)ECHO | ICANON | IEXTEN);

  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

  printf("Raw mode enabled. Type anything (Ctrl+C to quit):\r\n");
}

void disableRawTerminal() {
  tcsetattr(STDERR_FILENO, TCSAFLUSH, &orig_termios);
}

void captureKeyboardInputs(NoteEventQueue &eventQueue) {
  char c;
  while (1) {
    c = '\0';
    if (read(STDIN_FILENO, &c, 1) == 1) {
      if (iscntrl(c)) {
        printf("%d\r\n", c); // Print ASCII code for control keys
      } else {
        if (c == 'Q') {
          disableRawTerminal();
          printf("Raw mode disable\r\n");
          break;
        } else {
          uint8_t midiNote{asciiToMidi(c)};
          if (midiNote) {
            eventQueue.push(
                utils::NoteEvent{utils::NoteEventType::NoteOn, midiNote, 100});
            printf("NoteEvent added to queue: '%c' - MIDI: %d\r\n", c,
                   midiNote);
          }
        }
      }
    }
  }
}

uint8_t asciiToMidi(char key) {
  static constexpr uint8_t SEMITONES = 12;
  static uint8_t octiveOffset = 0;

  uint8_t midiKey = 0;

  // Change Octive
  if (key == 122) { // ('z')
    --octiveOffset;
  }

  if (key == 120) { // ('x')
    ++octiveOffset;
  }

  // Change Velocity
  // 99  // ('c')
  // 118 // ('v')

  switch (key) {
  case 97: //  ('a') "C"  64
    midiKey = 64;
    break;
  case 119: // ('w') "C#" 65
    midiKey = 65;
    break;
  case 115: // ('s') "D"  66
    midiKey = 66;
    break;
  case 101: // ('e') "D#" 67
    midiKey = 67;
    break;
  case 100: // ('d') "E"  68
    midiKey = 68;
    break;
  case 102: // ('f') "F"  69
    midiKey = 69;
    break;
  case 116: // ('t') "F#" 70
    midiKey = 70;
    break;
  case 103: // ('g') "G"  71
    midiKey = 71;
    break;
  case 121: // ('y') "G#" 72
    midiKey = 72;
    break;
  case 104: // ('h') "A"  73
    midiKey = 73;
    break;
  case 117: // ('u') "A#" 74
    midiKey = 74;
    break;
  case 106: // ('j') "B"  75
    midiKey = 75;
    break;
  case 107: // ('k') "C"  76
    midiKey = 76;
    break;
  case 111: // ('o') "C#" 77
    midiKey = 77;
    break;
  case 108: // ('l') "D"  78
    midiKey = 78;
    break;
  case 112: // ('p') "D#" 79
    midiKey = 79;
    break;

  default:
    return 0; // unmapped key
  }

  return midiKey + (octiveOffset * SEMITONES);
}

} // namespace utils
