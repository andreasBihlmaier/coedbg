#include "coe_field.h"

#include <stdexcept>

#include <libwireshark/ws_dissect.h>
#include <boost/format.hpp>

namespace coe {

std::string CoeField::get_name() const {
  return m_name;
}

void CoeField::set_name(const std::string &name) {
  m_name = name;
}

enum ftenum CoeField::get_type() const {
  return m_type;
}

void CoeField::set_type(enum ftenum type) {
  m_type = type;
}

class ValueToStringVisitor : public boost::static_visitor<> {
 private:
  std::string &m_str;

 public:
  ValueToStringVisitor(std::string &str) : m_str(str) {
  }
  // template <typename T>
  // void operator()(T &operand) const {
  //  m_str = std::to_string(operand);
  //}
  void operator()(const bool &operand) const {
    m_str = (boost::format("%s=%d") % (operand == true ? "true" : "false") % operand).str();
  }
  void operator()(const uint8_t &operand) const {
    m_str = (boost::format("0x%02x=%u") % operand % operand).str();
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
  void operator()(const nstime_t &operand) const {
    m_str = (boost::format("%d.%09d") % operand.secs % operand.nsecs).str();
  }
};

std::string CoeField::to_string() const {
  std::string str;
  str += "name=" + m_name;
  str += " type=" + std::string(ftype_name(m_type));
  std::string value_str;
  boost::apply_visitor(ValueToStringVisitor(value_str), m_value);
  str += " value=" + value_str;
  return str;
}

void CoeField::set_value(const fvalue_t &value, enum ftenum type) {
  switch (type) {
    case FT_BOOLEAN:
      m_value = static_cast<bool>(value.value.uinteger);
      break;
    case FT_UINT8:
      m_value = static_cast<uint8_t>(value.value.uinteger);
      break;
    case FT_UINT16:
      m_value = static_cast<uint16_t>(value.value.uinteger);
      break;
    case FT_UINT32:
      m_value = static_cast<uint32_t>(value.value.uinteger);
      break;
    case FT_UINT64:
      m_value = static_cast<uint64_t>(value.value.uinteger64);
      break;
    case FT_INT8:
      m_value = static_cast<uint8_t>(value.value.sinteger);
      break;
    case FT_INT16:
      m_value = static_cast<uint16_t>(value.value.sinteger);
      break;
    case FT_INT32:
      m_value = static_cast<uint32_t>(value.value.sinteger);
      break;
    case FT_INT64:
      m_value = static_cast<int64_t>(value.value.sinteger64);
      break;
    case FT_FLOAT:
      m_value = static_cast<float>(value.value.floating);
      break;
    case FT_DOUBLE:
      m_value = static_cast<double>(value.value.floating);
      break;
    case FT_STRING:
      m_value = std::string(value.value.string);
      break;
    case FT_ABSOLUTE_TIME:  // fallthrough
    case FT_RELATIVE_TIME:
      m_value = value.value.time;
      break;
    default:
      printf("field type %s (0x%02x) not supported\n", ftype_name(m_type), type);
      break;
  }
}

CoeField CoeField::create_from_fieldinfo(field_info *fieldinfo) {
  CoeField field;

  header_field_info *headerfieldinfo = fieldinfo->hfinfo;
  if (headerfieldinfo == NULL) {
    throw std::runtime_error("headerfieldinfo is NULL");
  }
  field.set_name(headerfieldinfo->abbrev);
  field.set_type(headerfieldinfo->type);
  field.set_value(fieldinfo->value, headerfieldinfo->type);

  return field;
}

}  // namespace coe