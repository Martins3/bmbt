#ifndef CPU_ALL_H_9ZPFYTXB
#define CPU_ALL_H_9ZPFYTXB
#include "cpu-defs.h"
#include "exec-all.h"
#include "memory.h"

#define EXCP_INTERRUPT 0x10000 /* async interruption */
#define EXCP_HLT 0x10001       /* hlt instruction reached */
#define EXCP_DEBUG 0x10002  /* cpu stopped after a breakpoint or singlestep */
#define EXCP_HALTED 0x10003 /* cpu is halted (waiting for external event) */
#define EXCP_YIELD 0x10004  /* cpu wants to yield timeslice to another */
#define EXCP_ATOMIC 0x10005 /* stop-the-world and emulate atomic */

#define TARGET_PAGE_BITS_MIN TARGET_PAGE_BITS
#define TARGET_PAGE_SIZE (1 << TARGET_PAGE_BITS)

// To make ccls happy, change the syntax.
// If we want to support 64bit, it may doesn't work
// https://stackoverflow.com/questions/40508958/shifting-a-negative-signed-value-is-undefined
// #define TARGET_PAGE_MASK   ((target_long)-1 << TARGET_PAGE_BITS)
#define TARGET_PAGE_MASK ((-1u) << TARGET_PAGE_BITS)

#define TARGET_PAGE_ALIGN(addr) ROUND_UP((addr), TARGET_PAGE_SIZE)

/* same as PROT_xxx */
#define PAGE_READ 0x0001
#define PAGE_WRITE 0x0002
#define PAGE_EXEC 0x0004
#define PAGE_BITS (PAGE_READ | PAGE_WRITE | PAGE_EXEC)
#define PAGE_VALID 0x0008
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
#define CPU_INTERRUPT_HARD 0x0002

/* Exit the current TB.  This is typically used when some system-level device
   makes some change to the memory mapping.  E.g. the a20 line change.  */
#define CPU_INTERRUPT_EXITTB 0x0004

/* Halt the CPU.  */
#define CPU_INTERRUPT_HALT 0x0020

/* Debug event pending.  */
#define CPU_INTERRUPT_DEBUG 0x0080

/* Reset signal.  */
#define CPU_INTERRUPT_RESET 0x0400

/* Several target-specific external hardware interrupts.  Each target/cpu.h
   should define proper names based on these defines.  */
#define CPU_INTERRUPT_TGT_EXT_0 0x0008
#define CPU_INTERRUPT_TGT_EXT_1 0x0010
#define CPU_INTERRUPT_TGT_EXT_2 0x0040
#define CPU_INTERRUPT_TGT_EXT_3 0x0200
#define CPU_INTERRUPT_TGT_EXT_4 0x1000

/* Several target-specific internal interrupts.  These differ from the
   preceding target-specific interrupts in that they are intended to
   originate from within the cpu itself, typically in response to some
   instruction being executed.  These, therefore, are not masked while
   single-stepping within the debugger.  */
#define CPU_INTERRUPT_TGT_INT_0 0x0100
#define CPU_INTERRUPT_TGT_INT_1 0x0800
#define CPU_INTERRUPT_TGT_INT_2 0x2000

/**
 * env_archcpu(env)
 * @env: The architecture environment
 *
 * Return the ArchCPU associated with the environment.
 */
static inline ArchCPU *env_archcpu(CPUArchState *env) {
  return container_of(env, ArchCPU, env);
}

/**
 * env_cpu(env)
 * @env: The architecture environment
 *
 * Return the CPUState associated with the environment.
 */
static inline CPUState *env_cpu(CPUArchState *env) {
  return &env_archcpu(env)->parent_obj;
}

/**
 * env_neg(env)
 * @env: The architecture environment
 *
 * Return the CPUNegativeOffsetState associated with the environment.
 */
static inline CPUNegativeOffsetState *env_neg(CPUArchState *env) {
  ArchCPU *arch_cpu = container_of(env, ArchCPU, env);
  return &arch_cpu->neg;
}

/**
 * env_tlb(env)
 * @env: The architecture environment
 *
 * Return the CPUTLB state associated with the environment.
 */
