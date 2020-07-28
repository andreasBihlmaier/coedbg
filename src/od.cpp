#include "od.h"

#include <iostream>

#include <boost/format.hpp>

namespace coe {

void OD::add_entry(const OdEntry& entry) {
  if (m_od.find(entry.index) == m_od.end()) {
    m_od[entry.index] = std::map<uint8_t, OdEntry>{};
  }
  auto& m_od_index = m_od[entry.index];
  if (m_od_index.find(entry.subindex) != m_od_index.end()) {
    throw std::runtime_error("Entry " +
                             (boost::format("(0x%04x, %d)") % entry.index % (unsigned int)entry.subindex).str() +
                             " already contained in OD");
  }
  m_od_index[entry.subindex] = entry;
}

void OD::add_datatype(const OdDataType& datatype) {
  if (datatype.name.empty()) {
    throw std::runtime_error("Tried to add DataTypes with empty name");
  }
  if (datatype.abstract_type.empty() && datatype.base_type == OdBaseType::Invalid && datatype.subindex_types.empty()) {
    throw std::runtime_error("Tried to add DataTypes with empty type, Invalid base_type and no subindex_types");
  }
  if (m_datatypes.find(datatype.name) != m_datatypes.end()) {
    throw std::runtime_error("DataType " + datatype.name + " already contained in DataTypes");
  }
  m_datatypes[datatype.name] = datatype;
}

OdDataType OD::get_type(const std::string& datatype_name) const {
  if (m_datatypes.find(datatype_name) == m_datatypes.end()) {
    throw std::runtime_error("DataType " + datatype_name + " unknown");
  }
  return m_datatypes.find(datatype_name)->second;
}

OdDataType OD::get_type(const std::string& datatype_name, uint8_t subindex) const {
  OdDataType datatype = get_type(datatype_name);
  OdDataType type;
  if (subindex == 0 && datatype.subindex_types.find(subindex) == datatype.subindex_types.end()) {
    type = datatype;
  } else {
    if (datatype.subindex_types.find(subindex) == datatype.subindex_types.end()) {
      throw std::runtime_error("DataType " + datatype_name + " subindex " + std::to_string(subindex) + " unknown");
    }
    type = get_type(datatype.subindex_types.find(subindex)->second.abstract_type, 0);
  }
  return type;
}

OdDataType OD::get_type(const std::string& datatype_name, const std::string& subindex_type_name) const {
  OdDataType datatype = get_type(datatype_name);
  for (auto& subindex_type : datatype.subindex_types) {
    if (subindex_type.second.name == subindex_type_name) {
      return subindex_type.second;
      break;
    }
  }
  for (auto& subindex_type : datatype.subindex_types) {
    if (subindex_type.second.name == "Elements") {
      return subindex_type.second;
      break;
    }
  }
  throw std::runtime_error("DataType (" + datatype_name + ", " + subindex_type_name + ") unknown");
  return OdDataType{};
}

}  // namespace coe