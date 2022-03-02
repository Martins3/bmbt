#include "env.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "flag-lbt.h"
#include "translate.h"

bool translate_setz(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_setz(pir1);
#else
    /* 1. prepare the value operand */
    IR2_OPND value_opnd;
    bool value_opnd_is_temp = false;
    if (ir1_opnd_is_8l(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_zx(ir1_get_opnd(pir1, 0), 8)) {
        /* the other bits of the target register is 0, so we can set the */
        /* register directly */
        value_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    } else {
        value_opnd = ra_alloc_itemp();
        value_opnd_is_temp = true;
    }

    /* 2. set the value operand */
#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, value_opnd)) {
#endif
        IR2_OPND zf_opnd = ra_alloc_itemp();
        get_eflag_condition(&zf_opnd, pir1);
        la_append_ir2_opnd2i_em(LISA_SRLI_W, value_opnd, zf_opnd, ZF_BIT_INDEX);
        ra_free_temp(zf_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    /* 3. store to the dest operand */
    store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false);
    if (value_opnd_is_temp)
        ra_free_temp(value_opnd);
    return true;
#endif
}

bool translate_setnz(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_setnz(pir1);
#else
    /* 1. prepare the value operand */
    IR2_OPND value_opnd;
    bool value_opnd_is_temp = false;
    if (ir1_opnd_is_8l(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_zx(ir1_get_opnd(pir1, 0), 8)) {
        /* the other bits of the target register is 0, so we can set the */
        /* register directly */
        value_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    } else {
        value_opnd = ra_alloc_itemp();
        value_opnd_is_temp = true;
    }

    /* 2. set the value operand */
#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, value_opnd)) {
#endif
        IR2_OPND zf_opnd = ra_alloc_itemp();
        get_eflag_condition(&zf_opnd, pir1);
        la_append_ir2_opnd2i_em(LISA_SLTUI, value_opnd, zf_opnd, 1);
        ra_free_temp(zf_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    /* 3. store to the dest operand */
    store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false);
    if (value_opnd_is_temp)
        ra_free_temp(value_opnd);
    return true;
#endif
}

bool translate_seto(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_seto(pir1);
#else
    /* 1. prepare the value operand */
    IR2_OPND value_opnd;
    bool value_opnd_is_temp = false;
    if (ir1_opnd_is_8l(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_zx(ir1_get_opnd(pir1, 0), 8)) {
        /* the other bits of the target register is 0, so we can set the */
        /* register directly */
        value_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    } else {
        value_opnd = ra_alloc_itemp();
        value_opnd_is_temp = true;
    }

#ifdef CONFIG_LATX_FLAG_PATTERN
    /* 2. set the value operand */
    if (!fp_translate_pattern_tail(pir1, value_opnd)) {
#endif
        IR2_OPND of_opnd = ra_alloc_itemp();
        get_eflag_condition(&of_opnd, pir1);
        la_append_ir2_opnd2i_em(LISA_SRLI_W, value_opnd, of_opnd, OF_BIT_INDEX);
        ra_free_temp(of_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    /* 3. store to the dest operand */
    store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false);
    if (value_opnd_is_temp)
        ra_free_temp(value_opnd);
    return true;
#endif
}

bool translate_setno(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_setno(pir1);
#else
    /* 1. prepare the value operand */
    IR2_OPND value_opnd;
    bool value_opnd_is_temp = false;
    if (ir1_opnd_is_8l(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_zx(ir1_get_opnd(pir1, 0), 8)) {
        /* the other bits of the target register is 0, so we can set the */
        /* register directly */
        value_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    } else {
        value_opnd = ra_alloc_itemp();
        value_opnd_is_temp = true;
    }

#ifdef CONFIG_LATX_FLAG_PATTERN
    /* 2. set the value operand */
    if (!fp_translate_pattern_tail(pir1, value_opnd)) {
#endif
        IR2_OPND of_opnd = ra_alloc_itemp();
        get_eflag_condition(&of_opnd, pir1);
        la_append_ir2_opnd2i_em(LISA_SLTUI, value_opnd, of_opnd, 1);
        ra_free_temp(of_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    /* 3. store to the dest operand */
    store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false);
    if (value_opnd_is_temp)
        ra_free_temp(value_opnd);
    return true;
#endif
}

bool translate_setb(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_setb(pir1);
#else
    /* 1. prepare the value operand */
    IR2_OPND value_opnd;
    bool value_opnd_is_temp = false;
    if (ir1_opnd_is_8l(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_zx(ir1_get_opnd(pir1, 0), 8)) {
        /* the other bits cf the target register is 0, so we can set the */
        /* register directly */
        value_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    } else {
        value_opnd = ra_alloc_itemp();
        value_opnd_is_temp = true;
    }

    /* 2. set the value operand */
#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, value_opnd)) {
#endif
        get_eflag_condition(&value_opnd, pir1);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    /* 3. store to the dest operand */
    store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false);
    if (value_opnd_is_temp)
        ra_free_temp(value_opnd);
    return true;
#endif
}

bool translate_setae(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_setae(pir1);
#else
    /* 1. prepare the value operand */
    IR2_OPND value_opnd;
    bool value_opnd_is_temp = false;
    if (ir1_opnd_is_8l(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_zx(ir1_get_opnd(pir1, 0), 8)) {
        /* the other bits cf the target register is 0, so we can set the */
        /* register directly */
        value_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    } else {
        value_opnd = ra_alloc_itemp();
        value_opnd_is_temp = true;
    }

    /* 2. set the value operand */
#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, value_opnd)) {
#endif
        IR2_OPND cf_opnd = ra_alloc_itemp();
        get_eflag_condition(&cf_opnd, pir1);
        la_append_ir2_opnd2i_em(LISA_SLTUI, value_opnd, cf_opnd, 1);
        ra_free_temp(cf_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    /* 3. store to the dest operand */
    store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false);
    if (value_opnd_is_temp)
        ra_free_temp(value_opnd);
    return true;
#endif
}

bool translate_setbe(IR1_INST *pir1)
{ /* cf==1 || zf==1 */
#ifdef CONFIG_SOFTMMU
    return latxs_translate_setbe(pir1);
#else
    /* 1. prepare the value operand */
    IR2_OPND value_opnd;
    bool value_opnd_is_temp = false;
    if (ir1_opnd_is_8l(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_zx(ir1_get_opnd(pir1, 0), 8)) {
        /* the other bits cf the target register is 0, so we can set the */
        /* register directly */
        value_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    } else {
        value_opnd = ra_alloc_itemp();
        value_opnd_is_temp = true;
    }

    /* 2. set the value operand */
#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, value_opnd)) {
#endif
        IR2_OPND cfzf_opnd = ra_alloc_itemp();
        get_eflag_condition(&cfzf_opnd, pir1);
        la_append_ir2_opnd3_em(LISA_SLTU, value_opnd, zero_ir2_opnd, cfzf_opnd);
        ra_free_temp(cfzf_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    /* 3. store to the dest operand */
    store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false);
    if (value_opnd_is_temp)
        ra_free_temp(value_opnd);
    return true;
#endif
}

bool translate_seta(IR1_INST *pir1)
{ /* cf==0 && zf==0 */
#ifdef CONFIG_SOFTMMU
    return latxs_translate_seta(pir1);
#else
    /* 1. prepare the value operand */
    IR2_OPND value_opnd;
    bool value_opnd_is_temp = false;
    if (ir1_opnd_is_8l(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_zx(ir1_get_opnd(pir1, 0), 8)) {
        /* the other bits cf the target register is 0, so we can set the */
        /* register directly */
        value_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    } else {
        value_opnd = ra_alloc_itemp();
        value_opnd_is_temp = true;
    }

    /* 2. set the value operand */
#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, value_opnd)) {
#endif
        IR2_OPND cfzf_opnd = ra_alloc_itemp();
        get_eflag_condition(&cfzf_opnd, pir1);
        la_append_ir2_opnd2i_em(LISA_SLTUI, value_opnd, cfzf_opnd, 1);
        ra_free_temp(cfzf_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    /* 3. store to the dest operand */
    store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false);
    if (value_opnd_is_temp)
        ra_free_temp(value_opnd);
    return true;
#endif
}

bool translate_sets(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_sets(pir1);
#else
    /* 1. prepare the value operand */
    IR2_OPND value_opnd;
    bool value_opnd_is_temp = false;
    if (ir1_opnd_is_8l(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_zx(ir1_get_opnd(pir1, 0), 8)) {
        /* the other bits sf the target register is 0, so we can set the */
        /* register directly */
        value_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    } else {
        value_opnd = ra_alloc_itemp();
        value_opnd_is_temp = true;
    }

    /* 2. set the value operand */
#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, value_opnd)) {
#endif
        IR2_OPND sf_opnd = ra_alloc_itemp();
        get_eflag_condition(&sf_opnd, pir1);
        la_append_ir2_opnd2i_em(LISA_SRLI_W, value_opnd, sf_opnd, SF_BIT_INDEX);
        ra_free_temp(sf_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    /* 3. store to the dest operand */
    store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false);
    if (value_opnd_is_temp)
        ra_free_temp(value_opnd);
    return true;
#endif
}

bool translate_setns(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_setns(pir1);
#else
    /* 1. prepare the value operand */
    IR2_OPND value_opnd;
    bool value_opnd_is_temp = false;
    if (ir1_opnd_is_8l(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_zx(ir1_get_opnd(pir1, 0), 8)) {
        /* the other bits sf the target register is 0, so we can set the */
        /* register directly */
        value_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    } else {
        value_opnd = ra_alloc_itemp();
        value_opnd_is_temp = true;
    }

    /* 2. set the value operand */
#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, value_opnd)) {
#endif
        IR2_OPND sf_opnd = ra_alloc_itemp();
        get_eflag_condition(&sf_opnd, pir1);
        la_append_ir2_opnd2i_em(LISA_SLTUI, value_opnd, sf_opnd, SF_BIT_INDEX);
        ra_free_temp(sf_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    /* 3. store to the dest operand */
    store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false);
    if (value_opnd_is_temp)
        ra_free_temp(value_opnd);
    return true;
#endif
}

bool translate_setp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_setp(pir1);
#else
    /* 1. prepare the value operand */
    IR2_OPND value_opnd;
    bool value_opnd_is_temp = false;
    if (ir1_opnd_is_8l(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_zx(ir1_get_opnd(pir1, 0), 8)) {
        /* the other bits pf the target register is 0, so we can set the */
        /* register directly */
        value_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    } else {
        value_opnd = ra_alloc_itemp();
        value_opnd_is_temp = true;
    }

    /* 2. set the value operand */
#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, value_opnd)) {
#endif
        IR2_OPND pf_opnd = ra_alloc_itemp();
        get_eflag_condition(&pf_opnd, pir1);
        la_append_ir2_opnd2i_em(LISA_SRLI_W, value_opnd, pf_opnd, PF_BIT_INDEX);
        ra_free_temp(pf_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    /* 3. store to the dest operand */
    store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false);
    if (value_opnd_is_temp)
        ra_free_temp(value_opnd);
    return true;
#endif
}

bool translate_setnp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_setnp(pir1);
#else
    /* 1. prepare the value operand */
    IR2_OPND value_opnd;
    bool value_opnd_is_temp = false;
    if (ir1_opnd_is_8l(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_zx(ir1_get_opnd(pir1, 0), 8)) {
        /* the other bits pf the target register is 0, so we can set the */
        /* register directly */
        value_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    } else {
        value_opnd = ra_alloc_itemp();
        value_opnd_is_temp = true;
    }

    /* 2. set the value operand */
#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, value_opnd)) {
#endif
        IR2_OPND pf_opnd = ra_alloc_itemp();
        get_eflag_condition(&pf_opnd, pir1);
        la_append_ir2_opnd2i_em(LISA_SLTUI, value_opnd, pf_opnd, 1);
        ra_free_temp(pf_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    /* 3. store to the dest operand */
    store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false);
    if (value_opnd_is_temp)
        ra_free_temp(value_opnd);
    return true;
#endif
}

bool translate_setl(IR1_INST *pir1)
{ /* sf != of */
#ifdef CONFIG_SOFTMMU
    return latxs_translate_setl(pir1);
#else
    /* 1. prepare the value operand */
    IR2_OPND value_opnd;
    bool value_opnd_is_temp = false;
    if (ir1_opnd_is_8l(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_zx(ir1_get_opnd(pir1, 0), 8)) {
        /* the other bits pf the target register is 0, so we can set the */
        /* register directly */
        value_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    } else {
        value_opnd = ra_alloc_itemp();
        value_opnd_is_temp = true;
    }

    /* 2. set the value operand */
#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, value_opnd)) {
#endif
        get_eflag_condition(&value_opnd, pir1);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    /* 3. store to the dest operand */
    store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false);
    if (value_opnd_is_temp)
        ra_free_temp(value_opnd);
    return true;
#endif
}

bool translate_setge(IR1_INST *pir1)
{ /* sf == of */
#ifdef CONFIG_SOFTMMU
    return latxs_translate_setge(pir1);
#else
    /* 1. prepare the value operand */
    IR2_OPND value_opnd;
    bool value_opnd_is_temp = false;
    if (ir1_opnd_is_8l(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_zx(ir1_get_opnd(pir1, 0), 8)) {
        /* the other bits pf the target register is 0, so we can set the */
        /* register directly */
        value_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    } else {
        value_opnd = ra_alloc_itemp();
        value_opnd_is_temp = true;
    }

    /* 2. set the value operand */
#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, value_opnd)) {
#endif
        get_eflag_condition(&value_opnd, pir1);
        la_append_ir2_opnd2i_em(LISA_SLTUI, value_opnd, value_opnd, 1);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    /* 3. store to the dest operand */
    store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false);
    if (value_opnd_is_temp)
        ra_free_temp(value_opnd);
    return true;
#endif
}

bool translate_setle(IR1_INST *pir1)
{ /* zf==1 || sf!=of */
#ifdef CONFIG_SOFTMMU
    return latxs_translate_setle(pir1);
#else
    /* 1. prepare the value operand */
    IR2_OPND value_opnd;
    bool value_opnd_is_temp = false;
    if (ir1_opnd_is_8l(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_zx(ir1_get_opnd(pir1, 0), 8)) {
        /* the other bits pf the target register is 0, so we can set the */
        /* register directly */
        value_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    } else {
        value_opnd = ra_alloc_itemp();
        value_opnd_is_temp = true;
    }

    /* 2. set the value operand */
#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, value_opnd)) {
#endif
        get_eflag_condition(&value_opnd, pir1);

        la_append_ir2_opnd3_em(LISA_SLTU, value_opnd, zero_ir2_opnd, value_opnd);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    /* 3. store to the dest operand */
    store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false);
    if (value_opnd_is_temp)
        ra_free_temp(value_opnd);
    return true;
#endif
}

bool translate_setg(IR1_INST *pir1)
{ /* zf==0 && sf==of */
#ifdef CONFIG_SOFTMMU
    return latxs_translate_setg(pir1);
#else
    /* 1. prepare the value operand */
    IR2_OPND value_opnd;
    bool value_opnd_is_temp = false;
    if (ir1_opnd_is_8l(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_zx(ir1_get_opnd(pir1, 0), 8)) {
        /* the other bits pf the target register is 0, so we can set the */
        /* register directly */
        value_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    } else {
        value_opnd = ra_alloc_itemp();
        value_opnd_is_temp = true;
    }

    /* 2. set the value operand */
#ifdef CONFIG_LATX_FLAG_PATTERN
    if (!fp_translate_pattern_tail(pir1, value_opnd)) {
#endif
        get_eflag_condition(&value_opnd, pir1);

        la_append_ir2_opnd2i_em(LISA_SLTUI, value_opnd, value_opnd, 1);
#ifdef CONFIG_LATX_FLAG_PATTERN
    }
#endif

    /* 3. store to the dest operand */
    store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false);
    if (value_opnd_is_temp)
        ra_free_temp(value_opnd);
    return true;
#endif
}

bool translate_bsf(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_bsf(pir1);
#else
    IR2_OPND src_opnd =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
    IR2_OPND count = ra_alloc_itemp();
    IR2_OPND t_opnd = ra_alloc_itemp();

    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    //set zf
    if (ir1_is_zf_def(pir1)) {
        la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x8);
    }
    la_append_ir2_opnd3(LISA_BEQ, src_opnd, zero_ir2_opnd, label_exit);

    la_append_ir2_opnd2i_em(LISA_ORI, count, zero_ir2_opnd, 0);
    IR2_OPND label_1 = ir2_opnd_new_type(IR2_OPND_LABEL);
    /* label_1 */
    la_append_ir2_opnd1(LISA_LABEL, label_1);

    la_append_ir2_opnd3_em(LISA_SRL_D, t_opnd, src_opnd, count);
    la_append_ir2_opnd2i_em(LISA_ANDI, t_opnd, t_opnd, 1);
    IR2_OPND label_2 = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BNE, t_opnd, zero_ir2_opnd, label_2);

    la_append_ir2_opnd2i_em(LISA_ADDI_D, count, count, 1);
    la_append_ir2_opnd1(LISA_B, label_1);

    la_append_ir2_opnd1(LISA_LABEL, label_2);
    store_ireg_to_ir1(count, ir1_get_opnd(pir1, 0), false); 
    if (ir1_is_zf_def(pir1)) {
        la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x8);
    }
    
    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(count);
    ra_free_temp(t_opnd);
    return true;
#endif
}

bool translate_bsr(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_bsr(pir1);
#else
    IR2_OPND count = ra_alloc_itemp();
    IR2_OPND t_opnd = ra_alloc_itemp();
    IR2_OPND src_opnd =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);

    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    //set zf
    if (ir1_is_zf_def(pir1)) {
        la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x8);
    }

    la_append_ir2_opnd3(LISA_BEQ, src_opnd, zero_ir2_opnd, label_exit);

    la_append_ir2_opnd2i_em(LISA_ORI, count, zero_ir2_opnd,
                      ir1_opnd_size(ir1_get_opnd(pir1, 0) + 1) - 1);
    IR2_OPND label_1 = ir2_opnd_new_type(IR2_OPND_LABEL);
    /* label_1 */
    la_append_ir2_opnd1(LISA_LABEL, label_1);

    la_append_ir2_opnd3_em(LISA_SRL_D, t_opnd, src_opnd, count);
    la_append_ir2_opnd2i_em(LISA_ANDI, t_opnd, t_opnd, 1);
    IR2_OPND label_2 = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BNE, t_opnd, zero_ir2_opnd, label_2);

    la_append_ir2_opnd2i_em(LISA_ADDI_D, count, count, -1);
    la_append_ir2_opnd1(LISA_B, label_1);

    /* label_2 */
    la_append_ir2_opnd1(LISA_LABEL, label_2);
    store_ireg_to_ir1(count, ir1_get_opnd(pir1, 0), false); 
    if (ir1_is_zf_def(pir1)) {
        la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x8);
    }

    /* label_exit */
    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(count);
    ra_free_temp(t_opnd);
    return true;
#endif
}

bool translate_btx(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_btx(pir1);
#else
    IR2_OPND bit_opnd = ra_alloc_itemp();
    IR2_OPND bit_offset = ra_alloc_itemp();
    IR2_OPND eflag_opnd = ra_alloc_itemp();
    IR2_OPND zf_opnd = ra_alloc_itemp();
    IR2_OPND src_opnd_0;
    IR2_OPND mem_opnd;
    IR2_OPND lat_lock_addr;
    /* imm will not be used, if opnd1 is a ireg.*/
    int imm = 0x0;
    int t_imm = 0x1f;

    IR2_OPND src_opnd_1 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
    la_append_ir2_opnd2i_em(LISA_ANDI, bit_offset, src_opnd_1, t_imm);

    /* read src_opnd_0 */
    if (ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0))) {
        src_opnd_0 = convert_gpr_opnd(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION);
    } else {
        src_opnd_0 = ra_alloc_itemp();
        mem_opnd = mem_ir1_to_ir2_opnd(ir1_get_opnd(pir1, 0), false);
        imm = ir2_opnd_imm(&mem_opnd);
        mem_opnd._type = IR2_OPND_IREG;

        if (ir1_is_prefix_lock(pir1)) {
		    lat_lock_addr = tr_lat_spin_lock(mem_opnd, imm);
        }
        la_append_ir2_opnd2i_em(LISA_LD_W, src_opnd_0, mem_opnd, imm);
    }

    /* for set eflag */
    la_append_ir2_opnd3_em(LISA_SRL_D, eflag_opnd, src_opnd_0, bit_offset);

    la_append_ir2_opnd2i_em(LISA_ORI, bit_opnd, zero_ir2_opnd, 1);
    la_append_ir2_opnd3_em(LISA_SLL_D, bit_opnd, bit_opnd, bit_offset);
    switch (ir1_opcode(pir1)) {
    case X86_INS_BT:
        break;
    case X86_INS_BTS:
        la_append_ir2_opnd3_em(LISA_OR, src_opnd_0, src_opnd_0, bit_opnd);
        break;
    case X86_INS_BTR:
        la_append_ir2_opnd3_em(LISA_NOR, bit_opnd, bit_opnd, zero_ir2_opnd);
        la_append_ir2_opnd3_em(LISA_AND, src_opnd_0, src_opnd_0, bit_opnd);
        break;
    case X86_INS_BTC:
        la_append_ir2_opnd3_em(LISA_XOR, src_opnd_0, src_opnd_0, bit_opnd);
        break;
    default:
        lsassertm(0, "Invalid opcode in translate_btx\n");
    }

    /* Write back to memory */
    if (ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0))) {
        store_ireg_to_ir1(src_opnd_0, ir1_get_opnd(pir1, 0), false);
    } else {
        la_append_ir2_opnd2i(LISA_ST_W, src_opnd_0, mem_opnd, imm);
        ra_free_temp(src_opnd_0);
    }

    la_append_ir2_opnd2i_em(LISA_ANDI, eflag_opnd, eflag_opnd, 1);
    la_append_ir2_opnd1i_em(LISA_X86MFFLAG, zf_opnd, 0x8);
    la_append_ir2_opnd3_em(LISA_OR, eflag_opnd, eflag_opnd, zf_opnd);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, eflag_opnd, 0x3f);

    if (ir1_is_prefix_lock(pir1)) {
        tr_lat_spin_unlock(lat_lock_addr);
    }
    ra_free_temp(zf_opnd);
    ra_free_temp(bit_opnd);
    ra_free_temp(bit_offset);
    ra_free_temp(eflag_opnd);
    return true;
#endif
}
