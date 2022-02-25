#include "common.h"
#include "reg-alloc.h"
#include "flag-lbt.h"
#include "latx-options.h"
#include "translate.h"

bool translate_shrd_imm(IR1_INST *pir1);
bool translate_shrd_cl(IR1_INST *pir1);
bool translate_shld_cl(IR1_INST *pir1);
bool translate_shld_imm(IR1_INST *pir1);

bool translate_xor(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_xor(pir1);
#else
    IR2_OPND src_opnd_1 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);
    IR2_OPND dest_opnd = ra_alloc_itemp();

    IR2_OPND lat_lock_addr;

    if (ir1_is_prefix_lock(pir1)) {
        IR2_OPND mem_opnd = mem_ir1_to_ir2_opnd(ir1_get_opnd(pir1, 0), false);
        int imm = ir2_opnd_imm(&mem_opnd);
        mem_opnd._type = IR2_OPND_IREG;
        lat_lock_addr = tr_lat_spin_lock(mem_opnd, imm);
	}

    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    la_append_ir2_opnd3_em(LISA_XOR, dest_opnd, src_opnd_0, src_opnd_1);
    generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    if (ir1_is_prefix_lock(pir1)) {
        tr_lat_spin_unlock(lat_lock_addr);
    }

    ra_free_temp(dest_opnd);
    return true;
#endif
}

bool translate_and(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_and(pir1);
#else
    IR2_OPND src_opnd_1 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
    IR2_OPND dest_opnd = ra_alloc_itemp();

    IR2_OPND lat_lock_addr;

    if (ir1_is_prefix_lock(pir1)) {
        IR2_OPND mem_opnd = mem_ir1_to_ir2_opnd(ir1_get_opnd(pir1, 0), false);
        int imm = ir2_opnd_imm(&mem_opnd);
        mem_opnd._type = IR2_OPND_IREG;
        lat_lock_addr = tr_lat_spin_lock(mem_opnd, imm);
	}

    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    la_append_ir2_opnd3_em(LISA_AND, dest_opnd, src_opnd_0, src_opnd_1);
    generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    if (ir1_is_prefix_lock(pir1)) {
        tr_lat_spin_unlock(lat_lock_addr);
    }

    ra_free_temp(dest_opnd);
    return true;
#endif
}

bool translate_test(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_test(pir1);
#else
    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    IR2_OPND src_opnd_1 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);

    IR2_OPND dest_opnd = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_AND, dest_opnd, src_opnd_0, src_opnd_1);

    generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);

    ra_free_temp(dest_opnd);
    return true;
#endif
}

bool translate_or(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_or(pir1);
#else
    IR2_OPND src_opnd_1 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
    IR2_OPND dest_opnd = ra_alloc_itemp();

    IR2_OPND lat_lock_addr;

    if (ir1_is_prefix_lock(pir1)) {
        IR2_OPND mem_opnd = mem_ir1_to_ir2_opnd(ir1_get_opnd(pir1, 0), false);
        int imm = ir2_opnd_imm(&mem_opnd);
        mem_opnd._type = IR2_OPND_IREG;
        lat_lock_addr = tr_lat_spin_lock(mem_opnd, imm);
	}

    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    la_append_ir2_opnd3_em(LISA_OR, dest_opnd, src_opnd_0, src_opnd_1);
    generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    if (ir1_is_prefix_lock(pir1)) {
        tr_lat_spin_unlock(lat_lock_addr);
    }

    ra_free_temp(dest_opnd);
    return true;
#endif
}

bool translate_not(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_not(pir1);
#else
    IR2_OPND dest_opnd = ra_alloc_itemp();

    IR2_OPND lat_lock_addr;

    if (ir1_is_prefix_lock(pir1)) {
        IR2_OPND mem_opnd = mem_ir1_to_ir2_opnd(ir1_get_opnd(pir1, 0), false);
        int imm = ir2_opnd_imm(&mem_opnd);
        mem_opnd._type = IR2_OPND_IREG;
        lat_lock_addr = tr_lat_spin_lock(mem_opnd, imm);
	}

    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    la_append_ir2_opnd3_em(LISA_NOR, dest_opnd, zero_ir2_opnd, src_opnd_0);
    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    if (ir1_is_prefix_lock(pir1)) {
        tr_lat_spin_unlock(lat_lock_addr);
    }

    ra_free_temp(dest_opnd);
    return true;
#endif
}

