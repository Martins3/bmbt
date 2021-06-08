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

typedef u64 target_ulong ;
#endif

#endif /* end of include guard: TYPES_H_OZP1YQJN */
