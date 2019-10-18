#include "sequence_table.h"

#include <sstream>

namespace yela {

SequenceTable::SequenceTable(const std::string &table_string) {
  std::istringstream iss(table_string);
  std::string word;

  while (getline(iss, word, ' ')) {
    int sep_index = word.find(':');
    std::string key = word.substr(0, sep_index);
    std::string val = word.substr(sep_index + 1, word.size() - sep_index - 1);

    table_[key] = std::stoi(val);
  }
}

void SequenceTable::Set(const Id &id, int sequence_num) {
  std::lock_guard<std::mutex> lock(mutex_);
  table_[id] = sequence_num;
}

void SequenceTable::Increment(const Id &id) {
  std::lock_guard<std::mutex> lock(mutex_);
  ++table_[id];
}

void SequenceTable::SetIfAbsent(const Id &id, int sequence_num) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (table_.find(id) == table_.end()) {
    table_[id] = sequence_num;
  }
}

int SequenceTable::Get(const Id &id) const {
  std::lock_guard<std::mutex> lock(mutex_);
  auto p = table_.find(id);
  return p == table_.end() ? -1 : p->second;
}

std::string SequenceTable::ToString() const {
  std::lock_guard<std::mutex> lock(mutex_);

  // TODO: it is better to let cereal handle the serialization
  std::string serialized;
  for (auto p = table_.begin(); p != table_.end(); ++p) {
    // WARNING: if id contains ':', this code will be wrong
    serialized += p->first + ":" + std::to_string(p->second) + " ";
  }
  return serialized;
}

}
