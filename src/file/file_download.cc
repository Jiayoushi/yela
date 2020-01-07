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

  while (isspace(target_sha1.back())) {
    target_sha1.pop_back();
  }

  if (target_node_id.size() == 0) {
    Log("Error: incomplete input to download a file");
    return;                                                                           
  }                                                                                   
                                                                                      
  Message msg;                                                                        
  msg["id"] = network_->GetOrigin();                                                      
  msg["type"] = kTypes[kBlockRequest];                                                
  msg["dest"] = target_node_id;                                                       
  msg["blockrequest"] = target_sha1;
  msg["hoplimit"] = std::to_string(kBlockRequestHopLimit);

  // Just send it and assumes it reaches
  for (int i = 0; i < 10; ++i) // TODO: change this
  network_->SendMessageToRandomPeer(msg);
}

void DownloadManager::HandleBlockRequest(const Message &request) {
  // I am not the target, relay or don't send it
  if (request["dest"] != network_->GetOrigin()) {
    RelayMessage(request);
    return;
  }

  std::string sha1 = request["blockrequest"];
  int file_info_index = 0;
  if ((file_info_index = upload_manager_->HasFile(sha1)) < 0) {
    // TODO:
    // Or maybe should tell the requester the sha1 does not exist?
    Log("No such file exist with sha1: " + sha1 + ";");
    return;
  }

  const FileInfo &file_info = upload_manager_->GetFile(file_info_index);
  Message reply;
  reply["id"] = network_->GetOrigin();
  reply["type"] = kTypes[kBlockReply];
  reply["dest"] = request["id"];
  reply["blockreply"] = sha1;
  reply["hoplimit"] = std::to_string(kBlockReplyHopLimit);
    
  reply["data"] = file_info.content[0];

  for (int i = 0; i < 10; ++i) // TODO: change this
  network_->SendMessageToRandomPeer(reply);
}

// Get the downloaded file put it to local
void DownloadManager::HandleBlockReply(const Message &reply) {
  if (reply["dest"] != network_->GetOrigin()) {
    RelayMessage(reply);
    return;
  }

  if (upload_manager_->CheckSha1(reply["data"], reply["blockreply"])) {
    WriteToFile(reply["data"], reply["blockreply"]);
    Log("File written to current directory with sha1: " + reply["blockreply"]);
  } else {
    Log("Unmatched file sha1");
  }
}

void DownloadManager::WriteToFile(const std::string &file_content,
                                  const std::string &filename) {
  std::ofstream out(filename);
  out.write(file_content.c_str(), file_content.size());
  out.close();
}

void DownloadManager::RelayMessage(const Message &msg) {
  int hoplimit = std::stoi(msg["hoplimit"]);
  if (--hoplimit >= 0) {
    Message relay_msg = msg;
    relay_msg["hoplimit"] = std::to_string(hoplimit);
    network_->SendMessageToRandomPeer(relay_msg);
  }
}

void DownloadManager::RegisterNetwork(std::shared_ptr<Network> network) {
  network_ = network;
}

void DownloadManager::RegisterUploadManager(std::shared_ptr<UploadManager> upload_manager) {
  upload_manager_= upload_manager;
}

}
