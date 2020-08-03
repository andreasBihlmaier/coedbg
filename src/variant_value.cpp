#include "variant_value.h"

namespace coe {

bool operator==(const NsTime& lhs, const NsTime& rhs) {
  return lhs.secs == rhs.secs && lhs.nsecs == rhs.nsecs;
}

}  // namespace coe