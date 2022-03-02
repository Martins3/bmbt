#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "flag-lbt.h"
#include "sys-excp.h"
#include <string.h>

void latxs_sys_logic_register_ir1(void)
{
    latxs_register_ir1(X86_INS_XOR);
    latxs_register_ir1(X86_INS_AND);
    latxs_register_ir1(X86_INS_TEST);
    latxs_register_ir1(X86_INS_OR);
    latxs_register_ir1(X86_INS_NOT);

    latxs_register_ir1(X86_INS_SHL);
    latxs_register_ir1(X86_INS_SHR);
    latxs_register_ir1(X86_INS_SAL);
    latxs_register_ir1(X86_INS_SAR);

    latxs_register_ir1(X86_INS_ROL);
    latxs_register_ir1(X86_INS_ROR);
    latxs_register_ir1(X86_INS_RCL);
    latxs_register_ir1(X86_INS_RCR);

    latxs_register_ir1(X86_INS_SHRD);
    latxs_register_ir1(X86_INS_SHLD);

    latxs_register_ir1(X86_INS_BSWAP);
}

bool latxs_translate_xor(IR1_INST *pir1)
{
    if (latxs_ir1_has_prefix_lock(pir1) &&
        (lsenv->tr_data->sys.cflags & CF_PARALLEL)) {
        return latxs_translate_lock_xor(pir1);
    }
#ifdef TARGET_X86_64
    if (option_by_hand_64 && latxs_translate_xor_byhand64(pir1)) {
        return true;
    }
#endif
    if (option_by_hand) {
        return latxs_translate_xor_byhand(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_XOR, &dest, &src0, &src1);

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
    return true;
}

bool latxs_translate_and(IR1_INST *pir1)
{
    if (latxs_ir1_has_prefix_lock(pir1) &&
        (lsenv->tr_data->sys.cflags & CF_PARALLEL)) {
        return latxs_translate_lock_and(pir1);
    }
    if (option_by_hand) {
        return latxs_translate_and_byhand(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_AND, &dest, &src0, &src1);

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
    return true;
}

bool latxs_translate_test(IR1_INST *pir1)
{
#ifdef TARGET_X86_64
    if (option_by_hand_64 && latxs_translate_test_byhand64(pir1)) {
        return true;
    }
#endif
    if (option_by_hand) {
        return latxs_translate_test_byhand(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    IR2_OPND dest = latxs_ra_alloc_itemp();

    latxs_append_ir2_opnd3(LISA_AND, &dest, &src0, &src1);

    latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);

    latxs_ra_free_temp(&dest);
    return true;
}

bool latxs_translate_or(IR1_INST *pir1)
{
    if (latxs_ir1_has_prefix_lock(pir1) &&
        (lsenv->tr_data->sys.cflags & CF_PARALLEL)) {
        return latxs_translate_lock_or(pir1);
    }
    if (option_by_hand) {
        return latxs_translate_or_byhand(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_OR, &dest, &src0, &src1);

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
    return true;
}

bool latxs_translate_not(IR1_INST *pir1)
{
    if (latxs_ir1_has_prefix_lock(pir1) &&
        (lsenv->tr_data->sys.cflags & CF_PARALLEL)) {
        return latxs_translate_lock_not(pir1);
    }
    if (option_by_hand) {
        return latxs_translate_not_byhand(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND dest = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S);

    latxs_append_ir2_opnd2_(lisa_not, &dest, &src0);

    latxs_store_ir2_to_ir1(&dest, opnd0);

    latxs_ra_free_temp(&dest);
    return true;
}

bool latxs_translate_shl(IR1_INST *pir1)
{
    if (option_by_hand) {
        return latxs_translate_shl_byhand(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src, opnd0, EXMode_S);

    IR2_OPND src_shift = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_shift , opnd1, EXMode_Z);

    int shift_mask = 0x1f;

#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64 && ir1_opnd_size(opnd0) == 64) {
        shift_mask = 0x3f;
    }
#endif

    IR2_OPND shift = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ANDI, &shift, &src_shift, shift_mask);
    latxs_ra_free_temp(&src_shift);

    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &shift,
            &latxs_zero_ir2_opnd, &label_exit);

    IR2_OPND dest = latxs_ra_alloc_itemp();
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64 && ir1_opnd_size(opnd0) == 64) {
        latxs_append_ir2_opnd3(LISA_SLL_D, &dest, &src, &shift);
    } else {
        latxs_append_ir2_opnd3(LISA_SLL_W, &dest, &src, &shift);
    }
#else
    latxs_append_ir2_opnd3(LISA_SLL_W, &dest, &src, &shift);
#endif

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&dest, opnd0);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&dest, &src, &shift, pir1, true);
        latxs_ra_free_temp(&dest);
        latxs_ra_free_temp(&shift);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&dest, &src, &shift, pir1, true);
        latxs_ra_free_temp(&src);
        latxs_ra_free_temp(&shift);
        latxs_store_ir2_to_ir1(&dest, opnd0);
        latxs_ra_free_temp(&dest);
    }

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);

