#include "../header/loongarchregs.h"
#include "../header/regdef.h"
#include "../header/config.h"
#include "../header/asm.h"
.section .text

ENTRY(kernel_entry)
	la t0, __bss_start		# clear .bss
	st.d		zero, t0, 0
	la.abs t1, __bss_stop - LONGSIZE
1:
	addi.d		t0, t0, LONGSIZE
	st.d		zero, t0, 0
	bne		t0, t1, 1b

	la t0, fw_arg0
	st.d		a0, t0, 0		# firmware arguments
	la t0, fw_arg1
	st.d		a1, t0, 0
	la t0, fw_arg2
	st.d		a2, t0, 0
	la t0, fw_arg3
	st.d		a3, t0, 0

	# Config direct window and set PG
	# 0x8000 xxxx xxxx xxxx
	dli	t0, CSR_DMW0_INIT	# UC, PLV0
	csrwr	t0, LOONGARCH_CSR_DMWIN0
	# 0x9000 xxxx xxxx xxxx
	dli	t0, CSR_DMW1_INIT	# CA, PLV0
	csrwr	t0, LOONGARCH_CSR_DMWIN1
	# Enable PG
	li	t0, 0xb0	# PLV=0, IE=0, PG
	csrwr	t0, LOONGARCH_CSR_CRMD

  // TODO define a array for tp and sp

  b start_kernel
END(kernel_entry)
