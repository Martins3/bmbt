#include "../../include/exec/cpu-ldst.h"
#include "../../include/exec/exec-all.h"
#include "../../include/fpu/softfloat-helper.h"
#include "../../include/fpu/softfloat.h"
#include "../../include/qemu/bswap.h"
#include "LATX/x86tomips-config.h"
#include "cpu.h"

target_ulong helper_read_eflags(CPUX86State *env) {
  uint32_t eflags;

  eflags = cpu_cc_compute_all(env, CC_OP);
#ifdef CONFIG_X86toMIPS
  eflags |= env->eflags;
  if (env->df == 1) {
    eflags &= ~DF_MASK;
  } else {
    eflags |= DF_MASK;
  }
#else
  eflags |= (env->df & DF_MASK);
  eflags |= env->eflags & ~(VM_MASK | RF_MASK);
#endif
  return eflags;
}

void helper_write_eflags(CPUX86State *env, target_ulong t0,
                         uint32_t update_mask) {
  cpu_load_eflags(env, t0, update_mask);
}
