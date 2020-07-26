#include "coe_debugger.h"

#include <unistd.h>
#include <stdexcept>

#include <libwireshark/ws_capture.h>
#include <libwireshark/ws_dissect.h>

namespace coe {

void CoeDebugger::add_packet(const CoePacket &packet) {
  m_packets.push_back(packet);
}

static void print_field_value(field_info *fieldinfo) {
  header_field_info *headerfieldinfo = fieldinfo->hfinfo;
  if (headerfieldinfo == NULL) {
    printf("headerfieldinfo is NULL\n");
    return;
  }
  switch (headerfieldinfo->type) {
    case FT_UINT8:
      printf("value=0x%02x=%d", (uint8_t)(fieldinfo->value.value.uinteger), (uint8_t)(fieldinfo->value.value.uinteger));
      break;
    case FT_UINT16:
      printf("value=0x%04x=%d", (uint16_t)(fieldinfo->value.value.uinteger),
             (uint16_t)(fieldinfo->value.value.uinteger));
      break;
    default:
      printf("field type 0x%02x not supported\n", headerfieldinfo->type);
      break;
  }
}

static void foreach_print_fieldname(proto_node *node, gpointer data) {
  (void)data;

  if (node == NULL) {
    printf("node is NULL\n");
    return;
  }
  field_info *fieldinfo = node->finfo;
  if (fieldinfo == NULL) {
    printf("fieldinfo is NULL\n");
    return;
  }
  header_field_info *headerfieldinfo = fieldinfo->hfinfo;
  if (headerfieldinfo == NULL) {
    printf("headerfieldinfo is NULL\n");
    return;
  }
  // if (headerfieldinfo->name == NULL) {
  //	printf("headerfieldinfo->name is NULL\n");
  //	return;
  //}
  // printf("%s", headerfieldinfo->name);
  if (headerfieldinfo->abbrev == NULL) {
    printf("headerfieldinfo->abbrev is NULL\n");
    return;
  }
  printf("%s: type=%s=0x%02x\n", headerfieldinfo->abbrev, ftype_name(headerfieldinfo->type), headerfieldinfo->type);
  print_field_value(fieldinfo);
  printf("\n");

  if (node->first_child != NULL) {
    proto_tree_children_foreach(node, foreach_print_fieldname, NULL);
  }
}

void CoeDebugger::read_pcap(const std::string &pcap_path) {
  if (access(pcap_path.c_str(), R_OK) == -1) {
    throw std::runtime_error("PCAP file cannot be read.");
    return;
  }

  ws_capture_init();
  ws_dissect_init();

  int err_code;
  char *err_info;
  ws_capture_t *cap = ws_capture_open_offline(pcap_path.c_str(), 0, &err_code, &err_info);
  if (cap == nullptr) {
    throw std::runtime_error("Failed to open PCAP file (" + std::to_string(err_code) + "): " + std::string(err_info));
  }

  ws_dissect_t *dissector = ws_dissect_capture(cap);
  if (dissector == nullptr) {
    throw std::runtime_error("Failed to dissect PCAP file.");
  }

  struct ws_dissection packet;
  while (ws_dissect_next(dissector, &packet, NULL, NULL)) {
    epan_dissect_t *edt = packet.edt;
    if (edt == NULL) {
      printf("edt is NULL\n");
      continue;
    }
    proto_tree *tree = edt->tree;
    if (tree == NULL) {
      printf("tree is NULL\n");
      continue;
    }
    proto_tree_children_foreach(tree, foreach_print_fieldname, NULL);
  }
}

}  // namespace coe