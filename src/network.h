#ifndef YELA_NETWORK_H_
#define YELA_NETWORK_H_

#include <string>
#include <vector>

namespace yela {

class Network {
 public:
  Network(int my_port, const std::vector<int> &peers);
  ~Network();
 protected:
  std::vector<int> peers_;
  int my_port_;
  int recv_fd_; 
  void BroadcastMessage(const std::string &message);

 private:
  void EstablishReceiver();
  void SendMessage(int target_port, const std::string &message);
  
  const int kMaxEventsNum = 256;
  struct epoll_event event_;
  struct epoll_event events_[kMaxEventsNum];
  int epoll_fd_;

  void InitializeEpoll();
  void PollEvents();
};

}

#endif
