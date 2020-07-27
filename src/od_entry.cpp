#include "od_entry.h"

#include <boost/format.hpp>

namespace coe {

std::string OdEntry::to_string() const {
  std::string str;

  str += "(";
  str += "index=" + (boost::format("0x%04x") % index).str();
  str += " subindex=" + std::to_string(subindex);
  str += " name=" + name;
  if (!type_name.empty()) {
    str += " type_name=" + type_name;
  }
  if (!default_data.empty()) {
    str += " default_data=" + default_data;
  }
  str += ")";

  return str;
}

}  // namespace coe