static inline CPUTLB *env_tlb(CPUArchState *env) { return &env_neg(env)->tlb; }

// bmbt version of include/exec/memory_ldst.inc.h
extern uint32_t address_space_lduw(AddressSpace *as, hwaddr addr,
                                   MemTxAttrs attrs, MemTxResult *result);
extern uint32_t address_space_ldl(AddressSpace *as, hwaddr addr,
                                  MemTxAttrs attrs, MemTxResult *result);
extern uint64_t address_space_ldq(AddressSpace *as, hwaddr addr,
                                  MemTxAttrs attrs, MemTxResult *result);
extern void address_space_stl_notdirty(AddressSpace *as, hwaddr addr,
                                       uint32_t val, MemTxAttrs attrs,
                                       MemTxResult *result);
extern void address_space_stw(AddressSpace *as, hwaddr addr, uint32_t val,
                              MemTxAttrs attrs, MemTxResult *result);
extern void address_space_stl(AddressSpace *as, hwaddr addr, uint32_t val,
                              MemTxAttrs attrs, MemTxResult *result);
extern void address_space_stl_le(AddressSpace *as, hwaddr addr, uint32_t val,
                                 MemTxAttrs attrs, MemTxResult *result);
extern void address_space_stq(AddressSpace *as, hwaddr addr, uint64_t val,
                              MemTxAttrs attrs, MemTxResult *result);
extern uint32_t address_space_ldub(AddressSpace *as, hwaddr addr,
                                   MemTxAttrs attrs, MemTxResult *result);

/*
 * Above address_space_* prototypes above are autogenerated, why this one copied
 * manually ?
 *
 * Actually they are all generated by include/exec/memory_ldst.inc.h
 * address_space_stb is TARGET_ENDIANNESS version, other TARGET_ENDIANNESS
 * version used by devices simulation, becase we almost don't need device
 * simulation anymore, so only address_space_stb still exist
 */
extern void address_space_stb(AddressSpace *as, hwaddr addr, uint32_t val,
                              MemTxAttrs attrs, MemTxResult *result);

static inline void stl_le_phys(AddressSpace *as, hwaddr addr, uint32_t val) {
  address_space_stl_le(as, addr, val, MEMTXATTRS_UNSPECIFIED, NULL);
}

#if !defined(CONFIG_USER_ONLY)

/*
 * Flags stored in the low bits of the TLB virtual address.
 * These are defined so that fast path ram access is all zeros.
 * The flags all must be between TARGET_PAGE_BITS and
 * maximum address alignment bit.
 *
 * Use TARGET_PAGE_BITS_MIN so that these bits are constant
 * when TARGET_PAGE_BITS_VARY is in effect.
 */
/* Zero if TLB entry is valid.  */
#define TLB_INVALID_MASK (1 << (TARGET_PAGE_BITS_MIN - 1))
/* Set if TLB entry references a clean RAM page.  The iotlb entry will
   contain the page physical address.  */
#define TLB_NOTDIRTY (1 << (TARGET_PAGE_BITS_MIN - 2))
/* Set if TLB entry is an IO callback.  */
#define TLB_MMIO (1 << (TARGET_PAGE_BITS_MIN - 3))
/* Set if TLB entry contains a watchpoint.  */
#define TLB_WATCHPOINT (1 << (TARGET_PAGE_BITS_MIN - 4))
/* Set if TLB entry requires byte swap.  */
#define TLB_BSWAP (1 << (TARGET_PAGE_BITS_MIN - 5))
/* Set if TLB entry writes ignored.  */
#define TLB_DISCARD_WRITE (1 << (TARGET_PAGE_BITS_MIN - 6))

/* Use this mask to check interception with an alignment mask
 * in a TCG backend.
 */
#define TLB_FLAGS_MASK                                                         \
  (TLB_INVALID_MASK | TLB_NOTDIRTY | TLB_MMIO | TLB_WATCHPOINT | TLB_BSWAP |   \
   TLB_DISCARD_WRITE)

/**
 * tlb_hit_page: return true if page aligned @addr is a hit against the
 * TLB entry @tlb_addr
 *
 * @addr: virtual address to test (must be page aligned)
 * @tlb_addr: TLB entry address (a CPUTLBEntry addr_read/write/code value)
 */
