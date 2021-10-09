#ifndef COMPILER_H_LG8KIJYO
#define COMPILER_H_LG8KIJYO

#ifndef likely
#if __GNUC__ < 3
#define __builtin_expect(x, n) (x)
#endif
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#ifndef glue
#define xglue(x, y) x##y
#define glue(x, y) xglue(x, y)
#define stringify(s) tostring(s)
#define tostring(s) #s
#endif

#ifndef container_of
#define container_of(ptr, type, member)                                        \
  ({                                                                           \
    const typeof(((type *)0)->member) *__mptr = (ptr);                         \
    (type *)((char *)__mptr - offsetof(type, member));                         \
  })
#endif

#define QEMU_NORETURN __attribute__((__noreturn__))
#define GCC_FMT_ATTR(n, m) __attribute__((format(printf, n, m)))

#define QEMU_PACKED __attribute__((packed))

#define QEMU_BUILD_BUG_MSG(x, msg) _Static_assert(!(x), msg)
#define QEMU_BUILD_BUG_ON(x) QEMU_BUILD_BUG_MSG(x, "not expecting: " #x)

#define QEMU_ALWAYS_INLINE __attribute__((always_inline))

#define qemu_build_not_reached() g_assert_not_reached()

#define QEMU_BUILD_BUG_ON_STRUCT(x)                                            \
  struct {                                                                     \
    int : (x) ? -1 : 1;                                                        \
  }

#define QEMU_BUILD_BUG_ON_ZERO(x)                                              \
  (sizeof(QEMU_BUILD_BUG_ON_STRUCT(x)) - sizeof(QEMU_BUILD_BUG_ON_STRUCT(x)))

#define sizeof_field(type, field) sizeof(((type *)0)->field)

/*
 * GCC doesn't provide __has_attribute() until GCC 5, but we know all the GCC
 * versions we support have the "flatten" attribute. Clang may not have the
 * "flatten" attribute but always has __has_attribute() to check for it.
 */
#if __has_attribute(flatten) || !defined(__clang__)
#define QEMU_FLATTEN __attribute__((flatten))
#else
#define QEMU_FLATTEN
#endif

#define bmbt_assert()                                                          \
  do {                                                                         \
    printf("not implement yet %s %d\n", __FUNCTION__, __LINE__);               \
    assert(0);                                                                 \
  } while (0);

#define duck_check(x)                                                          \
  {                                                                            \
    if (!(x)) {                                                                \
      g_assert_not_reached()                                                   \
    }                                                                          \
  }

#endif /* end of include guard: COMPILER_H_LG8KIJYO */
