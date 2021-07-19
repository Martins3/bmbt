#include "../../include/exec/cpu-all.h"
#include "../../include/exec/cpu-defs.h"
#include "../../include/exec/exec-all.h"
#include "../../include/exec/tb-context.h"
#include "../../include/exec/tb-hash.h"
#include "../../include/exec/tb-lookup.h"
#include "../../include/hw/core/cpu.h"
#include "../../include/hw/i386/apic.h"
#include "../../include/qemu/atomic.h"
#include "../../include/qemu/qht.h"
#include "../../include/qemu/rcu.h"
#include "../../include/qemu/plugin.h"
#include "../../include/sysemu/cpus.h"
#include "../../include/sysemu/replay.h"
#include "../../include/types.h"
#include "../i386/LATX/x86tomips-config.h"
#include "../i386/cpu.h"
#include "../i386/svm.h"
#include "tcg.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#define qemu_log_mask_and_addr(MASK, ADDR, FMT, ...)                           \
  do {                                                                         \
  } while (0)

target_ulong breakpoint_addrx = 0;
int breakpoint_hit_count = 0;
int breakpoint_ignore_count = 0;

/* Execute a TB, and fix up the CPU state afterwards if necessary */
static inline tcg_target_ulong cpu_tb_exec(CPUState *cpu,
                                           TranslationBlock *itb) {
  CPUArchState *env = cpu->env_ptr;
  uintptr_t ret;
  TranslationBlock *last_tb;
  int tb_exit;
  uint8_t *tb_ptr = itb->tc.ptr;

  qemu_log_mask_and_addr(CPU_LOG_EXEC, itb->pc,
                         "Trace %d: %p [" TARGET_FMT_lx "/" TARGET_FMT_lx
                         "/%#x] %s\n",
                         cpu->cpu_index, itb->tc.ptr, itb->cs_base, itb->pc,
                         itb->flags, lookup_symbol(itb->pc));

#if defined(DEBUG_DISAS)
  if (qemu_loglevel_mask(CPU_LOG_TB_CPU) && qemu_log_in_addr_range(itb->pc)) {
    qemu_log_lock();
    int flags = 0;
    if (qemu_loglevel_mask(CPU_LOG_TB_FPU)) {
      flags |= CPU_DUMP_FPU;
    }
#if defined(TARGET_I386)
    flags |= CPU_DUMP_CCOP;
#endif

#if (defined CONFIG_DEBUG_CHECK) && (!defined CONFIG_SOFTMMU)
    if (last_is_rep_tb == false) {
      ++tb_counter;
      if (print_enable) {
        show_counter++;
        log_cpu_state(cpu, flags);
      }
    }
#else
    log_cpu_state(cpu, flags);
#endif
    qemu_log_unlock();
  }
#endif /* DEBUG_DISAS */

#ifdef CONFIG_X86toMIPS
  x86_to_mips_before_exec_tb(cpu, itb);
#endif
  if (itb->pc == breakpoint_addrx) {
    breakpoint_hit_count += 1;
    if (breakpoint_hit_count >= breakpoint_ignore_count) {
      fprintf(stderr,
              "[debug] break point TB exec" TARGET_FMT_lx " cnt = %d.\n",
              itb->pc, breakpoint_hit_count);
    }
  }

  // FIXME why xqm need extra hacking for this?
  ret = tcg_qemu_tb_exec(env, tb_ptr);
#ifdef CONFIG_X86toMIPS
#ifndef CONFIG_SOFTMMU
#ifdef N64
  if (ret & ~TB_EXIT_MASK)
    ret |= (uintptr_t)itb & 0xffffffff00000000;
#endif
#endif
  x86_to_mips_after_exec_tb(env, itb);
#endif
  cpu->can_do_io = 1;
  last_tb = (TranslationBlock *)(ret & ~TB_EXIT_MASK);
  tb_exit = ret & TB_EXIT_MASK;

  // fuck_trace_exec_tb_exit(last_tb, tb_exit);
  if (tb_exit > TB_EXIT_IDX1) {
    /* We didn't start executing this TB (eg because the instruction
     * counter hit zero); we must restore the guest PC to the address
     * of the start of the TB.
     */
    CPUClass *cc = CPU_GET_CLASS(cpu);
    qemu_log_mask_and_addr(
        CPU_LOG_EXEC, last_tb->pc,
        "Stopped execution of TB chain before %p [" TARGET_FMT_lx "] %s\n",
        last_tb->tc.ptr, last_tb->pc, lookup_symbol(last_tb->pc));
    if (cc->synchronize_from_tb) {
      cc->synchronize_from_tb(cpu, last_tb);
    } else {
      assert(cc->set_pc);
      cc->set_pc(cpu, last_tb->pc);
    }
  }
  return ret;
}

