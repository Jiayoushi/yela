#ifndef YELA_NODE_H_
#define YELA_NODE_H_

#include "node.h"

#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <fstream>
#include <iostream>

namespace yela {

Node::Node(const int my_port, const std::vector<int> &peers):
  Network(my_port, peers),
  origin_(std::to_string(my_port)),
  sequence_number_(1) {
}

Node::~Node() {
  WriteHistoryToFile();
  std::cout << "Node has exited" << std::endl;
}

// TODO: probably bad design
// TODO: needs callback
void Node::PollEvents() {                                                          
  int event_count = epoll_wait(epoll_fd_, events_, kMaxEventsNum, -1);
  if (event_count < 0) {
    perror("Error: epoll_wait failed");                                               
    exit(EXIT_FAILURE);                                                               
  }
  
  for (int i = 0; i < event_count; ++i) {
    if (events_[i].data.fd == listen_fd_) {
      HandleMessageFromPeer();
    } else if (events_[i].data.fd == STDIN_FILENO) {
      HandleLocalHostInput();
    } else {
      std::cerr << "Unmatched file descriptor" << std::endl;
    }
  }
}

void Node::HandleMessageFromPeer() {
  struct sockaddr_in peer_addr;
  socklen_t addrlen = sizeof(peer_addr);
  char buf[kMaxMessageSize + 1];
  int len = recvfrom(listen_fd_, buf, kMaxMessageSize, 0,
              (struct sockaddr *)&peer_addr, &addrlen);
  if (len < 0) {
    perror("recvfrom failed");
    return;
  }

  Message msg = ParseMessage(buf, len);
  int &last_msg_sequence_number = sequence_number_tables_[origin_][msg.origin];
  if (last_msg_sequence_number == 0) {
    last_msg_sequence_number = 1;
    SendMessageToRandomPeer(msg);
    // needs to wait for status message
  } else {
    if (last_msg_sequence_number >= msg.sequence_number) {
      // Discard
    } else if (last_msg_sequence_number + 1 < msg.sequence_number) {
      buffer_table_[msg.origin].push(msg);
    } else { // last_msg_sequence_number + 1 == msg.sequence_number
      Buffer buffer = buffer_table_[msg.origin];
      if (buffer.size() != 0) {
        buffer.push(msg);
        const Message &out_msg = buffer.top();
        SendMessageToRandomPeer(out_msg);
        buffer.pop();
      } else {
        SendMessageToRandomPeer(msg);
      }
      ++last_msg_sequence_number;
    }
  }
}

void Node::HandleLocalHostInput() {
  const std::string input = ReadInput();
  if (!terminate) {
    Message msg(sequence_number_, origin_, input);
    SendMessageToRandomPeer(msg);

    ++sequence_number_;
    Insert(msg);
  }
}

void Node::Run() {
  ClearScreen();
  while (true) { 
    PrintPrompt();
    PollEvents();

    ClearScreen();                                                                    
    if (terminate) {
      break;
    }

    PrintDialogue();
  }
}

void Node::WriteHistoryToFile() {
  const std::string kFileName = std::to_string(my_port_) + ".txt";
  std::ofstream of;
  of.open(kFileName);
  of << dialogue_;
  of.close();
}

}

#endif
