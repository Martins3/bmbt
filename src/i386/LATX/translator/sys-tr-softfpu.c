#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "flag-lbt.h"
#include "sys-excp.h"

#ifdef TARGET_X86_64
#define LATXS_MOV_ADDR_TO_ARG(argx, src)                       \
    do {                                                       \
        if (latxs_ir1_addr_size(pir1) == 8) {                  \
            latxs_append_ir2_opnd2_(lisa_mov, &argx, &src);    \
        } else {                                               \
            latxs_append_ir2_opnd2_(lisa_mov32z, &argx, &src); \
        }                                                      \
    } while (0)
#else
#define LATXS_MOV_ADDR_TO_ARG(argx, src)                   \
    do {                                                   \
        latxs_append_ir2_opnd2_(lisa_mov32s, &argx, &src); \
    } while (0)
#endif

bool latxs_translate_fld_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    int opnd_size = ir1_opnd_size(opnd);
    if (ir1_opnd_is_fpr(opnd)) {
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpush,
                                         default_helper_cfg);

        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_ST0_STN,
                                         (ir1_opnd_base_reg_num(opnd) + 1) & 7,
                                         default_helper_cfg);
        return true;
    } else if (opnd_size == 32) {
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_load_ir1_to_ir2(&temp, opnd, EXMode_S);
        latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                &latxs_env_ir2_opnd);
        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &temp);
        latxs_tr_gen_call_to_helper((ADDR)helper_flds_ST0);
        latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
        latxs_ra_free_temp(&temp);
        return true;
    } else if (opnd_size == 64) {
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_load_ir1_to_ir2(&temp, opnd, EXMode_S);
        latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                &latxs_env_ir2_opnd);
        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &temp);
        latxs_tr_gen_call_to_helper((ADDR)helper_fldl_ST0);
        latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
        latxs_ra_free_temp(&temp);
        return true;
    } else if (opnd_size == 80) {
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_convert_mem_opnd_no_offset(&temp, opnd);
        latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
        LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, temp);
        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                &latxs_env_ir2_opnd);
        latxs_tr_gen_call_to_helper((ADDR)helper_fldt_ST0);
        latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
        latxs_ra_free_temp(&temp);
        return true;
    }
    lsassert(0);
    return false;
}

bool latxs_translate_fild_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    int opnd_size = ir1_opnd_size(opnd);
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_load_ir1_to_ir2(&latxs_arg1_ir2_opnd, opnd, EXMode_S);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    if (opnd_size == 16 || opnd_size == 32) {
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fildl_ST0,
                                         default_helper_cfg);
    } else if (opnd_size == 64) {
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fildll_ST0,
                                         default_helper_cfg);
    } else {
        lsassert(0);
    }
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
    return true;
}

bool latxs_translate_fbld_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&temp, opnd);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, temp);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_tr_gen_call_to_helper((ADDR)helper_fbld_ST0);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
    return true;
}

bool latxs_translate_fst_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    int opnd_size = ir1_opnd_size(opnd);
    if (ir1_opnd_is_fpr(opnd)) {
        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_STN_ST0,
                                         ir1_opnd_base_reg_num(opnd),
                                         default_helper_cfg);
        return true;
    } else if (opnd_size == 32) {
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fsts_ST0,
                                         default_helper_cfg);
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2_(lisa_mov, &temp, &latxs_ret0_ir2_opnd);
        latxs_store_ir2_to_ir1(&temp, opnd);
        latxs_ra_free_temp(&temp);
        return true;
    } else if (opnd_size == 64) {
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fstl_ST0,
                                         default_helper_cfg);
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2_(lisa_mov, &temp, &latxs_ret0_ir2_opnd);
        latxs_store_ir2_to_ir1(&temp, opnd);
        latxs_ra_free_temp(&temp);
        return true;
    }
    lsassert(0);
    return false;
}

bool latxs_translate_fist_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *dst = ir1_get_opnd(pir1, 0);
    int opnd_size = ir1_opnd_size(dst);
    if (opnd_size == 16) {
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fist_ST0,
                                         default_helper_cfg);
    } else if (opnd_size == 32) {
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fistl_ST0,
                                         default_helper_cfg);
    } else if (opnd_size == 64) {
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fistll_ST0,
                                         default_helper_cfg);
    } else {
        lsassert(0);
    }
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2_(lisa_mov, &temp, &latxs_ret0_ir2_opnd);
    latxs_store_ir2_to_ir1(&temp, dst);
    latxs_ra_free_temp(&temp);

    return true;
}

