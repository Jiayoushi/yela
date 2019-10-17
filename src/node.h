#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <queue>
#include <thread>
#include <iostream>

#include "file/file_manager.h"
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
  void RegisterFileManager(std::shared_ptr<FileManager> file_manager);

  void Run();
 private:
  // Components
  std::shared_ptr<Interface> interface_;
  std::shared_ptr<Network> network_;
  std::shared_ptr<FileManager> file_manager_;

  // Information about this node
  Id id_;

  void PollEvents();

  // Anti-entropy
  // Periodic send table
  const int kPeriodInMs = 2000;
  void SendTableToRandomPeer();
  bool running;

  // Sequence table
  const int kInitialSequenceNumber = 1;
  SequenceNumberTable seq_num_table_;

  // Store the messages this node has received
  TextStorage text_storage_;

  void AcknowledgeMessage(const Id &origin);
  void RelayMessage(const Message &msg);
  void InsertNewRumorMessage(const Message &msg);
  void HandleStatusMessage(const Message &msg);
  void HandleRumorMessage(const Message &msg, sockaddr_in &peer_addr);
  void HandleMessageFromPeer();
  void DispatchRemoteMessageToHandler(const Message &msg, sockaddr_in &peer_addr);
  void HandleLocalHostInput();
};
}
