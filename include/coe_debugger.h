#ifndef COE_DEBUGGER_H_
#define COE_DEBUGGER_H_

#include <string>
#include <vector>

#include "coe_packet.h"

namespace coe {

class CoeDebugger {
 private:
  std::vector<CoePacket> m_packets;

 public:
  void add_packet(const CoePacket& packet);
  void read_pcap(const std::string& pcap_path);
  void set_plugin_dir(const std::string& plugin_dir);
};

}  // namespace coe

#endif  // COE_DEBUGGER_H_