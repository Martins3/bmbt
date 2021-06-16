#include "../../include/exec/cpu-all.h"
#include "../../include/exec/cpu-defs.h"
#include "../../include/exec/exec-all.h"
#include "../../include/exec/tb-lookup.h"
#include "../../include/hw/core/cpu.h"
#include "../../include/qemu/atomic.h"
#include "../../include/sysemu/replay.h"
#include "../../include/types.h"
#include "tcg.h"
#include "../i386/LATX/x86tomips-config.h"
#include <assert.h>
#include <stdbool.h> // FIXME header for bool
#include <stdio.h>

// FIXME qemu log
#define qemu_log_mask_and_addr(MASK, ADDR, FMT, ...)    \
    do {                                                \
    } while (0)

target_ulong breakpoint_addrx = 0;
int breakpoint_hit_count= 0;
int breakpoint_ignore_count= 0;

/* Execute a TB, and fix up the CPU state afterwards if necessary */
static inline tcg_target_ulong cpu_tb_exec(CPUState *cpu,
                                           TranslationBlock *itb) {
    CPUArchState *env = cpu->env_ptr;
    uintptr_t ret;
    TranslationBlock *last_tb;
    int tb_exit;
    uint8_t *tb_ptr = itb->tc.ptr;

    qemu_log_mask_and_addr(CPU_LOG_EXEC, itb->pc,
                           "Trace %d: %p ["
                           TARGET_FMT_lx "/" TARGET_FMT_lx "/%#x] %s\n",
                           cpu->cpu_index, itb->tc.ptr,
                           itb->cs_base, itb->pc, itb->flags,
                           lookup_symbol(itb->pc));

#if defined(DEBUG_DISAS)
    if (qemu_loglevel_mask(CPU_LOG_TB_CPU)
        && qemu_log_in_addr_range(itb->pc)) {
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
            if(print_enable){
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
    if(itb->pc == breakpoint_addrx) {
        breakpoint_hit_count += 1;
        if (breakpoint_hit_count >= breakpoint_ignore_count) {
            // FIXME of course, this is a quick fix
            fprintf(stderr, "[debug] break point TB exec" TARGET_FMT_lx " cnt = %d.\n",
                itb->pc, breakpoint_hit_count);
        }
    }

    // FIXME why xqm need extra hacking for this?
    ret = tcg_qemu_tb_exec(env, tb_ptr);
#ifdef CONFIG_X86toMIPS
#ifndef CONFIG_SOFTMMU
  #ifdef N64
    if(ret & ~ TB_EXIT_MASK)   
        ret |= (uintptr_t)itb & 0xffffffff00000000;
  #endif
#endif
    x86_to_mips_after_exec_tb(env, itb);
#endif
    cpu->can_do_io = 1;
    last_tb = (TranslationBlock *)(ret & ~TB_EXIT_MASK);
    tb_exit = ret & TB_EXIT_MASK;
    // FIXME trace it later
    // trace_exec_tb_exit(last_tb, tb_exit);

    if (tb_exit > TB_EXIT_IDX1) {
        /* We didn't start executing this TB (eg because the instruction
         * counter hit zero); we must restore the guest PC to the address
         * of the start of the TB.
         */
        // FIXME
        // 2. 增加注释文档
        x86_cpu_synchronize_from_tb(cpu, last_tb);
        /**
        CPUClass *cc = CPU_GET_CLASS(cpu);
        qemu_log_mask_and_addr(CPU_LOG_EXEC, last_tb->pc,
                               "Stopped execution of TB chain before %p ["
                               TARGET_FMT_lx "] %s\n",
                               last_tb->tc.ptr, last_tb->pc,
                               lookup_symbol(last_tb->pc));

        if (cc->synchronize_from_tb) {
            cc->synchronize_from_tb(cpu, last_tb);
        } else {
            assert(cc->set_pc);
            cc->set_pc(cpu, last_tb->pc);
        }
        */

    }
    return ret;
}

void tb_set_jmp_target(TranslationBlock *tb, int n, uintptr_t addr) {}

static inline void tb_add_jump(TranslationBlock *tb, int n,
                               TranslationBlock *tb_next) {
  // TODO
}

static inline void cpu_loop_exec_tb(CPUState *cpu, TranslationBlock *tb,
                                    TranslationBlock **last_tb, int *tb_exit) {
  uintptr_t ret;
  int32_t insns_left;

  // FIXME fix trace later
  // trace_exec_tb(tb, tb->pc);
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

  // FIXME I don't understand icount
#if 0
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
#endif
}

static inline TranslationBlock *
tb_find(CPUState *cpu, TranslationBlock *last_tb, int tb_exit, u32 cf_mask) {
  TranslationBlock *tb;
  target_ulong cs_base, pc;
  u32 flags;

  tb = tb_lookup__cpu_state(cpu, &pc, &cs_base, &flags, cf_mask);

  if (tb == NULL) {
    // TODO why gen code needs mmap_lock?
    // mmap_lock();
    tb = tb_gen_code(cpu, pc, cs_base, flags, cf_mask);
    // mmap_unlock();
    /* We add the TB in the virtual pc hash table for the fast lookup */
    // TODO
    // atomic_set(&cpu->tb_jmp_cache[tb_jmp_cache_hash_func(pc)], tb);
    //
  }

  // FIXME deeper understanding of tb expanding two pages is needed
  /* We don't take care of direct jumps when address mapping changes in
   * system emulation. So it's not safe to make a direct jump to a TB
   * spanning two pages because the mapping for the second page can change.
   */
  if (tb->page_addr[1] != -1) {
    last_tb = NULL;
  }

  /* See if we can patch the calling TB. */
  if (last_tb) {
    tb_add_jump(last_tb, tb_exit, tb);
  }
  return tb;
}

static inline bool cpu_handle_interrupt(CPUState *cpu,
                                        TranslationBlock **last_tb) {
  // TODO
}

static inline bool cpu_handle_exception(CPUState *cpu, int *ret) {
  // TODO
}

int cpu_exec(CPUState *cpu) {
  // FIXME how
  // CPUClass *cc = CPU_GET_CLASS(cpu);
  CPUClass *cc = cpu->cc;

  int ret;

  // TODO is user mode come to here too?
  //
  // SyncClocks sc = { 0 };

  /* replay_interrupt may need current_cpu */
  // current_cpu = cpu;

  // if (cpu_handle_halt(cpu)) {
  // return EXCP_HALTED;
  // }

  // rcu_read_lock();

  cc->cpu_exec_enter(cpu);

  /* Calculate difference between guest clock and host clock.
   * This delay includes the delay of the last cycle, so
   * what we have to do is sleep until it is 0. As for the
   * advance/delay we gain here, we try to fix it next time.
   */
  // init_delay_params(&sc, cpu);

  // TODO find out from where the code flow jump to here
  if (sigsetjmp(cpu->jmp_env, 0) != 0) {
    // FIXME some checks about iothread here
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

      // TODO
      // defined at target/i386/LATX/x86tomips-config.c
      // trace_tb_execution(cpu, tb);

      cpu_loop_exec_tb(cpu, tb, &last_tb, &tb_exit);

      /* Try to align the host and virtual clocks
         if the guest is in advance */
      // TODO icount related
      // align_clocks(&sc, cpu);
    }
  }

  cc->cpu_exec_exit(cpu);

  return ret;

  return 0;
}
