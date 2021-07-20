#include "../../include/exec/cpu-all.h"
#include "../../include/exec/helper-head.h"
#include "../../include/exec/tb-lookup.h"
#include "../i386/cpu.h"

#if defined(CONFIG_X86toMIPS) && defined(CONFIG_SOFTMMU)
extern void cam_insert(TranslationBlock *tb);
#endif

void *HELPER(lookup_tb)(CPUArchState *env) {
  CPUState *cpu = env_cpu(env);
  TranslationBlock *tb;
  target_ulong cs_base, pc;
  uint32_t flags;

  tb = tb_lookup__cpu_state(cpu, &pc, &cs_base, &flags, curr_cflags());
#if defined(CONFIG_X86toMIPS) && defined(CONFIG_SOFTMMU)
  if (xtm_tblookup_opt())
    cam_insert(tb);
#endif
  return tb;
}
