#include "../../include/exec/cpu-all.h"
#include "../../include/exec/exec-all.h"
#include "../../include/exec/memory.h"
#include "../../include/hw/core/cpu.h"
#include "../../include/qemu/error-report.h"
#include <errno.h>

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

// FIXME maybe I need it later
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
