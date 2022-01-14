#include "syscall.h"
#include <stdlib.h>

_Noreturn void _Exit(int ec) {
  libc_syscall(SYS_exit_group, ec);
  for (;;)
    libc_syscall(SYS_exit, ec);
}
