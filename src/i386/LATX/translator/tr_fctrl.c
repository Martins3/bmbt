#include "common.h"
#include "reg_alloc.h"
#include "env.h"
#include "x86tomips-options.h"

bool translate_fnstcw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif
    /* 1. load the value of fpu control word */
    IR2_OPND cw_opnd = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_LD_HU, &cw_opnd, &env_ir2_opnd,
                      lsenv_offset_of_control_word(lsenv));

    /* 2. store the control word to the dest_opnd */
    //fix capstone bug
    pir1->info->detail->x86.operands[0].size = 2;
    store_ir2_to_ir1(&cw_opnd, ir1_get_opnd(pir1, 0), false);

    ra_free_temp(&cw_opnd);
    return true;
}

bool translate_fstcw(IR1_INST *pir1) { return translate_fnstcw(pir1); }

static void update_fcsr_enable(IR2_OPND control_word, IR2_OPND fcsr)
{
    IR2_OPND temp = ra_alloc_itemp();

    /* reset enables */
    append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &zero_ir2_opnd,
            FCSR_OFF_EN_V, FCSR_OFF_EN_I);
    /* IM */
    append_ir2_opnd2ii(LISA_BSTRPICK_W, &temp, &control_word,
            X87_CR_OFF_IM, X87_CR_OFF_IM);
    append_ir2_opnd2i (LISA_XORI, &temp, &control_word, 1);
    append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &temp,
            FCSR_OFF_EN_V, FCSR_OFF_EN_V);
    /* DM */
    append_ir2_opnd2ii(LISA_BSTRPICK_W, &temp, &control_word,
            X87_CR_OFF_DM, X87_CR_OFF_DM);
    append_ir2_opnd2i (LISA_XORI, &temp, &control_word, 1);
    append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &temp,
            FCSR_OFF_EN_I, FCSR_OFF_EN_I);
    /* ZM */
    append_ir2_opnd2ii(LISA_BSTRPICK_W, &temp, &control_word,
            X87_CR_OFF_ZM, X87_CR_OFF_ZM);
    append_ir2_opnd2i (LISA_XORI, &temp, &control_word, 1);
    append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &temp,
            FCSR_OFF_EN_Z, FCSR_OFF_EN_Z);
    /* OM */
    append_ir2_opnd2ii(LISA_BSTRPICK_W, &temp, &control_word,
            X87_CR_OFF_OM, X87_CR_OFF_OM);
    append_ir2_opnd2i (LISA_XORI, &temp, &control_word, 1);
    append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &temp,
            FCSR_OFF_EN_O, FCSR_OFF_EN_O);
    /* UM */
    append_ir2_opnd2ii(LISA_BSTRPICK_W, &temp, &control_word,
            X87_CR_OFF_UM, X87_CR_OFF_UM);
    append_ir2_opnd2i (LISA_XORI, &temp, &control_word, 1);
    append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &temp,
            FCSR_OFF_EN_U, FCSR_OFF_EN_U);
    append_ir2_opnd2(LISA_MOVGR2FCSR, &fcsr_ir2_opnd, &fcsr);

    ra_free_temp(&temp);
}

static void update_fcsr_rm(IR2_OPND control_word, IR2_OPND fcsr)
{
    /* reset rounding mode */
    append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &zero_ir2_opnd,
            FCSR_OFF_RM + 1, FCSR_OFF_RM);
    /* load rounding mode in x86 control register */
    append_ir2_opnd2ii(LISA_BSTRPICK_W, &control_word, &control_word,
            X87_CR_OFF_RC + 1, X87_CR_OFF_RC);
    /*
     *turn x86 rm to LA rm
     *          x86      LA
     *
     * RN       00       00
     * RD       01       11
     * RU       10       10
     * RZ       11       01
     *
     */
    IR2_OPND temp_cw = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ANDI, &temp_cw, &control_word, 1);
    IR2_OPND label = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BEQ, &temp_cw, &zero_ir2_opnd, &label);
    append_ir2_opnd2i(LISA_XORI, &control_word, &control_word, 2);
    append_ir2_opnd1(LISA_LABEL, &label);
    /* set rounding mode in LA FCSR */
    append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &control_word,
            FCSR_OFF_RM + 1, FCSR_OFF_RM);
    append_ir2_opnd2(LISA_MOVGR2FCSR, &fcsr_ir2_opnd, &fcsr);

    ra_free_temp(&temp_cw);
}

