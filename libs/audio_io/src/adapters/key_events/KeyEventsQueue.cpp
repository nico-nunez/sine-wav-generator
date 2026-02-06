#include "audio_io/KeyEventsQueue.h"
#include <cstddef>

namespace audio_io {

bool KeyEventQueue::push(const KeyEvent &event) {
  size_t currentIndex = writeIndex.load();
  size_t nextIndex = (currentIndex + 1) & WRAP;

  if (nextIndex == readIndex.load())
    return false;

  queue[currentIndex] = event;
  writeIndex.store(nextIndex);

  return true;
}

bool KeyEventQueue::pop(KeyEvent &event) {
  size_t currentIndex = readIndex.load();

  if (currentIndex == writeIndex.load())
    return false;

  event = queue[currentIndex];
  readIndex.store((currentIndex + 1) & WRAP);

  return true;
}

void KeyEventQueue::printEvent(KeyEvent &event) {
  printf("==== Event ====\n");
  printf("type: %d\n", (int)event.type);
  printf("midi: %d\n", event.keycode);
  printf("velocity: %d\n", event.asciiCode);
}

void KeyEventQueue::printQueue() {
  size_t currentIndex = readIndex.load();
  size_t endIndex = writeIndex.load();

  // Only print events that are able to be read
  printf("======== Event Queue ========\n");
  for (; currentIndex < endIndex; currentIndex++) {
    printEvent(queue[currentIndex]);
  }
}

} // namespace audio_io
