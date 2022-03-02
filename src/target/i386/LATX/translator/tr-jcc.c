#include "common.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "flag-pattern.h"
#include "translate.h"

bool translate_jz(IR1_INST *pir1){
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jz(pir1);
#else
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL); 

#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, target_label_opnd)) {
#endif
        IR2_OPND cond_opnd = ra_alloc_itemp();
        la_append_ir2_opnd1i(LISA_SETX86J, cond_opnd, COND_E);
        la_append_ir2_opnd3(LISA_BNE, cond_opnd, zero_ir2_opnd, target_label_opnd);
        ra_free_temp(cond_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);
    return true;
#endif
}

bool translate_jnz(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jnz(pir1);
#else
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);

#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, target_label_opnd)) {
#endif
        IR2_OPND cond_opnd = ra_alloc_itemp();
        la_append_ir2_opnd1i(LISA_SETX86J, cond_opnd, COND_NE);
        la_append_ir2_opnd3(LISA_BNE, cond_opnd, zero_ir2_opnd, target_label_opnd);
        ra_free_temp(cond_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
#endif
}

bool translate_js(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_js(pir1);
#else
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);

#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, target_label_opnd)) {
#endif
        IR2_OPND cond_opnd = ra_alloc_itemp();
        la_append_ir2_opnd1i(LISA_SETX86J, cond_opnd, COND_S);
        la_append_ir2_opnd3(LISA_BNE, cond_opnd, zero_ir2_opnd, target_label_opnd);
        ra_free_temp(cond_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
#endif
}

bool translate_jns(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jns(pir1);
#else
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);

#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, target_label_opnd)) {
#endif
        IR2_OPND cond_opnd = ra_alloc_itemp();
        la_append_ir2_opnd1i(LISA_SETX86J, cond_opnd, COND_NS);
        la_append_ir2_opnd3(LISA_BNE, cond_opnd, zero_ir2_opnd, target_label_opnd);
        ra_free_temp(cond_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
#endif
}

bool translate_jb(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jb(pir1);
#else
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);

#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, target_label_opnd)) {
#endif
        IR2_OPND cond_opnd = ra_alloc_itemp();
        la_append_ir2_opnd1i(LISA_SETX86J, cond_opnd, COND_B);
        la_append_ir2_opnd3(LISA_BNE, cond_opnd, zero_ir2_opnd, target_label_opnd);
        ra_free_temp(cond_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
#endif
}

bool translate_jae(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jae(pir1);
#else
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);

#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, target_label_opnd)) {
#endif
        IR2_OPND cond_opnd = ra_alloc_itemp();
        la_append_ir2_opnd1i(LISA_SETX86J, cond_opnd, COND_AE);
        la_append_ir2_opnd3(LISA_BNE, cond_opnd, zero_ir2_opnd, target_label_opnd);
        ra_free_temp(cond_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
#endif
}

bool translate_jo(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jo(pir1);
#else
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);

#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, target_label_opnd)) {
#endif
        IR2_OPND cond_opnd = ra_alloc_itemp();
        la_append_ir2_opnd1i(LISA_SETX86J, cond_opnd, COND_O);
        la_append_ir2_opnd3(LISA_BNE, cond_opnd, zero_ir2_opnd, target_label_opnd);
        ra_free_temp(cond_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
#endif
}

bool translate_jno(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jno(pir1);
#else
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);

#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, target_label_opnd)) {
#endif
        IR2_OPND cond_opnd = ra_alloc_itemp();
        la_append_ir2_opnd1i(LISA_SETX86J, cond_opnd, COND_NO);
        la_append_ir2_opnd3(LISA_BNE, cond_opnd, zero_ir2_opnd, target_label_opnd);
        ra_free_temp(cond_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
#endif
}

bool translate_jbe(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jbe(pir1);
#else
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);

#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, target_label_opnd)) {
#endif
        IR2_OPND cond_opnd = ra_alloc_itemp();
        la_append_ir2_opnd1i(LISA_SETX86J, cond_opnd, COND_BE);
        la_append_ir2_opnd3(LISA_BNE, cond_opnd, zero_ir2_opnd, target_label_opnd);
        ra_free_temp(cond_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
#endif
}

bool translate_ja(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_ja(pir1);
#else
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);

