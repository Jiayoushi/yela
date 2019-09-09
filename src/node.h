#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <queue>
#include <thread>
#include <iostream>

#include "file_manager.h"
#include "network.h"
#include "interface.h"
#include "log.h"

namespace yela {

class TextStorage {
 public:
  TextStorage() {}

  void Put(const Id &id, const SequenceNumber &seq_num, const std::string &content,
           const long timestamp) {
    storage_[id][seq_num] = Chat(content, timestamp);
  }

  Chat Get(const Id &id, const SequenceNumber &seq_num) {
    auto p = storage_.find(id);
    if (p == storage_.end()) {
      std::string msg = "ERROR: message storage failed to map id: " + id +
       " with sequence number " + std::to_string(seq_num);
      Log(msg);
      return Chat();
    }

    auto x = p->second.find(seq_num);
    if (x == p->second.end()) {
      std::string msg = "ERROR: message storage failed to map sequence number: "
        + std::to_string(seq_num) + " from id: " + id;
      Log(msg);
      return Chat();
    }

    return storage_[id][seq_num];
  }

 private:
  std::unordered_map<Id, std::unordered_map<SequenceNumber, Chat>> storage_;
};



class Node {
 public:
  Node(const std::string &settings_file);
  ~Node();

  void Run();
 private:
  // Components
  Interface interface_;
  Network network_;
  FileManager file_manager_;

  // Information about this node
  Id id_;

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
  void HandleRumorMessage(const Message &msg, sockaddr_in &peer_addr);
  void HandleMessageFromPeer();
  void HandleLocalHostInput();
  
  void FileUploadPostAction(int status, const std::string &filename);
};
}
