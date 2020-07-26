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

 public:
  void add_field(const CoeField& field);
  static CoePacket create_from_dissection(struct ws_dissection* dissection);
  std::string to_string() const;
};

}  // namespace coe

#endif  // COE_PACKET_H_