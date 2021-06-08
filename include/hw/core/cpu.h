#ifndef CPU_H_5RAXENPS
#define CPU_H_5RAXENPS

// FIXME can I include this in kernel mode?
#include <setjmp.h>
#include "../../types.h"

typedef struct CPUState CPUState;

typedef struct CPUClass {
 
  void (*cpu_exec_exit)(CPUState *cpu);
  void (*cpu_exec_enter)(CPUState *cpu);


} CPUClass;

typedef struct CPUState {
  CPUClass *cc; // TODO init this
  sigjmp_buf jmp_env;
  u32 cflags_next_tb; // TODO why I need this ?

} CPUState;

#endif /* end of include guard: CPU_H_5RAXENPS */
