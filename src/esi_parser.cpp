#include "esi_parser.h"

#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace coe {

namespace pt = boost::property_tree;

void EsiParser::read_file(const std::string& esi_path) {
  pt::ptree tree;

  pt::read_xml(esi_path, tree);

  // TODO Parse Dictionary.DataTypes

  for (auto& kv : tree.get_child("EtherCATInfo.Descriptions.Devices.Device.Profile.Dictionary.Objects")) {
    auto& tree_object = kv.second;
    auto index_string = tree_object.get<std::string>("Index");
    if (index_string[0] == '#') {
      index_string[0] = '0';
    }
    OdEntry entry;
    entry.index = std::stoul(index_string, 0, 16);
    entry.name = tree_object.get<std::string>("Name");
    if (!tree_object.get_child_optional("Info.SubItem")) {  // OD entry without subindices
      entry.subindex = 0;
      entry.type_name = tree_object.get<std::string>("Type");
      // TODO entry.type
      auto tree_default_data = tree_object.get_optional<std::string>("Info.DefaultData");
      if (tree_default_data) {
        entry.default_data = *tree_default_data;
      }
      m_od.add(entry);
      std::cout << entry.to_string() << '\n';
    } else {
      int subindex = 0;
      for (auto& sub_kv : tree_object.get_child("Info")) {
        auto& tree_subobject = sub_kv.second;
        OdEntry subentry;
        subentry.index = entry.index;
        subentry.subindex = subindex;
        subentry.name = tree_subobject.get<std::string>("Name");
        // TODO subentry.type
        m_od.add(subentry);
        std::cout << subentry.to_string() << '\n';
        subindex++;
      }
    }
  }
}

}  // namespace coe