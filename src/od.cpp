#include "od.h"

#include <iostream>

#include <boost/format.hpp>

namespace coe {

PdoMapping OD::pdo_mappings(uint16_t start_index, uint16_t end_index) {
  uint16_t byte_offset = 0;
  PdoMapping mapping;
  for (auto object : get_objects_in_range(start_index, end_index)) {
    for (auto entry_kv : *object) {
      if (entry_kv.first == 0) {  // skip array index
        continue;
      }
      auto decoded_mapping = decode_pdo_mapping(entry_kv.second);
      mapping[byte_offset] = get_entry(decoded_mapping.index, decoded_mapping.subindex);
      if (mapping[byte_offset] == nullptr) {
        mapping[byte_offset] = new OdEntry();  // TODO memory leak
        mapping[byte_offset]->index = decoded_mapping.index;
        mapping[byte_offset]->subindex = decoded_mapping.subindex;
        mapping[byte_offset]->name = "UNKNOWN OBJECT referenced by " + entry_kv.second.to_string();
      }
      byte_offset += decoded_mapping.byte_size;
    }
  }
  return mapping;
}

std::string OD::pdo_mapping_to_string(const PdoMapping& mapping) {
  std::string str;
  str += "(";
  for (auto byte_offset_entry_kv : mapping) {
    str += "(offset=" + std::to_string(byte_offset_entry_kv.first) +
           " entry=" + byte_offset_entry_kv.second->to_string() + ")";
  }
  str += ")";
  return str;
}

PdoMappingDecoding OD::decode_pdo_mapping(const OdEntry& entry) {
  if (entry.type != OdBaseType::Uint32) {
    throw std::runtime_error("OdEntry " + entry.to_string() + " has wrong type for a PDO mapping");
  }
  uint32_t mapping_value = boost::get<uint32_t>(entry.value);
  return PdoMappingDecoding{static_cast<uint16_t>((mapping_value & (0xFFFF << 16)) >> 16),
                            static_cast<uint8_t>((mapping_value & (0xFF << 8)) >> 8),
                            static_cast<uint8_t>(mapping_value & 0xFF)};
}

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
    throw std::runtime_error("Tried to add DataType with empty name");
  }
  if (datatype.abstract_type.empty() && datatype.base_type == OdBaseType::Invalid && datatype.subindex_types.empty()) {
    throw std::runtime_error("Tried to add DataType with empty type, Invalid base_type and no subindex_types: " +
                             datatype.to_string());
  }
  if (m_datatypes.find(datatype.name) != m_datatypes.end()) {
    throw std::runtime_error("DataType " + datatype.name + " already contained in DataTypes: " + datatype.to_string());
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

void OD::update_pdo_mapping_cache() {
  m_rxpdo_mapping = pdo_mappings(0x1600, 0x17ff);
  m_txpdo_mapping = pdo_mappings(0x1a00, 0x1bff);
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
  if (object == nullptr) {
    return nullptr;
  }
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

std::string OD::pdo_mappings_to_string() const {
  std::string str;
  str += "(RxPDO=" + pdo_mapping_to_string(m_rxpdo_mapping) + ")";
  str += "(TxPDO=" + pdo_mapping_to_string(m_txpdo_mapping) + ")";
  return str;
}

}  // namespace coe