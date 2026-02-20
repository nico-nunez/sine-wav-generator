#include "InputProcessor.h"

#include "synth/ParamBindings.h"

#include "synth_io/SynthIO.h"

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

namespace synth::utils {
namespace s_io = synth_io;

namespace pb = param_bindings;

// ==== Internal Helpers ====
namespace {

// Parse input string and update param value
int setInputParam(std::istringstream &iss, s_io::hSynthSession session) {
  std::string paramName;
  float paramValue;

  iss >> paramName;

  // Check if param is setting waveformType (string)
  if (pb::isWaveFormParam(paramName.c_str())) {
    std::string value;
    iss >> value;

    auto waveformType = pb::getWaveformType(value.c_str());
    paramValue = static_cast<float>(waveformType);

  } else {
    // All other params values are floats (denormalized)
    iss >> paramValue;
  }

  pb::ParamID id = pb::findParamByName(paramName.c_str());
  if (id == param_bindings::PARAM_COUNT) {
    printf("Error: Unknown parameter '%s'\n", paramName.c_str());
    return 1;
  }

  /*
   * NOTE(nico): User is entering denormalized value and param is stored
   * denormalized.  May consider normalizing in the future, but seems
   * pointless at this time.
   *
   * Except for WaveformType.  need to handle that case
   */
  if (!s_io::setParam(session, static_cast<uint8_t>(id), paramValue)) {
    printf("Warning: Param queue full, event dropped\n");
    return 2;
  }

  return 0;
}

// List params
void printParamList(const char *optionalParam) {
  if (optionalParam) {

    printf("Available parameters for: %s\n", optionalParam);
    for (const auto &mapping : param_bindings::PARAM_NAMES) {
      if (strstr(mapping.name, optionalParam) != NULL)
        printf("  %s\n", mapping.name);
    }

  } else {

    printf("Available parameters:\n");
    for (const auto &mapping : param_bindings::PARAM_NAMES) {
      printf("  %s\n", mapping.name);
    }
  }
}

} // namespace

void parseCommand(const std::string &line, Engine &engine,
                  s_io::hSynthSession session) {
  std::istringstream iss(line);
  std::string cmd;
  iss >> cmd;

  int errStatus = 0;

  // SET: set param value (adds ParamEvent to the queue)
  if (cmd == "set") {

    errStatus = setInputParam(iss, session);
    if (!errStatus)
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

    std::string optionalParam;
    iss >> optionalParam;

    printParamList(optionalParam.c_str());

    // HELP: print available commands
  } else if (cmd == "help") {
    printf("Commands:\n");
    printf("  set <param> <value>  - Set parameter value\n");
    printf("  get <param>          - Query parameter value\n");
    printf("  list                 - List all parameters\n");
    printf("  help                 - Show this help\n");
    printf("  quit                 - Exit\n");
    printf("\nNote commands: a-k (play notes)\n");

  } else if (cmd == "clear") {
    // Clear console
    system("clear");

    // Invalid command
  } else if (cmd != "quit") {
    std::cout << "Invalid command: " << cmd << std::endl;
    printf("Enter 'help' for list of valid commands.\n");
  }
}

} // namespace synth::utils
