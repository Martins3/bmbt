#include "../include/common.h"
#include "../include/env.h"
#include "../include/reg-alloc.h"
#include "../include/flag-lbt.h"
#include "../x86tomips-options.h"

/* tr_arich.c in system mode
 *
 * 1. translation should follow precise exception
 * 2. generate illop exception instead of exit()
 *
 * *. only i386 is considered
 * *. only N64 is considered */

bool translate_shrd_imm(IR1_INST *pir1);
bool translate_shrd_cl(IR1_INST *pir1);
bool translate_shld_cl(IR1_INST *pir1);
bool translate_shld_imm(IR1_INST *pir1);

bool translate_xor(IR1_INST *pir1)
{
    if (option_by_hand) return translate_xor_byhand(pir1);

    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1     = ir1_get_opnd(pir1, 1);

    IR2_OPND src_opnd_0 = ra_alloc_itemp();
    IR2_OPND src_opnd_1 = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0, SIGN_EXTENSION, false);
    load_ir1_to_ir2(&src_opnd_1, opnd1, SIGN_EXTENSION, false);

    IR2_OPND dest_opnd = ra_alloc_itemp();
    append_ir2_opnd3(LISA_XOR, &dest_opnd, &src_opnd_0, &src_opnd_1);

    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&dest_opnd, opnd0, false);
        /* calculate elfags after store */
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
    }
    else {
        /* Destination is GPR, no exception will generate */
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
        store_ir2_to_ir1(&dest_opnd, opnd0, false);
    }

    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_and(IR1_INST *pir1)
{
    if (option_by_hand) return translate_and_byhand(pir1);

    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1     = ir1_get_opnd(pir1, 1);

    IR2_OPND src_opnd_0 = ra_alloc_itemp();
    IR2_OPND src_opnd_1 = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0, SIGN_EXTENSION, false);
    load_ir1_to_ir2(&src_opnd_1, opnd1, SIGN_EXTENSION, false);

    IR2_OPND dest_opnd = ra_alloc_itemp();
    append_ir2_opnd3(LISA_AND, &dest_opnd, &src_opnd_0, &src_opnd_1);

    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&dest_opnd, opnd0, false);
        /* calculate elfags after store */
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
    }
    else {
        /* Destination is GPR, no exception will generate */
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
        store_ir2_to_ir1(&dest_opnd, opnd0, false);
    }

    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_test(IR1_INST *pir1)
{
    if (option_by_hand) return translate_test_byhand(pir1);

    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1     = ir1_get_opnd(pir1, 1);

    IR2_OPND src_opnd_0 = ra_alloc_itemp();
    IR2_OPND src_opnd_1 = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0, SIGN_EXTENSION, false);
    load_ir1_to_ir2(&src_opnd_1, opnd1, SIGN_EXTENSION, false);

    IR2_OPND dest_opnd  = ra_alloc_itemp();

    append_ir2_opnd3(LISA_AND, &dest_opnd, &src_opnd_0, &src_opnd_1);

    generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);

    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_or(IR1_INST *pir1)
{
    if (option_by_hand) return translate_or_byhand(pir1);

    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1     = ir1_get_opnd(pir1, 1);

    IR2_OPND src_opnd_0 = ra_alloc_itemp();
    IR2_OPND src_opnd_1 = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0, SIGN_EXTENSION, false);
    load_ir1_to_ir2(&src_opnd_1, opnd1, SIGN_EXTENSION, false);

    IR2_OPND dest_opnd = ra_alloc_itemp();
    append_ir2_opnd3(LISA_OR, &dest_opnd, &src_opnd_0, &src_opnd_1);

    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&dest_opnd, opnd0, false);
        /* calculate elfags after store */
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
    }
    else {
        /* Destination is GPR, no exception will generate */
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
        store_ir2_to_ir1(&dest_opnd, opnd0, false);
    }

    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_not(IR1_INST *pir1)
{
    if (option_by_hand) return translate_not_byhand(pir1);

    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);
    IR2_OPND src_opnd_0 = ra_alloc_itemp();
    IR2_OPND dest_opnd  = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0, SIGN_EXTENSION, false);

    append_ir2_opnd2_(lisa_not, &dest_opnd, &src_opnd_0);

    store_ir2_to_ir1(&dest_opnd, opnd0, false);

    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_shl(IR1_INST *pir1)
{
    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1     = ir1_get_opnd(pir1, 1);

    IR2_OPND src = ra_alloc_itemp();
    load_ir1_to_ir2(&src       , opnd0, SIGN_EXTENSION, false);

    IR2_OPND src_shift = ra_alloc_itemp();
    load_ir1_to_ir2(&src_shift , opnd1, ZERO_EXTENSION, false);

    IR2_OPND shift = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ANDI, &shift, &src_shift, 0x1f);
    ra_free_temp(&src_shift);

    IR2_OPND label_exit = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BEQ, &shift, &zero_ir2_opnd, &label_exit);

    IR2_OPND dest = ra_alloc_itemp();
    append_ir2_opnd3(LISA_SLL_W, &dest, &src, &shift);

    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&dest, opnd0, false);
        /* calculate elfags after store */
        generate_eflag_calculation(&dest, &src, &shift, pir1, true);
        ra_free_temp(&dest);
        ra_free_temp(&shift);
    }
    else {
        /* Destination is GPR, no exception will generate */
        generate_eflag_calculation(&dest, &src, &shift, pir1, true);
        ra_free_temp(&src);
        ra_free_temp(&shift);
        store_ir2_to_ir1(&dest, opnd0, false);
        ra_free_temp(&dest);
    }

    append_ir2_opnd1(LISA_LABEL, &label_exit);

    return true;
}