bool translate_shl(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_shl(pir1);
#else
    IR2_OPND src = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    IR2_OPND original_count =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
    IR2_OPND dest = ra_alloc_itemp();

    IR2_OPND count = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ANDI, count, original_count, 31);
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_exit);

    la_append_ir2_opnd3_em(LISA_SLL_W, dest, src, count);

    generate_eflag_calculation(dest, src, count, pir1, true);

    store_ireg_to_ir1(dest, ir1_get_opnd(pir1, 0), false);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(count);
    ra_free_temp(dest);
    return true;
#endif
}

bool translate_shr(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_shr(pir1);
#else
    EXTENSION_MODE em = ZERO_EXTENSION;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
        em = SIGN_EXTENSION;
    IR2_OPND src = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), em, false);
    IR2_OPND original_count =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
    IR2_OPND dest = ra_alloc_itemp();

    IR2_OPND count = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ANDI, count, original_count, 31);
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_exit);

    la_append_ir2_opnd3_em(LISA_SRL_W, dest, src, count);

    generate_eflag_calculation(dest, src, count, pir1, true);

    store_ireg_to_ir1(dest, ir1_get_opnd(pir1, 0), false);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(dest);
    return true;
#endif
}

bool translate_sal(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_sal(pir1);
#else
    IR2_OPND src = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    IR2_OPND original_count =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
    IR2_OPND dest = ra_alloc_itemp();

    IR2_OPND count = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ANDI, count, original_count, 31);
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_exit);

    la_append_ir2_opnd3_em(LISA_SLL_W, dest, src, count);

    generate_eflag_calculation(dest, src, count, pir1, true);

    store_ireg_to_ir1(dest, ir1_get_opnd(pir1, 0), false);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(dest);
    return true;
#endif
}

bool translate_sar(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_sar(pir1);
#else
    IR2_OPND src = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    IR2_OPND original_count =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
    IR2_OPND dest = ra_alloc_itemp();

    IR2_OPND count = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ANDI, count, original_count, 31);
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_exit);

    la_append_ir2_opnd3(LISA_SRA_W, dest, src, count);

    generate_eflag_calculation(dest, src, count, pir1, true);

    store_ireg_to_ir1(dest, ir1_get_opnd(pir1, 0), false);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(count);
    ra_free_temp(dest);
    return true;
#endif
}

bool translate_rol(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_rol(pir1);
#else
    IR2_OPND dest = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
    //IR2_OPND dest = ra_alloc_itemp();
    //load_ireg_from_ir1_2(dest, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
    IR2_OPND original_count =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);

    IR2_OPND count = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ANDI, count, original_count, 0x1f);
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_exit);

    if (ir1_need_calculate_any_flag(pir1)) {
        if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8){
            la_append_ir2_opnd2(LISA_X86ROTL_B, dest, original_count);
        }
        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16){
            la_append_ir2_opnd2(LISA_X86ROTL_H, dest, original_count);
        }
        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32){
            la_append_ir2_opnd2(LISA_X86ROTL_W, dest, original_count);
        }
    }


    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
        la_append_ir2_opnd2i_em(LISA_ANDI, count, count, 7);
    else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16)
        la_append_ir2_opnd2i_em(LISA_ANDI, count, count, 15);

    IR2_OPND label_calc_eflags = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_calc_eflags);
    IR2_OPND low_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SLL_D, low_dest, dest, count);
    IR2_OPND tmp_count = ra_alloc_itemp();
    la_append_ir2_opnd3(LISA_SUB_W, tmp_count, zero_ir2_opnd, count);
    la_append_ir2_opnd2i_em(LISA_ANDI, tmp_count, tmp_count,
                      ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
    IR2_OPND high_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SRL_D, high_dest, dest, tmp_count);
    ra_free_temp(tmp_count);

    la_append_ir2_opnd3_em(LISA_OR, high_dest, high_dest, low_dest);

    store_ireg_to_ir1(high_dest, ir1_get_opnd(pir1, 0), false);

    la_append_ir2_opnd1(LISA_LABEL, label_calc_eflags);
    generate_eflag_calculation(dest, dest, count, pir1, true);

    ra_free_temp(high_dest);
    ra_free_temp(low_dest);
    ra_free_temp(count);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    return true;
#endif
}

