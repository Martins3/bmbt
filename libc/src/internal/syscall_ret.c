#include "syscall.h"
#include <errno.h>

long libc_syscall_ret(unsigned long r) {
  if (r > -4096UL) {
    errno = -r;
    return -1;
  }
  return r;
}
