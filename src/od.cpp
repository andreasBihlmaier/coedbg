#include "od.h"

#include <boost/format.hpp>

namespace coe {

void OD::add(const OdEntry& entry) {
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

}  // namespace coe