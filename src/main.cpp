#include <iostream>

#include <boost/program_options.hpp>

#include "coedbg.h"

int main(int argc, char** argv) {
  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()                                                //
      ("help", "This help message")                                 //
      ("from-pcap", po::value<std::string>(), "PCAP file to read")  //
      ("plugin-dir", po::value<std::string>(), "Wireshark plugin directory");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  coe::CoeDebugger debugger;

  if (vm.count("plugin-dir")) {
    debugger.set_plugin_dir(vm["plugin-dir"].as<std::string>());
  }

  if (vm.count("from-pcap")) {
    debugger.read_pcap(vm["from-pcap"].as<std::string>());
    auto coe_packets = debugger.get_packets_containing_field("ecat_mailbox.coe");
    printf("Found %zd CoE packets\n", coe_packets.size());
    for (auto& packet : coe_packets) {
      std::cout << packet->to_string() << "\n";
    }
  } else {
    std::cout << "Nothing to do (see --help).\n";
    return 1;
  }

  return 0;
}