bool latxs_translate_fstp_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    int opnd_size = ir1_opnd_size(opnd);
    if (ir1_opnd_is_fpr(opnd)) {
        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_STN_ST0,
                                         ir1_opnd_base_reg_num(opnd),
                                         default_helper_cfg);
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);
        return true;
    } else if (opnd_size == 32) {
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fsts_ST0,
                                         default_helper_cfg);
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2_(lisa_mov, &temp, &latxs_ret0_ir2_opnd);
        latxs_store_ir2_to_ir1(&temp, opnd);
        latxs_ra_free_temp(&temp);
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);
        return true;
    } else if (opnd_size == 64) {
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fstl_ST0,
                                         default_helper_cfg);
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2_(lisa_mov, &temp, &latxs_ret0_ir2_opnd);
        latxs_store_ir2_to_ir1(&temp, opnd);
        latxs_ra_free_temp(&temp);
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);
        return true;
    } else if (opnd_size == 80) {
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_convert_mem_opnd_no_offset(&temp, opnd);
        latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

        LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, temp);
        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                &latxs_env_ir2_opnd);
        latxs_tr_gen_call_to_helper((ADDR)helper_fstt_ST0);
        latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);
        latxs_ra_free_temp(&temp);
        return true;
    }
    lsassert(0);
    return false;
}

bool latxs_translate_fistp_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *dst = ir1_get_opnd(pir1, 0);
    int opnd_size = ir1_opnd_size(dst);
    if (opnd_size == 16) {
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fist_ST0,
                                         default_helper_cfg);
    } else if (opnd_size == 32) {
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fistl_ST0,
                                         default_helper_cfg);
    } else if (opnd_size == 64) {
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fistll_ST0,
                                         default_helper_cfg);
    } else {
        lsassert(0);
    }
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2_(lisa_mov, &temp, &latxs_ret0_ir2_opnd);
    latxs_store_ir2_to_ir1(&temp, dst);
    latxs_ra_free_temp(&temp);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);
    return true;
}

bool latxs_translate_fbstp_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&temp, opnd);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, temp);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_tr_gen_call_to_helper((ADDR)helper_fbst_ST0);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);
    return true;
}

bool latxs_translate_fxch_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *src = ir1_get_opnd(pir1, 0);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fxchg_ST0_STN,
                                     ir1_opnd_base_reg_num(src),
                                     default_helper_cfg);
    return true;
}

bool latxs_translate_fcmovb_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1) ||
        latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    IR2_OPND cf_opnd = latxs_ra_alloc_itemp();
    latxs_get_eflag_condition(&cf_opnd, pir1);

    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &cf_opnd, &latxs_zero_ir2_opnd,
                           &label_exit);

    IR1_OPND *opnd = ir1_get_opnd(pir1, 1);
    int reg_num = ir1_opnd_base_reg_num(opnd);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_ST0_STN, reg_num,
                                     default_helper_cfg);

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);
    latxs_ra_free_temp(&cf_opnd);
    return true;
}

bool latxs_translate_fcmove_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1) ||
        latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    IR2_OPND zf_opnd = latxs_ra_alloc_itemp();
    latxs_get_eflag_condition(&zf_opnd, pir1);

    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &zf_opnd, &latxs_zero_ir2_opnd,
                           &label_exit);

    IR1_OPND *opnd = ir1_get_opnd(pir1, 1);
    int reg_num = ir1_opnd_base_reg_num(opnd);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_ST0_STN, reg_num,
                                     default_helper_cfg);

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);
    latxs_ra_free_temp(&zf_opnd);
    return true;
}

bool latxs_translate_fcmovbe_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1) ||
        latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    IR2_OPND cfzf_opnd = latxs_ra_alloc_itemp();
    latxs_get_eflag_condition(&cfzf_opnd, pir1);

    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &cfzf_opnd, &latxs_zero_ir2_opnd,
                           &label_exit);

    IR1_OPND *opnd = ir1_get_opnd(pir1, 1);
    int reg_num = ir1_opnd_base_reg_num(opnd);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_ST0_STN, reg_num,
                                     default_helper_cfg);

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);
    latxs_ra_free_temp(&cfzf_opnd);
    return true;
}

