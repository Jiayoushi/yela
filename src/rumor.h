#ifndef YELA_RUMOR_H_
#define YELA_RUMOR_H_

#include <unordered_map>
#include <string>

#include "text_storage.h"
#include "network.h"
#include "interface.h"
#include "sequence_table.h"
#include "base/stoppable.h"
#include "base/blocking_queue.h"

namespace yela {

class Rumor: public Stoppable {
 public:
  Rumor() = default;
  ~Rumor();
  void RegisterNetwork(std::shared_ptr<Network> network);
  void RegisterInterface(std::shared_ptr<Interface> interface);

  void PushLocalInput(const Input &input);
  void PushRemoteMessage(const Message &msg);

  void Run();
 private:
  void ProcessOneMessage();

  void HandleStatusMessage(const Message &msg);
  void HandleRumorMessage(const Message &msg);

  void RelayMessage(const Message &msg);
  void AcknowledgeMessage(const Origin &origin);
  void SendTableToRandomPeer();

  const int kPeriodInMs = 2000;

  // Sequence number of each rumor message
  const int kInitialSequenceNumber = 1;
  SequenceTable seq_num_table_;

  // Store rumor message 
  void InsertNewRumorMessage(const Message &msg);
  TextStorage text_storage_;

  // Including both remote and local
  BlockingQueue<Message> msg_queue_;

  std::unordered_map<Origin, std::shared_ptr<Message>> exchanges_;

  // Other components
  std::shared_ptr<Network> network_;
  std::shared_ptr<Interface> interface_;
};


}

#endif
