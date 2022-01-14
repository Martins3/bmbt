#include <asm/loongarchregs.h>
#include <asm/regdef.h>
#include <asm/config.h>
#include <asm/asm.h>
#include <stack.h>
.section .text

ENTRY(start_entry)
	la t0, __bss_start		# clear .bss
	st.d		zero, t0, 0
	la.abs t1, __bss_stop - LONGSIZE
1:
	addi.d		t0, t0, LONGSIZE
	st.d		zero, t0, 0
	bne		t0, t1, 1b

	la t0, fw_arg0
	st.d		a0, t0, 0	# firmware arguments
	la t0, fw_arg1
	st.d		a1, t0, 0
	la t0, fw_arg2
	st.d		a2, t0, 0
	la t0, fw_arg3
	st.d		a3, t0, 0

	# Config direct window and set PG
	# 0x8000 xxxx xxxx xxxx
	li.d	t0, CSR_DMW0_INIT	# UC, PLV0
	csrwr	t0, LOONGARCH_CSR_DMWIN0
	# 0x9000 xxxx xxxx xxxx
	li.d	t0, CSR_DMW1_INIT	# CA, PLV0
	csrwr	t0, LOONGARCH_CSR_DMWIN1
	// @todo enable PG?
	# Enable PG
	li.w	t0, 0xb0		# PLV=0, IE=0, PG
	csrwr	t0, LOONGARCH_CSR_CRMD
	li.w    t0, 0x15                # enable FP SXE ASXE BTE
	csrwr   t0, LOONGARCH_CSR_EUEN

	# init sp
	la	t0, kernel_stack
	li.d		sp, _THREAD_SIZE
	add.d		sp, sp, t0
	addi.d		sp, sp, -16	# init stack pointer
	# init fp
	li.d fp, 0

	b start_kernel
END(start_entry)
