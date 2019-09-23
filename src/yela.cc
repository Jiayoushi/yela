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
  //for (int i = 0; i < threads_.size(); ++i) {
  //  background_threads_[i].join();
  //}
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

  // The main thread is handling the GUI
  // As long as the GUI exits, the whole program
  // should exit

  RunInterface();
}

}
