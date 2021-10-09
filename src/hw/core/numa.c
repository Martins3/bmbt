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

static int max_numa_nodeid; /* Highest specified NUMA node ID, plus one.
                             * For all nodes, nodeid < max_numa_nodeid
                             */

void numa_complete_configuration(MachineState *ms) {
  int i;
  MachineClass *mc = MACHINE_GET_CLASS(ms);
  NodeInfo *numa_info = ms->numa_state->nodes;
  /*
   * If memory hotplug is enabled (slots > 0) but without '-numa'
   * options explicitly on CLI, guestes will break.
   *
   *   Windows: won't enable memory hotplug without SRAT table at all
   *
   *   Linux: if QEMU is started with initial memory all below 4Gb
   *   and no SRAT table present, guest kernel will use nommu DMA ops,
   *   which breaks 32bit hw drivers when memory is hotplugged and
   *   guest tries to use it with that drivers.
   *
   * Enable NUMA implicitly by adding a new NUMA node automatically.
   *
   * Or if MachineClass::auto_enable_numa is true and no NUMA nodes,
   * assume there is just one node with whole RAM.
   */
  if (ms->numa_state->num_nodes == 0 &&
      ((ms->ram_slots > 0 && mc->auto_enable_numa_with_memhp) ||
       mc->auto_enable_numa)) {
    g_assert_not_reached();
  }

  assert(max_numa_nodeid <= MAX_NODES);

  /* No support for sparse NUMA node IDs yet: */
  for (i = max_numa_nodeid - 1; i >= 0; i--) {
    /* Report large node IDs first, to make mistakes easier to spot */
    if (!numa_info[i].present) {
      g_assert_not_reached();
    }
  }

  /* This must be always true if all nodes are present: */
  assert(ms->numa_state->num_nodes == max_numa_nodeid);

  if (ms->numa_state->num_nodes > 0) {
    g_assert_not_reached();
  }
}

void numa_default_auto_assign_ram(MachineClass *mc, NodeInfo *nodes,
                                  int nb_nodes, ram_addr_t size) {
  // numa_default_auto_assign_ram is simplified, this function is useless
  g_assert_not_reached();
}