bool latxs_translate_fcmovu_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1) ||
        latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    IR2_OPND pf_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&pf_opnd, pir1);

    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &pf_opnd, &latxs_zero_ir2_opnd,
                           &label_exit);

    IR1_OPND *opnd = ir1_get_opnd(pir1, 1);
    int reg_num = ir1_opnd_base_reg_num(opnd);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_ST0_STN, reg_num,
                                     default_helper_cfg);

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);
    latxs_ra_free_temp(&pf_opnd);
    return true;
}

bool latxs_translate_fcmovnb_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1) ||
        latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    IR2_OPND cf_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&cf_opnd, pir1);

    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BNE, &cf_opnd, &latxs_zero_ir2_opnd,
                           &label_exit);

    IR1_OPND *opnd = ir1_get_opnd(pir1, 1);
    int reg_num = ir1_opnd_base_reg_num(opnd);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_ST0_STN, reg_num,
                                     default_helper_cfg);

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);
    latxs_ra_free_temp(&cf_opnd);
    return true;
}

bool latxs_translate_fcmovne_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1) ||
        latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    IR2_OPND zf_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&zf_opnd, pir1);

    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BNE, &zf_opnd, &latxs_zero_ir2_opnd,
                           &label_exit);

    IR1_OPND *opnd = ir1_get_opnd(pir1, 1);
    int reg_num = ir1_opnd_base_reg_num(opnd);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_ST0_STN, reg_num,
                                     default_helper_cfg);

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);
    latxs_ra_free_temp(&zf_opnd);
    return true;
}

bool latxs_translate_fcmovnbe_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1) ||
        latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    IR2_OPND cfzf_opnd = latxs_ra_alloc_itemp();
    latxs_get_eflag_condition(&cfzf_opnd, pir1);

    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BNE, &cfzf_opnd, &latxs_zero_ir2_opnd,
                           &label_exit);

    IR1_OPND *opnd = ir1_get_opnd(pir1, 1);
    int reg_num = ir1_opnd_base_reg_num(opnd);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_ST0_STN, reg_num,
                                     default_helper_cfg);

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);
    latxs_ra_free_temp(&cfzf_opnd);
    return true;
}

bool latxs_translate_fcmovnu_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1) ||
        latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    IR2_OPND pf_opnd = latxs_ra_alloc_itemp();
    latxs_get_eflag_condition(&pf_opnd, pir1);

    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BNE, &pf_opnd, &latxs_zero_ir2_opnd,
                           &label_exit);

    IR1_OPND *opnd = ir1_get_opnd(pir1, 1);
    int reg_num = ir1_opnd_base_reg_num(opnd);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_ST0_STN, reg_num,
                                     default_helper_cfg);

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);
    latxs_ra_free_temp(&pf_opnd);
    return true;
}

bool latxs_translate_fldz_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpush, default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fldz_ST0, default_helper_cfg);
    return true;
}

bool latxs_translate_fld1_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpush, default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fld1_ST0, default_helper_cfg);
    return true;
}

bool latxs_translate_fldl2e_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpush, default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fldl2e_ST0,
                                     default_helper_cfg);
    return true;
}

bool latxs_translate_fldl2t_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpush, default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fldl2t_ST0,
                                     default_helper_cfg);
    return true;
}

bool latxs_translate_fldlg2_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpush, default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fldlg2_ST0,
                                     default_helper_cfg);
    return true;
}

bool latxs_translate_fldln2_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpush, default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fldln2_ST0,
                                     default_helper_cfg);
    return true;
}

bool latxs_translate_fldpi_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpush, default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fldpi_ST0,
                                     default_helper_cfg);
    return true;
}

