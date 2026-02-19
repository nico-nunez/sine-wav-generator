#include "synth/ParamBindings.h"
#include "synth_io/SynthIO.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>

namespace synth::input_proc {
namespace pb = param_bindings;

void parseCommand(const std::string &line, Engine &engine,
                  synth_io::hSynthSession session) {
  std::istringstream iss(line);
  std::string cmd;
  iss >> cmd;

  // SET: set param value (adds ParamEvent to the queue)
  if (cmd == "set") {
    std::string paramName;
    float rawValue;
    iss >> paramName >> rawValue;

    pb::ParamID id = pb::findParamByName(paramName.c_str());
    if (id == param_bindings::PARAM_COUNT) {
      printf("Error: Unknown parameter '%s'\n", paramName.c_str());
      return;
    }

    // Convert raw value to normalized [0, 1]
    // float normalized = pb::normalizeParamValue(engine, id, rawValue);

    if (!synth_io::setParam(session, static_cast<uint8_t>(id), rawValue)) {
      printf("Warning: Param queue full, event dropped\n");
    }

    printf("OK\n");

    // GET: print param value
  } else if (cmd == "get") {
    std::string paramName;
    iss >> paramName;

    pb::ParamID id = pb::findParamByName(paramName.c_str());
    if (id == param_bindings::PARAM_COUNT) {
      printf("Error: Unknown parameter '%s'\n", paramName.c_str());
      return;
    }

    float rawValue = pb::getParamValueByID(engine, id);

    printf("%s = %.2f\n", paramName.c_str(), rawValue);

    // LIST: print available param names
  } else if (cmd == "list") {
    printf("Available parameters:\n");
    for (const auto &mapping : param_bindings::PARAM_NAMES) {
      printf("  %s\n", mapping.name);
    }

    // HELP: print available commands
  } else if (cmd == "help") {
    printf("Commands:\n");
    printf("  set <param> <value>  - Set parameter value\n");
    printf("  get <param>          - Query parameter value\n");
    printf("  list                 - List all parameters\n");
    printf("  help                 - Show this help\n");
    printf("  quit                 - Exit\n");
    printf("\nNote commands: a-k (play notes)\n");
  } else if (cmd != "quit") {
    std::cout << "Invalid command: " << cmd << std::endl;
    printf("Enter 'help' for list of valid commands.\n");
  }
}

} // namespace synth::input_proc
