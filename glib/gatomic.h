#ifndef GLIB_STUB_H_78QYZX2K
#define GLIB_STUB_H_78QYZX2K

#include <stdlib.h>
#ifndef USE_SYSTEM_GLIB

#define g_atomic_int_inc(atomic) \
  (G_GNUC_EXTENSION ({                                                       \
    assert(sizeof *(atomic) == sizeof (gint));                               \
    (void) (0 ? *(atomic) ^ *(atomic) : 1);                                  \
    (void) __sync_fetch_and_add ((atomic), 1);                               \
  }))

#define g_atomic_int_dec_and_test(atomic) \
  (G_GNUC_EXTENSION ({                                                       \
    assert(sizeof *(atomic) == sizeof (gint));                               \
    (void) (0 ? *(atomic) ^ *(atomic) : 1);                                  \
    __sync_fetch_and_sub ((atomic), 1) == 1;                                 \
  }))

#define G_STRINGIFY(macro_or_string)	G_STRINGIFY_ARG (macro_or_string)
#define	G_STRINGIFY_ARG(contents)	#contents
#define G_STRLOC	__FILE__ ":" G_STRINGIFY (__LINE__)

#ifdef __has_attribute
#define g_macro__has_attribute __has_attribute
#else
#define g_macro__has_attribute(x) 0
#endif

#define G_GNUC_MALLOC __attribute__((__malloc__))

#if (!defined(__clang__) && ((__GNUC__ > 4) ||  \
    (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))) || \
    (defined(__clang__) && g_macro__has_attribute(__alloc_size__))
#define G_GNUC_ALLOC_SIZE(x) __attribute__((__alloc_size__(x)))
#define G_GNUC_ALLOC_SIZE2(x,y) __attribute__((__alloc_size__(x,y)))
#else
#define G_GNUC_ALLOC_SIZE(x)
#define G_GNUC_ALLOC_SIZE2(x,y)
#endif

#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#define G_GNUC_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define G_GNUC_WARN_UNUSED_RESULT
#endif /* __GNUC__ */

#else
#include <glib.h>
#endif

#endif /* end of include guard: GLIB_STUB_H_78QYZX2K */

