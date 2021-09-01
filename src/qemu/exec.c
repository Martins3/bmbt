#include "../../include/exec/cpu-all.h"
#include "../../include/exec/exec-all.h"
#include "../../include/exec/memory.h"
#include "../../include/hw/core/cpu.h"
#include "../../include/qemu/error-report.h"
#include <errno.h>

// FIXME maybe init it as zero
/* 0 = Do not count executed instructions.
   1 = Precise instruction counting.
   2 = Adaptive rate instruction counting.  */
int use_icount;

static void breakpoint_invalidate(CPUState *cpu, target_ulong pc) {
  MemTxAttrs attrs;
  hwaddr phys = cpu_get_phys_page_attrs_debug(cpu, pc, &attrs);
  int asidx = cpu_asidx_from_attrs(cpu, attrs);
  if (phys != -1) {
    /* Locks grabbed by tb_invalidate_phys_addr */
    tb_invalidate_phys_addr(cpu->cpu_ases[asidx].as,
                            phys | (pc & ~TARGET_PAGE_MASK), attrs);
  }
}

#ifndef CONFIG_USER_ONLY
/* Add a watchpoint.  */
int cpu_watchpoint_insert(CPUState *cpu, vaddr addr, vaddr len, int flags,
                          CPUWatchpoint **watchpoint) {
  CPUWatchpoint *wp;

  /* forbid ranges which are empty or run off the end of the address space */
  if (len == 0 || (addr + len - 1) < addr) {
    error_report("tried to set invalid watchpoint at %" VADDR_PRIx
                 ", len=%" VADDR_PRIu,
                 addr, len);
    return -EINVAL;
  }
  wp = g_malloc(sizeof(*wp));

  wp->vaddr = addr;
  wp->len = len;
  wp->flags = flags;

  /* keep all GDB-injected watchpoints in front */
  if (flags & BP_GDB) {
    QTAILQ_INSERT_HEAD(&cpu->watchpoints, wp, entry);
  } else {
    QTAILQ_INSERT_TAIL(&cpu->watchpoints, wp, entry);
  }

  tlb_flush_page(cpu, addr);

  if (watchpoint)
    *watchpoint = wp;
  return 0;
}

/* Remove a specific watchpoint.  */
int cpu_watchpoint_remove(CPUState *cpu, vaddr addr, vaddr len, int flags) {
  CPUWatchpoint *wp;

  QTAILQ_FOREACH(wp, &cpu->watchpoints, entry) {
    if (addr == wp->vaddr && len == wp->len &&
        flags == (wp->flags & ~BP_WATCHPOINT_HIT)) {
      cpu_watchpoint_remove_by_ref(cpu, wp);
      return 0;
    }
  }
  return -ENOENT;
}

/* Remove a specific watchpoint by reference.  */
void cpu_watchpoint_remove_by_ref(CPUState *cpu, CPUWatchpoint *watchpoint) {
  QTAILQ_REMOVE(&cpu->watchpoints, watchpoint, entry);

  tlb_flush_page(cpu, watchpoint->vaddr);

  g_free(watchpoint);
}

/* Remove all matching watchpoints.  */
void cpu_watchpoint_remove_all(CPUState *cpu, int mask) {
  CPUWatchpoint *wp, *next;

  QTAILQ_FOREACH_SAFE(wp, &cpu->watchpoints, entry, next) {
    if (wp->flags & mask) {
      cpu_watchpoint_remove_by_ref(cpu, wp);
    }
  }
}

/* Return true if this watchpoint address matches the specified
 * access (ie the address range covered by the watchpoint overlaps
 * partially or completely with the address range covered by the
 * access).
 */
static inline bool watchpoint_address_matches(CPUWatchpoint *wp, vaddr addr,
                                              vaddr len) {
  /* We know the lengths are non-zero, but a little caution is
   * required to avoid errors in the case where the range ends
   * exactly at the top of the address space and so addr + len
   * wraps round to zero.
   */
  vaddr wpend = wp->vaddr + wp->len - 1;
  vaddr addrend = addr + len - 1;

  return !(addr > wpend || wp->vaddr > addrend);
}

/* Return flags for watchpoints that match addr + prot.  */
int cpu_watchpoint_address_matches(CPUState *cpu, vaddr addr, vaddr len) {
  CPUWatchpoint *wp;
  int ret = 0;

  QTAILQ_FOREACH(wp, &cpu->watchpoints, entry) {
    if (watchpoint_address_matches(wp, addr, TARGET_PAGE_SIZE)) {
      ret |= wp->flags;
    }
  }
  return ret;
}
#endif /* !CONFIG_USER_ONLY */

/* Add a breakpoint.  */
int cpu_breakpoint_insert(CPUState *cpu, vaddr pc, int flags,
                          CPUBreakpoint **breakpoint) {
  CPUBreakpoint *bp;

  bp = g_malloc(sizeof(*bp));

  bp->pc = pc;
  bp->flags = flags;

  /* keep all GDB-injected breakpoints in front */
  if (flags & BP_GDB) {
    QTAILQ_INSERT_HEAD(&cpu->breakpoints, bp, entry);
  } else {
    QTAILQ_INSERT_TAIL(&cpu->breakpoints, bp, entry);
  }

  breakpoint_invalidate(cpu, pc);

  if (breakpoint) {
    *breakpoint = bp;
  }
  return 0;
}

