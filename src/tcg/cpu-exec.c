#include "../../include/exec/exec-all.h"
#include "../../include/hw/core/cpu.h"
#include "../../include/sysemu/replay.h"
#include "../../include/types.h"
#include "../../include/exec/tb-lookup.h"
#include "../../include/exec/cpu-defs.h"
#include <stdbool.h> // FIXME header for bool

static inline void tb_add_jump(TranslationBlock *tb, int n,
                               TranslationBlock *tb_next)
{
  // TODO
}

static inline void cpu_loop_exec_tb(CPUState *cpu, TranslationBlock *tb,
                                    TranslationBlock **last_tb, int *tb_exit) {
  // TODO
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