bool latxs_translate_fadd_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    IR1_OPND *src = ir1_get_opnd(pir1, 0);
    if (ir1_opnd_is_mem(src)) {
        int opnd_size = ir1_opnd_size(src);
        if (opnd_size == 32) {
            latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
            latxs_load_ir1_to_ir2(&latxs_arg1_ir2_opnd, src, EXMode_S);
            latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                    &latxs_env_ir2_opnd);
            latxs_tr_gen_call_to_helper((ADDR)helper_flds_FT0);
            latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
            latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fadd_ST0_FT0,
                                             default_helper_cfg);
            return true;
        } else {
            latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
            latxs_load_ir1_to_ir2(&latxs_arg1_ir2_opnd, src, EXMode_S);
            latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                    &latxs_env_ir2_opnd);
            latxs_tr_gen_call_to_helper((ADDR)helper_fldl_FT0);
            latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
            latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fadd_ST0_FT0,
                                             default_helper_cfg);
            return true;
        }
        lsassert(0);
    } else if (opnd_num == 1) {
        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_FT0_STN,
                                         ir1_opnd_base_reg_num(src),
                                         default_helper_cfg);
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fadd_ST0_FT0,
                                         default_helper_cfg);

        return true;
    } else if (opnd_num == 2) {
        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fadd_STN_ST0,
                                         ir1_opnd_base_reg_num(src),
                                         default_helper_cfg);
        return true;
    }
    lsassert(0);
    return false;
}

bool latxs_translate_faddp_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *src = ir1_get_opnd(pir1, 0);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fadd_STN_ST0,
                                     ir1_opnd_base_reg_num(src),
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);

    return true;
}

bool latxs_translate_fiadd_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);

    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&temp, opnd, EXMode_S);
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &temp);
    latxs_tr_gen_call_to_helper((ADDR)helper_fildl_FT0);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fadd_ST0_FT0,
                                     default_helper_cfg);

    latxs_ra_free_temp(&temp);

    return true;
}

bool latxs_translate_fmul_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    IR1_OPND *src = ir1_get_opnd(pir1, 0);
    if (ir1_opnd_is_mem(src)) {
        int opnd_size = ir1_opnd_size(src);
        if (opnd_size == 32) {
            latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
            latxs_load_ir1_to_ir2(&latxs_arg1_ir2_opnd, src, EXMode_S);
            latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                    &latxs_env_ir2_opnd);
            latxs_tr_gen_call_to_helper((ADDR)helper_flds_FT0);
            latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
            latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fmul_ST0_FT0,
                                             default_helper_cfg);
            return true;
        } else {
            latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
            latxs_load_ir1_to_ir2(&latxs_arg1_ir2_opnd, src, EXMode_S);
            latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                    &latxs_env_ir2_opnd);
            latxs_tr_gen_call_to_helper((ADDR)helper_fldl_FT0);
            latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
            latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fmul_ST0_FT0,
                                             default_helper_cfg);
            return true;
        }
        lsassert(0);
    } else if (opnd_num == 1) {
        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_FT0_STN,
                                         ir1_opnd_base_reg_num(src),
                                         default_helper_cfg);
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fmul_ST0_FT0,
                                         default_helper_cfg);

        return true;
    } else if (opnd_num == 2) {
        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmul_STN_ST0,
                                         ir1_opnd_base_reg_num(src),
                                         default_helper_cfg);
        return true;
    }
    lsassert(0);
    return false;
}

bool latxs_translate_fmulp_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *src = ir1_get_opnd(pir1, 0);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmul_STN_ST0,
                                     ir1_opnd_base_reg_num(src),
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);

    return true;
}

bool latxs_translate_fimul_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);

    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&temp, opnd, EXMode_S);
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &temp);
    latxs_tr_gen_call_to_helper((ADDR)helper_fildl_FT0);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fmul_ST0_FT0,
                                     default_helper_cfg);

    latxs_ra_free_temp(&temp);

    return true;
}

bool latxs_translate_fsub_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    IR1_OPND *src = ir1_get_opnd(pir1, 0);
    if (ir1_opnd_is_mem(src)) {
        int opnd_size = ir1_opnd_size(src);
        if (opnd_size == 32) {
            latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
            latxs_load_ir1_to_ir2(&latxs_arg1_ir2_opnd, src, EXMode_S);
            latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                    &latxs_env_ir2_opnd);
            latxs_tr_gen_call_to_helper((ADDR)helper_flds_FT0);
            latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
            latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fsub_ST0_FT0,
                                             default_helper_cfg);
            return true;
        } else {
            latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
            latxs_load_ir1_to_ir2(&latxs_arg1_ir2_opnd, src, EXMode_S);
            latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                    &latxs_env_ir2_opnd);
            latxs_tr_gen_call_to_helper((ADDR)helper_fldl_FT0);
            latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
            latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fsub_ST0_FT0,
                                             default_helper_cfg);
            return true;
        }
        lsassert(0);
    } else if (opnd_num == 1) {
        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_FT0_STN,
                                         ir1_opnd_base_reg_num(src),
                                         default_helper_cfg);
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fsub_ST0_FT0,
                                         default_helper_cfg);

        return true;
    } else if (opnd_num == 2) {
        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fsub_STN_ST0,
                                         ir1_opnd_base_reg_num(src),
                                         default_helper_cfg);
        return true;
    }
    lsassert(0);
    return false;
}

