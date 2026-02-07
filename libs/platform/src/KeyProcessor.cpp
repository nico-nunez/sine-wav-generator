#include "input_io/KeyCapture.h"
#include "input_io/RawTerminal.h"
#include "platform/NoteEventQueue.h"
#include <cstdio>

namespace platform {
// Handle keyboard events
static void keyEventCallback(KeyEvent event, void *userContext) {
  auto ctx = static_cast<NoteEventQueue *>(userContext);

  // Currently 'z' & 'x' control octive up/down
  // Need to ignore keyup (note off) for now
  if ((event.character == 120 || event.character == 122) &&
      event.type == KeyEventType::KeyUp) {
    return;
  } else if (event.type == KeyEventType::KeyDown) {
    // Note "ON" event
    ctx->push(NoteEvent{NoteEventType::NoteOn,
                        input_io::asciiToMidi(event.character), 127});
  } else if (event.type == KeyEventType::KeyUp) {
    // Note "OFF" event
    ctx->push(NoteEvent{NoteEventType::NoteOff,
                        input_io::asciiToMidi(event.character), 127});
  }
  // "ESC" to quit
  if (event.type == KeyEventType::KeyDown && event.keyCode == 53) {
    printf("ESC pressed, stopping...\n");
    stopKeyCaptureLoop();
  }
}

int startKeyInputCapture(NoteEventQueue &eventQueue) {
  printf("KeyCapture Example\n");
  printf("------------------\n");
  printf("Press keys to see events. ESC to quit.\n\n");

  // 1. Initialize Cocoa app
  initKeyCaptureApp();

  // 2. Create a minimal window (required for local capture without permissions)
  WindowConfig config = defaultWindowConfig();
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
  if (!startKeyCapture(keyEventCallback, &eventQueue, CaptureMode::Local)) {
    printf("Failed to start key capture\n");
    return 1;
  }

  // Update window text
  setWindowText("Press keys... (ESC to quit)");

  // 4. Run the event loop (blocks until stopKeyCaptureLoop() called)
  runKeyCaptureLoop();

  // 5. Cleanup
  stopKeyCapture();

  printf("Done.\n");
  return 0;
}

} // namespace platform
