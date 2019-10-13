#ifndef YELA_FILE_SEARCH_H_
#define YELA_FILE_SEARCH_H_

#include "../global.h"
#include "../log.h"
#include "../network.h"
#include "../base/stoppable.h"

#include "file_upload.h"

namespace yela {

class SearchManager: public Stoppable {
 public:
  SearchManager();
  ~SearchManager();

  void Run();
  void Search(const std::string &input);
  void HandleSearchRequest(const Message &msg);
  std::string HandleSearchReply(const Message &msg);

  // TODO: maybe these stuff should provide messages queues
  //       and all instance just call a global function to push message to
  //       the message queue
  void RegisterNetwork(std::shared_ptr<Network> network);
 private:
  const int kDefaultSearchRequestBudget = 2;
  const int kDefaultSearchReplyBudget = 100;

  void RelayMessage(const Message &msg);

  std::shared_ptr<UploadManager> upload_manager_;
  std::shared_ptr<Network> network_;
};

}

#endif