#ifdef TARGET_X86_64
    /* make sure high 32 bits set to zero. TODO: simplify */
    if (ir1_opnd_size(opnd0) == 32 && ir1_opnd_is_gpr(opnd0)) {
        IR2_OPND dest_opnd = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        latxs_append_ir2_opnd2_(lisa_mov32z, &dest_opnd, &dest_opnd);
    }
#endif

    return true;
}

bool latxs_translate_shr(IR1_INST *pir1)
{
    if (option_by_hand) {
        return latxs_translate_shr_byhand(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    int opsize = ir1_opnd_size(opnd0);
    EXMode em = EXMode_Z;
    if (opsize == 32) {
        em = EXMode_S;
    }

    IR2_OPND src = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src, opnd0, em);

    IR2_OPND src_shift = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_shift, opnd1, EXMode_Z);

    int shift_mask = 0x1f;

#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64 && ir1_opnd_size(opnd0) == 64) {
        shift_mask = 0x3f;
    }
#endif

    IR2_OPND shift = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ANDI, &shift, &src_shift, shift_mask);
    latxs_ra_free_temp(&src_shift);

    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &shift,
            &latxs_zero_ir2_opnd, &label_exit);

    IR2_OPND dest = latxs_ra_alloc_itemp();

#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64 && ir1_opnd_size(opnd0) == 64) {
        latxs_append_ir2_opnd3(LISA_SRL_D, &dest, &src, &shift);
    } else {
        latxs_append_ir2_opnd3(LISA_SRL_W, &dest, &src, &shift);
    }
#else
    latxs_append_ir2_opnd3(LISA_SRL_W, &dest, &src, &shift);
#endif

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&dest, opnd0);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&dest, &src, &shift, pir1, true);
        latxs_ra_free_temp(&dest);
        latxs_ra_free_temp(&shift);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&dest, &src, &shift, pir1, true);
        latxs_ra_free_temp(&src);
        latxs_ra_free_temp(&shift);
        latxs_store_ir2_to_ir1(&dest, opnd0);
        latxs_ra_free_temp(&dest);
    }

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);
#ifdef TARGET_X86_64
    /* make sure high 32 bits set to zero. TODO: simplify */
    if (ir1_opnd_size(opnd0) == 32 && ir1_opnd_is_gpr(opnd0)) {
        IR2_OPND dest_opnd = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        latxs_append_ir2_opnd2_(lisa_mov32z, &dest_opnd, &dest_opnd);
    }
#endif
    return true;
}

bool latxs_translate_sal(IR1_INST *pir1)
{
    return latxs_translate_shl(pir1);
}

bool latxs_translate_sar(IR1_INST *pir1)
{
    if (option_by_hand) {
        return latxs_translate_sar_byhand(pir1);
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src = latxs_ra_alloc_itemp();
    IR2_OPND src_shift = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src , opnd0, EXMode_S);
    latxs_load_ir1_to_ir2(&src_shift , opnd1, EXMode_Z);

    int shift_mask = 0x1f;

#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64 && ir1_opnd_size(opnd0) == 64) {
        shift_mask = 0x3f;
    }
#endif

    IR2_OPND shift = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ANDI, &shift, &src_shift, shift_mask);
    latxs_ra_free_temp(&src_shift);

    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &shift,
            &latxs_zero_ir2_opnd, &label_exit);

    IR2_OPND dest = latxs_ra_alloc_itemp();

#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64 && ir1_opnd_size(opnd0) == 64) {
        latxs_append_ir2_opnd3(LISA_SRA_D, &dest, &src, &shift);
    } else {
        latxs_append_ir2_opnd3(LISA_SRA_W, &dest, &src, &shift);
    }
#else
    latxs_append_ir2_opnd3(LISA_SRA_W, &dest, &src, &shift);
#endif

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&dest, opnd0);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&dest, &src, &shift, pir1, true);
        latxs_ra_free_temp(&dest);
        latxs_ra_free_temp(&shift);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&dest, &src, &shift, pir1, true);
        latxs_ra_free_temp(&src);
        latxs_ra_free_temp(&shift);
        latxs_store_ir2_to_ir1(&dest, opnd0);
        latxs_ra_free_temp(&dest);
    }

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);
#ifdef TARGET_X86_64
    /* make sure high 32 bits set to zero. TODO: simplify */
    if (ir1_opnd_size(opnd0) == 32 && ir1_opnd_is_gpr(opnd0)) {
        IR2_OPND dest_opnd = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        latxs_append_ir2_opnd2_(lisa_mov32z, &dest_opnd, &dest_opnd);
    }
