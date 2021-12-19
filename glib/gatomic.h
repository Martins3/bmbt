#ifndef GATOMIC_H_CVKD9N7D
#define GATOMIC_H_CVKD9N7D
#include "gmacros.h"

#define g_atomic_int_inc(atomic)                                               \
  (G_GNUC_EXTENSION({                                                          \
    G_STATIC_ASSERT(sizeof *(atomic) == sizeof(gint));                         \
    (void)(0 ? *(atomic) ^ *(atomic) : 1);                                     \
    (void)__atomic_fetch_add((atomic), 1, __ATOMIC_SEQ_CST);                   \
  }))

#define g_atomic_int_dec_and_test(atomic)                                      \
  (G_GNUC_EXTENSION({                                                          \
    G_STATIC_ASSERT(sizeof *(atomic) == sizeof(gint));                         \
    (void)(0 ? *(atomic) ^ *(atomic) : 1);                                     \
    __atomic_fetch_sub((atomic), 1, __ATOMIC_SEQ_CST) == 1;                    \
  }))
#endif /* end of include guard: GATOMIC_H_CVKD9N7D */
