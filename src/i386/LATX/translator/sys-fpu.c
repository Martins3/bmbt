#include "common.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "translate.h"
#include "latx-options.h"

void latxs_tr_fpu_init(TRANSLATION_DATA *td, TranslationBlock *tb)
{
    if (option_lsfpu || option_soft_fpu) {
        return;
    }

    if (tb != NULL) {
        latxs_td_fpu_set_top(tb->_top_in);
        lsassert(td->curr_top != -1);
    } else {
        latxs_td_fpu_set_top(0);
    }
}

void latxs_td_fpu_set_top(int ctop)
{
    if (option_lsfpu || option_soft_fpu) {
        lsassert(0);
    }

    TRANSLATION_DATA *td = lsenv->tr_data;
    lsassert(td);

    td->curr_top = ctop & 0x7;
}

int latxs_td_fpu_get_top(void)
{
    if (option_lsfpu || option_soft_fpu) {
        lsassert(0);
    }

    TRANSLATION_DATA *td = lsenv->tr_data;
    lsassert(td);

    return td->curr_top & 0x7;
}

void latxs_tr_fpu_push(void) { latxs_tr_fpu_dec(); }
void latxs_tr_fpu_pop(void) { latxs_tr_fpu_inc(); }

void latxs_tr_fpu_inc(void)
{
    if (option_soft_fpu) {
        lsassert(0);
    }

    if (option_lsfpu) {
        latxs_append_ir2_opnd0(LISA_X86INCTOP);
    } else {
        lsenv->tr_data->curr_top++;
        lsenv->tr_data->curr_top &= 7;
    }
}

void latxs_tr_fpu_dec(void)
{
    if (option_soft_fpu) {
        lsassert(0);
    }

    if (option_lsfpu) {
        latxs_append_ir2_opnd0(LISA_X86DECTOP);
    } else {
        lsenv->tr_data->curr_top--;
        lsenv->tr_data->curr_top &= 7;
    }
}

void latxs_tr_gen_save_curr_top(void)
{
    if (option_soft_fpu) {
        lsassert(0);
    }

    TRANSLATION_DATA *td = lsenv->tr_data;

    int ctop = td->curr_top;

    if (!option_lsfpu) {
        if (ctop) {
            IR2_OPND top = latxs_ra_alloc_itemp();
            latxs_load_imm32_to_ir2(&top, ctop, EXMode_N);
            latxs_append_ir2_opnd2i(LISA_ST_W, &top,
                    &latxs_env_ir2_opnd, lsenv_offset_of_top(lsenv));
            latxs_ra_free_temp(&top);
        } else {
            latxs_append_ir2_opnd2i(LISA_ST_W, &latxs_zero_ir2_opnd,
                    &latxs_env_ir2_opnd, lsenv_offset_of_top(lsenv));
        }
    } else {
        IR2_OPND top = latxs_ra_alloc_itemp();
        latxs_tr_save_lstop_to_env(&top);
        latxs_ra_free_temp(&top);
    }
}

void latxs_fpu_fix_before_exec_tb(CPUX86State *env, TranslationBlock *tb)
{
    if (env->fpstt != tb->_top_in) {
        rotate_fpu_to_top(tb->_top_in);
    }
}

void latxs_fpu_fix_after_exec_tb(CPUX86State *env, TranslationBlock *tb)
{
    int latxs_fpu = env->latxs_fpu;

    /* 1. rotate FPU top */
    if (lsenv_get_top_bias(lsenv) != 0) {
        rotate_fpu_to_bias(0);
    }

    /* 2. fix TOP when loaded/reseted FPU top */
    if (latxs_fpu_need_fix(latxs_fpu)) {
        if (latxs_fpu & LATXS_FPU_LOAD_ST_MASK) { /* fldenv, ... */
            env->fpstt = latxs_fpu_get_top(latxs_fpu);
            lsenv_set_top_bias(lsenv, 0);
        } else { /* XTM_FPU_RESET_ST: fninit, ... */
            env->fpstt = 0;
        }
    }

    /* 3. clean env->latxs_fpu */
    env->latxs_fpu = LATXS_FPU_RESET_VALUE;
}

void latxs_fpu_fix_cpu_loop_exit(void)
{
    /*
     * FPU Top fix in system-mode:
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
    if (lsenv_get_top_bias(lsenv) != 0) {
        rotate_fpu_to_bias(0);
    }
}

#if defined(LATX_SYS_FCSR) && defined(LATX_SYS_FCSR_SIMD)
static __thread int simd_fcsr_saved;
/*
 * latxs_set_fpu_fcsr_rounding_field_by_x86() : set()
 * latxs_set_fpu_fcsr()                       : reset()
 *
 * RM: rounding mode
 *
 * In x86, SIMD's RM is controled by env->mxcsr
 *         FPU's  RM is controled by env->fcsr
 *
 * In LA, both are controled by LA's FCSR
 *
 * Some SIMD instructions will use env->mxcsr's RM:
 *  1> read current LA's FCSR into temp register @tmp
 *  2> call set() to set LA's FCSR RM according to env->mxcsr
 *  3> do SIMD operation
 *  4> call reset() to set LA's FCSR according to @tmp
 *
 * During step <3>, the LA's FCSR is NOT env->fcsr  (x86's FPU)
 *                                   but env->mxcsr (x86's SIMD)
 *
 * So we hack the set() and reset() function to indicate it:
 *  >> in step <2> set()   will set env->is_fscr_simd = 1
 *  >> in step <4> reset() will set env->is_fcse_simd = 0
 *
 * During context switch, if env->is_fcsr_simd is set to 1, then
 * the LA's FCSR will be saved/loaded into/from env->fscr_simd.
 *
 * TODO: what if exception occurs in step<3> ?
 * For now we simply assert it in latxs_fix_after_excp_or_int()
 */
#endif

void latxs_set_fpu_fcsr(IR2_OPND *new_fcsr)
{
    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, &latxs_fcsr_ir2_opnd, new_fcsr);
#if defined(LATX_SYS_FCSR) && defined(LATX_SYS_FCSR_SIMD)
    assert(simd_fcsr_saved == 1);
    simd_fcsr_saved = 0;

    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND tmp_int = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ORI, &tmp_int, zero, 0);
    latxs_append_ir2_opnd2i(LISA_ST_W, &tmp_int, &latxs_env_ir2_opnd,
            offsetof(CPUX86State, is_fcsr_simd));
    latxs_ra_free_temp(&tmp_int);
