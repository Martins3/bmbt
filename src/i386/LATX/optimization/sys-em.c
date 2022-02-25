#include "lsenv.h"
#include "common.h"
#include "reg-alloc.h"
#include "flag-lbt.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>

/* Extension Mode
 *
 * Key Rules:
 *  > target GPR only has 3 * 3  - 2 + 1 = 8 types of EM
 *    +------------+------+------+------+
 *    | size \ ext | sign | zero | none |
 *    +------------+------+------+------+
 *    |       0    |         0          |
 *    +------------+------+------+------+
 *    |       8    |  8s  |  8z  |      |
 *    +------------+------+------+------+
 *    |      16    | 16s  | 16z  |      |
 *    +------------+------+------+------+
 *    |      32    | 32s  | 32z  | 32n  |
 *    +------------+------+------+------+
 *  > One key functions to maintion EM
 *    > store_ir2_to_ir1_gpr_em
 *  > One key function to use EM as optimization
 *    > convert_gpr_opnd
 *  > Enable EM analysis through option byhand
 */

IR2_OPND latxs_convert_gpr_opnd(IR1_OPND *opnd1, EXMode em)
{
    lsassert(ir1_opnd_is_gpr(opnd1));
    int gpr_num = ir1_opnd_base_reg_num(opnd1);
    int opnd_size = ir1_opnd_size(opnd1);

    /*
     * what we need:
     * > EXMode is @em
     * > EXBits is @opnd_size
     */

    EXMode gpr_em = latxs_td_get_reg_extm(gpr_num);
    EXBits gpr_eb = latxs_td_get_reg_extb(gpr_num);

    IR2_OPND gpr = latxs_ra_alloc_gpr(gpr_num);

    /* 8H could not be used directly! */
    if (!ir1_opnd_is_8h(opnd1)) {

        /* GPR contains what we need */
        if (gpr_em == em && gpr_eb <= opnd_size) {
            return gpr;
        }

        /* 16z and 8z are still 32s ; 8z is still 16s */
        if (em == EXMode_S && gpr_em == EXMode_Z && gpr_eb < opnd_size) {
            return gpr;
        }

        /* We actually need any extension */
        if (em == EXMode_N) {
            return gpr;
        }

        if (gpr_eb == 0) {
            return gpr;
        }
    }

    IR2_OPND tmp = latxs_ra_alloc_itemp();
    switch (opnd_size) {
    case 32: {
        if (em == EXMode_S) {
            latxs_append_ir2_opnd2_(lisa_mov32s, &tmp, &gpr);
        } else if (em == EXMode_Z) {
            latxs_append_ir2_opnd2_(lisa_mov32z, &tmp, &gpr);
        } else {
            latxs_append_ir2_opnd2_(lisa_mov, &tmp, &gpr);
        }
        break;
    }
    case 16: {
        if (em == EXMode_S) {
            latxs_append_ir2_opnd2_(lisa_mov16s, &tmp, &gpr);
        } else if (em == EXMode_Z) {
            latxs_append_ir2_opnd2_(lisa_mov16z, &tmp, &gpr);
        } else {
            latxs_append_ir2_opnd2_(lisa_mov, &tmp, &gpr);
        }
        break;
    }
    case 8: {
        if (ir1_opnd_is_8h(opnd1)) {
            if (em == EXMode_S) {
                latxs_append_ir2_opnd2i(LISA_SRLI_D, &tmp, &gpr, 8);
                latxs_append_ir2_opnd2(LISA_EXT_W_B, &tmp, &tmp);
            } else if (em == EXMode_Z) {
                latxs_append_ir2_opnd2ii(LISA_BSTRPICK_D, &tmp, &gpr, 15, 8);
            } else {
                latxs_append_ir2_opnd2i(LISA_SRLI_D, &tmp, &gpr, 8);
            }
        } else { /* ir1_opnd_is_8l */
            if (em == EXMode_S) {
                latxs_append_ir2_opnd2_(lisa_mov8s, &tmp, &gpr);
            } else if (em == EXMode_Z) {
                latxs_append_ir2_opnd2_(lisa_mov8z, &tmp, &gpr);
            } else {
                latxs_append_ir2_opnd2_(lisa_mov, &tmp, &gpr);
            }
        }
        break;
    }
    default:
        lsassertm(0, "convert gpr opnd fail\n");
        break;
    }

    return tmp;
}

