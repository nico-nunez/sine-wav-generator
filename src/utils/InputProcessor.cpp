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

namespace pb = param::bindings;

// ==== Internal Helpers ====
namespace {

// Parse input string and update param value
int setInputParam(std::istringstream &iss, s_io::hSynthSession session) {
  std::string paramName;
  float paramValue;

  iss >> paramName;

  pb::ParamMapping param = pb::findParamByName(paramName.c_str());
  if (param.id == param::bindings::PARAM_COUNT) {
    printf("Error: Unknown parameter '%s'\n", paramName.c_str());
    return 1;
  }

  // Check if param is setting waveformType (string)
  switch (param.type) {
  // Set Oscillator Waveform
  case pb::ParamValueType::WAVEFORM: {
    std::string value;
    iss >> value;

    auto waveformType = pb::getWaveformType(value.c_str());
    paramValue = static_cast<float>(waveformType);

  } break;

  // Enable/Disable Item
  case pb::ParamValueType::BOOL: {
    std::string value;
    iss >> value;

    paramValue = strcasecmp(value.c_str(), "true") == 0 ? 1.0f : 0.0f;

  } break;

  // Set SVF Mode
  case pb::ParamValueType::FILTER_MODE: {
    std::string value;
    iss >> value;

    auto filterMode = pb::getSVFModeType(value.c_str());
    paramValue = static_cast<float>(filterMode);

  } break;

  // Treat all other params values as floats (denormalized)
  default:
    iss >> paramValue;
  }

  /*
   * NOTE(nico): User is entering denormalized value and param is stored
   * denormalized.  May consider normalizing in the future, but seems
   * pointless at this time.
   */
  if (!s_io::setParam(session, static_cast<uint8_t>(param.id), paramValue)) {
    printf("Warning: Param queue full, event dropped\n");
    return 2;
  }

  return 0;
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

    pb::ParamMapping param = pb::findParamByName(paramName.c_str());
    if (param.id == param::bindings::PARAM_COUNT) {
      printf("Error: Unknown parameter '%s'\n", paramName.c_str());
      return;
    }

    float rawValue = pb::getParamValueByID(engine, param.id);

    printf("%s = %.2f\n", paramName.c_str(), rawValue);

    // LIST: print available param names
  } else if (cmd == "list") {

    std::string optionalParam;
    iss >> optionalParam;

    pb::printParamList(optionalParam.empty() ? nullptr : optionalParam.c_str());

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
