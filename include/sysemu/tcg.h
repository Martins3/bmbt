#ifndef TCG_H_ZDBR7L4E
#define TCG_H_ZDBR7L4E

#include "../qemu/compiler.h"
#include "../qemu/config-host.h"
#include <assert.h>
#include <stdbool.h>

void tcg_exec_init(unsigned long tb_size);
int tcg_init();

extern bool tcg_allowed;
static inline bool tcg_enabled() {
  assert(tcg_allowed == true);
  return tcg_allowed;
}

#endif /* end of include guard: TCG_H_ZDBR7L4E */
