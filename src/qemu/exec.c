#include "../../include/exec/cpu-all.h"
#include "../../include/exec/exec-all.h"
#include "../../include/exec/memory.h"
#include "../../include/exec/ram_addr.h"
#include "../../include/hw/core/cpu.h"
#include "../../include/qemu/error-report.h"
#include "../tcg/translate-all.h"

#include <errno.h>

/* 0 = Do not count executed instructions.
   1 = Precise instruction counting.
   2 = Adaptive rate instruction counting.  */
int use_icount;

/* current CPU in the current thread. It is only valid inside
   cpu_exec() */
CPUState *current_cpu;

RAMList ram_list;

AddressSpace address_space_io;
AddressSpace address_space_memory;
AddressSpace address_space_smm_memory;

/* Called from RCU critical section */
static RAMBlock *qemu_get_ram_block(ram_addr_t addr) {
  RAMBlock *block;

  block = atomic_rcu_read(&ram_list.mru_block);
  if (block && addr - block->offset < block->max_length) {
    return block;
  }
  RAMBLOCK_FOREACH(block) {
    if (addr - block->offset < block->max_length) {
      goto found;
    }
  }

  fprintf(stderr, "Bad ram offset %" PRIx64 "\n", (uint64_t)addr);
  abort();

found:
  /* It is safe to write mru_block outside the iothread lock.  This
   * is what happens:
   *
   *     mru_block = xxx
   *     rcu_read_unlock()
   *                                        xxx removed from list
   *                  rcu_read_lock()
   *                  read mru_block
   *                                        mru_block = NULL;
   *                                        call_rcu(reclaim_ramblock, xxx);
   *                  rcu_read_unlock()
   *
   * atomic_rcu_set is not needed here.  The block was already published
   * when it was placed into the list.  Here we're just making an extra
   * copy of the pointer.
   */
  ram_list.mru_block = block;
  return block;
}