static void update_fcsr_by_cw(IR2_OPND cw)
{
    IR2_OPND old_fcsr = ra_alloc_itemp();
    append_ir2_opnd2(LISA_MOVFCSR2GR, &old_fcsr, &fcsr_ir2_opnd);
    update_fcsr_enable(cw, old_fcsr);
    update_fcsr_rm(cw, old_fcsr);
}

bool translate_fldcw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif
    /* 1. load new control word from the source opnd */
    IR2_OPND new_cw = ra_alloc_itemp();
    load_ir1_to_ir2(&new_cw, ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

    /* 2. store the value into the env->fpu_control_word */
    append_ir2_opnd2i(LISA_ST_H, &new_cw, &env_ir2_opnd,
                      lsenv_offset_of_control_word(lsenv));

    update_fcsr_by_cw(new_cw);
    //tr_gen_call_to_helper1((ADDR)update_fp_status);

    return true;
}

bool translate_stmxcsr(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
    if (tr_gen_excp_check(pir1)) return true;
#endif
    /* 1. load the value of the mxcsr register state from env */
    IR2_OPND mxcsr_opnd = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_LD_WU, &mxcsr_opnd, &env_ir2_opnd,
                      lsenv_offset_of_mxcsr(lsenv));

    /* 2. store  the value of the mxcsr register state to the dest_opnd */
    store_ir2_to_ir1(&mxcsr_opnd, ir1_get_opnd(pir1, 0), false);

    ra_free_temp(&mxcsr_opnd);
    return true;
}

