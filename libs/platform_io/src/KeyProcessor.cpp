#include "platform_io/KeyProcessor.h"
#include "device_io/KeyCapture.h"
#include "platform_io/NoteEventQueue.h"
#include <cstdio>

namespace platform_io {
// Handle keyboard events
static void keyEventCallback(device_io::KeyEvent event, void *userContext) {
  auto ctx = static_cast<NoteEventQueue *>(userContext);

  // Currently 'z' & 'x' control octive up/down
  // Need to ignore keyup (note off) for now
  if ((event.character == 120 || event.character == 122) &&
      event.type == device_io::KeyEventType::KeyUp) {
    return;
  } else if (event.type == device_io::KeyEventType::KeyDown) {
    // Note "ON" event
    ctx->push(
        NoteEvent{NoteEventType::NoteOn, asciiToMidi(event.character), 127});
  } else if (event.type == device_io::KeyEventType::KeyUp) {
    // Note "OFF" event
    ctx->push(
        NoteEvent{NoteEventType::NoteOff, asciiToMidi(event.character), 127});
  }
  // "ESC" to quit
  if (event.type == device_io::KeyEventType::KeyDown && event.keyCode == 53) {
    printf("ESC pressed, stopping...\n");
    device_io::stopKeyCaptureLoop();
  }
}

int startKeyInputCapture(NoteEventQueue &eventQueue) {
  printf("KeyCapture Example\n");
  printf("------------------\n");
  printf("Press keys to see events. ESC to quit.\n\n");

  // 1. Initialize Cocoa app
  device_io::initKeyCaptureApp();

  // 2. Create a minimal window (required for local capture without permissions)
  device_io::WindowConfig config = device_io::defaultWindowConfig();
  config.title = "Key Capture Demo";
  config.width = 500;
  config.height = 500;

  if (!createCaptureWindow(config)) {
    printf("Failed to create window\n");
    return 1;
  }

  // 3. Start capturing with local mode (no permissions needed when window
  // focused)
  //    Change to CaptureMode::Global if you need capture when not focused
  //    Change to CaptureMode::Both if you want both behaviors
  if (!startKeyCapture(keyEventCallback, &eventQueue,
                       device_io::CaptureMode::Local)) {
    printf("Failed to start key capture\n");
    return 1;
  }

  // Update window text
  device_io::setWindowText("Press keys... (ESC to quit)");

  // 4. Run the event loop (blocks until stopKeyCaptureLoop() called)
  device_io::runKeyCaptureLoop();

  // 5. Cleanup
  device_io::stopKeyCapture();

  printf("Done.\n");
  return 0;
}

MIDINote asciiToMidi(char key) {
  static constexpr uint8_t SEMITONES = 12;
  static uint8_t octiveOffset = 0;

  MIDINote midiKey = 0;

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

} // namespace platform_io