#endif
    return true;
}

bool latxs_translate_rol(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src_rotate = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_rotate, opnd1, EXMode_Z);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&dest, opnd0, EXMode_Z);

    int opnd_size = ir1_opnd_size(opnd0);
    int shift_mask = 0x1f;

#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64 && opnd_size == 64) {
        shift_mask = 0x3f;
    }
#endif

    IR2_OPND rotate = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ANDI, &rotate, &src_rotate, shift_mask);
    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &rotate, &zero_ir2_opnd, &label_exit);

    if (ir1_need_calculate_any_flag(pir1)) {
        switch (opnd_size) {
        case 8:
            latxs_append_ir2_opnd2(LISA_X86ROTL_B, &dest, &src_rotate);
            break;
        case 16:
            latxs_append_ir2_opnd2(LISA_X86ROTL_H, &dest, &src_rotate);
            break;
        case 32:
            latxs_append_ir2_opnd2(LISA_X86ROTL_W, &dest, &src_rotate);
            break;
#ifdef TARGET_X86_64
        case 64:
            latxs_append_ir2_opnd2(LISA_X86ROTL_D, &dest, &src_rotate);
            break;
#endif
        default:
            lsassert(0);
            break;
        }
    }
    if (latxs_ir2_opnd_is_itemp(&src_rotate)) {
        latxs_ra_free_temp(&src_rotate);
    }

    if (opnd_size == 8) {
        latxs_append_ir2_opnd2i(LISA_ANDI, &rotate, &rotate, 0x7);
    } else if (opnd_size == 16) {
        latxs_append_ir2_opnd2i(LISA_ANDI, &rotate, &rotate, 0xf);
    }

    IR2_OPND label_calc_eflags = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &rotate,
            &latxs_zero_ir2_opnd, &label_calc_eflags);

    IR2_OPND low_dest   = latxs_ra_alloc_itemp();
    IR2_OPND high_dest  = latxs_ra_alloc_itemp();
    IR2_OPND tmp_rotate = latxs_ra_alloc_itemp();

    latxs_append_ir2_opnd3(LISA_SLL_D, &low_dest, &dest, &rotate);
    latxs_append_ir2_opnd3(LISA_SUB_W, &tmp_rotate,
                                        &latxs_zero_ir2_opnd, &rotate);
    latxs_append_ir2_opnd2i(LISA_ANDI, &tmp_rotate,
                                       &tmp_rotate, opnd_size - 1);
    latxs_append_ir2_opnd3(LISA_SRL_D, &high_dest, &dest, &tmp_rotate);
    latxs_append_ir2_opnd3(LISA_OR, &dest, &high_dest, &low_dest);

    latxs_ra_free_temp(&high_dest);
    latxs_ra_free_temp(&low_dest);
    latxs_ra_free_temp(&tmp_rotate);

    latxs_store_ir2_to_ir1(&dest, opnd0);

    latxs_append_ir2_opnd1(LISA_LABEL, &label_calc_eflags);
    latxs_generate_eflag_calculation(&dest, &dest, &rotate, pir1, true);

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&rotate);

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);

#ifdef TARGET_X86_64
    /* make sure high 32 bits set to zero. TODO: simplify */
    if (opnd_size == 32 && ir1_opnd_is_gpr(opnd0)) {
        IR2_OPND dest_opnd = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        latxs_append_ir2_opnd2_(lisa_mov32z, &dest_opnd, &dest_opnd);
    }
#endif
    return true;
}

bool latxs_translate_ror(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src_rotate = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_rotate, opnd1, EXMode_Z);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&dest, opnd0, EXMode_Z);
    int opnd_size = ir1_opnd_size(opnd0);

    int shift_mask = 0x1f;

#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64 && opnd_size == 64) {
        shift_mask = 0x3f;
    }
