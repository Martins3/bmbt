#include "gmacros.h"
#include "glibconfig.h"
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#ifndef USE_SYSTEM_GLIB

#define g_assert(expr)                                    \
  {                                                       \
    if (!(expr)) {                                        \
      printf("crashed");                                  \
      exit(1);                                            \
    }                                                     \
  }

#define g_assert_not_reached()                            \
  { exit(0); }

#define g_assert_true(expr) \
  G_STMT_START { \
       if G_LIKELY (expr) ; else \
         fprintf (stderr, "'%d' should be TRUE", expr); \
  } G_STMT_END
#define g_assert_false(expr) \
  G_STMT_START { \
       if G_LIKELY (!(expr)) ; else \
          fprintf (stderr, "'%d' should be FALSE", expr); \
  } G_STMT_END
#define g_assert_cmpfloat(n1,cmp,n2) \
  G_STMT_START { \
       long double __n1 = (long double) (n1), __n2 = (long double) (n2); \
       if (__n1 cmp __n2) ; else \
          fprintf (stderr, "%Lf " #cmp " %Lf", __n1, __n2);\
  } G_STMT_END
#define g_assert_cmpuint(n1, cmp, n2) \
  G_STMT_START { \
       guint64 __n1 = (n1), __n2 = (n2); \
       if (__n1 cmp __n2) ; else \
          fprintf (stderr, "%ld " #cmp " %ld", __n1, __n2);\
  } G_STMT_END

#else
#include <glib.h>
#endif

