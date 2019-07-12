#include "interface.h"

#include <stdio.h>
#include <unistd.h>
#include <iostream>

namespace yela {

Interface::Interface():
  terminate(false) {
}

Interface::~Interface() {
}

// Input typed by user
std::string Interface::ReadInput() {
  char buffer[256];

  int bytes_read = 0;
  if ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer))) < 0) {
    perror("NODE: failed to read user input");
  }
  buffer[bytes_read] = '\0';

  std::string s(buffer);
  //std::cerr << "BYTES_READ: " << bytes_read << " msg:" << s << ";" << std::endl;
  if (bytes_read == 0 || s == "EXIT") terminate = true;

  return s;
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