static inline bool tlb_hit_page(target_ulong tlb_addr, target_ulong addr) {
  assert(TARGET_PAGE_MASK == 0xfffff000LL);
  return addr == (tlb_addr & (TARGET_PAGE_MASK | TLB_INVALID_MASK));
}

/**
 * tlb_hit: return true if @addr is a hit against the TLB entry @tlb_addr
 *
 * @addr: virtual address to test (need not be page aligned)
 * @tlb_addr: TLB entry address (a CPUTLBEntry addr_read/write/code value)
 */
static inline bool tlb_hit(target_ulong tlb_addr, target_ulong addr) {
  assert(TARGET_PAGE_MASK == 0xfffff000LL);
  return tlb_hit_page(tlb_addr, addr & TARGET_PAGE_MASK);
}
#endif

/**
 * cpu_neg(cpu)
 * @cpu: The generic CPUState
 *
 * Return the CPUNegativeOffsetState associated with the cpu.
 */
static inline CPUNegativeOffsetState *cpu_neg(CPUState *cpu) {
  ArchCPU *arch_cpu = container_of(cpu, ArchCPU, parent_obj);
  return &arch_cpu->neg;
}

int cpu_exec(CPUState *cpu);
void cpu_exec_step_atomic(CPUState *cpu);

/* The set of all bits that should be masked when single-stepping.  */
#define CPU_INTERRUPT_SSTEP_MASK                                               \
  (CPU_INTERRUPT_HARD | CPU_INTERRUPT_TGT_EXT_0 | CPU_INTERRUPT_TGT_EXT_1 |    \
   CPU_INTERRUPT_TGT_EXT_2 | CPU_INTERRUPT_TGT_EXT_3 |                         \
   CPU_INTERRUPT_TGT_EXT_4)

/* Target-endianness CPU memory access functions. These fit into the
 * {ld,st}{type}{sign}{size}{endian}_p naming scheme described in bswap.h.
 */
#if defined(TARGET_WORDS_BIGENDIAN)
#define lduw_p(p) lduw_be_p(p)
#define ldsw_p(p) ldsw_be_p(p)
#define ldl_p(p) ldl_be_p(p)
#define ldq_p(p) ldq_be_p(p)
#define ldfl_p(p) ldfl_be_p(p)
#define ldfq_p(p) ldfq_be_p(p)
#define stw_p(p, v) stw_be_p(p, v)
#define stl_p(p, v) stl_be_p(p, v)
#define stq_p(p, v) stq_be_p(p, v)
#define stfl_p(p, v) stfl_be_p(p, v)
#define stfq_p(p, v) stfq_be_p(p, v)
#define ldn_p(p, sz) ldn_be_p(p, sz)
#define stn_p(p, sz, v) stn_be_p(p, sz, v)
#else
#define lduw_p(p) lduw_le_p(p)
#define ldsw_p(p) ldsw_le_p(p)
#define ldl_p(p) ldl_le_p(p)
#define ldq_p(p) ldq_le_p(p)
#define ldfl_p(p) ldfl_le_p(p)
#define ldfq_p(p) ldfq_le_p(p)
#define stw_p(p, v) stw_le_p(p, v)
#define stl_p(p, v) stl_le_p(p, v)
#define stq_p(p, v) stq_le_p(p, v)
#define stfl_p(p, v) stfl_le_p(p, v)
#define stfq_p(p, v) stfq_le_p(p, v)
#define ldn_p(p, sz) ldn_le_p(p, sz)
#define stn_p(p, sz, v) stn_le_p(p, sz, v)
#endif

/**
 * cpu_set_cpustate_pointers(cpu)
 * @cpu: The cpu object
 *
 * Set the generic pointers in CPUState into the outer object.
 */
static inline void cpu_set_cpustate_pointers(ArchCPU *cpu) {
  cpu->parent_obj.env_ptr = &cpu->env;
  cpu->parent_obj.icount_decr_ptr = &cpu->neg.icount_decr;
}

#endif /* end of include guard: CPU_ALL_H_9ZPFYTXB */
