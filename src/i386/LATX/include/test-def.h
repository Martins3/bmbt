#ifndef _XTM_TEST_DEF_H_
#define _XTM_TEST_DEF_H_

#include "../include/common.h"
#include "../x86tomips-config.h"
#include "../x86tomips-options.h"
#include "../ir1/ir1.h"
#include "../ir2/ir2.h"
#include "../include/env.h"
#include "../include/etb.h"
#include "../include/reg-alloc.h"
#include "../include/flag-pattern.h"
#include "../include/flag-lbt.h"
#include <string.h>

#include "../include/qemu-def.h"

extern void  tt_reset_cpu(CPUState *cpu);
extern void *tt_test(CPUState *cpu);
extern void  tt_exit(CPUState *cpu);

void *tt_test_dump_translate_inst(CPUState *cpu);
void *tt_test_run_on_tb(CPUState *cpu);

/* Functions implemented in accel/tcg/translate-all.c */
extern TranslationBlock *tt_alloc_tb(CPUState *cpu);
extern void *get_code_highwater(void);

typedef struct testenv {
    target_ulong regs[CPU_NB_REGS];
    target_ulong eflags;
    SegmentCache segs[6];
    uint8_t pe;
    uint8_t vm86;
}TESTENV;

typedef struct testcfg {
    int inst_max_num;
}TESTCFG;

#define TT_INST_MAX_DEFAULT 64

extern TESTCFG tt_cfg;
extern TESTENV tt_env_reset;
extern char   *tt_inst_str;

uint8_t tt_convert_2char_to_1byte(char *c, int i);
int tt_get_ir1_array(IR1_INST *ir1_array, int ir1_max);

#endif
