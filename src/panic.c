
#define BUG_ON(conditon)                                                       \
  do {                                                                         \
    if (conditon) {                                                            \
      printf("%s:%s failed\n", __FUNCTION__, __LINE__);                        \
      while (1)                                                                \
        ;                                                                      \
    }                                                                          \
  } while (0);
