#ifndef YELA_MESSAGE_H_
#define YELA_MESSAGE_H_

#include <unordered_map>
#include <string>

#include "sequence_table.h"
#include "cereal/types/string.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/types/concepts/pair_associative_container.hpp"

namespace yela {

typedef std::string Data;

const std::vector<std::string> kTypes = {
  "Rumor", 
  "Status", 
  "BlockRequest", 
  "BlockReply",
  "SearchRequest",
  "SearchReply"
};

enum kTypeKeys {
  kRumor          = 0, 
  kStatus         = 1,
  kBlockRequest   = 2, 
  kBlockReply     = 3,
  kSearchRequest  = 4,
  kSearchReply    = 5
};

// Attributes for different messages
//
// Common to all
//  origin
//  type
// Status
//  data
//  seqtable
// Rumor
//  seqnum
//  timestamp
// SearchRequest
//  origin
//  search
//  budget
// SearchReply
// BlockRequest
//  origin
//  type
//  dest
//  hash
//  hoplimit
// BlockReply
class Message {
 public:
  const static size_t kMaxMessageSize = 1024;
  const std::string kEmptyString = std::string();

  Message();
  Message(const Origin &origin, const SequenceNumber &seq_num, const std::string &data, 
          const long timestamp);
  Message(const Origin &origin, const SequenceTable &table);

  std::string & operator[](const std::string &key);
  const std::string & operator[](const std::string &key) const;

  ~Message();

  template<typename Archive>
  void serialize(Archive &archive) {
    archive(kv_map_);
  }
 private:
  std::unordered_map<std::string, std::string> kv_map_;

  void ConstructRumorMessage(const Origin &origin, const SequenceNumber &seq_num,
                             const std::string &data, const long timestamp);
  void ConstructStatusMessage(const Origin &origin, const SequenceTable &table);
};

}

#endif
