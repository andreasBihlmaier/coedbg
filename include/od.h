#ifndef OD_H_
#define OD_H_

#include <map>
#include <string>

#include "od_entry.h"

namespace coe {

class OD {
 private:
  std::map<uint16_t, std::map<uint8_t, OdEntry>> m_od;

 public:
  void add(const OdEntry& entry);
};

}  // namespace coe

#endif  // OD_H_