bool translate_ror(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_ror(pir1);
#else
    IR2_OPND dest = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
    //IR2_OPND dest = ra_alloc_itemp();
    //load_ireg_from_ir1_2(dest, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
    IR2_OPND original_count =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);

    IR2_OPND count = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ANDI, count, original_count, 0x1f);

    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_exit);

    if (ir1_need_calculate_any_flag(pir1)) {
        if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8){
            la_append_ir2_opnd2(LISA_X86ROTR_B, dest, original_count);
        }
        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16){
            la_append_ir2_opnd2(LISA_X86ROTR_H, dest, original_count);
        }
        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32){
            la_append_ir2_opnd2(LISA_X86ROTR_W, dest, original_count);
        }
    }


    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
        la_append_ir2_opnd2i_em(LISA_ANDI, count, count, 7);
    else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16)
        la_append_ir2_opnd2i_em(LISA_ANDI, count, count, 15);

    IR2_OPND label_calc_eflags = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_calc_eflags);

    IR2_OPND high_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SRL_D, high_dest, dest, count);
    IR2_OPND tmp_count = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SUB_W, tmp_count, zero_ir2_opnd, count);
    la_append_ir2_opnd2i_em(LISA_ANDI, tmp_count, tmp_count,
                      ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
    IR2_OPND low_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SLL_D, low_dest, dest, tmp_count);
    ra_free_temp(tmp_count);

    la_append_ir2_opnd3_em(LISA_OR, high_dest, high_dest, low_dest);

    store_ireg_to_ir1(high_dest, ir1_get_opnd(pir1, 0), false);

    la_append_ir2_opnd1(LISA_LABEL, label_calc_eflags);
    generate_eflag_calculation(dest, dest, count, pir1, true);

    ra_free_temp(high_dest);
    ra_free_temp(low_dest);
    ra_free_temp(count);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);
    return true;
#endif
}

bool translate_rcl(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_rcl(pir1);
#else
    IR2_OPND dest = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
    IR2_OPND original_count =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);

    IR2_OPND count = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ANDI, count, original_count, 0x1f);
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_exit);

    IR2_OPND tmp_imm = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ADDI_D, tmp_imm, zero_ir2_opnd,
                      ir1_opnd_size(ir1_get_opnd(pir1, 0)) + 1);
    la_append_ir2_opnd3(LISA_MOD_DU, count, count, tmp_imm);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_exit);
    ra_free_temp(tmp_imm);

    IR2_OPND cf = ra_alloc_itemp();

    get_eflag_condition(&cf, pir1);

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
        la_append_ir2_opnd2i_em(LISA_SLLI_D, cf, cf, 32);
    else
        la_append_ir2_opnd2i_em(LISA_SLLI_D, cf, cf, ir1_opnd_size(ir1_get_opnd(pir1, 0)));
    IR2_OPND tmp_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_OR, tmp_dest, dest, cf);
    ra_free_temp(cf);

    IR2_OPND high_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SLL_D, high_dest, tmp_dest, count);

    IR2_OPND tmp_count = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ADDI_D, tmp_count, count,
                      -1 - ir1_opnd_size(ir1_get_opnd(pir1, 0)));
    la_append_ir2_opnd3_em(LISA_SUB_D, tmp_count, zero_ir2_opnd, tmp_count);

    IR2_OPND low_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SRL_D, low_dest, tmp_dest, tmp_count);
    ra_free_temp(tmp_count);
    ra_free_temp(tmp_dest);

    IR2_OPND final_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_OR, final_dest, high_dest, low_dest);
    ra_free_temp(high_dest);
    ra_free_temp(low_dest);

    generate_eflag_calculation(final_dest, dest, count, pir1, true);
    ra_free_temp(count);

    store_ireg_to_ir1(final_dest, ir1_get_opnd(pir1, 0), false);

    ra_free_temp(final_dest);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);
    return true;
#endif
}