bool translate_ldmxcsr(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
    if (tr_gen_excp_check(pir1)) return true;
#endif
    /* 1. load new mxcsr value from the source opnd */
    IR2_OPND new_mxcsr = ra_alloc_itemp();
    load_ir1_to_ir2(&new_mxcsr, ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

    /* 2. store the value into the env->mxcsr */
    append_ir2_opnd2i(LISA_ST_W, &new_mxcsr, &env_ir2_opnd,
                      lsenv_offset_of_mxcsr(lsenv));

    tr_gen_call_to_helper1((ADDR)update_mxcsr_status, 1);

    return true;
}

bool translate_fcomi(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
    if (tr_gen_excp_check(pir1)) return true;
#endif

    bool is_zpc_def =
        ir1_is_zf_def(pir1) || ir1_is_pf_def(pir1) || ir1_is_cf_def(pir1);
    bool is_osa_def =
        ir1_is_of_def(pir1) || ir1_is_sf_def(pir1) || ir1_is_af_def(pir1);

    lsassertm(option_lbt, "fcomi not implemented without LBT.\n");

    if (is_zpc_def || is_osa_def) {
        IR2_OPND eflags_temp = ra_alloc_itemp();

        /* calculate OF, SF and AF */
        if (is_osa_def || is_zpc_def) {
            append_ir2_opnd1i(LISA_X86MTFLAG, &zero_ir2_opnd, 0x3f);
        }

        /* calculate ZF, PF, CF */
        if (is_zpc_def) {
            IR2_OPND st0 = ra_alloc_st(0);
            IR2_OPND sti = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);

            append_ir2_opnd2i(LISA_ORI, &eflags_temp, &zero_ir2_opnd, 0xfff);

            IR2_OPND label_for_exit = ir2_opnd_new_label();
            IR2_OPND label_for_not_unordered = ir2_opnd_new_label();
            IR2_OPND label_for_sti_cle_st0 = ir2_opnd_new_label();
            IR2_OPND label_for_sti_ceq_st0 = ir2_opnd_new_label();

            /* First: if unordered, set ZF/PF/CF and exit, else jmp to label_for_not_unordered */
            append_ir2_opnd3i(LISA_FCMP_COND_D, &fcc0_ir2_opnd, &sti, &st0, FCMP_COND_CUN);
            append_ir2_opnd2 (LISA_BCEQZ, &fcc0_ir2_opnd, &label_for_not_unordered);
            append_ir2_opnd1i(LISA_X86MTFLAG, &eflags_temp, 0xb);
            append_ir2_opnd1 (LISA_B, &label_for_exit);

            /* LABEL: label_for_not_unordered, if(st0>=sti) then jmp to label_for_sti_cle_st0 */
            append_ir2_opnd1 (LISA_LABEL, &label_for_not_unordered);
            append_ir2_opnd3i(LISA_FCMP_COND_D, &fcc0_ir2_opnd, &sti, &st0, FCMP_COND_CLE);
            append_ir2_opnd2 (LISA_BCNEZ, &fcc0_ir2_opnd, &label_for_sti_cle_st0);

            /* else if (st0<st1), set CF and exit*/
            append_ir2_opnd1i(LISA_X86MTFLAG, &eflags_temp, 0x1);
            append_ir2_opnd1 (LISA_B, &label_for_exit);

            /* LABEL: label_for_sti_cle_st0 if(st0 == sti), set fcc0 then jmp to label label_for_sti_ceq_st0*/
            append_ir2_opnd1 (LISA_LABEL, &label_for_sti_cle_st0);
            append_ir2_opnd3i(LISA_FCMP_COND_D, &fcc0_ir2_opnd, &sti, &st0, FCMP_COND_CEQ);
            append_ir2_opnd2 (LISA_BCNEZ, &fcc0_ir2_opnd, &label_for_sti_ceq_st0);

            /* else if (st0 > sti), ZF, PF and CF are all clear, just exit directly */
            append_ir2_opnd1(LISA_B, &label_for_exit);

            /* LABEL: label_for_sti_ceq_st0, set ZF if (st0 == sti) and exit*/
            append_ir2_opnd1 (LISA_LABEL, &label_for_sti_ceq_st0);
            append_ir2_opnd1i(LISA_X86MTFLAG, &eflags_temp, 0x8);

            append_ir2_opnd1(LISA_LABEL, &label_for_exit);
        }

        ra_free_temp(&eflags_temp);
    }

    return true;
}

bool translate_fucomi(IR1_INST *pir1)
{
    translate_fcomi(pir1);
    return true;
}

bool translate_fucomip(IR1_INST *pir1)
{
    translate_fcomi(pir1);
    tr_fpu_pop();
    return true;
}

bool translate_fcomip(IR1_INST *pir1)
{
    translate_fcomi(pir1);
    tr_fpu_pop();

    return true;
}

