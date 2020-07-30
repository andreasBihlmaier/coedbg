#ifndef OD_H_
#define OD_H_

#include <map>
#include <string>
#include <vector>

#include "od_data_type.h"
#include "od_entry.h"
#include "od_object.h"

namespace coe {

class OD {
 private:
  std::map<uint16_t, OdObject> m_od;
  std::map<std::string, OdDataType> m_datatypes;
  std::map<std::string, OdEntry*> m_name_to_entry;

 public:
  void add_entry(const OdEntry& entry);
  void add_datatype(const OdDataType& datatype);
  void update_name_cache();
  OdDataType get_type(const std::string& datatype_name) const;
  OdDataType get_type(const std::string& datatype_name, uint8_t subindex) const;
  OdDataType get_type(const std::string& datatype_name, const std::string& subindex_type_name) const;
  OdObject* get_object(uint16_t index);
  OdEntry* get_entry(uint16_t index, uint8_t subindex);
  OdEntry* get_entry(const std::string& name);
  std::vector<OdObject*> get_objects_in_range(uint16_t start_index, uint16_t end_index);
};

}  // namespace coe

#endif  // OD_H_