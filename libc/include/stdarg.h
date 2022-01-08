#ifndef STDARG_H_YSNU3HWT
#define STDARG_H_YSNU3HWT

#define __NEED_va_list

#include <bits/alltypes.h>

#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)
#define va_copy(d, s) __builtin_va_copy(d, s)

#endif /* end of include guard: STDARG_H_YSNU3HWT */
