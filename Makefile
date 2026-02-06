CXX = clang++
DEBUG_FLAGS = -std=c++17 -Wall -Weffc++ -Wextra -Werror -pedantic-errors -Wconversion -Wsign-conversion -ggdb -O0
RELEASE_FLAGS = -std=c++17 Wall -Weffc++ -Wextra -Werror -pedantic-errors -Wconversion -Wsign-conversion -O2 -DNDEBUG
TARGET = main 

# Find all source files
SOURCES = $(shell find src -name '*.cpp') $(shell find libs/audio_io/src -name '*.cpp')

# Add src/ to include search path
INCLUDES = -Isrc -Ilibs/audio_io/include -Ilibs/audio_io/src 

LDFLAGS = -framework CoreAudio -framework AudioToolbox -framework ApplicationServices

debug: CXXFLAGS = $(DEBUG_FLAGS)
debug: $(TARGET)

release: CXXFLAGS = $(RELEASE_FLAGS)
release: $(TARGET)


$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: debug release clean
