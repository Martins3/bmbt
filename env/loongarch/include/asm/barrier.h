#ifndef BARRIER_H_LPXTU4I8
#define BARRIER_H_LPXTU4I8

#define __sync()                                                               \
  __asm__ __volatile__("dbar 0\n\t"                                            \
                       : /* no output */                                       \
                       : /* no input */                                        \
                       : "memory")

#define fast_wmb() __sync()
#define fast_rmb() __sync()
#define fast_mb() __sync()

#define fast_iob() __sync()

#define wmb() fast_wmb()
#define rmb() fast_rmb()
#define mb() fast_mb()
#define iob() fast_iob()

#endif /* end of include guard: BARRIER_H_LPXTU4I8 */
