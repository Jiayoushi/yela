#include "file_manager.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <chrono>

#include "../global.h"

namespace yela {

FileManager::FileManager():
  upload_manager_(std::make_shared<UploadManager>()),
  search_manager_(std::make_shared<SearchManager>()),
  download_manager_(std::make_shared<DownloadManager>()) {

  download_manager_->RegisterUploadManager(upload_manager_);

  Log("File Manager component initialized.");
}

FileManager::~FileManager() {

}

void FileManager::Run() {
  while (true) {
    // Should instead sleep
    if (StopRequested()) {
      download_manager_->Stop();
      search_manager_->Stop();
      break;
    }
  }
}

// TODO: gotta have a better way to handle this
// TODO: global message queue for network
void FileManager::RegisterNetwork(std::shared_ptr<Network> network) {
  network_ = network;
  
  upload_manager_->RegisterNetwork(network);
  search_manager_->RegisterNetwork(network);
  download_manager_->RegisterNetwork(network);
}

void FileManager::RegisterInterface(std::shared_ptr<Interface> interface) {
  interface_ = interface;
}

void FileManager::Search(const std::string &input) {
  search_manager_->Search(input);
}

void FileManager::Download(const std::string &input) {
  download_manager_->Download(input);
}

void FileManager::HandleBlockRequest(const Message &msg) {
  download_manager_->HandleBlockRequest(msg);
}

void FileManager::HandleBlockReply(const Message &msg) {
  download_manager_->HandleBlockReply(msg);
}

void FileManager::HandleSearchRequest(const Message &msg) {
  search_manager_->HandleSearchRequest(msg);
}

void FileManager::HandleSearchReply(const Message &msg) {
  std::string display_msg = search_manager_->HandleSearchReply(msg);
  if (!display_msg.empty()) {
    interface_->PrintToSystemWindow(display_msg);
  }
}

std::string FileManager::Upload(const std::string &input) {
  return upload_manager_->Upload(input);
}

}
