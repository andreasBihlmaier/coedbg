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

}  // namespace coe