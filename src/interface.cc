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
  if (std::cin.eof() || buffer == "EXIT") {
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

void Interface::Insert(const Message &msg) {
  InsertToDialogue(msg);
  InsertToHistory(msg);
}

void Interface::PrintDialogue() {
  std::cout << dialogue_ << std::endl;
}

void Interface::InsertToDialogue(const Message &msg) {
  std::string complete_msg;
  complete_msg.reserve(64);
  complete_msg.append("<");
  complete_msg.append(msg.origin);
  complete_msg.append(",");
  complete_msg.append(std::to_string(msg.sequence_number));
  complete_msg.append(">");
  complete_msg.append(": ");
  complete_msg.append(msg.chat_text);
  complete_msg.append("\n");

  dialogue_.append(complete_msg);
}

void Interface::InsertToHistory(const Message &msg) {
  history_.push_back(msg);
}

}
