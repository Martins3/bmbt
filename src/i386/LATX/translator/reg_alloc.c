#include "../include/common.h"
#include "../include/reg_alloc.h"
#include "../include/env.h"
#include <string.h>
#include "../x86tomips-options.h"

#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
#include "../x86tomips-profile-sys.h"
#endif

TEMP_REG_STATUS itemp_status_default[itemp_status_num] = {
    {12}, {13}, {14}, {15}, /* t0, t1, t2, t3 */
    {16}, {17}, {18}, {19}  /* t4, t5, t6, t7 */
};

TEMP_REG_STATUS ftemp_status_default[ftemp_status_num] = {
    {9}, {10}, {11}, {12}, {13}, {14}, {15},
};

IR2_OPND ra_alloc_gpr(int gpr_num)
{
    /* LoongArch: S1 - S8 */
    static const int8 rai_imap_gpr[8] =
    {24, 25, 26, 27, 28, 29, 30, 31};

    IR2_OPND opnd;
    ir2_opnd_build(&opnd, IR2_OPND_GPR, rai_imap_gpr[gpr_num]);

    return opnd;
}

/* to improve efficiency, we map some frequently used variables
 * to host registers:
 *   bit 0: guest base, mapped to $20, t8; should be valid for all native code
 *   bit 1: last executed tb, mapped to $10, a6; register not always valid
 *   bit 2: next x86 addr, mapped to $11, a7; register not always valid
 *   bit 3: top_bias, not mapped to register
 *   bit 4: mask for fast context switch, not mapped to register
 * they can be saved to and loaded from lsenv->cpu_state->vregs
 */
#define IR2_VREG_NUM 5
IR2_OPND ra_alloc_vreg(int vr_num)
{
    static const int8 rai_imap_vr[IR2_VREG_NUM] =
    {20, 10, 11, 0, 0};

    lsassert(vr_num >= 0 && vr_num <= IR2_VREG_NUM);

    int reg = rai_imap_vr[vr_num];
    IR2_OPND opnd = ir2_opnd_new(IR2_OPND_GPR, reg);
    return opnd;
}

IR2_OPND ra_alloc_guest_base(void) { return ra_alloc_vreg(0); }

IR2_OPND ra_alloc_dbt_arg1(void) { return ra_alloc_vreg(1); }
IR2_OPND ra_alloc_dbt_arg2(void) { return ra_alloc_vreg(2); }

IR2_OPND ra_alloc_st(int st_num)
{
    static const int8 rai_fmap_mmx[8] =
    {0, 1, 2, 3, 4, 5, 6, 7};

    int fpr_num = st_num;
    if (!option_lsfpu) {
        fpr_num = (td_fpu_get_top() + st_num) & 7;
    }

    return ir2_opnd_new(IR2_OPND_FPR,
                        rai_fmap_mmx[fpr_num]);
}

IR2_OPND ra_alloc_mmx(int mmx_num)
{
    static const int8 rai_fmap_mmx[8] =
    {0, 1, 2, 3, 4, 5, 6, 7};

    return ir2_opnd_new(IR2_OPND_FPR,
                        rai_fmap_mmx[mmx_num]);
}

IR2_OPND ra_alloc_xmm_lo(int xmm_lo_num)
{
    lsassert(!option_xmm128map);
    static const int8 rai_fmap_xmm_lo[8] =
    {16, 18, 20, 22, 24, 26, 28, 30};
    return ir2_opnd_new(IR2_OPND_FPR,
                        rai_fmap_xmm_lo[xmm_lo_num]);
}

IR2_OPND ra_alloc_xmm_hi(int xmm_hi_num)
{
    lsassert(!option_xmm128map);
    static const int8 rai_fmap_xmm_hi[8] =
    {17, 19, 21, 23, 25, 27, 29, 31};
    return ir2_opnd_new(IR2_OPND_FPR,
                        rai_fmap_xmm_hi[xmm_hi_num]);
}

IR2_OPND ra_alloc_xmm(int num)
{
    lsassert(option_xmm128map);
    lsassert(0 <= num && num <=7);
    return ir2_opnd_new(IR2_OPND_FPR, 16 + num);
}

