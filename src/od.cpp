#include "od.h"

#include <iostream>

#include <boost/format.hpp>

namespace coe {

void OD::add_entry(const OdEntry& entry) {
  if (m_od.find(entry.index) == m_od.end()) {
    m_od[entry.index] = OdObject{};
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

void OD::update_name_cache() {
  m_name_to_entry.clear();
  for (auto& object_kv : m_od) {
    for (auto& entry_kv : object_kv.second) {
      OdEntry& entry = entry_kv.second;
      if (m_name_to_entry.find(entry.name) != m_name_to_entry.end()) {
        throw std::runtime_error("Object name " + entry.name + " not unique");
      }
      m_name_to_entry[entry.name] = &entry;
    }
  }
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

OdObject* OD::get_object(uint16_t index) {
  auto object_kv = m_od.find(index);
  if (object_kv == m_od.end()) {
    return nullptr;
  }
  return &object_kv->second;
}

OdEntry* OD::get_entry(uint16_t index, uint8_t subindex) {
  OdObject* object = get_object(index);
  auto entry_kv = object->find(subindex);
  if (entry_kv == object->end()) {
    return nullptr;
  }
  return &entry_kv->second;
}

OdEntry* OD::get_entry(const std::string& name) {
  return m_name_to_entry[name];
}

std::vector<OdObject*> OD::get_objects_in_range(uint16_t start_index, uint16_t end_index) {
  std::vector<OdObject*> objects;
  auto object_iter = m_od.lower_bound(start_index);
  if (object_iter == m_od.end()) {
    return objects;
  }
  uint16_t upper_bound_index = end_index;
  if (upper_bound_index < std::numeric_limits<uint16_t>::max() - 1) {
    upper_bound_index++;
  }
  auto upper_bound_iter = m_od.lower_bound(upper_bound_index);
  while (object_iter != upper_bound_iter) {
    objects.push_back(&object_iter->second);
    object_iter++;
  }
  return objects;
}

}  // namespace coe