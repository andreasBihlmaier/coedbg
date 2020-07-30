#ifndef OD_OBJECT_H_
#define OD_OBJECT_H_

#include <map>
#include <string>

#include "od_entry.h"

namespace coe {

class OdObject {
 private:
  std::map<uint8_t, OdEntry> m_entries;

 public:
  std::string to_string() const;
  OdEntry& operator[](const uint8_t& key) {
    return m_entries[key];
  }
  std::map<uint8_t, OdEntry>::iterator find(const uint8_t& key) {
    return m_entries.find(key);
  }
  std::map<uint8_t, OdEntry>::iterator begin() {
    return m_entries.begin();
  }
  std::map<uint8_t, OdEntry>::iterator end() {
    return m_entries.end();
  }
};

}  // namespace coe

#endif  // OD_OBJECT_H_