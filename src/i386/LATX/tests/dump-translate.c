#include "../include/common.h"
#include "../x86tomips-config.h"
#include "../x86tomips-options.h"
#include "../ir1/ir1.h"
#include "../ir2/ir2.h"
#include "../include/env.h"
#include "../include/etb.h"
#include "../include/reg_alloc.h"
#include "../include/flag_pattern.h"
#include "../include/flag_lbt.h"
#include <string.h>

#include "../include/qemu-def.h"

#include "../include/test-def.h"

#ifdef CONFIG_SOFTMMU
void *tt_test_dump_translate_inst(CPUState *cpu)
{
    CPUX86State *env = cpu->env_ptr;

    TranslationBlock *tb = tt_alloc_tb(cpu);

    ETB *etb = (ETB*)mm_malloc(sizeof(ETB));
    etb_init(etb);
    tb->extra_tb = etb;

    option_dump       = 1;
    option_dump_ir1   = 1;
    option_dump_ir2   = 1;
    option_dump_host  = 1;
    option_dump_itemp = 1;

    tr_sys_init(tb, 64, get_code_highwater());

    int ir1_max = tt_cfg.inst_max_num;
    IR1_INST *ir1 = mm_calloc(ir1_max, sizeof(IR1_INST));
    int ir1_nr = tt_get_ir1_array(ir1, ir1_max);

    etb->_ir1_instructions = ir1;
    etb->_ir1_num = ir1_nr;

    lsenv->tr_data->ir1_inst_array = ir1;
    lsenv->tr_data->ir1_nr = ir1_nr;
    lsenv->tr_data->curr_ir1_inst = NULL;

    tb_flag_usedef(tb);

    etb_check_top_in(etb, env->fpstt);

    int search_size;
    tr_translate_tb(tb, &search_size);

    exit(-1);

    return tb;
}
#else
void *tt_test_dump_translate_inst(CPUState *cpu)
{
    printf("Func %s Not implemented in user-mode.\n",
            __func__);
    return NULL;
}
#endif
