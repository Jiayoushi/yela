#include "file_search.h"

namespace yela {

SearchManager::SearchManager() {

}

SearchManager::~SearchManager() {

}

void SearchManager::Run() {

}

void SearchManager::Search(const std::string &filename) {
  Message msg;

  msg["id"] = network_->GetOrigin();
  msg["type"] = kTypes[kSearchRequest];
  msg["search"] = filename;

  // TODO: doubling budget each time
  for (int i = 1; i < 10; ++i) {
    msg["budget"] = std::to_string(kDefaultSearchRequestBudget * i);
    network_->SendMessageToRandomPeer(msg);
  }
}

void SearchManager::HandleSearchRequest(const Message &request) {
  Log("Handle search request A");
  const std::string &filename = request["search"];
  Log("Handle search request A.5");
  int file_info_index = upload_manager_->FindFileByName(filename);
  Log("handle search request A.6");
  if (file_info_index == -1) {
    RelayMessage(request);
    return;
  }

  Log("Handle Search request B");
  const FileInfo &file_info = upload_manager_->GetFile(file_info_index);
  Message reply;
  reply["id"] = network_->GetOrigin();
  reply["type"] = kTypes[kSearchReply];
  reply["dest"] = request["id"];
  reply["search"] = request["search"];
  reply["sha1"] = file_info.metafile;
  reply["hoplimit"] = std::to_string(kDefaultSearchReplyBudget);

  Log("Handle search request C");
  // TODO: Actually, if this message got lost, just wait for another request
  for (int i = 0; i < 10; ++i) // TODO: change this
  network_->SendMessageToRandomPeer(reply);
}

std::string SearchManager::HandleSearchReply(const Message &reply) {
  if (reply["dest"] != network_->GetOrigin()) {
    RelayMessage(reply);
    return "";
  }

  return reply["search"] + " from: " + reply["id"] + " sha1: " + reply["sha1"];
}

void SearchManager::RegisterNetwork(std::shared_ptr<Network> network) {
  network_ = network;
}

void SearchManager::RegisterUploadManager(std::shared_ptr<UploadManager> upload_manager) {
  upload_manager_ = upload_manager;
}

void SearchManager::RelayMessage(const Message &msg) {
  int budget = std::stoi(msg["budget"]);
  if (--budget >= 0) {
    Message relay_msg = msg;
    relay_msg["budget"] = std::to_string(budget);
    network_->SendMessageToRandomPeer(relay_msg);
  }
}

}
