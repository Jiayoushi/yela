#include <iostream>

namespace yela {

class Node {
 public:
  Node();
  ~Node();

  void Run();
 private:




  // User Interaction
  const std::string kPrompt = 
    "\n\n\n\n\n\n\n\n\n\n\nEnter a message:\n";
  void ClearScreen();
  void PrintPrompt();
  bool terminate;
  std::string ReadInput();
};

}
