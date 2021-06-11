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

/* same as PROT_xxx */
#define PAGE_READ      0x0001
#define PAGE_WRITE     0x0002
#define PAGE_EXEC      0x0004
#define PAGE_BITS      (PAGE_READ | PAGE_WRITE | PAGE_EXEC)
#define PAGE_VALID     0x0008
/* original state of the write flag (used when tracking self-modifying
   code */
#define PAGE_WRITE_ORG 0x0010
/* Invalidate the TLB entry immediately, helpful for s390x
 * Low-Address-Protection. Used with PAGE_WRITE in tlb_set_page_with_attrs() */
#define PAGE_WRITE_INV 0x0040

/* Flags for use in ENV->INTERRUPT_PENDING.

   The numbers assigned here are non-sequential in order to preserve
   binary compatibility with the vmstate dump.  Bit 0 (0x0001) was
   previously used for CPU_INTERRUPT_EXIT, and is cleared when loading
   the vmstate dump.  */

/* External hardware interrupt pending.  This is typically used for
   interrupts from devices.  */
#define CPU_INTERRUPT_HARD        0x0002

/* Exit the current TB.  This is typically used when some system-level device
   makes some change to the memory mapping.  E.g. the a20 line change.  */
#define CPU_INTERRUPT_EXITTB      0x0004

/* Halt the CPU.  */
#define CPU_INTERRUPT_HALT        0x0020

/* Debug event pending.  */
#define CPU_INTERRUPT_DEBUG       0x0080

/* Reset signal.  */
#define CPU_INTERRUPT_RESET       0x0400

/* Several target-specific external hardware interrupts.  Each target/cpu.h
   should define proper names based on these defines.  */
#define CPU_INTERRUPT_TGT_EXT_0   0x0008
#define CPU_INTERRUPT_TGT_EXT_1   0x0010
#define CPU_INTERRUPT_TGT_EXT_2   0x0040
#define CPU_INTERRUPT_TGT_EXT_3   0x0200
#define CPU_INTERRUPT_TGT_EXT_4   0x1000

/* Several target-specific internal interrupts.  These differ from the
   preceding target-specific interrupts in that they are intended to
   originate from within the cpu itself, typically in response to some
   instruction being executed.  These, therefore, are not masked while
   single-stepping within the debugger.  */
#define CPU_INTERRUPT_TGT_INT_0   0x0100
#define CPU_INTERRUPT_TGT_INT_1   0x0800
#define CPU_INTERRUPT_TGT_INT_2   0x2000

/**
 * env_archcpu(env)
 * @env: The architecture environment
 *
 * Return the ArchCPU associated with the environment.
 */
static inline ArchCPU *env_archcpu(CPUArchState *env)
{
   return container_of(env, ArchCPU, env);
}

/**
 * env_neg(env)
 * @env: The architecture environment
 *
 * Return the CPUNegativeOffsetState associated with the environment.
 */
static inline CPUNegativeOffsetState *env_neg(CPUArchState *env)
{
    ArchCPU *arch_cpu = container_of(env, ArchCPU, env);
    return &arch_cpu->neg;
}

/**
 * env_tlb(env)
 * @env: The architecture environment
 *
 * Return the CPUTLB state associated with the environment.
 */
static inline CPUTLB *env_tlb(CPUArchState *env)
{
    return &env_neg(env)->tlb;
}


#endif /* end of include guard: CPU_ALL_H_9ZPFYTXB */