bool translate_fcom(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

#define C0_BIT (8)
#define C1_BIT (9)
#define C2_BIT (10)
#define C3_BIT (14)

    int opnd_num = ir1_get_opnd_num(pir1);

    IR2_OPND src;
    if (opnd_num == 0)
        src = ra_alloc_st(1);
    else
        src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
    IR2_OPND st0 = ra_alloc_st(0);
    IR2_OPND sw_opnd = ra_alloc_itemp();

    IR2_OPND all_one = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ADDI_D, &all_one, &zero_ir2_opnd, -1);

    /* load status_word */
    IR2_OPND mem_opnd;
    ir2_opnd_build_mem(&mem_opnd, ir2_opnd_reg(&env_ir2_opnd),
            lsenv_offset_of_status_word(lsenv));
    append_ir2_opnd2(LISA_LD_HU, &sw_opnd, &mem_opnd);

    /* clear status_word c0 c2 c3 */
    append_ir2_opnd2ii(LISA_BSTRINS_D, &sw_opnd, &zero_ir2_opnd, C0_BIT, C0_BIT);
    append_ir2_opnd2ii(LISA_BSTRINS_D, &sw_opnd, &zero_ir2_opnd, C1_BIT, C1_BIT);
    append_ir2_opnd2ii(LISA_BSTRINS_D, &sw_opnd, &zero_ir2_opnd, C2_BIT, C2_BIT);
    append_ir2_opnd2ii(LISA_BSTRINS_D, &sw_opnd, &zero_ir2_opnd, C3_BIT, C3_BIT);

    IR2_OPND label_for_exit = ir2_opnd_new_label();
    IR2_OPND label_for_not_unordered = ir2_opnd_new_label();
    IR2_OPND label_for_src_cle_st0 = ir2_opnd_new_label();
    IR2_OPND label_for_src_ceq_st0 = ir2_opnd_new_label();

    /* First: if unordered, set C0/C2/C3 and exit, else jmp to label_for_not_unordered */
    append_ir2_opnd3i (LISA_FCMP_COND_D, &fcc0_ir2_opnd, &src, &st0, FCMP_COND_CUN);
    append_ir2_opnd2  (LISA_BCEQZ, &fcc0_ir2_opnd, &label_for_not_unordered);
    append_ir2_opnd2ii(LISA_BSTRINS_D, &sw_opnd, &all_one, C0_BIT, C0_BIT);
    append_ir2_opnd2ii(LISA_BSTRINS_D, &sw_opnd, &all_one, C2_BIT, C2_BIT);
    append_ir2_opnd2ii(LISA_BSTRINS_D, &sw_opnd, &all_one, C3_BIT, C3_BIT);
    append_ir2_opnd1  (LISA_B, &label_for_exit);

    /* LABEL: label_for_not_unordered, if(st0>=src) then jmp to label_for_src_cle_st0 */
    append_ir2_opnd1 (LISA_LABEL, &label_for_not_unordered);
    append_ir2_opnd3i(LISA_FCMP_COND_D, &fcc0_ir2_opnd, &src, &st0, FCMP_COND_CLE);
    append_ir2_opnd2 (LISA_BCNEZ, &fcc0_ir2_opnd, &label_for_src_cle_st0);

    /* else if (st0<st1), set CF and exit*/
    append_ir2_opnd2ii(LISA_BSTRINS_D, &sw_opnd, &all_one, C0_BIT, C0_BIT);
    append_ir2_opnd1  (LISA_B, &label_for_exit);

    /* LABEL: label_for_src_cle_st0 if(st0 == src), set fcc0 then jmp to label label_for_src_ceq_st0*/
    append_ir2_opnd1 (LISA_LABEL, &label_for_src_cle_st0);
    append_ir2_opnd3i(LISA_FCMP_COND_D, &fcc0_ir2_opnd, &src, &st0, FCMP_COND_CEQ);
    append_ir2_opnd2 (LISA_BCNEZ, &fcc0_ir2_opnd, &label_for_src_ceq_st0);

    /* else if (st0 > sti), ZF, PF and CF are all clear, just exit directly */
    append_ir2_opnd1 (LISA_B, &label_for_exit);

    /* LABEL: label_for_src_ceq_st0, set ZF if (st0 == src) and exit*/
    append_ir2_opnd1  (LISA_LABEL, &label_for_src_ceq_st0);
    append_ir2_opnd2ii(LISA_BSTRINS_D, &sw_opnd, &all_one, C3_BIT, C3_BIT);
    ra_free_temp(&all_one);

    append_ir2_opnd1(LISA_LABEL, &label_for_exit);

    append_ir2_opnd2(LISA_ST_H, &sw_opnd, &mem_opnd);
    ra_free_temp(&sw_opnd);

    return true;
}

bool translate_fcomp(IR1_INST *pir1)
{
    translate_fcom(pir1);
    tr_fpu_pop();

    return true;
}