bool latxs_translate_fsubr_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    IR1_OPND *src = ir1_get_opnd(pir1, 0);
    if (ir1_opnd_is_mem(src)) {
        int opnd_size = ir1_opnd_size(src);
        if (opnd_size == 32) {
            latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
            latxs_load_ir1_to_ir2(&latxs_arg1_ir2_opnd, src, EXMode_S);
            latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                    &latxs_env_ir2_opnd);
            latxs_tr_gen_call_to_helper((ADDR)helper_flds_FT0);
            latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
            latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fsubr_ST0_FT0,
                                             default_helper_cfg);
            return true;
        } else {
            latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
            latxs_load_ir1_to_ir2(&latxs_arg1_ir2_opnd, src, EXMode_S);
            latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                    &latxs_env_ir2_opnd);
            latxs_tr_gen_call_to_helper((ADDR)helper_fldl_FT0);
            latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
            latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fsubr_ST0_FT0,
                                             default_helper_cfg);
            return true;
        }
        lsassert(0);
    } else if (opnd_num == 1) {
        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_FT0_STN,
                                         ir1_opnd_base_reg_num(src),
                                         default_helper_cfg);
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fsubr_ST0_FT0,
                                         default_helper_cfg);

        return true;
    } else if (opnd_num == 2) {
        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fsubr_STN_ST0,
                                         ir1_opnd_base_reg_num(src),
                                         default_helper_cfg);
        return true;
    }
    lsassert(0);
    return false;
}

bool latxs_translate_fsubp_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *src = ir1_get_opnd(pir1, 0);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fsub_STN_ST0,
                                     ir1_opnd_base_reg_num(src),
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);

    return true;
}

bool latxs_translate_fsubrp_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *src = ir1_get_opnd(pir1, 0);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fsubr_STN_ST0,
                                     ir1_opnd_base_reg_num(src),
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);

    return true;
}

bool latxs_translate_fisub_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);

    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&temp, opnd, EXMode_S);
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &temp);
    latxs_tr_gen_call_to_helper((ADDR)helper_fildl_FT0);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fsub_ST0_FT0,
                                     default_helper_cfg);

    latxs_ra_free_temp(&temp);

    return true;
}

bool latxs_translate_fisubr_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);

    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&temp, opnd, EXMode_S);
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &temp);
    latxs_tr_gen_call_to_helper((ADDR)helper_fildl_FT0);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fsubr_ST0_FT0,
                                     default_helper_cfg);

    latxs_ra_free_temp(&temp);

    return true;
}

bool latxs_translate_fdiv_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    IR1_OPND *src = ir1_get_opnd(pir1, 0);
    if (ir1_opnd_is_mem(src)) {
        int opnd_size = ir1_opnd_size(src);
        if (opnd_size == 32) {
            latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
            latxs_load_ir1_to_ir2(&latxs_arg1_ir2_opnd, src, EXMode_S);
            latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                    &latxs_env_ir2_opnd);
            latxs_tr_gen_call_to_helper((ADDR)helper_flds_FT0);
            latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
            latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fdiv_ST0_FT0,
                                             default_helper_cfg);
            return true;
        } else {
            latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
            latxs_load_ir1_to_ir2(&latxs_arg1_ir2_opnd, src, EXMode_S);
            latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                    &latxs_env_ir2_opnd);
            latxs_tr_gen_call_to_helper((ADDR)helper_fldl_FT0);
            latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
            latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fdiv_ST0_FT0,
                                             default_helper_cfg);
            return true;
        }
        lsassert(0);
    } else if (opnd_num == 1) {
        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_FT0_STN,
                                         ir1_opnd_base_reg_num(src),
                                         default_helper_cfg);
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fdiv_ST0_FT0,
                                         default_helper_cfg);

        return true;
    } else if (opnd_num == 2) {
        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fdiv_STN_ST0,
                                         ir1_opnd_base_reg_num(src),
                                         default_helper_cfg);
        return true;
    }
    lsassert(0);
    return false;
}

