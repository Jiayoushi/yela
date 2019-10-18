#ifndef YELA_SEQUENCE_TABLE_H_
#define YELA_SEQUENCE_TABLE_H_

#include <unordered_map>
#include <string>
#include <mutex>

namespace yela {

typedef std::string Id;
typedef int SequenceNumber;

class SequenceTable {
 public:
  SequenceTable() = default;
  SequenceTable(const std::string &table_string);

  void Set(const Id &id, int sequence_num);
  int Get(const Id &id) const;
  void SetIfAbsent(const Id &id, int sequence_num);
  void Increment(const Id &id);

  std::string ToString() const;

  std::unordered_map<Id, SequenceNumber>::const_iterator cbegin() const { return table_.begin(); }
  std::unordered_map<Id, SequenceNumber>::const_iterator cend() const { return table_.end(); }
 private:
  mutable std::mutex mutex_;
  std::unordered_map<Id, SequenceNumber> table_; 
};


}

#endif