bool translate_fucom(IR1_INST *pir1)
{
    translate_fcom(pir1);
    return true;
}

bool translate_fucomp(IR1_INST *pir1)
{
    translate_fcom(pir1);
    tr_fpu_pop();

    return true;
}

bool translate_fcompp(IR1_INST *pir1)
{
    translate_fcom(pir1);
    tr_fpu_pop();
    tr_fpu_pop();

    return true;
}

bool translate_fucompp(IR1_INST *pir1)
{
    translate_fcom(pir1);
    tr_fpu_pop();
    tr_fpu_pop();

    return true;
}

bool translate_ficom(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
    if (tr_gen_excp_check(pir1)) return true;
#endif
    lsassertm(0, "ficom to be implemented in LoongArch.\n");
//    IR2_OPND st0_opnd = ra_alloc_st(0);
//    IR2_OPND mint_opnd = ra_alloc_ftemp();
//    load_freg_from_ir1_2(&mint_opnd, ir1_get_opnd(pir1, 0), false, false);
//    append_ir2_opnd2(mips_cvt_d_l, &mint_opnd, &mint_opnd);
//    IR2_OPND sw_opnd = ra_alloc_itemp();
//
//    /* load status_word */
//    IR2_OPND mem_opnd = ir2_opnd_new2(IR2_OPND_MEM, ir2_opnd_base_reg_num(&env_ir2_opnd),
//                                      lsenv_offset_of_status_word(lsenv));
//    append_ir2_opnd2(mips_lhu, &sw_opnd, &mem_opnd);
//
//    /* clear status_word c0 c2 c3 */
//    append_ir2_opnd2i(mips_andi, &sw_opnd, &sw_opnd, 0xbaff);
//
//    IR2_OPND label_un = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_eq = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_lt = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    /* check is unordered */
//    append_ir2_opnd2(mips_c_un_d, &st0_opnd, &mint_opnd);
//    append_ir2_opnd1(mips_bc1t, &label_un);
//    /* check is equal */
//    append_ir2_opnd2(mips_c_eq_d, &st0_opnd, &mint_opnd);
//    append_ir2_opnd1(mips_bc1t, &label_eq);
//    /* check is less than */
//    append_ir2_opnd2(mips_c_lt_d, &st0_opnd, &mint_opnd);
//    append_ir2_opnd1(mips_bc1t, &label_lt);
//    append_ir2_opnd1(mips_b, &label_exit);
//    /* lt: */
//    append_ir2_opnd1(mips_label, &label_lt);
//    append_ir2_opnd2i(mips_ori, &sw_opnd, &sw_opnd, 0x0100);
//    append_ir2_opnd1(mips_b, &label_exit);
//    /* eq: */
//    append_ir2_opnd1(mips_label, &label_eq);
//    append_ir2_opnd2i(mips_ori, &sw_opnd, &sw_opnd, 0x4000);
//    append_ir2_opnd1(mips_b, &label_exit);
//    /* un: */
//    append_ir2_opnd1(mips_label, &label_un);
//    append_ir2_opnd2i(mips_ori, &sw_opnd, &sw_opnd, 0x4500);
//    /* exit: */
//    append_ir2_opnd1(mips_label, &label_exit);
//    /* append_ir2_opnd2i(mips_sh, &sw_opnd, &mem_opnd); */
//    ra_free_temp(&sw_opnd);
//    ra_free_temp(&mint_opnd);

    return true;
}

bool translate_ficomp(IR1_INST *pir1)
{
    translate_ficom(pir1);
    tr_fpu_pop();
    return true;
}

/* TODO: */
bool translate_ud2(IR1_INST *pir1)
{
    /* lsassert(lsenv->tr_data->static_translation); */
    return true;
}

