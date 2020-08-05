#include <iostream>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include "coedbg.h"

std::string to_sdo_string(const coe::CoePacket& packet, coe::OD* od) {
  std::string str;
  bool sdo_failed = false;
  if (packet.was_sent_from_master()) {
    str += "M->S";
  } else if (packet.was_sent_from_slave()) {
    str += "S->M";
  } else {
    str += "?->?";
  }
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

void print_value_change(const coe::OdEntry& entry, const coe::VariantValue& new_value) {
  if (entry.value != new_value) {
    std::cout << (boost::format("OD value changed: index=0x%04x subindex=%03d fieldname=\"%s\" old=%s new=%s\n") %
                  entry.index % (unsigned int)entry.subindex % entry.name % entry.value_to_string() %
                  coe::OdEntry::value_to_string(new_value))
                     .str();
  }
}

int main(int argc, char** argv) {
  bool print_esi_od;
  bool print_pdo_mappings;
  bool print_od_changes;
  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()                                                                                            //
      ("help", "This help message")                                                                             //
      ("print-esi-od", po::bool_switch(&print_esi_od)->default_value(false), "Print OD")                        //
      ("print-pdo-mappings", po::bool_switch(&print_pdo_mappings)->default_value(false), "Print PDO mappings")  //
      ("print-od-changes", po::bool_switch(&print_od_changes)->default_value(false),
       "Print every value change in OD (only shows writes that actually change a value)")  //
      ("plugin-dir", po::value<std::string>(), "Wireshark plugin directory")               //
      ("from-pcap", po::value<std::string>(), "PCAP file to read")                         //
      ("slave-address", po::value<uint16_t>(), "Slave ID")                                 //
      ("esi-file", po::value<std::string>(), "ESI XML file");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
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
        // std::cout << "CoE: " << packet.to_string() << "\n";
        std::cout << to_sdo_string(packet, od) << "\n";
        // TODO warn about SDO failures/aborts
        // TODO update PDO mapping on SDO writes to respective objects
      } else if (packet.is_pdo()) {
        // std::cout << "PDO: " << packet.to_string() << "\n";
        auto& pdo_data = packet.get_field("ecat.data")->get_value<std::vector<uint8_t>>();
        std::cout << "pdo_data.size()=" << pdo_data.size() << "\n";  // TODO to_pdo_string(packet, od)
        // TODO decode and print each PDO packet
        // TODO update OD for each PDO
      }
      if (od != nullptr) {
        // TODO filter packets: only process packets related to slave-address
        debugger.update_od(packet, false);
      }
    }
  } else {
    std::cout << "Nothing to do (see --help).\n";
    return 1;
  }

  return 0;
}