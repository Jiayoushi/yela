#include <string.h>
#include <fstream>
#include <string>

#include "node.h"

struct Arguments {
  int my_port;
  Arguments(): my_port(0) {}
};

Arguments GetCommandLine(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: ./yela <my_port>" << std::endl;
    std::cerr << " Expected: 2 arguments. Got: " << argc << std::endl;
    exit(EXIT_FAILURE);
  }

  Arguments arguments;
  arguments.my_port = std::stoi(argv[1]);

  return arguments;
}

int main(int argc, char *argv[]) {
  Arguments arguments = GetCommandLine(argc, argv);
  yela::Node node(arguments.my_port);
  node.Run();

  return 0;
}
