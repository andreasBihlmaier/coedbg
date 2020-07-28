#include "esi_parser.h"

#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace coe {

namespace pt = boost::property_tree;

void EsiParser::read_file(const std::string& esi_path) {
  pt::ptree tree;

  pt::read_xml(esi_path, tree);

  for (auto& kv : tree.get_child("EtherCATInfo.Descriptions.Devices.Device.Profile.Dictionary.DataTypes")) {
    auto& tree_datatype = kv.second;
    OdDataType datatype;
    datatype.name = tree_datatype.get<std::string>("Name");
    OdBaseType base_type = od_base_type_from_string(datatype.name);
    if (base_type != OdBaseType::Invalid) {  // DataType definition of a base type
      datatype.base_type = base_type;
    }
    datatype.bit_size = tree_datatype.get<uint32_t>("BitSize");
    auto tree_base_type = tree_datatype.get_optional<std::string>("BaseType");
    if (tree_base_type) {
      datatype.base_type = od_base_type_from_string(*tree_base_type);
    }
    uint last_subindex = 0;
    for (auto& sub_kv : tree_datatype.get_child("")) {
      if (sub_kv.first != "SubItem") {
        continue;
      }
      auto& tree_subdatatype = sub_kv.second;
      OdDataType sub_datatype;
      sub_datatype.name = tree_subdatatype.get<std::string>("Name");
      sub_datatype.type = tree_subdatatype.get<std::string>("Type");
      sub_datatype.bit_size = tree_subdatatype.get<uint32_t>("BitSize");
      sub_datatype.bit_offset = tree_subdatatype.get<uint32_t>("BitOffs");
      auto optional_subindex = tree_subdatatype.get_optional<uint8_t>("SubIdx");
      if (optional_subindex) {
        sub_datatype.subindex = *optional_subindex;
      } else {
        sub_datatype.subindex = last_subindex + 1;
      }
      datatype.add_subtype(sub_datatype);
      last_subindex = sub_datatype.subindex;
    }
    m_od.add_datatype(datatype);
    std::cout << datatype.to_string() << '\n';
  }

  for (auto& kv : tree.get_child("EtherCATInfo.Descriptions.Devices.Device.Profile.Dictionary.Objects")) {
    auto& tree_object = kv.second;
    OdEntry entry;
    auto index_string = tree_object.get<std::string>("Index");
    if (index_string[0] == '#') {
      index_string[0] = '0';
    }
    entry.index = std::stoul(index_string, 0, 16);
    entry.name = tree_object.get<std::string>("Name");
    entry.bit_size = tree_object.get<uint32_t>("BitSize");
    if (!tree_object.get_child_optional("Info.SubItem")) {  // OD entry without subindices
      entry.subindex = 0;
      entry.type_name = tree_object.get<std::string>("Type");
      // TODO entry.type
      auto tree_default_data = tree_object.get_optional<std::string>("Info.DefaultData");
      if (tree_default_data) {
        entry.default_data = *tree_default_data;
      }
      m_od.add_entry(entry);
      std::cout << entry.to_string() << '\n';
    } else {
      int subindex = 0;
      for (auto& sub_kv : tree_object.get_child("Info")) {
        auto& tree_subobject = sub_kv.second;
        OdEntry subentry;
        subentry.index = entry.index;
        subentry.subindex = subindex;
        subentry.name = entry.name + "/" + tree_subobject.get<std::string>("Name");
        // TODO subentry.type
        m_od.add_entry(subentry);
        std::cout << subentry.to_string() << '\n';
        subindex++;
      }
    }
  }
}  // namespace coe

}  // namespace coe