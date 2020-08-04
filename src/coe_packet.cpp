#include "coe_packet.h"

#include <stdexcept>

#include <libwireshark/ws_dissect.h>

namespace coe {

void CoePacket::add_field(const CoeField &field) {
  std::string field_name = field.get_name();
  int field_name_count = 0;
  while (m_fields.find(field_name) != m_fields.end()) {
    field_name_count++;
    field_name = field.get_name() + "." + std::to_string(field_name_count);
  }
  m_fields[field_name] = field;
}

const CoeField *CoePacket::get_field(const std::string &field_name) const {
  const CoeField *field = nullptr;
  auto field_kv = m_fields.find(field_name);
  if (field_kv != m_fields.end()) {
    field = &field_kv->second;
  }
  return field;
}

uint32_t CoePacket::get_number() const {
  return m_number;
}

void CoePacket::set_number(uint32_t number) {
  m_number = number;
}

std::string CoePacket::to_string() const {
  std::string str;

  str = "packet#" + std::to_string(m_number) + ": ";
  for (auto &entry : m_fields) {
    str += "(" + entry.second.to_string() + ")";
  }

  return str;
}

static void foreach_add_fields_to_packet(proto_node *node, gpointer data) {
  CoePacket *packet = static_cast<CoePacket *>(data);

  if (node == NULL) {
    throw std::runtime_error("node is NULL");
  }
  field_info *fieldinfo = node->finfo;
  if (fieldinfo == NULL) {
    throw std::runtime_error("fieldinfo is NULL");
  }
  header_field_info *headerfieldinfo = fieldinfo->hfinfo;
  if (headerfieldinfo == NULL) {
    throw std::runtime_error("headerfieldinfo is NULL");
  }
  // printf("%s", headerfieldinfo->name);
  if (headerfieldinfo->abbrev == NULL) {
    throw std::runtime_error("headerfieldinfo->abbrev is NULL");
    return;
  }
  std::string abbrev{headerfieldinfo->abbrev};
  packet->add_field(CoeField::create_from_fieldinfo(fieldinfo));

  if (node->first_child != NULL) {
    proto_tree_children_foreach(node, foreach_add_fields_to_packet, packet);
  }
}

CoePacket CoePacket::create_from_dissection(ws_dissection *dissection) {
  CoePacket packet;

  epan_dissect_t *edt = dissection->edt;
  if (edt == NULL) {
    throw std::runtime_error("edt is NULL");
  }
  proto_tree *tree = edt->tree;
  if (tree == NULL) {
    throw std::runtime_error("tree is NULL");
  }
  proto_tree_children_foreach(tree, foreach_add_fields_to_packet, &packet);

  return packet;
}

bool CoePacket::contains_field(const std::string &field_name) const {
  return m_fields.find(field_name) != m_fields.end();
}

bool CoePacket::was_sent_from_master() const {
  return get_field("eth.src")->get_value<std::vector<uint8_t>>() == MasterSourceAddress;
}

bool CoePacket::was_sent_from_slave() const {
  return get_field("eth.src")->get_value<std::vector<uint8_t>>() == SlaveSourceAddress;
}

bool CoePacket::is_sdo_type(SdoType sdo_type) const {
  return static_cast<SdoType>(get_field("ecat_mailbox.coe.type")->get_value<uint16_t>()) == sdo_type;
}

bool CoePacket::is_client_command_specifier(SdoClientCommandSpecifier sdo_ccs) const {
  return static_cast<SdoClientCommandSpecifier>(get_field("ecat_mailbox.coe.sdoccsiu")->get_value<uint8_t>() >> 5) ==
         sdo_ccs;
}

bool CoePacket::is_server_command_specifier(SdoServerCommandSpecifier sdo_scs) const {
  return static_cast<SdoServerCommandSpecifier>(get_field("ecat_mailbox.coe.sdoscsiu")->get_value<uint8_t>() >> 5) ==
         sdo_scs;
}

bool CoePacket::is_sdo() const {
  return contains_field("ecat_mailbox.coe");
}

bool CoePacket::is_pdo() const {
  return (contains_field("ecat.lad") &&
          get_field("ecat.cmd")->get_value<uint8_t>() ==
              static_cast<std::underlying_type_t<EtherCatCommand>>(EtherCatCommand::LogicalReadWrite));
}

bool CoePacket::is_complete_access() const {
  return ((contains_field("ecat_mailbox.coe.sdoscsiu_complete") &&
           get_field("ecat_mailbox.coe.sdoscsiu_complete")->get_value<bool>()) ||
          (contains_field("ecat_mailbox.coe.sdoccsid.complete") &&
           get_field("ecat_mailbox.coe.sdoccsid.complete")->get_value<bool>()));
}

}  // namespace coe