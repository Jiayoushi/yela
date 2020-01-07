#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <queue>
#include <thread>
#include <iostream>

#include "file/file_manager.h"
#include "rumor.h"
#include "network.h"
#include "interface.h"
#include "text_storage.h"
#include "base/stoppable.h"

namespace yela {

class Node: public Stoppable {
 public:
  Node();
  ~Node();

  void RegisterNetwork(std::shared_ptr<Network> network);
  void RegisterInterface(std::shared_ptr<Interface> interface);

  void Run();
 private:
  // Components
  std::shared_ptr<Interface> interface_;
  std::shared_ptr<Network> network_;

  // Information about this node
  Origin origin_;

  void PollEvents();

  std::shared_ptr<FileManager> file_manager_;
  std::shared_ptr<Rumor> rumor_;
  void RunRumor();
  void RunFileSharing();

  void HandleMessageFromPeer();
  void DispatchRemoteMessageToHandler(const Message &msg, sockaddr_in &peer_addr);
  void HandleLocalHostInput();
};

}
