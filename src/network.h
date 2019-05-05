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
  Message() {
  }

  Message(const std::string &ip, const std::string &port, const std::string &d):
    sender_ip(ip), sender_port(port), data(d) {
  }

  std::string sender_ip;
  std::string sender_port;
  std::string data;

  template<typename Archive>
  void serialize(Archive &archive) {
    archive(sender_ip, sender_port, data);
  }
};

class Network {
 public:
  Network(int my_port, const std::vector<int> &peers);
  ~Network();
 protected:
  std::vector<int> peers_;
  const std::string my_ip_ = "127.0.0.1";
  int my_port_;
  int listen_fd_; 
  void BroadcastMessage(const Message &msg);

  void EstablishReceiver();
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
