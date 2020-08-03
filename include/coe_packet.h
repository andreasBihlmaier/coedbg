#ifndef COE_PACKET_H_
#define COE_PACKET_H_

#include <map>
#include <string>

#include "coe_field.h"

struct ws_dissection;

namespace coe {

enum class SdoType : uint16_t {
  SdoRequest = 2,
  SdoResponse = 3,
};
enum class SdoClientCommandSpecifier : uint8_t {
  InitiateDownload = 1,
  InitiateUpload = 2,
};
enum class SdoServerCommandSpecifier : uint8_t {
  UploadSegment = 0,
  DownloadSegment = 1,
  InitiateUpload = 2,
  InitiateDownload = 3,
};
enum class EtherCatCommand : uint8_t {
  LogicalReadWrite = 12,
};
const std::vector<uint8_t> MasterSourceAddress{1, 1, 1, 1, 1, 1};
const std::vector<uint8_t> SlaveSourceAddress{3, 1, 1, 1, 1, 1};

class CoePacket {
 public:
  void add_field(const CoeField& field);
  const CoeField* get_field(const std::string& field_name) const;
  uint32_t get_number() const;
  void set_number(uint32_t number);
  static CoePacket create_from_dissection(struct ws_dissection* dissection);
  std::string to_string() const;
  bool contains_field(const std::string& field_name) const;
  bool was_sent_from_master() const;
  bool was_sent_from_slave() const;
  bool is_sdo_type(SdoType sdo_type) const;
  bool is_client_command_specifier(SdoClientCommandSpecifier sdo_ccs) const;
  bool is_server_command_specifier(SdoServerCommandSpecifier sdo_scs) const;
  bool is_sdo() const;
  bool is_pdo() const;

 private:
  std::map<std::string, CoeField> m_fields;
  uint32_t m_number;  // Starting at 1
};

}  // namespace coe

#endif  // COE_PACKET_H_