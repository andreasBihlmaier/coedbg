#include <iostream>
#include <set>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include "coedbg.h"

std::string common_part_to_string(const coe::CoePacket& packet) {
  std::string str;
  if (packet.was_sent_from_master()) {
    str += "M->S";
  } else if (packet.was_sent_from_slave()) {
    str += "S->M";
  } else {
    str += "?->?";
  }
  return str;
}

std::string sdo_to_string(const coe::CoePacket& packet, coe::OD* od) {
  bool sdo_failed = false;
  std::string str;
  str += common_part_to_string(packet);
  str += (boost::format(" slave=0x%04x") % packet.get_field("ecat.adp")->get_value<uint16_t>()).str();
  str += (boost::format(" wcnt=%03d") % packet.get_field("ecat.cnt")->get_value<uint16_t>()).str();
  str += " SDO";
  if (packet.is_sdo_type(coe::SdoType::SdoRequest)) {
    str += "  Request";
    if (packet.is_client_command_specifier(coe::SdoClientCommandSpecifier::InitiateDownload)) {
      str += " Download";
    } else if (packet.is_client_command_specifier(coe::SdoClientCommandSpecifier::InitiateUpload)) {
      str += "   Upload";
    } else if (packet.is_client_command_specifier(coe::SdoClientCommandSpecifier::AbortTransfer)) {
      str += "    Abort";
      sdo_failed = true;
    } else {
      str += "  UNKNOWN";
    }
  } else if (packet.is_sdo_type(coe::SdoType::SdoResponse)) {
    str += " Response";
    if (packet.is_server_command_specifier(coe::SdoServerCommandSpecifier::InitiateDownload)) {
      str += " Download";
    } else if (packet.is_server_command_specifier(coe::SdoServerCommandSpecifier::InitiateUpload)) {
      str += "   Upload";
    } else {
      str += "  UNKNOWN";
    }
  } else {
    str += "  UNKNOWN";
  }
  if (packet.is_complete_access()) {
    str += " Complete-Access";
  }
  str += ":";
  if (sdo_failed) {
    str += " SDO FAILED";
  } else {
    if (packet.contains_field("ecat_mailbox.coe.sdoidx") && packet.get_field("ecat_mailbox.coe.sdosub")) {
      uint16_t index = packet.get_field("ecat_mailbox.coe.sdoidx")->get_value<uint16_t>();
      str += (boost::format(" index=0x%04x") % index).str();
      uint8_t subindex = packet.get_field("ecat_mailbox.coe.sdosub")->get_value<uint8_t>();
      str += (boost::format(" subindex=%03d") % (unsigned int)subindex).str();
      if (od != nullptr) {
        str += " fieldname=";
        auto od_entry = od->get_entry(index, subindex);
        if (od_entry != nullptr) {
          str += "\"" + od_entry->name + "\"";
        } else {
          str += "UNKNOWN";
        }
      }
    } else {
      str += " (index, subindex) MISSING";
    }
  }
  if (packet.is_sdo_type(coe::SdoType::SdoResponse)) {
    if (packet.get_field("ecat_mailbox.coe.dsoldata") != nullptr) {
      str += " data=" + packet.get_field("ecat_mailbox.coe.dsoldata")->value_to_string();
    } else if (packet.get_field("ecat_mailbox.coe.sdodata") != nullptr) {
      str += " data=" + packet.get_field("ecat_mailbox.coe.sdodata")->value_to_string();
    } else {
      str += " NO-DATA";
    }
  }
  return str;
}

std::string to_hex_string(const std::vector<uint8_t>& data, uint16_t start, uint16_t length) {
  std::string str;
  for (uint16_t i = start; i < start + length; ++i) {
    str += (boost::format("%02x") % (unsigned int)data[i]).str();
  }
  return str;
}

std::string pdo_data_to_string(const std::vector<uint8_t>& data, const coe::PdoMapping& pdo_mapping,
                               uint16_t number_of_slaves, uint16_t pdo_data_byte_offset) {
  std::string str;
  uint16_t pdo_size = coe::OD::get_pdo_byte_size(pdo_mapping);
  for (uint16_t slave_position = 0; slave_position < number_of_slaves; ++slave_position) {
    str += " slave" + std::to_string(slave_position + 1) + "=(";
    for (const auto& byte_offset_entry_kv : pdo_mapping) {
      coe::OdEntry* entry = byte_offset_entry_kv.second;
      uint16_t relative_byte_offset_start = byte_offset_entry_kv.first;
      uint16_t absolute_byte_offset_start =
          pdo_data_byte_offset + pdo_size * slave_position + relative_byte_offset_start;
      uint16_t byte_length = entry->bit_size / 8;
      str += (boost::format("(off=r%d=a%d,len=%d,val=0x%s->(%04x, %d)=%s,val=%s)") % relative_byte_offset_start %
              absolute_byte_offset_start % byte_length % to_hex_string(data, absolute_byte_offset_start, byte_length) %
              entry->index % (unsigned int)entry->subindex % entry->name %
              variant_value_to_string(entry->decode_data(
                  coe::CoeDebugger::extract_entry_data(data, absolute_byte_offset_start, byte_length))))
                 .str();
    }
    str += ")";
  }
  return str;
}

