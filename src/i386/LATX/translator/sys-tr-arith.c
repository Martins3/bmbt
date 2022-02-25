#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>

void latxs_sys_arith_register_ir1(void)
{
    latxs_register_ir1(X86_INS_ADD);
    latxs_register_ir1(X86_INS_ADC);
    latxs_register_ir1(X86_INS_SUB);
    latxs_register_ir1(X86_INS_SBB);
    latxs_register_ir1(X86_INS_INC);
    latxs_register_ir1(X86_INS_DEC);
    latxs_register_ir1(X86_INS_NEG);
    latxs_register_ir1(X86_INS_CMP);
    latxs_register_ir1(X86_INS_MUL);
    latxs_register_ir1(X86_INS_IMUL);
    latxs_register_ir1(X86_INS_DIV);
    latxs_register_ir1(X86_INS_IDIV);

    latxs_register_ir1(X86_INS_AAA);
    latxs_register_ir1(X86_INS_AAD);
    latxs_register_ir1(X86_INS_AAM);
    latxs_register_ir1(X86_INS_AAS);
    latxs_register_ir1(X86_INS_DAA);
    latxs_register_ir1(X86_INS_DAS);

    latxs_register_ir1(X86_INS_XADD);
}

bool latxs_translate_add(IR1_INST *pir1)
{
    if (latxs_ir1_has_prefix_lock(pir1) &&
        (lsenv->tr_data->sys.cflags & CF_PARALLEL)) {
        return latxs_translate_lock_add(pir1);
    }
#ifdef TARGET_X86_64
    if (option_by_hand_64 && latxs_translate_add_byhand64(pir1)) {
        return true;
    }
#endif
    if (option_by_hand) {
        return latxs_translate_add_byhand(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    IR2_OPND dest = latxs_ra_alloc_itemp();

#ifdef TARGET_X86_64
    latxs_append_ir2_opnd3(LISA_ADD_D, &dest, &src0, &src1);
#else
    latxs_append_ir2_opnd3(LISA_ADD_W, &dest, &src0, &src1);
#endif

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&dest, opnd0);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
        latxs_store_ir2_to_ir1(&dest, opnd0);
    }

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    latxs_ra_free_temp(&src1);
    return true;
}

bool latxs_translate_adc(IR1_INST *pir1)
{
    if (latxs_ir1_has_prefix_lock(pir1) &&
        (lsenv->tr_data->sys.cflags & CF_PARALLEL)) {
        return latxs_translate_lock_adc(pir1);
    }
    if (option_by_hand) {
        return latxs_translate_adc_byhand(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_load_eflags_cf_to_ir2(&dest);

#ifdef TARGET_X86_64
    latxs_append_ir2_opnd3(LISA_ADD_D, &dest, &dest, &src0);
    latxs_append_ir2_opnd3(LISA_ADD_D, &dest, &dest, &src1);
#else
    latxs_append_ir2_opnd3(LISA_ADD_W, &dest, &dest, &src0);
    latxs_append_ir2_opnd3(LISA_ADD_W, &dest, &dest, &src1);
#endif

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&dest, opnd0);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
        latxs_store_ir2_to_ir1(&dest, opnd0);
    }

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    latxs_ra_free_temp(&src1);
    return true;
}

bool latxs_translate_sub(IR1_INST *pir1)
{
    if (latxs_ir1_has_prefix_lock(pir1) &&
        (lsenv->tr_data->sys.cflags & CF_PARALLEL)) {
        return latxs_translate_lock_sub(pir1);
    }
    if (option_by_hand) {
        return latxs_translate_sub_byhand(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    IR2_OPND dest = latxs_ra_alloc_itemp();

#ifdef TARGET_X86_64
    latxs_append_ir2_opnd3(LISA_SUB_D, &dest, &src0, &src1);
#else
    latxs_append_ir2_opnd3(LISA_SUB_W, &dest, &src0, &src1);
#endif

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&dest, opnd0);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
        latxs_store_ir2_to_ir1(&dest, opnd0);
    }

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    latxs_ra_free_temp(&src1);
    return true;
}

