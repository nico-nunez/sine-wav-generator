#pragma once

#include <string>

namespace synth_io {
struct SynthSession;
using hSynthSession = SynthSession *;
} // namespace synth_io
namespace synth {
struct Engine;
}

namespace synth::input_proc {

void parseCommand(const std::string &line, Engine &engine,
                  synth_io::hSynthSession session);

} // namespace synth::input_proc
