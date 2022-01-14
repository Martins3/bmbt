#include "syscall.h"
#include <unistd.h>

pid_t getpid(void) { return libc_syscall(SYS_getpid); }
