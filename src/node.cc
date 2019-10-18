#ifndef YELA_NODE_H_
#define YELA_NODE_H_

#include "node.h"

#include <cassert>
#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>

namespace yela {

Node::Node() {
  Log("Node component initialized.");

  file_manager_ = std::make_shared<FileManager>();
  rumor_ = std::make_shared<Rumor>();
}

Node::~Node() {
  Log("Node component terminated.");
  // TODO: other processes should handle this.
  // CloseLop should be handled by a Logger class
  // dialogue should handled by interface itself.
  CloseLog();
  interface_->WriteDialogueToFile(network_->GetId());
}

void Node::RegisterNetwork(std::shared_ptr<Network> network) {
  network_ = network;
  rumor_->RegisterNetwork(network);
  file_manager_->RegisterNetwork(network);
}

void Node::RegisterInterface(std::shared_ptr<Interface> interface) {
  interface_ = interface;
  rumor_->RegisterInterface(interface);
  file_manager_->RegisterInterface(interface);
}

// TODO: network module should handle this
void Node::PollEvents() {
  int event_count = epoll_wait(network_->epoll_fd_, network_->events_, 
                              network_->kMaxEventsNum, network_->kEpollFrequencyInMs);
  if (event_count < 0) {
    if (errno == EINTR) {
      return;
    }

    Log("Error: epoll_wait failed");                                               
    exit(EXIT_FAILURE);
  } else {
    // Check if there is any local user input
    if (interface_->local_inputs_.size() > 0) {
      HandleLocalHostInput();
    }
    // If event_count != 0, process registerd events
    for (int i = 0; i < event_count; ++i) {
      if (network_->events_[i].data.fd == network_->listen_fd_) {
        HandleMessageFromPeer();
      } else {
        Log("Unmatched event in epoll");
      }
    }
  }
}

void Node::HandleMessageFromPeer() {
  // Read from raw sockets
  struct sockaddr_in peer_addr;
  socklen_t addrlen = sizeof(peer_addr);
  char buf[Message::kMaxMessageSize + 1];
  int len = recvfrom(network_->listen_fd_, buf, Message::kMaxMessageSize, 0,
              (struct sockaddr *)&peer_addr, &addrlen);
  if (len < 0) {
    perror("recvfrom failed");
    return;
  }

  // Parse
  Message msg = network_->ParseMessage(buf, len);

  // Register a remote node's network information
  if (!network_->IsKnownPeer(msg["id"])) {
    network_->InsertPeer(msg["id"], peer_addr);
  }

  // Dispatch
  DispatchRemoteMessageToHandler(msg, peer_addr);
}

void Node::DispatchRemoteMessageToHandler(const Message &msg, sockaddr_in &peer_addr) {
  // Rumor and Status message
  if (msg["type"] == kTypes[kRumor] || msg["type"] == kTypes[kStatus]) {
    rumor_->PushRemoteMessage(msg);
  } else {
    file_manager_->HandleRemoteMessage(msg);
  }
}

// Read user input and send to random peer
void Node::HandleLocalHostInput() {
  interface_->local_inputs_mutex_.lock();

  while (interface_->local_inputs_.size() != 0) {
    const Input &input = interface_->local_inputs_.front();

    if (input.mode == kChat) {
      rumor_->PushLocalInput(input);
    } else if (input.mode == kUpload   || 
               input.mode == kDownload || 
               input.mode == kSearch) {
      file_manager_->HandleLocalRequest(input);
    } else {
      Log("WARNING: current input mode " + std::to_string(input.mode) + 
          " is not matched to any of the mode");
    }

    interface_->local_inputs_.pop();
  }

  interface_->local_inputs_mutex_.unlock();
}

void Node::RunRumor() {
  rumor_->Run();
}

void Node::RunFileSharing() {
  file_manager_->Run();  
}

void Node::Run() {
  std::thread rumor_thread(&Node::RunRumor, this);
  std::thread file_thread(&Node::RunFileSharing, this);

  while (!StopRequested()) {
    PollEvents();
  }

  rumor_->Stop();
  file_manager_->Stop();

  rumor_thread.join();
  file_thread.join();
}

}

#endif
