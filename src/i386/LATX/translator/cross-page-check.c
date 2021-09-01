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

#include "../include/cross-page-check.h"

#ifdef CONFIG_SOFTMMU

/* Code Page Table management */

void xtm_cpt_flush(void)
{
    if (!xtm_cpc_enabled()) return;
    if (lsenv) {
        cpt_t *cpt = &lsenv->cpc_data.cpt;
        memset(&cpt->cpt_v[0], 0,
               sizeof(cpt_t) - offsetof(cpt_t, cpt_v));
    }
}

void xtm_cpt_insert_tb(void *_tb)
{
    if (!xtm_cpc_enabled()) return;

    TranslationBlock *tb = _tb;
    if (tb->page_addr[1] != -1) return;

    cpt_t *cpt = &lsenv->cpc_data.cpt;

    uint32_t gva = tb->pc & TARGET_PAGE_MASK;
    uint32_t gpa = tb->page_addr[0] & TARGET_PAGE_MASK;

    int idx = (gva >> TARGET_PAGE_BITS) & CODE_PAGE_TABLE_MASK;

    cpt->cpt_v[idx] = 1;
    cpt->cpt_e[idx].gpa = gpa;
}

void xtm_cpt_flush_page(uint32_t addr)
{
    if (!xtm_cpc_enabled()) return;
    int idx = (addr & TARGET_PAGE_MASK) & CODE_PAGE_TABLE_MASK;
    lsenv->cpc_data.cpt.cpt_v[idx] = 0;
}

/* jmp glue for cross page check */

ADDR native_jmp_glue_cpc_0;
ADDR native_jmp_glue_cpc_1;

int xtm_tb_need_cpc(TranslationBlock *tb, IR1_INST *jmp, int n)
{
    ADDRX pc_start = tb->pc & TARGET_PAGE_MASK;
    ADDRX pc_end = 0;

    if (!xtm_cpc_enabled()) return 0;

    if (ir1_is_branch(jmp)) {
        if (n == 0) {
            pc_end = ir1_addr_next(jmp) & TARGET_PAGE_MASK;
            if (pc_end != pc_start) return 1;
        }
        if (n == 1) {
            pc_end = ir1_target_addr(jmp) & TARGET_PAGE_MASK;
            if (pc_end != pc_start) return 1;
        }
        return 0;
    }

    if (ir1_is_call(jmp) && !ir1_is_indirect_call(jmp)) {
        pc_end = ir1_target_addr(jmp) & TARGET_PAGE_MASK;
        if (pc_end != pc_start) return 1;
    }

    if (ir1_is_jump(jmp) && !ir1_is_indirect_jmp(jmp)) {
        pc_end = ir1_target_addr(jmp) & TARGET_PAGE_MASK;
        if (pc_end != pc_start) return 1;
    }

    return 0;
}

