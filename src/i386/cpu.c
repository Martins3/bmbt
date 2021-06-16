#include "cpu.h"
#include "../../include/exec/cpu-ldst.h"
#include "../../include/exec/exec-all.h"
#include "../../include/fpu/softfloat-helper.h"
#include "../../include/fpu/softfloat.h"
#include "../../include/qemu/bswap.h"
#include "../../include/qemu/log-for-trace.h"

#include <stdbool.h>

/*
static int64_t x86_cpu_get_arch_id(CPUState *cs)
{
    X86CPU *cpu = X86_CPU(cs);

    return cpu->apic_id;
}
*/

static bool x86_cpu_get_paging_enabled(const CPUState *cs) {
  X86CPU *cpu = X86_CPU(cs);

  return cpu->env.cr[0] & CR0_PG_MASK;
}

static void x86_cpu_set_pc(CPUState *cs, vaddr value) {
  X86CPU *cpu = X86_CPU(cs);

  cpu->env.eip = value;
}

void x86_cpu_synchronize_from_tb(CPUState *cs, TranslationBlock *tb) {
  X86CPU *cpu = X86_CPU(cs);

  cpu->env.eip = tb->pc - tb->cs_base;
}
