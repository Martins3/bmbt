#include <linux/linkage.h>
#include <asm/unwind_hints.h>
#include <asm/stackframe.h>

	.text
	.align	5
SYM_CODE_START(ret_from_exception)
	UNWIND_HINT_REGS
	b	__ret_from_irq
SYM_CODE_END(ret_from_exception)

SYM_CODE_START(ret_from_irq)
	UNWIND_HINT_REGS
	b	__ret_from_irq
SYM_CODE_END(ret_from_irq)

SYM_CODE_START(__ret_from_irq)
	UNWIND_HINT_REGS
	b	restore_all
restore_all:				# restore full frame
	RESTORE_TEMP
	RESTORE_STATIC
restore_partial:		# restore partial frame
	RESTORE_SOME
	RESTORE_SP_AND_RET
SYM_CODE_END(__ret_from_irq)