static
int generate_cross_page_check(void *code_buffer, int n)
{
    int mips_num = 0;
    tr_init(NULL);
    lsassertm(0, "cross page check to be implemented in LoongArch\n");
//
//    lsassert(n == 0 || n == 1);
//
//    /* $t8: prev TB's address (aka. &TB) */
//    IR2_OPND tb = ra_alloc_dbt_arg1();
//
//    IR2_OPND *stmp1 = &stmp1_ir2_opnd; /* next TB  */
//    IR2_OPND *stmp2 = &stmp2_ir2_opnd; /* base CPT */
//
//    /* load TB.etb */
//    ir2_opnd_set_em(&tb, EM_MIPS_ADDRESS, 32);
//    append_ir2_opnd2i(mips_ld, stmp1, &tb,
//            offsetof(TranslationBlock, extra_tb));
//    /* stmp1: load TB.etb.next_tb[n] */
//    ir2_opnd_set_em(stmp1, EM_MIPS_ADDRESS, 32);
//    append_ir2_opnd2i(mips_ld, stmp1, stmp1,
//            offsetof(struct ExtraBlock, next_tb) + n * sizeof(void *));
//    ir2_opnd_set_em(stmp1, EM_MIPS_ADDRESS, 32);
//
//    /* stmp2: load Code Page Table */
//    append_ir2_opnd2i(mips_ld, stmp2, &env_ir2_opnd,
//            offsetof(CPUX86State, cpt_ptr));
//
//    /* arg0: next TB.PC          aka. GVA */
//    append_ir2_opnd2i(mips_lwu, &arg0_ir2_opnd, stmp1,
//            offsetof(TranslationBlock, pc));
//    /* arg1: next TB.page addr   aka. GPA */
//    append_ir2_opnd2i(mips_lwu, &arg1_ir2_opnd, stmp1,
//            offsetof(TranslationBlock, page_addr));
//
//    /* arg2: index = (GVA >> 12) & 0x3ff */
//    append_ir2_opnd2i(mips_dsrl, &arg2_ir2_opnd, &arg0_ir2_opnd,
//            TARGET_PAGE_BITS);
//    append_ir2_opnd2i(mips_andi, &arg2_ir2_opnd, &arg2_ir2_opnd,
//            CODE_PAGE_TABLE_MASK);
//
//    /* arg4: valid bit: cpt.cpt_v[index] */
//    append_ir2_opnd2i(mips_daddiu, &arg3_ir2_opnd, stmp2,
//                      offsetof(cpt_t, cpt_v));
//    append_ir2_opnd3(mips_daddu, &arg3_ir2_opnd, &arg3_ir2_opnd,
//                     &arg2_ir2_opnd);
//    ir2_opnd_set_em(&arg3_ir2_opnd, EM_MIPS_ADDRESS, 32);
//    append_ir2_opnd2i(mips_lbu, &arg4_ir2_opnd, &arg3_ir2_opnd, 0);
//
//    /* arg5: tag to compare = GPA | (valid - 1) */
//    append_ir2_opnd2i(mips_dsubiu, &arg5_ir2_opnd, &arg4_ir2_opnd, 1);
//    append_ir2_opnd3(mips_or, &arg5_ir2_opnd, &arg5_ir2_opnd, &arg1_ir2_opnd);
//
//    /* arg6: GPA in Code Page Table */
//    switch (sizeof(cpt_entry_t)) {
//    case 4:
//        append_ir2_opnd2i(mips_dsll, &arg3_ir2_opnd, &arg2_ir2_opnd, 2);
//        break;
//    default: lsassert(0); break;
//    }
//    append_ir2_opnd3(mips_daddu, &arg3_ir2_opnd, stmp2, &arg3_ir2_opnd);
//    ir2_opnd_set_em(&arg3_ir2_opnd, EM_MIPS_ADDRESS, 32);
//    append_ir2_opnd2i(mips_lwu, &arg6_ir2_opnd, &arg3_ir2_opnd, 0);
//
//    /* COMPARE */
//    IR2_OPND label_context_switch = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_bne, &arg5_ir2_opnd, &arg6_ir2_opnd,
//            &label_context_switch);
//
//    append_ir2_opnd2i(mips_ld, &arg3_ir2_opnd, stmp1,
//            offsetof(TranslationBlock, tc) + offsetof(struct tb_tc, ptr));
//#ifdef CONFIG_XTM_PROFILE
//    tr_pf_inc_cpc_hit();
//#endif
//    append_ir2_opnd1(mips_jr, &arg3_ir2_opnd);
//
//    append_ir2_opnd1(mips_label, &label_context_switch);
//    IR2_OPND next_eip_opnd = ra_alloc_dbt_arg2();
//    append_ir2_opnd3(mips_or, &next_eip_opnd, &zero_ir2_opnd, &arg0_ir2_opnd);
//#ifdef CONFIG_XTM_PROFILE
//    tr_pf_inc_cpc_miss();
//#endif
//    append_ir2_opnda_not_nop(mips_j, context_switch_native_to_bt);
//    append_ir2_opnd2i(mips_ori, &ret_ir2_opnd, &zero_ir2_opnd, n);
//
//    mips_num = tr_ir2_assemble(code_buffer);
//    tr_fini(false);
    return mips_num;
}

int generate_jmp_glue_cpc(void *code_buffer, int n)
{
    void *code_buf = code_buffer;
    int mips_num = 0;

    if (n) {
        native_jmp_glue_cpc_1 = (ADDR)code_buf;
    } else {
        native_jmp_glue_cpc_0 = (ADDR)code_buf;
    }

    mips_num += generate_cross_page_check(code_buf, n);
    code_buf = code_buffer + (mips_num << 2);

    return mips_num << 2;
}

#else

void xtm_cpt_flush(void) {}
void xtm_cpt_insert_tb(void *_tb) {}
void xtm_cpt_flush_page(uint32_t addr) {}

#endif
