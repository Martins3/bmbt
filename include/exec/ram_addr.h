#ifndef RAM_ADDR_H_ACNMERX5
#define RAM_ADDR_H_ACNMERX5
#include "../qemu/atomic.h"
#include "../qemu/bitmap.h"
#include "../qemu/rcu.h"
#include "cpu-all.h"
#include "cpu-common.h"
#include "cpu-para.h"
#include "ramlist.h"
#include <assert.h>
#include <hw/pci-host/pam.h>
#include <qemu/units.h>

typedef struct RAMBlock {
  uint8_t *host;
  ram_addr_t offset;     // offset in ram list
  ram_addr_t max_length; // same with MemoryRegion::size
  MemoryRegion *mr;
} RAMBlock;

// pc.ram low
#define PC_LOW_RAM_INDEX 0
// smram
#define SMRAM_INDEX 1
// pam
#define PAM_INDEX 2
// pam expan
#define PAM_EXPAN_INDEX PAM_INDEX
#define PAM_EXPAN_NUM 8
// pam extend
#define PAM_EXBIOS_INDEX (PAM_EXPAN_INDEX + PAM_EXPAN_NUM)
#define PAM_EXBIOS_NUM 4
// pam system bios
#define PAM_BIOS_INDEX (PAM_EXBIOS_INDEX + PAM_EXBIOS_NUM)
#define PAM_BIOS_NUM 1
#define PAM_NUM (PAM_EXPAN_NUM + PAM_EXBIOS_NUM + PAM_BIOS_NUM)
// pc.ram
#define PC_RAM_INDEX (PAM_INDEX + PAM_NUM)
// pc.bios
#define PC_BIOS_INDEX (PC_RAM_INDEX + 1)

#define RAM_BLOCK_NUM (PC_BIOS_INDEX + 1)
#define X86_BIOS_MEM_SIZE (PAM_BIOS_END + 1)
#define PC_RAM_SIZE (CONFIG_GUEST_RAM_SIZE - X86_BIOS_MEM_SIZE)

/**
 * pc.ram low  * 1
 * smram       * 1
 * 0x4000 pam  * 12
 *  * pam expan  * 8
 *  * pam extend * 4 ----\ pc.bios 128k
 * system bios * 1   ----/
 * pc.ram      * 1
 * pc.bios     * 1
 */
QEMU_BUILD_BUG_ON(RAM_BLOCK_NUM != (1 + 1 + 12 + 1 + 1 + 1));
QEMU_BUILD_BUG_ON(PAM_EXBIOS_SIZE * 4 + PAM_BIOS_SIZE != 128 * KiB);

typedef struct RAMList {
  DirtyMemoryBlocks *dirty_memory[DIRTY_MEMORY_NUM];

  /* RCU-enabled, writes protected by the ramlist lock. */
  struct {
    RAMBlock block;
    MemoryRegion mr;
  } blocks[RAM_BLOCK_NUM];
  RAMBlock *mru_block;
} RAMList;

extern RAMList ram_list;

static inline MemoryRegion *get_ram_mr(unsigned idx) {
  return &ram_list.blocks[idx].mr;
}

#define RAMBLOCK_FOREACH(block)                                                \
  int __i;                                                                     \
  for (__i = 0, block = &ram_list.blocks[__i].block; __i < RAM_BLOCK_NUM;      \
       __i = __i + 1, block = &ram_list.blocks[__i].block)

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

  bmbt_check(client == DIRTY_MEMORY_CODE);
  blocks = qatomic_rcu_read(&ram_list.dirty_memory[client]);

  set_bit_atomic(offset, blocks->blocks[idx]);
}