bool translate_shr(IR1_INST *pir1)
{
    if (option_by_hand) return translate_shr_byhand(pir1);

    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1     = ir1_get_opnd(pir1, 1);

    int opsize = ir1_opnd_size(opnd0);
    EXTENSION_MODE    em = ZERO_EXTENSION;
    if (opsize == 32) em = SIGN_EXTENSION;

    IR2_OPND src = ra_alloc_itemp();
    load_ir1_to_ir2(&src, opnd0, em, false);

    IR2_OPND src_shift = ra_alloc_itemp();
    load_ir1_to_ir2(&src_shift, opnd1, ZERO_EXTENSION, false);

    IR2_OPND shift = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ANDI, &shift, &src_shift, 0x1f);
    ra_free_temp(&src_shift);

    IR2_OPND label_exit = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BEQ, &shift, &zero_ir2_opnd, &label_exit);

    IR2_OPND dest = ra_alloc_itemp();
    append_ir2_opnd3(LISA_SRL_W, &dest, &src, &shift);

    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&dest, opnd0, false);
        /* calculate elfags after store */
        generate_eflag_calculation(&dest, &src, &shift, pir1, true);
        ra_free_temp(&dest);
        ra_free_temp(&shift);
    }
    else {
        /* Destination is GPR, no exception will generate */
        generate_eflag_calculation(&dest, &src, &shift, pir1, true);
        ra_free_temp(&src);
        ra_free_temp(&shift);
        store_ir2_to_ir1(&dest, opnd0, false);
        ra_free_temp(&dest);
    }

    append_ir2_opnd1(LISA_LABEL, &label_exit);

    return true;
}

bool translate_sal(IR1_INST *pir1)
{
    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1     = ir1_get_opnd(pir1, 1);

    IR2_OPND src = ra_alloc_itemp();
    load_ir1_to_ir2(&src, opnd0, SIGN_EXTENSION, false);

    IR2_OPND src_shift = ra_alloc_itemp();
    load_ir1_to_ir2(&src_shift, opnd1, ZERO_EXTENSION, false);

    IR2_OPND shift = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ANDI, &shift, &src_shift, 0x1f);
    ra_free_temp(&src_shift);

    IR2_OPND label_exit = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BEQ, &shift, &zero_ir2_opnd, &label_exit);

    IR2_OPND dest = ra_alloc_itemp();
    append_ir2_opnd3(LISA_SLL_W, &dest, &src, &shift);

    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&dest, opnd0, false);
        /* calculate elfags after store */
        generate_eflag_calculation(&dest, &src, &shift, pir1, true);
        ra_free_temp(&dest);
        ra_free_temp(&shift);
    }
    else {
        /* Destination is GPR, no exception will generate */
        generate_eflag_calculation(&dest, &src, &shift, pir1, true);
        ra_free_temp(&src);
        ra_free_temp(&shift);
        store_ir2_to_ir1(&dest, opnd0, false);
        ra_free_temp(&dest);
    }

    append_ir2_opnd1(LISA_LABEL, &label_exit);

    return true;
}

