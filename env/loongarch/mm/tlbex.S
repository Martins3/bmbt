#include <linux/linkage.h>
#include <asm/loongarchregs.h>
#include <asm/regdef.h>
#include <accel/internal.h>
#include <asm/unwind_hints.h>
#include <asm/stackframe.h>

// SYM_FUNC_START(handle_tlb_refill)
//   csrwr t0, LOONGARCH_CSR_KS0
//   csrwr t1, LOONGARCH_CSR_KS1
//
//   // turn sp into physical address
//   li.d  t0, 0xffffffff
//   and sp, sp, t0
//   SAVE_ALL
//
//   // recover sp
//   li.d  t0, 0x9000000000000000
//   or  sp, sp, t0
//
//   // in order make backtrace intact
//   csrrd t0, LOONGARCH_CSR_TLBRERA
//   move ra, t0
//
//   // prepare arguments
//   move  a0, sp
//   li.d  a1, 1
//
//   la.abs  t0, do_reserved
//   ori t0, t0,  1 // isTLBR
//   csrwr t0, LOONGARCH_CSR_TLBRERA
//
//   ertn
// SYM_FUNC_END(handle_tlb_refill)

/* doing nothing and skip the instruction
SYM_FUNC_START(handle_tlb_refill)
  csrwr t0, LOONGARCH_CSR_TLBRSAVE
  csrrd t0, LOONGARCH_CSR_TLBRERA
  addi.d  t0, t0, 4
  csrwr t0, LOONGARCH_CSR_TLBRERA
  csrrd t0, LOONGARCH_CSR_TLBRSAVE
  ertn
SYM_FUNC_END(handle_tlb_refill)
*/

SYM_FUNC_START(handle_tlb_refill)
csrwr    t0, LOONGARCH_CSR_TLBRSAVE
csrwr    t1, LOONGARCH_CSR_KS0
csrwr    t2, LOONGARCH_CSR_KS1

// t0: bad vaddr
// t1: 0x100000000
csrrd    t0, LOONGARCH_CSR_TLBRBADV
or       t1, zero, zero
li.d	 t1, 0x100000000
bltu     t0, t1, addr_exp
// t1: 0x200000000
li.d     t1, 0x200000000
bgeu     t0, t1, addr_exp

mem_accel:
csrrd  t0, LOONGARCH_CSR_TLBREHI

// set TLBREHI.PS
srli.d t0, t0, 0x6
slli.d t0, t0, 0x6
ori    t1, t0, 0xc

csrwr  t1, LOONGARCH_CSR_TLBREHI
csrwr  zero, LOONGARCH_CSR_TLBRELO0
csrwr  zero, LOONGARCH_CSR_TLBRELO1
tlbfill

tlbr_restore:
csrrd    t0, LOONGARCH_CSR_TLBRSAVE
csrrd    t1, LOONGARCH_CSR_KS0
csrrd    t2, LOONGARCH_CSR_KS1

ertn

addr_exp:
// turn sp into physical address
li.d  t0, 0xffffffff
and sp, sp, t0
SAVE_ALL

// recover sp
li.d  t0, 0x9000000000000000
or  sp, sp, t0

// in order make backtrace intact
csrrd t0, LOONGARCH_CSR_TLBRERA
move ra, t0

// prepare arguments
move  a0, sp
li.d  a1, 1

la.abs  t0, do_reserved
ori t0, t0,  1 // isTLBR
csrwr t0, LOONGARCH_CSR_TLBRERA

ertn
SYM_FUNC_END(handle_tlb_refill)

SYM_FUNC_START(handle_tlb_load)
csrwr   t0, LOONGARCH_CSR_KS0
csrwr   t1, LOONGARCH_CSR_KS1

// make t0 0x9000000008c24000
or      t0, zero, zero
// lu12i.w t0, 0x8c24000>>12
// lu52i.d t0, t0, 0x9000000000000000>>52
li.d    t0, 0x90000000bfc40000

csrrd   t1, LOONGARCH_CSR_BADV
st.d    t1, t0, (BADV_INDEX*8)
csrrd   t1, LOONGARCH_CSR_ERA
st.d    t1, t0, (ERA_INDEX*8)

// lu12i.w t1, 0x8c28000>>12
// lu52i.d t1, t1, 0x9000000000000000>>52
li.d    t1, 0x90000000bfc44000
csrwr   t1, LOONGARCH_CSR_ERA

