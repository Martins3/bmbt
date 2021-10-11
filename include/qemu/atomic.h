#ifndef ATOMIC_H_VE645TXJ
#define ATOMIC_H_VE645TXJ
// [interface 31]

/* The variable that receives the old value of an atomically-accessed
 * variable must be non-qualified, because atomic builtins return values
 * through a pointer-type argument as in __atomic_load(&var, &old, MODEL).
 *
 * This macro has to handle types smaller than int manually, because of
 * implicit promotion.  int and larger types, as well as pointers, can be
 * converted to a non-qualified type just by applying a binary operator.
 */
#define typeof_strip_qual(expr)                                                \
  typeof(__builtin_choose_expr(                                                \
      __builtin_types_compatible_p(typeof(expr), bool) ||                      \
          __builtin_types_compatible_p(typeof(expr), const bool) ||            \
          __builtin_types_compatible_p(typeof(expr), volatile bool) ||         \
          __builtin_types_compatible_p(typeof(expr), const volatile bool),     \
      (bool)1,                                                                 \
      __builtin_choose_expr(                                                   \
          __builtin_types_compatible_p(typeof(expr), signed char) ||           \
              __builtin_types_compatible_p(typeof(expr), const signed char) || \
              __builtin_types_compatible_p(typeof(expr),                       \
                                           volatile signed char) ||            \
              __builtin_types_compatible_p(typeof(expr),                       \
                                           const volatile signed char),        \
          (signed char)1,                                                      \
          __builtin_choose_expr(                                               \
              __builtin_types_compatible_p(typeof(expr), unsigned char) ||     \
                  __builtin_types_compatible_p(typeof(expr),                   \
                                               const unsigned char) ||         \
                  __builtin_types_compatible_p(typeof(expr),                   \
                                               volatile unsigned char) ||      \
                  __builtin_types_compatible_p(typeof(expr),                   \
                                               const volatile unsigned char),  \
              (unsigned char)1,                                                \
              __builtin_choose_expr(                                           \
                  __builtin_types_compatible_p(typeof(expr), signed short) ||  \
                      __builtin_types_compatible_p(typeof(expr),               \
                                                   const signed short) ||      \
                      __builtin_types_compatible_p(typeof(expr),               \
                                                   volatile signed short) ||   \
                      __builtin_types_compatible_p(                            \
                          typeof(expr), const volatile signed short),          \
                  (signed short)1,                                             \
                  __builtin_choose_expr(                                       \
                      __builtin_types_compatible_p(typeof(expr),               \
                                                   unsigned short) ||          \
                          __builtin_types_compatible_p(                        \
                              typeof(expr), const unsigned short) ||           \
                          __builtin_types_compatible_p(                        \
                              typeof(expr), volatile unsigned short) ||        \
                          __builtin_types_compatible_p(                        \
                              typeof(expr), const volatile unsigned short),    \
                      (unsigned short)1, (expr) + 0))))))

#define ATOMIC_REG_SIZE 8

#define atomic_read(ptr) (*ptr)

#define atomic_set(ptr, i)                                                     \
  do {                                                                         \
    *ptr = i;                                                                  \
  } while (0)

#define atomic_or(ptr, n) ((void)__atomic_fetch_or(ptr, n, __ATOMIC_SEQ_CST))

#define atomic_cmpxchg(ptr, old, new) ({ (old); })

#define atomic_mb_set(ptr, i)                                                  \
  do {                                                                         \
    *ptr = i;                                                                  \
  } while (0)

#define atomic_rcu_read(ptr) (*ptr)

#define atomic_mb_read(ptr) (*ptr)

#define atomic_and(ptr, n) (*ptr &= n)

#define atomic_or_fetch(ptr, n) __atomic_or_fetch(ptr, n, __ATOMIC_SEQ_CST)
#define atomic_fetch_inc(ptr) *ptr = *ptr + 1;

/* Returns the eventual value, failed or not */
#define atomic_cmpxchg__nocheck(ptr, old, new)                                 \
  ({                                                                           \
    typeof_strip_qual(*ptr) _old = (old);                                      \
    *ptr = new;                                                                \
    _old;                                                                      \
  })

#define atomic_fetch_and(ptr, n) __atomic_fetch_and(ptr, n, __ATOMIC_SEQ_CST)

#define atomic_xchg(ptr, i)                                                    \
  ({                                                                           \
    QEMU_BUILD_BUG_ON(sizeof(*ptr) > ATOMIC_REG_SIZE);                         \
    *ptr = i;                                                                  \
  })

/* Compiler barrier */
#define barrier()                                                              \
  ({                                                                           \
    asm volatile("" ::: "memory");                                             \
    (void)0;                                                                   \
  })

#define smp_mb()                                                               \
  ({                                                                           \
    barrier();                                                                 \
    __atomic_thread_fence(__ATOMIC_SEQ_CST);                                   \
  })

#define smp_mb_acquire()                                                       \
  ({                                                                           \
    barrier();                                                                 \
    __atomic_thread_fence(__ATOMIC_ACQUIRE);                                   \
  })

#define smp_mb_release()                                                       \
  ({                                                                           \
    barrier();                                                                 \
    __atomic_thread_fence(__ATOMIC_RELEASE);                                   \
  })

#define smp_rmb() smp_mb_acquire()
#define smp_wmb() smp_mb_release()

#endif /* end of include guard: ATOMIC_H_VE645TXJ */
