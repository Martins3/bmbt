#ifndef TCG_H_ZDBR7L4E
#define TCG_H_ZDBR7L4E

#include <stdbool.h>
#include "../qemu/config-host.h"

// FIXME copied from include/sysemu/tcg.h
// it will be deleted later?
extern bool tcg_allowed;
void tcg_exec_init(unsigned long tb_size);
#ifdef CONFIG_TCG
#define tcg_enabled() (tcg_allowed)
#define kvm_enabled() 0
#define whpx_enabled() 0
#else
#define tcg_enabled() 0
#endif

#endif /* end of include guard: TCG_H_ZDBR7L4E */
