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
#include <iostream>

namespace yela {

Node::Node(int my_port, const std::vector<int> &peers):
  Network(my_port, peers) {
}

Node::~Node() {
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
      buf[len] = '\0';
      std::string data(buf);
      std::cout << peer_addr.sin_port << std::endl;
      Insert("", "", buf);
    } else if (events_[i].data.fd == STDIN_FILENO) {
      std::string input = ReadInput();
      Insert("", "", "You: " + input);
      BroadcastMessage(input);
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

    PrintHistory();                                                                   
  }
}

}

#endif