IR2_OPND ra_alloc_itemp(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    IR2_OPND ir2_opnd;
    ir2_opnd._type = IR2_OPND_GPR;

    /* 1. allocate physical register */
//    TEMP_REG_STATUS *p = td->itemp_status;
    TEMP_REG_STATUS *p = itemp_status_default;

    int mask = td->itemp_mask;
    int i = 0;
    while ((mask >> i) & 0x1) i++;

    if (i < itemp_status_num) {
        ir2_opnd.val = p[i].physical_id;
        td->itemp_mask = td->itemp_mask | (1 << i);

#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
        xtm_pf_inc_rs_ra_alloc_int_nr();
        td->curr_ir1_itemp_num += 1;
        xtm_pf_inc_rs_ra_int_used_max(td->curr_ir1_itemp_num);
#endif

        return ir2_opnd;
    }

    lsassertm(0, "can not alloc itemp.\n");
    return ir2_opnd;
}

IR2_OPND ra_alloc_ftemp(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    IR2_OPND ir2_opnd;
    ir2_opnd._type = IR2_OPND_FPR;

    /* 1. allocate physical register */
//    TEMP_REG_STATUS *p = td->ftemp_status;
    TEMP_REG_STATUS *p = ftemp_status_default;

    uint32_t mask = td->ftemp_mask;
    int i = 0;
    while ((mask >> i) & 0x1) i++;

    if (i < ftemp_status_num) {
        ir2_opnd.val = p[i].physical_id;
        td->ftemp_mask = td->ftemp_mask | (1 << i);

#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
        xtm_pf_inc_rs_ra_alloc_fp_nr();
        td->curr_ir1_ftemp_num += 1;
        xtm_pf_inc_rs_ra_fp_used_max(td->curr_ir1_ftemp_num);
#endif

        return ir2_opnd;
    }

    lsassertm(0, "can not alloc ftemp.\n");
    return ir2_opnd;
}

IR2_OPND ra_alloc_itemp_internal(void) { return ra_alloc_itemp(); }
IR2_OPND ra_alloc_ftemp_internal(void) { return ra_alloc_ftemp(); }

void ra_free_temp(IR2_OPND *opnd)
{
    if (!ir2_opnd_is_reg_temp(opnd)) return;

    TRANSLATION_DATA *td = lsenv->tr_data;

    TEMP_REG_STATUS *ireg_map = itemp_status_default;
    TEMP_REG_STATUS *freg_map = ftemp_status_default;

    int physical_reg_num = ir2_opnd_reg(opnd);

    int i = 0;
    switch (ir2_opnd_type(opnd)) {
    case IR2_OPND_GPR:
    case IR2_OPND_MEM:
        for (i = 0; i < itemp_status_num; ++i) {
            if (ireg_map[i].physical_id == physical_reg_num) {
                td->itemp_mask = td->itemp_mask & ~(1 << i);
#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
                xtm_pf_inc_rs_ra_free_int_nr();
                td->curr_ir1_itemp_num -= 1;
#endif
                break;
            }
        }
        break;
    case IR2_OPND_FPR:
        for (i = 0; i < ftemp_status_num; ++i) {
            if (freg_map[i].physical_id == physical_reg_num) {
                td->ftemp_mask = td->ftemp_mask & ~(1 << i);
#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
                xtm_pf_inc_rs_ra_free_fp_nr();
                td->curr_ir1_ftemp_num -= 1;
#endif
                break;
            }
        }
        break;
    default:
        lsassertm(0, "free opnd not ireg,freg,mem.\n");
        break;
    }
}
void ra_free_itemp(int i) { IR2_OPND ir2 = ir2_opnd_new(IR2_OPND_GPR, i); ra_free_temp(&ir2); }
void ra_free_ftemp(int i) { IR2_OPND ir2 = ir2_opnd_new(IR2_OPND_FPR, i); ra_free_temp(&ir2); }

int ra_get_temp_nr(void)
{
    return sizeof(itemp_status_default) / sizeof(TEMP_REG_STATUS);
}

int ra_get_temp_reg_by_idx(int idx)
{
    lsassert(idx >= 0 && idx < ra_get_temp_nr());
    return itemp_status_default[idx].physical_id;
}
