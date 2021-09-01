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
#include <assert.h>

#include <string.h>

void tr_fpu_push(void) { tr_fpu_dec(); }
void tr_fpu_pop(void) { tr_fpu_inc(); }

void tr_fpu_inc(void)
{
    if (option_lsfpu) {
        lsassertm(0, "lsfpu to be implemented in LoongArch\n");
        /*append_ir2_opnd0(mips_inctop);*/
    } else {
        lsenv->tr_data->curr_top++;
        lsenv->tr_data->curr_top &= 7;
#ifdef CONFIG_SOFTMMU
        lsenv->tr_data->is_top_saved = 0;
#endif
    }
}

void tr_fpu_dec(void)
{
    if (option_lsfpu) {
        lsassertm(0, "lsfpu to be implemented in LoongArch\n");
        /*append_ir2_opnd0(mips_dectop);*/
    } else {
        lsenv->tr_data->curr_top--;
        lsenv->tr_data->curr_top &= 7;
#ifdef CONFIG_SOFTMMU
        lsenv->tr_data->is_top_saved = 0;
#endif
    }
}

void td_fpu_set_top(int ctop)
{
    if (option_lsfpu) lsassert(0);
    lsenv->tr_data->curr_top = ctop & 0x7;
#ifdef CONFIG_SOFTMMU
    lsenv->tr_data->is_top_saved = 0;
#endif
}

int td_fpu_get_top(void)
{
    if (option_lsfpu) lsassert(0);
    return lsenv->tr_data->curr_top & 0x7;
}

void tr_gen_save_curr_top(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
#ifdef CONFIG_SOFTMMU
#ifdef CONFIG_XTM_FAST_CS
    TranslationBlock *tb = td->curr_tb;
    if (tb) {
        /* Under fast context switch, if current TB
         * does not use FPU, saving current_top has
         * no meaning and will ruin env->fpstt */
        int fast_cs_mask = tb->extra_tb->fast_cs_mask;
        if (!(fast_cs_mask & XTM_FAST_CS_MASK_FPU))
            return;
    }
#endif
    if (td->is_top_saved) return;
#endif
    int ctop = td->curr_top;
    if (!option_lsfpu) {
        if (ctop) {
            IR2_OPND top = ra_alloc_itemp();
            load_imm32_to_ir2(&top, ctop, UNKNOWN_EXTENSION);
            append_ir2_opnd2i(LISA_ST_W, &top,
                    &env_ir2_opnd, lsenv_offset_of_top(lsenv));
            ra_free_temp(&top);
        } else {
            append_ir2_opnd2i(LISA_ST_W, &zero_ir2_opnd,
                    &env_ir2_opnd, lsenv_offset_of_top(lsenv));
        }
    } else {
        IR2_OPND top = ra_alloc_itemp();
        tr_save_lstop_to_env(&top);
        ra_free_temp(&top);
    }
}

/* native fpu rotate in tr_static_code.c */

void rotate_fpu_to_bias(int bias)
{
    int step = bias - lsenv_get_top_bias(lsenv);
    rotate_fpu_by(step);
}

void rotate_fpu_to_top(int top)
{
    int step = lsenv_get_top(lsenv) - top;
    rotate_fpu_by(step);
}

