#include "network.h"

#include <unistd.h>
#include <netdb.h>                                                                    
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <iostream>
#include <random>
#include <cassert>
#include <cstring>

#include "log.h"

namespace yela {

Network::Network(const std::string &settings_file) {
  ReadSettings(settings_file);
  Listen();
  InitializeEpoll();
}

Network::~Network() {
  shutdown(listen_fd_, SHUT_RDWR);
  close(listen_fd_);
  close(epoll_fd_);
}

NetworkId Network::ParseSettingLine(const std::string &line) {
  std::stringstream ss(line);
  std::string item;
  std::vector<std::string> items;
  while (std::getline(ss, item, ' ')) {
    items.push_back(item);
  }

  return NetworkId(items[0], items[1], items[3], atoi(items[2].c_str()));
}

void Network::ReadSettings(const std::string &settings_file) {
  std::ifstream f(settings_file);
  if (!f.is_open()) {
    std::cerr << "Error: failed to open settings file." << std::endl;
    exit(EXIT_FAILURE);
  }

  std::string line;
  // The first line is this node's information
  getline(f, line);
  me_ = ParseSettingLine(line);

  // The rest is other nodes' information
  while (getline(f, line)) {
    peers_.push_back(ParseSettingLine(line));
  }

  f.close();
}

void Network::InitializeEpoll() {
  if ((epoll_fd_ = epoll_create1(0)) < 0) {
    perror("Error: epoll_create1 failed");
    exit(EXIT_FAILURE);
  }

  // Listen to remote message
  peer_event_.events = EPOLLIN;
  peer_event_.data.fd = listen_fd_;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &peer_event_) < 0) {
    perror("Error: failed to add file descriptor 0 to epoll");
    exit(EXIT_FAILURE);
  }
}

void Network::Listen() {
  // socket(domain, type, protocol)                                                   
  if ((listen_fd_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Error: cannot create socket");                                            
    exit(EXIT_FAILURE);                                                               
  }                                                                                   
  struct sockaddr_in my_address;                                                      
  memset(&my_address, 0, sizeof(my_address));                                         
  my_address.sin_family = AF_INET;                                                    
  my_address.sin_addr.s_addr = htonl(INADDR_ANY);                                     
  // htons: host to network - short: convert a number into a 16-bit network           
  // representation. This is commonly used to store a port number into a              
  // sockaddr structure                                                               
  my_address.sin_port = htons(me_.port);

  if (bind(listen_fd_, (struct sockaddr *)&my_address, sizeof(my_address)) < 0) {
    std::cerr << "Attempt to bind to port " << me_.port << std::endl;
    perror("Error: cannot bind");
    exit(EXIT_FAILURE);                                                               
  }                                                                                   
}

void Network::SendMessage(const NetworkId &target, const Message &msg) {
  struct sockaddr_in target_address;
  memset(&target_address, 0, sizeof(target_address));
  target_address.sin_family = AF_INET;
  target_address.sin_port = htons(target.port);

  struct hostent *host_info = gethostbyname(target.ip.c_str());
  if (host_info == nullptr) {
    perror("Error: gethostbyname failed");
    exit(EXIT_FAILURE);
  }

  // h_addr_list contains a list of address matching the host name
  std::memcpy(&target_address.sin_addr, host_info->h_addr_list[0], 
              host_info->h_length);

  // Serialize the Message struct into bytes
  std::stringstream ss;
  cereal::BinaryOutputArchive o_archive(ss);
  o_archive(msg);

  // Note: Copy byte by byte, do not append null character
  char buf[kMaxMessageSize];
  for (int i = 0; i < ss.str().size(); ++i) {
    buf[i] = ss.str()[i];
  }

  // Send the serialized message
  if (sendto(listen_fd_, buf, ss.str().size(), 0,
             (struct sockaddr *)&target_address, sizeof(target_address)) < 0) {
    perror("Error: SendMessage sendto failed");
    exit(EXIT_FAILURE);
  }

  // Log
  if (msg.message_type == kRumorMessage) {
    Log("Send rumor to " + target.id +  
        " (" + target.ip + "," + std::to_string(target.port) + ") " +
        " \"" + msg.chat_text + "\"");
  } else {
    std::string log_msg = "Send table to " + target.id + 
      " (" + target.ip + "," + std::to_string(target.port) + ") " + 
      " [";
    for (auto p = msg.table.cbegin(); p != msg.table.cend(); ++p) {
      log_msg += p->first + ":" + std::to_string(p->second) + " ";
    }
    if (log_msg.back() != '[') {
      log_msg.pop_back();
    }
    log_msg += "]";
    Log(log_msg);
  }
}

void Network::SendMessageToRandomPeer(const Message &msg) {
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> gen(0, peers_.size() - 1);
  
  int random_index = gen(mt);
  if (peers_[random_index].id == msg.id) {
    return;
  }

  SendMessage(peers_[random_index], msg);
}

Message Network::ParseMessage(const char *data, const int size) {
  std::string s(data, size);
  std::stringstream ss(s);
  cereal::BinaryInputArchive i_archive(ss);
  Message msg;
  i_archive(msg);
  return msg;
}

std::string Network::GetIp(const struct sockaddr_in &addr) {
  return std::string(inet_ntoa(addr.sin_addr));
}

int Network::GetPort(const struct sockaddr_in &addr) {
  int t = ntohs(addr.sin_port);
  return t;
}

void Network::InsertPeer(const Id &id, const struct sockaddr_in &addr) {
  peers_.push_back(NetworkId(id, GetIp(addr), "", GetPort(addr)));
}

bool Network::IsKnownPeer(const std::string &id) {
  for (const NetworkId &nid: peers_) {
    if (nid.id == id) {
      return true;
    }
  }
  return false;
}

void Network::InsertPeer(const NetworkId &peer) {
  peers_.push_back(peer);
}

}
