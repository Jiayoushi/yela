#ifndef YELA_NETWORK_H_
#define YELA_NETWORK_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/epoll.h>
#include <string>
#include <vector>

#include "cereal/types/string.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/types/concepts/pair_associative_container.hpp"

namespace yela {

struct NetworkId {
  std::string id; 
  std::string ip;
  std::string hostname; // TODO: just delete this later
  int port;

  NetworkId() {}

  NetworkId(const std::string &d, const std::string &p,
            const std::string &h, int pt):
            id(d), ip(p), hostname(h), port(pt) {
  }
};


const size_t kMaxMessageSize = 1024;

enum MessageType {
  kRumorMessage = 0,
  kStatusMessage = 1,
};

typedef std::string Id;
typedef int SequenceNumber;

// table[id] is the expected next sequence number from id.
typedef std::unordered_map<Id, SequenceNumber> SequenceNumberTable;

typedef std::string Content;

struct Message {
 public:
  Message() {}

  // Constructor for rumor message
  Message(const Id &i, const SequenceNumber &seq_num, const Content &text):
    message_type(kRumorMessage), sequence_number(seq_num), 
    id(i), content(text) {

  }

  // Constructor for Status message
  Message(const Id &i, const SequenceNumberTable &t):
    id(i), message_type(kStatusMessage), table(t) {

  }

  MessageType message_type;

  // Rumor message
  SequenceNumber sequence_number;
  Id id;
  Content content;

  // Status message
  SequenceNumberTable table;

  template<typename Archive>
  void serialize(Archive &archive) {
    archive(message_type,
            sequence_number, id, content,
            table);
  }
};




class Network {
 public:
  Network(const std::string &settings_file);
  ~Network();

 private:
  void ReadSettings(const std::string &settings_file);
  NetworkId ParseSettingLine(const std::string &line);
  void SendMessage(const NetworkId &id, const Message &msg);

 protected:
  int listen_fd_; 
  void SendMessageToRandomPeer(const Message &msg);

  void Listen();
  Message ParseMessage(const char *data, const int size);
  
  static const int kMaxEventsNum = 256;
  struct epoll_event events_[kMaxEventsNum];

  const int kEpollFrequencyInMs = 500;
  int epoll_fd_;
  struct epoll_event input_event_;
  struct epoll_event peer_event_;

  void InitializeEpoll();

  // Helper functions
  std::string GetIp(const struct sockaddr_in &addr);
  int GetPort(const struct sockaddr_in &addr);

  void InsertPeer(const Id &id, const struct sockaddr_in &addr);

  // This node's network id
  NetworkId me_;

  // All nodes' network id
  std::vector<NetworkId> distance_vector_;
  void UpdateDistanceVector(const Id &msg_id, const struct sockaddr_in &addr);

  // Dynamically peers
  bool IsKnownPeer(const std::string &id);
  void InsertPeer(const NetworkId &peer);
};

}

#endif
