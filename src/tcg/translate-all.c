#include "../../include/exec/exec-all.h"
#include "../../include/exec/tb-lookup.h"
#include "../../include/hw/core/cpu.h"
#include "../../include/sysemu/replay.h"
#include "../../include/types.h"
#include <stdbool.h> // FIXME header for bool

/* Called with mmap_lock held for user mode emulation.  */
TranslationBlock *tb_gen_code(CPUState *cpu, target_ulong pc,
                              target_ulong cs_base, u32 flags, int cflags) {
  // TODO
}
