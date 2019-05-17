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

namespace yela {

const size_t kMaxMessageSize = 1024;

struct Message {
 public:
  Message() {}

  Message(int seqn, const std::string &orig, const std::string &text):
    sequence_number(seqn), origin(orig), chat_text(text) {}

  int sequence_number;
  std::string origin;
  std::string chat_text;

  template<typename Archive>
  void serialize(Archive &archive) {
    archive(sequence_number, origin, chat_text);
  }
};

class Network {
 public:
  Network(const int my_port, const std::vector<int> &peers);
  ~Network();
 protected:
  std::vector<int> peers_;
  const std::string my_ip_ = "127.0.0.1";
  int my_port_;

  int listen_fd_; 
  void SendMessageToRandomPeer(const Message &msg);
  void BroadcastMessage(const Message &msg);

  void Listen();
  void SendMessage(int peer_port, const Message &msg);
  Message ParseMessage(const char *data, const int size);
  
  static const int kMaxEventsNum = 256;
  struct epoll_event events_[kMaxEventsNum];

  int epoll_fd_;
  struct epoll_event input_event_;
  struct epoll_event peer_event_;

  void InitializeEpoll();

  // Helper functions
  std::string GetIp(struct sockaddr_in &addr);
  std::string GetPort(struct sockaddr_in &addr);
};

}

#endif
