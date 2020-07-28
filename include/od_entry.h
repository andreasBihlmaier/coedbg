#ifndef OD_ENTRY_H_
#define OD_ENTRY_H_

#include <string>

#include <boost/variant.hpp>

#include "od_base_type.h"

namespace coe {

class OdEntry {
 public:
  uint16_t index;
  uint8_t subindex;
  std::string name;
  std::string type_name;
  using value_t = boost::variant<bool, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t, float,
                                 double, std::string>;
  value_t value;
  OdBaseType type;
  std::string default_data_string;
  uint32_t bit_size;

  OdEntry() : index(0), subindex(0), type(OdBaseType::Invalid), bit_size(0) {
  }
  std::string to_string() const;
  void set_value_from_default_data();
};

}  // namespace coe

#endif  // OD_ENTRY_H_