#include <linux/linkage.h>
#include <asm/loongarchregs.h>
#include <asm/regdef.h>

#include <asm/unwind_hints.h>
#include <asm/stackframe.h>

SYM_FUNC_START(handle_tlb_refill)
	csrwr	t0, LOONGARCH_CSR_KS0
	csrwr	t1, LOONGARCH_CSR_KS1

	// turn sp into physical address
	li.d	t0, 0xffffffff
	and	sp, sp, t0
	SAVE_ALL

	// recover sp
	li.d	t0, 0x9000000000000000
	or	sp, sp, t0

	// prepare arguments
	move	a0, sp
	li.d	a1, 1

	la.abs	t0, do_reserved
	ori	t0, t0,  1 // isTLBR
	csrwr	t0, LOONGARCH_CSR_TLBRERA

	ertn
SYM_FUNC_END(handle_tlb_refill)

/* doing nothing and skip the instruction
SYM_FUNC_START(handle_tlb_refill)
	csrwr	t0, LOONGARCH_CSR_TLBRSAVE
	csrrd	t0, LOONGARCH_CSR_TLBRERA
	addi.d  t0, t0, 4
	csrwr	t0, LOONGARCH_CSR_TLBRERA
	csrrd	t0, LOONGARCH_CSR_TLBRSAVE
	ertn
SYM_FUNC_END(handle_tlb_refill)
*/