/* Remove a specific breakpoint.  */
int cpu_breakpoint_remove(CPUState *cpu, vaddr pc, int flags) {
  CPUBreakpoint *bp;

  QTAILQ_FOREACH(bp, &cpu->breakpoints, entry) {
    if (bp->pc == pc && bp->flags == flags) {
      cpu_breakpoint_remove_by_ref(cpu, bp);
      return 0;
    }
  }
  return -ENOENT;
}

/* Remove a specific breakpoint by reference.  */
void cpu_breakpoint_remove_by_ref(CPUState *cpu, CPUBreakpoint *breakpoint) {
  QTAILQ_REMOVE(&cpu->breakpoints, breakpoint, entry);

  breakpoint_invalidate(cpu, breakpoint->pc);

  g_free(breakpoint);
}

/* Remove all matching breakpoints. */
void cpu_breakpoint_remove_all(CPUState *cpu, int mask) {
  CPUBreakpoint *bp, *next;

  QTAILQ_FOREACH_SAFE(bp, &cpu->breakpoints, entry, next) {
    if (bp->flags & mask) {
      cpu_breakpoint_remove_by_ref(cpu, bp);
    }
  }
}

#if 0
/* enable or disable single step mode. EXCP_DEBUG is returned by the
   CPU loop after each instruction */
void cpu_single_step(CPUState *cpu, int enabled) {
  if (cpu->singlestep_enabled != enabled) {
    cpu->singlestep_enabled = enabled;
    if (kvm_enabled()) {
      // kvm_update_guest_debug(cpu, 0);
    } else {
      /* must flush all the translated code to avoid inconsistencies */
      /* XXX: only flush what is necessary */
      tb_flush(cpu);
    }
  }
}
#endif

// FIXME trace it
uintptr_t qemu_host_page_size;
intptr_t qemu_host_page_mask;

void page_size_init(void) {
  /* NOTE: we can always suppose that qemu_host_page_size >=
     TARGET_PAGE_SIZE */
  if (qemu_host_page_size == 0) {
    qemu_host_page_size = qemu_real_host_page_size;
  }
  if (qemu_host_page_size < TARGET_PAGE_SIZE) {
    qemu_host_page_size = TARGET_PAGE_SIZE;
  }
  qemu_host_page_mask = -(intptr_t)qemu_host_page_size;
}

MemoryRegion *iotlb_to_section(CPUState *cpu,
                                      hwaddr index, MemTxAttrs attrs)
{
    // [interface 6]
    int asidx = cpu_asidx_from_attrs(cpu, attrs);
    CPUAddressSpace *cpuas = &cpu->cpu_ases[asidx];
    // AddressSpaceDispatch *d = atomic_rcu_read(&cpuas->memory_dispatch);
    MemoryRegion *sections = cpuas->as->segments;
    return &sections[index & ~TARGET_PAGE_MASK];
}

static void tlb_reset_dirty_range_all(ram_addr_t start, ram_addr_t length)
{
    CPUState *cpu;
    ram_addr_t start1;
    RAMBlock *block;
    ram_addr_t end;

    assert(tcg_enabled());
    end = TARGET_PAGE_ALIGN(start + length);
    start &= TARGET_PAGE_MASK;

    RCU_READ_LOCK_GUARD();
    block = qemu_get_ram_block(start);
    assert(block == qemu_get_ram_block(end - 1));
    start1 = (uintptr_t)ramblock_ptr(block, start - block->offset);
    CPU_FOREACH(cpu) {
        tlb_reset_dirty(cpu, start1, length);
    }
}

/* Note: start and end must be within the same ram block.  */
inline bool cpu_physical_memory_test_and_clear_dirty(ram_addr_t start,
                                                            ram_addr_t length,
                                                            unsigned client) {
  DirtyMemoryBlocks *blocks;
  unsigned long end, page;
  bool dirty = false;
  RAMBlock *ramblock;
  uint64_t mr_offset, mr_size;

  if (length == 0) {
    return false;
  }

  end = TARGET_PAGE_ALIGN(start + length) >> TARGET_PAGE_BITS;
  page = start >> TARGET_PAGE_BITS;

  WITH_RCU_READ_LOCK_GUARD() {
    blocks = atomic_rcu_read(&ram_list.dirty_memory[client]);
    ramblock = qemu_get_ram_block(start);
    /* Range sanity check on the ramblock */
    assert(start >= ramblock->offset &&
           start + length <= ramblock->offset + ramblock->used_length);

    while (page < end) {
      unsigned long idx = page / DIRTY_MEMORY_BLOCK_SIZE;
      unsigned long offset = page % DIRTY_MEMORY_BLOCK_SIZE;
      unsigned long num = MIN(end - page, DIRTY_MEMORY_BLOCK_SIZE - offset);

      dirty |= bitmap_test_and_clear_atomic(blocks->blocks[idx], offset, num);
      page += num;
    }

    mr_offset = (ram_addr_t)(page << TARGET_PAGE_BITS) - ramblock->offset;
    mr_size = (end - page) << TARGET_PAGE_BITS;

    // memory_region_clear_dirty_bitmap(ramblock->mr, mr_offset, mr_size);
  }

  if (dirty && tcg_enabled()) {
    tlb_reset_dirty_range_all(start, length);
  }

  return dirty;
}