bool latxs_translate_sbb(IR1_INST *pir1)
{
    if (latxs_ir1_has_prefix_lock(pir1) &&
        (lsenv->tr_data->sys.cflags & CF_PARALLEL)) {
        return latxs_translate_lock_sbb(pir1);
    }
    if (option_by_hand) {
        return latxs_translate_sbb_byhand(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_load_eflags_cf_to_ir2(&dest);

#ifdef TARGET_X86_64
    latxs_append_ir2_opnd3(LISA_SUB_D, &dest, &src0, &dest);
    latxs_append_ir2_opnd3(LISA_SUB_D, &dest, &dest, &src1);
#else
    latxs_append_ir2_opnd3(LISA_SUB_W, &dest, &src0, &dest);
    latxs_append_ir2_opnd3(LISA_SUB_W, &dest, &dest, &src1);
#endif

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&dest, opnd0);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
        latxs_store_ir2_to_ir1(&dest, opnd0);
    }

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    latxs_ra_free_temp(&src1);
    return true;
}

bool latxs_translate_inc(IR1_INST *pir1)
{
    if (latxs_ir1_has_prefix_lock(pir1) &&
        (lsenv->tr_data->sys.cflags & CF_PARALLEL)) {
        return latxs_translate_lock_inc(pir1);
    }
    if (option_by_hand) {
        return latxs_translate_inc_byhand(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S);

    IR2_OPND dest = latxs_ra_alloc_itemp();

#ifdef TARGET_X86_64
    latxs_append_ir2_opnd2i(LISA_ADDI_D, &dest, &src0, 1);
#else
    latxs_append_ir2_opnd2i(LISA_ADDI_W, &dest, &src0, 1);
#endif

    IR2_OPND imm1 = latxs_ir2_opnd_new(IR2_OPND_IMMH, 1);

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&dest, opnd0);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&dest, &src0, &imm1, pir1, true);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&dest, &src0, &imm1, pir1, true);
        latxs_store_ir2_to_ir1(&dest, opnd0);
    }

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    return true;
}

bool latxs_translate_dec(IR1_INST *pir1)
{
    if (latxs_ir1_has_prefix_lock(pir1) &&
        (lsenv->tr_data->sys.cflags & CF_PARALLEL)) {
        return latxs_translate_lock_dec(pir1);
    }
    if (option_by_hand) {
        return latxs_translate_dec_byhand(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S);

    IR2_OPND dest = latxs_ra_alloc_itemp();

#ifdef TARGET_X86_64
    latxs_append_ir2_opnd2i(LISA_ADDI_D, &dest, &src0, -1);
#else
    latxs_append_ir2_opnd2i(LISA_ADDI_W, &dest, &src0, -1);
#endif

    IR2_OPND imm1 = latxs_ir2_opnd_new(IR2_OPND_IMMH, 1);

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&dest, opnd0);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&dest, &src0, &imm1, pir1, true);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&dest, &src0, &imm1, pir1, true);
        latxs_store_ir2_to_ir1(&dest, opnd0);
    }

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    return true;
}

bool latxs_translate_neg(IR1_INST *pir1)
{
    if (latxs_ir1_has_prefix_lock(pir1) &&
        (lsenv->tr_data->sys.cflags & CF_PARALLEL)) {
        return latxs_translate_lock_neg(pir1);
    }
    if (option_by_hand) {
        return latxs_translate_neg_byhand(pir1);
    }

    IR2_OPND *zero = &latxs_zero_ir2_opnd;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S);

    IR2_OPND dest = latxs_ra_alloc_itemp();
#ifdef TARGET_X86_64
    latxs_append_ir2_opnd3(LISA_SUB_D, &dest, zero, &src0);
#else
    latxs_append_ir2_opnd3(LISA_SUB_W, &dest, zero, &src0);
