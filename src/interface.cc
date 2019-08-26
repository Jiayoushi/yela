#include "interface.h"

#include <string>
#include <stdio.h>
#include <unistd.h>
#include <iostream>

#include "log.h"

namespace yela {

Interface::Interface():
  run_program_(true), current_mode_(kChat) {

  // Initialize memory for display
  initscr();

  // Set cursor to invisible
  curs_set(0);  

  // Don't directly show user typed character
  noecho();

  int kMaxScreenY = 0, kMaxScreenX = 0;
  getmaxyx(stdscr, kMaxScreenY, kMaxScreenX);

  // Allocate dialogue window
  int height = 30, width = kMaxScreenX - 12, start_y = 0, start_x = 0;
  dialogue_window_ = newwin(height, width, start_y, start_x);

  // How many messages to fit in one screen?
  kMaxMsgToPrint = height - 2;

  // Allocate textbox window
  int height2 = 3, width2 = kMaxScreenX - 25, 
      start_y2 = kMaxScreenY - 12, start_x2 = 5;
  textbox_window_ = newwin(height2, width2, start_y2, start_x2);
  
  keypad(textbox_window_, true);

  // Allocate mode window
  int height3 = 6, width3 = 10,
      start_y3 = kMaxScreenY - 8, start_x3 = 5;
  mode_window_ = newwin(height3, width3, start_y3, start_x3);

  // ??
  refresh();

  // Draw outlines around the windows
  box(dialogue_window_, 0, 0);
  box(textbox_window_, 0, 0);
  box(mode_window_, 0, 0);


  // Show the window
  wrefresh(dialogue_window_);
  wrefresh(textbox_window_);
  wrefresh(mode_window_);


  input_thread_ = std::thread(&Interface::ReadInput, this);
  print_thread_ = std::thread(&Interface::PrintDialogue, this);
}

Interface::~Interface() {
  input_thread_.join();
  print_thread_.join();
  endwin();
}

// Read local user input 
// This is another thread
void Interface::ReadInput() {
  std::string sentence;
  while (run_program_) {
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
      run_program_ = false;
    } else if (c == kEnter) {
      if (sentence.size() == 0) {
        continue;
      }

      if (current_mode_ == kChat) {
        sentence += c;
      }
      local_inputs_mutex_.lock();
      local_inputs_.push(std::make_pair(current_mode_, sentence));
      local_inputs_mutex_.unlock();

      // Clear and redraw textbox
      sentence.clear();
      wclear(textbox_window_);
      box(textbox_window_, 0, 0);
      wrefresh(textbox_window_);
    } else if (c == kBackspace) {
      if (sentence.size() == 0) {
        continue;
      }
      sentence.pop_back();

      // Clear and redraw textbox and text
      wclear(textbox_window_);
      box(textbox_window_, 0, 0);
      mvwprintw(textbox_window_, 1, 1, sentence.c_str());
      wrefresh(textbox_window_);
    } else {
      sentence += c;

      // Just print it
      mvwprintw(textbox_window_, 1, 1, sentence.c_str());
    }
  }
}

void Interface::PrintDialogue() {
  while (run_program_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(kPrintFrequencyInMs));
   
    // Print the latest limited number of messages
    dialogue_mutex_.lock();
    int end = dialogue_.size();
    // 30 - 30 == 0
    int start = std::max(0, end - kMaxMsgToPrint);
    int y_pos = 1;
    int x_pos = 1;
    for (int i = start; i < end; ++i) {
      mvwprintw(dialogue_window_, y_pos, x_pos, dialogue_[i].c_str());
      box(dialogue_window_, 0, 0);
      wrefresh(dialogue_window_);
      ++y_pos;
    }

    dialogue_mutex_.unlock();
  }
}

void Interface::InsertToDialogue(const Id &id, const Data &data) {
  std::string formatted_msg;
  formatted_msg.reserve(64);
  formatted_msg.append("<");
  formatted_msg.append(id);
  formatted_msg.append(">");
  formatted_msg.append(": ");
  formatted_msg.append(data);

  dialogue_mutex_.lock();
  dialogue_.push_back(formatted_msg);
  dialogue_mutex_.unlock();
}

void Interface::WriteDialogueToFile(const Id &id) {
  const std::string kFileName = id + "_dialogue.txt";
  std::ofstream of;
  of.open(kFileName);
  for (const std::string &s: dialogue_) {
    of << s;
  }
  of.close();
}

}
