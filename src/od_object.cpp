#include "od_object.h"

namespace coe {

std::string OdObject::to_string() const {
  std::string str;
  str += "(";
  for (auto& entry_kv : m_entries) {
    str += entry_kv.second.to_string();
  }
  str += ")";
  return str;
}

}  // namespace coe