#include <linux/linkage.h>
#include <asm/unwind_hints.h>
#include <asm/stackframe.h>

SYM_FUNC_START(except_vec_cex)
	b	handle_reserved
	nop
SYM_FUNC_END(except_vec_cex)

	.macro	__BUILD_HANDLER exception handler clear verbose ext
	.align	5
	SYM_FUNC_START(handle_\exception)
	csrwr	t0, LOONGARCH_CSR_KS0
	csrwr	t1, LOONGARCH_CSR_KS1
	SAVE_ALL
	move	a0, sp
	li.d	a1, 0
	la.abs	t0, do_\handler
	jirl    ra, t0, 0
	la.abs	t0, ret_from_exception
	jirl    zero, t0, 0
	SYM_FUNC_END(handle_\exception)
	.endm

	.macro	BUILD_HANDLER exception handler clear verbose
	__BUILD_HANDLER \exception \handler \clear \verbose _int
	.endm

	BUILD_HANDLER reserved reserved sti verbose	/* others */

	.align	5	/* 32 byte rollback region */
SYM_FUNC_START(__cpu_wait)
	/* start of rollback region */
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	idle	0
	/* end of rollback region (the region size must be power of two) */
1:
	jirl	zero, ra, 0
SYM_FUNC_END(__cpu_wait)

/*
 * Common Vectored Interrupt code
 * Complete the register saves and invoke the handler which is passed in $v0
 */
SYM_FUNC_START(except_vec_vi_handler)
	UNWIND_HINT_REGS
	la      t1, __cpu_wait
	ld.d    t0, sp, PT_ERA
	/* 32 byte rollback region */
	ori	t0, t0, 0x1f
	xori	t0, t0, 0x1f
	bne	t0, t1, 1f
	st.d    t0, sp, PT_ERA
1:
	LOAD_TEMP_FROM_KSCRATCH
	SAVE_TEMP
	SAVE_STATIC
	CLI
2:
	/* v0 is equal to a0
	 * v0: irq number for do_vi
	 */
	la		t0, do_vi
	jirl		ra, t0, 0

	la	t0, ret_from_irq
	jirl    zero, t0, 0
SYM_FUNC_END(except_vec_vi_handler)


/*
 * Macro helper for vectored interrupt handler.
 */
	.macro	BUILD_VI_HANDLER num
	.align	5
SYM_FUNC_START(handle_vi_\num)
	csrwr	t0, LOONGARCH_CSR_KS0
	csrwr	t1, LOONGARCH_CSR_KS1
	SAVE_SOME #docfi=1
	addi.d	v0, zero, \num
	la.abs	v1, except_vec_vi_handler
	jirl	zero, v1, 0
SYM_FUNC_END(handle_vi_\num)
	.endm

	BUILD_VI_HANDLER 0
	BUILD_VI_HANDLER 1
	BUILD_VI_HANDLER 2
	BUILD_VI_HANDLER 3
	BUILD_VI_HANDLER 4
	BUILD_VI_HANDLER 5
	BUILD_VI_HANDLER 6
	BUILD_VI_HANDLER 7
	BUILD_VI_HANDLER 8
	BUILD_VI_HANDLER 9
	BUILD_VI_HANDLER 10
	BUILD_VI_HANDLER 11
	BUILD_VI_HANDLER 12
	BUILD_VI_HANDLER 13

	.section .data, "aw"
	.align	3
        .globl  vector_table
vector_table:
	PTR	handle_vi_0
	PTR	handle_vi_1
	PTR	handle_vi_2
	PTR	handle_vi_3
	PTR	handle_vi_4
	PTR	handle_vi_5
	PTR	handle_vi_6
	PTR	handle_vi_7
	PTR	handle_vi_8
	PTR	handle_vi_9
	PTR	handle_vi_10
	PTR	handle_vi_11
	PTR	handle_vi_12
	PTR	handle_vi_13
