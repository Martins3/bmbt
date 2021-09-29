#include "../../include/hw/core/cpu.h"
#include "tcg.h"

#ifdef CONFIG_X86toMIPS
extern int xtm_sigint_opt(void);
#define XTM_SIGINT_SIGNAL 63
#endif

unsigned long tcg_tb_size;

/* mask must never be zero, except for A20 change call */
static void tcg_handle_interrupt(CPUState *cpu, int mask) {
  int old_mask;
  g_assert(qemu_mutex_iothread_locked());

  old_mask = cpu->interrupt_request;
  cpu->interrupt_request |= mask;

  /*
   * If called from iothread context, wake the target cpu in
   * case its halted.
   */
  if (!qemu_cpu_is_self(cpu)) {
    g_assert_not_reached();
    // qemu_cpu_kick(cpu);
  } else {
#ifdef CONFIG_X86toMIPS
    if (xtm_sigint_opt()) {
      g_assert_not_reached();
      // pthread_kill(cpu->thread->thread, XTM_SIGINT_SIGNAL);
    } else
#endif
      atomic_set(&cpu_neg(cpu)->icount_decr.u16.high, -1);
#if BMBT
    if (use_icount && !cpu->can_do_io && (mask & ~old_mask) != 0) {
      cpu_abort(cpu, "Raised interrupt while not in I/O function");
    }
#endif
  }
}

int tcg_init() {
  tcg_exec_init(tcg_tb_size * 1024 * 1024);
  cpu_interrupt_handler = tcg_handle_interrupt;
  return 0;
}
