#include "common.h"
#include "reg-alloc.h"
#include "lsenv.h"
#include <string.h>
#include "latx-options.h"
#include "translate.h"

IR2_OPND ra_alloc_gpr(int gpr_num)
{
//    static const int8 rai_imap_gpr[8] = {15, 3, 1, 19, 20, 21, 22, 23};
    /*
     * Mapping to LA regs.
     */
    static const int8 rai_imap_gpr[8] = {15, 18, 19, 26, 27, 28, 29, 30};

    IR2_OPND opnd;
    ir2_opnd_build(&opnd, IR2_OPND_GPR, rai_imap_gpr[gpr_num]);

    return opnd;
}

IR2_OPND ra_alloc_mda(void)
{
    IR2_OPND opnd;
    /*MIPS R31 ---> LA R1*/
    ir2_opnd_build(&opnd, IR2_OPND_IREG, 1);
    return opnd;
}

/* to improve efficiency, we map some frequently used variables
 * to host registers:
 *   bit 0: guest base, mapped to $14, t2; should be valid for all native code
 *   bit 1: last executed tb, mapped to $24, t8; register not always valid
 *   bit 2: next x86 addr, mapped to $25; registerk not always valid
 *   bit 3: top_bias, not mapped to register
 *   bit 4: shadow_stack
 * they can be saved to and loaded from lsenv->cpu_state->vregs
 */
IR2_OPND ra_alloc_vreg(int vr_num)
{
    //static const int8 rai_imap_vr[5] = {14, 24, 25, 28, 17};
    /*
     * Mapping to LA directly.
     */
    static const int8 rai_imap_vr[5] = {14, 20, 16, 31, 24};

    IR2_OPND opnd;

    lsassert(vr_num >= 0 && vr_num <= 4);

    ir2_opnd_build(&opnd, IR2_OPND_IREG, rai_imap_vr[vr_num]);

    return opnd;
}

IR2_OPND ra_alloc_guest_base(void) { return ra_alloc_vreg(0); }

IR2_OPND ra_alloc_dbt_arg1(void) { return ra_alloc_vreg(1); }

IR2_OPND ra_alloc_dbt_arg2(void) { return ra_alloc_vreg(2); }

IR2_OPND ra_alloc_flag_pattern_saved_opnd0(void) { return ra_alloc_vreg(1); }

IR2_OPND ra_alloc_flag_pattern_saved_opnd1(void) { return ra_alloc_vreg(2); }

/* Mapping to LA 17->24*/
IR2_OPND ra_alloc_ss(void)
{
    IR2_OPND ir2_ss;
    ir2_opnd_build(&ir2_ss, IR2_OPND_IREG, 24);
    return ir2_ss;
}
/* Mapping to LA 18->25*/
IR2_OPND ra_alloc_env(void) { return ir2_opnd_new(IR2_OPND_IREG, 25); }
/* Mapping to LA 28->31*/
IR2_OPND ra_alloc_top(void) { return ir2_opnd_new(IR2_OPND_IREG, 31); }

/* Mapping to LA 30->22*/
IR2_OPND ra_alloc_eflags(void) { return ir2_opnd_new(IR2_OPND_IREG, 22); }
/*
 * FIXME: What is the freg mapping rules for LA?
 */
IR2_OPND ra_alloc_f32(void) { return ir2_opnd_new(IR2_OPND_FREG, 8); }

IR2_OPND ra_alloc_st(int st_num)
{
    if (option_lsfpu) {
        static const int8 rai_fmap_mmx[8] = {0, 1, 2, 3, 4, 5, 6, 7};
        return ir2_opnd_new(IR2_OPND_FREG, rai_fmap_mmx[st_num]);
    } else {
        static const int8 rai_fmap_mmx[8] = {0, 1, 2, 3, 4, 5, 6, 7};
        int fpr_num = (lsenv->tr_data->curr_top + st_num) & 7;
        return ir2_opnd_new(IR2_OPND_FREG, rai_fmap_mmx[fpr_num]);
    }
}

IR2_OPND ra_alloc_mmx(int mmx_num)
{
    if (option_lsfpu) {
        static const int8 rai_fmap_mmx[8] = {0, 1, 2, 3, 4, 5, 6, 7};
        return ir2_opnd_new(IR2_OPND_FREG, rai_fmap_mmx[mmx_num]);
    } else {
        static const int8 rai_fmap_mmx[8] = {0, 1, 2, 3, 4, 5, 6, 7};
        return ir2_opnd_new(IR2_OPND_FREG, rai_fmap_mmx[mmx_num]);
    }
}

