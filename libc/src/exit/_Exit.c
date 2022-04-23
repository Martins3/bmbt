#include "syscall.h"
#include <stdlib.h>

// TMP_TODO 用户态和kernel 各自搞一个 对应的 backt
_Noreturn void _Exit(int ec) {
  libc_syscall(SYS_exit_group, ec);
  for (;;)
    libc_syscall(SYS_exit, ec);
}
