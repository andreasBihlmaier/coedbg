#include "esi_parser.h"

#include <iostream>

#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace coe {

namespace pt = boost::property_tree;

EsiParser::EsiParser(OD* od) : m_od(od) {
}

boost::optional<std::string> get_default_data(const pt::ptree& tree) {
  auto tree_default_data = tree.get_optional<std::string>("Info.DefaultData");
  auto tree_default_value = tree.get_optional<std::string>("Info.DefaultValue");
  if (tree_default_data && tree_default_value) {
    throw std::runtime_error("Item contains both Info.DefaultData and Info.DefaultValue");
  }
  if (tree_default_value) {
    tree_default_data = tree_default_value;
  }
  if (tree_default_data && tree_default_data->rfind("#x", 0) == 0) {
    (*tree_default_data)[0] = '0';
  }
  return tree_default_data;
}  // namespace coe

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
    auto tree_array_info = tree_datatype.get_child_optional("ArrayInfo");
    if (tree_array_info) {
      datatype.array_lbound = tree_array_info->get<uint8_t>("LBound");
      datatype.array_elements = tree_array_info->get<uint8_t>("Elements");
    }
    uint last_subindex = 0;
    for (auto& sub_kv : tree_datatype.get_child("")) {
      if (sub_kv.first != "SubItem") {
        continue;
      }
      auto& tree_subdatatype = sub_kv.second;
      OdDataType sub_datatype;
      sub_datatype.name = tree_subdatatype.get<std::string>("Name");
      sub_datatype.abstract_type = tree_subdatatype.get<std::string>("Type");
      sub_datatype.base_type = m_od->get_type(sub_datatype.abstract_type).base_type;
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
    m_od->add_datatype(datatype);
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
    entry.type_name = tree_object.get<std::string>("Type");
    if (!tree_object.get_child_optional("Info.SubItem")) {  // OD entry without subindices
      entry.subindex = 0;
      entry.type = m_od->get_type(entry.type_name).base_type;
      auto tree_default_data = get_default_data(tree_object);
      if (tree_default_data) {
        entry.default_data_string = *tree_default_data;
        entry.set_value_from_default_data();
      }
      m_od->add_entry(entry);
    } else {
      uint8_t subindex = 0;
      for (auto& sub_kv : tree_object.get_child("Info")) {
        auto& tree_subobject = sub_kv.second;
        OdEntry subentry;
        subentry.index = entry.index;
        std::string subentry_name = tree_subobject.get<std::string>("Name");
        subentry.name = entry.name + "/" + subentry_name;
        OdDataType subentry_datatype = m_od->get_type(entry.type_name, subentry_name);
        subentry.type = subentry_datatype.base_type;
        if (subentry_datatype.name == "Elements") {
          OdDataType elements_subentry_datatype = m_od->get_type(subentry_datatype.abstract_type);
          if (elements_subentry_datatype.array_elements == 0) {
            throw std::runtime_error(
                (boost::format("Elements DataType for (0x%04x, %d) is missing ArrayInfo (array_elements=0)") %
                 entry.index % (unsigned int)subindex)
                    .str());
          }
          subentry.bit_size = elements_subentry_datatype.bit_size / elements_subentry_datatype.array_elements;
          subentry.subindex = subindex;
        } else {
          subentry.bit_size = subentry_datatype.bit_size;
          subentry.subindex = subentry_datatype.subindex;
        }
        auto tree_default_data = get_default_data(tree_subobject);
        if (tree_default_data) {
          subentry.default_data_string = *tree_default_data;
          subentry.set_value_from_default_data();
        }
        m_od->add_entry(subentry);
        subindex++;
      }
    }
  }
  m_od->update_name_cache();
  m_od->update_pdo_mapping_cache();
}

}  // namespace coe