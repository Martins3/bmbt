#ifndef _COMMON_H_
#define _COMMON_H_

#include "latx-types.h"
#include "ds-set.h"
#include "mem.h"
#include "error.h"

#define LATX_SYS_FCSR
#define LATX_SYS_FCSR_SIMD
/* #define LATX_SYS_FCSR_EXCP TODO */

/*
 * LATX_SYS_FCSR      : map x86 FPU FCSR => LA FCSR
 * LATX_SYS_FCSR_SIMD : enabled assert for x86 SIMD FCSR
 * LATX_SYS_FCSR_EXCP : enable x86 FPU exception TODO
 */

#define BITS_ARE_SET_ANY(value, bits) (((value) & (bits)) != 0)
#define BITS_ARE_SET_ALL(value, bits) (((value) & (bits)) == (bits))
#define BITS_ARE_SET(value, bits) BITS_ARE_SET_ALL(value, bits)
#define BITS_ARE_CLEAR_ANY(value, bits) (((value) & (bits)) != (bits))
#define BITS_ARE_CLEAR_ALL(value, bits) (((value) & (bits)) == 0)
#define BITS_ARE_CLEAR(value, bits) BITS_ARE_CLEAR_ALL(value, bits)
#define BITS_SET(value, bits) ((value) |= (bits))
#define BITS_CLEAR(value, bits) ((value) &= ~(bits))
#define HAS_OTHER_BITS(int, bits) (((int)&(~(bits))) != 0)

#define X86_PAGE_SIZE_ZERO_BITS_NUM (12)
#define X86_PAGE_SIZE (1 << X86_PAGE_SIZE_ZERO_BITS_NUM) /* 4096 */
#define X86_PAGE_MASK (~(X86_PAGE_SIZE - 1))
#define X86_PAGE_OFFSET(addr) (addr & (X86_PAGE_SIZE - 1))
#define X86_PAGE_IS_ALIGN(addr) (((addr) & (X86_PAGE_SIZE - 1)) == 0)
#define X86_PAGE_ALIGN_CEIL(addr) (((addr) + X86_PAGE_SIZE - 1) & X86_PAGE_MASK)
#define X86_PAGE_ALIGN_FLOOR(addr) ((addr)(&X86_PAGE_MASK))

#define MIPS_PAGE_SIZE_ZERO_BITS_NUM (14)
#define MIPS_PAGE_SIZE (1 << MIPS_PAGE_SIZE_ZERO_BITS_NUM) /* 16384 */
#define MIPS_PAGE_MASK (~(MIPS_PAGE_SIZE - 1))
#define MIPS_PAGE_OFFSET(addr) (addr & (MIPS_PAGE_SIZE - 1))
#define MIPS_PAGE_IS_ALIGN(addr) (((addr) & (MIPS_PAGE_SIZE - 1)) == 0)
#define MIPS_PAGE_ALIGN_CEIL(addr) \
    (((addr) + MIPS_PAGE_SIZE - 1) & MIPS_PAGE_MASK)
#define MIPS_PAGE_ALIGN_FLOOR(addr) ((addr)(&MIPS_PAGE_MASK))

#ifndef offsetof
#define offsetof(st, m) __builtin_offsetof(st, m)
#endif

#ifndef _Bool
#define _Bool char
#endif

#ifndef bool
#define bool _Bool
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

enum {
    UNKNOWN_EXTENSION = 96,
    SIGN_EXTENSION,
    ZERO_EXTENSION,
    EM_MIPS_ADDRESS,
    EM_X86_ADDRESS,
};
typedef int8 EXTENSION_MODE;

enum {
    EXMode_N = 96,
    EXMode_S,
    EXMode_Z,
};
typedef int8_t EXMode;
typedef int8_t EXBits;

/* @n should be in [1, 31] */
#define int32_in_intn(x, n) (!(((x) + (1 << ((n) - 1))) & (~((1 << n) - 1))))

#define int32_in_int4(x)   (!(((x) + 0x8) & (~(0xf))))
#define int32_in_int5(x)   int32_in_intn(x, 5)
#define int32_in_int8(x)   (!(((x) + 0x80) & (~(0xff))))
#define int32_in_int9(x)   int32_in_intn(x, 9)
#define int32_in_int10(x)  int32_in_intn(x, 10)
#define int32_in_int11(x)  int32_in_intn(x, 11)
#define int32_in_int12(x)  int32_in_intn(x, 12)
#define int32_in_int13(x)  int32_in_intn(x, 13)
#define int32_in_int14(x)  int32_in_intn(x, 14)
#define int32_in_int16(x)  (!(((x) + 0x8000) & (~(0xffff))))
#define int32_in_int20(x)  int32_in_intn(x, 20)
#define int32_in_int21(x)  int32_in_intn(x, 21)
#define int32_in_int26(x)  int32_in_intn(x, 26)

/* @n should be in [1, 31] */
#define uint32_in_uintn(x, n)   (!((uint32_t)(x) >> (n)))

#define uint32_in_uint1(x)      uint32_in_uintn(x, 1)
#define uint32_in_uint2(x)      uint32_in_uintn(x, 2)
#define uint32_in_uint3(x)      uint32_in_uintn(x, 3)
#define uint32_in_uint4(x)      uint32_in_uintn(x, 4)
#define uint32_in_uint5(x)      uint32_in_uintn(x, 5)
#define uint32_in_uint6(x)      uint32_in_uintn(x, 6)
#define uint32_in_uint7(x)      uint32_in_uintn(x, 7)
#define uint32_in_uint8(x)      uint32_in_uintn(x, 8)
#define uint32_in_uint12(x)     uint32_in_uintn(x, 12)

#ifndef N64
#define N64
#endif

#endif /* _COMMON_H_ */
