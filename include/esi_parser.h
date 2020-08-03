#ifndef ESI_PARSER_H_
#define ESI_PARSER_H_

#include <map>
#include <string>

#include "od.h"

namespace coe {

class EsiParser {
 public:
  EsiParser(OD* od);
  void read_file(const std::string& esi_path);

 private:
  OD* m_od;
};

}  // namespace coe

#endif  // ESI_PARSER_H_