bool translate_sar(IR1_INST *pir1)
{
    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1     = ir1_get_opnd(pir1, 1);

    IR2_OPND src        = ra_alloc_itemp();
    IR2_OPND src_shift  = ra_alloc_itemp();

    load_ir1_to_ir2(&src       , opnd0, SIGN_EXTENSION, false);
    load_ir1_to_ir2(&src_shift , opnd1, ZERO_EXTENSION, false);

    IR2_OPND shift = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ANDI, &shift, &src_shift, 0x1f);
    ra_free_temp(&src_shift);

    IR2_OPND label_exit = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BEQ, &shift, &zero_ir2_opnd, &label_exit);

    IR2_OPND dest = ra_alloc_itemp();
    append_ir2_opnd3(LISA_SRA_W, &dest, &src, &shift);

    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&dest, opnd0, false);
        /* calculate elfags after store */
        generate_eflag_calculation(&dest, &src, &shift, pir1, true);
        ra_free_temp(&dest);
        ra_free_temp(&shift);
    }
    else {
        /* Destination is GPR, no exception will generate */
        generate_eflag_calculation(&dest, &src, &shift, pir1, true);
        ra_free_temp(&src);
        ra_free_temp(&shift);
        store_ir2_to_ir1(&dest, opnd0, false);
        ra_free_temp(&dest);
    }

    append_ir2_opnd1(LISA_LABEL, &label_exit);

    return true;
}

bool translate_rol(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src_rotate = ra_alloc_itemp();
    load_ir1_to_ir2(&src_rotate, opnd1, ZERO_EXTENSION, false);

    IR2_OPND dest = ra_alloc_itemp();
    load_ir1_to_ir2(&dest, opnd0, ZERO_EXTENSION, false);

    IR2_OPND rotate = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ANDI, &rotate, &src_rotate, 0x1f);
    IR2_OPND label_exit = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BEQ, &rotate, &zero_ir2_opnd, &label_exit);

    int opnd_size = ir1_opnd_size(opnd0);
    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
        switch (opnd_size) {
        case 8:
            append_ir2_opnd2(LISA_X86ROTL_B, &dest, &src_rotate);
            break;
        case 16:
            append_ir2_opnd2(LISA_X86ROTL_H, &dest, &src_rotate);
            break;
        case 32:
            append_ir2_opnd2(LISA_X86ROTL_W, &dest, &src_rotate);
            break;
        default: break;
        }
    }
    if (ir2_opnd_is_itemp(&src_rotate))
        ra_free_temp(&src_rotate);

    if (opnd_size == 8)
        append_ir2_opnd2i(LISA_ANDI, &rotate, &rotate, 0x7);
    else if (opnd_size == 16)
        append_ir2_opnd2i(LISA_ANDI, &rotate, &rotate, 0xf);

    IR2_OPND label_calc_eflags = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BEQ, &rotate, &zero_ir2_opnd, &label_calc_eflags);

    IR2_OPND low_dest   = ra_alloc_itemp();
    IR2_OPND high_dest  = ra_alloc_itemp();
    IR2_OPND tmp_rotate = ra_alloc_itemp();

    append_ir2_opnd3 (LISA_SLL_D,  &low_dest,    &dest,           &rotate);
    append_ir2_opnd3 (LISA_SUB_W,  &tmp_rotate,  &zero_ir2_opnd,  &rotate);
    append_ir2_opnd2i(LISA_ANDI,   &tmp_rotate,  &tmp_rotate,      opnd_size - 1);
    append_ir2_opnd3 (LISA_SRL_D,  &high_dest,   &dest,           &tmp_rotate);
    append_ir2_opnd3 (LISA_OR,     &dest,        &high_dest,      &low_dest);
    ra_free_temp(&high_dest);
    ra_free_temp(&low_dest);
    ra_free_temp(&tmp_rotate);

    store_ir2_to_ir1(&dest, opnd0, false);

    append_ir2_opnd1(LISA_LABEL, &label_calc_eflags);
    generate_eflag_calculation(&dest, &dest, &rotate, pir1, true);

    ra_free_temp(&dest);
    ra_free_temp(&rotate);

    append_ir2_opnd1(LISA_LABEL, &label_exit);
    return true;
}

