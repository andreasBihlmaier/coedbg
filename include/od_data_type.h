#ifndef OD_DATA_TYPE_H_
#define OD_DATA_TYPE_H_

#include <map>
#include <string>

#include "od_base_type.h"

namespace coe {

class OdDataType {
 public:
  std::string name;
  std::string abstract_type;
  uint8_t subindex;
  uint32_t bit_size;
  uint32_t bit_offset;
  OdBaseType base_type;
  std::map<uint8_t, OdDataType> subindex_types;

  OdDataType() : subindex(0), bit_size(0), bit_offset(0), base_type(OdBaseType::Invalid) {
  }
  std::string to_string(bool print_subindex = false) const;
  void add_subtype(const OdDataType& subtype);
};

}  // namespace coe

#endif  // OD_DATA_TYPE_H_