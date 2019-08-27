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
#include <ctime>
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
    if (local_inputs_.size() > 0) {
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
  
  char buf[Message::kMaxMessageSize + 1];
  int len = recvfrom(listen_fd_, buf, Message::kMaxMessageSize, 0,
              (struct sockaddr *)&peer_addr, &addrlen);
  if (len < 0) {
    perror("recvfrom failed");
    return;
  }

  Message msg = ParseMessage(buf, len);

  // Check if it is a new node first
  // If it is a new peer, parse its chat text in the form of IP:PORT, or HOSTNAME:PORT
  if (!IsKnownPeer(msg["id"])) {
    InsertPeer(msg["id"], peer_addr);
  }

  // Handle message 
  if (msg["type"] == kTypes[kRumor]) {
    bool new_seq_num = HandleRumorMessage(msg);
    
    // Update destination-sequenced distance vector
    if (new_seq_num) {
      UpdateDistanceVector(msg["id"], peer_addr);
    }
  } else {
    HandleStatusMessage(msg);
  }
}

// Two tasks:
//  1. Check if the sender needs anything that can be sent from this node
//  2. Check if this node needs anything that sender has already seen
void Node::HandleStatusMessage(const Message &msg) {
  Log("Received status message from " + msg["id"]);
  const SequenceNumberTable msg_sqn_table = Message::Deserialize(msg["seqtable"]);

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
      const Chat chat = text_storage_.Get(id, seq_num);
      SendMessageToRandomPeer(Message(id, seq_num, chat.content, chat.timestamp));
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

bool Node::HandleRumorMessage(const Message &msg) {
  bool new_seq_num = false;

  if (msg["id"] == me_.id) {
    return new_seq_num;
  }

  // If a new id comes, the default sequence number for that id should be set
  if (seq_num_table_.find(msg["id"]) == seq_num_table_.end()) {
    seq_num_table_[msg["id"]] = kInitialSequenceNumber;
  }

  int &last_sequence_number = seq_num_table_[msg["id"]];
  int msg_seq_num = std::stoi(msg["seqnum"]);
  // A message already received, discard
  if (msg_seq_num < last_sequence_number) {
    return new_seq_num;

  // The expected message sequence number
  } else if (msg_seq_num == last_sequence_number) {
    ProcessRumorMessage(msg);
    new_seq_num = true;
  // Waiting for message with seq#1, instead seq#2 is received. Discard it.
  } else {
    // Discard
  }

  // Acknowledge
  AcknowledgeMessage(msg["id"]);
  return new_seq_num;
}

void Node::ProcessRumorMessage(const Message &msg) {
  // Log for debug
  Log("Received Rumor message from " + msg["id"] + " seq_number: " + 
      msg["seqnum"] + " \"" + msg["data"] + "\" timestamp:" + msg["timestamp"]);

  // Store this message
  text_storage_.Put(msg["id"], std::stoi(msg["seqnum"]), msg["data"], 
                    std::stol(msg["timestamp"]));

  // Send to random neighbor
  SendMessageToRandomPeer(msg);

  // Insert into dialogue to be printed
  Chat chat(msg["data"], std::stol(msg["timestamp"]));
  InsertToDialogue(msg["id"], msg["data"], std::stol(msg["timestamp"]));

  // Update sequence number table
  ++seq_num_table_[msg["id"]];
}

// Read user input and send to random peer
void Node::HandleLocalHostInput() {
  local_inputs_mutex_.lock();

  while (local_inputs_.size() != 0) {
    const Input &input = local_inputs_.front();

    if (input.sentence == kExitMsgForTesting) {
      run_program_ = false;
      return;
    }

    if (input.mode == kChat) {
      Message msg(me_.id, seq_num_table_[me_.id], input.sentence, input.timestamp);
      ProcessRumorMessage(msg);
    } else if (input.mode == kUpload) {
      int status = file_manager_.Upload(input.sentence);
      if (status == 0) {
        PrintToSystemWindow("File '" + input.sentence + "' has been successfully uploaded.");
      } else if (status == -1) {
        PrintToSystemWindow("File '" + input.sentence + "' is not found.");
      }
    } else if (input.mode == kDownload) {

    } else if (input.mode == kSearch) {

    } else {
      Log("WARNING: current input mode " + std::to_string(input.mode) + 
      " is not matched to any of the mode");
    }

    local_inputs_.pop();
  }

  local_inputs_mutex_.unlock();
}

void Node::Run() {
  while (run_program_) {
    PollEvents();
  }
}

}

#endif
