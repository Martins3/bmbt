#ifndef UNWIND_HINTS_H_PJXUIKZC
#define UNWIND_HINTS_H_PJXUIKZC
#ifdef __ASSEMBLY__

/*
 * In asm, there are two kinds of code: normal C-type callable functions and
 * the rest.  The normal callable functions can be called by other code, and
 * don't do anything unusual with the stack.  Such normal callable functions
 * are annotated with the ENTRY/ENDPROC macros.  Most asm code falls in this
 * category.  In this case, no special debugging annotations are needed because
 * objtool can automatically generate the ORC data for the ORC unwinder to read
 * at runtime.
 *
 * Anything which doesn't fall into the above category, such as syscall and
 * interrupt handlers, tends to not be called directly by other functions, and
 * often does unusual non-C-function-type things with the stack pointer.  Such
 * code needs to be annotated such that objtool can understand it.  The
 * following CFI hint macros are for this type of code.
 *
 * These macros provide hints to objtool about the state of the stack at each
 * instruction.  Objtool starts from the hints and follows the code flow,
 * making automatic CFI adjustments when it sees pushes and pops, filling out
 * the debuginfo as necessary.  It will also warn if it sees any
 * inconsistencies.
 */
.macro UNWIND_HINT sp_reg=ORC_REG_SP sp_offset=0 type=ORC_TYPE_CALL end=0
#ifdef CONFIG_STACK_VALIDATION
.Lunwind_hint_ip_\@:
	.pushsection .discard.unwind_hints
		/* struct unwind_hint */
		.long .Lunwind_hint_ip_\@ - .
		.short \sp_offset
		.byte \sp_reg
		.byte \type
		.byte \end
		.balign 4
	.popsection
#endif
.endm

.macro UNWIND_HINT_EMPTY
	UNWIND_HINT sp_reg=ORC_REG_UNDEFINED end=1
.endm

.macro UNWIND_HINT_REGS base=ORC_REG_SP offset=0
	UNWIND_HINT sp_reg=\base sp_offset=\offset type=ORC_TYPE_REGS
.endm

.macro UNWIND_HINT_FUNC offset=0
	UNWIND_HINT sp_offset=\offset
.endm

.macro NOT_SIBLING_CALL_HINT
876:	.pushsection .discard.not_sibling_call
	.long 876b - .
	.popsection
.endm
#endif /* !__ASSEMBLY__ */

#endif /* end of include guard: UNWIND_HINTS_H_PJXUIKZC */
