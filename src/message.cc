#include "message.h"

#include "log.h"

namespace yela {

Message::Message() {

}

Message::Message(const Id &id, const SequenceNumber &seq_num,
  const std::string &data, const long timestamp) {
  ConstructRumorMessage(id, seq_num, data, timestamp);
}

Message::Message(const Id &id, const SequenceNumberTable &table) {
  ConstructStatusMessage(id, table);
}

void Message::ConstructRumorMessage(const Id &id, const SequenceNumber &seq_num,
  const std::string &data, const long timestamp) {
  kv_map_["type"] = kTypes[kRumor];
  kv_map_["id"] = id;
  kv_map_["seqnum"] = std::to_string(seq_num);
  kv_map_["data"] = data;
  kv_map_["timestamp"] = std::to_string(timestamp);
}

void Message::ConstructStatusMessage(const Id &id, const SequenceNumberTable &table) {
  kv_map_["type"] = kTypes[kStatus];
  kv_map_["id"] = id;
  kv_map_["seqtable"] = SeqTableToString(table);
}

std::string & Message::operator[](const std::string &key) {
  return kv_map_[key];
}

const std::string & Message::operator[](const std::string &key) const {
  if (kv_map_.find(key) == kv_map_.end()) {
    Log("Unmatched key: '" + key + "' to access message map");
    return kEmptyString;
  }

  return kv_map_.at(key);
}

SequenceNumberTable Message::Deserialize(const std::string &table) {
  SequenceNumberTable t;
  std::istringstream iss(table);
  std::string word;

  while (getline(iss, word, ' ')) {
    int sep_index = word.find(':');
    std::string key = word.substr(0, sep_index);
    std::string val = word.substr(sep_index + 1, word.size() - sep_index - 1);

    t[key] = std::stoi(val);
  }

  return t;
}

std::string Message::SeqTableToString(const SequenceNumberTable &table) {
  // TODO: it is better to let cereal handle the serialization
  std::string serialized;
  for (auto p = table.begin(); p != table.end(); ++p) {
    if (p->first.size() == 0) {
      Log("ERROR: sequence number table entry is not correct: id is not set");
    }
    // WARNING: if id contains ':', this code will be wrong
    serialized += p->first + ":" + std::to_string(p->second) + " ";
  }
  return serialized;
}

}
