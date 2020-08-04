#include "od_object.h"

#include <iostream>

namespace coe {

std::string OdObject::to_string() const {
  std::string str;
  str += "(";
  for (auto& entry_kv : m_entries) {
    str += entry_kv.second.to_string();
  }
  str += ")";
  return str;
}

void OdObject::set_complete_value(const VariantValue& new_complete_value) {
  const auto& data = boost::get<std::vector<uint8_t>>(new_complete_value);
  (void)data;
  uint16_t byte_offset = 0;
  for (auto& entry_kv : m_entries) {
    uint16_t entry_byte_length = entry_kv.second.bit_size / 8;
    std::vector<uint8_t> entry_data(&data[byte_offset], &data[byte_offset + entry_byte_length]);
    entry_kv.second.set_value(entry_data);
    if (entry_kv.second.subindex == 0) {  // special case: subindex 0 has size of 1 byte, but 16 bit offset
      byte_offset += 2;
    } else {
      byte_offset += entry_byte_length;
    }
  }
}

}  // namespace coe