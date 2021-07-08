#ifndef CPU_H_5RAXENPS
#define CPU_H_5RAXENPS

// FIXME can I include this in kernel mode?
#include "../../exec/memattrs.h"
#include "../../qemu/bitmap.h"
#include "../../qemu/queue.h"
#include "../../types.h"
#include <setjmp.h>

typedef struct CPUState CPUState;

/**
 * vaddr:
 * Type wide enough to contain any #target_ulong virtual address.
 */
typedef uint64_t vaddr;

typedef struct CPUBreakpoint {
  vaddr pc;
  int flags; /* BP_* */
  QTAILQ_ENTRY(CPUBreakpoint) entry;
} CPUBreakpoint;

struct CPUWatchpoint {
  vaddr vaddr;
  // FIXME  originally style warned by ccls
  // I don't know why ccls doesn't warn me in QEMU source code tree
  ::vaddr len;
  ::vaddr hitaddr;
  MemTxAttrs hitattrs;
  int flags; /* BP_* */
  QTAILQ_ENTRY(CPUWatchpoint) entry;
};

#define TB_JMP_CACHE_BITS 12
#define TB_JMP_CACHE_SIZE (1 << TB_JMP_CACHE_BITS)

typedef struct CPUState {
  // FIXME cores
  int nr_cores;
  int nr_threads;

  sigjmp_buf jmp_env;
  u32 cflags_next_tb; // FIXME why I need this ?

  void *env_ptr; /* CPUArchState */

  int singlestep_enabled;

  // FIXME
  // what does index mean?
  // - cpu_index, only one cpu

  /* TODO Move common fields from CPUArchState here. */
  int cpu_index;
  int cluster_index;
  uint32_t halted;
  uint32_t can_do_io;
  int32_t exception_index;

  /* ice debug support */
  QTAILQ_HEAD(, CPUBreakpoint) breakpoints;

  QTAILQ_HEAD(, CPUWatchpoint) watchpoints;
  CPUWatchpoint *watchpoint_hit;

  uint32_t interrupt_request;

  /* In order to avoid passing too many arguments to the MMIO helpers,
   * we store some rarely used information in the CPU context.
   */
  uintptr_t mem_io_pc;

  /* Accessed in parallel; all accesses must be atomic */
  struct TranslationBlock *tb_jmp_cache[TB_JMP_CACHE_SIZE];

  int64_t icount_budget;
  int64_t icount_extra;

  bool exit_request;

  bool in_exclusive_context;

  // FIXME the macros defines here temporarily
#define CPU_TRACE_DSTATE_MAX_EVENTS 32
  // FIXME currently, this field is only referenced by tb_lookup__cpu_state
  // it's weird, find how it works in original qemu
  DECLARE_BITMAP(trace_dstate, CPU_TRACE_DSTATE_MAX_EVENTS);
} CPUState;

// FIXME
// 1. this is originally a per thread value
/* current CPU in the current thread. It is only valid inside
   cpu_exec() */
extern CPUState *current_cpu;

/* Breakpoint/watchpoint flags */
#define BP_MEM_READ 0x01
#define BP_MEM_WRITE 0x02
#define BP_MEM_ACCESS (BP_MEM_READ | BP_MEM_WRITE)
#define BP_STOP_BEFORE_ACCESS 0x04
/* 0x08 currently unused */
#define BP_GDB 0x10
#define BP_CPU 0x20
#define BP_ANY (BP_GDB | BP_CPU)
#define BP_WATCHPOINT_HIT_READ 0x40
#define BP_WATCHPOINT_HIT_WRITE 0x80
#define BP_WATCHPOINT_HIT (BP_WATCHPOINT_HIT_READ | BP_WATCHPOINT_HIT_WRITE)

// FIXME wanna cry
// breakpoint will jump to ./exec.c
// so deep, dark, fantasy
int cpu_breakpoint_insert(CPUState *cpu, vaddr pc, int flags,
                          CPUBreakpoint **breakpoint);
int cpu_breakpoint_remove(CPUState *cpu, vaddr pc, int flags);
void cpu_breakpoint_remove_by_ref(CPUState *cpu, CPUBreakpoint *breakpoint);
void cpu_breakpoint_remove_all(CPUState *cpu, int mask);

// FIXME add a bunch of prototype
int cpu_watchpoint_insert(CPUState *cpu, vaddr addr, vaddr len, int flags,
                          CPUWatchpoint **watchpoint);
int cpu_watchpoint_remove(CPUState *cpu, vaddr addr, vaddr len, int flags);
void cpu_watchpoint_remove_by_ref(CPUState *cpu, CPUWatchpoint *watchpoint);
void cpu_watchpoint_remove_all(CPUState *cpu, int mask);
void cpu_check_watchpoint(CPUState *cpu, vaddr addr, vaddr len,
                          MemTxAttrs attrs, int flags, uintptr_t ra);
