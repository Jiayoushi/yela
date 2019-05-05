#ifndef YELA_INTERFACE_H_
#define YELA_INTERFACE_H_

#include <string>
#include <vector>
#include <iostream>


namespace yela {

struct Message {
 public:
  Message(const std::string &ip, const std::string &port, const std::string &d):
    sender_ip(ip), sender_port(port), data(d) {
  }

  std::string sender_ip;
  std::string sender_port;
  std::string data;
};

class Interface {
 public:
  Interface();
  ~Interface();

 protected:
  bool terminate;
  // User Interaction
  const std::string kPrompt = "\n\n\n\n\n\n\n\n\n\n\nEnter a message:\n";
  
  void ClearScreen();
  void PrintPrompt();
  std::string ReadInput();

  std::vector<Message> history_;
  void Insert(const std::string &ip, const std::string &port, 
              const std::string &data);
  void PrintHistory();
};

}

#endif
