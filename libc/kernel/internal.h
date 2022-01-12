#ifndef INTERNAL_H_IMYZ2RLB
#define INTERNAL_H_IMYZ2RLB

int duck_printf(const char *fmt, ...);

void __duck_assert_fail(const char *expr, const char *file, int line,
                             const char *func);
#define duck_assert(x)                                                         \
  ((void)((x) || (__duck_assert_fail(#x, __FILE__, __LINE__, __func__), 0)))

long kernel_writev(long arg0, long arg1, long arg2, long arg3, long arg4,
                   long arg5, long arg6);

void early_console_write(const char *s, unsigned n);

#endif /* end of include guard: INTERNAL_H_IMYZ2RLB */
