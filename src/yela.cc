#include "yela.h"

#include <string>

namespace yela {

Yela::Yela(const Arguments &arg) {
  // Network
  network_ = std::make_shared<yela::Network>(arg.settings_file);

  // Log
  std::string id = network_->GetId();
  InitLog(id);

  // Interface
  interface_ = std::make_shared<Interface>();

  // File sharing
  file_manager_ = std::make_shared<FileManager>();
  file_manager_->RegisterNetwork(network_);

  // Gossip
  node_ = std::make_shared<Node>();
  node_->RegisterNetwork(network_);
  node_->RegisterInterface(interface_);
  node_->RegisterFileManager(file_manager_);
}

Yela::~Yela() {

}

void Yela::RunNetwork() {
 // TODO: network should handle the epoll for Gossip
}

void Yela::RunGossip() {
  node_->Run();
}

void Yela::RunInterface() {
  interface_->Run();
}

void Yela::RunFileSharing() {
  file_manager_->Run();
}

void Yela::Run() {
  // Spawn threads to run node and file sharing
  background_threads_.push_back(std::thread(&Yela::RunGossip, this));
  background_threads_.push_back(std::thread(&Yela::RunFileSharing, this));

  // The main thread is handling the GUI
  // Right after the GUI exits, the whole program should exit
  RunInterface();

  // Stop
  file_manager_->Stop();
  node_->Stop();

  // Join
  for (int i = 0; i < background_threads_.size(); ++i) {
    background_threads_[i].join();
  }
}

}
