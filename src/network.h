#ifndef YELA_NETWORK_H_
#define YELA_NETWORK_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/epoll.h>
#include <string>
#include <vector>

namespace yela {

class Network {
 public:
  Network(int my_port, const std::vector<int> &peers);
  ~Network();
 protected:
  std::vector<int> peers_;
  const std::string my_ip_ = "127.0.0.1";
  int my_port_;
  int listen_fd_; 
  void BroadcastMessage(const std::string &message);

  void EstablishReceiver();
  void SendMessage(int target_port, const std::string &message);
  
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
