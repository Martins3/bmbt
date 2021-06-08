#ifndef EXEC_ALL_H_SFIHOIQZ
#define EXEC_ALL_H_SFIHOIQZ
#include "../hw/core/cpu.h"
#include "../types.h"

typedef struct TranslationBlock {
  u64 page_addr[2];

} TranslationBlock;

/* current cflags for hashing/comparison */
static inline u32 curr_cflags(void) {
  // TODO
  return 0;
}

TranslationBlock *tb_gen_code(CPUState *cpu, target_ulong pc,
                              target_ulong cs_base, u32 flags, int cflags);

#endif /* end of include guard: EXEC_ALL_H_SFIHOIQZ */
