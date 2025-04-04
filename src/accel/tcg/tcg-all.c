#include <hw/core/cpu.h>
#include "tcg/tcg.h"

unsigned long tcg_tb_size;

/* mask must never be zero, except for A20 change call */
void tcg_handle_interrupt(CPUState *cpu, int mask) {
  // int old_mask;
  g_assert(qemu_mutex_iothread_locked() || !qemu_cpu_is_self(cpu));

  // old_mask = cpu->interrupt_request;
  cpu->interrupt_request |= mask;

  /*
   * If called from iothread context, wake the target cpu in
   * case its halted.
   */
  if (!qemu_cpu_is_self(cpu)) {
    qemu_cpu_kick(cpu);
  } else {
      qatomic_set(&cpu_neg(cpu)->icount_decr.u16.high, -1);
#if BMBT
    if (use_icount && !cpu->can_do_io && (mask & ~old_mask) != 0) {
      cpu_abort(cpu, "Raised interrupt while not in I/O function");
    }
#endif
  }
}

int tcg_init() {
  // -accel tcg,tb-size=2048
  tcg_tb_size = 2048;
  tcg_exec_init(tcg_tb_size * 1024 * 1024);
  return 0;
}