static void breakpoint_invalidate(CPUState *cpu, target_ulong pc) {
  MemTxAttrs attrs;
  hwaddr phys = cpu_get_phys_page_attrs_debug(cpu, pc, &attrs);
  int asidx = cpu_asidx_from_attrs(cpu, attrs);
  if (phys != -1) {
    duck_check(asidx == 0);
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

#ifdef BMBT
uintptr_t qemu_host_page_size;
intptr_t qemu_host_page_mask;

// [interface 27]
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
#endif

MemoryRegion *iotlb_to_section(CPUState *cpu, hwaddr index, MemTxAttrs attrs) {
  g_assert_not_reached();
}

/* Return a host pointer to ram allocated with qemu_ram_alloc.
 * This should not be used for general purpose DMA.  Use address_space_map
 * or address_space_rw instead. For local memory (e.g. video ram) that the
 * device owns, use memory_region_get_ram_ptr.
 *
 * Called within RCU critical section.
 */
void *qemu_map_ram_ptr(RAMBlock *ram_block, ram_addr_t addr) {
  duck_check(ram_block != NULL);

  return ramblock_ptr(ram_block, addr);
}

static void tlb_reset_dirty_range_all(ram_addr_t start, ram_addr_t length) {
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
  CPU_FOREACH(cpu) { tlb_reset_dirty(cpu, start1, length); }
}

/* Note: start and end must be within the same ram block.  */
bool cpu_physical_memory_test_and_clear_dirty(ram_addr_t start,
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
    duck_check(client == DIRTY_MEMORY_CODE);
    blocks = atomic_rcu_read(&ram_list.dirty_memory[client]);
    ramblock = qemu_get_ram_block(start);
    /* Range sanity check on the ramblock */
    assert(start >= ramblock->offset &&
           start + length <= ramblock->offset + ramblock->max_length);

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

/*
 * Translates a host ptr back to a RAMBlock, a ram_addr and an offset
 * in that RAMBlock.
 *
 * ptr: Host pointer to look up
 * round_offset: If true round the result offset down to a page boundary
 * *ram_addr: set to result ram_addr
 * *offset: set to result offset within the RAMBlock
 *
 * Returns: RAMBlock (or NULL if not found)
 *
 * By the time this function returns, the returned pointer is not protected
 * by RCU anymore.  If the caller is not within an RCU critical section and
 * does not hold the iothread lock, it must have other means of protecting the
 * pointer, such as a reference to the region that includes the incoming
 * ram_addr_t.
 */
RAMBlock *qemu_ram_block_from_host(void *ptr, bool round_offset,
                                   ram_addr_t *offset) {
  RAMBlock *block;
  uint8_t *host = ptr;
#ifdef BMBT
  if (xen_enabled()) {
    ram_addr_t ram_addr;
    RCU_READ_LOCK_GUARD();
    ram_addr = xen_ram_addr_from_mapcache(ptr);
    block = qemu_get_ram_block(ram_addr);
    if (block) {
      *offset = ram_addr - block->offset;
    }
    return block;
  }
#endif

  RCU_READ_LOCK_GUARD();
  block = atomic_rcu_read(&ram_list.mru_block);
  if (block && block->host && host - block->host < block->max_length) {
    goto found;
  }

  RAMBLOCK_FOREACH(block) {
    /* This case append when the block is not mapped. */
    if (block->host == NULL) {
      continue;
    }
    if (host - block->host < block->max_length) {
      goto found;
    }
  }

  return NULL;

found:
  *offset = (host - block->host);
  if (round_offset) {
    *offset &= TARGET_PAGE_MASK;
  }
  return block;
}

ram_addr_t qemu_ram_addr_from_host(void *ptr) {
  RAMBlock *block;
  ram_addr_t offset;

  block = qemu_ram_block_from_host(ptr, false, &offset);
  if (!block) {
    return RAM_ADDR_INVALID;
  }

  return block->offset + offset;
}

#if defined(CONFIG_USER_ONLY)
void tb_invalidate_phys_addr(target_ulong addr) {
  mmap_lock();
  tb_invalidate_phys_page_range(addr, addr + 1);
  mmap_unlock();
}

static void breakpoint_invalidate(CPUState *cpu, target_ulong pc) {
  tb_invalidate_phys_addr(pc);
}
#else
void tb_invalidate_phys_addr(AddressSpace *as, hwaddr addr, MemTxAttrs attrs) {
  ram_addr_t ram_addr;
  MemoryRegion *mr;
  hwaddr l = 1;

  if (!tcg_enabled()) {
    return;
  }

  RCU_READ_LOCK_GUARD();
  mr = address_space_translate(as, addr, &addr, &l, false, attrs);
  if (!(memory_region_is_ram(mr))) {
    return;
  }
  ram_addr = memory_region_get_ram_addr(mr) + addr;
  tb_invalidate_phys_page_range(ram_addr, ram_addr + 1);
}
#endif

void cpu_exec_initfn(CPUState *cpu) {
#ifdef BMBT
  cpu->as = NULL;
  cpu->num_ases = 0;

#ifndef CONFIG_USER_ONLY
  cpu->thread_id = qemu_get_thread_id();
  cpu->memory = system_memory;
  object_ref(OBJECT(cpu->memory));
#endif
#endif
}

void cpu_exec_realizefn(CPUState *cpu) {
  CPUClass *cc = CPU_GET_CLASS(cpu);
  static bool tcg_target_initialized;

  cpu_list_add(cpu);

  if (tcg_enabled() && !tcg_target_initialized) {
    tcg_target_initialized = true;
    cc->tcg_initialize();
  }
  tlb_init(cpu);

  // qemu_plugin_vcpu_init_hook(cpu);

#ifndef CONFIG_USER_ONLY
#ifdef BMBT
  if (qdev_get_vmsd(DEVICE(cpu)) == NULL) {
    vmstate_register(NULL, cpu->cpu_index, &vmstate_cpu_common, cpu);
  }
  if (cc->vmsd != NULL) {
    vmstate_register(NULL, cpu->cpu_index, cc->vmsd, cpu);
  }

  cpu->iommu_notifiers = g_array_new(false, true, sizeof(TCGIOMMUNotifier *));
#endif
#endif
}

static CPUAddressSpace __cpu_ases[2];

void cpu_address_space_init(CPUState *cpu, int asidx, const char *prefix) {
  CPUAddressSpace *newas;
  AddressSpace *as =
      asidx == 0 ? &address_space_memory : &address_space_smm_memory;

  // all address space already initialized memory_map_init
#ifdef BMBT
  char *as_name;

  assert(mr);
  as_name = g_strdup_printf("%s-%d", prefix, cpu->cpu_index);
  address_space_init(as, mr, as_name);
  g_free(as_name);
#endif

  /* Target code should have set num_ases before calling us */
  assert(asidx < cpu->num_ases);
  duck_check(cpu->num_ases == 2);

#ifdef BMBT
  if (asidx == 0) {
    /* address space 0 gets the convenience alias */
    cpu->as = as;
  }

  /* KVM cannot currently support multiple address spaces. */
  assert(asidx == 0 || !kvm_enabled());
#endif

  if (!cpu->cpu_ases) {
    cpu->cpu_ases = &__cpu_ases[0];
  }

  newas = &cpu->cpu_ases[asidx];
  // CPUAddressSpace::cpu is only used in tcg_commit
#ifdef BMBT
  newas->cpu = cpu;
#endif
  newas->as = as;
#ifdef BMBT
  if (tcg_enabled()) {
    newas->tcg_as_listener.log_global_after_sync = tcg_log_global_after_sync;
    newas->tcg_as_listener.commit = tcg_commit;
    memory_listener_register(&newas->tcg_as_listener, as);
  }
#endif
}

void tcg_commit() {
#ifdef BMBT
  CPUAddressSpace *cpuas;
  AddressSpaceDispatch *d;

  assert(tcg_enabled());
  /* since each CPU stores ram addresses in its TLB cache, we must
     reset the modified entries */
  cpuas = container_of(listener, CPUAddressSpace, tcg_as_listener);
  cpu_reloading_memory_map();
  /* The CPU and TLB are protected by the iothread lock.
   * We reload the dispatch pointer now because cpu_reloading_memory_map()
   * may have split the RCU critical section.
   */
  d = address_space_to_dispatch(cpuas->as);
  atomic_rcu_set(&cpuas->memory_dispatch, d);
#endif
  if (first_cpu != NULL)
    tlb_flush(first_cpu);
}

void invalidate_and_set_dirty(MemoryRegion *mr, hwaddr addr, hwaddr length) {
  uint8_t dirty_log_mask = memory_region_get_dirty_log_mask(mr);
  addr += memory_region_get_ram_addr(mr);

  /* No early return if dirty_log_mask is or becomes 0, because
   * cpu_physical_memory_set_dirty_range will still call
   * xen_modified_memory.
   */
  if (dirty_log_mask) {
    dirty_log_mask =
        cpu_physical_memory_range_includes_clean(addr, length, dirty_log_mask);
  }
  if (dirty_log_mask & (1 << DIRTY_MEMORY_CODE)) {
    assert(tcg_enabled());
    tb_invalidate_phys_range(addr, addr + length);
    dirty_log_mask &= ~(1 << DIRTY_MEMORY_CODE);
  }
  cpu_physical_memory_set_dirty_range(addr, length, dirty_log_mask);
}

/* Generate a debug exception if a watchpoint has been hit.  */
void cpu_check_watchpoint(CPUState *cpu, vaddr addr, vaddr len,
                          MemTxAttrs attrs, int flags, uintptr_t ra) {
  CPUClass *cc = CPU_GET_CLASS(cpu);
  CPUWatchpoint *wp;

  assert(tcg_enabled());
  if (cpu->watchpoint_hit) {
    /*
     * We re-entered the check after replacing the TB.
     * Now raise the debug interrupt so that it will
     * trigger after the current instruction.
     */
    qemu_mutex_lock_iothread();
    cpu_interrupt(cpu, CPU_INTERRUPT_DEBUG);
    qemu_mutex_unlock_iothread();
    return;
  }

  addr = cc->adjust_watchpoint_address(cpu, addr, len);
  QTAILQ_FOREACH(wp, &cpu->watchpoints, entry) {
    if (watchpoint_address_matches(wp, addr, len) && (wp->flags & flags)) {
      if (flags == BP_MEM_READ) {
        wp->flags |= BP_WATCHPOINT_HIT_READ;
      } else {
        wp->flags |= BP_WATCHPOINT_HIT_WRITE;
      }
      wp->hitaddr = MAX(addr, wp->vaddr);
      wp->hitattrs = attrs;
      if (!cpu->watchpoint_hit) {
        if (wp->flags & BP_CPU && !cc->debug_check_watchpoint(cpu, wp)) {
          wp->flags &= ~BP_WATCHPOINT_HIT;
          continue;
        }
        cpu->watchpoint_hit = wp;

        mmap_lock();
        tb_check_watchpoint(cpu, ra);
        if (wp->flags & BP_STOP_BEFORE_ACCESS) {
          cpu->exception_index = EXCP_DEBUG;
          mmap_unlock();
          cpu_loop_exit_restore(cpu, ra);
        } else {
          /* Force execution of one insn next time.  */
          cpu->cflags_next_tb = 1 | curr_cflags();
          mmap_unlock();
          if (ra) {
            cpu_restore_state(cpu, ra, true);
          }
          cpu_loop_exit_noexc(cpu);
        }
      }
    } else {
      wp->flags &= ~BP_WATCHPOINT_HIT;
    }
  }
}

void cpu_abort(CPUState *cpu, const char *fmt, ...) {
  va_list ap;
  va_list ap2;

  va_start(ap, fmt);
  va_copy(ap2, ap);
  fprintf(stderr, "qemu: fatal: ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  cpu_dump_state(cpu, stderr, CPU_DUMP_FPU | CPU_DUMP_CCOP);
  // log by printf, doesn't support separated log
#ifdef BMBT
  if (qemu_log_separate()) {
    qemu_log_lock();
    qemu_log("qemu: fatal: ");
    qemu_log_vprintf(fmt, ap2);
    qemu_log("\n");
    log_cpu_state(cpu, CPU_DUMP_FPU | CPU_DUMP_CCOP);
    qemu_log_flush();
    qemu_log_unlock();
    qemu_log_close();
  }
#endif
  va_end(ap2);
  va_end(ap);
  // doesn't support replay
#ifdef BMBT
  replay_finish();
#endif
#if defined(CONFIG_USER_ONLY)
  {
    struct sigaction act;
    sigfillset(&act.sa_mask);
    act.sa_handler = SIG_DFL;
    act.sa_flags = 0;
    sigaction(SIGABRT, &act, NULL);
  }
#endif
  abort();
}