void tb_set_jmp_target(TranslationBlock *tb, int n, uintptr_t addr) {}

static inline void tb_add_jump(TranslationBlock *tb, int n,
                               TranslationBlock *tb_next) {
  uintptr_t old;

  assert(n < ARRAY_SIZE(tb->jmp_list_next));
  qemu_spin_lock(&tb_next->jmp_lock);

  /* make sure the destination TB is valid */
  if (tb_next->cflags & CF_INVALID) {
    goto out_unlock_next;
  }
  /* Atomically claim the jump destination slot only if it was NULL */
  old = atomic_cmpxchg(&tb->jmp_dest[n], (uintptr_t)NULL, (uintptr_t)tb_next);
  if (old) {
    goto out_unlock_next;
  }

#ifndef CONFIG_X86toMIPS
  /* patch the native jump address */
  tb_set_jmp_target(tb, n, (uintptr_t)tb_next->tc.ptr);
#else
  /* check fpu rotate and patch the native jump address */
  x86_to_mips_tb_set_jmp_target(tb, n, tb_next);
#endif

  /* add in TB jmp list */
  tb->jmp_list_next[n] = tb_next->jmp_list_head;
  tb_next->jmp_list_head = (uintptr_t)tb | n;

  qemu_spin_unlock(&tb_next->jmp_lock);

  qemu_log_mask_and_addr(CPU_LOG_EXEC, tb->pc,
                         "Linking TBs %p [" TARGET_FMT_lx
                         "] index %d -> %p [" TARGET_FMT_lx "]\n",
                         tb->tc.ptr, tb->pc, n, tb_next->tc.ptr, tb_next->pc);
  return;

out_unlock_next:
  qemu_spin_unlock(&tb_next->jmp_lock);
  return;
}

#ifndef CONFIG_USER_ONLY
/* Execute the code without caching the generated code. An interpreter
   could be used if available. */
static void cpu_exec_nocache(CPUState *cpu, int max_cycles,
                             TranslationBlock *orig_tb, bool ignore_icount) {
  TranslationBlock *tb;
  uint32_t cflags = curr_cflags() | CF_NOCACHE;

  if (ignore_icount) {
    cflags &= ~CF_USE_ICOUNT;
  }

  /* Should never happen.
     We only end up here when an existing TB is too long.  */
  cflags |= MIN(max_cycles, CF_COUNT_MASK);

  mmap_lock();
  tb = tb_gen_code(cpu, orig_tb->pc, orig_tb->cs_base, orig_tb->flags, cflags);
  tb->orig_tb = orig_tb;
  mmap_unlock();

  /* execute the generated code */
  // fuck_trace_exec_tb_nocache(tb, tb->pc);
  cpu_tb_exec(cpu, tb);

  mmap_lock();
  tb_phys_invalidate(tb, -1);
  mmap_unlock();
  tcg_tb_remove(tb);
}
#endif

static void cpu_update_icount(CPUState *cpu) {
  // FIXME
}

