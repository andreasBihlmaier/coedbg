#ifndef COE_FIELD_H_
#define COE_FIELD_H_

#include <string>
#include <vector>

#include <libwireshark/ws_dissect.h>

#include "variant_value.h"

struct field_info;

namespace coe {

class CoeField {
 public:
  std::string get_name() const;
  void set_name(const std::string& name);
  enum ftenum get_type() const;
  void set_type(enum ftenum type);
  void set_value(const fvalue_t& value, enum ftenum type);
  static CoeField create_from_fieldinfo(field_info* fieldinfo);
  std::string value_to_string() const;
  std::string to_string() const;

  template <class T>
  const T& get_value() const;

 private:
  std::string m_name;
  enum ftenum m_type;
  VariantValue m_value;
};

}  // namespace coe

#endif  // COE_FIELD_H_