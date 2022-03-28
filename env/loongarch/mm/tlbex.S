#include <linux/linkage.h>
#include <asm/loongarchregs.h>
#include <asm/regdef.h>

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

// make t0 0x9000000000de4000
or      t0, zero, zero
lu12i.w t0, 0xde4000>>12
lu52i.d t0, t0, 0x9000000000000000>>52

csrrd   t1, LOONGARCH_CSR_BADV
st.d    t1, t0, (33*8)
csrrd   t1, LOONGARCH_CSR_ERA
st.d    t1, t0, (32*8)

lu12i.w t1, 0xde8000>>12
lu52i.d t1, t1, 0x9000000000000000>>52
csrwr   t1, LOONGARCH_CSR_ERA

csrrd   t1, LOONGARCH_CSR_KS1
csrrd   t0, LOONGARCH_CSR_KS0
ertn
SYM_FUNC_END(handle_tlb_load)

SYM_FUNC_START(handle_tlb_store)
csrwr   t0, LOONGARCH_CSR_KS0
csrwr   t1, LOONGARCH_CSR_KS1

// make t0 0x9000000000de4000
or      t0, zero, zero
lu12i.w t0, 0xde4000>>12
lu52i.d t0, t0, 0x9000000000000000>>52

csrrd   t1, LOONGARCH_CSR_BADV
st.d    t1, t0, (33*8)
csrrd   t1, LOONGARCH_CSR_ERA
st.d    t1, t0, (32*8)

lu12i.w t1, 0xde8000>>12
lu52i.d t1, t1, 0x9000000000000000>>52
csrwr   t1, LOONGARCH_CSR_ERA

csrrd   t1, LOONGARCH_CSR_KS1
csrrd   t0, LOONGARCH_CSR_KS0
ertn
SYM_FUNC_END(handle_tlb_store)

SYM_FUNC_START(handle_tlb_modify)
csrwr   t0, LOONGARCH_CSR_KS0
csrwr   t1, LOONGARCH_CSR_KS1

// make t0 0x9000000000de4000
or      t0, zero, zero
lu12i.w t0, 0xde4000>>12
lu52i.d t0, t0, 0x9000000000000000>>52

csrrd   t1, LOONGARCH_CSR_BADV
st.d    t1, t0, (33*8)
csrrd   t1, LOONGARCH_CSR_ERA
st.d    t1, t0, (32*8)

lu12i.w t1, 0xde8000>>12
lu52i.d t1, t1, 0x9000000000000000>>52
csrwr   t1, LOONGARCH_CSR_ERA

csrrd   t1, LOONGARCH_CSR_KS1
csrrd   t0, LOONGARCH_CSR_KS0

ertn
SYM_FUNC_END(handle_tlb_modify)