#endif

    IR2_OPND rotate = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ANDI, &rotate, &src_rotate, shift_mask);
    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &rotate,
            &latxs_zero_ir2_opnd, &label_exit);

    if (ir1_need_calculate_any_flag(pir1)) {
        switch (opnd_size) {
        case 8:
            latxs_append_ir2_opnd2(LISA_X86ROTR_B, &dest, &src_rotate);
            break;
        case 16:
            latxs_append_ir2_opnd2(LISA_X86ROTR_H, &dest, &src_rotate);
            break;
        case 32:
            latxs_append_ir2_opnd2(LISA_X86ROTR_W, &dest, &src_rotate);
            break;
#ifdef TARGET_X86_64
        case 64:
            latxs_append_ir2_opnd2(LISA_X86ROTR_D, &dest, &src_rotate);
            break;
#endif
        default:
            lsassert(0);
            break;
        }
    }
    if (latxs_ir2_opnd_is_itemp(&src_rotate)) {
        latxs_ra_free_temp(&src_rotate);
    }

    if (opnd_size == 8) {
        latxs_append_ir2_opnd2i(LISA_ANDI, &rotate, &rotate, 0x7);
    } else if (opnd_size == 16) {
        latxs_append_ir2_opnd2i(LISA_ANDI, &rotate, &rotate, 0xf);
    }

    IR2_OPND low_dest   = latxs_ra_alloc_itemp();
    IR2_OPND high_dest  = latxs_ra_alloc_itemp();
    IR2_OPND tmp_rotate = latxs_ra_alloc_itemp();

    IR2_OPND label_calc_eflags = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &rotate, &latxs_zero_ir2_opnd,
                                     &label_calc_eflags);
    latxs_append_ir2_opnd3(LISA_SRL_D, &high_dest, &dest, &rotate);
    latxs_append_ir2_opnd3(LISA_SUB_W, &tmp_rotate,
                                       &latxs_zero_ir2_opnd, &rotate);
    latxs_append_ir2_opnd2i(LISA_ANDI, &tmp_rotate,
                                       &tmp_rotate, opnd_size - 1);
    latxs_append_ir2_opnd3(LISA_SLL_D, &low_dest, &dest, &tmp_rotate);
    latxs_append_ir2_opnd3(LISA_OR, &dest, &high_dest, &low_dest);

    latxs_ra_free_temp(&high_dest);
    latxs_ra_free_temp(&low_dest);
    latxs_ra_free_temp(&tmp_rotate);

    latxs_store_ir2_to_ir1(&dest, opnd0);

    latxs_append_ir2_opnd1(LISA_LABEL, &label_calc_eflags);
    latxs_generate_eflag_calculation(&dest, &dest, &rotate, pir1, true);

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&rotate);

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);

#ifdef TARGET_X86_64
    /* make sure high 32 bits set to zero. TODO: simplify */
    if (opnd_size == 32 && ir1_opnd_is_gpr(opnd0)) {
        IR2_OPND dest_opnd = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        latxs_append_ir2_opnd2_(lisa_mov32z, &dest_opnd, &dest_opnd);
    }
#endif
    return true;
}

bool latxs_translate_rcl(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    IR2_OPND src_rotate = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&dest, opnd0, EXMode_Z);
    latxs_load_ir1_to_ir2(&src_rotate, opnd1, EXMode_Z);

    int opnd_size = ir1_opnd_size(opnd0);

    IR2_OPND rotate = latxs_ra_alloc_itemp();

    int shift_mask = 0x1f;

#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64 && opnd_size == 64) {
        shift_mask = 0x3f;
    }
#endif
    latxs_append_ir2_opnd2i(LISA_ANDI, &rotate, &src_rotate, shift_mask);
    latxs_ra_free_temp(&src_rotate);

    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &rotate,
            &latxs_zero_ir2_opnd, &label_exit);

    IR2_OPND tmp_imm = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp_imm,
            &latxs_zero_ir2_opnd, opnd_size + 1);
    latxs_append_ir2_opnd3(LISA_MOD_DU, &rotate, &rotate,  &tmp_imm);
    latxs_append_ir2_opnd3(LISA_BEQ, &rotate,
            &latxs_zero_ir2_opnd,  &label_exit);
    latxs_ra_free_temp(&tmp_imm);

#ifdef TARGET_X86_64
    /* this also should work with 32 */
    IR2_OPND tmp_dest   = dest;
    IR2_OPND high_dest  = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SLL_D, &high_dest, &tmp_dest, &rotate);
    IR2_OPND tmp_rotate = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp_rotate,
                                         &rotate, -1 - opnd_size);
    latxs_append_ir2_opnd3(LISA_SUB_D, &tmp_rotate,
                                       &latxs_zero_ir2_opnd, &tmp_rotate);
    IR2_OPND low_dest   = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SRL_D, &low_dest, &tmp_dest, &tmp_rotate);
    if (opnd_size == 64) {
        latxs_append_ir2_opnd2i(LISA_ANDI, &tmp_rotate, &tmp_rotate, 0x40);
        latxs_append_ir2_opnd3(LISA_MASKNEZ, &low_dest, &low_dest, &tmp_rotate);
    }

    latxs_ra_free_temp(&tmp_rotate);
    IR2_OPND final_dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_OR, &final_dest, &high_dest, &low_dest);
    latxs_ra_free_temp(&low_dest);
    latxs_ra_free_temp(&high_dest);

    IR2_OPND cf = latxs_ra_alloc_itemp();
    latxs_get_eflag_condition(&cf, pir1);
    latxs_append_ir2_opnd3(LISA_SLL_D, &cf, &cf, &rotate);
    latxs_append_ir2_opnd2i(LISA_SRLI_D, &cf, &cf, 1);
    latxs_append_ir2_opnd3(LISA_OR, &final_dest, &final_dest, &cf);
    latxs_ra_free_temp(&cf);
