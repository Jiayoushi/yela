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

  seq_num_table_.Set(network_->GetOrigin(), kInitialSequenceNumber);
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
  Message msg(network_->GetOrigin(), seq_num_table_.Get(network_->GetOrigin()), 
              input.content, input.timestamp);
  msg_queue_.Push(msg);
}

void Rumor::ProcessOneMessage() {
  std::shared_ptr<Message> msg = msg_queue_.Pop(std::chrono::seconds(1));
  if (msg == nullptr)
    return;

  Origin origin = (*msg)["origin"];

  if ((*msg)["type"] == kTypes[kRumor]) { 
    HandleRumorMessage(*msg);
  } else if ((*msg)["type"] == kTypes[kStatus]) {
    MsgPtr sent = HandleStatusMessage(*msg);
    if (sent == nullptr) {
      exchanges_[origin].need_ack = false; 
      return;
    }

    std::lock_guard<std::mutex> lck(exchanges_[origin].lock);
    exchanges_[origin].last_sent_msg = sent;
  }
}

// Handle rumor message that is from remote peers or local user's input
void Rumor::HandleRumorMessage(const Message &msg) {
  // If a new origin comes, the default sequence number for that origin should be set
  seq_num_table_.SetIfAbsent(msg["origin"], kInitialSequenceNumber);

  int last_sequence_number = seq_num_table_.Get(msg["origin"]);
  int msg_seq_num = std::stoi(msg["seqnum"]);

  /* The expected message sequence number
   * NOTE: Rumormongering
   * If a message has been seen before, this node does not even relay the message
   */
  if (msg_seq_num != last_sequence_number)
    return;

  InsertNewRumorMessage(msg);
  
  Origin origin = msg["origin"];
  // Send status vector to where this message come from
  AcknowledgeMessage(origin);

  Origin neighbor = network_->SendMessageToRandomPeer(msg);
  std::lock_guard<std::mutex> lck(exg_lock_);
  if (exchanges_.find(origin) == exchanges_.end()) {
    exchanges_.emplace(origin, msg);
  } else {
    exchanges_[origin].need_ack = true;
    exchanges_[origin].last_sent_msg = std::make_shared<Message>(msg);
  }
}

// Two tasks:
//  1. Check if the sender needs anything that can be sent from this node
//  2. Check if this node needs anything that sender has already seen
MsgPtr Rumor::HandleStatusMessage(const Message &msg) {
  Log("Received status message from " + msg["origin"]);
  const SequenceTable neighbor_seq_table(msg["seqtable"]);

  Origin origin = msg["origin"];

  bool send_status_message = false;
  for (auto p = neighbor_seq_table.cbegin(); p != neighbor_seq_table.cend(); ++p) {
    int seq_num = p->second;

    // If this node has never seen this origin before, it needs to record this
    // new origin, and set the default seq number.
    seq_num_table_.SetIfAbsent(p->first, kInitialSequenceNumber);

    // If I have rumor my neighbor has not seen before, send that rumor 
    if (seq_num_table_.Get(p->first) > seq_num) {
      const Chat chat = text_storage_.Get(p->first, seq_num);
      MsgPtr m = std::make_shared<Message>(p->first, seq_num, 
                                           chat.content, chat.timestamp);
      network_->SendMessageToTargetPeer(*m.get(), origin);
      return m;
    // My neighbor has rumor that I do not have
    } else if (seq_num_table_.Get(p->first) < seq_num) {
      Log(" Wants to have seq_num: " + std::to_string(seq_num_table_.Get(p->first)) +
          " from " + p->first);
      send_status_message = true;
    }
  }

  // This node has found message it has not received, send status message
  // letting others know this node want unrecieved messages.
  if (send_status_message)
    network_->SendMessageToTargetPeer(Message(network_->GetOrigin(), seq_num_table_),
                                      origin);

  return nullptr;
}

void Rumor::AcknowledgeMessage(const Origin &origin) {
  Message status_message(network_->GetOrigin(), seq_num_table_);
  network_->SendMessageToTargetPeer(status_message, origin);
}

void Rumor::InsertNewRumorMessage(const Message &msg) {
  Log("Received Rumor message from " + msg["origin"] + " seq_number: " +                  
      msg["seqnum"] + " \"" + msg["data"] + "\" timestamp:" + msg["timestamp"]);      
  
  // Store this message                                                               
  text_storage_.Put(msg["origin"], std::stoi(msg["seqnum"]), msg["data"],
                    std::stol(msg["timestamp"]));
                                                                                      
  // Insert into dialogue to be printed                                               
  Chat chat(msg["data"], std::stol(msg["timestamp"]));                                
  interface_->InsertToDialogue(msg["origin"], msg["data"],
                               std::stol(msg["timestamp"]));  
  
  // Update sequence number table
  seq_num_table_.Increment(msg["origin"]);
}

void Rumor::Exchange() {
  while (!StopRequested()) {
    std::lock_guard<std::mutex> lck(exg_lock_);

    for (auto p = exchanges_.begin(); p != exchanges_.end(); ++p)
      if (p->second.need_ack)
        network_->SendMessageToTargetPeer(*p->second.last_sent_msg.get(), p->first);

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void Rumor::Run() {
  std::thread table_sending(&Rumor::SendTableToRandomPeer, this);
  std::thread exchange_thread(&Rumor::Exchange, this);

  while (!StopRequested())
    ProcessOneMessage();

  table_sending.join();
}

// This is another thread, so as to not mix the logic
void Rumor::SendTableToRandomPeer() {
  while (!StopRequested()) {
    Message status_message(network_->GetOrigin(), seq_num_table_);
    network_->SendMessageToRandomPeer(status_message);

    std::this_thread::sleep_for(std::chrono::milliseconds(kPeriodInMs));
  }
}

}