static inline void cpu_loop_exec_tb(CPUState *cpu, TranslationBlock *tb,
                                    TranslationBlock **last_tb, int *tb_exit) {
  uintptr_t ret;
  int32_t insns_left;

  // fuck_trace_exec_tb(tb, tb->pc);
  ret = cpu_tb_exec(cpu, tb);
  tb = (TranslationBlock *)(ret & ~TB_EXIT_MASK);
  *tb_exit = ret & TB_EXIT_MASK;
  if (*tb_exit != TB_EXIT_REQUESTED) {
    *last_tb = tb;
    return;
  }

  *last_tb = NULL;
  insns_left = atomic_read(&cpu_neg(cpu)->icount_decr.u32);
  if (insns_left < 0) {
    /* Something asked us to stop executing chained TBs; just
     * continue round the main loop. Whatever requested the exit
     * will also have set something else (eg exit_request or
     * interrupt_request) which will be handled by
     * cpu_handle_interrupt.  cpu_handle_interrupt will also
     * clear cpu->icount_decr.u16.high.
     */
    return;
  }

  /* Instruction counter expired.  */
  assert(use_icount);
#ifndef CONFIG_USER_ONLY
  /* Ensure global icount has gone forward */
  cpu_update_icount(cpu);
  /* Refill decrementer and continue execution.  */
  insns_left = MIN(0xffff, cpu->icount_budget);
  cpu_neg(cpu)->icount_decr.u16.low = insns_left;
  cpu->icount_extra = cpu->icount_budget - insns_left;
  if (!cpu->icount_extra) {
    /* Execute any remaining instructions, then let the main loop
     * handle the next event.
     */
    if (insns_left > 0) {
      cpu_exec_nocache(cpu, insns_left, tb, false);
    }
  }
#endif
}

static inline TranslationBlock *
tb_find(CPUState *cpu, TranslationBlock *last_tb, int tb_exit, u32 cf_mask) {
  TranslationBlock *tb;
  target_ulong cs_base, pc;
  u32 flags;

  tb = tb_lookup__cpu_state(cpu, &pc, &cs_base, &flags, cf_mask);

  if (tb == NULL) {
    mmap_lock();
    tb = tb_gen_code(cpu, pc, cs_base, flags, cf_mask);
#if defined(CONFIG_X86toMIPS) && !defined(CONFIG_SOFTMMU)
    /* Enable Shadow Stack in user-mode only */
    /* Option Profile in user-mode only */
    /* set tb field in etb */
    ETB *etb;
    if (option_shadow_stack || option_profile) {
      etb = etb_cache_find(pc, false);
      etb->tb = tb;
    }
    /* Enable IBTC in user-mode only */
    if (last_tb == NULL) { /* last tb is indirect block */
      update_ibtc(pc, tb);
    }
#endif
    mmap_unlock();
    /* We add the TB in the virtual pc hash table for the fast lookup */
    atomic_set(&cpu->tb_jmp_cache[tb_jmp_cache_hash_func(pc)], tb);
  }

#ifndef CONFIG_USER_ONLY
  // FIXME deeper understanding how tb expands to second pages
  /* We don't take care of direct jumps when address mapping changes in
   * system emulation. So it's not safe to make a direct jump to a TB
   * spanning two pages because the mapping for the second page can change.
   */
  if (tb->page_addr[1] != -1) {
    last_tb = NULL;
  }
#endif

  /* See if we can patch the calling TB. */
  if (last_tb) {
    tb_add_jump(last_tb, tb_exit, tb);
  }
  return tb;
}

static inline void cpu_handle_debug_exception(CPUState *cpu) {
  CPUClass *cc = CPU_GET_CLASS(cpu);
  CPUWatchpoint *wp;

  if (!cpu->watchpoint_hit) {
    QTAILQ_FOREACH(wp, &cpu->watchpoints, entry) {
      wp->flags &= ~BP_WATCHPOINT_HIT;
    }
  }

  cc->debug_excp_handler(cpu);
}

