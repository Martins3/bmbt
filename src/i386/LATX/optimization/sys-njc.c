#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>

#include "exec/tb-hash.h"

int njc_enabled(void)
{
    return option_njc;
}

int gen_latxs_njc_lookup_tb(void *code_ptr)
{
    int code_nr = 0;

    latxs_tr_init(NULL);

    /* s0: &CPUX86State */
    /* a2: pc loaded from env, any reg should be ok */
    IR2_OPND gpc = latxs_ir2_opnd_new(IR2_OPND_GPR, 0x7);
#ifdef TARGET_X86_64
    latxs_append_ir2_opnd2i(LISA_LD_D, &gpc, &latxs_env_ir2_opnd,
        lsenv_offset_of_eip(lsenv));
#else
    latxs_append_ir2_opnd2i(LISA_LD_WU, &gpc, &latxs_env_ir2_opnd,
        lsenv_offset_of_eip(lsenv));
#endif
    IR2_OPND tmp = latxs_ir2_opnd_new(IR2_OPND_GPR, 0x7); /* a3 */

    IR2_OPND *stmp1 = &latxs_stmp1_ir2_opnd; /* $a4 */
    IR2_OPND *stmp2 = &latxs_stmp2_ir2_opnd; /* $a5 */

    /* 1. Calculate hash of PC */

    /*
     * stmp1 = pc
     *         ^
     *         (pc >> (TARGET_PAGE_BITS - TB_JMP_PAGE_BITS))
     */
    latxs_append_ir2_opnd2i(LISA_SRLI_D, stmp1, &gpc,
            (TARGET_PAGE_BITS - TB_JMP_PAGE_BITS));
    latxs_append_ir2_opnd3(LISA_XOR, stmp1, &gpc, stmp1);

    /*
     * stmp2 = ((tmp >> (TARGET_PAGE_BITS - TB_JMP_PAGE_BITS))
     *          &
     *          TB_JMP_PAGE_MASK)
     */
    latxs_load_imm32_to_ir2(&tmp, TB_JMP_PAGE_MASK, EXMode_Z);
    latxs_append_ir2_opnd2i(LISA_SRLI_D, stmp2, stmp1,
            (TARGET_PAGE_BITS - TB_JMP_PAGE_BITS));
    latxs_append_ir2_opnd3(LISA_AND, stmp2, stmp2, &tmp);

    /* stmp1 = (tmp & TB_JMP_ADDR_MASK)) */
    latxs_load_imm32_to_ir2(&tmp, TB_JMP_ADDR_MASK, EXMode_Z);
    latxs_append_ir2_opnd3(LISA_AND, stmp1, stmp1, &tmp);

    /* index = stmp1 | stmp2 */
    latxs_append_ir2_opnd3(LISA_OR, stmp1, stmp1, stmp2);

    /* cpu->tb_jmp_cache */
    /* X86CPU *xcpu = mm_malloc(sizeof(X86CPU)); */
    /* CPUState *cpu = (void*)xcpu; */
    /* uintptr_t offset = (uintptr_t)&xcpu->env - */
                       /* (uintptr_t)&cpu->tb_jmp_cache; */
    /* free(xcpu); */

    /* latxs_load_imm32_to_ir2(&tmp, (int)offset, EXMode_S); */
    /* latxs_append_ir2_opnd3(LISA_SUB_D, &tmp, &latxs_env_ir2_opnd, &tmp); */
    latxs_append_ir2_opnd2i(LISA_LD_D, &tmp, &latxs_env_ir2_opnd,
            lsenv_offset_of_tb_jmp_cache_ptr(lsenv));

    /* cpu->tb_jmp_cache[index] */
    if (sizeof(void *) == 8) {
        latxs_append_ir2_opnd2i(LISA_SLLI_D, stmp1, stmp1, 3);
    } else {
        lsassert(0);
    }

    /* &cpu->tb_jmp_cache[index] */
    latxs_append_ir2_opnd3(LISA_ADD_D, &tmp, &tmp, stmp1);
    latxs_append_ir2_opnd2i(LISA_LD_D, &latxs_ret0_ir2_opnd, &tmp, 0);

    /* jirl     zero, ra, 0 */
    latxs_append_ir2_opnd0_(lisa_return);

    code_nr = latxs_tr_ir2_assemble(code_ptr);
    latxs_tr_fini();
    return code_nr;
}
