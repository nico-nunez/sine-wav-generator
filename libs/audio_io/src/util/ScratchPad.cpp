#include <cstdint>
#include <cstdlib>

namespace scratch_pad {
class AudioArena {
public:
  AudioArena(size_t sizeInBytes) {
    // Aligned to 32 bytes for AVX/SIMD performance
    data = (uint8_t *)std::aligned_alloc(32, sizeInBytes);
    capacity = sizeInBytes;
    offset = 0;
  }

  ~AudioArena() { std::free(data); }

  // The "Pointer Bump" allocation
  void *alloc(size_t size) {
    if (offset + size > capacity)
      return nullptr; // Out of memory!
    void *ptr = data + offset;
    offset += size;
    return ptr;
  }

  void reset() { offset = 0; }

private:
  uint8_t *data;
  size_t capacity;
  size_t offset;
};

// ===============================
class MySynth {
public:
  // This is called by your CoreAudio wrapper every block
  void process(float **outputBuffers, int numChannels, int numSamples,
               AudioArena &scratchpad) {
    // 1. Reset the arena at the start of the block
    scratchpad.reset();

    // 2. Need temporary space for a filter or envelope?
    // Grab it from the arena instead of using 'new'
    float *tempBuffer = (float *)scratchpad.alloc(numSamples * sizeof(float));

    for (int ch = 0; ch < numChannels; ++ch) {
      float *channelData = outputBuffers[ch]; // CoreAudio's pointers
                                              // Do your DSP here...
    }
  }
};

} // namespace scratch_pad
