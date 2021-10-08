#include "../../include/sysemu/numa.h"
#include "../../include/hw/boards.h"

void numa_cpu_pre_plug(const struct CPUArchId *slot, struct X86CPU *cpu) {
  int node_id = cpu->node_id;

  if (node_id == CPU_UNSET_NUMA_NODE_ID) {
    /* due to bug in libvirt, it doesn't pass node-id from props on
     * device_add as expected, so we have to fix it up here */
    if (slot->props.has_node_id) {
      g_assert_not_reached();
    }
  } else if (node_id != slot->props.node_id) {
    g_assert_not_reached();
  }
}
