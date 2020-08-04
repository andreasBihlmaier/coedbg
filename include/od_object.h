#ifndef OD_OBJECT_H_
#define OD_OBJECT_H_

#include <map>
#include <string>

#include "od_entry.h"

namespace coe {

class OdObject {
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
  const std::map<uint8_t, OdEntry>::const_iterator begin() const {
    return m_entries.begin();
  }
  std::map<uint8_t, OdEntry>::iterator end() {
    return m_entries.end();
  }
  const std::map<uint8_t, OdEntry>::const_iterator end() const {
    return m_entries.end();
  }
  void set_complete_value(const VariantValue& new_complete_value);

 private:
  std::map<uint8_t, OdEntry> m_entries;
};

}  // namespace coe

#endif  // OD_OBJECT_H_