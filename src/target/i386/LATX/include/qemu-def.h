#ifndef _QEMU_DEF_H_
#define _QEMU_DEF_H_

#include <qemu/osdep.h>
#include "../../cpu.h"
#include <exec/exec-all.h>
#include <exec/cpu-ldst.h>
#include <exec/memop.h>
#include <exec/tb-hash.h>
#include <qemu/qemu-printf.h>
#include <qemu/queue.h>
#include <fpu/softfloat.h>
#include "../../src/tcg/tcg.h"
#include <exec/helper-proto.h>
#include <exec/helper-gen.h>

/*#include "exec/gen-icount.h"*/

typedef struct TranslationBlock TB;
typedef struct qht QHT;

#endif
