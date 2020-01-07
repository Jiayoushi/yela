#ifndef YELA_SEQUENCE_TABLE_H_
#define YELA_SEQUENCE_TABLE_H_

#include <unordered_map>
#include <string>
#include <mutex>

namespace yela {

typedef std::string Origin;
typedef int SequenceNumber;

class SequenceTable {
 public:
  SequenceTable() = default;
  SequenceTable(const std::string &table_string);

  void Set(const Origin &origin, int sequence_num);
  int Get(const Origin &origin) const;
  void SetIfAbsent(const Origin &origin, int sequence_num);
  void Increment(const Origin &origin);

  std::string ToString() const;

  std::unordered_map<Origin, SequenceNumber>::const_iterator cbegin() const { return table_.begin(); }
  std::unordered_map<Origin, SequenceNumber>::const_iterator cend() const { return table_.end(); }
 private:
  mutable std::mutex mutex_;
  std::unordered_map<Origin, SequenceNumber> table_; 
};


}

#endif