#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, target_label_opnd)) {
#endif
        IR2_OPND cond_opnd = ra_alloc_itemp();
        la_append_ir2_opnd1i(LISA_SETX86J, cond_opnd, COND_A);
        la_append_ir2_opnd3(LISA_BNE, cond_opnd, zero_ir2_opnd, target_label_opnd);
        ra_free_temp(cond_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
#endif
}

bool translate_jp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jp(pir1);
#else
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);

#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, target_label_opnd)) {
#endif
        IR2_OPND cond_opnd = ra_alloc_itemp();
        la_append_ir2_opnd1i(LISA_SETX86J, cond_opnd, COND_PE);
        la_append_ir2_opnd3(LISA_BNE, cond_opnd, zero_ir2_opnd, target_label_opnd);
        ra_free_temp(cond_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
#endif
}

bool translate_jnp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jnp(pir1);
#else
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);

#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, target_label_opnd)) {
#endif
        IR2_OPND cond_opnd = ra_alloc_itemp();
        la_append_ir2_opnd1i(LISA_SETX86J, cond_opnd, COND_PO);
        la_append_ir2_opnd3(LISA_BNE, cond_opnd, zero_ir2_opnd, target_label_opnd);
        ra_free_temp(cond_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
#endif
}

bool translate_jl(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jl(pir1);
#else
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);

#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, target_label_opnd)) {
#endif
        IR2_OPND cond_opnd = ra_alloc_itemp();
        la_append_ir2_opnd1i(LISA_SETX86J, cond_opnd, COND_L);
        la_append_ir2_opnd3(LISA_BNE, cond_opnd, zero_ir2_opnd, target_label_opnd);
        ra_free_temp(cond_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
#endif
}

bool translate_jge(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jge(pir1);
#else
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);

#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, target_label_opnd)) {
#endif
        IR2_OPND cond_opnd = ra_alloc_itemp();
        la_append_ir2_opnd1i(LISA_SETX86J, cond_opnd, COND_GE);
        la_append_ir2_opnd3(LISA_BNE, cond_opnd, zero_ir2_opnd, target_label_opnd);
        ra_free_temp(cond_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
#endif
}

bool translate_jle(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jle(pir1);
#else
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);

#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, target_label_opnd)) {
#endif
        IR2_OPND cond_opnd = ra_alloc_itemp();
        la_append_ir2_opnd1i(LISA_SETX86J, cond_opnd, COND_LE);
        la_append_ir2_opnd3(LISA_BNE, cond_opnd, zero_ir2_opnd, target_label_opnd);
        ra_free_temp(cond_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
#endif
}

bool translate_jg(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jg(pir1);
#else
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);

#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, target_label_opnd)) {
#endif
        IR2_OPND cond_opnd = ra_alloc_itemp();
        la_append_ir2_opnd1i(LISA_SETX86J, cond_opnd, COND_G);
        la_append_ir2_opnd3(LISA_BNE, cond_opnd, zero_ir2_opnd, target_label_opnd);
        ra_free_temp(cond_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
#endif
}

bool translate_jcxz(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jcxz(pir1);
#else
    /* 1. load cx */
    IR2_OPND cx_opnd = load_ireg_from_ir1(&cx_ir1_opnd, ZERO_EXTENSION, false);
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, cx_opnd, zero_ir2_opnd, target_label_opnd);

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
#endif
}

bool translate_jecxz(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jecxz(pir1);
#else
    IR2_OPND ecx_opnd =
        load_ireg_from_ir1(&ecx_ir1_opnd, UNKNOWN_EXTENSION, false);

    /* 1. if the returned ecx is zero_extend or sign_extend, we don't need to */
    /* zero the high 32 bits */
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);
    if (ir2_opnd_is_ax(&ecx_opnd, 32)) {
        IR2_OPND tmp_ecx = ra_alloc_itemp();
        la_append_ir2_opnd2_em(LISA_MOV32_SX, tmp_ecx, ecx_opnd);
        la_append_ir2_opnd3(LISA_BEQ, tmp_ecx, zero_ir2_opnd, target_label_opnd);
        ra_free_temp(tmp_ecx);
    } else {
        la_append_ir2_opnd3(LISA_BEQ, ecx_opnd, zero_ir2_opnd, target_label_opnd);
    }

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
#endif
}

bool translate_jrcxz(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jrcxz(pir1);
#else
    /* 1. load rcx */
    IR2_OPND rcx_opnd = ra_alloc_gpr(ecx_index);
    IR2_OPND target_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, rcx_opnd, zero_ir2_opnd, target_label_opnd);

    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
#endif
}
