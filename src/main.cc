#include "node.h"

#include <fstream>
#include <string>

struct Arguments {
  int my_port;
  std::vector<int> peers;
};


void ReadPeers(const std::string &filename, int my_port, std::vector<int> &peer_ports) {
  std::ifstream infile(filename);
  for (std::string port; infile >> port; ) {
    int peer_port = std::stoi(port);
    if (peer_port != my_port) {
      peer_ports.push_back(peer_port);
    }
  }
}

Arguments GetCommandLine(int argc, char *argv[]) {
  Arguments arguments;

  if (argc != 3) {
    std::cerr << "Usage: ./yela <my_port> <peers.txt>" << std::endl;
    exit(EXIT_FAILURE);
  }

  arguments.my_port = std::stoi(argv[1]);
  ReadPeers(std::string(argv[2]), arguments.my_port, arguments.peers);
  return arguments;
}

int main(int argc, char *argv[]) {
  Arguments arguments = GetCommandLine(argc, argv);
  yela::Node node(arguments.my_port, arguments.peers);
  node.Run();

  return 0;
}
