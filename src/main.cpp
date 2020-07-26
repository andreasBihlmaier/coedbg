#include <iostream>

#include <boost/program_options.hpp>

#include "coedbg.h"

int main(int argc, char** argv) {
  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()                 //
      ("help", "This help message")  //
      ("from-pcap", po::value<std::string>(), "PCAP file to read");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  coe::CoeDebugger debugger;

  if (vm.count("from-pcap")) {
    debugger.read_pcap(vm["from-pcap"].as<std::string>());
  } else {
    std::cout << "Nothing to do (see --help).\n";
    return 1;
  }

  return 0;
}