#endif

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&dest, opnd0);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&dest, zero, &src0, pir1, true);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&dest, zero, &src0, pir1, true);
        latxs_store_ir2_to_ir1(&dest, opnd0);
    }

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    return true;
}

bool latxs_translate_cmp(IR1_INST *pir1)
{
#ifdef TARGET_X86_64
    if (option_by_hand_64 && latxs_translate_cmp_byhand64(pir1)) {
        return true;
    }
#endif
    if (option_by_hand) {
        return latxs_translate_cmp_byhand(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    IR2_OPND dest = latxs_ra_alloc_itemp();

#ifdef TARGET_X86_64
    latxs_append_ir2_opnd3(LISA_SUB_D, &dest, &src0, &src1);
#else
    latxs_append_ir2_opnd3(LISA_SUB_W, &dest, &src0, &src1);
#endif

    latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    latxs_ra_free_temp(&src1);
    return true;
}

bool latxs_translate_mul(IR1_INST *pir1)
{
    if (option_by_hand) {
        return latxs_translate_mul_byhand(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_Z);

    IR2_OPND dest = latxs_ra_alloc_itemp();
#ifdef TARGET_X86_64
    IR2_OPND dest_hi = latxs_ra_alloc_itemp();
#endif

    int opnd_size = ir1_opnd_size(opnd0);
    switch (opnd_size) {
    case 8:
        latxs_load_ir1_gpr_to_ir2(&src1, &al_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd3(LISA_MUL_W, &dest, &src1, &src0);
        latxs_store_ir2_to_ir1(&dest, &ax_ir1_opnd);
        break;
    case 16:
        latxs_load_ir1_gpr_to_ir2(&src1, &ax_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd3(LISA_MUL_W, &dest, &src1, &src0);
        latxs_store_ir2_to_ir1(&dest, &ax_ir1_opnd);
        latxs_append_ir2_opnd2i(LISA_SRLI_W, &dest, &dest, 16);
        latxs_store_ir2_to_ir1(&dest, &dx_ir1_opnd);
        break;
    case 32:
        latxs_load_ir1_gpr_to_ir2(&src1, &eax_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd3(LISA_MUL_D, &dest, &src1, &src0);
        latxs_store_ir2_to_ir1(&dest, &eax_ir1_opnd);
        latxs_append_ir2_opnd2i(LISA_SRLI_D, &dest, &dest, 32);
        latxs_store_ir2_to_ir1(&dest, &edx_ir1_opnd);
        break;
#ifdef TARGET_X86_64
    case 64:
        latxs_load_ir1_gpr_to_ir2(&src1, &rax_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd3(LISA_MUL_D, &dest, &src1, &src0);
        latxs_append_ir2_opnd3(LISA_MULH_DU, &dest_hi, &src1, &src0);
        latxs_store_ir2_to_ir1(&dest, &rax_ir1_opnd);
        latxs_store_ir2_to_ir1(&dest_hi, &rdx_ir1_opnd);
        break;
#endif
    default:
        lsassert(0);
        break;
    }

    latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
#ifdef TARGET_X86_64
    latxs_ra_free_temp(&dest_hi);
#endif
    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    latxs_ra_free_temp(&src1);
    return true;
}

static bool latxs_translate_imul_1_opnd(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S);

    IR2_OPND dest = latxs_ra_alloc_itemp();
#ifdef TARGET_X86_64
    IR2_OPND dest_hi = latxs_ra_alloc_itemp();
#endif

    /* Destination is always GPR, no exception will generate */
    int opnd_size = ir1_opnd_size(opnd0);
    switch (opnd_size) {
    case 8:
        latxs_load_ir1_gpr_to_ir2(&src1, &al_ir1_opnd, EXMode_S);
        latxs_append_ir2_opnd3(LISA_MUL_W, &dest, &src1, &src0);
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
        latxs_store_ir2_to_ir1(&dest, &ax_ir1_opnd);
        break;
    case 16:
        latxs_load_ir1_gpr_to_ir2(&src1, &ax_ir1_opnd, EXMode_S);
        latxs_append_ir2_opnd3(LISA_MUL_W, &dest, &src1, &src0);
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
        latxs_store_ir2_to_ir1(&dest, &ax_ir1_opnd);
        latxs_append_ir2_opnd2i(LISA_SRLI_D, &dest, &dest, 16);
        latxs_store_ir2_to_ir1(&dest, &dx_ir1_opnd);
        break;
    case 32:
        latxs_load_ir1_gpr_to_ir2(&src1, &eax_ir1_opnd, EXMode_S);
        latxs_append_ir2_opnd3(LISA_MUL_D, &dest, &src1, &src0);
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
        latxs_store_ir2_to_ir1(&dest, &eax_ir1_opnd);
        latxs_append_ir2_opnd2i(LISA_SRLI_D, &dest, &dest, 32);
        latxs_store_ir2_to_ir1(&dest, &edx_ir1_opnd);
        break;
#ifdef TARGET_X86_64
    case 64:
        latxs_load_ir1_gpr_to_ir2(&src1, &rax_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd3(LISA_MUL_D, &dest, &src1, &src0);
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
        latxs_append_ir2_opnd3(LISA_MULH_D, &dest_hi, &src1, &src0);
        latxs_store_ir2_to_ir1(&dest, &rax_ir1_opnd);
        latxs_store_ir2_to_ir1(&dest_hi, &rdx_ir1_opnd);
        break;
#endif
    default:
        lsassert(0);
        break;
    }

#ifdef TARGET_X86_64
    latxs_ra_free_temp(&dest_hi);
#endif

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    latxs_ra_free_temp(&src1);
    return true;
}

bool latxs_translate_imul(IR1_INST *pir1)
{
    if (option_by_hand) {
        return latxs_translate_imul_byhand(pir1);
    }

    if (ir1_opnd_num(pir1) == 1) {
        return latxs_translate_imul_1_opnd(pir1);
    }

    IR1_OPND *srcopnd0 = ir1_get_src_opnd(pir1, 0);
    IR1_OPND *srcopnd1 = ir1_get_src_opnd(pir1, 1);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, srcopnd0, EXMode_S);
    latxs_load_ir1_to_ir2(&src1, srcopnd1, EXMode_S);

    IR2_OPND dest = latxs_ra_alloc_itemp();

    int opnd_size = ir1_opnd_size(ir1_get_opnd(pir1, 0));

#ifndef TARGET_X86_64
    lsassertm_illop(ir1_addr(pir1), opnd_size != 64,
            "64-bit translate_imul is unimplemented.\n");
#endif

#ifdef TARGET_X86_64
    if (opnd_size == 64) {
        latxs_append_ir2_opnd3(LISA_MUL_D, &dest, &src1, &src0);
    } else
#endif
    if (opnd_size == 32) {
        latxs_append_ir2_opnd3(LISA_MUL_D, &dest, &src1, &src0);
    } else {
        latxs_append_ir2_opnd3(LISA_MUL_W, &dest, &src1, &src0);
    }

    latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);

    latxs_ra_free_temp(&src0);
    latxs_ra_free_temp(&src1);

    /* Destination is always GPR, no exception will generate */
    if (ir1_opnd_num(pir1) == 3) {
        latxs_store_ir2_to_ir1(&dest, ir1_get_dest_opnd(pir1, 0));
    } else {
        latxs_store_ir2_to_ir1(&dest, ir1_get_src_opnd(pir1, 0));
    }

    latxs_ra_free_temp(&dest);
    return true;
}

void latxs_tr_gen_div_result_check(IR1_INST *pir1,
        IR2_OPND result, int size, int is_idiv)
{
    /*
     * u8   [0,     2^8 -1] = [0x0,         0xff        ]
     * u16  [0,     2^16-1] = [0x0,         0xffff      ]
     * u32  [0,     2^32-1] = [0x0,         0xffff_ffff ]
     * s8   [-2^7,  2^7 -1] = [0xffff_ff80, 0x7f        ]
     * s16  [-2^15, 2^15-1] = [0xffff_8000, 0x7fff      ]
     * s32  [-2^31, 2^31-1] = [0x8000_0000, 0x7fff_ffff ]
     */
    IR2_OPND label_no_excp = latxs_ir2_opnd_new_label();
    IR2_OPND tmp = latxs_ra_alloc_itemp();

    /* 1. chechk and branch to 'label_no_excp' */
    switch (size) {
    case 8:
        if (is_idiv) { /* signed 8-bit */
            latxs_append_ir2_opnd2_(lisa_mov8s, &tmp, &result);
        } else { /* unsigned 8-bit */
            latxs_append_ir2_opnd2_(lisa_mov8z, &tmp, &result);
        }
        break;
    case 16:
        if (is_idiv) { /* signed 16-bit */
            latxs_append_ir2_opnd2_(lisa_mov16s, &tmp, &result);
        } else { /* unsigned 16-bit */
            latxs_append_ir2_opnd2_(lisa_mov16z, &tmp, &result);
        }
        break;
    case 32:
        if (is_idiv) { /* signed 32-bit */
            latxs_append_ir2_opnd2_(lisa_mov32s, &tmp, &result);
        } else { /* unsigned 32-bit */
            latxs_append_ir2_opnd2_(lisa_mov32z, &tmp, &result);
        }
        break;
    default:
        /* should never reach here */
        lsassertm(0, "should never reach here [%s:%d].\n",
                __func__, __LINE__);
        break;
    }
    latxs_append_ir2_opnd3(LISA_BEQ, &result, &tmp, &label_no_excp);
    latxs_ra_free_temp(&tmp);

    /* 2. not branch: generate exception */
    latxs_tr_gen_excp_divz(pir1, 0);

    /* 3. branch: no exception */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_no_excp);
}

bool latxs_translate_div(IR1_INST *pir1)
{
    if (option_by_hand) {
        return latxs_translate_div_byhand(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();
    IR2_OPND src2 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_Z);

#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64 && (ir1_opnd_size(opnd0) == 64)) {
        latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &src0);
        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                &latxs_env_ir2_opnd);
        latxs_tr_gen_call_to_helper((ADDR)helper_divq_EAX);
        latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
        return true;
    }
#endif

    /* 1. if div zero, generate DIVZ exception */
    IR2_OPND label_not_zero = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BNE, &src0,
            &latxs_zero_ir2_opnd, &label_not_zero);
    /* 1.1 not branch : generate exception and never return */
    latxs_tr_gen_excp_divz(pir1, 0);
    /* 1.2 branch: go on finish div operation */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_not_zero);

    IR2_OPND div_res = latxs_ra_alloc_itemp();
    IR2_OPND div_mod = latxs_ir2_opnd_new_inv();

    IR2_OPND src_tmp = latxs_ir2_opnd_new_inv();

    /* 2. do div operation */
    int opnd_size = ir1_opnd_size(opnd0);
    switch (opnd_size) {
    case 8:
        latxs_load_ir1_gpr_to_ir2(&src1, &ax_ir1_opnd, EXMode_Z);
        div_mod = latxs_ra_alloc_itemp();

        /* 2.1 do div operation and get result */
        latxs_append_ir2_opnd3(LISA_DIV_DU, &div_res, &src1, &src0);
        latxs_append_ir2_opnd3(LISA_MOD_DU, &div_mod, &src1, &src0);
        latxs_ra_free_temp(&src0);
        latxs_ra_free_temp(&src1);

        /* 2.2 check if result is valid and generate exception if not */
        latxs_tr_gen_div_result_check(pir1, div_res, 8, 0);
        /* 2.3 store the results into destination */
        latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &div_res, &div_mod, 15, 8);
        latxs_ra_free_temp(&div_mod);
        latxs_store_ir2_to_ir1(&div_res, &ax_ir1_opnd);
        break;
    case 16:
        latxs_load_ir1_gpr_to_ir2(&src1, &ax_ir1_opnd, EXMode_Z);
        latxs_load_ir1_gpr_to_ir2(&src2, &dx_ir1_opnd, EXMode_Z);
        src_tmp = latxs_ra_alloc_itemp();
        div_mod = latxs_ra_alloc_itemp();

        /* 2.1 do div operation and get result */
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &src_tmp, &src2, 16);
        latxs_append_ir2_opnd3(LISA_OR, &src_tmp, &src_tmp, &src1);
        latxs_ra_free_temp(&src1);
        latxs_ra_free_temp(&src2);

        latxs_append_ir2_opnd3(LISA_DIV_DU, &div_res, &src_tmp, &src0);
        latxs_append_ir2_opnd3(LISA_MOD_DU, &div_mod, &src_tmp, &src0);
        latxs_ra_free_temp(&src_tmp);
        latxs_ra_free_temp(&src0);

        /* 2.2 check if result is valid and generate exception if not */
        latxs_tr_gen_div_result_check(pir1, div_res, 16, 0);
        /* 2.3 store the results into destination */
        latxs_store_ir2_to_ir1(&div_mod, &dx_ir1_opnd);
        latxs_ra_free_temp(&div_mod);
        latxs_store_ir2_to_ir1(&div_res, &ax_ir1_opnd);
        break;
    case 32:
        latxs_load_ir1_gpr_to_ir2(&src1, &eax_ir1_opnd, EXMode_Z);
        latxs_load_ir1_gpr_to_ir2(&src2, &edx_ir1_opnd, EXMode_N);
        src_tmp = latxs_ra_alloc_itemp();
        div_mod = latxs_ra_alloc_itemp();

        /* 2.1 do div operation and get result */
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &src_tmp, &src2, 32);
        latxs_append_ir2_opnd3(LISA_OR, &src_tmp, &src_tmp, &src1);
        latxs_ra_free_temp(&src1);
        latxs_ra_free_temp(&src2);

        latxs_append_ir2_opnd3(LISA_DIV_DU, &div_res, &src_tmp, &src0);
        latxs_append_ir2_opnd3(LISA_MOD_DU, &div_mod, &src_tmp, &src0);
        latxs_ra_free_temp(&src_tmp);
        latxs_ra_free_temp(&src0);

        /* 2.2 check if result is valid and generate exception if not */
        latxs_tr_gen_div_result_check(pir1, div_res, 32, 0);
        /* 2.3 store the results into destination */
        latxs_store_ir2_to_ir1(&div_mod, &edx_ir1_opnd);
        latxs_ra_free_temp(&div_mod);
        latxs_store_ir2_to_ir1(&div_res, &eax_ir1_opnd);
        break;
    default:
        /* 64 bit div : call helper at the begin of translate_div */
        lsassert(0);
        break;
    }

    latxs_ra_free_temp(&div_res);
    return true;
}