#else
    IR2_OPND cf = latxs_ra_alloc_itemp();
    latxs_get_eflag_condition(&cf, pir1);

    lsassert(opnd_size != 64);

    if (opnd_size == 32) {
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &cf, &cf, 32);
    } else {
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &cf, &cf, opnd_size);
    }

    IR2_OPND tmp_dest   = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_OR, &tmp_dest, &dest, &cf);
    latxs_ra_free_temp(&cf);
    IR2_OPND high_dest  = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SLL_D, &high_dest, &tmp_dest, &rotate);
    IR2_OPND tmp_rotate = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp_rotate,
                                         &rotate, -1 - opnd_size);
    latxs_append_ir2_opnd3(LISA_SUB_D, &tmp_rotate,
                                       &latxs_zero_ir2_opnd, &tmp_rotate);
    IR2_OPND low_dest   = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SRL_D, &low_dest, &tmp_dest, &tmp_rotate);
    latxs_ra_free_temp(&tmp_dest);
    latxs_ra_free_temp(&tmp_rotate);
    IR2_OPND final_dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_OR, &final_dest, &high_dest, &low_dest);
    latxs_ra_free_temp(&low_dest);
    latxs_ra_free_temp(&high_dest);
#endif
    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&final_dest, opnd0);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&final_dest,
                &dest, &rotate, pir1, true);
        latxs_ra_free_temp(&rotate);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&final_dest,
                &dest, &rotate, pir1, true);
        latxs_ra_free_temp(&dest);
        latxs_ra_free_temp(&rotate);
        latxs_store_ir2_to_ir1(&final_dest, opnd0);
    }

    latxs_ra_free_temp(&final_dest);

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);

#ifdef TARGET_X86_64
    /* make sure high 32 bits set to zero. TODO: simplify */
    if (opnd_size == 32 && ir1_opnd_is_gpr(opnd0)) {
        IR2_OPND dest_opnd = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        latxs_append_ir2_opnd2_(lisa_mov32z, &dest_opnd, &dest_opnd);
    }
#endif
    return true;
}

bool latxs_translate_rcr(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    IR2_OPND src_rotate = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&dest, opnd0, EXMode_Z);
    latxs_load_ir1_to_ir2(&src_rotate, opnd1, EXMode_Z);

    int opnd_size = ir1_opnd_size(opnd0);

    IR2_OPND rotate = latxs_ra_alloc_itemp();

    int shift_mask = 0x1f;

#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64 && opnd_size == 64) {
        shift_mask = 0x3f;
    }
#endif
    latxs_append_ir2_opnd2i(LISA_ANDI, &rotate, &src_rotate, shift_mask);
    latxs_ra_free_temp(&src_rotate);

    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &rotate,
            &latxs_zero_ir2_opnd, &label_exit);

    IR2_OPND tmp_imm = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp_imm,
            &latxs_zero_ir2_opnd, opnd_size + 1);
    latxs_append_ir2_opnd3(LISA_MOD_DU, &rotate, &rotate,  &tmp_imm);
    latxs_append_ir2_opnd3(LISA_BEQ, &rotate,
            &latxs_zero_ir2_opnd, &label_exit);
    latxs_ra_free_temp(&tmp_imm);

#ifdef TARGET_X86_64
    /* this also should work with 32 */
    IR2_OPND tmp_dest = dest;
    IR2_OPND low_dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SRL_D, &low_dest, &tmp_dest, &rotate);
    IR2_OPND tmp_rotate = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp_rotate, &rotate, -1 - opnd_size);
    latxs_append_ir2_opnd3(LISA_SUB_D, &tmp_rotate, &latxs_zero_ir2_opnd,
                           &tmp_rotate);
    IR2_OPND high_dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SLL_D, &high_dest, &tmp_dest, &tmp_rotate);

    if (opnd_size == 64) {
        latxs_append_ir2_opnd2i(LISA_ANDI, &tmp_rotate, &tmp_rotate, 0x40);
        latxs_append_ir2_opnd3(LISA_MASKNEZ, &high_dest, &high_dest,
                               &tmp_rotate);
    }

    IR2_OPND final_dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_OR, &final_dest, &high_dest, &low_dest);
    latxs_ra_free_temp(&high_dest);
    latxs_ra_free_temp(&low_dest);
    IR2_OPND cf = latxs_ra_alloc_itemp();
    latxs_get_eflag_condition(&cf, pir1);
    latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp_rotate, &rotate, -opnd_size);
    latxs_append_ir2_opnd3(LISA_SUB_D, &tmp_rotate, &latxs_zero_ir2_opnd,
                           &tmp_rotate);
    latxs_append_ir2_opnd3(LISA_SLL_D, &cf, &cf, &tmp_rotate);
    latxs_append_ir2_opnd3(LISA_OR, &final_dest, &final_dest, &cf);
    latxs_ra_free_temp(&cf);
    latxs_ra_free_temp(&tmp_rotate);
