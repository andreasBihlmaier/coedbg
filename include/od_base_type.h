#ifndef OD_BASE_TYPE_H_
#define OD_BASE_TYPE_H_

#include <string>

#include <boost/variant.hpp>

namespace coe {

enum class OdBaseType : uint8_t {
  Invalid,
  Bool,
  Int8,
  Uint8,
  Int16,
  Uint16,
  Int32,
  Uint32,
  Int64,
  Uint64,
  Float,
  Double,
  String
};

std::string od_base_type_name(OdBaseType type);
OdBaseType od_base_type_from_string(const std::string& base_type_name);

}  // namespace coe

#endif  // OD_BASE_TYPE_H_