bool latxs_translate_idiv(IR1_INST *pir1)
{
    if (option_by_hand) {
        return latxs_translate_idiv_byhand(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();
    IR2_OPND src2 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S);

#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64 && (ir1_opnd_size(opnd0) == 64)) {
        latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &src0);
        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                &latxs_env_ir2_opnd);
        latxs_tr_gen_call_to_helper((ADDR)helper_idivq_EAX);
        latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
        return true;
    }
#endif

    /* 1. if div zero, generate DIVZ exception */
    IR2_OPND label_not_zero = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BNE, &src0,
            &latxs_zero_ir2_opnd, &label_not_zero);
    /* 1.1 not branch : generate exception and never return */
    latxs_tr_gen_excp_divz(pir1, 0);
    /* 1.2 branch: go on finish div operation */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_not_zero);

    IR2_OPND div_res = latxs_ra_alloc_itemp();
    IR2_OPND div_mod = latxs_ir2_opnd_new_inv();

    IR2_OPND src_tmp = latxs_ir2_opnd_new_inv();

    /* 2. do div operation */
    int opnd_size = ir1_opnd_size(opnd0);
    switch (opnd_size) {
    case 8:
        latxs_load_ir1_gpr_to_ir2(&src1, &ax_ir1_opnd, EXMode_S);
        div_mod = latxs_ra_alloc_itemp();

        /* 2.1 do div operation and get result */
        latxs_append_ir2_opnd3(LISA_DIV_D, &div_res, &src1, &src0);
        latxs_append_ir2_opnd3(LISA_MOD_D, &div_mod, &src1, &src0);
        latxs_ra_free_temp(&src0);
        latxs_ra_free_temp(&src1);

        /* 2.2 check if result is valid and generate exception if not */
        latxs_tr_gen_div_result_check(pir1, div_res, 8, 1);
        /* 2.3 store the results into destination */
        latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &div_res, &div_mod, 15, 8);
        latxs_ra_free_temp(&div_mod);
        latxs_store_ir2_to_ir1(&div_res, &ax_ir1_opnd);
        break;
    case 16:
        latxs_load_ir1_gpr_to_ir2(&src1, &ax_ir1_opnd, EXMode_Z);
        latxs_load_ir1_gpr_to_ir2(&src2, &dx_ir1_opnd, EXMode_S);
        src_tmp = latxs_ra_alloc_itemp();
        div_mod = latxs_ra_alloc_itemp();

        /* 2.1 do div operation and get result */
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &src_tmp, &src2, 16);
        latxs_append_ir2_opnd3(LISA_OR, &src_tmp, &src_tmp, &src1);
        latxs_ra_free_temp(&src1);
        latxs_ra_free_temp(&src2);

        latxs_append_ir2_opnd3(LISA_DIV_D, &div_res, &src_tmp, &src0);
        latxs_append_ir2_opnd3(LISA_MOD_D, &div_mod, &src_tmp, &src0);
        latxs_ra_free_temp(&src_tmp);
        latxs_ra_free_temp(&src0);

        /* 2.2 check if result is valid and generate exception if not */
        latxs_tr_gen_div_result_check(pir1, div_res, 16, 1);
        /* 2.3 store the results into destination */
        latxs_store_ir2_to_ir1(&div_mod, &dx_ir1_opnd);
        latxs_ra_free_temp(&div_mod);
        latxs_store_ir2_to_ir1(&div_res, &ax_ir1_opnd);
        break;
    case 32:
        latxs_load_ir1_gpr_to_ir2(&src1, &eax_ir1_opnd, EXMode_Z);
        latxs_load_ir1_gpr_to_ir2(&src2, &edx_ir1_opnd, EXMode_N);
        src_tmp = latxs_ra_alloc_itemp();
        div_mod = latxs_ra_alloc_itemp();

        /* 2.1 do div operation and get result */
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &src_tmp, &src2, 32);
        latxs_append_ir2_opnd3(LISA_OR, &src_tmp, &src_tmp, &src1);
        latxs_ra_free_temp(&src1);
        latxs_ra_free_temp(&src2);

        latxs_append_ir2_opnd3(LISA_DIV_D, &div_res, &src_tmp, &src0);
        latxs_append_ir2_opnd3(LISA_MOD_D, &div_mod, &src_tmp, &src0);
        latxs_ra_free_temp(&src_tmp);
        latxs_ra_free_temp(&src0);

        /* 2.2 check if result is valid and generate exception if not */
        latxs_tr_gen_div_result_check(pir1, div_res, 32, 1);
        /* 2.3 store the results into destination */
        latxs_store_ir2_to_ir1(&div_mod, &edx_ir1_opnd);
        latxs_ra_free_temp(&div_mod);
        latxs_store_ir2_to_ir1(&div_res, &eax_ir1_opnd);
        break;
    default:
        lsassertm_illop(ir1_addr(pir1), 0,
                "64-bit translate_idiv is unimplemented.\n");
        break;
    }

    latxs_ra_free_temp(&div_res);
    return true;
}