// FIXME I don't know how cpu_handle_exception works,
// it seems some thing important has happened somewhere else.
static inline bool cpu_handle_exception(CPUState *cpu, int *ret) {
  if (cpu->exception_index < 0) {
#ifndef CONFIG_USER_ONLY
    if (replay_has_exception() &&
        cpu_neg(cpu)->icount_decr.u16.low + cpu->icount_extra == 0) {
      /* try to cause an exception pending in the log */
      cpu_exec_nocache(cpu, 1, tb_find(cpu, NULL, 0, curr_cflags()), true);
    }
#endif
    if (cpu->exception_index < 0) {
      return false;
    }
  }

  if (cpu->exception_index >= EXCP_INTERRUPT) {
    /* exit request from the cpu execution loop */
    *ret = cpu->exception_index;
    if (*ret == EXCP_DEBUG) {
      cpu_handle_debug_exception(cpu);
    }
    cpu->exception_index = -1;
    return true;
  } else {
#if defined(CONFIG_USER_ONLY)
    /* if user mode only, we simulate a fake exception
       which will be handled outside the cpu execution
       loop */
#if defined(TARGET_I386)
    CPUClass *cc = CPU_GET_CLASS(cpu);
    cc->do_interrupt(cpu);
#endif
    *ret = cpu->exception_index;
    cpu->exception_index = -1;
    return true;
#else
    if (replay_exception()) {
      CPUClass *cc = CPU_GET_CLASS(cpu);
      qemu_mutex_lock_iothread();
      cc->do_interrupt(cpu);
      qemu_mutex_unlock_iothread();
      cpu->exception_index = -1;
    } else if (!replay_has_interrupt()) {
      /* give a chance to iothread in replay mode */
      *ret = EXCP_INTERRUPT;
      return true;
    }
#endif
  }

  return false;
}

static inline bool cpu_handle_interrupt(CPUState *cpu,
                                        TranslationBlock **last_tb) {

  CPUClass *cc = CPU_GET_CLASS(cpu);

  /* Clear the interrupt flag now since we're processing
   * cpu->interrupt_request and cpu->exit_request.
   * Ensure zeroing happens before reading cpu->exit_request or
   * cpu->interrupt_request (see also smp_wmb in cpu_exit())
   */
  atomic_mb_set(&cpu_neg(cpu)->icount_decr.u16.high, 0);

  if (unlikely(atomic_read(&cpu->interrupt_request))) {
    int interrupt_request;
    qemu_mutex_lock_iothread();
    interrupt_request = cpu->interrupt_request;
    if (unlikely(cpu->singlestep_enabled & SSTEP_NOIRQ)) {
      /* Mask out external interrupts for this step. */
      interrupt_request &= ~CPU_INTERRUPT_SSTEP_MASK;
    }
    if (interrupt_request & CPU_INTERRUPT_DEBUG) {
      cpu->interrupt_request &= ~CPU_INTERRUPT_DEBUG;
      cpu->exception_index = EXCP_DEBUG;
      qemu_mutex_unlock_iothread();
      return true;
    }
    if (replay_mode == REPLAY_MODE_PLAY && !replay_has_interrupt()) {
      /* Do nothing */
    } else if (interrupt_request & CPU_INTERRUPT_HALT) {
      replay_interrupt();
      cpu->interrupt_request &= ~CPU_INTERRUPT_HALT;
      cpu->halted = 1;
      cpu->exception_index = EXCP_HLT;
      qemu_mutex_unlock_iothread();
      return true;
    }
#if defined(TARGET_I386)
    else if (interrupt_request & CPU_INTERRUPT_INIT) {
      X86CPU *x86_cpu = X86_CPU(cpu);
      CPUArchState *env = &x86_cpu->env;
      replay_interrupt();
      cpu_svm_check_intercept_param(env, SVM_EXIT_INIT, 0, 0);
      do_cpu_init(x86_cpu);
      cpu->exception_index = EXCP_HALTED;
      qemu_mutex_unlock_iothread();
      return true;
    }
#else
    else if (interrupt_request & CPU_INTERRUPT_RESET) {
      replay_interrupt();
      cpu_reset(cpu);
      qemu_mutex_unlock_iothread();
      return true;
    }
#endif
    /* The target hook has 3 exit conditions:
       False when the interrupt isn't processed,
       True when it is, and we should restart on a new TB,
       and via longjmp via cpu_loop_exit.  */
    else {
      if (cc->cpu_exec_interrupt(cpu, interrupt_request)) {
        replay_interrupt();
        cpu->exception_index = -1;
        *last_tb = NULL;
      }
      /* The target hook may have updated the 'cpu->interrupt_request';
       * reload the 'interrupt_request' value */
      interrupt_request = cpu->interrupt_request;
    }
    if (interrupt_request & CPU_INTERRUPT_EXITTB) {
      cpu->interrupt_request &= ~CPU_INTERRUPT_EXITTB;
      /* ensure that no TB jump will be modified as
         the program flow was changed */
      *last_tb = NULL;
    }

    /* If we exit via cpu_loop_exit/longjmp it is reset in cpu_exec */
    qemu_mutex_unlock_iothread();
  }

  /* Finally, check if we need to exit to the main loop.  */
  if (unlikely(atomic_read(&cpu->exit_request)) ||
      (use_icount &&
       cpu_neg(cpu)->icount_decr.u16.low + cpu->icount_extra == 0)) {
    atomic_set(&cpu->exit_request, 0);
    if (cpu->exception_index == -1) {
      cpu->exception_index = EXCP_INTERRUPT;
    }
    return true;
  }
  return false;
}

