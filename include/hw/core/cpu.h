#ifndef CPU_H_5RAXENPS
#define CPU_H_5RAXENPS

// FIXME can I include this in kernel mode?
#include "../../qemu/queue.h"
#include "../../types.h"
#include "../../exec/memattrs.h"
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

#include "../../../src/i386/cpu.h"

#endif /* end of include guard: CPU_H_5RAXENPS */
