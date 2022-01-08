#include "syscall.h"
#include <stdlib.h>

_Noreturn void _Exit(int ec) {
  __syscall(SYS_exit_group, ec);
  for (;;)
    __syscall(SYS_exit, ec);
}
