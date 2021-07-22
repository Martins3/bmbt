#ifndef DISAS_H_1FXNJ9VM
#define DISAS_H_1FXNJ9VM

#include "../exec/cpu-defs.h"

/* Look up symbol for debugging purpose.  Returns "" if unknown. */
const char *lookup_symbol(target_ulong orig_addr);

#endif /* end of include guard: DISAS_H_1FXNJ9VM */
