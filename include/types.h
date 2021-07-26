#ifndef TYPES_H_OZP1YQJN
#define TYPES_H_OZP1YQJN


#ifdef __ASSEMBLY__
#define _CONST64_
#else
#define _CONST64_(x) x##l
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h> // for EOF
#include "qemu/config-host.h"
#include "qemu/config-target.h"

// FIXME size_t conflicts with <stddef>, as for how to deal with glibc
// no solution yet.

// typedef unsigned int size_t;
typedef unsigned long long u64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef int i32;

typedef u64 tb_page_addr_t;

typedef unsigned long int uintptr_t;

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int int16_t;
typedef unsigned short int uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long int int64_t;
typedef unsigned long int uint64_t;

// FIXME below belong to compiler.h
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

#ifndef offsetof
#define offsetof(st, m) __builtin_offsetof(st, m)
#endif

// FIXME originally, container_of copied from
// home/maritns3/core/ld/x86-qemu-mips/slirp/src/util.h
// but it cause env_archcpu work abnormal, following version
// is copied from linux kenrel
//
// It's really disturbing that env_archcpu complaints again
// I don't know what's happending
#ifndef container_of
#define container_of(ptr, type, member)                                        \
  ({                                                                           \
    void *__mptr = (void *)(ptr);                                              \
    ((type *)(__mptr - offsetof(type, member)));                               \
  })

#endif

// FIXME This line is belongs to compiler.h
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

#define bmbt_assert()                                                          \
  do {                                                                         \
    printf("not implement yet %s %d\n", __FUNCTION__, __LINE__);               \
    assert(0);                                                                 \
  } while (0);

/*
 * GCC doesn't provide __has_attribute() until GCC 5, but we know all the GCC
 * versions we support have the "flatten" attribute. Clang may not have the
 * "flatten" attribute but always has __has_attribute() to check for it.
 */
#if __has_attribute(flatten) || !defined(__clang__)
# define QEMU_FLATTEN __attribute__((flatten))
#else
# define QEMU_FLATTEN
#endif

#endif

#endif /* end of include guard: TYPES_H_OZP1YQJN */
