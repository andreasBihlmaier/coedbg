#include "od_data_type.h"

namespace coe {

std::string OdDataType::to_string(bool print_subindex) const {
  std::string str;

  str += "(";
  if (print_subindex) {
    str += "subindex=" + std::to_string(subindex) + " ";
  }
  str += "name=" + name;
  str += " bit_size=" + std::to_string(bit_size);
  str += " bit_offset=" + std::to_string(bit_offset);
  str += " abstract_type=" + abstract_type;
  str += " base_type=" + od_base_type_name(base_type);
  if (!subindex_types.empty()) {
    str += " subindex_types=(";
    for (auto& subindex_type : subindex_types) {
      str += subindex_type.second.to_string(true);
    }
    str += ")";
  }
  str += ")";

  return str;
}

void OdDataType::add_subtype(const OdDataType& subtype) {
  if (subindex_types.find(subtype.subindex) != subindex_types.end()) {
    throw std::runtime_error("Type for subindex " + std::to_string(subtype.subindex) + " already defined");
  }
  subindex_types[subtype.subindex] = subtype;
}

}  // namespace coe