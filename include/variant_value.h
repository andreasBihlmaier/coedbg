#ifndef VARIANT_VALUE_H_
#define VARIANT_VALUE_H_

#include <string>
#include <vector>

#include <boost/variant.hpp>

namespace coe {

struct NsTime {
  time_t secs;
  int nsecs;
};

bool operator==(const NsTime& lhs, const NsTime& rhs);

using VariantValue = boost::variant<bool, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t,
                                    float, double, std::string, std::vector<uint8_t>, NsTime>;

std::string variant_value_to_string(const VariantValue& value);

}  // namespace coe

#endif  // VARIANT_VALUE_H_