bool latxs_translate_fdivr_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    IR1_OPND *src = ir1_get_opnd(pir1, 0);
    if (ir1_opnd_is_mem(src)) {
        int opnd_size = ir1_opnd_size(src);
        if (opnd_size == 32) {
            latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
            latxs_load_ir1_to_ir2(&latxs_arg1_ir2_opnd, src, EXMode_S);
            latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                    &latxs_env_ir2_opnd);
            latxs_tr_gen_call_to_helper((ADDR)helper_flds_FT0);
            latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
            latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fdivr_ST0_FT0,
                                             default_helper_cfg);
            return true;
        } else {
            latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
            latxs_load_ir1_to_ir2(&latxs_arg1_ir2_opnd, src, EXMode_S);
            latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                    &latxs_env_ir2_opnd);
            latxs_tr_gen_call_to_helper((ADDR)helper_fldl_FT0);
            latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
            latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fdivr_ST0_FT0,
                                             default_helper_cfg);
            return true;
        }
        lsassert(0);
    } else if (opnd_num == 1) {
        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_FT0_STN,
                                         ir1_opnd_base_reg_num(src),
                                         default_helper_cfg);
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fdivr_ST0_FT0,
                                         default_helper_cfg);

        return true;
    } else if (opnd_num == 2) {
        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fdivr_STN_ST0,
                                         ir1_opnd_base_reg_num(src),
                                         default_helper_cfg);
        return true;
    }
    lsassert(0);
    return false;
}

bool latxs_translate_fdivp_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *src = ir1_get_opnd(pir1, 0);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fdiv_STN_ST0,
                                     ir1_opnd_base_reg_num(src),
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);

    return true;
}

bool latxs_translate_fdivrp_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *src = ir1_get_opnd(pir1, 0);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fdivr_STN_ST0,
                                     ir1_opnd_base_reg_num(src),
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);

    return true;
}

bool latxs_translate_fidiv_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);

    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&temp, opnd, EXMode_S);
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &temp);
    latxs_tr_gen_call_to_helper((ADDR)helper_fildl_FT0);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fdiv_ST0_FT0,
                                     default_helper_cfg);

    latxs_ra_free_temp(&temp);

    return true;
}

bool latxs_translate_fidivr_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);

    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&temp, opnd, EXMode_S);
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &temp);
    latxs_tr_gen_call_to_helper((ADDR)helper_fildl_FT0);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fdivr_ST0_FT0,
                                     default_helper_cfg);

    latxs_ra_free_temp(&temp);

    return true;
}

bool latxs_translate_fabs_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fabs_ST0, default_helper_cfg);
    return true;
}

bool latxs_translate_fchs_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fchs_ST0, default_helper_cfg);
    return true;
}

bool latxs_translate_fsqrt_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fsqrt, default_helper_cfg);
    return true;
}

bool latxs_translate_fprem_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fprem, default_helper_cfg);
    return true;
}

bool latxs_translate_fprem1_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fprem1, default_helper_cfg);
    return true;
}

bool latxs_translate_frndint_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_frndint, default_helper_cfg);
    return true;
}

bool latxs_translate_fxtract_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fxtract, default_helper_cfg);
    return true;
}

bool latxs_translate_fcom_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    int opnd_size = ir1_opnd_size(opnd);
    if (ir1_opnd_is_fpr(opnd)) {
        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_FT0_STN,
                                         ir1_opnd_base_reg_num(opnd),
                                         default_helper_cfg);

        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fcom_ST0_FT0,
                                         default_helper_cfg);
        return true;
    } else if (opnd_size == 32) {
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_load_ir1_to_ir2(&temp, opnd, EXMode_S);
        latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                &latxs_env_ir2_opnd);
        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &temp);
        latxs_tr_gen_call_to_helper((ADDR)helper_flds_FT0);
        latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fcom_ST0_FT0,
                                         default_helper_cfg);
        latxs_ra_free_temp(&temp);
        return true;
    } else if (opnd_size == 64) {
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_load_ir1_to_ir2(&temp, opnd, EXMode_S);
        latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                &latxs_env_ir2_opnd);
        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &temp);
        latxs_tr_gen_call_to_helper((ADDR)helper_fldl_FT0);
        latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
        latxs_ra_free_temp(&temp);
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fcom_ST0_FT0,
                                         default_helper_cfg);
        return true;
    }
    lsassert(0);
    return false;
}