static bool latxs_do_translate_bcd(IR1_INST *pir1, int val)
{
    /* 1. only need to save eflags */
    helper_cfg_t cfg = default_helper_cfg;
    cfg.sv_eflags = 1;

    /*
     * 2. call helper and sync the eflags
     *    > EAX is used (caller-saved, always saved)
     *    > eflags is used and updated
     *
     * target/i386/int_helper.c
     * void helper_daa(CPUX86State *env)
     * void helper_das(CPUX86State *env)
     * void helper_aaa(CPUX86State *env)
     * void helper_aas(CPUX86State *env)
     * void helper_aam(CPUX86State *env, int base)
     * void helper_aad(CPUX86State *env, int base)
     */
    switch (ir1_opcode(pir1)) {
    case X86_INS_DAA:
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_daa, cfg);
        break;
    case X86_INS_DAS:
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_das, cfg);
        break;
    case X86_INS_AAA:
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_aaa, cfg);
        break;
    case X86_INS_AAS:
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_aas, cfg);
        break;
    case X86_INS_AAM:
        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_aam, val, cfg);
        break;
    case X86_INS_AAD:
        latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_aad, val, cfg);
        break;
    default:
        lsassertm_illop(ir1_addr(pir1), 0,
                "unknown bcd instruction.\n");
        break;
    }

    return true;
}

