#include "../../../include/hw/core/cpu.h"
#include "../../../include/sysemu/cpus.h"

/* Resetting the IRQ comes from across the code base so we take the
 * BQL here if we need to.  cpu_interrupt assumes it is held.*/
void cpu_reset_interrupt(CPUState *cpu, int mask) {
  bool need_lock = !qemu_mutex_iothread_locked();

  if (need_lock) {
    qemu_mutex_lock_iothread();
  }
  cpu->interrupt_request &= ~mask;
  if (need_lock) {
    qemu_mutex_unlock_iothread();
  }
}
