#ifndef COE_FIELD_H_
#define COE_FIELD_H_

#include <string>
#include <vector>

#include <libwireshark/ws_dissect.h>
#include <boost/variant.hpp>

struct field_info;

namespace coe {

class CoeField {
 private:
  std::string m_name;
  using value_t = boost::variant<bool, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t, float,
                                 double, std::string, std::vector<uint8_t>, nstime_t>;
  enum ftenum m_type;
  value_t m_value;

 public:
  std::string get_name() const;
  void set_name(const std::string& name);
  enum ftenum get_type() const;
  void set_type(enum ftenum type);
  void set_value(const fvalue_t& value, enum ftenum type);
  static CoeField create_from_fieldinfo(field_info* fieldinfo);
  std::string to_string() const;

  template <class T>
  const T& get_value() const;
};

}  // namespace coe

#endif  // COE_FIELD_H_