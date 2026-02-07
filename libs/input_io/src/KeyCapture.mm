// KeyCapture.mm
// Cocoa keyboard capture implementation

#include "input_io/KeyCapture.h"
#import <Cocoa/Cocoa.h>

// -----------------------------------------------------------------------------
// Static state
// -----------------------------------------------------------------------------

static id g_localMonitor = nil;
static id g_globalMonitor = nil;
static KeyCallback g_callback = nullptr;
static void *g_userData = nullptr;
static NSWindow *g_window = nil;
static NSTextField *g_textField = nil;

// -----------------------------------------------------------------------------
// Internal: Convert NSEvent to KeyEvent and dispatch
// -----------------------------------------------------------------------------

static void populateModifiers(KeyEvent &ke, NSEventModifierFlags flags) {
  ke.shift = (flags & NSEventModifierFlagShift) != 0;
  ke.ctrl = (flags & NSEventModifierFlagControl) != 0;
  ke.alt = (flags & NSEventModifierFlagOption) != 0;
  ke.cmd = (flags & NSEventModifierFlagCommand) != 0;
  ke.capsLock = (flags & NSEventModifierFlagCapsLock) != 0;
  ke.fn = (flags & NSEventModifierFlagFunction) != 0;
}

static void dispatchKeyEvent(NSEvent *event, KeyEventType type) {
  if (!g_callback)
    return;

  KeyEvent ke{};
  ke.type = type;
  ke.keyCode = event.keyCode;

  // Get ASCII character if available (not applicable for modifier events)
  if (type != KeyEventType::ModifierChanged && event.characters.length > 0) {
    unichar c = [event.characters characterAtIndex:0];
    ke.character = (c < 128) ? static_cast<char>(c) : 0;
  } else {
    ke.character = 0;
  }

  // Modifier flags
  populateModifiers(ke, event.modifierFlags);

  g_callback(ke, g_userData);
}

// -----------------------------------------------------------------------------
// App Delegate for handling app lifecycle
// -----------------------------------------------------------------------------

@interface KeyCaptureAppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation KeyCaptureAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
  // Make the app appear in dock and allow it to become active
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  [NSApp activateIgnoringOtherApps:YES];

  // Focus our window if it exists
  if (g_window) {
    [g_window makeKeyAndOrderFront:nil];
  }
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:
    (NSApplication *)sender {
  return YES;
}

@end

// -----------------------------------------------------------------------------
// Custom View that accepts key events
// -----------------------------------------------------------------------------

@interface KeyCaptureView : NSView
@end

@implementation KeyCaptureView

- (BOOL)acceptsFirstResponder {
  return YES;
}

- (BOOL)canBecomeKeyView {
  return YES;
}

// Prevent system beep on key press
- (void)keyDown:(NSEvent *)event {
  // Handled by monitor, but override to prevent beep
}

- (void)keyUp:(NSEvent *)event {
  // Handled by monitor
}

@end

// -----------------------------------------------------------------------------
// Public API Implementation
// -----------------------------------------------------------------------------

void initKeyCaptureApp() {
  // Initialize the shared application
  [NSApplication sharedApplication];

  // Set up app delegate
  KeyCaptureAppDelegate *delegate = [[KeyCaptureAppDelegate alloc] init];
  [NSApp setDelegate:delegate];
}