#else
    IR2_OPND cf = latxs_ra_alloc_itemp();
    latxs_get_eflag_condition(&cf, pir1);

    lsassert(opnd_size != 64);
    if (opnd_size == 32) {
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &cf, &cf, 32);
    } else {
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &cf, &cf, opnd_size);
    }

    IR2_OPND tmp_dest   = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_OR, &tmp_dest, &dest, &cf);
    latxs_ra_free_temp(&cf);
    IR2_OPND low_dest   = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SRL_D, &low_dest, &tmp_dest, &rotate);
    IR2_OPND tmp_rotate = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp_rotate,
                                         &rotate, -1 - opnd_size);
    latxs_append_ir2_opnd3(LISA_SUB_D, &tmp_rotate,
                                       &latxs_zero_ir2_opnd, &tmp_rotate);
    IR2_OPND high_dest  = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SLL_D, &high_dest,  &tmp_dest, &tmp_rotate);
    latxs_ra_free_temp(&tmp_rotate);
    latxs_ra_free_temp(&tmp_dest);
    IR2_OPND final_dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_OR, &final_dest, &high_dest, &low_dest);
    latxs_ra_free_temp(&high_dest);
    latxs_ra_free_temp(&low_dest);
#endif

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&final_dest, opnd0);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&final_dest,
                &dest, &rotate, pir1, true);
        latxs_ra_free_temp(&rotate);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&final_dest,
                &dest, &rotate, pir1, true);
        latxs_ra_free_temp(&dest);
        latxs_ra_free_temp(&rotate);
        latxs_store_ir2_to_ir1(&final_dest, opnd0);
    }

    latxs_ra_free_temp(&final_dest);

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);

#ifdef TARGET_X86_64
    /* make sure high 32 bits set to zero. TODO: simplify */
    if (opnd_size == 32 && ir1_opnd_is_gpr(opnd0)) {
        IR2_OPND dest_opnd = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        latxs_append_ir2_opnd2_(lisa_mov32z, &dest_opnd, &dest_opnd);
    }
#endif
    return true;
}

static bool latxs_translate_shrd_cl(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR1_OPND *opnd2 = ir1_get_opnd(pir1, 2);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    IR2_OPND src = latxs_ra_alloc_itemp();
    IR2_OPND shift = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&dest, opnd0, EXMode_Z);
    latxs_load_ir1_to_ir2(&src, opnd1, EXMode_Z);
    latxs_load_ir1_to_ir2(&shift, opnd2, EXMode_Z);

    int opnd_size = ir1_opnd_size(opnd0);

    int shift_mask = 0x1f;
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64 && opnd_size == 64) {
        shift_mask = 0x3f;
    }
#endif
    latxs_append_ir2_opnd2i(LISA_ANDI, &shift, &shift, shift_mask);

    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &shift,
            &latxs_zero_ir2_opnd, &label_exit);

    IR2_OPND left_shift = latxs_ra_alloc_itemp();
    IR2_OPND size = latxs_ra_alloc_itemp();
    latxs_load_imm32_to_ir2(&size, opnd_size, EXMode_S);
    latxs_append_ir2_opnd3(LISA_SUB_W, &left_shift, &size, &shift);
    latxs_ra_free_temp(&size);
    latxs_ra_free_temp(&src);

    IR2_OPND high_dest  = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SLL_D, &high_dest, &src, &left_shift);
    latxs_ra_free_temp(&left_shift);

    IR2_OPND low_dest   = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SRL_D, &low_dest, &dest, &shift);

    IR2_OPND final_dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_OR, &final_dest, &high_dest, &low_dest);

    latxs_ra_free_temp(&low_dest);
    latxs_ra_free_temp(&high_dest);

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&final_dest, opnd0);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&final_dest,
                &dest, &shift, pir1, true);
        latxs_ra_free_temp(&shift);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&final_dest,
                &dest, &shift, pir1, true);
        latxs_ra_free_temp(&dest);
        latxs_ra_free_temp(&shift);
        latxs_store_ir2_to_ir1(&final_dest, opnd0);
    }

    latxs_ra_free_temp(&final_dest);

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);