void rotate_fpu_by(int step)
{
    assert(step >= -7 && step <= 7);
    assert(step != 0);
    lsenv_set_top(lsenv, (lsenv_get_top(lsenv) - step) & 7);
#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
    xtm_pf_inc_fpu_rotate_bt();
#endif
    switch (step) {
    case 1:
    case -7: {
        FPReg ftemp0;
        ftemp0 = lsenv_get_fpregs(lsenv, 0);
        lsenv_set_fpregs(lsenv, 0, lsenv_get_fpregs(lsenv, 1));
        lsenv_set_fpregs(lsenv, 1, lsenv_get_fpregs(lsenv, 2));
        lsenv_set_fpregs(lsenv, 2, lsenv_get_fpregs(lsenv, 3));
        lsenv_set_fpregs(lsenv, 3, lsenv_get_fpregs(lsenv, 4));
        lsenv_set_fpregs(lsenv, 4, lsenv_get_fpregs(lsenv, 5));
        lsenv_set_fpregs(lsenv, 5, lsenv_get_fpregs(lsenv, 6));
        lsenv_set_fpregs(lsenv, 6, lsenv_get_fpregs(lsenv, 7));
        lsenv_set_fpregs(lsenv, 7, ftemp0);
        lsenv_set_top_bias(lsenv, (lsenv_get_top_bias(lsenv) + 1) & 7);
    } break;
    case 2:
    case -6: {
        FPReg ftemp0, ftemp1;
        ftemp0 = lsenv_get_fpregs(lsenv, 0);
        ftemp1 = lsenv_get_fpregs(lsenv, 1);
        lsenv_set_fpregs(lsenv, 0, lsenv_get_fpregs(lsenv, 2));
        lsenv_set_fpregs(lsenv, 1, lsenv_get_fpregs(lsenv, 3));
        lsenv_set_fpregs(lsenv, 2, lsenv_get_fpregs(lsenv, 4));
        lsenv_set_fpregs(lsenv, 3, lsenv_get_fpregs(lsenv, 5));
        lsenv_set_fpregs(lsenv, 4, lsenv_get_fpregs(lsenv, 6));
        lsenv_set_fpregs(lsenv, 5, lsenv_get_fpregs(lsenv, 7));
        lsenv_set_fpregs(lsenv, 6, ftemp0);
        lsenv_set_fpregs(lsenv, 7, ftemp1);
        lsenv_set_top_bias(lsenv, (lsenv_get_top_bias(lsenv) + 2) & 7);
    } break;
    case 3:
    case -5: {
        FPReg ftemp0, ftemp1, ftemp2;
        ftemp0 = lsenv_get_fpregs(lsenv, 0);
        ftemp1 = lsenv_get_fpregs(lsenv, 1);
        ftemp2 = lsenv_get_fpregs(lsenv, 2);
        lsenv_set_fpregs(lsenv, 0, lsenv_get_fpregs(lsenv, 3));
        lsenv_set_fpregs(lsenv, 1, lsenv_get_fpregs(lsenv, 4));
        lsenv_set_fpregs(lsenv, 2, lsenv_get_fpregs(lsenv, 5));
        lsenv_set_fpregs(lsenv, 3, lsenv_get_fpregs(lsenv, 6));
        lsenv_set_fpregs(lsenv, 4, lsenv_get_fpregs(lsenv, 7));
        lsenv_set_fpregs(lsenv, 5, ftemp0);
        lsenv_set_fpregs(lsenv, 6, ftemp1);
        lsenv_set_fpregs(lsenv, 7, ftemp2);
        lsenv_set_top_bias(lsenv, (lsenv_get_top_bias(lsenv) + 3) & 7);
    } break;
    case 4:
    case -4: {
        FPReg ftemp0, ftemp1, ftemp2, ftemp3;
        ftemp0 = lsenv_get_fpregs(lsenv, 0);
        ftemp1 = lsenv_get_fpregs(lsenv, 1);
        ftemp2 = lsenv_get_fpregs(lsenv, 2);
        ftemp3 = lsenv_get_fpregs(lsenv, 3);
        lsenv_set_fpregs(lsenv, 0, lsenv_get_fpregs(lsenv, 4));
        lsenv_set_fpregs(lsenv, 1, lsenv_get_fpregs(lsenv, 5));
        lsenv_set_fpregs(lsenv, 2, lsenv_get_fpregs(lsenv, 6));
        lsenv_set_fpregs(lsenv, 3, lsenv_get_fpregs(lsenv, 7));
        lsenv_set_fpregs(lsenv, 4, ftemp0);
        lsenv_set_fpregs(lsenv, 5, ftemp1);
        lsenv_set_fpregs(lsenv, 6, ftemp2);
        lsenv_set_fpregs(lsenv, 7, ftemp3);
        lsenv_set_top_bias(lsenv, (lsenv_get_top_bias(lsenv) + 4) & 7);
    } break;
    case 5:
    case -3: {
        FPReg ftemp0, ftemp1, ftemp2, ftemp3, ftemp4;
        ftemp0 = lsenv_get_fpregs(lsenv, 0);
        ftemp1 = lsenv_get_fpregs(lsenv, 1);
        ftemp2 = lsenv_get_fpregs(lsenv, 2);
        ftemp3 = lsenv_get_fpregs(lsenv, 3);
        ftemp4 = lsenv_get_fpregs(lsenv, 4);
        lsenv_set_fpregs(lsenv, 0, lsenv_get_fpregs(lsenv, 5));
        lsenv_set_fpregs(lsenv, 1, lsenv_get_fpregs(lsenv, 6));
        lsenv_set_fpregs(lsenv, 2, lsenv_get_fpregs(lsenv, 7));
        lsenv_set_fpregs(lsenv, 3, ftemp0);
        lsenv_set_fpregs(lsenv, 4, ftemp1);
        lsenv_set_fpregs(lsenv, 5, ftemp2);
        lsenv_set_fpregs(lsenv, 6, ftemp3);
        lsenv_set_fpregs(lsenv, 7, ftemp4);
        lsenv_set_top_bias(lsenv, (lsenv_get_top_bias(lsenv) + 5) & 7);
    } break;
    case 6:
    case -2: {
        FPReg ftemp0, ftemp1, ftemp2, ftemp3, ftemp4, ftemp5;
        ftemp0 = lsenv_get_fpregs(lsenv, 0);
        ftemp1 = lsenv_get_fpregs(lsenv, 1);
        ftemp2 = lsenv_get_fpregs(lsenv, 2);
        ftemp3 = lsenv_get_fpregs(lsenv, 3);
        ftemp4 = lsenv_get_fpregs(lsenv, 4);
        ftemp5 = lsenv_get_fpregs(lsenv, 5);
        lsenv_set_fpregs(lsenv, 0, lsenv_get_fpregs(lsenv, 6));
        lsenv_set_fpregs(lsenv, 1, lsenv_get_fpregs(lsenv, 7));
        lsenv_set_fpregs(lsenv, 2, ftemp0);
        lsenv_set_fpregs(lsenv, 3, ftemp1);
        lsenv_set_fpregs(lsenv, 4, ftemp2);
        lsenv_set_fpregs(lsenv, 5, ftemp3);
        lsenv_set_fpregs(lsenv, 6, ftemp4);
        lsenv_set_fpregs(lsenv, 7, ftemp5);
        lsenv_set_top_bias(lsenv, (lsenv_get_top_bias(lsenv) + 6) & 7);
    } break;
    case 7:
    case -1: {
        FPReg ftemp0, ftemp1, ftemp2, ftemp3, ftemp4, ftemp5, ftemp6;
        ftemp0 = lsenv_get_fpregs(lsenv, 0);
        ftemp1 = lsenv_get_fpregs(lsenv, 1);
        ftemp2 = lsenv_get_fpregs(lsenv, 2);
        ftemp3 = lsenv_get_fpregs(lsenv, 3);
        ftemp4 = lsenv_get_fpregs(lsenv, 4);
        ftemp5 = lsenv_get_fpregs(lsenv, 5);
        ftemp6 = lsenv_get_fpregs(lsenv, 6);
        lsenv_set_fpregs(lsenv, 0, lsenv_get_fpregs(lsenv, 7));
        lsenv_set_fpregs(lsenv, 1, ftemp0);
        lsenv_set_fpregs(lsenv, 2, ftemp1);
        lsenv_set_fpregs(lsenv, 3, ftemp2);
        lsenv_set_fpregs(lsenv, 4, ftemp3);
        lsenv_set_fpregs(lsenv, 5, ftemp4);
        lsenv_set_fpregs(lsenv, 6, ftemp5);
        lsenv_set_fpregs(lsenv, 7, ftemp6);
        lsenv_set_top_bias(lsenv, (lsenv_get_top_bias(lsenv) + 7) & 7);
    } break;
    }
}

