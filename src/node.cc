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
  send_table_thread.join();
  Log(me_.id + " successfully terminated.");
  CloseLog();
  WriteDialogueToFile(me_.id);
}

void Node::PollEvents() {
  int event_count = epoll_wait(epoll_fd_, events_, 
                               kMaxEventsNum, kEpollFrequencyInMs);
  if (event_count < 0) {
    perror("Error: epoll_wait failed");                                               
    exit(EXIT_FAILURE);
  } else if (event_count == 0) {
    // Check if there is any local user input
    if (local_msgs_.size() > 0) {
      HandleLocalHostInput();
    }
  } else {
    for (int i = 0; i < event_count; ++i) {
      if (events_[i].data.fd == listen_fd_) {
        HandleMessageFromPeer();
      } else {
        std::cerr << "Unmatched file descriptor" << std::endl;
      }
    }
  }
}

void Node::SendTableToRandomPeer() {
  while (run_program_) {
    Message status_message(me_.id, seq_num_table_);
    SendMessageToRandomPeer(status_message);
    std::this_thread::sleep_for(std::chrono::milliseconds(kPeriodInMs));
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

  // Check if it is a new node first
  // If it is a new peer, parse its chat text in the form of IP:PORT, or HOSTNAME:PORT
  if (!IsKnownPeer(msg.id)) {
    InsertPeer(msg.id, peer_addr);
  }

  // Handle message 
  if (msg.message_type == kRumorMessage) {
    HandleRumorMessage(msg);
  } else {
    HandleStatusMessage(msg);
  }

  // Update destination-sequenced distance vector
  //UpdateDistanceVector(msg.id, peer_addr); 
}

// Two tasks:
//  1. Check if the sender needs anything that can be sent from this node
//  2. Check if this node needs anything that sender has already seen
void Node::HandleStatusMessage(const Message &msg) {
  Log("Received status message from " + msg.id);
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
    SendMessageToRandomPeer(Message(me_.id, seq_num_table_));
  }
}

// Since it's UDP, we need to resend messages if datagrams are dropped
// The ack message is a status message 
void Node::AcknowledgeMessage(const Id &id) {
  Message status_message(me_.id, seq_num_table_);

  // TODO: Need to send to the sender
  SendMessageToRandomPeer(status_message);
}

void Node::HandleRumorMessage(const Message &msg) {
  if (msg.id == me_.id) {
    return;
  }

  // If a new id comes, the default sequence number for that id should be set
  if (seq_num_table_.find(msg.id) == seq_num_table_.end()) {
    seq_num_table_[msg.id] = kInitialSequenceNumber;
  }

  int &last_sequence_number = seq_num_table_[msg.id];

  // A message already received, discard
  if (msg.sequence_number < last_sequence_number) {
    return;

  // The expected message sequence number
  } else if (msg.sequence_number == last_sequence_number) {
    ProcessRumorMessage(msg);
  // Waiting for message with seq#1, instead seq#2 is received. Discard it.
  } else {
    // Discard
  }

  // Acknowledge
  AcknowledgeMessage(msg.id);
}

void Node::ProcessRumorMessage(const Message &msg) {
  // Log for debug
  Log("Received Rumor message from " + msg.id + " seq_number: " + 
      std::to_string(msg.sequence_number) + " \"" + msg.chat_text + "\"");

  // Store this message
  text_storage_.Put(msg.id, msg.sequence_number, msg.chat_text);

  // Send to random neighbor
  SendMessageToRandomPeer(msg);

  // Insert into dialogue to be printed
  InsertToDialogue(msg.id, msg.chat_text);

  // Update sequence number table
  ++seq_num_table_[msg.id];
}

// Read user input and send to random peer
void Node::HandleLocalHostInput() {
  local_msgs_mutex_.lock();

  while (local_msgs_.size() != 0) {
    if (local_msgs_.front() == kExitMsgForTesting) {
      run_program_ = false;
      return;
    }

    Message msg(me_.id, seq_num_table_[me_.id], local_msgs_.front());
    local_msgs_.pop();

    ProcessRumorMessage(msg);
  }

  local_msgs_mutex_.unlock();
}

void Node::Run() {
  while (run_program_) {
    PollEvents();
  }
}

}

#endif
