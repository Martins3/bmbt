#ifndef CPU_H_5RAXENPS
#define CPU_H_5RAXENPS

// FIXME can I include this in kernel mode?
#include "../../types.h"
#include <setjmp.h>

typedef struct CPUState CPUState;

typedef struct CPUClass {

  void (*cpu_exec_exit)(CPUState *cpu);
  void (*cpu_exec_enter)(CPUState *cpu);

} CPUClass;

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

} CPUState;

// FIXME
// 1. this is originally a per thread value
// 2. haven't be defined yet
/* current CPU in the current thread. It is only valid inside
   cpu_exec() */
extern CPUState *current_cpu;

#include "../../../src/i386/cpu.h"

#endif /* end of include guard: CPU_H_5RAXENPS */
