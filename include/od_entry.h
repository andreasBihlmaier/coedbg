#ifndef OD_ENTRY_H_
#define OD_ENTRY_H_

#include <map>
#include <string>

namespace coe {

class OdEntry {
 public:
  uint16_t index;
  uint8_t subindex;
  std::string name;
  std::string type_name;
  // TODO type unify with CoeField::m_type
  // TODO value unify with CoeField::m_value
  std::string default_data;

  OdEntry() : index(0), subindex(0) {
  }
  std::string to_string() const;
};

}  // namespace coe

#endif  // OD_ENTRY_H_