void latxs_store_ir2_to_ir1_gpr_em(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    lsassert(ir1_opnd_is_gpr(opnd1) && latxs_ir2_opnd_is_gpr(opnd2));
    int gpr_num = ir1_opnd_base_reg_num(opnd1);
    IR2_OPND gpr_opnd = latxs_ra_alloc_gpr(gpr_num);
    int opnd_size = ir1_opnd_size(opnd1);

    EXMode gpr_em = latxs_td_get_reg_extm(gpr_num);
    EXBits gpr_eb = latxs_td_get_reg_extb(gpr_num);

    if (opnd_size == 32) {
        if (!latxs_ir2_opnd_cmp(opnd2, &gpr_opnd)) {
            /* Always set to deault EM: 32-bit signed extension */
            latxs_append_ir2_opnd2_(lisa_mov32s, &gpr_opnd, opnd2);
            latxs_td_set_reg_extmb(gpr_num, EXMode_S, 32);
        } else {
            /*
             * If IR2 opnd is already the mapping register,
             * its extension mode should be maintained outside.
             */
            lsassertm(0, "IR2 opnd should not be X86 GPR in %s.\n", __func__);
        }
    } else if (opnd_size == 16) {
        latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &gpr_opnd, opnd2, 15, 0);
        if (gpr_eb <= 16 && gpr_em == EXMode_Z) {
            latxs_td_set_reg_extmb(gpr_num, EXMode_Z, 16);
        } else if (gpr_eb <= 16 && gpr_em == EXMode_S) {
            latxs_td_set_reg_extmb(gpr_num, EXMode_N, 32);
        }
    } else if (ir1_opnd_is_8h(opnd1)) {
        /*
         * Here we could modify [8,15] directly
         * For X86 GPR:
         *  > If it contains more than 16-bits data (aka. 32-bit)
         *    Its EM will not be changed.
         *  > If it contains 8-bits or 16-bits
         *    > If EM is ZERO, it will still be ZERO extension
         *    > If EM is NONE, it will still be NONE extension
         *    > If EM is SIGN, it might not be SIGN extension anymore
         *      So we set it to NONE extension
         */
        latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &gpr_opnd, opnd2, 15, 8);
        if (gpr_eb <= 16 && gpr_em == EXMode_Z) {
            latxs_td_set_reg_extmb(gpr_num, EXMode_Z, 16);
        } else if (gpr_eb <= 16 && gpr_em == EXMode_S) {
            latxs_td_set_reg_extmb(gpr_num, EXMode_N, 32);
        }
    } else {
        latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &gpr_opnd, opnd2,  7, 0);
        if (gpr_eb <= 8 && gpr_em == EXMode_Z) {
            latxs_td_set_reg_extmb(gpr_num, EXMode_Z, 8);
        } else if (gpr_eb <= 8 && gpr_em == EXMode_S) {
            latxs_td_set_reg_extmb(gpr_num, EXMode_N, 32);
        }
    }
}

void latxs_load_ir1_imm_to_ir2_em(IR2_OPND *opnd2,
        IR1_OPND *opnd1, EXMode em)
{
    lsassert(option_by_hand);
    lsassert(latxs_ir2_opnd_is_gpr(opnd2));
    lsassert(ir1_opnd_is_imm(opnd1));

    uint32_t value = 0;
    if (em == EXMode_Z) {
        value = ir1_opnd_uimm(opnd1);
    } else {
        value = ir1_opnd_simm(opnd1);
    }

    int16_t high_16_bits = value >> 16;
    int16_t low_16_bits = value;
    int64_t imm = 0;

    if (!value) {
        latxs_append_ir2_opnd2_(lisa_mov, opnd2, &zero_ir2_opnd);
        latxs_ir2_opnd_set_emb(opnd2, EXMode_Z, 0);
        return;
    }

    if (high_16_bits == 0) {
        /* 1. should be 0000 0000 0000 xxxx */
        imm = low_16_bits;
        latxs_load_imm64(opnd2, imm & 0xFFFF);
        if (option_by_hand) {
            if (!(imm >> 8)) {
                latxs_ir2_opnd_set_emb(opnd2, EXMode_Z, 8);
            } else  {
                latxs_ir2_opnd_set_emb(opnd2, EXMode_Z, 16);
            }
        }
        return;
    } else if (high_16_bits == -1) {
        if (em == EXMode_Z) {
            /* 2. should be 0000 0000 ffff xxxx */
            imm = value;
            imm = imm & 0xFFFFFFFF;
            lsassertm((imm & 0x00000000FFFF0000) == 0xFFFF0000,
                     "value=0x%x, imm = 0x%lx\n", value, imm);
            latxs_load_imm64(opnd2, imm);
            if (option_by_hand) {
                latxs_ir2_opnd_set_emb(opnd2, EXMode_Z, 32);
            }
            return;
        } else {
            /* 3. should be ffff ffff ffff xxxx */
            if (low_16_bits < 0) {
                imm = low_16_bits;
                lsassertm(((imm & 0xFFFFFFFFFFFF0000) == 0xFFFFFFFFFFFF0000),
                        "value=0x%x, imm = 0x%lx\n", value, imm);
                latxs_load_imm64(opnd2, imm);
                if (option_by_hand) {
                    latxs_ir2_opnd_set_emb(opnd2, EXMode_S, 16);
                }
                return;
            } else {
                imm = (int32_t)value;
                lsassertm(((imm & 0xFFFFFFFFFFFF0000) == 0xFFFFFFFFFFFF0000),
                        "value=0x%x, imm = 0x%lx\n", value, imm);
                latxs_load_imm64(opnd2, value);
                if (option_by_hand) {
                    latxs_ir2_opnd_set_emb(opnd2, EXMode_S, 32);
                }
                return;
            }
        }
    } else {
        if (em == EXMode_Z) {
            /* 5. should be 0000 0000 xxxx xxxx */
            imm = value;
            imm = imm & 0xFFFFFFFF;
            latxs_load_imm64(opnd2, imm);
            if (option_by_hand) {
                latxs_ir2_opnd_set_emb(opnd2, EXMode_Z, 32);
            }
            return;
        } else {
            /* 6. should be ssss ssss xxxx xxxx */
            imm = (int32_t)value;
            latxs_load_imm64(opnd2, imm);
            if (option_by_hand) {
                latxs_ir2_opnd_set_emb(opnd2, EXMode_S, 32);
            }
            return;
        }
    }
}