bool latxs_translate_fcomp_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    int opnd_size = ir1_opnd_size(opnd);
    if (ir1_opnd_is_fpr(opnd)) {
        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_FT0_STN,
                                         ir1_opnd_base_reg_num(opnd),
                                         default_helper_cfg);

        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fcom_ST0_FT0,
                                         default_helper_cfg);
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);

        return true;
    } else if (opnd_size == 32) {
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_load_ir1_to_ir2(&temp, opnd, EXMode_S);
        latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                &latxs_env_ir2_opnd);
        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &temp);
        latxs_tr_gen_call_to_helper((ADDR)helper_flds_FT0);
        latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fcom_ST0_FT0,
                                         default_helper_cfg);
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);

        latxs_ra_free_temp(&temp);
        return true;
    } else if (opnd_size == 64) {
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_load_ir1_to_ir2(&temp, opnd, EXMode_S);
        latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                &latxs_env_ir2_opnd);
        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &temp);
        latxs_tr_gen_call_to_helper((ADDR)helper_fldl_FT0);
        latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
        latxs_ra_free_temp(&temp);
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fcom_ST0_FT0,
                                         default_helper_cfg);
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);

        return true;
    }
    lsassert(0);
    return false;
}
bool latxs_translate_fcompp_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    int reg_num = 1;
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_FT0_STN, reg_num,
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fcom_ST0_FT0,
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);
    return true;
}

bool latxs_translate_fucom_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    int reg_num = ir1_opnd_base_reg_num(opnd0);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_FT0_STN, reg_num,
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fucom_ST0_FT0,
                                     default_helper_cfg);
    return true;
}
bool latxs_translate_fucomp_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    int reg_num = ir1_opnd_base_reg_num(opnd0);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_FT0_STN, reg_num,
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fucom_ST0_FT0,
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);
    return true;
}
bool latxs_translate_fucompp_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_FT0_STN, 1,
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fucom_ST0_FT0,
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);
    return true;
}

bool latxs_translate_fcomi_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &zero_ir2_opnd, 0x34);
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    int reg_num = ir1_opnd_base_reg_num(opnd0);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_FT0_STN, reg_num,
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fcomi_ST0_FT0,
                                     default_helper_cfg);
    return true;
}
bool latxs_translate_fcomip_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &zero_ir2_opnd, 0x34);
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    int reg_num = ir1_opnd_base_reg_num(opnd0);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_FT0_STN, reg_num,
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fcomi_ST0_FT0,
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);
    return true;
}
bool latxs_translate_fucomi_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &zero_ir2_opnd, 0x34);
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    int reg_num = ir1_opnd_base_reg_num(opnd0);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_FT0_STN, reg_num,
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fucomi_ST0_FT0,
                                     default_helper_cfg);
    return true;
}
bool latxs_translate_fucomip_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &zero_ir2_opnd, 0x34);
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    int reg_num = ir1_opnd_base_reg_num(opnd0);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_fmov_FT0_STN, reg_num,
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fucomi_ST0_FT0,
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);
    return true;
}

bool latxs_translate_ficom_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&temp, opnd, EXMode_S);
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &temp);
    latxs_tr_gen_call_to_helper((ADDR)helper_fildl_FT0);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fcom_ST0_FT0,
                                     default_helper_cfg);
    latxs_ra_free_temp(&temp);
    return true;
}

bool latxs_translate_ficomp_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&temp, opnd, EXMode_S);
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &temp);
    latxs_tr_gen_call_to_helper((ADDR)helper_fildl_FT0);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fcom_ST0_FT0,
                                     default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpop, default_helper_cfg);
    latxs_ra_free_temp(&temp);
    return true;
}

bool latxs_translate_ftst_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fldz_FT0, default_helper_cfg);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fcom_ST0_FT0,
                                     default_helper_cfg);
    return true;
}

bool latxs_translate_fxam_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fxam_ST0, default_helper_cfg);
    return true;
}

bool latxs_translate_fsin_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fsin, default_helper_cfg);
    return true;
}

