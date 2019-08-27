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
    kBackspace = KEY_BACKSPACE,
    kUp = KEY_UP,
    kDown = KEY_DOWN,
    kLeft = KEY_LEFT,
    kRight = KEY_RIGHT
  };
  std::vector<std::string> inputs_;

  void InsertToDialogue(const Id &id, const std::string &data);
  void WriteDialogueToFile(const Id &id);

  const int kPrintFrequencyInMs = 500;
  int kMaxMsgToPrint;
  void PrintDialogue();
  void PrintToSystemWindow(const std::string &s);
  void ReadInput();

  // Menu for options for either chat, upload file, download file or search file
  enum Mode {
    kChat = 0, 
    kUpload = 1, 
    kDownload = 2, 
    kSearch = 3
  };
  int current_mode_;

  const std::vector<std::string> kModeString = {
    "Chat", "Upload", "Download", "Search"
  };

  std::mutex local_inputs_mutex_;
  std::queue<std::pair<int, std::string>> local_inputs_;
 private:
  WINDOW *dialogue_window_;
  WINDOW *textbox_window_;
  WINDOW *mode_window_;
  WINDOW *system_window_;

  // Thread that manages local user input
  std::thread input_thread_;

  // Thread that manages printing dialogue
  std::thread print_thread_;

  std::mutex dialogue_mutex_;
  std::vector<std::string> dialogue_;

  std::vector<std::string> system_messages_;
};

}

#endif
