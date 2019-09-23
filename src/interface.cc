#include "interface.h"

#include <string>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <ctime>

#include "global.h"
#include "log.h"

namespace yela {

Interface::Interface():
  current_mode_(kChat),
  running(true) {

  // Initialize memory for display
  initscr();

  // Set cursor to invisible
  curs_set(0);  

  // Don't directly show user typed character
  noecho();

  int kMaxScreenY = 0, kMaxScreenX = 0;
  getmaxyx(stdscr, kMaxScreenY, kMaxScreenX);

  // Allocate dialogue window
  int dheight = 30, dwidth = kMaxScreenX / 2 - 5, dstart_y = 0, dstart_x = 0;
  dialogue_window_ = newwin(dheight, dwidth, dstart_y, dstart_x);
  // How many messages to fit in one screen?
  kMaxMsgToPrint = dheight - 2;

  // Allocate system message window
  int sheight = 30, swidth = dwidth, 
      sstart_y = 0, sstart_x = dwidth + 5;
  system_window_ = newwin(sheight, swidth, sstart_y, sstart_x);

  // Allocate textbox window
  int theight = 3, twidth = kMaxScreenX - 25, 
      tstart_y = kMaxScreenY - 12, tstart_x = 10;
  textbox_window_ = newwin(theight, twidth, tstart_y, tstart_x);
  
  keypad(textbox_window_, true);

  // Allocate mode window
  int mheight = 6, mwidth = 10,
      mstart_y = kMaxScreenY - 8, mstart_x = 5;
  mode_window_ = newwin(mheight, mwidth, mstart_y, mstart_x);

  // ??
  refresh();

  // Draw outlines around the windows
  box(dialogue_window_, 0, 0);
  box(system_window_, 0, 0);
  box(textbox_window_, 0, 0);
  box(mode_window_, 0, 0);

  // Show the window
  wrefresh(dialogue_window_);
  wrefresh(system_window_);
  wrefresh(textbox_window_);
  wrefresh(mode_window_);

  Log("Interface component initialized");
}

Interface::~Interface() {
  if (endwin() == ERR) {
    Log("ERROR: Failed to endwin");
  } else {
    Log("GUI successfully terminated");
  }
}

// Read local user input 
// This is another thread
void Interface::ReadInput() {
  std::string content;

  while (running) {
    // Update mode menu if there is any change
    current_mode_ = std::max(0, current_mode_);
    current_mode_ = std::min((int)kModeString.size() - 1, current_mode_);
    for (int i = 0; i < kModeString.size(); ++i) {
      if (current_mode_ == i) {
        wattron(mode_window_, A_REVERSE);
      }
      mvwprintw(mode_window_, i + 1, 1, kModeString[i].c_str());
      wattroff(mode_window_, A_REVERSE);
    }
    wrefresh(mode_window_);

    // Read and process user input
    int c = wgetch(textbox_window_);
    if (c == kUp) {
      --current_mode_;
    } else if (c == kDown) {
      ++current_mode_;
    } else if (c == kLeft || c == kRight) {
      // Do nothing
    } else if (c == kControlD) {
      running = false;
    } else if (c == kEnter) {
      if (content.size() == 0) {
        continue;
      }

      if (current_mode_ == kChat) {
        content += c;
      }
      local_inputs_mutex_.lock();
      local_inputs_.push(Input(current_mode_, content, 
                               (long)std::time(nullptr)));
      local_inputs_mutex_.unlock();

      // Clear and redraw textbox
      content.clear();
      wclear(textbox_window_);
      box(textbox_window_, 0, 0);
      wrefresh(textbox_window_);
    } else if (c == kBackspace) {
      if (content.size() == 0) {
        continue;
      }
      content.pop_back();

      // Clear and redraw textbox and text
      wclear(textbox_window_);
      box(textbox_window_, 0, 0);
      mvwprintw(textbox_window_, 1, 1, content.c_str());
      wrefresh(textbox_window_);
    } else {
      content += c;

      // Just print it
      mvwprintw(textbox_window_, 1, 1, content.c_str());
    }
  }
}

void Interface::PrintDialogue() {
  while (running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(kPrintFrequencyInMs));
   
    // Print the latest limited number of messages
    dialogue_mutex_.lock();

    int y_pos = 1;
    int x_pos = 1;
    std::priority_queue<Chat, std::vector<Chat>, CompareChat> dialogue = dialogue_;
    while (dialogue.size() != 0) {
      mvwprintw(dialogue_window_, y_pos, x_pos, dialogue.top().content.c_str());
      box(dialogue_window_, 0, 0);
      wrefresh(dialogue_window_);
      ++y_pos;
      dialogue.pop();
    }

    dialogue_mutex_.unlock();
  }
}

void Interface::InsertToDialogue(const Id &id, const Data &data, 
                                 const long timestamp) {
  std::string formatted_msg;
  formatted_msg.reserve(64);
  formatted_msg.append("<");
  formatted_msg.append(id);
  formatted_msg.append(">");
  formatted_msg.append(": ");
  formatted_msg.append(data);

  dialogue_mutex_.lock();
  dialogue_.push(Chat(formatted_msg, timestamp));
  dialogue_mutex_.unlock();
}

void Interface::WriteDialogueToFile(const Id &id) {
  const std::string kFileName = id + "_dialogue.txt";
  std::ofstream of;
  of.open(kFileName);
  while (dialogue_.size() != 0) {
    of << dialogue_.top().content;
    dialogue_.pop();
  }
  of.close();
}

void Interface::PrintToSystemWindow(const std::string &s) {
  system_messages_.push_back(s);

  int end = system_messages_.size();
  int start = std::max(0, end - kMaxMsgToPrint);
  int y_pos = 1;
  int x_pos = 1;
  for (int i = start; i < end; ++i) {
    mvwprintw(system_window_, y_pos, x_pos, system_messages_[i].c_str());
    box(system_window_, 0, 0);
    wrefresh(system_window_);
    ++y_pos;
  }
}

void Interface::Run() {
  input_thread_ = std::thread(&Interface::ReadInput, this);
  print_thread_ = std::thread(&Interface::PrintDialogue, this);

  input_thread_.join();
  print_thread_.join();
}

}
