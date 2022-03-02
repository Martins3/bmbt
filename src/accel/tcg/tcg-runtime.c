#include <exec/cpu-all.h>
#include <exec/helper-proto.h>
#include <exec/tb-lookup.h>
#include "cpu.h"

void *HELPER(lookup_tb)(CPUArchState *env) {
  CPUState *cpu = env_cpu(env);
  TranslationBlock *tb;
  target_ulong cs_base, pc;
  uint32_t flags;

  tb = tb_lookup__cpu_state(cpu, &pc, &cs_base, &flags, curr_cflags());
  return tb;
}
