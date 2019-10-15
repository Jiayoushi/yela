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

#include "global.h"

namespace yela {

Node::Node():
  running(false) {
  Log("Node component initialized.");
}

Node::~Node() {
  Log("Node component destroyed.");
  // TODO: other processes should handle this.
  // CloseLop should be handled by a Logger class
  // dialogue should handled by interface itself.
  CloseLog();
  interface_->WriteDialogueToFile(network_->GetId());
}

void Node::RegisterNetwork(std::shared_ptr<Network> network) {
  network_ = network;

  // Every node keeps its own sequence number.
  seq_num_table_[network_->GetId()] = kInitialSequenceNumber;
}

void Node::RegisterInterface(std::shared_ptr<Interface> interface) {
  interface_ = interface;
}

void Node::RegisterFileManager(std::shared_ptr<FileManager> file_manager) {
  file_manager_ = file_manager;
}

// TODO: network module should handle this
void Node::PollEvents() {
  int event_count = epoll_wait(network_->epoll_fd_, network_->events_, 
                              network_->kMaxEventsNum, network_->kEpollFrequencyInMs);
  if (event_count < 0) {
    if (errno == EINTR) {
      return;
    }

    Log("Error: epoll_wait failed");                                               
    exit(EXIT_FAILURE);
  } else if (event_count == 0) {
    // Check if there is any local user input
    if (interface_->local_inputs_.size() > 0) {
      HandleLocalHostInput();
    }
  } else {
    for (int i = 0; i < event_count; ++i) {
      if (network_->events_[i].data.fd == network_->listen_fd_) {
        HandleMessageFromPeer();
      } else {
        std::cerr << "Unmatched file descriptor" << std::endl;
      }
    }
  }
}

void Node::SendTableToRandomPeer() {
  while (!StopRequested()) {
    Message status_message(network_->GetId(), seq_num_table_);
    network_->SendMessageToRandomPeer(status_message);
    std::this_thread::sleep_for(std::chrono::milliseconds(kPeriodInMs));
  }
}

void Node::HandleMessageFromPeer() {
  struct sockaddr_in peer_addr;
  socklen_t addrlen = sizeof(peer_addr);
  
  char buf[Message::kMaxMessageSize + 1];
  int len = recvfrom(network_->listen_fd_, buf, Message::kMaxMessageSize, 0,
              (struct sockaddr *)&peer_addr, &addrlen);
  if (len < 0) {
    perror("recvfrom failed");
    return;
  }

  Message msg = network_->ParseMessage(buf, len);

  // Check if it is a new node first
  // If it is a new peer, parse its chat text in the form of IP:PORT, or HOSTNAME:PORT
  if (!network_->IsKnownPeer(msg["id"])) {
    network_->InsertPeer(msg["id"], peer_addr);
  }

  // TODO: msg type should be converted to integer
  //  and it's better to use switch
  // TODO: should have file_manager handle different operations related to file managing
  // Handle message
  if (msg["type"] == kTypes[kRumor]) {
    HandleRumorMessage(msg, peer_addr);
  } else if (msg["type"] == kTypes[kStatus]) {
    HandleStatusMessage(msg);
  } else if (msg["type"] == kTypes[kBlockRequest]) {
    file_manager_->HandleBlockRequest(msg);
  } else if (msg["type"] == kTypes[kBlockReply]) {
    file_manager_->HandleBlockReply(msg);
  } else if (msg["type"] == kTypes[kSearchRequest]) {
    file_manager_->HandleSearchRequest(msg);
  } else if (msg["type"] == kTypes[kSearchReply]) {
    file_manager_->HandleSearchReply(msg);
  } else {
    Log("Unmatched file type " + msg["type"]);
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
      network_->SendMessageToRandomPeer(Message(id, seq_num, chat.content, chat.timestamp));
    } else if (seq_num_table_[id] < seq_num) {
      Log("wants to have seq_num: " + std::to_string(seq_num_table_[id]) + 
          " from " + id);
      send_status_message = true;
    }
  }

  // This node has found message it has not received, send status message
  // letting others know this node want unrecieved messages.
  if (send_status_message) {
    network_->SendMessageToRandomPeer(Message(network_->GetId(), seq_num_table_));
  }
}

// Since it's UDP, we need to resend messages if datagrams are dropped
// The ack message is a status message 
void Node::AcknowledgeMessage(const Id &id) {
  Message status_message(network_->GetId(), seq_num_table_);

  // TODO: Need to send to the sender as the ACK instead of a random neighbor
  network_->SendMessageToRandomPeer(status_message);
}

// Handle rumor message that is from remote nodes
void Node::HandleRumorMessage(const Message &msg, sockaddr_in &peer_addr) {
  bool new_seq_num = false;

  if (msg["id"] == network_->GetId()) {
    return;
  }

  // If a new id comes, the default sequence number for that id should be set
  if (seq_num_table_.find(msg["id"]) == seq_num_table_.end()) {
    seq_num_table_[msg["id"]] = kInitialSequenceNumber;
  }

  int &last_sequence_number = seq_num_table_[msg["id"]];
  int msg_seq_num = std::stoi(msg["seqnum"]);
  // A message already received, discard
  if (msg_seq_num < last_sequence_number) {
    return ;
  // The expected message sequence number
  } else if (msg_seq_num == last_sequence_number) {
    ProcessRumorMessage(msg);
    new_seq_num = true;
  // Waiting for message with seq#1, instead seq#2 is received. Discard it.
  // TODO: Maybe we should cache it?
  } else {
    // Discard
  }

  // Update destination-sequenced distance vector
  if (new_seq_num) {
    network_->UpdateDistanceVector(msg["id"], peer_addr);
  }
}

void Node::ProcessRumorMessage(const Message &msg) {
  // Log for debug
  Log("Received Rumor message from " + msg["id"] + " seq_number: " + 
      msg["seqnum"] + " \"" + msg["data"] + "\" timestamp:" + msg["timestamp"]);

  // Store this message
  text_storage_.Put(msg["id"], std::stoi(msg["seqnum"]), msg["data"], 
                    std::stol(msg["timestamp"]));

  // Send to random neighbor
  network_->SendMessageToRandomPeer(msg);

  // Insert into dialogue to be printed
  Chat chat(msg["data"], std::stol(msg["timestamp"]));
  interface_->InsertToDialogue(msg["id"], msg["data"], std::stol(msg["timestamp"]));

  // Update sequence number table
  ++seq_num_table_[msg["id"]];
}

// Read user input and send to random peer
void Node::HandleLocalHostInput() {
  interface_->local_inputs_mutex_.lock();

  while (interface_->local_inputs_.size() != 0) {
    const Input &input = interface_->local_inputs_.front();

    if (input.mode == kChat) {
      Message msg(network_->GetId(), seq_num_table_[network_->GetId()], input.content, input.timestamp);
      ProcessRumorMessage(msg);
    } else if (input.mode == kUpload) {
      file_manager_->Upload(input.content);
    } else if (input.mode == kDownload) {
      file_manager_->Download(input.content);
    } else if (input.mode == kSearch) {
      file_manager_->Search(input.content);
    } else {
      Log("WARNING: current input mode " + std::to_string(input.mode) + 
      " is not matched to any of the mode");
    }

    interface_->local_inputs_.pop();
  }

  interface_->local_inputs_mutex_.unlock();
}

void Node::Run() {
  std::thread send_table_thread(&Node::SendTableToRandomPeer, this);

  while (!StopRequested()) {
    PollEvents();
  }

  send_table_thread.join();
}

}

#endif
