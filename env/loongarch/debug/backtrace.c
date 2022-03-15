#include "../syscall/internal.h"
#include <asm/debug.h>
#include <linux/stack.h>
#include <stdbool.h>
#include <stdlib.h>

static inline bool in_stack(void *fp) {
  return fp < (void *)kernel_stack + _THREAD_SIZE && fp >= (void *)kernel_stack;
}

void backtrace(long *fp) {
  if (fp == NULL) {
    fp = read_fp();
  }

  while (fp != NULL) {
    void *fp_1 = (void *)*(fp - 1);
    void *fp_2 = (void *)*(fp - 2);
    void *ip = NULL;
    long *real_fp = NULL;
    // if compiler think it's a leaf function
    // ra register will not be preserved
    if (in_stack(fp_1)) {
      real_fp = fp_1;
    } else if (in_stack(fp_2)) {
      real_fp = fp_2;
      ip = fp_1;
    } else {
      kern_printf("---------------------\n");
      break;
    }

    if (ip != NULL)
      kern_printf("%lx\n", ip);
    fp = real_fp;
  }
}