#ifndef CONFIG_SOFTMMU
/* user-mode ffree */
bool translate_ffree(IR1_INST *pir1)
{
    lsassertm(0, "ffree to be implemented in LoongArch.\n");
//    /* 根据 status_word 的 TOP bits 处理 tag word: mark the ST(i) register as */
//    /* empty */
//    /* TODO:r */
//    IR2_OPND value_tag = ra_alloc_itemp();
//    IR2_OPND value_status = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_lh, &value_tag, &env_ir2_opnd,
//                      lsenv_offset_of_tag_word(lsenv)); /* tag_word */
//    append_ir2_opnd2i(mips_lh, &value_status, &env_ir2_opnd,
//                      lsenv_offset_of_status_word(lsenv)); /* status_word */
//
//    append_ir2_opnd2i(mips_sra, &value_status, &value_status, 11);
//    append_ir2_opnd2i(mips_andi, &value_status, &value_status, 0x7ULL);
//    append_ir2_opnd2i(mips_sll, &value_status, &value_status, 1);
//    IR2_OPND temp = ra_alloc_itemp();
//    append_ir2_opnd3(mips_xor, &temp, &temp, &temp);
//    append_ir2_opnd2i(mips_ori, &temp, &temp, 0x3ULL);
//    append_ir2_opnd3(mips_sllv, &temp, &temp, &value_status);
//    append_ir2_opnd3(mips_or, &value_tag, &value_tag, &temp);
//    append_ir2_opnd2i(mips_sh, &value_tag, &env_ir2_opnd,
//                      lsenv_offset_of_tag_word(lsenv)); /* tag_word */
    return true;
}
#endif

#ifndef CONFIG_SOFTMMU
/* fldenv in user-mode */
bool translate_fldenv(IR1_INST *pir1)
{
    lsassertm(0, "fldenv to be implemented in LoongArch.\n");
//    IR2_OPND value = ra_alloc_itemp();
//    IR2_OPND mem_opnd;
//    convert_mem_opnd(&mem_opnd, ir1_get_opnd(pir1, 0), -1);
//
//    if (cpu_get_guest_base() != 0) {
//        IR2_OPND gbase = ra_alloc_guest_base();
//        IR2_OPND tmp = ra_alloc_itemp();
//        IR2_OPND mem = ir2_opnd_mem_get_base_ireg(&mem_opnd);
//#ifdef N64
//        append_ir2_opnd3(mips_daddu, &tmp, &mem, &gbase);
//#else
//        append_ir2_opnd3(mips_addu, &tmp, &mem, &gbase);
//#endif
//        ir2_opnd_mem_set_base_ireg(&mem_opnd, &tmp);
//        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
//    }
//
//    append_ir2_opnd2(mips_lh, &value, &mem_opnd);
//    append_ir2_opnd2i(mips_sh, &value, &env_ir2_opnd,
//                      lsenv_offset_of_control_word(lsenv));
//
//    assert(mem_opnd._imm16 + 24 <= 32767);
//    IR2_OPND mem_opnd_plus_4 = ir2_opnd_new2(IR2_OPND_MEM,
//            ir2_opnd_base_reg_num(&mem_opnd),
//            ir2_opnd_imm(&mem_opnd) + 4);
//    append_ir2_opnd2(mips_lh, &value, &mem_opnd_plus_4);
//    append_ir2_opnd2i(mips_sh, &value, &env_ir2_opnd,
//                      lsenv_offset_of_status_word(lsenv));
//
//    /* get top value */
//    append_ir2_opnd2i(mips_srl, &value, &value, 11);
//    append_ir2_opnd2i(mips_andi, &value, &value, 7);
//    /* set fpstt */
//    append_ir2_opnd2i(mips_sh, &value, &env_ir2_opnd,
//                      lsenv_offset_of_top(lsenv));
//    IR2_OPND top = ra_alloc_itemp();
//    tr_load_lstop_from_env(&top); 
//    ra_free_temp(&top);
//
//    //tr_gen_call_to_helper1((ADDR)update_fp_status);
//
//    IR2_OPND mem_opnd_plus_8 = ir2_opnd_new2(IR2_OPND_MEM,
//            ir2_opnd_base_reg_num(&mem_opnd),
//            ir2_opnd_imm(&mem_opnd) + 8);
//    append_ir2_opnd2(mips_lh, &value, &mem_opnd_plus_8);
//
//    /* dispose tag word */
//    IR2_OPND temp_1 = ra_alloc_itemp();
//    append_ir2_opnd2(mips_mov32_sx, &temp_1, &value);
//    append_ir2_opnd2i(mips_sll, &temp_1, &temp_1, 1);
//    append_ir2_opnd3(mips_and, &value, &value, &temp_1);
//    append_ir2_opnd2i(mips_ori, &value, &value, 0x5555ULL);
//    append_ir2_opnd2i(mips_sh, &value, &env_ir2_opnd,
//                      lsenv_offset_of_tag_word(lsenv));
//
//    /* append_ir2_opnd2(mips_lw, &value, &ir2_opnd_new2(IR2_OPND_MEM, &*/
//    /* mem_opnd->_reg_num, &mem_opnd->_imm16+12)); append_ir2_opnd2i(mips_sw, &*/
//    /* value, env_ir2_opnd, lsenv_offset_of_instruction_pointer_offset(lsenv));
//     */
//    /* // */
//    /* */
//    /* append_ir2_opnd2(mips_lw, &value, &ir2_opnd_new2(IR2_OPND_MEM, &*/
//    /* mem_opnd->_reg_num, &mem_opnd->_imm16+16)); append_ir2_opnd2i(mips_sw, &*/
//    /* value, env_ir2_opnd, lsenv_offset_of_selector_opcode(lsenv)); // */
//    ra_free_temp(&value);
    return true;
}
#endif