bool translate_ror(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src_rotate = ra_alloc_itemp();
    load_ir1_to_ir2(&src_rotate, opnd1, ZERO_EXTENSION, false);

    IR2_OPND dest = ra_alloc_itemp();
    load_ir1_to_ir2(&dest, opnd0, ZERO_EXTENSION, false);

    IR2_OPND rotate = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ANDI, &rotate, &src_rotate, 0x1f);
    IR2_OPND label_exit = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BEQ, &rotate, &zero_ir2_opnd, &label_exit);

    int opnd_size = ir1_opnd_size(opnd0);
    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
        switch (opnd_size) {
        case 8:
            append_ir2_opnd2(LISA_X86ROTR_B, &dest, &src_rotate);
            break;
        case 16:
            append_ir2_opnd2(LISA_X86ROTR_H, &dest, &src_rotate);
            break;
        case 32:
            append_ir2_opnd2(LISA_X86ROTR_W, &dest, &src_rotate);
            break;
        default: break;
        }
    }
    if (ir2_opnd_is_itemp(&src_rotate))
        ra_free_temp(&src_rotate);

    if (opnd_size == 8)
        append_ir2_opnd2i(LISA_ANDI, &rotate, &rotate, 0x7);
    else if (opnd_size == 16)
        append_ir2_opnd2i(LISA_ANDI, &rotate, &rotate, 0xf);

    IR2_OPND low_dest   = ra_alloc_itemp();
    IR2_OPND high_dest  = ra_alloc_itemp();
    IR2_OPND tmp_rotate = ra_alloc_itemp();

    IR2_OPND label_calc_eflags = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BEQ,   &rotate,     &zero_ir2_opnd, &label_calc_eflags);
    append_ir2_opnd3(LISA_SRL_D, &high_dest,  &dest,          &rotate);
    append_ir2_opnd3(LISA_SUB_W, &tmp_rotate, &zero_ir2_opnd, &rotate);
    append_ir2_opnd2i(LISA_ANDI, &tmp_rotate, &tmp_rotate,    opnd_size - 1);
    append_ir2_opnd3(LISA_SLL_D, &low_dest,   &dest,          &tmp_rotate);
    append_ir2_opnd3(LISA_OR,    &dest,       &high_dest,     &low_dest);

    ra_free_temp(&high_dest);
    ra_free_temp(&low_dest);
    ra_free_temp(&tmp_rotate);

    store_ir2_to_ir1(&dest, opnd0, false);

    append_ir2_opnd1(LISA_LABEL, &label_calc_eflags);
    generate_eflag_calculation(&dest, &dest, &rotate, pir1, true);

    ra_free_temp(&dest);
    ra_free_temp(&rotate);

    append_ir2_opnd1(LISA_LABEL, &label_exit);
    return true;
}

bool translate_rcl(IR1_INST *pir1)
{
    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1     = ir1_get_opnd(pir1, 1);
    IR2_OPND dest       = ra_alloc_itemp();
    IR2_OPND src_rotate = ra_alloc_itemp();

    load_ir1_to_ir2(&dest      , opnd0, ZERO_EXTENSION, false);
    load_ir1_to_ir2(&src_rotate, opnd1, ZERO_EXTENSION, false);

    int opnd_size = ir1_opnd_size(opnd0);

    IR2_OPND rotate = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ANDI, &rotate, &src_rotate, 0x1f);
    ra_free_temp(&src_rotate);

    IR2_OPND label_exit = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BEQ, &rotate, &zero_ir2_opnd, &label_exit);

    IR2_OPND tmp_imm = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ADDI_D, &tmp_imm,  &zero_ir2_opnd,   opnd_size + 1);
    append_ir2_opnd3(LISA_MOD_DU, &rotate, &rotate,  &tmp_imm);
