#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <cstddef>
#include <cstdio>

#include "input_io/RawTerminal.h"
#include "platform/NoteEventQueue.h"

namespace audio_io {

static CGEventRef event_callback(CGEventTapProxy /*proxy*/, CGEventType type,
                                 CGEventRef event, void *refcon) {
  auto ctx = static_cast<platform::NoteEventQueue *>(refcon);

  if (type == kCGEventKeyDown || type == kCGEventKeyUp) {
    // CGKeyCode keycode =
    //     (CGKeyCode)CGEventGetIntegerValueField(event,
    //     kCGKeyboardEventKeycode);

    // Get ASCII Value
    char asciiValue{};
    UniChar unicodeString[1];
    UniCharCount stringLength;

    if (CGEventGetIntegerValueField(event, kCGKeyboardEventAutorepeat))
      return NULL;

    CGEventKeyboardGetUnicodeString(event, 1, &stringLength, unicodeString);
    if (stringLength) {
      asciiValue = (char)unicodeString[0];
    }

    int is_down = (type == kCGEventKeyDown);

    // printf("keycode: %u | ASCII: %d %s\n", keycode, asciiValue,
    //        is_down ? "down" : "up");

    // Stop Capturing ('q' or 'Q')
    if (asciiValue == 113 || asciiValue == 81) {
      CFRunLoopStop(CFRunLoopGetCurrent());
      printf("Thanks for playing....Goodbye :)\n");

    } else if ((asciiValue == 120 || asciiValue == 122) && !is_down) {
      return NULL;
    } else {
      // Push NoteEvent
      ctx->push(platform::NoteEvent{is_down ? platform::NoteEventType::NoteOn
                                            : platform::NoteEventType::NoteOff,
                                    input_io::asciiToMidi(asciiValue), 127});
    }

    fflush(stdout);
  }

  return NULL;
}

int checkAccessibilityPermissions() {

  // 1. Create a dictionary to tell macOS to prompt the user if no access
  const void *keys[] = {kAXTrustedCheckOptionPrompt};
  const void *values[] = {kCFBooleanTrue};

  CFDictionaryRef options = CFDictionaryCreate(
      NULL, keys, values, 1, &kCFCopyStringDictionaryKeyCallBacks,
      &kCFTypeDictionaryValueCallBacks);

  // 2. Check and/or trigger the system prompt
  bool trusted = AXIsProcessTrustedWithOptions(options);
  CFRelease(options);

  if (!trusted) {
    printf("Permission denied. Check System Settings > Privacy & Security > "
           "Accessibility.\n");
    return 1;
  }
  return 0;
}

int captureKeyEvents(platform::NoteEventQueue &eventQueue) {
  CGEventMask mask =
      CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventKeyUp);

  CFMachPortRef tap =
      CGEventTapCreate(kCGSessionEventTap, // session-level
                       kCGHeadInsertEventTap, kCGEventTapOptionDefault, mask,
                       event_callback, &eventQueue);

  if (!tap) {
    fprintf(stderr, "Failed to create event tap\n");
    return 1;
  }

  printf("Synth running....Press 'Q' or 'q' to quit:\n");

  CFRunLoopSourceRef src =
      CFMachPortCreateRunLoopSource(kCFAllocatorDefault, tap, 0);

  CFRunLoopAddSource(CFRunLoopGetCurrent(), src, kCFRunLoopCommonModes);

  CGEventTapEnable(tap, true);
  CFRunLoopRun();
  return 0;
};

} // namespace audio_io