void convert_fpregs_64_to_x80(void)
{
    int i;
    CPUX86State *env = (CPUX86State*)lsenv->cpu_state;
    float_status s = env->fp_status;
    for (i = 0; i < 8; i++) {
        FPReg *p = &(env->fpregs[i]);
        p->d = float64_to_floatx80((float64)p->d.low, &s);
    }
}

void convert_fpregs_x80_to_64(void)
{
    int i;
    CPUX86State *env = (CPUX86State*)lsenv->cpu_state;
    float_status s = env->fp_status;
    for (i = 0; i < 8; i++) {
        FPReg *p = &(env->fpregs[i]);
        p->d.low = (uint64_t)floatx80_to_float64(p->d, &s);
        p->d.high = 0;
    }
}

/* save old fcsr in fcsr_opnd temporary register  for reload , then set fcsr
 * according to x86 MXCSR register */

IR2_OPND set_fpu_fcsr_rounding_field_by_x86(void)
{
    lsassertm(0, "fpu fcsr round to be implemented in LoongArch.\n");
    /*IR2_OPND temp_fcsr = ra_alloc_itemp_internal();*/
    /*append_ir2_opnd2(mips_cfc1, &temp_fcsr, &fcsr_ir2_opnd);*/

    /* save fscr in fcsr_opnd for reload */
    IR2_OPND fcsr_opnd = ra_alloc_itemp_internal();
    /*append_ir2_opnd3(mips_or, &fcsr_opnd, &temp_fcsr, &zero_ir2_opnd);*/

    /* set fcsr according to x86 MXCSR register */
    /*IR2_OPND temp_mxcsr = ra_alloc_itemp_internal();
    IR2_OPND mem_opnd = ir2_opnd_new2(
        IR2_OPND_MEM, ir2_opnd_base_reg_num(&env_ir2_opnd),
        lsenv_offset_of_mxcsr(lsenv));
    append_ir2_opnd2(mips_lwu, &temp_mxcsr, &mem_opnd);
    append_ir2_opnd2i(mips_dsrl, &temp_mxcsr, &temp_mxcsr, 0x13);
    append_ir2_opnd2i(mips_andi, &temp_mxcsr, &temp_mxcsr, 0x3);
    IR2_OPND temp_int = ra_alloc_itemp_internal();
    append_ir2_opnd3(mips_daddu, &temp_int, &temp_mxcsr, &zero_ir2_opnd);
    append_ir2_opnd2i(mips_andi, &temp_int, &temp_int, 0x1);
    IR2_OPND label1 = ir2_opnd_new_type(IR2_OPND_LABEL);
    append_ir2_opnd2(mips_blez, &temp_int, &label1);
    append_ir2_opnd2i(mips_daddiu, &temp_mxcsr, &temp_mxcsr, 0x2);
    append_ir2_opnd2i(mips_andi, &temp_mxcsr, &temp_mxcsr, 0x3);
    append_ir2_opnd1(mips_label, &label1);
    append_ir2_opnd2i(mips_dsrl, &temp_fcsr, &temp_fcsr, 0x2);
    append_ir2_opnd2i(mips_dsll, &temp_fcsr, &temp_fcsr, 0x2);
    append_ir2_opnd3(mips_or, &temp_fcsr, &temp_fcsr, &temp_mxcsr);
    append_ir2_opnd2(mips_ctc1, &temp_fcsr, &fcsr_ir2_opnd);
    ra_free_temp(&temp_fcsr);
    ra_free_temp(&temp_mxcsr);
    ra_free_temp(&temp_int);*/
    return fcsr_opnd;
}