//    append_ir2_opnd2 (mips_ddivu,  &rotate,   &tmp_imm);
//    append_ir2_opnd1 (mips_mfhi,   &rotate);
    append_ir2_opnd3 (LISA_BEQ,    &rotate,   &zero_ir2_opnd,  &label_exit);
    ra_free_temp(&tmp_imm);

    IR2_OPND cf = ra_alloc_itemp();
    get_eflag_condition(&cf, pir1);

    if (opnd_size == 32)
        append_ir2_opnd2i(LISA_SLLI_D, &cf, &cf, 32);
    else
        append_ir2_opnd2i(LISA_SLLI_D, &cf, &cf, opnd_size);

    IR2_OPND low_dest   = ra_alloc_itemp();
    IR2_OPND high_dest  = ra_alloc_itemp();
    IR2_OPND final_dest = ra_alloc_itemp();
    IR2_OPND tmp_dest   = ra_alloc_itemp();
    IR2_OPND tmp_rotate = ra_alloc_itemp();
    append_ir2_opnd3 (LISA_OR,     &tmp_dest,   &dest,          &cf);
    append_ir2_opnd3 (LISA_SLL_D,  &high_dest,  &tmp_dest,      &rotate);
    append_ir2_opnd2i(LISA_ADDI_D, &tmp_rotate, &rotate,         -1 - opnd_size);
    append_ir2_opnd3 (LISA_SUB_D,  &tmp_rotate, &zero_ir2_opnd, &tmp_rotate);
    append_ir2_opnd3 (LISA_SRL_D,  &low_dest,   &tmp_dest,      &tmp_rotate);
    append_ir2_opnd3 (LISA_OR,     &final_dest, &high_dest,     &low_dest);
    ra_free_temp(&cf);
    ra_free_temp(&low_dest);
    ra_free_temp(&high_dest);
    ra_free_temp(&tmp_rotate);
    ra_free_temp(&tmp_dest);

    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&final_dest, opnd0, false);
        /* calculate elfags after store */
        generate_eflag_calculation(&final_dest, &dest, &rotate, pir1, true);
        ra_free_temp(&rotate);
    }
    else {
        /* Destination is GPR, no exception will generate */
        generate_eflag_calculation(&final_dest, &dest, &rotate, pir1, true);
        ra_free_temp(&dest);
        ra_free_temp(&rotate);
        store_ir2_to_ir1(&final_dest, opnd0, false);
    }

    ra_free_temp(&final_dest);

    append_ir2_opnd1(LISA_LABEL, &label_exit);
    return true;
}

bool translate_rcr(IR1_INST *pir1)
{
    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1     = ir1_get_opnd(pir1, 1);

    IR2_OPND dest       = ra_alloc_itemp();
    IR2_OPND src_rotate = ra_alloc_itemp();

    load_ir1_to_ir2(&dest      , opnd0, ZERO_EXTENSION, false);
    load_ir1_to_ir2(&src_rotate, opnd1, ZERO_EXTENSION, false);

    int opnd_size = ir1_opnd_size(opnd0);

    IR2_OPND rotate = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ANDI, &rotate, &src_rotate, 0x1f);
    ra_free_temp(&src_rotate);

    IR2_OPND label_exit = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BEQ, &rotate, &zero_ir2_opnd, &label_exit);

    IR2_OPND tmp_imm = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ADDI_D, &tmp_imm,  &zero_ir2_opnd,   opnd_size + 1);
    append_ir2_opnd3(LISA_MOD_DU, &rotate, &rotate,  &tmp_imm);
