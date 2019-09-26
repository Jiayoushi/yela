#include "file_download.h"

namespace yela {

DownloadManager::DownloadManager() {

}

DownloadManager::~DownloadManager() {

}

void DownloadManager::Run() {
  
}

void DownloadManager::Download(const std::string &input) {
  std::istringstream ss(input);
  std::string target_node_id;
  std::string target_sha1;

  ss >> target_node_id;
  ss >> target_sha1;

  if (target_node_id.size() == 0) {
    Log("Error: incomplete input to download a file");
    return;                                                                           
  }                                                                                   
                                                                                      
  Message msg;                                                                        
  msg["id"] = network_->GetId();                                                      
  msg["type"] = kTypes[kBlockRequest];                                                
  msg["dest"] = target_node_id;                                                       
  msg["hash"] = target_sha1;                                                          
  msg["hoplimit"] = std::to_string(kDownloadFileHopLimit);
}

void DownloadManager::HandleBlockRequest(const Message &request) {

}

void DownloadManager::HandleBlockReply(const Message &reply) {

}

void DownloadManager::RegisterNetwork(std::shared_ptr<Network> network) {
  network_ = network;
}

void DownloadManager::RegisterUploadManager(std::shared_ptr<UploadManager> upload_manager) {
  upload_manager_= upload_manager;
}

}
