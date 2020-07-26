#include "coe_packet.h"

namespace coe {

void CoePacket::add_field(const CoeField& field) {
  m_fields.push_back(field);
}

}  // namespace coe