#endif
}

IR2_OPND latxs_set_fpu_fcsr_rounding_field_by_x86(void)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *fcsr = &latxs_fcsr_ir2_opnd;

    IR2_OPND temp_fcsr = latxs_ra_alloc_itemp();

    latxs_append_ir2_opnd2(LISA_MOVFCSR2GR, &temp_fcsr, fcsr);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &temp_fcsr, zero, 9, 8);

    /* save fscr in fcsr_opnd for reload */
    IR2_OPND fcsr_opnd = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_OR, &fcsr_opnd, &temp_fcsr, zero);

    /* set fcsr according to x86 MXCSR register */
    IR2_OPND temp_mxcsr = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_WU, &temp_mxcsr,
            &latxs_env_ir2_opnd,
            lsenv_offset_of_mxcsr(lsenv));
    latxs_append_ir2_opnd2ii(LISA_BSTRPICK_W, &temp_mxcsr,
                                              &temp_mxcsr, 14, 13);

    IR2_OPND temp_int = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ANDI, &temp_int, &temp_mxcsr, 0x1);

    IR2_OPND label1 = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &temp_int, zero, &label1);
    latxs_append_ir2_opnd2i(LISA_XORI, &temp_mxcsr, &temp_mxcsr, 0x2);
    latxs_append_ir2_opnd1(LISA_LABEL, &label1);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &temp_fcsr, &temp_mxcsr, 9, 8);
    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr, &temp_fcsr);

    latxs_ra_free_temp(&temp_fcsr);
    latxs_ra_free_temp(&temp_mxcsr);

#if defined(LATX_SYS_FCSR) && defined(LATX_SYS_FCSR_SIMD)
    latxs_append_ir2_opnd2i(LISA_ORI, &temp_int, zero, 1);
    latxs_append_ir2_opnd2i(LISA_ST_W, &temp_int, &latxs_env_ir2_opnd,
            offsetof(CPUX86State, is_fcsr_simd));

    latxs_append_ir2_opnd2i(LISA_ST_W, &fcsr_opnd,
            &latxs_env_ir2_opnd,
            offsetof(CPUX86State, fcsr));

    simd_fcsr_saved = 1;
#endif

    latxs_ra_free_temp(&temp_int);

    return fcsr_opnd;
}

void latxs_tr_reset_fcsr_rm(void)
{
    IR2_OPND *fcsr = &latxs_fcsr_ir2_opnd;
    IR2_OPND tmp = latxs_ra_alloc_itemp();

    latxs_append_ir2_opnd2(LISA_MOVFCSR2GR, &tmp, fcsr);

    latxs_append_ir2_opnd2i(LISA_ORI,  &tmp, &tmp, 0x300);
    latxs_append_ir2_opnd2i(LISA_XORI, &tmp, &tmp, 0x300);

    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr, &tmp);
}

void latxs_update_fcsr(void *_env)
{
    CPUX86State *env = _env;
#if defined(LATX_SYS_FCSR)
    lsassertm(!env->is_fcsr_simd, "simd fcsr update\n");
    env->fcsr = env->fcsr & ~0x300;
    switch (env->fpuc & 0xc00) {
    default:
    case 0x000: /* x86 00 -> LA 00 */
        break;
    case 0x400: /* x86 01 -> LA 11 */
        env->fcsr |= 0x300;
        break;
    case 0x800: /* x86 10 -> LA 10 */
        env->fcsr |= 0x200;
        break;
    case 0xc00: /* x86 11 -> LA 01 */
        env->fcsr |= 0x100;
        break;
    }
#endif
    /* TODO: support fpu exception */
#if defined(LATX_SYS_FCSR_EXCP)
    /* exception enable */
    env->fcsr ^= (env->fpuc & (0x1 << 0)) << 4; /* x86 IM 0 -> LA 4 */
    env->fcsr ^= (env->fpuc & (0x1 << 1)) >> 1; /* x86 DM 1 -> LA 0 */
    env->fcsr ^= (env->fpuc & (0x1 << 2)) << 1; /* x86 ZM 2 -> LA 3 */
    env->fcsr ^= (env->fpuc & (0x1 << 3)) >> 1; /* x86 OM 3 -> LA 2 */
    env->fcsr ^= (env->fpuc & (0x1 << 4)) >> 3; /* x86 UM 4 -> LA 1 */

    /* statuc word flag */
    env->fcsr |= (env->fpus & (0x1 << 0)) << 20; /* x86 IE 0 -> LA 20 */
    env->fcsr |= (env->fpus & (0x1 << 2)) << 17; /* x86 ZE 2 -> LA 19 */
    env->fcsr |= (env->fpus & (0x1 << 3)) << 15; /* x86 OE 3 -> LA 18 */
    env->fcsr |= (env->fpus & (0x1 << 4)) << 13; /* x86 UE 4 -> LA 17 */
    env->fcsr |= (env->fpus & (0x1 << 5)) << 11; /* x86 UE 5 -> LA 16 */
#endif
}
