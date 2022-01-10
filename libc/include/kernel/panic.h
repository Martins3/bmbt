#ifndef PANIC_H_KHEFPWVT
#define PANIC_H_KHEFPWVT
#include "interface.h"

#define BUG_ON(conditon)                                                       \
  do {                                                                         \
    if (conditon) {                                                            \
      duck_printf("%s:%d failed\n", __FUNCTION__, __LINE__);                   \
      while (1)                                                                \
        ;                                                                      \
    }                                                                          \
  } while (0);

#endif /* end of include guard: PANIC_H_KHEFPWVT */
