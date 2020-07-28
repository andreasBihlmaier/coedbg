#include "od_entry.h"

#include <boost/format.hpp>

namespace coe {

class OdEntryValueToStringVisitor : public boost::static_visitor<> {
 private:
  std::string &m_str;

 public:
  OdEntryValueToStringVisitor(std::string &str) : m_str(str) {
  }
  void operator()(const bool &operand) const {
    m_str = (boost::format("%s=%d") % (operand == true ? "true" : "false") % operand).str();
  }
  void operator()(const uint8_t &operand) const {
    m_str = (boost::format("0x%02x=%u") % (unsigned int)operand % (unsigned int)operand).str();
  }
  void operator()(const uint16_t &operand) const {
    m_str = (boost::format("0x%04x=%u") % operand % operand).str();
  }
  void operator()(const uint32_t &operand) const {
    m_str = (boost::format("0x%08x=%u") % operand % operand).str();
  }
  void operator()(const uint64_t &operand) const {
    m_str = (boost::format("0x%16x=%llu") % operand % operand).str();
  }
  void operator()(const int8_t &operand) const {
    m_str = (boost::format("0x%02x=%d") % operand % operand).str();
  }
  void operator()(const int16_t &operand) const {
    m_str = (boost::format("0x%04x=%d") % operand % operand).str();
  }
  void operator()(const int32_t &operand) const {
    m_str = (boost::format("0x%08x=%d") % operand % operand).str();
  }
  void operator()(const int64_t &operand) const {
    m_str = (boost::format("0x%16x=%lld") % operand % operand).str();
  }
  void operator()(const float &operand) const {
    m_str = (boost::format("%f") % operand).str();
  }
  void operator()(const double &operand) const {
    m_str = (boost::format("%lf") % operand).str();
  }
  void operator()(const std::string &operand) const {
    m_str = operand;
  }
};

static std::string od_entry_value_string(const OdEntry::value_t &value) {
  std::string value_str;
  boost::apply_visitor(OdEntryValueToStringVisitor(value_str), value);
  return value_str;
}

std::string OdEntry::to_string() const {
  std::string str;

  str += "(";
  str += "index=" + (boost::format("0x%04x") % index).str();
  str += " subindex=" + std::to_string(subindex);
  str += " name=" + name;
  if (!type_name.empty()) {
    str += " type_name=" + type_name;
  }
  str += " type=" + od_base_type_name(type);
  str += " bit_size=" + std::to_string(bit_size);
  if (!default_data.empty()) {
    str += " default_data=" + default_data;
  }
  str += " value=" + od_entry_value_string(value);
  str += ")";

  return str;
}

}  // namespace coe