#ifndef CONFIG_SOFTMMU
/* fnstenv in user-mode */
bool translate_fnstenv(IR1_INST *pir1)
{
    lsassertm(0, "fnstenv to be implemented in LoongArch.\n");
//    IR2_OPND value = ra_alloc_itemp();
//    IR2_OPND temp = ra_alloc_itemp();
//    IR2_OPND temp1 = ra_alloc_itemp();
//    load_imm32_to_ir2(&temp, 0xffff0000ULL, UNKNOWN_EXTENSION);
//    IR2_OPND mem_opnd;
//    convert_mem_opnd(&mem_opnd, ir1_get_opnd(pir1, 0), -1);
//
//    if (cpu_get_guest_base() != 0) {
//        IR2_OPND gbase = ra_alloc_guest_base();
//        IR2_OPND tmp = ra_alloc_itemp();
//        IR2_OPND mem = ir2_opnd_mem_get_base_ireg(&mem_opnd);
//#ifdef N64
//        append_ir2_opnd3(mips_daddu, &tmp, &mem, &gbase);
//#else
//        append_ir2_opnd3(mips_addu, &tmp, &mem, &gbase);
//#endif
//        ir2_opnd_mem_set_base_ireg(&mem_opnd, &tmp);
//        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
//    }
//
//    append_ir2_opnd2i(mips_lh, &value, &env_ir2_opnd,
//                      lsenv_offset_of_control_word(lsenv));
//    append_ir2_opnd3(mips_or, &value, &temp, &value);
//    append_ir2_opnd2(mips_sw, &value, &mem_opnd);
//
//    assert(mem_opnd._imm16 + 24 <= 32767);
//
//    append_ir2_opnd2i(mips_lh, &value, &env_ir2_opnd,
//                      lsenv_offset_of_status_word(lsenv));
//    /* update status world */
//    if (option_lsfpu) {
//        append_ir2_opnd1(mips_mftop, &temp1);
//        append_ir2_opnd2i(mips_sll, &temp1, &temp1, 11);
//        append_ir2_opnd2i(mips_andi, &value, &value, 0xc7ffULL);
//        append_ir2_opnd3(mips_or, &value, &value, &temp1);
//        append_ir2_opnd2i(mips_sh, &value, &env_ir2_opnd,
//                          lsenv_offset_of_status_word(lsenv));
//    }
//    append_ir2_opnd3(mips_or, &value, &temp, &value);
//
//    IR2_OPND mem_opnd_plus_4 = ir2_opnd_new2(IR2_OPND_MEM,
//            ir2_opnd_base_reg_num(&mem_opnd),
//            ir2_opnd_imm(&mem_opnd) + 4);
//    append_ir2_opnd2(mips_sw, &value, &mem_opnd_plus_4);
//
//    append_ir2_opnd2i(mips_lh, &value, &env_ir2_opnd,
//                      lsenv_offset_of_tag_word(lsenv));
//    append_ir2_opnd3(mips_or, &value, &temp, &value);
//
//    IR2_OPND mem_opnd_plus_8 = ir2_opnd_new2(IR2_OPND_MEM,
//            ir2_opnd_base_reg_num(&mem_opnd),
//            ir2_opnd_imm(&mem_opnd) + 8);
//    append_ir2_opnd2(mips_sw, &value, &mem_opnd_plus_8);
//
//    /* append_ir2_opnd2i(mips_lw, &value, &env_ir2_opnd, */
//    /* lsenv_offset_of_instruction_pointer_offset()); // append_ir2(mips_sw, */
//    /* value, ir2_opnd_new2(IR2_OPND_MEM, mem_opnd->_reg_num, */
//    /* mem_opnd->_imm16+12)); */
//    /* */
//    /* append_ir2(mips_lw, value, env_ir2_opnd, */
//    /* env->offset_of_selector_opcode()); // append_ir2(mips_sw, value, */
//    /* ir2_opnd_new2(IR2_OPND_MEM, mem_opnd->_reg_num, mem_opnd->_imm16+16)); */
//
//    IR2_OPND mem_opnd_plus_24 = ir2_opnd_new2(IR2_OPND_MEM,
//            ir2_opnd_base_reg_num(&mem_opnd),
//            ir2_opnd_imm(&mem_opnd) + 24);
//    append_ir2_opnd2(mips_sw, &temp, &mem_opnd_plus_24);
//
//    ra_free_temp(&value);
    return true;
}
#endif

