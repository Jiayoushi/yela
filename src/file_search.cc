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
  msg["budget"] = kBudgetPerMessage;
}

void SearchManager::HandleSearchRequest(const Message &msg) {

}

void SearchManager::HandleSearchReply(const Message &msg) {

}

void SearchManager::RegisterNetwork(std::shared_ptr<Network> network) {
  network_ = network;
}

}
