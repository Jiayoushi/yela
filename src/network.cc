#include "network.h"

#include <cstring>                                                                    
#include <unistd.h>
#include <netdb.h>                                                                    
#include <sys/types.h>                                                                
#include <sys/socket.h>                                                               
#include <netinet/in.h>                                                               
#include <netinet/ip.h>                                                               
#include <iostream>              

namespace yela {

Network::Network(int my_port, const std::vector<int> &peers):
  my_port_(my_port), peers_(peers) {
  EstablishReceiver();
  InitializeEpoll();
}

Network::~Network() {
  shutdown(recv_fd_, SHUT_RDWR);
  close(recv_fd_);
  close(epoll_fd_);
}

// TODO: understand this and add comments
void Network::InitializeEpoll() {
  if ((epoll_fd_ = epoll_create1(0)) < 0) {
    perror("Error: epoll_create1 failed");
    exit(EXIT_FAILURE);
  }

  event_.events = EPOLLIN;
  event_.data.fd = 0;

  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, 0, &event_) < 0) {
    perror("Error: failed to add file descriptor 0 to epoll");
    exit(EXIT_FAILURE);
  }

  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, recv_fd_, &event_) < 0) {
    perror("Error: failed to add file descriptor 0 to epoll");
    exit(EXIT_FAILURE);
  }
}

void Network::PollEvents() {
  int event_count = epoll_wait(epoll_fd, events, kMaxEventsNum, -1);
  if (event_count < 0) {
    perror("Error: epoll_wait failed");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < event_count; ++i) {
    if (events[i].data.fd == recv_fd_) {
      
    } else if (events[i].data.fd == 0) {

    } else {

    }
  }
}

void Network::EstablishReceiver() {
  // socket(domain, type, protocol)                                                   
  if ((recv_fd_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {                              
    perror("Error: cannot create socket");                                            
    exit(EXIT_FAILURE);                                                               
  }                                                                                   
  struct sockaddr_in my_address;                                                      
  memset(&my_address, 0, sizeof(my_address));                                         
  my_address.sin_family = AF_INET;                                                    
  //                                                                                  
  my_address.sin_addr.s_addr = htonl(INADDR_ANY);                                     
  // htons: host to network - short: convert a number into a 16-bit network           
  // representation. This is commonly used to store a port number into a              
  // sockaddr structure                                                               
  my_address.sin_port = htons(my_port_);                                                 
                                                                                      
  if (bind(recv_fd_, (struct sockaddr *)&my_address, sizeof(my_address)) < 0) {       
    perror("Error: cannot bind");                                                     
    exit(EXIT_FAILURE);                                                               
  }                                                                                   
}

void Network::BroadcastMessage(const std::string &message) {
  for (int peer_port: peers_) {
    SendMessage(peer_port, message);
  }
}

void Network::SendMessage(int target_port, const std::string &message) {
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

  if (sendto(fd, message.c_str(), message.size(), 0,
             (struct sockaddr *)&taget_address, sizeof(taget_address)) < 0) {
    perror("Error: SendMessage sendto failed");
    exit(EXIT_FAILURE);
  }

  // DEBUG
  //std::cout << "Message " << message << " sent to " << target_port << std::endl;
  close(fd);
}

}
