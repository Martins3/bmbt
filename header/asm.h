#ifndef ASM_H_LOAQNS21
#define ASM_H_LOAQNS21
/*
 * ENTRY - declare nested routine entry point
 */
#define ENTRY(symbol)			\
		.globl	symbol;				\
		.align	2;				\
		.type	symbol, @function;		\
symbol:							\
		.cfi_startproc;

/*
 * END - mark end of function
 */
#define END(function)					\
		.cfi_endproc;				\
		.size	function, .-function
#endif /* end of include guard: ASM_H_LOAQNS21 */
