#include "../../include/exec/memory.h"
#include "../../include/exec/ram_addr.h"

/**
 * memory_region_get_ram_addr: Get the ram address associated with a memory
 *                             region
 */
ram_addr_t memory_region_get_ram_addr(MemoryRegion *mr) {
  return mr->ram_block ? mr->ram_block->offset : RAM_ADDR_INVALID;
}
