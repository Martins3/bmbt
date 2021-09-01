#ifndef RAM_ADDR_H_ACNMERX5
#define RAM_ADDR_H_ACNMERX5
#include "../qemu/atomic.h"
#include "../qemu/bitmap.h"
#include "../qemu/rcu.h"
#include "cpu-common.h"
#include "cpu-para.h"
#include "cpu-all.h"
#include "ramlist.h"
#include <assert.h>

typedef struct RAMBlock {
  uint8_t *host;
  ram_addr_t offset;      // is zero
  ram_addr_t used_length; // size of ram

} RAMBlock;

typedef struct RAMList {
  DirtyMemoryBlocks *dirty_memory[DIRTY_MEMORY_NUM];
} RAMList;
extern RAMList ram_list;

/* Called from RCU critical section */
static RAMBlock *qemu_get_ram_block(ram_addr_t addr) {
  // FIXME add the only RAMBlock to RAMList, check the addr in range and return
  // the RAMBlock
  return NULL;
}

bool cpu_physical_memory_test_and_clear_dirty(ram_addr_t start,
                                              ram_addr_t length,
                                              unsigned client);

static inline void cpu_physical_memory_set_dirty_flag(ram_addr_t addr,
                                                      unsigned client) {
  unsigned long page, idx, offset;
  DirtyMemoryBlocks *blocks;

  assert(client < DIRTY_MEMORY_NUM);

  page = addr >> TARGET_PAGE_BITS;
  idx = page / DIRTY_MEMORY_BLOCK_SIZE;
  offset = page % DIRTY_MEMORY_BLOCK_SIZE;

  RCU_READ_LOCK_GUARD();

  blocks = atomic_rcu_read(&ram_list.dirty_memory[client]);

  set_bit_atomic(offset, blocks->blocks[idx]);
}

// FIXME there is one header problem:
// cpu_physical_memory_get_dirty need TARGET_PAGE_ALIGN, which is defined in
// cpu-all.h you can try what if I inlcude cpu-all.h in ram_addr.h it's a loop
// dependency
bool cpu_physical_memory_get_dirty(ram_addr_t start, ram_addr_t length,
                                   unsigned client);

static inline bool cpu_physical_memory_get_dirty_flag(ram_addr_t addr,
                                                      unsigned client) {
  return cpu_physical_memory_get_dirty(addr, 1, client);
}

void cpu_physical_memory_set_dirty_range(ram_addr_t start, ram_addr_t length,
                                         uint8_t mask);

static inline bool cpu_physical_memory_is_clean(ram_addr_t addr) {
  bool vga = cpu_physical_memory_get_dirty_flag(addr, DIRTY_MEMORY_VGA);
  bool code = cpu_physical_memory_get_dirty_flag(addr, DIRTY_MEMORY_CODE);
  bool migration =
      cpu_physical_memory_get_dirty_flag(addr, DIRTY_MEMORY_MIGRATION);
  return !(vga && code && migration);
}

static inline bool offset_in_ramblock(RAMBlock *b, ram_addr_t offset) {
  return (b && b->host && offset < b->used_length) ? true : false;
}

static inline void *ramblock_ptr(RAMBlock *block, ram_addr_t offset) {
  assert(offset_in_ramblock(block, offset));
  return (char *)block->host + offset;
}

#endif /* end of include guard: RAM_ADDR_H_ACNMERX5 */
