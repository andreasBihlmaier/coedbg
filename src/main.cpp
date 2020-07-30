#include <iostream>

#include <boost/program_options.hpp>

#include "coedbg.h"

int main(int argc, char** argv) {
  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()                                                          //
      ("help", "This help message")                                           //
      ("from-pcap", po::value<std::string>(), "PCAP file to read")            //
      ("plugin-dir", po::value<std::string>(), "Wireshark plugin directory")  //
      ("esi-file", po::value<std::string>(), "ESI XML file");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  coe::CoeDebugger debugger;

  if (vm.count("esi-file")) {
    debugger.read_esi(vm["esi-file"].as<std::string>());
    const coe::OD* od = debugger.get_od();
    std::cout << od->pdo_mappings_to_string() << '\n';
  }

  if (vm.count("plugin-dir")) {
    debugger.set_plugin_dir(vm["plugin-dir"].as<std::string>());
  }

  if (vm.count("from-pcap")) {
    debugger.read_pcap(vm["from-pcap"].as<std::string>());
    auto& packets = debugger.get_packets();
    for (auto& packet : packets) {
      const uint8_t LogicalReadWrite = 12;  // TODO enum
      if (packet.contains_field("ecat_mailbox.coe")) {
        std::cout << "CoE: " << packet.to_string() << "\n";
        // TODO print each SDO transfer
        // TODO warn about SDO failures/aborts
        // TODO update OD for each SDO transfer
        // TODO update PDO mapping on SDO writes to respective objects
      } else if (packet.contains_field("ecat.lad") &&
                 packet.get_field("ecat.cmd")->get_value<uint8_t>() == LogicalReadWrite) {
        std::cout << "PDO: " << packet.to_string() << "\n";
        auto& pdo_data = packet.get_field("ecat.data")->get_value<std::vector<uint8_t>>();
        std::cout << "pdo_data.size()=" << pdo_data.size() << "\n";
        // TODO decode and print each PDO packet
        // TODO update OD for each PDO
      }
    }
  } else {
    std::cout << "Nothing to do (see --help).\n";
    return 1;
  }

  return 0;
}