bool createCaptureWindow(WindowConfig config) {
  @autoreleasepool {
    // Create window
    NSRect frame = NSMakeRect(0, 0, config.width, config.height);
    NSWindowStyleMask style = NSWindowStyleMaskTitled |
                              NSWindowStyleMaskClosable |
                              NSWindowStyleMaskMiniaturizable;

    g_window = [[NSWindow alloc] initWithContentRect:frame
                                           styleMask:style
                                             backing:NSBackingStoreBuffered
                                               defer:NO];

    if (!g_window)
      return false;

    [g_window setTitle:[NSString stringWithUTF8String:config.title]];
    [g_window center];

    // Create custom view that accepts key input
    KeyCaptureView *captureView = [[KeyCaptureView alloc] initWithFrame:frame];
    [captureView setWantsLayer:YES];
    captureView.layer.backgroundColor = [[NSColor colorWithWhite:0.1
                                                           alpha:1.0] CGColor];

    // Create text label
    g_textField =
        [[NSTextField alloc] initWithFrame:NSMakeRect(10, 10, config.width - 20,
                                                      config.height - 20)];
    [g_textField setStringValue:@"Synth running..."];
    [g_textField setBezeled:NO];
    [g_textField setDrawsBackground:NO];
    [g_textField setEditable:NO];
    [g_textField setSelectable:NO];
    [g_textField setTextColor:[NSColor colorWithWhite:0.8 alpha:1.0]];
    [g_textField
        setFont:[NSFont monospacedSystemFontOfSize:14
                                            weight:NSFontWeightRegular]];
    [g_textField setAlignment:NSTextAlignmentCenter];

    [captureView addSubview:g_textField];
    [g_window setContentView:captureView];
    [g_window makeFirstResponder:captureView];

    if (config.showOnStart) {
      [g_window makeKeyAndOrderFront:nil];
    }

    return true;
  }
}

void setWindowText(const char *text) {
  if (g_textField && text) {
    [g_textField setStringValue:[NSString stringWithUTF8String:text]];
  }
}

static KeyEventType getKeyEventType(NSEvent *event) {
  switch (event.type) {
  case NSEventTypeKeyDown:
    return event.isARepeat ? KeyEventType::KeyRepeat : KeyEventType::KeyDown;
  case NSEventTypeKeyUp:
    return KeyEventType::KeyUp;
  case NSEventTypeFlagsChanged:
    return KeyEventType::ModifierChanged;
  default:
    return KeyEventType::KeyDown; // Fallback, shouldn't happen
  }
}

bool startKeyCapture(KeyCallback callback, void *userData, CaptureMode mode) {
  if (!callback)
    return false;

  g_callback = callback;
  g_userData = userData;

  NSEventMask mask =
      NSEventMaskKeyDown | NSEventMaskKeyUp | NSEventMaskFlagsChanged;

  // Set up local monitor (requires window focus)
  if (mode == CaptureMode::Local || mode == CaptureMode::Both) {
    g_localMonitor = [NSEvent
        addLocalMonitorForEventsMatchingMask:mask
                                     handler:^NSEvent *(NSEvent *event) {
                                       dispatchKeyEvent(event,
                                                        getKeyEventType(event));
                                       return event; // Return event to allow
                                                     // normal processing
                                     }];
  }

  // Set up global monitor (requires Accessibility permissions)
  if (mode == CaptureMode::Global || mode == CaptureMode::Both) {
    g_globalMonitor = [NSEvent
        addGlobalMonitorForEventsMatchingMask:mask
                                      handler:^(NSEvent *event) {
                                        dispatchKeyEvent(
                                            event, getKeyEventType(event));
                                      }];
  }

  // Check if at least one monitor was created
  bool success = false;
  if (mode == CaptureMode::Local) {
    success = (g_localMonitor != nil);
  } else if (mode == CaptureMode::Global) {
    success = (g_globalMonitor != nil);
  } else {
    success = (g_localMonitor != nil || g_globalMonitor != nil);
  }

  return success;
}

void stopKeyCapture() {
  if (g_localMonitor) {
    [NSEvent removeMonitor:g_localMonitor];
    g_localMonitor = nil;
  }
  if (g_globalMonitor) {
    [NSEvent removeMonitor:g_globalMonitor];
    g_globalMonitor = nil;
  }
  g_callback = nullptr;
  g_userData = nullptr;
}

void runKeyCaptureLoop() { [NSApp run]; }

void stopKeyCaptureLoop() { [NSApp terminate:nil]; }