bool latxs_translate_fcos_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fcos, default_helper_cfg);
    return true;
}

bool latxs_translate_fpatan_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fpatan, default_helper_cfg);
    return true;
}

bool latxs_translate_fsincos_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fsincos, default_helper_cfg);
    return true;
}

bool latxs_translate_fptan_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fptan, default_helper_cfg);
    return true;
}

bool latxs_translate_fyl2x_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fyl2x, default_helper_cfg);
    return true;
}

bool latxs_translate_fyl2xp1_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fyl2xp1, default_helper_cfg);
    return true;
}

bool latxs_translate_fscale_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fscale, default_helper_cfg);
    return true;
}

bool latxs_translate_f2xm1_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_f2xm1, default_helper_cfg);
    return true;
}

bool latxs_translate_fninit_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fninit, default_helper_cfg);
    return true;
}

bool latxs_translate_fldcw_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&temp, opnd, EXMode_Z);
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &temp);
    latxs_tr_gen_call_to_helper((ADDR)helper_fldcw);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
    latxs_ra_free_temp(&temp);
    return true;
}

bool latxs_translate_fnstcw_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fnstcw, default_helper_cfg);
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2_(lisa_mov, &temp, &latxs_ret0_ir2_opnd);
    latxs_store_ir2_to_ir1(&temp, opnd);
    latxs_ra_free_temp(&temp);

    return true;
}

bool latxs_translate_fnstsw_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fnstsw, default_helper_cfg);
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2_(lisa_mov, &temp, &latxs_ret0_ir2_opnd);
    latxs_store_ir2_to_ir1(&temp, opnd);
    latxs_ra_free_temp(&temp);

    return true;
}

bool latxs_translate_fnstenv_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&temp, opnd);

    int data_size = latxs_ir1_data_size(pir1);
    int data32 = data_size == 32 ? 1 : 0;

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, temp);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg2_ir2_opnd,
                            &latxs_zero_ir2_opnd, data32);
    latxs_tr_gen_call_to_helper((ADDR)helper_fstenv);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    return true;
}

bool latxs_translate_fldenv_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&temp, opnd);

    int data_size = latxs_ir1_data_size(pir1);
    int data32 = data_size == 32 ? 1 : 0;

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, temp);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg2_ir2_opnd,
                            &latxs_zero_ir2_opnd, data32);
    latxs_tr_gen_call_to_helper((ADDR)helper_fldenv);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    return true;
}

bool latxs_translate_fnsave_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&temp, opnd);

    int data_size = latxs_ir1_data_size(pir1);
    int data32 = data_size == 32 ? 1 : 0;

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, temp);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg2_ir2_opnd,
                            &latxs_zero_ir2_opnd, data32);
    latxs_tr_gen_call_to_helper((ADDR)helper_fsave);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    return true;
}

bool latxs_translate_frstor_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&temp, opnd);

    int data_size = latxs_ir1_data_size(pir1);
    int data32 = data_size == 32 ? 1 : 0;

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, temp);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg2_ir2_opnd,
                            &latxs_zero_ir2_opnd, data32);
    latxs_tr_gen_call_to_helper((ADDR)helper_frstor);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    return true;
}

bool latxs_translate_fnclex_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fclex, default_helper_cfg);
    return true;
}

bool latxs_translate_fincstp_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fincstp, default_helper_cfg);
    return true;
}

bool latxs_translate_fdecstp_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fdecstp, default_helper_cfg);
    return true;
}

bool latxs_translate_ffree_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    int reg_num = ir1_opnd_base_reg_num(opnd);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_ffree_STN, reg_num,
                                     default_helper_cfg);
    return true;
}

bool latxs_translate_fnop_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fwait, default_helper_cfg);
    return true;
}

bool latxs_translate_wait_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fwait, default_helper_cfg);
    return true;
}

/* non-x87 instruction, but accesss fpu stack */
bool latxs_translate_fxsave_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&temp, opnd);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, temp);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_tr_gen_call_to_helper((ADDR)helper_fxsave);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
    return true;
}

/* non-x87 instruction, but accesss fpu stack */
bool latxs_translate_fxrstor_softfpu(IR1_INST *pir1)
{
    if (latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&temp, opnd);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, temp);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_tr_gen_call_to_helper((ADDR)helper_fxrstor);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
    return true;
}