static inline bool cpu_handle_halt(CPUState *cpu) {
  if (cpu->halted) {
#if defined(TARGET_I386) && !defined(CONFIG_USER_ONLY)
    if ((cpu->interrupt_request & CPU_INTERRUPT_POLL) && replay_interrupt()) {
      X86CPU *x86_cpu = X86_CPU(cpu);
      qemu_mutex_lock_iothread();
      apic_poll_irq(x86_cpu->apic_state);
      cpu_reset_interrupt(cpu, CPU_INTERRUPT_POLL);
      qemu_mutex_unlock_iothread();
    }
#endif
    if (!cpu_has_work(cpu)) {
      return true;
    }

    cpu->halted = 0;
  }

  return false;
}

int cpu_exec(CPUState *cpu) {
  int ret;

  // FIXME how SyncClocks works
  // SyncClocks sc = { 0 };

  /* replay_interrupt may need current_cpu */
  current_cpu = cpu;

  if (cpu_handle_halt(cpu)) {
    return EXCP_HALTED;
  }

  rcu_read_lock();
  x86_cpu_exec_enter(cpu);

  /* Calculate difference between guest clock and host clock.
   * This delay includes the delay of the last cycle, so
   * what we have to do is sleep until it is 0. As for the
   * advance/delay we gain here, we try to fix it next time.
   */
  // init_delay_params(&sc, cpu);

  // FIXME find out from where the code flow jump to here
  if (sigsetjmp(cpu->jmp_env, 0) != 0) {
    // FIXME some checks for sigsetjmp bugs, review it later

    if (qemu_mutex_iothread_locked()) {
      qemu_mutex_unlock_iothread();
    }

    qemu_plugin_disable_mem_helpers(cpu);
    assert_no_pages_locked();
  }

  /* if an exception is pending, we execute it here */
  while (!cpu_handle_exception(cpu, &ret)) {
    TranslationBlock *last_tb = NULL;
    int tb_exit = 0;

    while (!cpu_handle_interrupt(cpu, &last_tb)) {
      u32 cflags = cpu->cflags_next_tb;
      TranslationBlock *tb;

      /* When requested, use an exact setting for cflags for the next
         execution.  This is used for icount, precise smc, and stop-
         after-access watchpoints.  Since this request should never
         have CF_INVALID set, -1 is a convenient invalid value that
         does not require tcg headers for cpu_common_reset.  */
      if (cflags == -1) {
        cflags = curr_cflags();
      } else {
        cpu->cflags_next_tb = -1;
      }

      tb = tb_find(cpu, last_tb, tb_exit, cflags);

      // FIXME
      // defined at target/i386/LATX/x86tomips-config.c
      // trace_tb_execution(cpu, tb);

      cpu_loop_exec_tb(cpu, tb, &last_tb, &tb_exit);

      /* Try to align the host and virtual clocks
         if the guest is in advance */
      // FIXME icount related
      // align_clocks(&sc, cpu);
    }
  }

  x86_cpu_exec_exit(cpu);
  rcu_read_unlock();

  return ret;

  return 0;
}

