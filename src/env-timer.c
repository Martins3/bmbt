#include <assert.h>
#include <env-timer.h>
#include <hw/core/cpu.h>
#include <qemu/main-loop.h>
#include <stdbool.h>

static bool __interrupt_context = false;
bool qemu_cpu_is_self(CPUState *cpu) { return !__interrupt_context; }

void native_qemu_mutex_lock_iothread_impl(const char *file, int line);
void native_qemu_mutex_unlock_iothread(void);

void enter_interrpt_context() {
  native_qemu_mutex_lock_iothread_impl(__FILE__, __LINE__);
  assert(!is_interrupt_blocked());
  assert(__interrupt_context == false);
  __interrupt_context = true;
}

void leave_interrpt_context() {
  native_qemu_mutex_unlock_iothread();
  assert(!is_interrupt_blocked());
  assert(__interrupt_context == true);
  __interrupt_context = false;
}