#ifdef TARGET_X86_64
    /* make sure high 32 bits set to zero. TODO: simplify */
    if (opnd_size == 32 && ir1_opnd_is_gpr(opnd0)) {
        IR2_OPND dest_opnd = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        latxs_append_ir2_opnd2_(lisa_mov32z, &dest_opnd, &dest_opnd);
    }
#endif
    return true;
}

static bool latxs_translate_shrd_imm(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR1_OPND *opnd2 = ir1_get_opnd(pir1, 2);

    lsassertm_illop(ir1_addr(pir1), ir1_opnd_is_imm(opnd2),
            "shrd imm operand2 is not imm.\n");

    int opnd_size = ir1_opnd_size(opnd0);
    int shift_mask = 0x1f;
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64 && opnd_size == 64) {
        shift_mask = 0x3f;
    }
#endif

    int shift = ir1_opnd_simm(opnd2) & shift_mask;
    if (!shift) {
#ifdef TARGET_X86_64
        if (opnd_size == 32 && ir1_opnd_is_gpr(opnd0)) {
            IR2_OPND dest_opnd =
                latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
            latxs_append_ir2_opnd2_(lisa_mov32z, &dest_opnd, &dest_opnd);
        }
#endif
        return true;
    }

    IR2_OPND dest = latxs_ra_alloc_itemp();
    IR2_OPND src  = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&dest, opnd0, EXMode_Z);
    latxs_load_ir1_to_ir2(&src , opnd1, EXMode_Z);

    IR2_OPND low_dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_SRLI_D, &low_dest, &dest, shift);

    int left_shift = opnd_size - shift;
    if (opnd_size == 16 && shift > 16) {
        fprintf(stderr, "[warning] <%s> %s:%d : ", __func__,
                                             __FILE__, __LINE__);
        fprintf(stderr, "undefined behaviour, shrd\n");
        ir1_dump(pir1);
    }
    IR2_OPND high_dest = latxs_ra_alloc_itemp();
    IR2_OPND final_dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_SLLI_D, &high_dest, &src, left_shift);
    latxs_append_ir2_opnd3(LISA_OR, &final_dest, &high_dest, &low_dest);
    latxs_ra_free_temp(&src);
    latxs_ra_free_temp(&low_dest);
    latxs_ra_free_temp(&high_dest);

    IR2_OPND shift_opnd = latxs_ir2_opnd_new(IR2_OPND_IMMH, (int16)shift);
    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&final_dest, opnd0);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&final_dest,
                &dest, &shift_opnd, pir1, true);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&final_dest,
                &dest, &shift_opnd, pir1, true);
        latxs_ra_free_temp(&dest);
        latxs_store_ir2_to_ir1(&final_dest, opnd0);
    }

    latxs_ra_free_temp(&final_dest);

    return true;
}

bool latxs_translate_shrd(IR1_INST *pir1)
{
    if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 2)) {
        return latxs_translate_shrd_imm(pir1);
    } else {
        return latxs_translate_shrd_cl(pir1);
    }

    return true;
}

static bool latxs_translate_shld_cl(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR1_OPND *opnd2 = ir1_get_opnd(pir1, 2);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    IR2_OPND src = latxs_ra_alloc_itemp();
    IR2_OPND shift = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&dest, opnd0, EXMode_Z);
    latxs_load_ir1_to_ir2(&src, opnd1, EXMode_Z);
    latxs_load_ir1_to_ir2(&shift, opnd2, EXMode_Z);

    int opnd_size = ir1_opnd_size(opnd0);

    int shift_mask = 0x1f;
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64 && opnd_size == 64) {
        shift_mask = 0x3f;
    }
#endif
    latxs_append_ir2_opnd2i(LISA_ANDI, &shift, &shift, shift_mask);

    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &shift,
            &latxs_zero_ir2_opnd, &label_exit);

    IR2_OPND left_shift = latxs_ra_alloc_itemp();
    IR2_OPND size       = latxs_ra_alloc_itemp();
    latxs_load_imm32_to_ir2(&size, opnd_size, EXMode_S);
    latxs_append_ir2_opnd3(LISA_SUB_W, &left_shift, &size, &shift);
    latxs_ra_free_temp(&size);
    IR2_OPND low_dest   = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SRL_D, &low_dest, &src, &left_shift);
    latxs_ra_free_temp(&left_shift);
    latxs_ra_free_temp(&src);

    IR2_OPND high_dest  = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SLL_D, &high_dest, &dest, &shift);
    IR2_OPND final_dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_OR, &final_dest, &high_dest, &low_dest);

    latxs_ra_free_temp(&low_dest);
    latxs_ra_free_temp(&high_dest);

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&final_dest, opnd0);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&final_dest,
                &dest, &shift, pir1, true);
        latxs_ra_free_temp(&shift);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&final_dest,
                &dest, &shift, pir1, true);
        latxs_ra_free_temp(&shift);
        latxs_store_ir2_to_ir1(&final_dest, opnd0);
    }

    latxs_ra_free_temp(&final_dest);

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);

