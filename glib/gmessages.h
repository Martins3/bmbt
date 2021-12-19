#ifndef GMESSAGES_H_OLO5QNZH
#define GMESSAGES_H_OLO5QNZH
#include <assert.h>

// [interface 49]
#define g_return_if_fail(expr)                                                 \
  G_STMT_START {                                                               \
    if (G_LIKELY(expr)) {                                                      \
    } else {                                                                   \
      assert(false);                                                           \
      return;                                                                  \
    }                                                                          \
  }                                                                            \
  G_STMT_END

#define g_return_val_if_fail(expr, val)                                        \
  G_STMT_START {                                                               \
    if (G_LIKELY(expr)) {                                                      \
    } else {                                                                   \
      assert(false);                                                           \
      return (val);                                                            \
    }                                                                          \
  }                                                                            \
  G_STMT_END

#define g_error(...)                                                           \
  G_STMT_START {                                                               \
    fprintf(stderr, __VA_ARGS__);                                              \
    assert(false);                                                             \
    for (;;)                                                                   \
      ;                                                                        \
  }                                                                            \
  G_STMT_END

#endif /* end of include guard: GMESSAGES_H_OLO5QNZH */
