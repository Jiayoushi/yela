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

class Input {
 public:
  int mode;
  std::string content;
  long timestamp;

  Input(int m, const std::string &s, const time_t &t):
    mode(m), content(s), timestamp(t) {

  }

  ~Input() {}
};

// Menu for options for either chat, upload file, download file or search file
enum Mode {
  kChat = 0, 
  kUpload = 1, 
  kDownload = 2, 
  kSearch = 3
};

struct Chat {
  std::string content; // <ID>: user's sentence
  long timestamp;

  Chat() {
 
  }

  Chat(const std::string &c, const long t):
    content(c), timestamp(t) {

  }
};

class Interface {
 public:
  Interface();
  ~Interface();

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

  void InsertToDialogue(const Origin &origin, const std::string &data, 
                        const long timestamp);
  void WriteDialogueToFile(const Origin &origin);

  const int kPrintFrequencyInMs = 500;
  int kMaxMsgToPrint;
  void PrintDialogue();
  void PrintToSystemWindow(const std::string &s);
  void ReadInput();

  bool running;
  int current_mode_;

  void Run();

  const std::vector<std::string> kModeString = {
    "Chat", "Upload", "Download", "Search"
  };

  std::mutex local_inputs_mutex_;
  std::queue<Input> local_inputs_;

 private:
  WINDOW *dialogue_window_;
  WINDOW *textbox_window_;
  WINDOW *mode_window_;
  WINDOW *system_window_;

  // Thread that manages local user input
  std::thread input_thread_;

  // Thread that manages printing dialogue
  std::thread print_thread_;

  struct CompareChat {
    bool operator()(const Chat &chat1, const Chat &chat2) {
      return chat1.timestamp > chat2.timestamp;
    }
  };
  std::priority_queue<Chat, std::vector<Chat>, CompareChat> dialogue_;
  std::mutex dialogue_mutex_;

  std::vector<std::string> system_messages_;
};

}

#endif
