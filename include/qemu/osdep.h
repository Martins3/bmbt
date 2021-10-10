#ifndef OSDEP_H_DXJTBG8M
#define OSDEP_H_DXJTBG8M

#include "../types.h"
#include <stdint.h>

/* Round number up to multiple. Requires that d be a power of 2 (see
 * QEMU_ALIGN_UP for a safer but slower version on arbitrary
 * numbers); works even if d is a smaller type than n.  */
#ifndef ROUND_UP
#define ROUND_UP(n, d) (((n) + (d)-1) & -(0 ? (n) : (d)))
#endif

/*
 * &(x)[0] is always a pointer - if it's same type as x then the argument is a
 * pointer, not an array.
 */
#define QEMU_IS_ARRAY(x)                                                       \
  (!__builtin_types_compatible_p(typeof(x), typeof(&(x)[0])))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)                                                          \
  ((sizeof(x) / sizeof((x)[0])) + QEMU_BUILD_BUG_ON_ZERO(!QEMU_IS_ARRAY(x)))
#endif

extern int qemu_icache_linesize;
extern int qemu_icache_linesize_log;
extern int qemu_dcache_linesize;
extern int qemu_dcache_linesize_log;

/* HOST_LONG_BITS is the size of a native pointer in bits. */
#if UINTPTR_MAX == UINT32_MAX
#define HOST_LONG_BITS 32
#elif UINTPTR_MAX == UINT64_MAX
#define HOST_LONG_BITS 64
#else
#error Unknown pointer size
#endif

/* Using intptr_t ensures that qemu_*_page_mask is sign-extended even
 * when intptr_t is 32-bit and we are aligning a long long.
 */
extern uintptr_t qemu_real_host_page_size;
extern intptr_t qemu_real_host_page_mask;

void init_real_host_page_size(void);
void init_cache_info(void);

#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))
#endif

/* Round number down to multiple */
#define QEMU_ALIGN_DOWN(n, m) ((n) / (m) * (m))

/* Round number up to multiple. Safe when m is not a power of 2 (see
 * ROUND_UP for a faster version when a power of 2 is guaranteed) */
#define QEMU_ALIGN_UP(n, m) QEMU_ALIGN_DOWN((n) + (m)-1, (m))

/* n-byte align pointer up */
#define QEMU_ALIGN_PTR_UP(p, n) ((typeof(p))QEMU_ALIGN_UP((uintptr_t)(p), (n)))

/* n-byte align pointer down */
#define QEMU_ALIGN_PTR_DOWN(p, n)                                              \
  ((typeof(p))QEMU_ALIGN_DOWN((uintptr_t)(p), (n)))

int qemu_mprotect_none(void *addr, size_t size);
void *qemu_memalign(size_t alignment, size_t size);

/* Starting on QEMU 2.5, qemu_hw_version() returns "2.5+" by default
 * instead of QEMU_VERSION, so setting hw_version on MachineClass
 * is no longer mandatory.
 *
 * Do NOT change this string, or it will break compatibility on all
 * machine classes that don't set hw_version.
 */
#define QEMU_HW_VERSION "2.5+"

#endif /* end of include guard: OSDEP_H_DXJTBG8M */
