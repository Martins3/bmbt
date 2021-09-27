#ifndef NUMA_H_4ZUXICHZ
#define NUMA_H_4ZUXICHZ
#include "../types.h"

struct CPUArchId;

#define MAX_NODES 128
#define NUMA_NODE_UNASSIGNED MAX_NODES
#define NUMA_DISTANCE_MIN 10
#define NUMA_DISTANCE_DEFAULT 20
#define NUMA_DISTANCE_MAX 254
#define NUMA_DISTANCE_UNREACHABLE 255

struct NodeInfo {
  uint64_t node_mem;
  struct HostMemoryBackend *node_memdev;
  bool present;
  uint8_t distance[MAX_NODES];
};

struct NumaNodeMem {
  uint64_t node_mem;
  uint64_t node_plugged_mem;
};

struct NumaState {
  /* Number of NUMA nodes */
  int num_nodes;

  /* Allow setting NUMA distance for different NUMA nodes */
  bool have_numa_distance;

  /* NUMA nodes information */
  struct NodeInfo nodes[MAX_NODES];
};
typedef struct NumaState NumaState;

#endif /* end of include guard: NUMA_H_4ZUXICHZ */
