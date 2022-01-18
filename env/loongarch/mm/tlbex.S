#include <linux/linkage.h>
SYM_FUNC_START(handle_tlb_refill)
	b handle_reserved
	nop
SYM_FUNC_END(handle_tlb_refill)
