#ifndef LINKAGE_H_1RQOMEAD
#define LINKAGE_H_1RQOMEAD
#include <asm/linkage.h>

/* Some toolchains use other characters (e.g. '`') to mark new line in macro */
#ifndef ASM_NL
#define ASM_NL ;
#endif

/* SYM_A_* -- align the symbol? */
#define SYM_A_ALIGN ALIGN
#define SYM_A_NONE /* nothing */

/* SYM_T_NONE -- type used by assembler to mark entries of unknown type */
#ifndef SYM_T_NONE
#define SYM_T_NONE STT_NOTYPE
#endif

/* SYM_L_* -- linkage of symbols */
#define SYM_L_GLOBAL(name) .globl name
#define SYM_L_WEAK(name) .weak name
#define SYM_L_LOCAL(name) /* nothing */

/* SYM_ENTRY -- use only if you have to for non-paired symbols */
#ifndef SYM_ENTRY
#define SYM_ENTRY(name, linkage, align...)                                     \
  linkage(name) ASM_NL align ASM_NL name:
#endif

/* SYM_START -- use only if you have to */
#ifndef SYM_START
#define SYM_START(name, linkage, align...) SYM_ENTRY(name, linkage, align)
#endif

/* SYM_END -- use only if you have to */
#ifndef SYM_END
#define SYM_END(name, sym_type) .type name sym_type ASM_NL.size name, .- name
#endif

/* SYM_CODE_START -- use for non-C (special) functions */
#ifndef SYM_CODE_START
#define SYM_CODE_START(name) SYM_START(name, SYM_L_GLOBAL, SYM_A_ALIGN)
#endif

/* SYM_CODE_END -- the end of SYM_CODE_START_LOCAL, SYM_CODE_START, ... */
#ifndef SYM_CODE_END
#define SYM_CODE_END(name) SYM_END(name, SYM_T_NONE)
#endif

#endif /* end of include guard: LINKAGE_H_1RQOMEAD */
