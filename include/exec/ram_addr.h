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
  ram_addr_t length; // size of ram

} RAMBlock;

typedef struct RAMList {
  DirtyMemoryBlocks *dirty_memory[DIRTY_MEMORY_NUM];
  RAMBlock * mru_block; // [interface 10]
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

static inline bool cpu_physical_memory_get_dirty(ram_addr_t start,
                                                 ram_addr_t length,
                                                 unsigned client)
{
    DirtyMemoryBlocks *blocks;
    unsigned long end, page;
    unsigned long idx, offset, base;
    bool dirty = false;

    assert(client < DIRTY_MEMORY_NUM);

    end = TARGET_PAGE_ALIGN(start + length) >> TARGET_PAGE_BITS;
    page = start >> TARGET_PAGE_BITS;

    WITH_RCU_READ_LOCK_GUARD() {
        blocks = atomic_rcu_read(&ram_list.dirty_memory[client]);

        idx = page / DIRTY_MEMORY_BLOCK_SIZE;
        offset = page % DIRTY_MEMORY_BLOCK_SIZE;
        base = page - offset;
        while (page < end) {
            unsigned long next = MIN(end, base + DIRTY_MEMORY_BLOCK_SIZE);
            unsigned long num = next - base;
            unsigned long found = find_next_bit(blocks->blocks[idx],
                                                num, offset);
            if (found < num) {
                dirty = true;
                break;
            }

            page = next;
            idx++;
            offset = 0;
            base += DIRTY_MEMORY_BLOCK_SIZE;
        }
    }

    return dirty;
}

static inline bool cpu_physical_memory_get_dirty_flag(ram_addr_t addr,
                                                      unsigned client) {
  return cpu_physical_memory_get_dirty(addr, 1, client);
}

static inline void cpu_physical_memory_set_dirty_range(ram_addr_t start, ram_addr_t length,
                                         uint8_t mask) {
    DirtyMemoryBlocks *blocks[DIRTY_MEMORY_NUM];
    unsigned long end, page;
    unsigned long idx, offset, base;
    int i;

    if (!mask) {
        return;
    }

    end = TARGET_PAGE_ALIGN(start + length) >> TARGET_PAGE_BITS;
    page = start >> TARGET_PAGE_BITS;

    WITH_RCU_READ_LOCK_GUARD() {
        for (i = 0; i < DIRTY_MEMORY_NUM; i++) {
            blocks[i] = atomic_rcu_read(&ram_list.dirty_memory[i]);
        }

        idx = page / DIRTY_MEMORY_BLOCK_SIZE;
        offset = page % DIRTY_MEMORY_BLOCK_SIZE;
        base = page - offset;
        while (page < end) {
            unsigned long next = MIN(end, base + DIRTY_MEMORY_BLOCK_SIZE);

            if (likely(mask & (1 << DIRTY_MEMORY_MIGRATION))) {
                bitmap_set_atomic(blocks[DIRTY_MEMORY_MIGRATION]->blocks[idx],
                                  offset, next - page);
            }
            if (unlikely(mask & (1 << DIRTY_MEMORY_VGA))) {
                bitmap_set_atomic(blocks[DIRTY_MEMORY_VGA]->blocks[idx],
                                  offset, next - page);
            }
            if (unlikely(mask & (1 << DIRTY_MEMORY_CODE))) {
                bitmap_set_atomic(blocks[DIRTY_MEMORY_CODE]->blocks[idx],
                                  offset, next - page);
            }

            page = next;
            idx++;
            offset = 0;
            base += DIRTY_MEMORY_BLOCK_SIZE;
        }
    }
}


static inline bool cpu_physical_memory_is_clean(ram_addr_t addr) {
  bool vga = cpu_physical_memory_get_dirty_flag(addr, DIRTY_MEMORY_VGA);
  bool code = cpu_physical_memory_get_dirty_flag(addr, DIRTY_MEMORY_CODE);
  bool migration =
      cpu_physical_memory_get_dirty_flag(addr, DIRTY_MEMORY_MIGRATION);
  return !(vga && code && migration);
}

static inline bool offset_in_ramblock(RAMBlock *b, ram_addr_t offset) {
  return (b && b->host && offset < b->length) ? true : false;
}

static inline void *ramblock_ptr(RAMBlock *block, ram_addr_t offset) {
  assert(offset_in_ramblock(block, offset));
  return (char *)block->host + offset;
}

#endif /* end of include guard: RAM_ADDR_H_ACNMERX5 */
