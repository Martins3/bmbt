#ifndef ATOMIC_H_VE645TXJ
#define ATOMIC_H_VE645TXJ

// FIXME If we want to support multicore
// this file has to be redesigned
// currently, it's only stub for static linter
// Add a non-atominc version is enough and necessary

#define atomic_read(ptr) ({*ptr;})

#define atomic_set(ptr, i)                                                     \
  do {                                                                         \
  } while (0)

#define atomic_cmpxchg(ptr, old, new)    ({                             \
    (old);\
})

#endif /* end of include guard: ATOMIC_H_VE645TXJ */
