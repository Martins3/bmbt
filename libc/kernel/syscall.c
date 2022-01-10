#include "internal.h"
long kernel_syscall(long arg0, long arg1, long arg2, long arg3, long arg4,
                    long arg5, long arg6, long number) {
  duck_printf("%d", number);
  return 12;
}
