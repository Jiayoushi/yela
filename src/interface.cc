#include "interface.h"

#include <iostream>

namespace yela {

Interface::Interface():
  terminate(false) {
}

Interface::~Interface() {
}

std::string Interface::ReadInput() {
  std::string buffer;
  getline(std::cin, buffer);
  // User has entered EOF character, usually ctrl+d in linux
  if (std::cin.eof()) {
    terminate = true;
  }
  return buffer;
}

void Interface::ClearScreen() {
  std::cout << "\033[2J\033[1;1H";
}

void Interface::PrintPrompt() {
  std::cout << kPrompt;
}

void Interface::Insert(const std::string &ip, const std::string &port, const std::string &data) {
  history_.emplace_back(ip, port, data);
}

void Interface::PrintHistory() {
  for (const Message &msg: history_) {
    std::cout << "<" << msg.sender_ip << ","
              << msg.sender_port << ">"
              << ": " << msg.data
              << std::endl;
  }
}

}
