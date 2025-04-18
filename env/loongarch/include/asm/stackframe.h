/* SPDX-License-Identifier: GPL-2.0 */
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994, 95, 96, 99, 2001 Ralf Baechle
 * Copyright (C) 1994, 1995, 1996 Paul M. Antoine.
 * Copyright (C) 1999 Silicon Graphics, Inc.
 * Copyright (C) 2007  Maciej W. Rozycki
 * Copyright (C) 2020 Loongson Technology Co., Ltd.
 */
#ifndef _ASM_STACKFRAME_H
#define _ASM_STACKFRAME_H

#include <asm/asm.h>
#include <asm/loongarchregs.h>
#include <asm/regdef.h>
#include <generated/asm-offset.h>

/* Make the addition of cfi info a little easier. */
    .macro cfi_rel_offset reg offset=0 docfi=0
    .if \docfi
    parse_r regval, \reg
    .cfi_rel_offset regval, \offset
    .endif
    .endm

    .macro cfi_st reg offset=0 docfi=0
    LONG_S  \reg, sp, \offset
    .endm

    .macro cfi_sq reg1 reg2 offset=0 docfi=0
    gssq  \reg2, \reg1, \offset(sp)
    .endm

    .macro cfi_restore reg offset=0 docfi=0
    .if \docfi
    parse_r regval, \reg
    .cfi_restore regval
    .endif
    .endm

    .macro cfi_ld reg offset=0 docfi=0
    LONG_L  \reg, sp, \offset
    .endm

    .macro  LOAD_TEMP_FROM_KSCRATCH docfi=0
    csrrd   $r12, LOONGARCH_CSR_KS0
    csrrd   $r13, LOONGARCH_CSR_KS1
    .endm

    .macro  SAVE_TEMP docfi=0
    cfi_st  $r12, PT_R12, \docfi
    cfi_st  $r13, PT_R13, \docfi
    cfi_st  $r14, PT_R14, \docfi
    cfi_st  $r15, PT_R15, \docfi
    cfi_st  $r16, PT_R16, \docfi
    cfi_st  $r17, PT_R17, \docfi
    cfi_st  $r18, PT_R18, \docfi
    cfi_st  $r19, PT_R19, \docfi
    cfi_st  $r20, PT_R20, \docfi
    .endm

    .macro  SAVE_STATIC docfi=0
    cfi_st  $r23, PT_R23, \docfi
    cfi_st  $r24, PT_R24, \docfi
    cfi_st  $r25, PT_R25, \docfi
    cfi_st  $r26, PT_R26, \docfi
    cfi_st  $r27, PT_R27, \docfi
    cfi_st  $r28, PT_R28, \docfi
    cfi_st  $r29, PT_R29, \docfi
    cfi_st  $r30, PT_R30, \docfi
    cfi_st  $r31, PT_R31, \docfi
    .endm

    .macro  SAVE_SOME docfi=0
    move  t0, sp
    PTR_ADDIU sp, sp, -PT_SIZE
    .if \docfi
    .cfi_def_cfa 3,0
    .endif
    cfi_st  t0, PT_R3, \docfi
    cfi_rel_offset  sp, PT_R3, \docfi
    /*
     * You might think that you don't need to save $0,
     * but the FPU emulator and gdb remote debug stub
     * need it to operate correctly
     */
    LONG_S  zero, sp, PT_R0
    cfi_st  $r4, PT_R4, \docfi
    cfi_st  $r5, PT_R5, \docfi
    csrrd t0, LOONGARCH_CSR_PRMD
    LONG_S  t0, sp, PT_PRMD

    // [BMBT_OPTIMIZE 3]
    csrrd t0, LOONGARCH_CSR_CRMD
    LONG_S  t0, sp, PT_CRMD

    csrrd t0, LOONGARCH_CSR_EUEN
    LONG_S  t0, sp, PT_EUEN

    csrrd t0, LOONGARCH_CSR_ECFG
    LONG_S  t0, sp, PT_ECFG

    csrrd t0, LOONGARCH_CSR_ESTAT
    LONG_S  t0, sp, PT_ESTAT

    csrrd t0, LOONGARCH_CSR_BADV
    LONG_S  t0, sp, PT_BVADDR

    csrrd t0, LOONGARCH_CSR_TLBRERA
    LONG_S  t0, sp, PT_TLBERA

    csrrd t0, LOONGARCH_CSR_TLBRPRMD
    LONG_S  t0, sp, PT_TLBRPRMD

    cfi_st  $r6, PT_R6, \docfi
    cfi_st  ra, PT_R1, \docfi
    cfi_st  $r7, PT_R7, \docfi
    cfi_st  $r8, PT_R8, \docfi
    cfi_st  $r9, PT_R9, \docfi
    cfi_st  $r10, PT_R10, \docfi
    cfi_st  $r11, PT_R11, \docfi
    csrrd ra, LOONGARCH_CSR_ERA
    LONG_S  ra, sp, PT_ERA
    .if \docfi
    .cfi_rel_offset 1, PT_ERA
    .endif
    cfi_st  tp, PT_R2, \docfi
    cfi_st  fp, PT_R22, \docfi
    cfi_st  $r21, PT_R21, \docfi

    UNWIND_HINT_REGS
    .endm

    .macro  SAVE_ALL docfi=0
    SAVE_SOME \docfi
    LOAD_TEMP_FROM_KSCRATCH
    SAVE_TEMP \docfi
    SAVE_STATIC \docfi
    .endm

    .macro  RESTORE_TEMP docfi=0
    cfi_ld  $r12, PT_R12, \docfi
    cfi_ld  $r13, PT_R13, \docfi
    cfi_ld  $r14, PT_R14, \docfi
    cfi_ld  $r15, PT_R15, \docfi
    cfi_ld  $r16, PT_R16, \docfi
    cfi_ld  $r17, PT_R17, \docfi
    cfi_ld  $r18, PT_R18, \docfi
    cfi_ld  $r19, PT_R19, \docfi
    cfi_ld  $r20, PT_R20, \docfi
    .endm

    .macro  RESTORE_STATIC docfi=0
    cfi_ld  $r23, PT_R23, \docfi
    cfi_ld  $r24, PT_R24, \docfi
    cfi_ld  $r25, PT_R25, \docfi
    cfi_ld  $r26, PT_R26, \docfi
    cfi_ld  $r27, PT_R27, \docfi
    cfi_ld  $r28, PT_R28, \docfi
    cfi_ld  $r29, PT_R29, \docfi
    cfi_ld  $r30, PT_R30, \docfi
    cfi_ld  $r31, PT_R31, \docfi
    .endm

    .macro  RESTORE_SP docfi=0
    cfi_ld  sp, PT_R3, \docfi
    .endm

    .macro  RESTORE_SOME docfi=0
    cfi_ld  $r21, PT_R21, \docfi
    cfi_ld  $r1, PT_R1, \docfi
    cfi_ld  $r11, PT_R11, \docfi
    cfi_ld  $r10, PT_R10, \docfi
    cfi_ld  $r9, PT_R9, \docfi
    cfi_ld  $r8, PT_R8, \docfi
    cfi_ld  $r7,  PT_R7, \docfi
    cfi_ld  $r6,  PT_R6, \docfi
    cfi_ld  $r5,  PT_R5, \docfi
    cfi_ld  $r4,  PT_R4, \docfi
    cfi_ld  tp, PT_R2, \docfi
    cfi_ld  fp, PT_R22, \docfi
    .endm

    .macro  RESTORE_SP_AND_RET docfi=0
    RESTORE_SP \docfi
    UNWIND_HINT
    ertn
    .endm

    .macro  RESTORE_ALL docfi=0
    RESTORE_TEMP \docfi
    RESTORE_STATIC \docfi
    RESTORE_SOME \docfi
    RESTORE_SP \docfi
    .endm
#endif /* _ASM_STACKFRAME_H */
