#include "exec/cpu-all.h"
#include "exec/exec-all.h"
#include "exec/tb-lookup.h"
#include "hw/core/cpu.h"
#include "sysemu/replay.h"
#include "types.h"

void cpu_loop_exit(CPUState *cpu);

int target_x86_to_mips_host(CPUState *cpu, TranslationBlock *tb, int max_insns,
                            void *code_highwater, int *search_size);

void g_assert_not_reached(void);

#define atomic_set(ptr, i)                                                     \
  do {                                                                         \
    *(ptr) = i;                                                                \
  } while (0)

void tcg_tb_insert(TranslationBlock *tb);
