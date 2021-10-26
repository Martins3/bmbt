#include "../include/common.h"
#include "../include/env.h"
#include "../include/etb.h"
#include "../ir2/ir2.h"
#include "../ir1/ir1.h"
#include "../include/reg_alloc.h"

#include "../include/flag_lbt.h"
#include "../x86tomips-options.h"

#include "../include/ibtc.h"
#include "../include/profile.h"
#include "../include/flag_pattern.h"
#include "../include/shadow_stack.h"

#ifdef CONFIG_SOFTMMU
#  ifdef CONFIG_XTM_PROFILE
#  include "x86tomips-profile-sys.h"
#  endif
#endif

#include <string.h>


/* Called for indirect jmp lookup
 *   > After CAM lookup miss
 *   > Before helper_lookup_tb()
 */
ADDR njc_lookup_tb;

void x86_to_mips_njc_init(void)
{
    njc_lookup_tb = 0;
}

static
int generate_njc_lookup_tb(void *code_buffer)
{
    int mips_num = 0;
    tr_init(NULL);

    /* a7: Guest PC
     * s0: &CPUX86State */
    IR2_OPND gpc = ra_alloc_dbt_arg2();
    IR2_OPND tmp = ir2_opnd_new(IR2_OPND_GPR, 0x7); /* a3 */

    IR2_OPND *stmp1 = &stmp1_ir2_opnd; /* $a4 */
    IR2_OPND *stmp2 = &stmp2_ir2_opnd; /* $a5 */

    /* 1. Calculate hash of PC */

    /* stmp1 = pc
     *         ^
     *         (pc >> (TARGET_PAGE_BITS - TB_JMP_PAGE_BITS)) */
    append_ir2_opnd2i(LISA_SRLI_D, stmp1, &gpc,
            (TARGET_PAGE_BITS - TB_JMP_PAGE_BITS));
    append_ir2_opnd3(LISA_XOR, stmp1, &gpc, stmp1);

    /* stmp2 = ((tmp >> (TARGET_PAGE_BITS - TB_JMP_PAGE_BITS))
     *          &
     *          TB_JMP_PAGE_MASK) */
    load_imm32_to_ir2(&tmp, TB_JMP_PAGE_MASK, ZERO_EXTENSION);
    append_ir2_opnd2i(LISA_SRLI_D, stmp2, stmp1,
            (TARGET_PAGE_BITS - TB_JMP_PAGE_BITS));
    append_ir2_opnd3(LISA_AND, stmp2, stmp2, &tmp);

    /* stmp1 = (tmp & TB_JMP_ADDR_MASK)) */
    load_imm32_to_ir2(&tmp, TB_JMP_ADDR_MASK, ZERO_EXTENSION);
    append_ir2_opnd3(LISA_AND, stmp1, stmp1, &tmp);

    /* index = stmp1 | stmp2 */
    append_ir2_opnd3(LISA_OR, stmp1, stmp1, stmp2);

    /* cpu->tb_jmp_cache */
    X86CPU *xcpu = mm_malloc(sizeof(X86CPU));
    CPUState *cpu = (void*)xcpu;
    uintptr_t offset = (uintptr_t)&xcpu->env - (uintptr_t)&cpu->tb_jmp_cache;
    free(xcpu);

    load_imm32_to_ir2(&tmp, (int)offset, SIGN_EXTENSION);
    append_ir2_opnd3(LISA_SUB_D, &tmp, &env_ir2_opnd, &tmp);

    /* cpu->tb_jmp_cache[index] */
    if (sizeof(void *) == 8) {
        append_ir2_opnd2i(LISA_SLLI_D, stmp1, stmp1, 3);
    } else {
        lsassert(0);
    }

    /* &cpu->tb_jmp_cache[index] */
    append_ir2_opnd3(LISA_ADD_D, &tmp, &tmp, stmp1);
    append_ir2_opnd2i(LISA_LD_D, &ret0_ir2_opnd, &tmp, 0);

    /* jirl     zero, ra, 0 */
    append_ir2_opnd0_(lisa_return);

    mips_num = tr_ir2_assemble(code_buffer);
    tr_fini(false);
    return mips_num;
}

int generate_native_jmp_cache(void *code_buffer)
{
    void *code_buf = code_buffer;
    int mips_num = 0;

    njc_lookup_tb = (ADDR)code_buf;
    mips_num += generate_njc_lookup_tb(code_buf);
    code_buf = code_buffer + (mips_num << 2);

    return mips_num << 2;
}
