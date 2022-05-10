#ifndef INTERNAL_H_IMYZ2RLB
#define INTERNAL_H_IMYZ2RLB

void uart_putc(char c);
int kern_printf(const char *fmt, ...);

_Noreturn void __kern_assert_fail(const char *expr, const char *file, int line,
                                  const char *func);
#define kern_not_reach(x) __kern_assert_fail(#x, __FILE__, __LINE__, __func__)

#define kern_assert(x)                                                         \
  ((void)((x) || (__kern_assert_fail(#x, __FILE__, __LINE__, __func__), 0)))

long kernel_writev(long arg0, long arg1, long arg2, long arg3, long arg4,
                   long arg5, long arg6);

long kernel_mmap(long arg0, long arg1, long arg2, long arg3, long arg4,
                 long arg5, long arg6);

long kernel_unmmap(long arg0, long arg1, long arg2, long arg3, long arg4,
                   long arg5, long arg6);

void early_console_write(const char *s, unsigned n);

long kernel_clock_gettime(long arg0, long arg1, long arg2, long arg3, long arg4,
                          long arg5, long arg6);

// TMP_TODO 修改为 static inline
long alloc_pages(long pages);
#endif /* end of include guard: INTERNAL_H_IMYZ2RLB */
