#ifndef YELA_FILE_SEARCH_H_
#define YELA_FILE_SEARCH_H_

#include "global.h"
#include "log.h"
#include "base/stoppable.h"

namespace yela {

class SearchManager: public Stoppable {
 public:
  SearchManager();
  ~SearchManager();

  void Run();
  void Search(const std::string &input);
  void HandleSearchRequest(const Message &msg);
  void HandleSearchReply(const Message &msg);
  void RegisterNetwork(std::shared_ptr<Network> network);
 private:
  const int kBudgetPerMessage = 100;

  std::shared_ptr<Network> network_;
};

}

#endif
