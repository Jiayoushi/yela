#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <queue>
#include <iostream>

#include "network.h"
#include "interface.h"


namespace yela {

class TextStorage {
 public:
  TextStorage() {}

  void Put(const Id &id, const SequenceNumber &seq_num, const ChatText &text) {
    storage_[id][seq_num] = text;
  }

  std::string Get(const Id &id, const SequenceNumber &seq_num) {
    auto p = storage_.find(id);
    if (p == storage_.end()) {
      std::cerr << "ERROR: message storage failed to map id: " << id << std::endl;
      return "";
    }

    auto x = p->second.find(seq_num);
    if (x == p->second.end()) {
      std::cerr << "ERROR: message storage failed to map sequence number: "
      << seq_num << std::endl;
      return "";
    }

    return storage_[id][seq_num];
  }

 private:
  std::unordered_map<Id, std::unordered_map<SequenceNumber, ChatText>> storage_;

};

class Node: public Interface, public Network {
 public:
  Node(const int my_port, const std::vector<int> &peers);
  ~Node();

  void PollEvents();
  void Run();
 private:
  // Unique string to identify a node
  Id id_;
 
  const int kInitialSequenceNumber = 1;
  int sequence_number_;
  SequenceNumberTable seq_num_table_;

  TextStorage text_storage_;

  void AcknowledgeMessage(const Id &origin);
  void HandleStatusMessage(const Message &msg);
  void HandleRumorMessage(const Message &msg, const sockaddr_in &peer_addr);
  void HandleMessageFromPeer();
  void HandleLocalHostInput();

  void WriteHistoryToFile();
};




}