int cpu_watchpoint_address_matches(CPUState *cpu, vaddr addr, vaddr len);

typedef enum MMUAccessType {
  MMU_DATA_LOAD = 0,
  MMU_DATA_STORE = 1,
  MMU_INST_FETCH = 2
} MMUAccessType;

/* Since this macro is used a lot in hot code paths and in conjunction with
 * FooCPU *foo_env_get_cpu(), we deviate from usual QOM practice by using
 * an unchecked cast.
 */
#define CPU(obj) ((CPUState *)(obj))

/**
 * CPUDumpFlags:
 * @CPU_DUMP_CODE:
 * @CPU_DUMP_FPU: dump FPU register state, not just integer
 * @CPU_DUMP_CCOP: dump info about TCG QEMU's condition code optimization state
 */
enum CPUDumpFlags {
  CPU_DUMP_CODE = 0x00010000,
  CPU_DUMP_FPU = 0x00020000,
  CPU_DUMP_CCOP = 0x00040000,
};

// FIXME implement later
/**
 * cpu_reset:
 * @cpu: The CPU whose state is to be reset.
 */
void cpu_reset(CPUState *cpu);

// FIXME initialize cpu_interrupt_handler
// surely, we can simplify it.
typedef void (*CPUInterruptHandler)(CPUState *, int);
extern CPUInterruptHandler cpu_interrupt_handler;

/**
 * cpu_interrupt:
 * @cpu: The CPU to set an interrupt on.
 * @mask: The interrupts to set.
 *
 * Invokes the interrupt handler.
 */
static inline void cpu_interrupt(CPUState *cpu, int mask) {
  cpu_interrupt_handler(cpu, mask);
}

// FIXME will redesing cpu_abort,
// GCC_FMT_ATTR(2, 3) ??
void QEMU_NORETURN cpu_abort(CPUState *cpu, const char *fmt, ...);

/* work queue */

/* The union type allows passing of 64 bit target pointers on 32 bit
 * hosts in a single parameter
 */
typedef union {
  int host_int;
  unsigned long host_ulong;
  void *host_ptr;
  vaddr target_ptr;
} run_on_cpu_data;

#define RUN_ON_CPU_HOST_PTR(p) ((run_on_cpu_data){.host_ptr = (p)})
#define RUN_ON_CPU_HOST_INT(i) ((run_on_cpu_data){.host_int = (i)})
#define RUN_ON_CPU_HOST_ULONG(ul) ((run_on_cpu_data){.host_ulong = (ul)})
#define RUN_ON_CPU_TARGET_PTR(v) ((run_on_cpu_data){.target_ptr = (v)})
#define RUN_ON_CPU_NULL RUN_ON_CPU_HOST_PTR(NULL)

typedef void (*run_on_cpu_func)(CPUState *cpu, run_on_cpu_data data);

// FIXME take care of, it seems the highlight is misleading
static inline void cpu_tb_jmp_cache_clear(CPUState *cpu);

/*
 * Low 16 bits: number of cycles left, used only in icount mode.
 * High 16 bits: Set to -1 to force TCG to stop executing linked TBs
 * for this CPU and return to its top level loop (even in non-icount mode).
 * This allows a single read-compare-cbranch-write sequence to test
 * for both decrementer underflow and exceptions.
 */
typedef union IcountDecr {
  uint32_t u32;
  struct {
#ifdef HOST_WORDS_BIGENDIAN
    uint16_t high;
    uint16_t low;
#else
    uint16_t low;
    uint16_t high;
#endif
  } u16;
} IcountDecr;

typedef struct CPUWatchpoint CPUWatchpoint;

#define SSTEP_ENABLE 0x1  /* Enable simulated HW single stepping */
#define SSTEP_NOIRQ 0x2   /* Do not use IRQ while single stepping */
#define SSTEP_NOTIMER 0x4 /* Do not Timers while single stepping */

#define CPU_UNSET_NUMA_NODE_ID -1
#define CPU_TRACE_DSTATE_MAX_EVENTS 32

#define first_cpu QTAILQ_FIRST_RCU(&cpus)
#define CPU_NEXT(cpu) QTAILQ_NEXT_RCU(cpu, node)
#define CPU_FOREACH(cpu) QTAILQ_FOREACH_RCU(cpu, &cpus, node)
#define CPU_FOREACH_SAFE(cpu, next_cpu)                                        \
  QTAILQ_FOREACH_SAFE_RCU(cpu, &cpus, node, next_cpu)

/**
 * cpu_in_exclusive_context()
 * @cpu: The vCPU to check
 *
 * Returns true if @cpu is an exclusive context, for example running
 * something which has previously been queued via async_safe_run_on_cpu().
 */
static inline bool cpu_in_exclusive_context(const CPUState *cpu) {
  return cpu->in_exclusive_context;
}

#include "../../../src/i386/cpu.h"

#endif /* end of include guard: CPU_H_5RAXENPS */
