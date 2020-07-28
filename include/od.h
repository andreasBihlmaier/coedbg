#ifndef OD_H_
#define OD_H_

#include <map>
#include <string>

#include "od_data_type.h"
#include "od_entry.h"

namespace coe {

class OD {
 private:
  std::map<uint16_t, std::map<uint8_t, OdEntry>> m_od;
  std::map<std::string, OdDataType> m_datatypes;

 public:
  void add_entry(const OdEntry& entry);
  void add_datatype(const OdDataType& datatype);
  OdDataType get_type(const std::string& datatype_name) const;
  OdDataType get_type(const std::string& datatype_name, uint8_t subindex) const;
  OdDataType get_type(const std::string& datatype_name, const std::string& subindex_type_name) const;
};

}  // namespace coe

#endif  // OD_H_