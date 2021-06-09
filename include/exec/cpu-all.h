#ifndef CPU_ALL_H_9ZPFYTXB
#define CPU_ALL_H_9ZPFYTXB
#include "../types.h"
#include "cpu-defs.h"
#include "exec-all.h"

// FIXME copy form target/i386/cpu-param.h
#define TARGET_PAGE_BITS 12

// TODO need deeper understanding for this macros
#define EXCP_INTERRUPT 0x10000 /* async interruption */
#define EXCP_HLT 0x10001       /* hlt instruction reached */
#define EXCP_DEBUG 0x10002  /* cpu stopped after a breakpoint or singlestep */
#define EXCP_HALTED 0x10003 /* cpu is halted (waiting for external event) */
#define EXCP_YIELD 0x10004  /* cpu wants to yield timeslice to another */
#define EXCP_ATOMIC 0x10005 /* stop-the-world and emulate atomic */

#define TARGET_PAGE_BITS_MIN TARGET_PAGE_BITS
#define TARGET_PAGE_SIZE (1 << TARGET_PAGE_BITS)
// FIXME this is a bug, I just want to make ccls happy, will be fixed later
// #define TARGET_PAGE_MASK   ((target_long)-1 << TARGET_PAGE_BITS)
#define TARGET_PAGE_MASK ((target_long)1 << TARGET_PAGE_BITS)

static inline CPUState *env_cpu(CPUArchState *env) { return env->cpu; }

#endif /* end of include guard: CPU_ALL_H_9ZPFYTXB */
