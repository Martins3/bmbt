#ifndef GTESTUTILS_H_PYMBKDEC
#define GTESTUTILS_H_PYMBKDEC

#include "glibconfig.h"
#include "gmacros.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define g_assert(expr)                                                         \
  {                                                                            \
    if (!(expr)) {                                                             \
      assert(false);                                                           \
    }                                                                          \
  }

#define g_assert_not_reached()                                                 \
  {                                                                            \
    printf("%s:%d: g_assert_not_reached", __func__, __LINE__);                 \
    exit(1);                                                                   \
  }

#define g_assert_true(expr)                                                    \
  G_STMT_START {                                                               \
    if G_LIKELY (expr)                                                         \
      ;                                                                        \
    else                                                                       \
      assert(false);                                                           \
  }                                                                            \
  G_STMT_END

#define g_assert_false(expr)                                                   \
  G_STMT_START {                                                               \
    if G_LIKELY (!(expr))                                                      \
      ;                                                                        \
    else                                                                       \
      fprintf(stderr, "'%d' should be FALSE", expr);                           \
  }                                                                            \
  G_STMT_END

#define g_assert_cmpfloat(n1, cmp, n2)                                         \
  G_STMT_START {                                                               \
    long double __n1 = (long double)(n1), __n2 = (long double)(n2);            \
    if (__n1 cmp __n2)                                                         \
      ;                                                                        \
    else                                                                       \
      assert(false);                                                           \
  }                                                                            \
  G_STMT_END

#define g_assert_cmpuint(n1, cmp, n2)                                          \
  G_STMT_START {                                                               \
    guint64 __n1 = (n1), __n2 = (n2);                                          \
    if (__n1 cmp __n2)                                                         \
      ;                                                                        \
    else                                                                       \
      assert(false);                                                           \
  }                                                                            \
  G_STMT_END

#endif /* end of include guard: GTESTUTILS_H_PYMBKDEC */