std::string pdo_to_string(const coe::CoePacket& packet, coe::OD* od, uint16_t number_of_slaves) {
  std::string str;
  str += common_part_to_string(packet);
  str += (boost::format(" idx=0x%02x") % (unsigned int)packet.get_field("ecat.idx")->get_value<uint8_t>()).str();
  str += (boost::format(" logical-address=0x%08x") % packet.get_field("ecat.lad")->get_value<uint32_t>()).str();
  std::vector<uint8_t> data = packet.get_field("ecat.data")->get_value<std::vector<uint8_t>>();
  str += " data_size=" + std::to_string(data.size());
  if (packet.was_sent_from_master()) {  // process RxPDO (master->slave)
    str += " RxPDO:";
    str += pdo_data_to_string(data, od->get_rxpdo_mapping(), number_of_slaves, 0);
  } else if (packet.was_sent_from_slave()) {  // process TxPDO (slave->master)
    str += " TxPDO:";
    str += pdo_data_to_string(data, od->get_txpdo_mapping(), number_of_slaves,
                              od->get_rxpdo_byte_size() * number_of_slaves);
  }
  return str;
}

void print_value_change(const coe::OdEntry& entry, const coe::VariantValue& new_value) {
  if (entry.value != new_value) {
    std::cout << (boost::format("OD value changed: index=0x%04x subindex=%03d fieldname=\"%s\" old=%s new=%s\n") %
                  entry.index % (unsigned int)entry.subindex % entry.name % entry.value_to_string() %
                  variant_value_to_string(new_value))
                     .str();
  }
}

// TODO replace this by something less hacky
#define PRINT_ONCE(str) print_once(str, __FILE__ ":" BOOST_PP_STRINGIZE(__LINE__))
void print_once(const std::string& str, const std::string& file_line) {
  static std::set<std::string> already_printed;
  if (already_printed.find(file_line) == already_printed.end()) {
    already_printed.insert(file_line);
    std::cout << str;
  }
}

int main(int argc, char** argv) {
  bool print_esi_od;
  bool print_pdo_mappings;
  bool print_od_changes;
  bool print_sdos;
  bool print_pdos;
  uint16_t slaves_count = 0;
  uint16_t slave_address = 0;
  uint16_t slave_position = 0;
  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()                                                                                            //
      ("help", "This help message")                                                                             //
      ("print-esi-od", po::bool_switch(&print_esi_od)->default_value(false), "Print OD")                        //
      ("print-pdo-mappings", po::bool_switch(&print_pdo_mappings)->default_value(false), "Print PDO mappings")  //
      ("print-od-changes", po::bool_switch(&print_od_changes)->default_value(false),
       "Print every value change in OD (only shows writes that actually change a value)")  //
      ("print-sdos", po::bool_switch(&print_sdos)->default_value(false), "Print SDOs")     //
      ("print-pdos", po::bool_switch(&print_pdos)->default_value(false), "Print PDOs")     //
      ("plugin-dir", po::value<std::string>(), "Wireshark plugin directory")               //
      ("from-pcap", po::value<std::string>(), "PCAP file to read")                         //
      ("slaves-count", po::value<uint16_t>(), "Number of slaves")                          //
      ("slave-address", po::value<std::string>(), "Slave address (hex value)")             //
      ("esi-file", po::value<std::string>(), "ESI XML file");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  if (vm.count("slaves-count")) {
    slaves_count = vm["slaves-count"].as<uint16_t>();
  }
  if (vm.count("slave-address")) {
    slave_address = std::stoul(vm["slave-address"].as<std::string>(), 0, 16);
    if (slave_address < 0x1001) {
      std::cout << (boost::format("Invalid slave address 0x%04x (must be >= 0x1001)\n") % slave_address).str();
      return 1;
    }
    slave_position = slave_address - 0x1001;
  }

  coe::CoeDebugger debugger;

  coe::OD* od = nullptr;
  if (vm.count("esi-file")) {
    debugger.read_esi(vm["esi-file"].as<std::string>());
    od = debugger.get_od();
    if (print_esi_od) {
      std::cout << "OD types:\n" << od->types_to_string() << '\n';
      std::cout << "OD content:\n" << od->to_string() << '\n';
    }
    if (print_pdo_mappings) {
      std::cout << od->pdo_mappings_to_string() << '\n';
    }
    if (print_od_changes) {
      od->set_value_change_callback(print_value_change);
    }
  }

  if (vm.count("plugin-dir")) {
    debugger.set_plugin_dir(vm["plugin-dir"].as<std::string>());
  }

  if (vm.count("from-pcap")) {
    debugger.read_pcap(vm["from-pcap"].as<std::string>());
    auto& packets = debugger.get_packets();
    for (auto& packet : packets) {
      if (packet.is_sdo()) {
        if (print_sdos) {
          std::cout << sdo_to_string(packet, od) << "\n";
        }
      } else if (packet.is_pdo()) {
        if (print_pdos) {
          if (slaves_count != 0) {
            // std::cout << "PDO: " << packet.to_string() << "\n";
            std::cout << pdo_to_string(packet, od, slaves_count) << "\n";
          } else {
            PRINT_ONCE("Number of slaves not specified, thus cannot print PDOs.\n");
          }
        }
      }
      if (od != nullptr) {
        if (slaves_count != 0 && slave_address != 0) {
          // TODO filter packets: only process packets related to slave-address AND pass on slaves-count
          // TODO update PDO mapping on SDO writes to respective objects
          debugger.update_od(packet, slaves_count, slave_position, false);
        } else {
          PRINT_ONCE("Both number of slaves and slave address must be specified in order to update OD.\n");
        }
      } else {
        // TODO support reconstructing OD from CoE Entry Req responses
        PRINT_ONCE("No ESI specified, thus cannot update OD as none is defined.\n");
      }
    }
  } else {
    std::cout << "Nothing to do (see --help).\n";
    return 1;
  }

  return 0;
}