/* reload fcsr according to previously saved fcsr */

void set_fpu_fcsr(IR2_OPND *new_fcsr)
{
    lsassertm(0, "fpu fcsr round to be implemented in LoongArch.\n");
    /*append_ir2_opnd2(mips_ctc1, new_fcsr, &fcsr_ir2_opnd);*/
}

#ifdef CONFIG_SOFTMMU

void xtm_fpu_fix_before_exec_tb(
        CPUX86State *env,
        TranslationBlock *tb)
{
#ifdef CONFIG_XTM_FAST_CS
    uint8_t fast_cs_mask = tb->extra_tb->fast_cs_mask;
    /* Only need to roate FPU when this TB does use FPU */
    if (fast_cs_mask & XTM_FAST_CS_MASK_FPU) {
        if (env->fpstt != etb_get_top_in(tb->extra_tb)) {
            rotate_fpu_to_top(etb_get_top_in(tb->extra_tb));
        }
    }
#else
    if (env->fpstt != etb_get_top_in(tb->extra_tb)) {
        rotate_fpu_to_top(etb_get_top_in(tb->extra_tb));
    }
#endif
}

void xtm_fpu_fix_after_exec_tb(
        CPUX86State *env,
        TranslationBlock *tb,
        TranslationBlock *last_tb)
{
    int fpu_need_fix_in_sys = 1;

#ifdef CONFIG_XTM_FAST_CS
    /* with fast-cs, only fix FPU when needed */
    int fast_cs_mask = lsenv_get_fast_cs_mask(lsenv);
    if (fast_cs_mask & XTM_FAST_CS_MASK_FPU) {
        fpu_need_fix_in_sys = 1;
    } else {
        fpu_need_fix_in_sys = 0;
    }
#endif

    if (fpu_need_fix_in_sys) {
        int xtm_fpu = env->xtm_fpu;
        /* 1. rotate FPU top */
        if (lsenv_get_top_bias(lsenv) != 0) {
            rotate_fpu_to_bias(0);
        }
        /* 2. fix TOP when loaded/reseted FPU top */
        if (xtm_fpu_need_fix(xtm_fpu)) {
            if (xtm_fpu & XTM_FPU_LOAD_ST_MASK) { /* fldenv, ... */
                env->fpstt = xtm_fpu_get_top(xtm_fpu);
                lsenv_set_top_bias(lsenv, 0);
            }
            else { /* XTM_FPU_RESET_ST: fninit, ... */
                env->fpstt = 0;
            }
        }
        /* 3. clean env->xtm_fpu */
        env->xtm_fpu = XTM_FPU_RESET_VALUE;
    }
}

