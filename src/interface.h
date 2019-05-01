#ifndef YELA_INTERFACE_H_
#define YELA_INTERFACE_H_

#include <string>
#include <vector>
#include <iostream>


namespace yela {

class Interface {
 public:
  Interface();
  ~Interface();
 protected:
  bool terminate;
  // User Interaction
  const std::string kPrompt = 
    "\n\n\n\n\n\n\n\n\n\n\nEnter a message:\n";
  std::vector<std::string> history_;
  
  void ClearScreen();
  void PrintPrompt();
  std::string ReadInput();

  void Insert(const std::string &msg);
  void PrintHistory();
};

}

#endif
