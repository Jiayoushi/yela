#include "file_manager.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <chrono>

namespace yela {

FileManager::FileManager():
  upload_manager_(std::make_shared<UploadManager>()),
  search_manager_(std::make_shared<SearchManager>()),
  download_manager_(std::make_shared<DownloadManager>()) {

  download_manager_->RegisterUploadManager(upload_manager_);
  search_manager_->RegisterUploadManager(upload_manager_);

  Log("File Manager component initialized.");
}

FileManager::~FileManager() {
  Log("File Manager terminted");
}


// TODO: do i need this?
void FileManager::Run() {
  while (!StopRequested()) {

  }
  
  download_manager_->Stop();
  search_manager_->Stop();
}

void FileManager::HandleLocalRequest(const Input &input) {
  if (input.mode == kUpload) {
    Upload(input.content);
  } else if (input.mode == kDownload) {                                             
    Download(input.content);                                         
  } else if (input.mode == kSearch) {                                               
    Search(input.content);
  }
}

void FileManager::HandleRemoteMessage(const Message &msg) {
  if (msg["type"] == kTypes[kBlockRequest]) {
    HandleBlockRequest(msg);
  } else if (msg["type"] == kTypes[kBlockReply]) {
    HandleBlockReply(msg);                                             
  } else if (msg["type"] == kTypes[kSearchRequest]) {
    HandleSearchRequest(msg);                                          
  } else if (msg["type"] == kTypes[kSearchReply]) {                                   
    HandleSearchReply(msg);                                            
  } else {
    Log("Unmatched file type " + msg["type"]);                                        
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

void FileManager::Upload(const std::string &filename) {
  const std::string metafile = upload_manager_->Upload(filename);

  if (metafile.size() > 0) {
    interface_->PrintToSystemWindow(filename + " uploade with sha1:"  + metafile);
  } else {
    interface_->PrintToSystemWindow("File '" + filename + "' is not found.");
  }
}

}