csrrd   t1, LOONGARCH_CSR_KS1
csrrd   t0, LOONGARCH_CSR_KS0
ertn
SYM_FUNC_END(handle_tlb_load)

SYM_FUNC_START(handle_tlb_store)
csrwr   t0, LOONGARCH_CSR_KS0
csrwr   t1, LOONGARCH_CSR_KS1

// make t0 0x9000000008c24000
or      t0, zero, zero
// lu12i.w t0, 0x8c24000>>12
// lu52i.d t0, t0, 0x9000000000000000>>52
li.d    t0, 0x90000000bfc40000

csrrd   t1, LOONGARCH_CSR_BADV
st.d    t1, t0, (BADV_INDEX*8)
csrrd   t1, LOONGARCH_CSR_ERA
st.d    t1, t0, (ERA_INDEX*8)

// lu12i.w t1, 0x8c28000>>12
// lu52i.d t1, t1, 0x9000000000000000>>52
li.d    t1, 0x90000000bfc44000
csrwr   t1, LOONGARCH_CSR_ERA

csrrd   t1, LOONGARCH_CSR_KS1
csrrd   t0, LOONGARCH_CSR_KS0
ertn
SYM_FUNC_END(handle_tlb_store)

SYM_FUNC_START(handle_tlb_modify)
csrwr   t0, LOONGARCH_CSR_KS0
csrwr   t1, LOONGARCH_CSR_KS1

// make t0 0x90000000008c24000
or      t0, zero, zero
// lu12i.w t0, 0x8c24000>>12
// lu52i.d t0, t0, 0x9000000000000000>>52
li.d    t0, 0x90000000bfc40000

csrrd   t1, LOONGARCH_CSR_BADV
st.d    t1, t0, (BADV_INDEX*8)
csrrd   t1, LOONGARCH_CSR_ERA
st.d    t1, t0, (ERA_INDEX*8)

// lu12i.w t1, 0x8c28000>>12
// lu52i.d t1, t1, 0x9000000000000000>>52
li.d    t1, 0x90000000bfc44000
csrwr   t1, LOONGARCH_CSR_ERA

csrrd   t1, LOONGARCH_CSR_KS1
csrrd   t0, LOONGARCH_CSR_KS0

ertn
SYM_FUNC_END(handle_tlb_modify)

SYM_FUNC_START(handle_bp)
csrwr   t0, LOONGARCH_CSR_KS0
csrwr   t1, LOONGARCH_CSR_KS1
csrwr   t2, LOONGARCH_CSR_KS2

or      t0, zero, zero
// lu12i.w t0, 0x8c24000>>12
// lu52i.d t0, t0, 0x9000000000000000>>52
li.d    t0, 0x90000000bfc40000

ldptr.d t1, t0, (TRAP_CODE_INDEX*8)

move    t2, t1
xori    t2, t2, 0x3
beq     t2, zero, restore_regs

UNEXPECTED_TRAP

restore_regs:
ld.d    ra, t0, 8*1
ld.d    tp, t0, 8*2
ld.d    sp, t0, 8*3
ld.d    a0, t0, 8*4
ld.d    a1, t0, 8*5
ld.d    a2, t0, 8*6
ld.d    a3, t0, 8*7
ld.d    a4, t0, 8*8
ld.d    a5, t0, 8*9
ld.d    a6, t0, 8*10
ld.d    a7, t0, 8*11
ld.d    t1, t0, 8*13
ld.d    t2, t0, 8*14
ld.d    t3, t0, 8*15
ld.d    t4, t0, 8*16
ld.d    t5, t0, 8*17
ld.d    t6, t0, 8*18
ld.d    t7, t0, 8*19
ld.d    t8, t0, 8*20
ld.d  $r21, t0, 8*21
ld.d    fp, t0, 8*22
ld.d    s0, t0, 8*23
ld.d    s1, t0, 8*24
ld.d    s2, t0, 8*25
ld.d    s3, t0, 8*26
ld.d    s4, t0, 8*27
ld.d    s5, t0, 8*28
ld.d    s6, t0, 8*29
ld.d    s7, t0, 8*30
ld.d    s8, t0, 8*31

csrwr   t1, LOONGARCH_CSR_KS0
ld.d    t1, t0, 8*32
csrwr   t1, LOONGARCH_CSR_EPC

ld.d    t0, t0, 8*12
csrrd   t1, LOONGARCH_CSR_KS0

ertn
SYM_FUNC_END(handle_bp)
