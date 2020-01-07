#include "message.h"

namespace yela {

Message::Message() {

}

Message::~Message() {

}

Message::Message(const Origin &origin, const SequenceNumber &seq_num,
  const std::string &data, const long timestamp) {
  ConstructRumorMessage(origin, seq_num, data, timestamp);
}

Message::Message(const Origin &origin, const SequenceTable &table) {
  ConstructStatusMessage(origin, table);
}

void Message::ConstructRumorMessage(const Origin &origin, const SequenceNumber &seq_num,
                                    const std::string &data, const long timestamp) {
  kv_map_["type"] = kTypes[kRumor];
  kv_map_["origin"] = origin;
  kv_map_["seqnum"] = std::to_string(seq_num);
  kv_map_["data"] = data;
  kv_map_["timestamp"] = std::to_string(timestamp);
}

void Message::ConstructStatusMessage(const Origin &origin, const SequenceTable &table) {
  kv_map_["type"] = kTypes[kStatus];
  kv_map_["origin"] = origin;
  kv_map_["seqtable"] = table.ToString();
}

std::string & Message::operator[](const std::string &key) {
  return kv_map_[key];
}

const std::string & Message::operator[](const std::string &key) const {
  if (kv_map_.find(key) == kv_map_.end()) {
    return kEmptyString;
  }

  return kv_map_.at(key);
}

}
