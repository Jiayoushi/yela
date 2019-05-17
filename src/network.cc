#include "network.h"

#include <cstring>                                                                    
#include <unistd.h>
#include <netdb.h>                                                                    
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <iostream>

namespace yela {

Network::Network(const int my_port, const std::vector<int> &peers):
  my_port_(my_port), peers_(peers) {
  Listen();
  InitializeEpoll();
}

Network::~Network() {
  shutdown(listen_fd_, SHUT_RDWR);
  close(listen_fd_);
  close(epoll_fd_);
}

// TODO: understand this and add comments
void Network::InitializeEpoll() {
  // 0 means epoll_create
  // Or it can set to EPOLL_CLOEXEC
  if ((epoll_fd_ = epoll_create1(0)) < 0) {
    perror("Error: epoll_create1 failed");
    exit(EXIT_FAILURE);
  }

  input_event_.events = EPOLLIN;
  input_event_.data.fd = 0;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, STDIN_FILENO, &input_event_) < 0) {
    perror("Error: failed to add file descriptor 0 to epoll");
    exit(EXIT_FAILURE);
  }

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
  my_address.sin_port = htons(my_port_);

  if (bind(listen_fd_, (struct sockaddr *)&my_address, sizeof(my_address)) < 0) {
    perror("Error: cannot bind");                                                     
    exit(EXIT_FAILURE);                                                               
  }                                                                                   
}

void Network::BroadcastMessage(const Message &msg) {
  for (int peer_port: peers_) {
    SendMessage(peer_port, msg);
  }
}

void Network::SendMessage(int target_port, const Message &msg) {
  int fd = socket(AF_INET, SOCK_DGRAM, 0);                                            
  if (fd < 0) {                                                                       
    perror("Error: SendMessage failed to create a socket.");
    return;
  }

  struct sockaddr_in taget_address;
  memset(&taget_address, 0, sizeof(taget_address));
  taget_address.sin_family = AF_INET;
  taget_address.sin_port = htons(target_port);

  struct hostent *host_info = gethostbyname("127.0.0.1");
  if (host_info == nullptr) {
    perror("Error: gethostbyname failed");
    exit(EXIT_FAILURE);
  }

  // h_addr_list contains a list of address matching the host name
  std::memcpy(&taget_address.sin_addr, host_info->h_addr_list[0], 
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
  if (sendto(fd, buf, ss.str().size(), 0,
             (struct sockaddr *)&taget_address, sizeof(taget_address)) < 0) {
    perror("Error: SendMessage sendto failed");
    exit(EXIT_FAILURE);
  }

  // DEBUG
  //std::cout << "Message " << message << " sent to " << target_port << std::endl;
  close(fd);
}

void Network::SendMessageToRandomPeer(const Message &msg) {
  int random_target_port = peers_[std::rand() % peers_.size()];
  SendMessage(random_target_port, msg);
}

Message Network::ParseMessage(const char *data, const int size) {
  std::string s(data, size);
  std::stringstream ss(s);
  cereal::BinaryInputArchive i_archive(ss);
  Message msg;
  i_archive(msg);
  return msg;
}

std::string Network::GetIp(struct sockaddr_in &addr) {
  return std::string(inet_ntoa(addr.sin_addr));
}

std::string Network::GetPort(struct sockaddr_in &addr) {
  return std::to_string(ntohs(addr.sin_port));
}

}
