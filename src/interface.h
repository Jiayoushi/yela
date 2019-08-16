#ifndef YELA_INTERFACE_H_
#define YELA_INTERFACE_H_

#include <curses.h>
#include <mutex>
#include <thread>
#include <string>
#include <queue>
#include <vector>
#include <iostream>

#include "network.h"

namespace yela {

class Interface {
 public:
  Interface();
  ~Interface();

 protected:
  bool run_program_;

  enum KeyTable {
    kEnter = 10,
    kControlD = 4,
    kBackspace = 127
  };
  std::vector<std::string> inputs_;

  void InsertToDialogue(const Id &id, const ChatText &chat_text);
  void WriteDialogueToFile(const Id &id);

  const int kPrintFrequencyInMs = 500;
  int kMaxMsgToPrint;
  void PrintDialogue();
  void ReadInput();

  std::mutex local_msgs_mutex_;
  std::queue<std::string> local_msgs_; 
 private:
  WINDOW *dialogue_window_;
  WINDOW *textbox_window_;

  // Thread that manages local user input
  std::thread input_thread_;

  // Thread that manages printing dialogue
  std::thread print_thread_;

  std::mutex dialogue_mutex_;
  std::vector<std::string> dialogue_;
};

}

#endif
