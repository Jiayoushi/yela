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
#include <iostream>

namespace yela {

Node::Node(int my_port, const std::vector<int> &peers):
  Network(my_port, peers) {
}

Node::~Node() {
  shutdown(recv_fd_, SHUT_RDWR);
  close(recv_fd_);
  std::cout << "Node has exited" << std::endl;
}

void Node::Run() {
  ClearScreen();
  while (true) { 
    PrintPrompt();
    //std::string input = ReadInput();
    PollEvents();


    ClearScreen();                                                                    
    if (terminate) {                                                                  
      break;                                                                          
    }                                                                                 
    Insert(input);

    // Send message
    BroadcastMessage(input);

    PrintHistory();                                                                   
  }
}

}

#endif
