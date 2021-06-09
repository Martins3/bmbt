#ifndef TYPES_H_OZP1YQJN
#define TYPES_H_OZP1YQJN

#undef NULL
#define NULL ((void *)0)

#define EOF -1

#ifdef __ASSEMBLY__
#define _CONST64_
#else
#define _CONST64_(x) x##l
typedef unsigned int size_t;
typedef unsigned long long u64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef int i32;

typedef u64 tb_page_addr_t;
typedef u32 tcg_insn_unit;

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
#define xglue(x, y) x ## y
#define glue(x, y) xglue(x, y)
#define stringify(s)	tostring(s)
#define tostring(s)	#s
#endif

#endif

#endif /* end of include guard: TYPES_H_OZP1YQJN */
