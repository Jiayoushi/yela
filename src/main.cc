#include <string.h>
#include <fstream>
#include <string>

#include "node.h"

struct Arguments {
  std::string settings_file;
};

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

int main(int argc, char *argv[]) {
  Arguments arguments = GetCommandLine(argc, argv);
  yela::Node node(arguments.settings_file);
  node.Run();

  return 0;
}
