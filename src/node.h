#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <queue>
#include <iostream>

#include "network.h"
#include "interface.h"


namespace yela {

// TODO: message part of the work should be in Node class
class Node: public Interface, public Network {
 public:
  Node(const int my_port, const std::vector<int> &peers);
  ~Node();

  void PollEvents();
  void Run();
 private:
  // Unique string to identify a node
  Origin origin_;
  
  // Sequence number is in essence vector clocks to Gossip
  typedef std::unordered_map<Origin, SequenceNumberTable> SequenceNumberTables;
  SequenceNumberTables sequence_number_tables_;

  int sequence_number_;

  // Buffer for out of order incoming messages
  // Sorted based on increasing sequence number
  bool Compare(const Message &msg_a, const Message &msg_b) {
    return msg_a.sequence_number < msg_b.sequence_number;
  }
  typedef std::priority_queue<Message, std::vector<Message>,
            std::function<bool(const Message &msg_a, const Message &msg_b)>> Buffer;
  typedef std::unordered_map<Origin, Buffer> BufferTable;
  BufferTable buffer_table_;

  void WriteHistoryToFile();
  void AcknowledgeMessage(const sockaddr_in &peer_addr, int expected_sequence_number);
  void HandleStatusMessage(const Message &msg);
  void HandleRumorMessage(const Message &msg, const sockaddr_in &peer_addr);
  void HandleMessageFromPeer();
  void HandleLocalHostInput();
};

}