bool translate_rcr(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_rcr(pir1);
#else
    IR2_OPND dest = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
    IR2_OPND original_count =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);

    IR2_OPND count = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ANDI, count, original_count, 0x1f);
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_exit);

    IR2_OPND tmp_imm = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ADDI_D, tmp_imm, zero_ir2_opnd,
                      ir1_opnd_size(ir1_get_opnd(pir1, 0)) + 1);
    la_append_ir2_opnd3(LISA_MOD_DU, count, count, tmp_imm);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_exit);
    ra_free_temp(tmp_imm);

    IR2_OPND cf = ra_alloc_itemp();

    get_eflag_condition(&cf, pir1);

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
        la_append_ir2_opnd2i_em(LISA_SLLI_D, cf, cf, 32);
    else
        la_append_ir2_opnd2i_em(LISA_SLLI_D, cf, cf, ir1_opnd_size(ir1_get_opnd(pir1, 0)));
    IR2_OPND tmp_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_OR, tmp_dest, dest, cf);
    ra_free_temp(cf);

    IR2_OPND low_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SRL_D, low_dest, tmp_dest, count);

    IR2_OPND tmp_count = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ADDI_D, tmp_count, count,
                      -1 - ir1_opnd_size(ir1_get_opnd(pir1, 0)));
    la_append_ir2_opnd3_em(LISA_SUB_D, tmp_count, zero_ir2_opnd, tmp_count);

    IR2_OPND high_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SLL_D, high_dest, tmp_dest, tmp_count);
    ra_free_temp(tmp_count);
    ra_free_temp(tmp_dest);

    IR2_OPND final_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_OR, final_dest, high_dest, low_dest);
    ra_free_temp(low_dest);
    ra_free_temp(high_dest);

    generate_eflag_calculation(final_dest, dest, count, pir1, true);
    ra_free_temp(count);

    store_ireg_to_ir1(final_dest, ir1_get_opnd(pir1, 0), false);

    ra_free_temp(final_dest);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);
    return true;
#endif
}

bool translate_shrd_cl(IR1_INST *pir1)
{
    IR2_OPND count_opnd =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 2, ZERO_EXTENSION, false);

    IR2_OPND count = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ANDI, count, count_opnd, 31);
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_exit);

    IR2_OPND size = ra_alloc_itemp();
    load_ireg_from_imm32(size, ir1_opnd_size(ir1_get_opnd(pir1, 0)), SIGN_EXTENSION);
    IR2_OPND left_count = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SUB_W, left_count, size, count);
    ra_free_temp(size);

    IR2_OPND dest_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
    IR2_OPND low_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SRL_D, low_dest, dest_opnd, count);

    IR2_OPND src_opnd =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
    IR2_OPND high_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SLL_D, high_dest, src_opnd, left_count);
    ra_free_temp(left_count);

    IR2_OPND final_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_OR, final_dest, high_dest, low_dest);
    ra_free_temp(low_dest);
    ra_free_temp(high_dest);

    generate_eflag_calculation(final_dest, dest_opnd, count, pir1, true);
    ra_free_temp(count);

    store_ireg_to_ir1(final_dest, ir1_get_opnd(pir1, 0), false);

    ra_free_temp(final_dest);
    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    return true;
}

bool translate_shrd_imm(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 2));
    int count = ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 2) & 0x1f;
    if (count == 0)
        return true;
    int left_count = ir1_opnd_size(ir1_get_opnd(pir1, 0)) - count;
    if (!(left_count >= 0 && left_count <= 31)) {
        return true;
    }
    IR2_OPND dest_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
    IR2_OPND low_dest = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_SRLI_D, low_dest, dest_opnd, count);

    IR2_OPND src_opnd =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
    IR2_OPND high_dest = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_SLLI_D, high_dest, src_opnd, left_count);

    IR2_OPND final_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_OR, final_dest, high_dest, low_dest);
    ra_free_temp(low_dest);
    ra_free_temp(high_dest);

    IR2_OPND count_opnd = ir2_opnd_new(IR2_OPND_IMM, (int16)count);
    generate_eflag_calculation(final_dest, dest_opnd, count_opnd, pir1, true);

    store_ireg_to_ir1(final_dest, ir1_get_opnd(pir1, 0), false);

    ra_free_temp(final_dest);

    return true;
}

bool translate_shrd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_shrd(pir1);
#else
    if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 2))
        return translate_shrd_imm(pir1);
    else
        return translate_shrd_cl(pir1);

    return true;
#endif
}

