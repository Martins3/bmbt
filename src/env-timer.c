#include <assert.h>
#include <env-timer.h>
#include <stdbool.h>
#include <hw/core/cpu.h>

static bool __interrupt_context = false;
bool qemu_cpu_is_self(CPUState *cpu) { return !__interrupt_context; }

void enter_interrpt_context() {
  assert(!is_interrupt_blocked());
  assert(__interrupt_context == false);
  __interrupt_context = true;
}

void leave_interrpt_context() {
  assert(!is_interrupt_blocked());
  assert(__interrupt_context == true);
  __interrupt_context = false;
}
