#ifndef YELA_TEXT_STORAGE_H_
#define YELA_TEXT_STORAGE_H_

#include <unordered_map>
#include <string>

#include "network.h"
#include "interface.h"

namespace yela {

// Store the rumor message's content
class TextStorage {                                                                   
 public:
  void Put(const Id &id, const SequenceNumber &seq_num, 
           const std::string &content, const long timestamp);

  Chat Get(const Id &id, const SequenceNumber &seq_num);
 
 private:
  std::unordered_map<Id, std::unordered_map<SequenceNumber, Chat>> storage_;          
};


}

#endif
