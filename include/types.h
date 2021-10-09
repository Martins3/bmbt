#ifndef TYPES_H_OZP1YQJN
#define TYPES_H_OZP1YQJN

#ifdef __ASSEMBLY__
#define _CONST64_
#else
#define _CONST64_(x) x##l
#include "qemu/config-host.h"
#include "qemu/config-target.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h> // for EOF

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

#include "qemu/compiler.h"

#endif

#endif /* end of include guard: TYPES_H_OZP1YQJN */
