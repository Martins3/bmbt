#ifndef _INTERNAL_SYSCALL_H
#define _INTERNAL_SYSCALL_H

#include "../bits/loongarch/syscall_arch.h"
#include <errno.h>
#include <features.h>
#include <sys/syscall.h>

#ifndef SYSCALL_RLIM_INFINITY
#define SYSCALL_RLIM_INFINITY (~0ULL)
#endif

#ifndef SYSCALL_MMAP2_UNIT
#define SYSCALL_MMAP2_UNIT 4096ULL
#endif

#ifndef __SYSCALL_LL_PRW
#define __SYSCALL_LL_PRW(x) __SYSCALL_LL_O(x)
#endif

#ifndef __scc
#define __scc(X) ((long)(X))
typedef long syscall_arg_t;
#endif

hidden long libc_syscall_ret(unsigned long),
    libc_syscall_cp(syscall_arg_t, syscall_arg_t, syscall_arg_t, syscall_arg_t,
                    syscall_arg_t, syscall_arg_t, syscall_arg_t);

#define libc_syscall1(n, a) libc_syscall1(n, __scc(a))
#define libc_syscall2(n, a, b) libc_syscall2(n, __scc(a), __scc(b))
#define libc_syscall3(n, a, b, c) libc_syscall3(n, __scc(a), __scc(b), __scc(c))
#define libc_syscall4(n, a, b, c, d)                                           \
  libc_syscall4(n, __scc(a), __scc(b), __scc(c), __scc(d))
#define libc_syscall5(n, a, b, c, d, e)                                        \
  libc_syscall5(n, __scc(a), __scc(b), __scc(c), __scc(d), __scc(e))
#define libc_syscall6(n, a, b, c, d, e, f)                                     \
  libc_syscall6(n, __scc(a), __scc(b), __scc(c), __scc(d), __scc(e), __scc(f))
#define libc_syscall7(n, a, b, c, d, e, f, g)                                  \
  libc_syscall7(n, __scc(a), __scc(b), __scc(c), __scc(d), __scc(e), __scc(f), \
                __scc(g))

#define __SYSCALL_NARGS_X(a, b, c, d, e, f, g, h, n, ...) n
#define __SYSCALL_NARGS(...)                                                   \
  __SYSCALL_NARGS_X(__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0, )
#define __SYSCALL_CONCAT_X(a, b) a##b
#define __SYSCALL_CONCAT(a, b) __SYSCALL_CONCAT_X(a, b)
#define __SYSCALL_DISP(b, ...)                                                 \
  __SYSCALL_CONCAT(b, __SYSCALL_NARGS(__VA_ARGS__))(__VA_ARGS__)

#define libc_syscall(...) __SYSCALL_DISP(libc_syscall, __VA_ARGS__)
#define syscall(...) libc_syscall_ret(libc_syscall(__VA_ARGS__))

#define socketcall(nm, a, b, c, d, e, f)                                       \
  libc_syscall_ret(__socketcall(nm, a, b, c, d, e, f))
#define socketcall_cp(nm, a, b, c, d, e, f)                                    \
  libc_syscall_ret(__socketcall_cp(nm, a, b, c, d, e, f))

#define libc_syscall_cp0(n) (libc_syscall_cp)(n, 0, 0, 0, 0, 0, 0)
#define libc_syscall_cp1(n, a) (libc_syscall_cp)(n, __scc(a), 0, 0, 0, 0, 0)
#define libc_syscall_cp2(n, a, b)                                              \
  (libc_syscall_cp)(n, __scc(a), __scc(b), 0, 0, 0, 0)
#define libc_syscall_cp3(n, a, b, c)                                           \
  (libc_syscall_cp)(n, __scc(a), __scc(b), __scc(c), 0, 0, 0)
#define libc_syscall_cp4(n, a, b, c, d)                                        \
  (libc_syscall_cp)(n, __scc(a), __scc(b), __scc(c), __scc(d), 0, 0)
#define libc_syscall_cp5(n, a, b, c, d, e)                                     \
  (libc_syscall_cp)(n, __scc(a), __scc(b), __scc(c), __scc(d), __scc(e), 0)
#define libc_syscall_cp6(n, a, b, c, d, e, f)                                  \
  (libc_syscall_cp)(n, __scc(a), __scc(b), __scc(c), __scc(d), __scc(e),       \
                    __scc(f))

#define libc_syscall_cp(...) __SYSCALL_DISP(libc_syscall_cp, __VA_ARGS__)
#define syscall_cp(...) libc_syscall_ret(libc_syscall_cp(__VA_ARGS__))

#endif
