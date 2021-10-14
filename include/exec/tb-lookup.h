#ifndef TB_LOOKUP_H_9IR5EWTY
#define TB_LOOKUP_H_9IR5EWTY
#include "../exec/exec-all.h"
#include "../exec/tb-hash.h"
#include "../hw/core/cpu.h"
#include "../types.h"

#if defined(CONFIG_X86toMIPS) && defined(CONFIG_SOFTMMU)
#ifdef CONFIG_XTM_PROFILE
#include "x86tomips-profile-sys.h"
#endif
// #include "../../src/i386/LATX/include/cross-page-check.h"
#endif

static inline TranslationBlock *tb_lookup__cpu_state(CPUState *cpu,
                                                     target_ulong *pc,
                                                     target_ulong *cs_base,
                                                     u32 *flags, u32 cf_mask) {
  CPUArchState *env = (CPUArchState *)cpu->env_ptr;
  TranslationBlock *tb;
  uint32_t hash;

  cpu_get_tb_cpu_state(env, pc, cs_base, flags);
  hash = tb_jmp_cache_hash_func(*pc);
  tb = atomic_rcu_read(&cpu->tb_jmp_cache[hash]);

  cf_mask &= ~CF_CLUSTER_MASK;
  cf_mask |= cpu->cluster_index << CF_CLUSTER_SHIFT;

  if (likely(tb && tb->pc == *pc && tb->cs_base == *cs_base &&
             tb->flags == *flags &&
             tb->trace_vcpu_dstate == *cpu->trace_dstate &&
             (tb_cflags(tb) & (CF_HASH_MASK | CF_INVALID)) == cf_mask)) {
#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
    xtm_pf_jmp_cache(cpu, tb);
#endif
    return tb;
  }
#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
  else {
    xtm_pf_jmp_cache(cpu, NULL);
  }
#endif
  tb = tb_htable_lookup(cpu, *pc, *cs_base, *flags, cf_mask);
  if (tb == NULL) {
    return NULL;
  }
  atomic_set(&cpu->tb_jmp_cache[hash], tb);
#if defined(CONFIG_X86toMIPS) && defined(CONFIG_SOFTMMU)
  xtm_cpt_insert_tb(tb);
#endif
  return tb;
}

#endif /* end of include guard: TB_LOOKUP_H_9IR5EWTY */
