#ifndef CPU_H_5RAXENPS
#define CPU_H_5RAXENPS

// FIXME can I include this in kernel mode?
#include "../../exec/memattrs.h"
#include "../../qemu/queue.h"
#include "../../types.h"
#include <setjmp.h>

typedef struct CPUState CPUState;

/**
 * vaddr:
 * Type wide enough to contain any #target_ulong virtual address.
 */
typedef uint64_t vaddr;

typedef struct CPUClass {

  void (*cpu_exec_exit)(CPUState *cpu);
  void (*cpu_exec_enter)(CPUState *cpu);

} CPUClass;

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

typedef struct CPUState {
  CPUClass *cc; // TODO init this

  // TODO cores
  int nr_cores;
  int nr_threads;

  sigjmp_buf jmp_env;
  u32 cflags_next_tb; // TODO why I need this ?

  void *env_ptr; /* CPUArchState */

  int singlestep_enabled;

  // FIXME
  // what does index mean?
  // - cpu_index, only one cpu
  //
  //
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

} CPUState;

// FIXME
// 1. this is originally a per thread value
// 2. haven't be defined yet
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
    MMU_DATA_LOAD  = 0,
    MMU_DATA_STORE = 1,
    MMU_INST_FETCH = 2
} MMUAccessType;


#include "../../../src/i386/cpu.h"

#endif /* end of include guard: CPU_H_5RAXENPS */
