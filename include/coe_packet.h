#ifndef COE_PACKET_H_
#define COE_PACKET_H_

#include <vector>

#include "coe_field.h"

namespace coe {

class CoePacket {
 private:
  std::vector<CoeField> m_fields;

 public:
  void add_field(const CoeField& field);
};

}  // namespace coe

#endif  // COE_PACKET_H_