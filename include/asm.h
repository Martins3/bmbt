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

#define PTR_ADD		dadd
#define PTR_ADDU	daddu
#define PTR_ADDI	daddi
#define PTR_ADDIU	daddiu
#define PTR_SUB		dsub
#define PTR_SUBU	dsubu
#define PTR_L		ld
#define PTR_S		sd
#define PTR_LA		dla
#define PTR_LI		dli
#define PTR_SLL		dsll
#define PTR_SLLV	dsllv
#define PTR_SRL		dsrl
#define PTR_SRLV	dsrlv
#define PTR_SRA		dsra
#define PTR_SRAV	dsrav

#define PTR_SCALESHIFT	3

#define PTR		.dword
#define PTRSIZE		8
#define PTRLOG		3
#endif /* end of include guard: ASM_H_LOAQNS21 */
