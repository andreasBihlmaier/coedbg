#include "coe_debugger.h"

#include <unistd.h>
#include <iostream>
#include <stdexcept>

#include <libwireshark/ws_capture.h>
#include <libwireshark/ws_dissect.h>

namespace coe {

CoeDebugger::CoeDebugger() : m_od(new OD) {
}

void CoeDebugger::add_packet(const CoePacket &packet) {
  m_packets.push_back(packet);
}

void CoeDebugger::set_plugin_dir(const std::string &plugin_dir) {
  if (!ws_dissect_plugin_dir(plugin_dir.c_str())) {
    throw std::runtime_error("Failed to set plugin directory");
  }
}

void CoeDebugger::read_pcap(const std::string &pcap_path) {
  if (access(pcap_path.c_str(), R_OK) == -1) {
    throw std::runtime_error("PCAP file cannot be read.");
    return;
  }

  ws_capture_init();
  ws_dissect_init();

  int err_code;
  char *err_info;
  ws_capture_t *cap = ws_capture_open_offline(pcap_path.c_str(), 0, &err_code, &err_info);
  if (cap == nullptr) {
    throw std::runtime_error("Failed to open PCAP file (" + std::to_string(err_code) + "): " + std::string(err_info));
  }

  ws_dissect_t *dissector = ws_dissect_capture(cap);
  if (dissector == nullptr) {
    throw std::runtime_error("Failed to dissect PCAP file.");
  }

  struct ws_dissection packet;
  uint32_t packet_number = 1;
  while (ws_dissect_next(dissector, &packet, NULL, NULL)) {
    add_packet(CoePacket::create_from_dissection(&packet));
    m_packets.back().set_number(packet_number);
    packet_number++;
  }

  printf("Read %ld packets\n", m_packets.size());
}

std::vector<const CoePacket *> CoeDebugger::get_packets_containing_field(const std::string &field_name) const {
  std::vector<const CoePacket *> matching_packets;
  for (auto &packet : m_packets) {
    if (packet.contains_field(field_name)) {
      matching_packets.push_back(&packet);
    }
  }
  return matching_packets;
}

void CoeDebugger::read_esi(const std::string &esi_path) {
  m_esi_parser.reset(new EsiParser{m_od.get()});
  m_esi_parser->read_file(esi_path);
}

OD *CoeDebugger::get_od() {
  return m_od.get();
}

}  // namespace coe