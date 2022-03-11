#include <asm/cpu-features.h>
#include <asm/loongarchregs.h>
#include <asm/mach-la64/irq.h>
#include <asm/setup.h>
#include <asm/time.h>
#include <assert.h>
#include <env-timer.h>
#include <hw/core/cpu.h>
#include <linux/irqflags.h>
#include <stdio.h>


static TimerHandler signal_timer_handler;
static void timer_handler(int irq) {
  assert(irq == ECFGB_TIMER);
  if (cpu_has_hypervisor)
    asm(".long 0x002b8000"); // kvm bug
  enter_interrpt_context();
  signal_timer_handler();
  leave_interrpt_context();
  /* Clear Timer Interrupt at the end of timer interrupt handler, otherwise kvm
   * will inject timer interrupt again. */
  write_csr_tintclear(CSR_TINTCLR_TI);
}

void setup_timer(TimerHandler handler) {
  signal_timer_handler = handler;
  set_vi_handler(EXCCODE_TIMER, timer_handler);
}

void soonest_interrupt_ns(long ns) {
  // [interface 57]
  constant_timer_next_event(ns);
}

static bool __blocked = false;
bool is_interrupt_blocked(void) { return __blocked; }

void block_interrupt() {
  assert(__blocked == false);
  assert(qemu_cpu_is_self(NULL));
  local_irq_disable();
  __blocked = true;
}

void unblock_interrupt() {
  assert(__blocked == true);
  assert(qemu_cpu_is_self(NULL));
  __blocked = false;
  local_irq_enable();
}

void fire_timer() { constant_timer_next_event(0); }
