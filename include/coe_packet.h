#ifndef COE_PACKET_H_
#define COE_PACKET_H_

#include <map>
#include <string>

#include "coe_field.h"

struct ws_dissection;

namespace coe {

class CoePacket {
 private:
  std::map<std::string, CoeField> m_fields;
  uint32_t m_number;  // Starting at 1

 public:
  void add_field(const CoeField& field);
  uint32_t get_number() const;
  void set_number(uint32_t number);
  static CoePacket create_from_dissection(struct ws_dissection* dissection);
  std::string to_string() const;
  bool contains_field(const std::string& field_name) const;
};

}  // namespace coe

#endif  // COE_PACKET_H_