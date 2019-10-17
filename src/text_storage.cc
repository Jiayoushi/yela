#include "text_storage.h"

#include "log.h"

namespace yela {

void TextStorage::Put(const Id &id, const SequenceNumber &seq_num, 
                      const std::string &content, const long timestamp) {
  storage_[id][seq_num] = Chat(content, timestamp);                                 
}

Chat TextStorage::Get(const Id &id, const SequenceNumber &seq_num) {
  auto p = storage_.find(id);                                                       
  if (p == storage_.end()) {
    std::string msg = "ERROR: message storage failed to map id: " + id +            
       " with sequence number " + std::to_string(seq_num);                            
    Log(msg);
    return Chat();                                                                  
  }                                                                                 
    
  auto x = p->second.find(seq_num);                                                 
  if (x == p->second.end()) {
    std::string msg = "ERROR: message storage failed to map sequence number: "      
        + std::to_string(seq_num) + " from id: " + id;                                
    Log(msg);
    return Chat();                                                                  
  }                                                                                 
    
  return storage_[id][seq_num];                                                     
}

}