//    append_ir2_opnd2 (mips_ddivu,  &rotate,   &tmp_imm);
//    append_ir2_opnd1 (mips_mfhi,   &rotate);
    append_ir2_opnd3 (LISA_BEQ,    &rotate,   &zero_ir2_opnd,  &label_exit);
    ra_free_temp(&tmp_imm);

    IR2_OPND cf = ra_alloc_itemp();
    get_eflag_condition(&cf, pir1);

    if (opnd_size == 32)
        append_ir2_opnd2i(LISA_SLLI_D, &cf, &cf, 32);
    else
        append_ir2_opnd2i(LISA_SLLI_D, &cf, &cf, opnd_size);

    IR2_OPND low_dest   = ra_alloc_itemp();
    IR2_OPND high_dest  = ra_alloc_itemp();
    IR2_OPND final_dest = ra_alloc_itemp();
    IR2_OPND tmp_rotate = ra_alloc_itemp();
    IR2_OPND tmp_dest   = ra_alloc_itemp();
    append_ir2_opnd3 (LISA_OR,     &tmp_dest,   &dest,          &cf);
    append_ir2_opnd3 (LISA_SRL_D,  &low_dest,   &tmp_dest,      &rotate);
    append_ir2_opnd2i(LISA_ADDI_D, &tmp_rotate, &rotate,         -1 - opnd_size);
    append_ir2_opnd3 (LISA_SUB_D,  &tmp_rotate, &zero_ir2_opnd, &tmp_rotate);
    append_ir2_opnd3 (LISA_SLL_D,  &high_dest,  &tmp_dest,      &tmp_rotate);
    append_ir2_opnd3 (LISA_OR,     &final_dest, &high_dest,     &low_dest);
    ra_free_temp(&cf);
    ra_free_temp(&high_dest);
    ra_free_temp(&low_dest);
    ra_free_temp(&tmp_rotate);
    ra_free_temp(&tmp_dest);

    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&final_dest, opnd0, false);
        /* calculate elfags after store */
        generate_eflag_calculation(&final_dest, &dest, &rotate, pir1, true);
        ra_free_temp(&rotate);
    }
    else {
        /* Destination is GPR, no exception will generate */
        generate_eflag_calculation(&final_dest, &dest, &rotate, pir1, true);
        ra_free_temp(&dest);
        ra_free_temp(&rotate);
        store_ir2_to_ir1(&final_dest, opnd0, false);
    }

    ra_free_temp(&final_dest);

    append_ir2_opnd1(LISA_LABEL, &label_exit);
    return true;
}

bool translate_shrd_cl(IR1_INST *pir1)
{
    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1     = ir1_get_opnd(pir1, 1);
    IR1_OPND *opnd2     = ir1_get_opnd(pir1, 2);

    IR2_OPND dest       = ra_alloc_itemp();
    IR2_OPND src        = ra_alloc_itemp();
    IR2_OPND src_shift  = ra_alloc_itemp();

    load_ir1_to_ir2(&dest      , opnd0, ZERO_EXTENSION, false);
    load_ir1_to_ir2(&src       , opnd1, ZERO_EXTENSION, false);
    load_ir1_to_ir2(&src_shift , opnd2, ZERO_EXTENSION, false);

    int opnd_size = ir1_opnd_size(opnd0);
    IR2_OPND shift = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ANDI, &shift, &src_shift, 0x1f);

    ra_free_temp(&src_shift);

    IR2_OPND label_exit = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BEQ, &shift, &zero_ir2_opnd, &label_exit);

    IR2_OPND low_dest   = ra_alloc_itemp();
    IR2_OPND high_dest  = ra_alloc_itemp();
    IR2_OPND final_dest = ra_alloc_itemp();
    IR2_OPND left_shift = ra_alloc_itemp();

    IR2_OPND size = ra_alloc_itemp();
    load_imm32_to_ir2(&size, opnd_size, SIGN_EXTENSION);

    append_ir2_opnd3(LISA_SUB_W,  &left_shift,  &size,       &shift);
    append_ir2_opnd3(LISA_SRL_D,  &low_dest,    &dest,       &shift);
    append_ir2_opnd3(LISA_SLL_D,  &high_dest,   &src,        &left_shift);
    append_ir2_opnd3(LISA_OR,     &final_dest,  &high_dest,  &low_dest);

    ra_free_temp(&low_dest);
    ra_free_temp(&high_dest);
    ra_free_temp(&left_shift);
    ra_free_temp(&size);
    ra_free_temp(&src);

    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&final_dest, opnd0, false);
        /* calculate elfags after store */
        generate_eflag_calculation(&final_dest, &dest, &shift, pir1, true);
        ra_free_temp(&shift);
    }
    else {
        /* Destination is GPR, no exception will generate */
        generate_eflag_calculation(&final_dest, &dest, &shift, pir1, true);
        ra_free_temp(&dest);
        ra_free_temp(&shift);
        store_ir2_to_ir1(&final_dest, opnd0, false);
    }

    ra_free_temp(&final_dest);

    append_ir2_opnd1(LISA_LABEL, &label_exit);
    return true;
}

