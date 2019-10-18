#include "rumor.h"

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

#include "log.h"

namespace yela {

Rumor::~Rumor() {
  Log("Rumor component terminated.");
}

void Rumor::RegisterNetwork(std::shared_ptr<Network> network) {
  network_ = network;

  seq_num_table_[network_->GetId()] = kInitialSequenceNumber;
}

void Rumor::RegisterInterface(std::shared_ptr<Interface> interface) {
  interface_ = interface;
}

void Rumor::RelayMessage(const Message &msg) {
  network_->SendMessageToRandomPeer(msg);
}
 
void Rumor::PushRemoteMessage(const Message &msg) {
  msg_queue_.Push(msg);
}

// Convert local input into a message form and then push
void Rumor::PushLocalInput(const Input &input) {
  Message msg(network_->GetId(), seq_num_table_[network_->GetId()], 
              input.content, input.timestamp);
  msg_queue_.Push(msg);
}

void Rumor::ProcessOneMessage() {
  std::shared_ptr<Message> msg = msg_queue_.Pop(std::chrono::seconds(1));
  if (msg == nullptr) {
    return;
  }

  if ((*msg)["type"] == kTypes[kRumor]) { 
    HandleRumorMessage(*msg);
  } else if ((*msg)["type"] == kTypes[kStatus]) {
    HandleStatusMessage(*msg);
  }
}

// Handle rumor message that is from remote peers or local user's input
void Rumor::HandleRumorMessage(const Message &msg) {
  // If a new id comes, the default sequence number for that id should be set
  if (seq_num_table_.find(msg["id"]) == seq_num_table_.end()) {
    seq_num_table_[msg["id"]] = kInitialSequenceNumber;
  }

  int &last_sequence_number = seq_num_table_[msg["id"]];
  int msg_seq_num = std::stoi(msg["seqnum"]);
  // The expected message sequence number
  if (msg_seq_num != last_sequence_number) {
    return;
  }

  InsertNewRumorMessage(msg);                                                         
  
  // Update destination-sequenced distance vector                                     
  //network_->UpdateDistanceVector(msg["id"], peer_addr);
} 

// Two tasks:
//  1. Check if the sender needs anything that can be sent from this node
//  2. Check if this node needs anything that sender has already seen
void Rumor::HandleStatusMessage(const Message &msg) {
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

void Rumor::AcknowledgeMessage(const Id &id) {
  Message status_message(network_->GetId(), seq_num_table_);
  network_->SendMessageToRandomPeer(status_message);
}

void Rumor::InsertNewRumorMessage(const Message &msg) {
  Log("Received Rumor message from " + msg["id"] + " seq_number: " +                  
      msg["seqnum"] + " \"" + msg["data"] + "\" timestamp:" + msg["timestamp"]);      
  
  // Store this message                                                               
  text_storage_.Put(msg["id"], std::stoi(msg["seqnum"]), msg["data"],                 
                    std::stol(msg["timestamp"]));                                     
                                                                                      
  // Insert into dialogue to be printed                                               
  Chat chat(msg["data"], std::stol(msg["timestamp"]));                                
  interface_->InsertToDialogue(msg["id"], msg["data"], std::stol(msg["timestamp"]));  
  
  // Update sequence number table                                                     
  ++seq_num_table_[msg["id"]];
}

void Rumor::Run() {
  std::thread table_sending(&Rumor::SendTableToRandomPeer, this);

  while (!StopRequested()) {
    ProcessOneMessage();
  }

  table_sending.join();
}

// This is another thread, so as to not mix the logic
void Rumor::SendTableToRandomPeer() {
  while (!StopRequested()) {
    Message status_message(network_->GetId(), seq_num_table_);
    network_->SendMessageToRandomPeer(status_message);

    std::this_thread::sleep_for(std::chrono::milliseconds(kPeriodInMs));
  }
}

}