#ifdef TARGET_X86_64
    /* make sure high 32 bits set to zero. TODO: simplify */
    if (opnd_size == 32 && ir1_opnd_is_gpr(opnd0)) {
        IR2_OPND dest_opnd = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        latxs_append_ir2_opnd2_(lisa_mov32z, &dest_opnd, &dest_opnd);
    }
#endif
    return true;
}

static bool latxs_translate_shld_imm(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR1_OPND *opnd2 = ir1_get_opnd(pir1, 2);

    lsassertm_illop(ir1_addr(pir1), ir1_opnd_is_imm(opnd2),
            "shld imm operand2 is not imm.\n");

    int opnd_size = ir1_opnd_size(opnd0);
    int shift_mask = 0x1f;
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64 && opnd_size == 64) {
        shift_mask = 0x3f;
    }
#endif
    int shift = ir1_opnd_simm(opnd2) & shift_mask;
    if (!shift) {
#ifdef TARGET_X86_64
        if (opnd_size == 32 && ir1_opnd_is_gpr(opnd0)) {
            IR2_OPND dest_opnd =
                latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
            latxs_append_ir2_opnd2_(lisa_mov32z, &dest_opnd, &dest_opnd);
        }
#endif
        return true;
    }

    IR2_OPND dest = latxs_ra_alloc_itemp();
    IR2_OPND src  = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&dest, opnd0, EXMode_Z);
    latxs_load_ir1_to_ir2(&src , opnd1, EXMode_Z);

    IR2_OPND high_dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_SLLI_D, &high_dest, &dest, shift);

    int left_shift = opnd_size - shift;
    if (opnd_size == 16 && shift > 16) {
        fprintf(stderr, "[warning] <%s> %s:%d : ", __func__,
                                            __FILE__, __LINE__);
        fprintf(stderr, "undefined behaviour, shld\n");
        ir1_dump(pir1);
    }

    IR2_OPND low_dest = latxs_ra_alloc_itemp();
    IR2_OPND final_dest = latxs_ra_alloc_itemp();

    latxs_append_ir2_opnd2i(LISA_SRLI_D, &low_dest, &src, left_shift);
    latxs_append_ir2_opnd3(LISA_OR, &final_dest, &high_dest, &low_dest);

    latxs_ra_free_temp(&low_dest);
    latxs_ra_free_temp(&high_dest);
    latxs_ra_free_temp(&src);

    IR2_OPND shift_opnd = latxs_ir2_opnd_new(IR2_OPND_IMMH, (int16)shift);
    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&final_dest, opnd0);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&final_dest,
                &dest, &shift_opnd, pir1, true);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&final_dest,
                &dest, &shift_opnd, pir1, true);
        latxs_ra_free_temp(&dest);
        latxs_store_ir2_to_ir1(&final_dest, opnd0);
    }

    latxs_ra_free_temp(&final_dest);

    return true;
}

bool latxs_translate_shld(IR1_INST *pir1)
{
    if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 2)) {
        return latxs_translate_shld_imm(pir1);
    } else {
        return latxs_translate_shld_cl(pir1);
    }

    return true;
}

bool latxs_translate_bswap(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    int opnd_size = ir1_opnd_size(opnd0);

    IR2_OPND value = latxs_ra_alloc_itemp();

    if (opnd_size == 16) {
        /* qemu just perform 32bit swap */
        /* real machine perform set low 16 bit to zero */
        /* eg. 66 0f c8 */
        fprintf(stderr, "[warning] <%s> %s:%d : ", __func__,
                                    __FILE__, __LINE__);
        fprintf(stderr, "undefined behaviour, bswap\n");
        ir1_dump(pir1);
        /* TODO : common log(info/warning) */
        opnd0->size = 4;
    }

    latxs_load_ir1_to_ir2(&value, opnd0, EXMode_Z);

#ifdef TARGET_X86_64
    if (opnd_size == 64) {
        latxs_append_ir2_opnd2(LISA_REVB_D, &value, &value);
    } else {
        latxs_append_ir2_opnd2(LISA_REVB_2W, &value, &value);
    }
#else
    latxs_append_ir2_opnd2(LISA_REVB_2W, &value, &value);
#endif

    latxs_store_ir2_to_ir1(&value, opnd0);

    latxs_ra_free_temp(&value);
    return true;
}
