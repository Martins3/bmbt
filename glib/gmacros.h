#ifndef GMACROS_H_17TQZ2BG
#define GMACROS_H_17TQZ2BG

#include <stdlib.h>
/*
 * The G_LIKELY and G_UNLIKELY macros let the programmer give hints to
 * the compiler about the expected result of an expression. Some compilers
 * can use this information for optimizations.
 *
 * The _G_BOOLEAN_EXPR macro is intended to trigger a gcc warning when
 * putting assignments in g_return_if_fail ().
 */
#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define _G_BOOLEAN_EXPR(expr)                                                  \
  G_GNUC_EXTENSION({                                                           \
    int _g_boolean_var_;                                                       \
    if (expr)                                                                  \
      _g_boolean_var_ = 1;                                                     \
    else                                                                       \
      _g_boolean_var_ = 0;                                                     \
    _g_boolean_var_;                                                           \
  })
#define G_LIKELY(expr) (__builtin_expect(_G_BOOLEAN_EXPR(expr), 1))
#define G_UNLIKELY(expr) (__builtin_expect(_G_BOOLEAN_EXPR(expr), 0))
#else
#define G_LIKELY(expr) (expr)
#define G_UNLIKELY(expr) (expr)
#endif

/* Here we provide G_GNUC_EXTENSION as an alias for __extension__,
 * where this is valid. This allows for warningless compilation of
 * "long long" types even in the presence of '-ansi -pedantic'.
 */
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 8)
#define G_GNUC_EXTENSION __extension__
#else
#define G_GNUC_EXTENSION
#endif

#undef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#undef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/* Provide a string identifying the current function, non-concatenatable */
#if defined(__func__)
#define G_STRFUNC ((const char *)(__func__))
#elif defined(__GNUC__) && defined(__cplusplus)
#define G_STRFUNC ((const char *)(__PRETTY_FUNCTION__))
#elif defined(__GNUC__) || (defined(_MSC_VER) && (_MSC_VER > 1300))
#define G_STRFUNC ((const char *)(__FUNCTION__))
#else
#define G_STRFUNC ((const char *)("???"))
#endif

#define G_STMT_START do
#define G_STMT_END while (0)

#define G_GNUC_PRINTF(format_idx, arg_idx)                                     \
  __attribute__((__format__(__printf__, format_idx, arg_idx)))

#define G_GNUC_MALLOC __attribute__((__malloc__))
#endif /* end of include guard: GMACROS_H_17TQZ2BG */
