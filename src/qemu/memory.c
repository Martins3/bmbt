#include "../../include/exec/memory.h"
#include "../../include/exec/ram_addr.h"

/**
 * memory_region_get_ram_addr: Get the ram address associated with a memory
 *                             region
 */
ram_addr_t memory_region_get_ram_addr(MemoryRegion *mr) {
  return mr->ram_block ? mr->ram_block->offset : RAM_ADDR_INVALID;
}
// originally defined in dirty_memory_extend
static void setup_dirty_memory(hwaddr total_ram_size) {
  ram_addr_t new_num_blocks =
      DIV_ROUND_UP(total_ram_size, DIRTY_MEMORY_BLOCK_SIZE);

  DirtyMemoryBlocks *new_blocks = g_malloc(
      sizeof(*new_blocks) + sizeof(new_blocks->blocks[0]) * new_num_blocks);

  for (int j = 0; j < new_num_blocks; j++) {
    new_blocks->blocks[j] = bitmap_new(DIRTY_MEMORY_BLOCK_SIZE);
  }
  ram_list.dirty_memory[DIRTY_MEMORY_CODE] = new_blocks;
}
