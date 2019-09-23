#include "cmd.h"

#include <iostream>

namespace yela {

Arguments GetCommandLine(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: ./yela <settings file>" << std::endl;
    std::cerr << " Expected: 2 arguments. Got: " << argc << std::endl;
    exit(EXIT_FAILURE);
  }

  Arguments arguments;
  arguments.settings_file = argv[1];
  return arguments;
}

}
