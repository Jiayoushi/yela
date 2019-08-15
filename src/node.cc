#ifndef YELA_NODE_H_
#define YELA_NODE_H_

#include "node.h"

#include <cassert>
#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <chrono>
#include <fstream>
#include <iostream>

namespace yela {

Node::Node(const std::string &settings_file):
  Network(settings_file),
  send_table_thread(&Node::SendTableToRandomPeer, this) {

  InitLog(me_.id);

  // Every node keeps its own sequence number.
  seq_num_table_[me_.id] = kInitialSequenceNumber;
}

Node::~Node() {
  WriteDialogueToFile();
  CloseLog();
  send_table_thread.join();
  std::cerr << me_.id << " successfully terminated." << std::endl;
}

void Node::PollEvents() {
  int event_count = epoll_wait(epoll_fd_, events_, kMaxEventsNum, -1);
  if (event_count < 0) {
    perror("Error: epoll_wait failed");                                               
    exit(EXIT_FAILURE);                                                               
  //} else if (event_count == 0) {
  //  Message status_message(seq_num_table_);
  //  SendMessageToRandomPeer(status_message);
  } else {
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
}

void Node::SendTableToRandomPeer() {
  Message status_message(seq_num_table_);
  SendMessageToRandomPeer(status_message);
  std::this_thread::sleep_for(std::chrono::milliseconds(kPeriodInMs));
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

  // Handle message 
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

    // If this node has never seen this id before, it needs to record this
    // new id, and set the default seq number.
    if (seq_num_table_.find(id) == seq_num_table_.end()) {
      seq_num_table_[id] = kInitialSequenceNumber;
    }

    if (seq_num_table_[id] > seq_num) {
      SendMessageToRandomPeer(Message(id, seq_num, 
                                      text_storage_.Get(id, seq_num)));
    } else if (seq_num_table_[id] < seq_num) {
      Log("wants to have seq_num: " + std::to_string(seq_num_table_[id]) + 
          " from " + id);
      send_status_message = true;
    }
  }

  // This node has found message it has not received, send status message
  // letting others know this node want unrecieved messages.
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
  // Check if it is a new node first
  // If it is a new peer, parse its chat text in the form of IP:PORT, or HOSTNAME:PORT
  if (!IsKnownPeer(msg.id)) {
    InsertPeer(msg.id, peer_addr);
  }

  // If this message was original from this node, there is no need to do anything
  if (msg.id == me_.id) return;

  // If a new id comes, the default sequence number for that id should be set
  // instead of 0.
  if (seq_num_table_.find(msg.id) == seq_num_table_.end()) {
    seq_num_table_[msg.id] = kInitialSequenceNumber;
  }

  int &last_sequence_number = seq_num_table_[msg.id];

  // A message already received, discard
  if (msg.sequence_number < last_sequence_number) {
    return;

  // The expected message sequence number
  } else if (msg.sequence_number == last_sequence_number) {
    // Store this message
    text_storage_.Put(msg.id, msg.sequence_number, msg.chat_text);

    // Log for debug
    Log("Received Rumor message from " + msg.id + " seq_number: " + 
        std::to_string(msg.sequence_number) + " message_content: " + msg.chat_text);

    // Randomly send
    SendMessageToRandomPeer(msg);

    // Insert into history to be printed
    Insert(msg);

    // Update 
    ++last_sequence_number;

  // Waiting for message with seq#1, instead seq#2 is received. Discard it.
  } else {
    // Discard
  }

  // Acknowledge
  AcknowledgeMessage(msg.id);
}

// Read user input and send to random peer
void Node::HandleLocalHostInput() {
  const std::string input = ReadInput();
  if (!terminate) {
    Message msg(me_.id, seq_num_table_[me_.id], input);
    SendMessageToRandomPeer(msg);
    text_storage_.Put(me_.id, seq_num_table_[me_.id], msg.chat_text);

    ++seq_num_table_[me_.id];
    Insert(msg);
  }
}

void Node::Run() {
  ClearScreen();
  while (!terminate) { 
    PrintPrompt();

    PollEvents();

    PrintDialogue();
  }
}

void Node::WriteDialogueToFile() {
  const std::string kFileName = std::to_string(me_.port) + ".txt";
  std::ofstream of;
  of.open(kFileName);
  of << dialogue_;
  of.close();
}

}

#endif
