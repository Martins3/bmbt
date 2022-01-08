#ifndef _SYS_ASM_H
#define _SYS_ASM_H

#include "regdef.h"
#include <features.h>
// #include "sysdep.h"

// bmbt: copied from sysdeps/generic/sysdep.h
#define cfi_startproc .cfi_startproc
#define cfi_endproc .cfi_endproc

/* Macros to handle different pointer/register sizes for 32/64-bit code.  */
#ifdef __loongarch64
#define PTRLOG 3
#define SZREG 8
#define SZFREG 8
#define REG_L ld.d
#define REG_S st.d
#define FREG_L fld.d
#define FREG_S fst.d
#else
#error __loongarch_xlen must equal 64
#endif

/* Declare leaf routine.  */
#define LEAF(symbol)                                                           \
  .text;                                                                       \
  .globl symbol;                                                               \
  .align 3;                                                                    \
  cfi_startproc;                                                               \
  .type symbol, @function;                                                     \
  symbol:

#define ENTRY(symbol) LEAF(symbol)

/* Mark end of function.  */
#undef END
#define END(function)                                                          \
  cfi_endproc;                                                                 \
  .size function, .- function;

/* Stack alignment.  */
#define ALMASK ~15

#endif /* sys/asm.h */