bool translate_fnclex(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;

    /* env->fpus &= 0x7f00 */

    IR2_OPND sw_opnd = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_LD_HU, &sw_opnd, &env_ir2_opnd,
            lsenv_offset_of_status_word(lsenv));

    IR2_OPND mask = ra_alloc_itemp();
    load_imm32_to_ir2(&mask, 0x7f00, ZERO_EXTENSION);

    append_ir2_opnd3(LISA_AND, &sw_opnd, &sw_opnd, &mask);

    append_ir2_opnd2i(LISA_ST_H, &sw_opnd, &env_ir2_opnd,
            lsenv_offset_of_status_word(lsenv));

    ra_free_temp(&sw_opnd);
    ra_free_temp(&mask);
#endif
    return true;
}

bool translate_finit(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif
    tr_gen_call_to_helper1((ADDR)helper_fninit, 1);

    tr_gen_top_mode_init();

    return true;
}

#ifndef CONFIG_SOFTMMU
/* fninit in user-mode */
bool translate_fninit(IR1_INST *pir1)
{
    lsassertm(0, "fninit to be implemented in LoongArch.\n");
//    IR2_OPND temp = ra_alloc_itemp();
//    load_imm32_to_ir2(&temp, 0xffff037fULL, UNKNOWN_EXTENSION);
//    append_ir2_opnd2i(mips_sw, &temp, &env_ir2_opnd,
//                      lsenv_offset_of_control_word(lsenv));
//    load_imm32_to_ir2(&temp, 0x0000ffffULL, UNKNOWN_EXTENSION);
//    append_ir2_opnd2i(mips_sw, &temp, &env_ir2_opnd,
//                      lsenv_offset_of_tag_word(lsenv));
    return true;
}
#endif