bool translate_shrd_imm(IR1_INST *pir1)
{
    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1     = ir1_get_opnd(pir1, 1);
    IR1_OPND *opnd2     = ir1_get_opnd(pir1, 2);

    lsassertm_illop(ir1_addr(pir1), ir1_opnd_is_imm(opnd2),
            "shrd imm operand2 is not imm.\n");

    int shift = ir1_opnd_simm(opnd2) & 0x1f;
    if (!shift) return true;

    IR2_OPND dest = ra_alloc_itemp();
    IR2_OPND src  = ra_alloc_itemp();

    load_ir1_to_ir2(&dest, opnd0, ZERO_EXTENSION, false);
    load_ir1_to_ir2(&src , opnd1, ZERO_EXTENSION, false);

    int opnd_size = ir1_opnd_size(opnd0);

    IR2_OPND low_dest = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_SRLI_D, &low_dest, &dest, shift);

    int left_shift = opnd_size - shift;
    lsassertm_illop(ir1_addr(pir1), left_shift >= 0 && left_shift <= 31,
            "shrd imm left_shift %d is not valid. opsize = %d, shift = %d\n",
            left_shift, opnd_size, shift);

    IR2_OPND high_dest = ra_alloc_itemp();
    IR2_OPND final_dest = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_SLLI_D, &high_dest,  &src,        left_shift);
    append_ir2_opnd3 (LISA_OR,     &final_dest, &high_dest, &low_dest);
    ra_free_temp(&src);
    ra_free_temp(&low_dest);
    ra_free_temp(&high_dest);

    IR2_OPND shift_opnd = ir2_opnd_new(IR2_OPND_IMMH, (int16)shift);
    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&final_dest, opnd0, false);
        /* calculate elfags after store */
        generate_eflag_calculation(&final_dest, &dest, &shift_opnd, pir1, true);
    } else {
        /* Destination is GPR, no exception will generate */
        generate_eflag_calculation(&final_dest, &dest, &shift_opnd, pir1, true);
        ra_free_temp(&dest);
        store_ir2_to_ir1(&final_dest, opnd0, false);
    }

    ra_free_temp(&final_dest);

    return true;
}

bool translate_shrd(IR1_INST *pir1)
{
    if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 2))
        return translate_shrd_imm(pir1);
    else
        return translate_shrd_cl(pir1);

    return true;
}

bool translate_shld_cl(IR1_INST *pir1)
{
    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1     = ir1_get_opnd(pir1, 1);
    IR1_OPND *opnd2     = ir1_get_opnd(pir1, 2);

    IR2_OPND dest       = ra_alloc_itemp();
    IR2_OPND src        = ra_alloc_itemp();
    IR2_OPND src_shift  = ra_alloc_itemp();

    load_ir1_to_ir2(&dest      , opnd0, ZERO_EXTENSION, false);
    load_ir1_to_ir2(&src       , opnd1, ZERO_EXTENSION, false);
    load_ir1_to_ir2(&src_shift , opnd2, ZERO_EXTENSION, false);

    int opnd_size = ir1_opnd_size(opnd0);

    IR2_OPND shift = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ANDI, &shift, &src_shift, 0x1f);
    ra_free_temp(&src_shift);

    IR2_OPND label_exit = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BEQ, &shift, &zero_ir2_opnd, &label_exit);

    IR2_OPND size       = ra_alloc_itemp();
    IR2_OPND low_dest   = ra_alloc_itemp();
    IR2_OPND high_dest  = ra_alloc_itemp();
    IR2_OPND final_dest = ra_alloc_itemp();
    IR2_OPND left_shift = ra_alloc_itemp();

    load_imm32_to_ir2(&size, opnd_size, SIGN_EXTENSION);
    append_ir2_opnd3(LISA_SUB_W, &left_shift, &size,      &shift);
    append_ir2_opnd3(LISA_SLL_D, &high_dest,  &dest,      &shift);
    append_ir2_opnd3(LISA_SRL_D, &low_dest,   &src,       &left_shift);
    append_ir2_opnd3(LISA_OR,    &final_dest, &high_dest, &low_dest);

    ra_free_temp(&low_dest);
    ra_free_temp(&high_dest);
    ra_free_temp(&left_shift);
    ra_free_temp(&size);
    ra_free_temp(&src);

    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&final_dest, opnd0, false);
        /* calculate elfags after store */
        generate_eflag_calculation(&final_dest, &dest, &shift, pir1, true);
        ra_free_temp(&shift);
    }
    else {
        /* Destination is GPR, no exception will generate */
        generate_eflag_calculation(&final_dest, &dest, &shift, pir1, true);
        ra_free_temp(&shift);
        store_ir2_to_ir1(&final_dest, opnd0, false);
    }

    ra_free_temp(&final_dest);

    append_ir2_opnd1(LISA_LABEL, &label_exit);
    return true;
}

