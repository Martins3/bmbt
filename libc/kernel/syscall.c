#include "internal.h"
#include <bits/syscall.h>
#include <kernel/config.h>
#include <stdint.h>
#include <unistd.h>

#define DEBUG_KERNEL_SYSCALL

static inline long *r_fp() {
  long *x;
  asm volatile("move %0, $fp" : "=r"(x));
  return x;
}

extern unsigned char kernel_stack[_THREAD_SIZE];

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

void kernel_dump() { backtrace(); }

long kernel_syscall(long arg0, long arg1, long arg2, long arg3, long arg4,
                    long arg5, long arg6, long sysno) {
#ifdef DEBUG_KERNEL_SYSCALL
  duck_printf("%ld: [0x%016lx], [0x%016lx], [0x%016lx], [0x%016lx], "
              "[0x%016lx], [0x%016lx]\n",
              sysno, arg0, arg1, arg2, arg3, arg4, arg5, arg6);
#endif
  switch (sysno) {
  case SYS_writev /* variable case */:
    break;
  default:
    duck_printf("unsported syscall\n");
    kernel_dump();
  }
  return 0;
}
