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

  // Gossip
  node_ = std::make_shared<Node>();
  node_->RegisterNetwork(network_);
  node_->RegisterInterface(interface_);
}

Yela::~Yela() {

}

void Yela::RunGossip() {
  node_->Run();
}

void Yela::RunInterface() {
  interface_->Run();
}

void Yela::Run() {
  // Spawn threads to run node and file sharing
  background_threads_.push_back(std::thread(&Yela::RunGossip, this));

  // The main thread is handling the GUI
  // Right after the GUI exits, the whole program should exit
  RunInterface();

  // Stop
  node_->Stop();

  // Join
  for (int i = 0; i < background_threads_.size(); ++i) {
    background_threads_[i].join();
  }

  CloseLog();
  interface_->WriteDialogueToFile(network_->GetId());
}

}
