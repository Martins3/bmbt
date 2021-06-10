#ifndef _QEMU_DEF_H_
#define _QEMU_DEF_H_

// TODO oh my god, the junks
#include "../../../../include/qemu/osdep.h"
// #include "qemu/host-utils.h"
#include "../../cpu.h"
// #include "../../../../include/disas/disas.h"
#include "../../../../include/exec/exec-all.h"
// #include "../../../../include/tcg-op.h"
#include "../../../../include/exec/memop.h"
// #include "../../../../include/exec/cpu_ldst.h"
// #include "../../../../include/exec/helper-proto.h"
// #include "../../../../include/exec/helper-gen.h"
// #include "../../../../include/trace-tcg.h"
// #include "../../../../include/exec/log.h"
// #include "../../../../include/qemu/qht.h"

#include "../../../../include/qemu/queue.h"

#include "../../../../include/todo.h"
#include "../../../tcg/tcg.h"
#include "../../../i386/helper.h"

/*#include "exec/gen-icount.h"*/

typedef struct TranslationBlock TB;
typedef struct qht QHT;

#endif
