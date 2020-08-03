#ifndef OD_ENTRY_H_
#define OD_ENTRY_H_

#include <functional>
#include <string>
#include <vector>

#include "od_base_type.h"
#include "variant_value.h"

namespace coe {

class OdEntry {
 public:
  using ValueChangeCallback = std::function<void(const OdEntry& entry, const VariantValue& new_value)>;

  static std::string value_to_string(const VariantValue& value);

  uint16_t index;
  uint8_t subindex;
  std::string name;
  std::string type_name;
  VariantValue value;
  OdBaseType type;
  std::string default_data_string;
  uint32_t bit_size;
  ValueChangeCallback value_change_callback;

  OdEntry() : index(0), subindex(0), type(OdBaseType::Invalid), bit_size(0) {
  }
  std::string to_string() const;
  std::string value_to_string() const;
  void set_value_from_default_data();
  void set_value(const std::vector<uint8_t>& data);
  void set_value(const VariantValue& new_value);
};

}  // namespace coe

#endif  // OD_ENTRY_H_