bool latxs_translate_aaa(IR1_INST *pir1)
{
    return latxs_do_translate_bcd(pir1, 1);
}
bool latxs_translate_aad(IR1_INST *pir1)
{
    int val;
    if (!ir1_opnd_num(pir1)) {
        val = 0x0A;
    } else {
        val = ir1_opnd_uimm(ir1_get_opnd(pir1, 0));
    }

    return latxs_do_translate_bcd(pir1, val);
}
bool latxs_translate_aam(IR1_INST *pir1)
{
    int val;
    if (!ir1_opnd_num(pir1)) {
        val = 0x0A;
    } else {
        val = ir1_opnd_uimm(ir1_get_opnd(pir1, 0));
    }

    return latxs_do_translate_bcd(pir1, val);
}
bool latxs_translate_aas(IR1_INST *pir1)
{
    return latxs_do_translate_bcd(pir1, 1);
}
bool latxs_translate_daa(IR1_INST *pir1)
{
    return latxs_do_translate_bcd(pir1, 1);
}
bool latxs_translate_das(IR1_INST *pir1)
{
    return latxs_do_translate_bcd(pir1, 1);
}

bool latxs_translate_xadd(IR1_INST *pir1)
{
    if (latxs_ir1_has_prefix_lock(pir1) &&
        (lsenv->tr_data->sys.cflags & CF_PARALLEL)) {
        return latxs_translate_lock_xadd(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* dest: GPR/MEM */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* src : GPR     */

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    IR2_OPND sum = latxs_ra_alloc_itemp();

    if (ir1_opnd_is_gpr(opnd0)) {
        latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S);
#ifdef TARGET_X86_64
        latxs_append_ir2_opnd3(LISA_ADD_D, &sum, &src0, &src1);
#else
        latxs_append_ir2_opnd3(LISA_ADD_W, &sum, &src0, &src1);
#endif
        latxs_store_ir2_to_ir1(&src0, opnd1);
        latxs_store_ir2_to_ir1(&sum, opnd0);
    } else {
        latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S);
#ifdef TARGET_X86_64
        latxs_append_ir2_opnd3(LISA_ADD_D, &sum, &src0, &src1);
#else
        latxs_append_ir2_opnd3(LISA_ADD_W, &sum, &src0, &src1);
#endif
        latxs_store_ir2_to_ir1(&sum, opnd0);
        latxs_store_ir2_to_ir1(&src0, opnd1);
    }

    latxs_generate_eflag_calculation(&sum, &src0, &src1, pir1, true);

    latxs_ra_free_temp(&sum);

    return true;
}
