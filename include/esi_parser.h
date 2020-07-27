#ifndef ESI_PARSER_H_
#define ESI_PARSER_H_

#include <map>
#include <string>

#include "od.h"

namespace coe {

class EsiParser {
 private:
  OD m_od;

 public:
  void read_file(const std::string& esi_path);
};

}  // namespace coe

#endif  // ESI_PARSER_H_