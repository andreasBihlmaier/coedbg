#include "coe_packet.h"

#include <stdexcept>

#include <libwireshark/ws_dissect.h>

namespace coe {

void CoePacket::add_field(const CoeField &field) {
  if (m_fields.find(field.get_name()) != m_fields.end()) {
    throw std::runtime_error("Field " + field.get_name() + " contained multiple times in same packet");
  }
  m_fields[field.get_name()] = field;
}

std::string CoePacket::to_string() const {
  std::string str;

  for (auto entry : m_fields) {
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
  if (abbrev != "text" && abbrev != "eth.addr" && abbrev != "eth.addr_resolved" && abbrev != "eth.lg" &&
      abbrev != "eth.ig") {  // These fields exist on multiple levels, thus breaking flattend structure.
                             // Ignore them as they are not needed.
    packet->add_field(CoeField::create_from_fieldinfo(fieldinfo));
  }

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

}  // namespace coe