struct tb_desc {
  target_ulong pc;
  target_ulong cs_base;
  CPUArchState *env;
  tb_page_addr_t phys_page1;
  uint32_t flags;
  uint32_t cf_mask;
  uint32_t trace_vcpu_dstate;
};

static bool tb_lookup_cmp(const void *p, const void *d) {
  const TranslationBlock *tb = p;
  const struct tb_desc *desc = d;

  if (tb->pc == desc->pc && tb->page_addr[0] == desc->phys_page1 &&
      tb->cs_base == desc->cs_base && tb->flags == desc->flags &&
      tb->trace_vcpu_dstate == desc->trace_vcpu_dstate &&
      (tb_cflags(tb) & (CF_HASH_MASK | CF_INVALID)) == desc->cf_mask) {
    /* check next page if needed */
    if (tb->page_addr[1] == -1) {
      return true;
    } else {
      tb_page_addr_t phys_page2;
      target_ulong virt_page2;

      virt_page2 = (desc->pc & TARGET_PAGE_MASK) + TARGET_PAGE_SIZE;
      phys_page2 = get_page_addr_code(desc->env, virt_page2);
      if (tb->page_addr[1] == phys_page2) {
        return true;
      }
    }
  }
  return false;
}

TranslationBlock *tb_htable_lookup(CPUState *cpu, target_ulong pc,
                                   target_ulong cs_base, uint32_t flags,
                                   uint32_t cf_mask) {
  tb_page_addr_t phys_pc;
  struct tb_desc desc;
  uint32_t h;

  desc.env = (CPUArchState *)cpu->env_ptr;
  desc.cs_base = cs_base;
  desc.flags = flags;
  desc.cf_mask = cf_mask;
  desc.trace_vcpu_dstate = *cpu->trace_dstate;
  desc.pc = pc;
  phys_pc = get_page_addr_code(desc.env, pc);
  if (phys_pc == -1) {
    return NULL;
  }
  desc.phys_page1 = phys_pc & TARGET_PAGE_MASK;
  h = tb_hash_func(phys_pc, pc, flags, cf_mask, *cpu->trace_dstate);
#if defined(CONFIG_SOFTMMU) && defined(CONFIG_X86toMIPS) &&                    \
    defined(CONFIG_XTM_PROFILE)
  tb_hash_cmp_cnt = 0;

  TranslationBlock *tb =
      qht_lookup_custom_xtm(&tb_ctx.htable, &desc, h, tb_lookup_cmp,
                            &tb_hash_cmp_cnt, xtm_pf_tb_hash_cmp_counter);

  xtm_pf_tb_hash_table(cpu, tb, tb_hash_cmp_cnt);
  tb_hash_cmp_cnt = 0;

  return tb;
#else
  return qht_lookup_custom(&tb_ctx.htable, &desc, h, tb_lookup_cmp);
#endif
}
