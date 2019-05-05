#ifndef YELA_INTERFACE_H_
#define YELA_INTERFACE_H_

#include <string>
#include <vector>
#include <iostream>

#include "network.h"

namespace yela {

class Interface {
 public:
  Interface();
  ~Interface();

 protected:
  bool terminate;
  // User Interaction
  const std::string kPrompt = "\n\n\n\n\n\n\n\n\n\n\nEnter a message:\n";
  
  void ClearScreen();
  void PrintPrompt();
  std::string ReadInput();

  std::vector<Message> history_;
  void Insert(const Message &msg);
  void PrintHistory();
};

}

#endif
