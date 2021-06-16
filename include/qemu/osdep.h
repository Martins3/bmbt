#ifndef OSDEP_H_DXJTBG8M
#define OSDEP_H_DXJTBG8M

#include "../types.h"


/* Round number up to multiple. Requires that d be a power of 2 (see
 * QEMU_ALIGN_UP for a safer but slower version on arbitrary
 * numbers); works even if d is a smaller type than n.  */
#ifndef ROUND_UP
#define ROUND_UP(n, d) (((n) + (d) - 1) & -(0 ? (n) : (d)))
#endif

/*
 * &(x)[0] is always a pointer - if it's same type as x then the argument is a
 * pointer, not an array.
 */
#define QEMU_IS_ARRAY(x) (!__builtin_types_compatible_p(typeof(x), \
                                                        typeof(&(x)[0])))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) ((sizeof(x) / sizeof((x)[0])) + \
                       QEMU_BUILD_BUG_ON_ZERO(!QEMU_IS_ARRAY(x)))
#endif

// FIXME defined in util/cacheinfo.c
extern int qemu_icache_linesize;
extern int qemu_icache_linesize_log;
extern int qemu_dcache_linesize;
extern int qemu_dcache_linesize_log;

#endif /* end of include guard: OSDEP_H_DXJTBG8M */
