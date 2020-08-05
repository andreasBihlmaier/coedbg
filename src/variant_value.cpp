#include "variant_value.h"

#include <boost/format.hpp>

namespace coe {

bool operator==(const NsTime &lhs, const NsTime &rhs) {
  return lhs.secs == rhs.secs && lhs.nsecs == rhs.nsecs;
}

class VariantValueToStringVisitor : public boost::static_visitor<> {
 private:
  std::string &m_str;

 public:
  VariantValueToStringVisitor(std::string &str) : m_str(str) {
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
  void operator()(const std::vector<uint8_t> &operand) const {
    m_str = "(len=" + std::to_string(operand.size());
    m_str += " data=0x";
    for (auto byte : operand) {
      m_str += (boost::format("%02x") % (unsigned int)byte).str();
    }
    m_str += ")";
  }
  void operator()(const NsTime &operand) const {
    m_str = (boost::format("%d.%09d") % operand.secs % operand.nsecs).str();
  }
};

std::string variant_value_to_string(const VariantValue &value) {
  std::string str;
  boost::apply_visitor(VariantValueToStringVisitor(str), value);
  return str;
}

}  // namespace coe