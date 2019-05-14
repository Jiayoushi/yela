#include <iostream>

#include "network.h"
#include "interface.h"

namespace yela {

class Node: public Interface, public Network {
 public:
  Node(int my_port, const std::vector<int> &peers);
  ~Node();

  void PollEvents();
  void Run();
 private:
  void WriteHistoryToFile();
};

}
