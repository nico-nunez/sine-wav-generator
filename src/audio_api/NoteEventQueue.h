#pragma once

#include <atomic>
#include <cstddef>
#include <cstdio>

namespace audio_api {
enum class NoteEventType { NoteOff, NoteOn };

struct NoteEvent {
  NoteEventType type;
  uint8_t note;
  uint8_t velocity;
};

struct NoteEventQueue {
  // NOTE(nico): SIZE value need to be power of to use bitmasking for wrapping
  // Alternative is modulo (%) which is more expensive
  static constexpr size_t SIZE{256};
  static constexpr size_t WRAP{SIZE - 1};

  NoteEvent queue[SIZE];

  std::atomic<size_t> readIndex{0};
  std::atomic<size_t> writeIndex{0};

  bool push(const NoteEvent &event) {
    size_t currentIndex = writeIndex.load();
    size_t nextIndex = (currentIndex + 1) & WRAP;

    if (nextIndex == readIndex.load())
      return false;

    queue[currentIndex] = event;
    writeIndex.store(nextIndex);

    return true;
  }

  bool pop(NoteEvent &event) {
    size_t currentIndex = readIndex.load();

    if (currentIndex == writeIndex.load())
      return false;

    event = queue[currentIndex];
    readIndex.store((currentIndex + 1) & WRAP);

    return true;
  }

  void printEvent(NoteEvent &event) {
    printf("==== Event ====\n");
    printf("type: %d\n", (int)event.type);
    printf("midi: %d\n", event.note);
    printf("velocity: %d\n", event.velocity);
  }

  void printQueue() {
    size_t currentIndex = readIndex.load();
    size_t endIndex = writeIndex.load();

    // Only print events that are able to be read
    printf("======== Event Queue ========\n");
    for (; currentIndex < endIndex; currentIndex++) {
      printEvent(queue[currentIndex]);
    }
  }
};

} // namespace audio_api
