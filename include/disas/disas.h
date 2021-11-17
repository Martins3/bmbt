#ifndef DISAS_H_1FXNJ9VM
#define DISAS_H_1FXNJ9VM

#include "../exec/cpu-defs.h"

/* Look up symbol for debugging purpose.  Returns "" if unknown. */
static inline const char *lookup_symbol(target_ulong orig_addr) {
  // @todo it seems that xqm doesn't use it, verify it
  return "lookup_symbol is only used in cpu_tb_exec for debugging";
}

#endif /* end of include guard: DISAS_H_1FXNJ9VM */