IR2_OPND ra_alloc_xmm_lo(int xmm_lo_num)
{
    lsassert(!option_xmm128map);
    static const int8 rai_fmap_xmm_lo[8] = {16, 18, 20, 22, 24, 26, 28, 30};
    return ir2_opnd_new(IR2_OPND_FREG, rai_fmap_xmm_lo[xmm_lo_num]);
}

IR2_OPND ra_alloc_xmm_hi(int xmm_hi_num)
{
    lsassert(!option_xmm128map);
    static const int8 rai_fmap_xmm_hi[8] = {17, 19, 21, 23, 25, 27, 29, 31};
    return ir2_opnd_new(IR2_OPND_FREG, rai_fmap_xmm_hi[xmm_hi_num]);
}

IR2_OPND ra_alloc_xmm(int num) {
    lsassert(option_xmm128map);
    lsassert(0 <= num && num <=7);
    return ir2_opnd_new(IR2_OPND_FREG, 16 + num);//replace by shell script
}

IR2_OPND ra_alloc_itemp(void)
{
#if defined(CONFIG_LATX_DEBUG)
    int itemp_index = (lsenv->tr_data->itemp_num)++;
#endif
    IR2_OPND ir2_opnd;
#if defined(CONFIG_LATX_DEBUG)
    lsassert(itemp_index < itemp_status_num);
#endif
    int itemp_num = -1;
    for (int i = 0; i < itemp_status_num; ++i) {
        if (!lsenv->tr_data->itemp_status[i].is_used) {
            lsenv->tr_data->itemp_status[i].is_used = true;
            itemp_num = lsenv->tr_data->itemp_status[i].physical_id;
            break;
        }
    }
    lsassertm(itemp_num != -1, "ra_alloc_itemp failed");

    lsenv->tr_data->ireg_em[itemp_num] = UNKNOWN_EXTENSION;
    lsenv->tr_data->ireg_eb[itemp_num] = 32;

    ir2_opnd_build(&ir2_opnd, IR2_OPND_IREG, itemp_num);
    return ir2_opnd;
}

IR2_OPND ra_alloc_ftemp(void)
{
#if defined(CONFIG_LATX_DEBUG)
    int ftemp_index = (lsenv->tr_data->ftemp_num)++;
    lsassert(ftemp_index < ftemp_status_num);
#endif
    int ftemp_num = -1;
    for (int i = 0; i < ftemp_status_num; ++i) {
        if (!lsenv->tr_data->ftemp_status[i].is_used) {
            lsenv->tr_data->ftemp_status[i].is_used = true;
            ftemp_num = lsenv->tr_data->ftemp_status[i].physical_id;
            break;
        }
    }
    lsassertm(ftemp_num != -1, "ra_alloc_ftemp failed");

    return ir2_opnd_new(IR2_OPND_FREG, ftemp_num);
}

IR2_OPND ra_alloc_itemp_internal(void) { return ra_alloc_itemp(); }

IR2_OPND ra_alloc_ftemp_internal(void) { return ra_alloc_ftemp(); }

void ra_free_temp(IR2_OPND opnd)
{
    if (ir2_opnd_is_itemp(&opnd)) {
        ra_free_itemp(opnd._reg_num);
    } else if (ir2_opnd_is_ftemp(&opnd)) {
        ra_free_ftemp(opnd._reg_num);
    } else {
        lsassertm(0, "attempt to free a non-temp register");
    }
}

void ra_free_all_internal_temp(void)
{
    /* reset itemp_num*/
    TRANSLATION_DATA *tr_data = lsenv->tr_data;

    ra_init();
    tr_data->itemp_num = 0;
    tr_data->ftemp_num = 0;
}

void ra_free_itemp(int phy_id)
{
#if defined(CONFIG_LATX_DEBUG)
    bool freed = false;
#endif
    for (int i = 0; i < itemp_status_num; ++i) {
        if (lsenv->tr_data->itemp_status[i].physical_id == phy_id) {
            lsassertm(lsenv->tr_data->itemp_status[i].is_used,
                      "attempt to free a unallocated itemp register %d", phy_id);
            lsenv->tr_data->itemp_status[i].is_used = false;
            lsenv->tr_data->itemp_num--;
#if defined(CONFIG_LATX_DEBUG)
            freed = true;
#endif
            break;
        }
    }
#if defined(CONFIG_LATX_DEBUG)
    lsassertm(freed, "ra_free_itemp failed");
#endif
}

