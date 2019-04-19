#include "node.h"

#include <iostream>

namespace yela {

Node::Node():
  terminate(false) {

}

Node::~Node() {
  std::cout << "Node has exited" << std::endl;
}

void Node::Run() {
  while (true) {
    PrintPrompt();
    std::string input = ReadInput();
    
    ClearScreen();
    if (terminate) {
      break;
    }
    std::cout << "User has entered: " << input << std::endl; 
  }
}

std::string Node::ReadInput() {
  std::string buffer;
  getline(std::cin, buffer);
  // User has entered EOF character, usually ctrl+d in linux
  if (std::cin.eof()) {
    terminate = true;
  }
  return buffer;
}

void Node::ClearScreen() {
  std::cout << "\033[2J\033[1;1H";
}

void Node::PrintPrompt() {
  std::cout << kPrompt;
}




}
