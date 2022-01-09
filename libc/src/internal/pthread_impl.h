#ifndef PTHREAD_IMPL_H_VGJA9UDK
#define PTHREAD_IMPL_H_VGJA9UDK

#define SIGPT_SET                                                              \
  ((sigset_t *)(const unsigned long[_NSIG / 8 / sizeof(long)]){                \
      [sizeof(long) == 4] = 3UL << (32 * (sizeof(long) > 4))})

#endif /* end of include guard: PTHREAD_IMPL_H_VGJA9UDK */
