#include <features.h>

long __kernel_syscall_stub(long arg0, long arg1, long arg2, long arg3,
                           long arg4, long arg5, long arg6, long number) {
  return 0;
}

long kernel_syscall(long arg0, long arg1, long arg2, long arg3, long arg4,
                    long arg5, long arg6, long number);

weak_alias(__kernel_syscall_stub, kernel_syscall);