bool translate_shld_imm(IR1_INST *pir1)
{
    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1     = ir1_get_opnd(pir1, 1);
    IR1_OPND *opnd2     = ir1_get_opnd(pir1, 2);

    lsassertm_illop(ir1_addr(pir1), ir1_opnd_is_imm(opnd2),
            "shld imm operand2 is not imm.\n");
    int shift = ir1_opnd_simm(opnd2) & 0x1f;
    if (!shift) return true;

    IR2_OPND dest = ra_alloc_itemp();
    IR2_OPND src  = ra_alloc_itemp();

    load_ir1_to_ir2(&dest, opnd0, ZERO_EXTENSION, false);
    load_ir1_to_ir2(&src , opnd1, ZERO_EXTENSION, false);

    int opnd_size = ir1_opnd_size(opnd0);

    IR2_OPND high_dest  = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_SLLI_D, &high_dest, &dest, shift);

    int left_shift = opnd_size - shift;
    lsassertm_illop(ir1_addr(pir1), left_shift >= 0 && left_shift <= 31,
            "shld imm left_shift %d is not valid. opsize = %d, shift = %d\n",
            left_shift, opnd_size, shift);

    IR2_OPND low_dest   = ra_alloc_itemp();
    IR2_OPND final_dest = ra_alloc_itemp();

    append_ir2_opnd2i(LISA_SRLI_D, &low_dest,    &src,        left_shift);
    append_ir2_opnd3 (LISA_OR,     &final_dest,  &high_dest, &low_dest);

    ra_free_temp(&low_dest);
    ra_free_temp(&high_dest);
    ra_free_temp(&src);

    IR2_OPND shift_opnd = ir2_opnd_new(IR2_OPND_IMMH, (int16)shift);
    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&final_dest, opnd0, false);
        /* calculate elfags after store */
        generate_eflag_calculation(&final_dest, &dest, &shift_opnd, pir1, true);
    }
    else {
        /* Destination is GPR, no exception will generate */
        generate_eflag_calculation(&final_dest, &dest, &shift_opnd, pir1, true);
        ra_free_temp(&dest);
        store_ir2_to_ir1(&final_dest, opnd0, false);
    }

    ra_free_temp(&final_dest);

    return true;
}

bool translate_shld(IR1_INST *pir1)
{
    if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 2))
        return translate_shld_imm(pir1);
    else
        return translate_shld_cl(pir1);

    return true;
}

bool translate_bswap(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    int opnd_size = ir1_opnd_size(opnd0);

    lsassertm_illop(ir1_addr(pir1), opnd_size == 32,
            "bswap with opnd size = %d is unimplemented.\n", opnd_size);

    IR2_OPND value = ra_alloc_itemp();
    load_ir1_to_ir2(&value, opnd0, ZERO_EXTENSION, false);

    append_ir2_opnd2(LISA_REVB_2W, &value, &value);

    store_ir2_to_ir1(&value, opnd0, false);

    ra_free_temp(&value);
    return true;
}
