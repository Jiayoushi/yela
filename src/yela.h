#ifndef YELA_YELA_H_
#define YELA_YELA_H_

#include <thread>
#include <memory>

#include "global.h"
#include "cmd.h"
#include "node.h"
#include "network.h"
#include "interface.h"

namespace yela {

class Yela {
 public:
  Yela(const Arguments &arg);
  ~Yela();

  void Run();
 private:
   void RunGossip();
   void RunNetwork();
   void RunInterface();

   std::shared_ptr<yela::Network> network_;
   std::shared_ptr<yela::Interface> interface_;
   //std::shared_ptr<yela::FileManager> file_manager_;
   std::shared_ptr<yela::Node> node_;

   std::vector<std::thread> background_threads_;
};

}

#endif
