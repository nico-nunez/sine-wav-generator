#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace audio_io {
enum class KeyEventType { KeyUp, KeyDown };

struct KeyEvent {
  KeyEventType type;
  uint8_t keycode;
  uint8_t asciiCode;
};

struct KeyEventQueue {
  // NOTE(nico): SIZE value need to be power of to use bitmasking for wrapping
  // Alternative is modulo (%) which is more expensive
  static constexpr size_t SIZE{256};
  static constexpr size_t WRAP{SIZE - 1};

  KeyEvent queue[SIZE];

  std::atomic<size_t> readIndex{0};
  std::atomic<size_t> writeIndex{0};

  // Methods
  bool push(const KeyEvent &event);
  bool pop(KeyEvent &event);

  void printEvent(KeyEvent &event);
  void printQueue();
};

} // namespace audio_io
