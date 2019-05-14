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

Node::Node(int my_port, const std::vector<int> &peers):
  Network(my_port, peers) {
}

Node::~Node() {
  WriteHistoryToFile();
  std::cout << "Node has exited" << std::endl;
}

// TODO: probably bad design
void Node::PollEvents() {                                                          
  int event_count = epoll_wait(epoll_fd_, events_, kMaxEventsNum, -1);
  if (event_count < 0) {
    perror("Error: epoll_wait failed");                                               
    exit(EXIT_FAILURE);                                                               
  }
  
  for (int i = 0; i < event_count; ++i) {
    // Read message from peers
    if (events_[i].data.fd == listen_fd_) {
      struct sockaddr_in peer_addr;
      socklen_t addrlen = sizeof(peer_addr);
      char buf[kMaxMessageSize + 1];
      int len = recvfrom(listen_fd_, buf, kMaxMessageSize, 0,
                  (struct sockaddr *)&peer_addr, &addrlen);
      if (len < 0) {
        perror("recvfrom failed");
        continue;
      }
      Message msg = ParseMessage(buf, len);
      Insert(msg);
    // Read message from input
    } else if (events_[i].data.fd == STDIN_FILENO) {
      const std::string input = ReadInput();
      if (!terminate) {
        Message msg(my_ip_, std::to_string(my_port_), input);
        Insert(msg);
        BroadcastMessage(msg);
      }
    } else {
      std::cerr << "Unmatched file descriptor" << std::endl;
    }
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
