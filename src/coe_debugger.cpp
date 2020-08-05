#include "coe_debugger.h"

#include <unistd.h>
#include <iostream>
#include <stdexcept>

#include <libwireshark/ws_capture.h>
#include <libwireshark/ws_dissect.h>
#include <boost/format.hpp>

namespace coe {

static std::vector<uint8_t> extract_entry_data(const std::vector<uint8_t> &data, uint16_t start, uint16_t length) {
  // TODO do this more efficiently, e.g. by implementing a set_value that takes (reverse) iterators
  return std::vector<uint8_t>{&data[start], &data[start + length]};
}

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

const std::vector<CoePacket> &CoeDebugger::get_packets() const {
  return m_packets;
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

OD *CoeDebugger::get_od() const {
  return m_od.get();
}

void CoeDebugger::update_od(const CoePacket &packet, bool missing_od_entries_are_errors) {
  if (packet.is_sdo() && packet.sdo_status_ok()) {
    uint16_t index = packet.get_field("ecat_mailbox.coe.sdoidx")->get_value<uint16_t>();
    uint8_t subindex = packet.get_field("ecat_mailbox.coe.sdosub")->get_value<uint8_t>();
    OdObject *object = m_od->get_object(index);
    OdEntry *entry = m_od->get_entry(index, subindex);
    if (entry == nullptr) {
      std::string msg =
          (boost::format("Entry index=0x%04x subindex=%d does not exist in OD") % index % (unsigned int)subindex).str();
      if (missing_od_entries_are_errors) {
        throw std::runtime_error(msg);
      } else {
        std::cout << "Warning: " << msg << "\n";
        return;
      }
    }
    VariantValue value;
    if (packet.get_field("ecat_mailbox.coe.dsoldata") != nullptr) {
      value = packet.get_field("ecat_mailbox.coe.dsoldata")->get_value<VariantValue>();
    } else if (packet.get_field("ecat_mailbox.coe.sdodata") != nullptr) {
      value = packet.get_field("ecat_mailbox.coe.sdodata")->get_value<VariantValue>();
    }

    if (packet.was_sent_from_master()) {
      if (packet.is_sdo_type(coe::SdoType::SdoRequest)) {
        if (packet.is_client_command_specifier(coe::SdoClientCommandSpecifier::InitiateDownload)) {
          std::cout << "CoeDebugger::update_od TODO handle SDO Download\n";
        } else if (packet.is_client_command_specifier(coe::SdoClientCommandSpecifier::InitiateUpload)) {
          // nothing to do
        } else {
          throw std::runtime_error("Master SdoRequest ServerCommandSpecifier not implemented");
        }
      } else {
        throw std::runtime_error("Master SdoType not implemented");
      }
    } else if (packet.was_sent_from_slave()) {
      if (packet.is_sdo_type(coe::SdoType::SdoResponse)) {
        if (packet.is_server_command_specifier(coe::SdoServerCommandSpecifier::InitiateUpload)) {
          if (packet.is_complete_access()) {
            object->set_complete_value(value);
          } else {
            entry->set_value(value);
          }
        } else {
          throw std::runtime_error("Slave SdoResponse SdoServerCommandSpecifier not implemented");
        }
      } else if (packet.is_sdo_type(coe::SdoType::SdoRequest)) {
        if (packet.is_client_command_specifier(coe::SdoClientCommandSpecifier::InitiateUpload)) {
          // nothing to do
        } else {
          throw std::runtime_error("Slave SdoRequest SdoClientCommandSpecifier not implemented");
        }
      } else {
        throw std::runtime_error("Slave SdoType not implemented");
      }
    } else {
      throw std::runtime_error("Packet from unknown sender");
    }
  } else if (packet.is_pdo()) {
    // TODO honor 0x1C12 (SM2) and obj0x1C13 (SM3) for PDO Assignments

    std::vector<uint8_t> data = packet.get_field("ecat.data")->get_value<std::vector<uint8_t>>();
    if (packet.was_sent_from_master()) {  // process RxPDO (master->slave)
      // TODO initial offset due to number of slaves and address of current one
      auto &rxpdo_mapping = m_od->get_rxpdo_mapping();
      for (const auto &byte_offset_entry_kv : rxpdo_mapping) {
        OdEntry *entry = byte_offset_entry_kv.second;
        entry->set_value(extract_entry_data(data, byte_offset_entry_kv.first, entry->bit_size / 8));
      }
    } else if (packet.was_sent_from_slave()) {  // process TxPDO (slave->master)
      const uint16_t NUMBER_OF_SLAVES = 6;      // XXX value 6 is a hack!
      const uint16_t SLAVE_POSITION = 5;        // XXX value 5 is a hack!
      // TODO the following must be multiplied by number of slaves
      uint16_t txpdo_start_byte_offset = m_od->get_rxpdo_byte_size() * NUMBER_OF_SLAVES;
      auto &txpdo_mapping = m_od->get_txpdo_mapping();
      // TODO the following must be offset by the position of this slave
      uint16_t txpdo_size = m_od->get_txpdo_byte_size();
      for (const auto &byte_offset_entry_kv : txpdo_mapping) {
        OdEntry *entry = byte_offset_entry_kv.second;
        entry->set_value(
            extract_entry_data(data, txpdo_start_byte_offset + txpdo_size * SLAVE_POSITION + byte_offset_entry_kv.first,
                               entry->bit_size / 8));
      }
    } else {
      throw std::runtime_error("Packet from unknown sender");
    }
  }
}

}  // namespace coe