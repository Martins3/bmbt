#include "internal.h"
#include <asm/config.h>
#include <bits/syscall.h>
#include <stack.h>
#include <stdint.h>
#include <unistd.h>

// #define DEBUG_KERNEL_SYSCALL

static inline long *r_fp() {
  long *x;
  asm volatile("move %0, $fp" : "=r"(x));
  return x;
}

void backtrace(void) {
  duck_printf("backtrace:\n");
  long *fp = r_fp();

  while (fp != NULL) {
    void *fp_1 = (void *)*(fp - 1);
    void *fp_2 = (void *)*(fp - 2);
    void *ip = NULL;
    long *real_fp = NULL;
    if (fp_1 < (void *)kernel_stack + _THREAD_SIZE &&
        fp_1 >= (void *)kernel_stack) {
      real_fp = fp_1;
    } else {
      real_fp = fp_2;
      ip = fp_1;
    }

    if (ip != NULL)
      duck_printf("%lx\n", ip);
    // duck_printf("%lx\n", real_fp);
    fp = real_fp;
  }
}

static _Noreturn void idle() {
  for (;;) {
  }
}

_Noreturn void kernel_dump() {
  backtrace();
  idle();
}

void __duck_assert_fail(const char *expr, const char *file, int line,
                        const char *func) {
  duck_printf("Assertion failed: %s (%s: %s: %d)\n", expr, file, func, line);
  kernel_dump();
}

static int syscall_counter = 0;
long kernel_syscall(long arg0, long arg1, long arg2, long arg3, long arg4,
                    long arg5, long arg6, long sysno) {
  // no recursive sycall
  duck_assert(syscall_counter == 0);
  syscall_counter++;
  int ret;
#ifdef DEBUG_KERNEL_SYSCALL
  duck_printf("%ld: [0x%016lx], [0x%016lx], [0x%016lx], [0x%016lx], "
              "[0x%016lx], [0x%016lx]\n",
              sysno, arg0, arg1, arg2, arg3, arg4, arg5, arg6);
#endif
  switch (sysno) {
  case SYS_writev /* variable case */:
    ret = kernel_writev(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
    break;
  case SYS_exit_group:
  case SYS_exit:
    duck_printf("!!! bmbt never call exit !!!\n");
    kernel_dump();
    break;
  case SYS_brk:
    // if SYS_brk failed, musl malloc will use mmap
    // see libc/src/malloc/mallocng/malloc.c
    ret = 0;
  case SYS_mmap:
    ret = kernel_mmap(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
  case SYS_munmap:
    ret = kernel_unmmap(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
  default:
    duck_printf("unsported syscall\n");
    kernel_dump();
  }
  syscall_counter--;
  return ret;
}
