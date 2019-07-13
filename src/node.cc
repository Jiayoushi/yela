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
  id_(std::to_string(my_port)),
  sequence_number_(kInitialSequenceNumber) {
}

Node::~Node() {
  WriteHistoryToFile();
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

// Two tasks:
//  1. Check if the sender needs anything that can be sent from this node
//  2. Check if this node needs anything that sender has already seen
void Node::HandleStatusMessage(const Message &msg) {
  const SequenceNumberTable &msg_sqn_table = msg.table;

  bool send_status_message = false;
  for (auto p = msg_sqn_table.begin(); p != msg_sqn_table.end(); ++p) {
    Id id = p->first;
    int seq_num = p->second;

    if (seq_num_table_[id] > seq_num) {
      SendMessageToRandomPeer(Message(id, seq_num, 
                                      text_storage_.Get(id, seq_num)));
    }

    if (seq_num_table_[id] < seq_num) {
      send_status_message = true;
    }
  }

  if (send_status_message) {
    SendMessageToRandomPeer(Message(seq_num_table_));
  }
}

// Since it's UDP, we need to resend messages if datagrams are dropped
// The ack message is a status message 
void Node::AcknowledgeMessage(const Id &id) {
  Message status_message(seq_num_table_);

  // TODO: ip is omitted here, need to be part of the target address
  // Note ACK is sent to random neighbor.
  // There is no way knowing who sends this message since a process may use
  // one port to send udp datagram and discard it right away.
  SendMessageToRandomPeer(status_message);
}

void Node::HandleRumorMessage(const Message &msg, const sockaddr_in &peer_addr) {
  // If this message was original from this node, there is no need to do anything
  if (msg.id == id_) return;

  int &last_sequence_number = seq_num_table_[msg.id];

  // A message already received, discard
  if (last_sequence_number >= msg.sequence_number) {
    return;

  // TODO: Out of order message, way in the future, what to do with this message?
  } else if (last_sequence_number + 1 < msg.sequence_number) {

  // The expected message sequence number
  // last_sequence_number + 1 == msg.sequence_number
  } else {
    // Store this message
    text_storage_.Put(msg.id, msg.sequence_number, msg.chat_text);

    SendMessageToRandomPeer(msg);

    Insert(msg);

    // Update 
    ++last_sequence_number;
  }

  // Acknowledge
  AcknowledgeMessage(msg.id);
}

// Read user input and send to random peer
void Node::HandleLocalHostInput() {
  const std::string input = ReadInput();
  //std::cerr << my_port_ << " terminate: " << terminate << " " << input << std::endl;
  if (!terminate) {
    Message msg(id_, sequence_number_, input);
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