static inline bool cpu_physical_memory_get_dirty(ram_addr_t start,
                                                 ram_addr_t length,
                                                 unsigned client) {
  DirtyMemoryBlocks *blocks;
  unsigned long end, page;
  unsigned long idx, offset, base;
  bool dirty = false;

  assert(client < DIRTY_MEMORY_NUM);

  end = TARGET_PAGE_ALIGN(start + length) >> TARGET_PAGE_BITS;
  page = start >> TARGET_PAGE_BITS;

  WITH_RCU_READ_LOCK_GUARD() {
    bmbt_check(client == DIRTY_MEMORY_CODE);
    blocks = qatomic_rcu_read(&ram_list.dirty_memory[client]);

    idx = page / DIRTY_MEMORY_BLOCK_SIZE;
    offset = page % DIRTY_MEMORY_BLOCK_SIZE;
    base = page - offset;
    while (page < end) {
      unsigned long next = MIN(end, base + DIRTY_MEMORY_BLOCK_SIZE);
      unsigned long num = next - base;
      unsigned long found = find_next_bit(blocks->blocks[idx], num, offset);
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

static inline void cpu_physical_memory_set_dirty_range(ram_addr_t start,
                                                       ram_addr_t length,
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
      blocks[i] = qatomic_rcu_read(&ram_list.dirty_memory[i]);
    }

    idx = page / DIRTY_MEMORY_BLOCK_SIZE;
    offset = page % DIRTY_MEMORY_BLOCK_SIZE;
    base = page - offset;
    while (page < end) {
      unsigned long next = MIN(end, base + DIRTY_MEMORY_BLOCK_SIZE);

#ifdef BMBT
      if (unlikely(mask & (1 << DIRTY_MEMORY_MIGRATION))) {
        bitmap_set_atomic(blocks[DIRTY_MEMORY_MIGRATION]->blocks[idx], offset,
                          next - page);
      }
      if (unlikely(mask & (1 << DIRTY_MEMORY_VGA))) {
        bitmap_set_atomic(blocks[DIRTY_MEMORY_VGA]->blocks[idx], offset,
                          next - page);
      }
#endif
      if (likely(mask & (1 << DIRTY_MEMORY_CODE))) {
        bitmap_set_atomic(blocks[DIRTY_MEMORY_CODE]->blocks[idx], offset,
                          next - page);
      } else {
        g_assert_not_reached();
      }

      page = next;
      idx++;
      offset = 0;
      base += DIRTY_MEMORY_BLOCK_SIZE;
    }
  }
}

static inline bool cpu_physical_memory_is_clean(ram_addr_t addr) {
#ifdef BMBT
  bool vga = cpu_physical_memory_get_dirty_flag(addr, DIRTY_MEMORY_VGA);
  bool code = cpu_physical_memory_get_dirty_flag(addr, DIRTY_MEMORY_CODE);
  bool migration =
      cpu_physical_memory_get_dirty_flag(addr, DIRTY_MEMORY_MIGRATION);
  return !(vga && code && migration);
#endif
  return !cpu_physical_memory_get_dirty_flag(addr, DIRTY_MEMORY_CODE);
}

static inline bool offset_in_ramblock(RAMBlock *b, ram_addr_t offset) {
  return (b && b->host && offset < b->max_length) ? true : false;
}

static inline void *ramblock_ptr(RAMBlock *block, ram_addr_t offset) {
  assert(offset_in_ramblock(block, offset));
  return (char *)block->host + offset;
}

static inline bool cpu_physical_memory_all_dirty(ram_addr_t start,
                                                 ram_addr_t length,
                                                 unsigned client) {
  DirtyMemoryBlocks *blocks;
  unsigned long end, page;
  unsigned long idx, offset, base;
  bool dirty = true;

  assert(client < DIRTY_MEMORY_NUM);

  end = TARGET_PAGE_ALIGN(start + length) >> TARGET_PAGE_BITS;
  page = start >> TARGET_PAGE_BITS;

  RCU_READ_LOCK_GUARD();

  blocks = qatomic_rcu_read(&ram_list.dirty_memory[client]);

  idx = page / DIRTY_MEMORY_BLOCK_SIZE;
  offset = page % DIRTY_MEMORY_BLOCK_SIZE;
  base = page - offset;
  while (page < end) {
    unsigned long next = MIN(end, base + DIRTY_MEMORY_BLOCK_SIZE);
    unsigned long num = next - base;
    unsigned long found = find_next_zero_bit(blocks->blocks[idx], num, offset);
    if (found < num) {
      dirty = false;
      break;
    }

    page = next;
    idx++;
    offset = 0;
    base += DIRTY_MEMORY_BLOCK_SIZE;
  }

  return dirty;
}

static inline uint8_t
cpu_physical_memory_range_includes_clean(ram_addr_t start, ram_addr_t length,
                                         uint8_t mask) {
  uint8_t ret = 0;
  bmbt_check((mask & (1 << DIRTY_MEMORY_CODE)) == (1 << DIRTY_MEMORY_CODE));

#ifdef BMBT
  if (mask & (1 << DIRTY_MEMORY_VGA) &&
      !cpu_physical_memory_all_dirty(start, length, DIRTY_MEMORY_VGA)) {
    ret |= (1 << DIRTY_MEMORY_VGA);
  }
#endif
  if (mask & (1 << DIRTY_MEMORY_CODE) &&
      !cpu_physical_memory_all_dirty(start, length, DIRTY_MEMORY_CODE)) {
    ret |= (1 << DIRTY_MEMORY_CODE);
  }

#ifdef BMBT
  if (mask & (1 << DIRTY_MEMORY_MIGRATION) &&
      !cpu_physical_memory_all_dirty(start, length, DIRTY_MEMORY_MIGRATION)) {
    ret |= (1 << DIRTY_MEMORY_MIGRATION);
  }
#endif
  return ret;
}

void tb_invalidate_phys_range(ram_addr_t start, ram_addr_t end);
#endif /* end of include guard: RAM_ADDR_H_ACNMERX5 */