void ra_free_ftemp(int phy_id)
{
#if defined(CONFIG_LATX_DEBUG)
    bool freed = false;
#endif
    for (int i = 0; i < ftemp_status_num; ++i) {
        if (lsenv->tr_data->ftemp_status[i].physical_id == phy_id) {
            lsassertm(lsenv->tr_data->ftemp_status[i].is_used,
                      "attempt to free a unallocated ftemp register %d", phy_id);
            lsenv->tr_data->ftemp_status[i].is_used = false;
            lsenv->tr_data->ftemp_num--;
#if defined(CONFIG_LATX_DEBUG)
            freed = true;
#endif
            break;
        }
    }
#if defined(CONFIG_LATX_DEBUG)
    lsassertm(freed, "ra_free_ftemp failed");
#endif
}

EXTENSION_MODE ir2_opnd_default_em(IR2_OPND *opnd)
{
    lsassert(ir2_opnd_is_ireg(opnd));
#ifdef N64 /* validate address */
    IR2_OPND ir2_esp = ra_alloc_gpr(esp_index);
    IR2_OPND ir2_ebp = ra_alloc_gpr(ebp_index);
    if (ir2_opnd_cmp(opnd, &ir2_esp))
        return EM_X86_ADDRESS;
    else if (ir2_opnd_cmp(opnd, &ir2_ebp))
        return ZERO_EXTENSION;
    else
        return SIGN_EXTENSION;
#else
    return SIGN_EXTENSION;
#endif
}

void ra_init(void)
{
    memcpy(lsenv->tr_data->itemp_status, itemp_status_default,
           sizeof(itemp_status_default));
    memcpy(lsenv->tr_data->ftemp_status, ftemp_status_default,
           sizeof(ftemp_status_default));
}

#ifdef CONFIG_SOFTMMU

void latxs_tr_ra_init(TRANSLATION_DATA *td)
{
    td->itemp_num = 32;
    td->ftemp_num = 32;

    td->itemp_mask = 0;
    td->ftemp_mask = 0;
}


/*
 * Integer Register Mapping of LATX-SYS
 * -------------------------------------------------------------------
 *  0 zero
 *  1 ra
 *  2 v0
 *  3 v1
 * -------------------------------------------------------------------
 *  4 a0/v0
 *  5 a1/v1
 *  6 a2
 *  7 a3
 * -------------------------------------------------------------------
 *  8 a4    STMP   : static temp registers
 *  9 a5    STMP   : static temp registers
 * 10 a6    TB     : DBT arg1 = address(HVA) of current executing TB
 * 11 a7    EIP    : DBT arg2 = pc(GVA) of next TB
 * -------------------------------------------------------------------
 * 12 t0    TMP    : temp registers
 * 13 t1    TMP    : temp registers
 * 14 t2    TMP    : temp registers
 * 15 t3    TMP    : temp registers
 * -------------------------------------------------------------------
 * 16 t4    TMP    : temp registers
 * 17 t5    TMP    : temp registers
 * 18 t6    TMP    : temp registers
 * 19 t7    TMP    : temp registers
 * -------------------------------------------------------------------
 * 20 t8    G-B    : guest base (used ONLY in user-mode)
 * 21 x
 * 22 fp           : not used
 * 23 s0    CPU    : address(HVA) of CPUX86State
 * -------------------------------------------------------------------
 * 24 s1    EAX    : x86 mapping registers eax
 * 25 s2    ECX    : x86 mapping registers ecx
 * 26 s3    EDX    : x86 mapping registers edx
 * 27 s4    EBX    : x86 mapping registers ebx
 * -------------------------------------------------------------------
 * 28 s5    ESP    : x86 mapping registers esp
 * 29 s6    EBP    : x86 mapping registers ebp
 * 30 s7    ESI    : x86 mapping registers esi
 * 31 s8    EDI    : x86 mapping registers edi
 * -------------------------------------------------------------------
 *
 * Floating Point Register Mapping of LATX-sys is the same with LATX-user
 */

/* allocate x86 register */

IR2_OPND latxs_ra_alloc_gpr(int gpr_num)
{
    lsassert(gpr_num >= 0 && gpr_num < CPU_NB_REGS);

    /* EAX - EDI => S1 - S8 */
    /* R8  - R15 => T0 - T7 */
    static const int8_t latxs_rai_imap_gpr[CPU_NB_REGS] = {
        24, 25, 26, 27, 28, 29, 30, 31,
#ifdef TARGET_X86_64
        12, 13, 14, 15, 16, 17, 18, 19
#endif
    };

    IR2_OPND opnd;
    latxs_ir2_opnd_build(&opnd, IR2_OPND_GPR, latxs_rai_imap_gpr[gpr_num]);

    return opnd;
}

