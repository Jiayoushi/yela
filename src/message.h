#ifndef YELA_MESSAGE_H_
#define YELA_MESSAGE_H_

#include <unordered_map>
#include <string>

#include "cereal/types/string.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/types/concepts/pair_associative_container.hpp"

namespace yela {

typedef std::string Data;
typedef std::string Id;
typedef int SequenceNumber;
typedef std::unordered_map<Id, SequenceNumber> SequenceNumberTable;

const std::vector<std::string> kTypes = {
  "Rumor", "Status", "BlockRequest", "BlockReply"
};
enum kTypeKeys {
  kRumor = 0, kStatus, kBlockRequest, kBlockReply
};

// [Access Key]
// 
// For basic message:
//  type
//  id
//  seqnum
//  data
//  seqtable
//  timestamp
//
// For files sharing:
//  dest
//  sha1
class Message {
 public:
  const static size_t kMaxMessageSize = 1024;
  const std::string kEmptyString = "";

  Message();
  Message(const Id &id, const SequenceNumber &seq_num, const std::string &data, 
          const long timestamp);
  Message(const Id &id, const SequenceNumberTable &table);

  std::string & operator[](const std::string &key);
  const std::string & operator[](const std::string &key) const;
  static SequenceNumberTable Deserialize(const std::string &table);

  template<typename Archive>
  void serialize(Archive &archive) {
    archive(kv_map_);
  }
 private:
  std::unordered_map<std::string, std::string> kv_map_;

  std::string SeqTableToString(const SequenceNumberTable &table);

  void ConstructRumorMessage(const Id &id, const SequenceNumber &seq_num,
                             const std::string &data, const long timestamp);
  void ConstructStatusMessage(const Id &id, const SequenceNumberTable &table);
};

}

#endif
