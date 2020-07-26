#include "coe_debugger.h"

#include <unistd.h>
#include <iostream>
#include <stdexcept>

#include <libwireshark/ws_capture.h>
#include <libwireshark/ws_dissect.h>

namespace coe {

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
  while (ws_dissect_next(dissector, &packet, NULL, NULL)) {
    add_packet(CoePacket::create_from_dissection(&packet));
    // std::cout << "packet: " << m_packets.back().to_string() << "\n";
  }

  printf("Read %ld packets\n", m_packets.size());
}

}  // namespace coe