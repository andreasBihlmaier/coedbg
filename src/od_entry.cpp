#include "od_entry.h"

#include <iostream>

#include <boost/format.hpp>

namespace coe {

std::string OdEntry::value_to_string() const {
  return variant_value_to_string(value);
}

std::string OdEntry::to_string() const {
  std::string str;

  str += "(";
  str += "index=" + (boost::format("0x%04x") % index).str();
  str += " subindex=" + std::to_string(subindex);
  str += " name=" + name;
  if (!type_name.empty()) {
    str += " type_name=" + type_name;
  }
  str += " type=" + od_base_type_name(type);
  str += " bit_size=" + std::to_string(bit_size);
  if (!default_data_string.empty()) {
    str += " default_data_string=" + default_data_string;
  }
  str += " value=" + value_to_string();
  str += ")";

  return str;
}

void OdEntry::set_value_from_default_data() {
  switch (type) {
    case OdBaseType::Invalid:
      throw std::runtime_error("Cannot set default data for value with Invalid type");
      break;
    case OdBaseType::Bool:
      value = static_cast<bool>(std::stoi(default_data_string));
      break;
    case OdBaseType::Int8:
      value = static_cast<int8_t>(std::stoi(default_data_string, 0, 16));
      break;
    case OdBaseType::Uint8:
      value = static_cast<uint8_t>(std::stoul(default_data_string, 0, 16));
      break;
    case OdBaseType::Int16:
      value = static_cast<int16_t>(std::stoi(default_data_string, 0, 16));
      break;
    case OdBaseType::Uint16:
      value = static_cast<uint16_t>(std::stoul(default_data_string, 0, 16));
      break;
    case OdBaseType::Int32:
      value = static_cast<int32_t>(std::stol(default_data_string, 0, 16));
      break;
    case OdBaseType::Uint32:
      value = static_cast<uint32_t>(std::stoul(default_data_string, 0, 16));
      break;
    case OdBaseType::Int64:
      value = static_cast<int64_t>(std::stoll(default_data_string, 0, 16));
      break;
    case OdBaseType::Uint64:
      value = static_cast<uint64_t>(std::stoull(default_data_string));
      break;
    case OdBaseType::Float:
      value = static_cast<float>(std::stof(default_data_string));
      break;
    case OdBaseType::Double:
      value = static_cast<double>(std::stod(default_data_string));
      break;
    case OdBaseType::String:
      value = default_data_string;  // TODO hex to ascii?
      break;
    default:
      throw std::runtime_error("Unkown value of OdBaseType (" +
                               std::to_string(static_cast<std::underlying_type_t<OdBaseType>>(type)) + ")");
      break;
  };
}

VariantValue OdEntry::decode_data(const std::vector<uint8_t> &data) const {
  VariantValue decoded_value;
  switch (type) {
    case OdBaseType::Invalid:
      throw std::runtime_error("Cannot set data for value with Invalid type");
      break;
    case OdBaseType::Bool:
      decoded_value = static_cast<bool>(data[0]);
      break;
    case OdBaseType::Int8:
      decoded_value = static_cast<int8_t>(data[0]);
      break;
    case OdBaseType::Uint8:
      decoded_value = data[0];
      break;
    case OdBaseType::Int16: {
      int16_t v;
      memcpy(&v, &data[0], sizeof(v));
      decoded_value = v;
      break;
    }
    case OdBaseType::Uint16: {
      uint16_t v;
      memcpy(&v, &data[0], sizeof(v));
      decoded_value = v;
      break;
    }
    case OdBaseType::Int32: {
      int32_t v;
      memcpy(&v, &data[0], sizeof(v));
      decoded_value = v;
      break;
    }
    case OdBaseType::Uint32: {
      uint32_t v;
      memcpy(&v, &data[0], sizeof(v));
      decoded_value = v;
      break;
    }
    case OdBaseType::Int64: {
      int64_t v;
      memcpy(&v, &data[0], sizeof(v));
      decoded_value = v;
      break;
    }
    case OdBaseType::Uint64: {
      uint64_t v;
      memcpy(&v, &data[0], sizeof(v));
      decoded_value = v;
      break;
    }
    case OdBaseType::Float: {
      float v;
      memcpy(&v, &data[0], sizeof(v));
      decoded_value = v;
      break;
    }
    case OdBaseType::Double: {
      double v;
      memcpy(&v, &data[0], sizeof(v));
      decoded_value = v;
      break;
    }
    case OdBaseType::String:
      decoded_value = std::string(&data[0], &data[0] + data.size());
      break;
    default:
      throw std::runtime_error("Unkown value of OdBaseType (" +
                               std::to_string(static_cast<std::underlying_type_t<OdBaseType>>(type)) + ")");
      break;
  };
  return decoded_value;
}

void OdEntry::set_value(const std::vector<uint8_t> &data) {
  set_value(decode_data(data));
}

void OdEntry::set_value(const VariantValue &new_value) {
  if (value_change_callback) {
    value_change_callback(*this, new_value);
  }
  value = new_value;
}

}  // namespace coe