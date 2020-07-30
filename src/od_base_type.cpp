#include "od_base_type.h"

namespace coe {

std::string od_base_type_name(OdBaseType type) {
  std::string str;
  switch (type) {
    case OdBaseType::Invalid:
      str = "INVALID";
      break;
    case OdBaseType::Bool:
      str = "Bool";
      break;
    case OdBaseType::Int8:
      str = "Int8";
      break;
    case OdBaseType::Uint8:
      str = "Uint8";
      break;
    case OdBaseType::Int16:
      str = "Int16";
      break;
    case OdBaseType::Uint16:
      str = "Uint16";
      break;
    case OdBaseType::Int32:
      str = "Int32";
      break;
    case OdBaseType::Uint32:
      str = "Uint32";
      break;
    case OdBaseType::Int64:
      str = "Int64";
      break;
    case OdBaseType::Uint64:
      str = "Uint64";
      break;
    case OdBaseType::Float:
      str = "Float";
      break;
    case OdBaseType::Double:
      str = "Double";
      break;
    case OdBaseType::String:
      str = "String";
      break;
    default:
      throw std::runtime_error("Unkown value of OdBaseType (" +
                               std::to_string(static_cast<std::underlying_type_t<OdBaseType>>(type)) + ")");
      break;
  };
  return str;
}

OdBaseType od_base_type_from_string(const std::string& base_type_name) {
  OdBaseType base_type;

  if (base_type_name == "BOOL") {
    base_type = OdBaseType::Bool;
  } else if (base_type_name == "USINT" || base_type_name == "BYTE") {
    base_type = OdBaseType::Uint8;
  } else if (base_type_name == "SINT") {
    base_type = OdBaseType::Int8;
  } else if (base_type_name == "UINT") {
    base_type = OdBaseType::Uint16;
  } else if (base_type_name == "INT") {
    base_type = OdBaseType::Int16;
  } else if (base_type_name == "UDINT") {
    base_type = OdBaseType::Uint32;
  } else if (base_type_name == "DINT") {
    base_type = OdBaseType::Int32;
  } else if (base_type_name == "ULINT") {
    base_type = OdBaseType::Uint64;
  } else if (base_type_name == "LINT") {
    base_type = OdBaseType::Int64;
  } else if (base_type_name == "REAL") {
    base_type = OdBaseType::Float;
  } else if (base_type_name == "LREAL") {
    base_type = OdBaseType::Double;
  } else if (base_type_name.rfind("STRING", 0) == 0) {
    base_type = OdBaseType::String;
  } else if (base_type_name.rfind("BIT", 0) == 0) {
    base_type = OdBaseType::Uint64;
  } else {
    base_type = OdBaseType::Invalid;
  }

  return base_type;
}

}  // namespace coe