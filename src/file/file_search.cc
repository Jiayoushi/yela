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

  msg["id"] = network_->GetId();
  msg["type"] = kTypes[kSearchRequest];
  msg["search"] = filename;

  // TODO: doubling budget each time
  for (int i = 1; i < 10; ++i) {
    msg["budget"] = kDefaultSearchRequestBudget * i;
    network_->SendMessageToRandomPeer(msg);
  }
}

void SearchManager::HandleSearchRequest(const Message &request) {
  const std::string &filename = request["search"];
  int file_info_index = upload_manager_->FindFileByName(filename);
  if (file_info_index == -1) {
    RelayMessage(request);
    return;
  }

  const FileInfo &file_info = upload_manager_->GetFile(file_info_index);
  Message reply;
  reply["id"] = network_->GetId();
  reply["type"] = kTypes[kSearchReply];
  reply["dest"] = request["id"];
  reply["search"] = request["search"];
  reply["sha1"] = file_info.metafile;
  reply["hoplimit"] = std::to_string(kDefaultSearchReplyBudget);

  // TODO: Actually, if this message got lost, just wait for another request
  for (int i = 0; i < 10; ++i) // TODO: change this
  network_->SendMessageToRandomPeer(reply);
}

std::string SearchManager::HandleSearchReply(const Message &reply) {
  if (reply["dest"] != network_->GetId()) {
    RelayMessage(reply);
    return "";
  }

  return reply["search"] + " matching sha1: " + reply["sha1"];
}

void SearchManager::RegisterNetwork(std::shared_ptr<Network> network) {
  network_ = network;
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
