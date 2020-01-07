#include "text_storage.h"

#include "log.h"

namespace yela {

void TextStorage::Put(const Origin &origin, const SequenceNumber &seq_num, 
                      const std::string &content, const long timestamp) {
  storage_[origin][seq_num] = Chat(content, timestamp);                                 
}

Chat TextStorage::Get(const Origin &origin, const SequenceNumber &seq_num) {
  auto p = storage_.find(origin);                                                       
  if (p == storage_.end()) {
    std::string msg = "ERROR: message storage failed to map origin: " + origin +            
       " with sequence number " + std::to_string(seq_num);                            
    Log(msg);
    return Chat();                                                                  
  }                                                                                 
    
  auto x = p->second.find(seq_num);                                                 
  if (x == p->second.end()) {
    std::string msg = "ERROR: message storage failed to map sequence number: "      
        + std::to_string(seq_num) + " from origin: " + origin;                                
    Log(msg);
    return Chat();                                                                  
  }                                                                                 
    
  return storage_[origin][seq_num];                                                     
}

}
