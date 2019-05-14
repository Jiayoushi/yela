#include <string.h>
#include <fstream>
#include <string>

#include "node.h"

struct Arguments {
  int my_port;
  std::vector<int> neighbors;
};

void ReadNeighbors(char *ports_str, std::vector<int> &neighbor_ports) {
  char *port = strtok(ports_str, ",");
  while (port != nullptr) {
    int neighbor_port = std::atoi(port);
    neighbor_ports.push_back(neighbor_port);
    port = strtok(nullptr, ",");
  }
}

Arguments GetCommandLine(int argc, char *argv[]) {
  Arguments arguments;

  if (argc != 3) {
    std::cerr << "Usage: ./yela <my_port> <neighbor1_port,neighbor2_port...>" << std::endl;
    exit(EXIT_FAILURE);
  }

  arguments.my_port = std::stoi(argv[1]);
  ReadNeighbors(argv[2], arguments.neighbors);
  return arguments;
}

int main(int argc, char *argv[]) {
  Arguments arguments = GetCommandLine(argc, argv);
  yela::Node node(arguments.my_port, arguments.neighbors);
  node.Run();

  return 0;
}
