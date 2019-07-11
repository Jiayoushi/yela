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
  if (msg.message_type == kRumorMessage) {
    HandleRumorMessage(msg, peer_addr);
  } else {
    HandleStatusMessage(msg);
  }
}

// If the sending peer receives a status message acknowledging the transmission, it compares the vector in the status message with its own status to see if it has any other new messages the remote peer has not yet seen and if so repeats the rumormongering process by sending one of those messages
void Node::HandleStatusMessage(const Message &msg) {
  std::cout << "Status Message Received!" << std::endl;
}

// Since it's UDP, we need to resend messages if datagrams are dropped
// The ack message is a status message 
void Node::AcknowledgeMessage(const sockaddr_in &peer_addr, const Origin &origin) {
  Message status_message(sequence_number_tables_[origin]);

  in_port_t port = peer_addr.sin_port;
  uint32_t ip = peer_addr.sin_addr.s_addr;

  // TODO: ip is omitted here, need to be part of the target address
  SendMessage(port, status_message);
}

void Node::HandleRumorMessage(const Message &msg, const sockaddr_in &peer_addr) {
  int &prev_sequence_number = sequence_number_tables_[origin_][msg.origin];

  // A message already received, discard
  if (prev_sequence_number >= msg.sequence_number) {
    return;

  // Message does not have the expected next sequence number. Store it for future
  } else if (prev_sequence_number + 1 < msg.sequence_number) {
    buffer_table_[msg.origin].push(msg);

  // The expected message sequence number
  // prev_sequence_number + 1 == msg.sequence_number
  } else { 
    Buffer buffer = buffer_table_[msg.origin];
    // If buffer is not empty, send the top of the buffer first
    if (buffer.size() != 0) {
      buffer.push(msg);
      const Message &out_msg = buffer.top();
      SendMessageToRandomPeer(out_msg);
      buffer.pop();
    // Nothing in the buffer, send the incoming message directly
    } else {
      SendMessageToRandomPeer(msg);
    }
    Insert(msg);

    // Update 
    ++prev_sequence_number;
  }

  // Acknowledge
  AcknowledgeMessage(peer_addr, msg.origin);
}

// Read user input and send to random peer
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