void xtm_fpu_fix_cpu_loop_exit(void)
{
    int fpu_need_fix_in_sys = 1;

#ifdef CONFIG_XTM_FAST_CS
    /* with fast-cs, only fix FPU when needed */
    int fast_cs_mask = lsenv_get_fast_cs_mask(lsenv);
    if (fast_cs_mask & XTM_FAST_CS_MASK_FPU) {
        fpu_need_fix_in_sys = 1;
    } else {
        fpu_need_fix_in_sys = 0;
    }
#endif

    /* FPU Top fix in system-mode:
     *
     *    If exception occur, the context switch will save
     *    td->curr_top into env->fpstt, which is the top value
     *    after rotating.
     *
     *    But we have not reach the end of TB, so the final
     *    top value(rotated) might not be TB->top_out.
     *
     *    So we only need to keep the env->fpstt here and
     *    rotate it back.
     */
    if (fpu_need_fix_in_sys &&
        lsenv_get_top_bias(lsenv) != 0) {
        rotate_fpu_to_bias(0);
    }
}

#else /* user-mode fpu fix */

void xtm_fpu_fix_before_exec_tb(
        CPUX86State *env,
        TranslationBlock *tb)
{
    if (env->fpstt != etb_get_top_in(tb->extra_tb)) {
        rotate_fpu_to_top(etb_get_top_in(tb->extra_tb));
    }
}

void xtm_fpu_fix_after_exec_tb(
        CPUX86State *env,
        TranslationBlock *tb,
        TranslationBlock *last_tb)
{
    ETB *etb;
    if (last_tb) {
        etb = last_tb->extra_tb;
    } else {
        etb = tb->extra_tb;
    }
    env->fpstt = etb->_top_out;
    if (lsenv_get_top_bias(lsenv) != 0) {
        rotate_fpu_to_bias(0);
    }
}

void xtm_fpu_fix_cpu_loop_exit(void) {}

#endif /* COFNIG_SOFTMMU */
