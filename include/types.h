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
#include <stdint.h>
#include <stdio.h> // for EOF

typedef unsigned long long u64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#include "qemu/compiler.h"

#endif

#endif /* end of include guard: TYPES_H_OZP1YQJN */
