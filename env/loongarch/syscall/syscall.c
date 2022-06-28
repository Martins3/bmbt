#include "internal.h"
#include <asm/debug.h>
#include <bits/syscall.h>
#include <linux/irqflags.h>
#include <linux/stack.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

// #define DEBUG_KERNEL_SYSCALL
#define CHECK_SYSCALL_RECURSIVE

static _Noreturn void idle() {
  for (;;) {
    // @todo use loongarch idle
  }
}

_Noreturn void kernel_dump() {
  local_irq_disable();
  backtrace(NULL);
  idle();
}

#include "cpu.h"
void show_guest_ip() {
  if (current_cpu != NULL) {
    CPUX86State *env = ((CPUX86State *)current_cpu->env_ptr);
    kern_printf("Guest ip : %x\n", env->segs[R_CS].base + env->eip);
  } else {
    kern_printf("current_cpu is NULL\n");
  }
}

_Noreturn void __kern_assert_fail(const char *expr, const char *file, int line,
                                  const char *func) {
  kern_printf("Assertion failed: %s (%s: %s: %d)\n", expr, file, func, line);
  kernel_dump();
}

#ifdef CHECK_SYSCALL_RECURSIVE
int syscall_counter = 0;
#endif
long kernel_syscall(long arg0, long arg1, long arg2, long arg3, long arg4,
                    long arg5, long arg6, long sysno) {
#ifdef CHECK_SYSCALL_RECURSIVE
  unsigned long flags;
  // Of course, we can call printf in interrupt handler that interrupts printf.
  // But it cause assert fail in recursive syscall checker
  local_irq_save(flags);
  kern_assert(syscall_counter == 0);
  syscall_counter = sysno;
#endif
  long ret;
#ifdef DEBUG_KERNEL_SYSCALL
  kern_printf("%ld: [0x%016lx], [0x%016lx], [0x%016lx], [0x%016lx], "
              "[0x%016lx], [0x%016lx]\n",
              sysno, arg0, arg1, arg2, arg3, arg4, arg5, arg6);
#endif
  switch (sysno) {
  case SYS_writev /* variable case */:
    ret = kernel_writev(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
    break;
  case SYS_exit_group:
  case SYS_exit:
    kern_printf("!!! bmbt never call exit !!!\n");
    show_guest_ip();
    kernel_dump();
    break;
  case SYS_brk:
    // if SYS_brk failed, musl malloc will use mmap
    // see libc/src/malloc/mallocng/malloc.c
    ret = 0;
    break;
  case SYS_mprotect:
  case SYS_madvise:
    ret = 0;
    break;
  case SYS_mmap:
    ret = kernel_mmap(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
    break;
  case SYS_clock_gettime:
    ret = kernel_clock_gettime(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
    break;
  case SYS_munmap:
    ret = kernel_unmmap(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
    break;
  default:
    kern_printf("unsported syscall\n");
    kernel_dump();
  }
#ifdef CHECK_SYSCALL_RECURSIVE
  syscall_counter = 0;
  asm volatile("" ::: "memory");
  local_irq_restore(flags);
#endif
  return ret;
}