#define LATXS_IR2_VREG_NUM 6
IR2_OPND latxs_ra_alloc_vreg(int vr_num)
{
    lsassert(vr_num >= 0 && vr_num < LATXS_IR2_VREG_NUM);

    /*
     * bit 0: guest base, not used in system mode
     * bit 1: last executed tb, mapped to $10, a6; register not always valid
     * bit 2: next x86 addr, mapped to $11; registerk not always valid
     * bit 3: top_bias, not mapped to register
     * bit 4: shadow stack, not used in system mode
     * bit 5: FastCS mask, not mapped to resgiter, not supported yet
     */

    static const int8_t latxs_rai_imap_vr[LATXS_IR2_VREG_NUM] = {
    0, 10, 0, 0, 0, 0};

    int reg = latxs_rai_imap_vr[vr_num];
    lsassert(reg);

    IR2_OPND opnd = latxs_ir2_opnd_new(IR2_OPND_GPR, reg);
    return opnd;
}

IR2_OPND latxs_ra_alloc_mmx(int mmx_num)
{
    static const int8_t latxs_rai_fmap_mmx[8] = {
    0, 1, 2, 3, 4, 5, 6, 7};

    return latxs_ir2_opnd_new(IR2_OPND_FPR, latxs_rai_fmap_mmx[mmx_num]);
}

IR2_OPND latxs_ra_alloc_st(int st_num)
{
    lsassert(!option_soft_fpu);
    static const int8_t latxs_rai_fmap_mmx[8] = {
    0, 1, 2, 3, 4, 5, 6, 7};

    int fpr_num = st_num;
    if (!option_lsfpu) {
        fpr_num = (latxs_td_fpu_get_top() + st_num) & 7;
    }

    return latxs_ir2_opnd_new(IR2_OPND_FPR, latxs_rai_fmap_mmx[fpr_num]);
}

IR2_OPND latxs_ra_alloc_xmm(int num)
{
    lsassert(0 <= num && num < CPU_NB_REGS);
    return latxs_ir2_opnd_new(IR2_OPND_FPR, 16 + num);
}

IR2_OPND latxs_ra_alloc_dbt_arg1(void) { return latxs_ra_alloc_vreg(1); }

IR2_OPND latxs_ra_alloc_itemp(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    IR2_OPND ir2_opnd;
    ir2_opnd._type = IR2_OPND_GPR;

    /* 1. allocate physical register */
    const TEMP_REG_STATUS *p = latxs_itemp_status_default;

    int mask = td->itemp_mask;
    int i = 0;
    while ((mask >> i) & 0x1) {
        i++;
    }

    if (i < latxs_itemp_status_num) {
        ir2_opnd.val = p[i].physical_id;
        td->itemp_mask = td->itemp_mask | (1 << i);
        return ir2_opnd;
    }

    lsassertm(0, "no more itemp to be alloc.\n");
    return ir2_opnd;
}

IR2_OPND latxs_ra_alloc_ftemp(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    IR2_OPND ir2_opnd;
    ir2_opnd._type = IR2_OPND_FPR;

    /* 1. allocate physical register */
    const TEMP_REG_STATUS *p = ftemp_status_default;

    uint32_t mask = td->ftemp_mask;
    int i = 0;
    while ((mask >> i) & 0x1) {
        i++;
    }

    if (i < ftemp_status_num) {
        ir2_opnd.val = p[i].physical_id;
        td->ftemp_mask = td->ftemp_mask | (1 << i);
        return ir2_opnd;
    }

    lsassertm(0, "no more ftemp to be alloc.\n");
    return ir2_opnd;
}

void latxs_ra_free_temp(IR2_OPND *opnd)
{
    if (!latxs_ir2_opnd_is_reg_temp(opnd)) {
        return;
    }

    TRANSLATION_DATA *td = lsenv->tr_data;

    const TEMP_REG_STATUS *ireg_map = latxs_itemp_status_default;
    const TEMP_REG_STATUS *freg_map = ftemp_status_default;

    int physical_reg_num = latxs_ir2_opnd_reg(opnd);

    int i = 0;
    switch (latxs_ir2_opnd_type(opnd)) {
    case IR2_OPND_GPR:
    case IR2_OPND_MEMY:
        for (i = 0; i < latxs_itemp_status_num; ++i) {
            if (ireg_map[i].physical_id == physical_reg_num) {
                td->itemp_mask = td->itemp_mask & ~(1 << i);
                break;
            }
        }
        break;
    case IR2_OPND_FPR:
        for (i = 0; i < ftemp_status_num; ++i) {
            if (freg_map[i].physical_id == physical_reg_num) {
                td->ftemp_mask = td->ftemp_mask & ~(1 << i);
                break;
            }
        }
        break;
    default:
        lsassertm(0, "free itemp opnd is not ireg,freg,mem.\n");
        break;
    }
}

#endif