/*
 * original                     new
 * EM       -> st_w -> ld_w ->  EM
 * ----------------------------------
 *     0                        0
 *     8s/z                     8s/z
 *    16s/z                    16s/z
 *    32n/z/s                  32s
 */
static void latxs_td_set_reg_extmb_ldw(int gpr)
{
    lsassert(gpr < CPU_NB_REGS);
    if (lsenv->tr_data->reg_exbits[gpr] < 32) {
        return;
    }
    /* eb == 32 */
    lsenv->tr_data->reg_exmode[gpr] = EXMode_S;
}

static void latxs_td_set_reg_extmb_after_cs_all(void)
{
    latxs_td_set_reg_extmb_ldw(0);
    latxs_td_set_reg_extmb_ldw(1);
    latxs_td_set_reg_extmb_ldw(2);
    latxs_td_set_reg_extmb_ldw(3);
    latxs_td_set_reg_extmb_ldw(4);
    latxs_td_set_reg_extmb_ldw(5);
    latxs_td_set_reg_extmb_ldw(6);
    latxs_td_set_reg_extmb_ldw(7);
}

void latxs_td_set_reg_extmb_after_cs(int mask)
{
    if (mask == 0xff) {
        latxs_td_set_reg_extmb_after_cs_all();
        return;
    }

    int i = 0;
    for (i = 0; i < 8; ++i) {
        if ((mask >> i) & 1) {
            latxs_td_set_reg_extmb_ldw(i);
        }
    }
}

static void latxs_tr_reset_reg_extmb(int gpr)
{
    lsassert(gpr < CPU_NB_REGS);
    EXMode gpr_em = latxs_td_get_reg_extm(gpr);
    EXBits gpr_eb = latxs_td_get_reg_extb(gpr);

    if (gpr_eb == 32 && gpr_em != EXMode_S) {
        /* 32n 32z */
        IR2_OPND gpr_opnd = latxs_ra_alloc_gpr(gpr);
        latxs_append_ir2_opnd2_(lisa_mov32s, &gpr_opnd, &gpr_opnd);
        latxs_td_set_reg_extmb(gpr, EXMode_S, 32);
    } else {
        /* 0 8s/z 16s/z 32s */
        latxs_td_set_reg_extmb(gpr, EXMode_S, 32);
    }
}

void latxs_tr_reset_extmb(int mask)
{
    /* reset to 32s */
    int i = 0;
    for (i = 0; i < 8; ++i) {
        if ((mask >> i) & 1) {
            latxs_tr_reset_reg_extmb(i);
        }
    }
}

static void latxs_tr_setto_reg_extmb_after_cs(int gpr)
{
    EXMode gpr_em = latxs_td_get_reg_extm(gpr);
    EXBits gpr_eb = latxs_td_get_reg_extb(gpr);

    /*
     * @gpr_em and @gpr_eb is the original EM
     * from softmmu fast path.
     *
     *  8s/z -> st_w -> ld_w ->  8s/z
     * 16s/z -> st_w -> ld_w -> 16s/z
     * 32s/z -> st_w -> ld_w -> 32s   : 32z
     * 32n   -> st_w -> ld_w -> 32s
     *  0    -> st_w -> ld_w ->  0
     *
     * If original EM is 32z, we need to reset it.
     */

    if (gpr_eb == 32 && gpr_em == EXMode_Z) {
        IR2_OPND gpr_opnd = latxs_ra_alloc_gpr(gpr);
        latxs_append_ir2_opnd2_(lisa_mov32z, &gpr_opnd, &gpr_opnd);
    }
}

/*
 * Should be called ONLY in softmmu slow path generation.
 */
void latxs_tr_setto_extmb_after_cs(int mask)
{
    int i = 0;
    for (i = 0; i < 8; ++i) {
        if ((mask >> i) & 1) {
            latxs_tr_setto_reg_extmb_after_cs(i);
        }
    }
}
