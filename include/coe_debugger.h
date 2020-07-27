#ifndef COE_DEBUGGER_H_
#define COE_DEBUGGER_H_

#include <memory>
#include <string>
#include <vector>

#include "coe_packet.h"
#include "esi_parser.h"

namespace coe {

class CoeDebugger {
 private:
  std::vector<CoePacket> m_packets;
  std::unique_ptr<EsiParser> m_esi_parser;

 public:
  void add_packet(const CoePacket& packet);
  void read_pcap(const std::string& pcap_path);
  void set_plugin_dir(const std::string& plugin_dir);
  std::vector<const CoePacket*> get_packets_containing_field(const std::string& field_name) const;
  void read_esi(const std::string& esi_path);
};

}  // namespace coe

#endif  // COE_DEBUGGER_H_