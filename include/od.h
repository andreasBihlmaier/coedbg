#ifndef OD_H_
#define OD_H_

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "od_data_type.h"
#include "od_entry.h"
#include "od_object.h"

namespace coe {

using PdoMapping = std::map<uint16_t, OdEntry*>;

struct PdoMappingDecoding {
  uint16_t index;
  uint8_t subindex;
  uint8_t byte_size;
};

class OD {
 public:
  static PdoMappingDecoding decode_pdo_mapping(const OdEntry& entry);
  static std::string pdo_mapping_to_string(const PdoMapping& mapping);

  void add_entry(const OdEntry& entry);
  void add_datatype(const OdDataType& datatype);
  void update_name_cache();
  void update_pdo_mapping_cache();
  OdDataType get_type(const std::string& datatype_name) const;
  OdDataType get_type(const std::string& datatype_name, uint8_t subindex) const;
  OdDataType get_type(const std::string& datatype_name, const std::string& subindex_type_name) const;
  OdObject* get_object(uint16_t index);
  OdEntry* get_entry(uint16_t index, uint8_t subindex);
  OdEntry* get_entry(const std::string& name);
  std::vector<OdObject*> get_objects_in_range(uint16_t start_index, uint16_t end_index);
  std::string pdo_mappings_to_string() const;
  std::string to_string() const;
  std::string types_to_string() const;
  void set_value_change_callback(OdEntry::ValueChangeCallback value_change_callback);
  const PdoMapping& get_rxpdo_mapping() const;
  uint16_t get_rxpdo_byte_size() const;
  const PdoMapping& get_txpdo_mapping() const;
  uint16_t get_txpdo_byte_size() const;

 private:
  std::map<uint16_t, OdObject> m_od;
  std::map<std::string, OdDataType> m_datatypes;
  std::map<std::string, OdEntry*> m_name_to_entry;
  PdoMapping m_rxpdo_mapping;
  PdoMapping m_txpdo_mapping;

  PdoMapping pdo_mappings(uint16_t start_index, uint16_t end_index);
};

}  // namespace coe

#endif  // OD_H_