bool translate_shld_cl(IR1_INST *pir1)
{
    IR2_OPND count_opnd =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 2, ZERO_EXTENSION, false);

    IR2_OPND count = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ANDI, count, count_opnd, 31);
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_exit);

    IR2_OPND size = ra_alloc_itemp();
    load_ireg_from_imm32(size, ir1_opnd_size(ir1_get_opnd(pir1, 0)), SIGN_EXTENSION);
    IR2_OPND left_count = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SUB_W, left_count, size, count);
    ra_free_temp(size);

    IR2_OPND dest_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
    IR2_OPND high_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SLL_D, high_dest, dest_opnd, count);

    IR2_OPND src_opnd =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
    IR2_OPND low_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SRL_D, low_dest, src_opnd, left_count);
    ra_free_temp(left_count);

    IR2_OPND final_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_OR, final_dest, high_dest, low_dest);
    ra_free_temp(low_dest);
    ra_free_temp(high_dest);

    generate_eflag_calculation(final_dest, dest_opnd, count, pir1, true);
    ra_free_temp(count);

    store_ireg_to_ir1(final_dest, ir1_get_opnd(pir1, 0), false);

    ra_free_temp(final_dest);
    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    return true;
}

bool translate_shld_imm(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 2));
    int count = ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 2) & 0x1f;
    if (count == 0)
        return true;
    int left_count = ir1_opnd_size(ir1_get_opnd(pir1, 0)) - count;
    if (!(left_count >= 0 && left_count <= 31)) {
        return true;
    }
    IR2_OPND dest_opnd = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
    IR2_OPND high_dest = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_SLLI_D, high_dest, dest_opnd, count);

    IR2_OPND src_opnd =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
    IR2_OPND low_dest = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_SRLI_D, low_dest, src_opnd, left_count);

    IR2_OPND final_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_OR, final_dest, high_dest, low_dest);
    ra_free_temp(low_dest);
    ra_free_temp(high_dest);

    IR2_OPND count_opnd = ir2_opnd_new(IR2_OPND_IMM, (int16)count);
    generate_eflag_calculation(final_dest, dest_opnd, count_opnd, pir1, true);

    store_ireg_to_ir1(final_dest, ir1_get_opnd(pir1, 0), false);

    ra_free_temp(final_dest);

    return true;
}

bool translate_shld(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_shld(pir1);
#else
    if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 2))
        return translate_shld_imm(pir1);
    else
        return translate_shld_cl(pir1);

    return true;
#endif
}

bool translate_bswap(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_bswap(pir1);
#else
    IR2_OPND bswap_opnd;
    IR1_OPND *ir1_opnd = ir1_get_opnd(pir1, 0);
    int opnd_size = ir1_opnd_size(ir1_opnd);

    if(opnd_size == 16){
        ir1_opnd->size = (32 >> 3);
        switch (ir1_opnd->reg) {
            case X86_REG_AX:
                ir1_opnd->reg = X86_REG_EAX;
                break;
            case X86_REG_BX:
                ir1_opnd->reg = X86_REG_EBX;
                break;
            case X86_REG_CX:
                ir1_opnd->reg = X86_REG_ECX;
                break;
            case X86_REG_DX:
                ir1_opnd->reg = X86_REG_EDX;
                break;
            case X86_REG_SP:
                ir1_opnd->reg = X86_REG_ESP;
                break;
            case X86_REG_BP:
                ir1_opnd->reg = X86_REG_EBP;
                break;
            case X86_REG_SI:
                ir1_opnd->reg = X86_REG_ESI;
                break;
            case X86_REG_DI:
                ir1_opnd->reg = X86_REG_EDI;
                break;
            default:
                lsassert(0);
                break;
        }
        bswap_opnd =
            load_ireg_from_ir1(ir1_opnd, UNKNOWN_EXTENSION, false);
        la_append_ir2_opnd2ii(LISA_BSTRINS_W, bswap_opnd, zero_ir2_opnd, 15, 0);
    } else {
        lsassert(opnd_size == 32);

        bswap_opnd =
            load_ireg_from_ir1(ir1_opnd, UNKNOWN_EXTENSION, false);

        la_append_ir2_opnd2(LISA_REVB_2W, bswap_opnd, bswap_opnd);
    }
   /*
    * FIXME: high 32bit sign extension may corrupt, add.w 0 to resolve
    */
    //lsassert(ir2_opnd_is_sx(&bswap_opnd, 32));
    ir2_opnd_set_em(&bswap_opnd, SIGN_EXTENSION, 32);

    store_ireg_to_ir1(bswap_opnd, ir1_get_opnd(pir1, 0), false);

    return true;
#endif
}
