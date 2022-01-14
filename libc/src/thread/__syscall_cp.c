#include "syscall.h"

hidden long libc_syscall_cp_c();

static long sccp(syscall_arg_t nr, syscall_arg_t u, syscall_arg_t v,
                 syscall_arg_t w, syscall_arg_t x, syscall_arg_t y,
                 syscall_arg_t z) {
  return libc_syscall(nr, u, v, w, x, y, z);
}

weak_alias(sccp, libc_syscall_cp_c);

long(libc_syscall_cp)(syscall_arg_t nr, syscall_arg_t u, syscall_arg_t v,
                      syscall_arg_t w, syscall_arg_t x, syscall_arg_t y,
                      syscall_arg_t z) {
  return libc_syscall_cp_c(nr, u, v, w, x, y, z);
}
