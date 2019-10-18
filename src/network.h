#ifndef YELA_NETWORK_H_
#define YELA_NETWORK_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/epoll.h>
#include <string>
#include <vector>

#include "message.h"

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

class Network {
 public:
  Network(const std::string &settings_file);
  ~Network();

  const Id & GetId();

  // Message
  void SendMessageToTargetPeer(const Message &msg, const Id &id);
  void SendMessageToRandomPeer(const Message &msg);
  Message ParseMessage(const char *data, const int size);

  // Dynamically peers
  bool IsKnownPeer(const std::string &id);
  void InsertPeer(const NetworkId &peer);
  void InsertPeer(const Id &id, const struct sockaddr_in &addr);
  void UpdateDistanceVector(const Id &id, const struct sockaddr_in &addr);

  // Epoll 
  // TODO: should be refactored
  static const int kMaxEventsNum = 256;
  struct epoll_event events_[kMaxEventsNum];

  const int kEpollFrequencyInMs = 500;
  int epoll_fd_;
  struct epoll_event input_event_;
  struct epoll_event peer_event_;

  int listen_fd_;
 private:
  // Init
  void ReadSettings(const std::string &settings_file);
  NetworkId ParseSettingLine(const std::string &line);
  
  // Message
  void SendMessage(const NetworkId &id, const Message &msg);

  void Listen();
  void InitializeEpoll();

  // Helper functions
  std::string GetIp(const struct sockaddr_in &addr);
  int GetPort(const struct sockaddr_in &addr);

  // This node's network id
  NetworkId me_;

  // All nodes' network id
  int FindTargetNetworkId(const Id &id);
  std::vector<NetworkId> distance_vector_;
};

}

#endif
