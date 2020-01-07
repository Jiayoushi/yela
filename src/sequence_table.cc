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

void SequenceTable::Set(const Origin &origin, int sequence_num) {
  std::lock_guard<std::mutex> lock(mutex_);
  table_[origin] = sequence_num;
}

void SequenceTable::Increment(const Origin &origin) {
  std::lock_guard<std::mutex> lock(mutex_);
  ++table_[origin];
}

void SequenceTable::SetIfAbsent(const Origin &origin, int sequence_num) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (table_.find(origin) == table_.end()) {
    table_[origin] = sequence_num;
  }
}

int SequenceTable::Get(const Origin &origin) const {
  std::lock_guard<std::mutex> lock(mutex_);
  auto p = table_.find(origin);
  return p == table_.end() ? -1 : p->second;
}

std::string SequenceTable::ToString() const {
  std::lock_guard<std::mutex> lock(mutex_);

  // TODO: it is better to let cereal handle the serialization
  std::string serialized;
  for (auto p = table_.begin(); p != table_.end(); ++p) {
    // WARNING: if origin contains ':', this code will be wrong
    serialized += p->first + ":" + std::to_string(p->second) + " ";
  }
  return serialized;
}

}
