#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <queue>
#include <thread>
#include <iostream>

#include "network.h"
#include "interface.h"
#include "log.h"

namespace yela {

class TextStorage {
 public:
  TextStorage() {}

  void Put(const Id &id, const SequenceNumber &seq_num, const Content &text) {
    storage_[id][seq_num] = text;
  }

  std::string Get(const Id &id, const SequenceNumber &seq_num) {
    auto p = storage_.find(id);
    if (p == storage_.end()) {
      std::string msg = "ERROR: message storage failed to map id: " + id +
       " with sequence number " + std::to_string(seq_num);
      Log(msg);
      return "";
    }

    auto x = p->second.find(seq_num);
    if (x == p->second.end()) {
      std::string msg = "ERROR: message storage failed to map sequence number: "
        + std::to_string(seq_num) + " from id: " + id;
      Log(msg);
      return "";
    }

    return storage_[id][seq_num];
  }

 private:
  std::unordered_map<Id, std::unordered_map<SequenceNumber, Content>> storage_;
};



class Node: public Interface, public Network {
 public:
  Node(const std::string &settings_file);
  ~Node();

  void Run();
 private:
  const std::string kExitMsgForTesting = "nSQ5oCay0gG6qNJURWxZ";
  void PollEvents();

  // Anti-entropy
  // Periodic send message
  const int kPeriodInMs = 1000;
  std::thread send_table_thread; 
  void SendTableToRandomPeer();

  // Sequence table
  const int kInitialSequenceNumber = 1;
  SequenceNumberTable seq_num_table_;

  // Store the messages this node has received
  TextStorage text_storage_;

  void AcknowledgeMessage(const Id &origin);
  void ProcessRumorMessage(const Message &msg);
  void HandleStatusMessage(const Message &msg);
  bool HandleRumorMessage(const Message &msg);
  void HandleMessageFromPeer();
  void HandleLocalHostInput();
};




}
