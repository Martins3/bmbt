#ifndef TB_LOOKUP_H_9IR5EWTY
#define TB_LOOKUP_H_9IR5EWTY
#include "../hw/core/cpu.h"
#include "../exec/exec-all.h"
#include "../types.h"


static inline TranslationBlock *
tb_lookup__cpu_state(CPUState *cpu, target_ulong *pc, target_ulong *cs_base,
                     u32 *flags, u32 cf_mask){
  // TODO
  // this function also called by tcg-runtime.c
  // does it mean tb is looked up while executed?
  TranslationBlock * tb;
  return tb;
}

#endif /* end of include guard: TB_LOOKUP_H_9IR5EWTY */
