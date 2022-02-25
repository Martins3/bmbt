#include "common.h"
#include "env.h"
#include "reg-alloc.h"
#include "translate.h"


bool translate_das(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_das(pir1);
#else
    IR1_OPND *reg_al = &al_ir1_opnd;
    IR2_OPND old_al = load_ireg_from_ir1(reg_al, ZERO_EXTENSION, false);
    IR2_OPND new_al = ra_alloc_itemp();
    IR2_OPND eflag = ra_alloc_itemp();
    IR2_OPND imm_opnd = ra_alloc_itemp();
    IR2_OPND temp_opnd = ra_alloc_itemp();
    IR2_OPND label_al_sub_6 = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_al_sub_60 = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_next = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_clear_zf = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_zf = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);

    /* read af and cf */
    la_append_ir2_opnd1i_em(LISA_X86MFFLAG, eflag, 0x5);
    /* af == 1 ? */
    la_append_ir2_opnd2i_em(LISA_ANDI, temp_opnd, eflag, 0x10);
    la_append_ir2_opnd3(LISA_BNE, temp_opnd, zero_ir2_opnd, label_al_sub_6);
    /* (AL AND 0FH) > 9 ? */
    la_append_ir2_opnd2i_em(LISA_ANDI, temp_opnd, old_al, 0xf);
    la_append_ir2_opnd2i_em(LISA_ORI, imm_opnd, zero_ir2_opnd, 0xa);
    la_append_ir2_opnd3_em(LISA_OR, new_al, zero_ir2_opnd, old_al);
    la_append_ir2_opnd3(LISA_BLT, temp_opnd, imm_opnd, label_next);
    /* al = (al - 0x6) & 0xff and set af */
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x4);
    la_append_ir2_opnd1(LISA_LABEL, label_al_sub_6);
    la_append_ir2_opnd2i_em(LISA_ORI, imm_opnd, zero_ir2_opnd, 0x6);
    la_append_ir2_opnd3_em(LISA_SUB_W, new_al, old_al, imm_opnd);
    la_append_ir2_opnd2i_em(LISA_ANDI, new_al, new_al, 0xff);
    /* old_al < 6 ? */
    la_append_ir2_opnd3(LISA_BGE, old_al, imm_opnd, label_next);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x1);

    /* cf == 1 ? */
    la_append_ir2_opnd1(LISA_LABEL, label_next);
    la_append_ir2_opnd2i_em(LISA_ANDI, temp_opnd, eflag, 0x1);
    la_append_ir2_opnd3(LISA_BNE, temp_opnd, zero_ir2_opnd, label_al_sub_60);
    /* old_AL > 99H ? */
    la_append_ir2_opnd2i_em(LISA_ORI, imm_opnd, zero_ir2_opnd, 0x9a);
    la_append_ir2_opnd3(LISA_BLT, old_al, imm_opnd, label_zf);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x1);
    /* al = (al - 0x60) & 0xff */
    la_append_ir2_opnd1(LISA_LABEL, label_al_sub_60);
    la_append_ir2_opnd2i_em(LISA_ORI, imm_opnd, zero_ir2_opnd, 0x60);
    la_append_ir2_opnd3_em(LISA_SUB_W, new_al, new_al, imm_opnd);
    la_append_ir2_opnd2i_em(LISA_ANDI, new_al, new_al, 0xff);
    /* zf */
    la_append_ir2_opnd1(LISA_LABEL, label_zf);
    la_append_ir2_opnd3(LISA_BNE, new_al, zero_ir2_opnd, label_clear_zf);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x8);
    la_append_ir2_opnd1(LISA_B, label_exit);
    la_append_ir2_opnd1(LISA_LABEL, label_clear_zf);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x8);
    /* exit */
    la_append_ir2_opnd1(LISA_LABEL, label_exit);
    store_ireg_to_ir1(new_al, reg_al, false);
    generate_eflag_calculation(new_al, old_al, old_al, pir1, true);


    return true;
#endif
}

bool translate_aam(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_aam(pir1);
#else
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *reg_ah = &ah_ir1_opnd;
    IR1_OPND *reg_al = &al_ir1_opnd;
    IR2_OPND imm_opnd = load_ireg_from_ir1(opnd0, ZERO_EXTENSION, false);
    IR2_OPND al = load_ireg_from_ir1(reg_al, ZERO_EXTENSION, false);
    IR2_OPND old_al = ra_alloc_itemp();
    IR2_OPND ah = ra_alloc_itemp();

    la_append_ir2_opnd3_em(LISA_OR, old_al, zero_ir2_opnd, al);
    la_append_ir2_opnd3(LISA_DIV_D, ah, al, imm_opnd);
    la_append_ir2_opnd3(LISA_MOD_D, al, al, imm_opnd);

    store_ireg_to_ir1(ah, reg_ah, false);
    store_ireg_to_ir1(al, reg_al, false);
    generate_eflag_calculation(al, old_al, imm_opnd, pir1, true);

    return true;
#endif
}

bool translate_aaa(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_aaa(pir1);
#else
    IR1_OPND *reg_ax = &ax_ir1_opnd;
    IR1_OPND *reg_al = &al_ir1_opnd;
    IR2_OPND temp_opnd = ra_alloc_itemp();
    IR2_OPND imm_opnd = ra_alloc_itemp();
    IR2_OPND set_af_cf = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND ax = load_ireg_from_ir1(reg_ax, ZERO_EXTENSION, false);

    /* af */
    la_append_ir2_opnd1i_em(LISA_X86MFFLAG, temp_opnd, 0x4);
    /* af == 1 ? */
    la_append_ir2_opnd2i_em(LISA_ANDI, temp_opnd, temp_opnd, 0x10);
    la_append_ir2_opnd3(LISA_BNE, temp_opnd, zero_ir2_opnd, set_af_cf);
    /* (AL AND 0FH) > 9 ? */
    la_append_ir2_opnd2i_em(LISA_ANDI, temp_opnd, ax, 0xf);
    la_append_ir2_opnd2i_em(LISA_ORI, imm_opnd, zero_ir2_opnd, 0xa);
    la_append_ir2_opnd3(LISA_BGE, temp_opnd, imm_opnd, set_af_cf);
    /* clear af and cf*/
    la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x5);
    la_append_ir2_opnd1(LISA_B, label_exit);
    /* set clear af and cf*/
    la_append_ir2_opnd1(LISA_LABEL, set_af_cf);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x5);
    la_append_ir2_opnd2i_em(LISA_ADDI_D, ax, ax, 0x106);
    /* exit */
    la_append_ir2_opnd1(LISA_LABEL, label_exit);
    la_append_ir2_opnd2i_em(LISA_ANDI, temp_opnd, ax, 0xf);
    store_ireg_to_ir1(ax, reg_ax, false);
    store_ireg_to_ir1(temp_opnd, reg_al, false);

    return true;
#endif
}

bool translate_aas(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_aas(pir1);
#else
    IR1_OPND *reg_ax = &ax_ir1_opnd;
    IR1_OPND *reg_al = &al_ir1_opnd;
    IR1_OPND *reg_ah = &ah_ir1_opnd;
    IR2_OPND imm_opnd = ra_alloc_itemp();
    IR2_OPND temp_opnd = ra_alloc_itemp();
    IR2_OPND label_set_af_cf = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_sub_carry = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND ax_ir2 = load_ireg_from_ir1(reg_ax, ZERO_EXTENSION, false);
    IR2_OPND ah_ir2 = load_ireg_from_ir1(reg_ah, ZERO_EXTENSION, false);
    IR2_OPND al_ir2 = load_ireg_from_ir1(reg_al, ZERO_EXTENSION, false);

    la_append_ir2_opnd1i_em(LISA_X86MFFLAG, temp_opnd, 0x4);
    /* af == 1 ? */
    la_append_ir2_opnd2i_em(LISA_ANDI, temp_opnd, temp_opnd, 0x10);
    la_append_ir2_opnd3(LISA_BNE, temp_opnd, zero_ir2_opnd, label_set_af_cf);
    /* (AL AND 0FH) > 9 ? */
    la_append_ir2_opnd2i_em(LISA_ANDI, temp_opnd, ax_ir2, 0xf);
    la_append_ir2_opnd2i_em(LISA_ORI, imm_opnd, zero_ir2_opnd, 0xa);
    la_append_ir2_opnd3(LISA_BGE, temp_opnd, imm_opnd, label_set_af_cf);
    /* check eflags */
    la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x5);
    la_append_ir2_opnd1(LISA_B, label_exit);
    /* check if need sub carry */
    la_append_ir2_opnd1(LISA_LABEL, label_set_af_cf);
    la_append_ir2_opnd2i_em(LISA_ORI, imm_opnd, zero_ir2_opnd, 0x6);
    la_append_ir2_opnd3(LISA_BGE, al_ir2, imm_opnd, label_sub_carry);
    /* sub carry */
    la_append_ir2_opnd2i_em(LISA_ADDI_W, ah_ir2, ah_ir2, -1);
    /* without sub carry */
    la_append_ir2_opnd1(LISA_LABEL, label_sub_carry);
    la_append_ir2_opnd2i_em(LISA_ADDI_W, ax_ir2, ax_ir2, -6);
    la_append_ir2_opnd2i_em(LISA_ADDI_W, ah_ir2, ah_ir2, -1);
    /* set EFLAGS AF and CF flags */
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x5);
    la_append_ir2_opnd1(LISA_LABEL, label_exit);
    la_append_ir2_opnd2i_em(LISA_ANDI, al_ir2, ax_ir2, 0xf);
    store_ireg_to_ir1(al_ir2, reg_al, false);
    store_ireg_to_ir1(ah_ir2, reg_ah, false);
    return true;
#endif
}

bool translate_aad(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_aad(pir1);
#else
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *reg_ah = &ah_ir1_opnd;
    IR1_OPND *reg_al = &al_ir1_opnd;
    IR2_OPND imm_opnd;
    IR2_OPND al = load_ireg_from_ir1(reg_al, ZERO_EXTENSION, false);
    IR2_OPND old_al = load_ireg_from_ir1(reg_al, ZERO_EXTENSION, false);
    IR2_OPND ah = load_ireg_from_ir1(reg_ah, ZERO_EXTENSION, false);
    IR2_OPND temp_opnd = ra_alloc_itemp();

    if (ir1_opnd_type(opnd0) == X86_OP_IMM && ir1_opnd_size(opnd0) != 0) {
        imm_opnd = load_ireg_from_ir1(opnd0, ZERO_EXTENSION, false);
    } else {
        imm_opnd = ra_alloc_itemp();
        la_append_ir2_opnd2i_em(LISA_ORI, imm_opnd, zero_ir2_opnd, 0x0A);
    }

    la_append_ir2_opnd3(LISA_MUL_D, temp_opnd, ah, imm_opnd);
    la_append_ir2_opnd3(LISA_ADD_D, temp_opnd, temp_opnd, old_al);
    la_append_ir2_opnd2i_em(LISA_ANDI, al, temp_opnd, 0xFF);
    la_append_ir2_opnd2i_em(LISA_ANDI, ah, ah, 0x00);
    store_ireg_to_ir1(ah, reg_ah, false);
    store_ireg_to_ir1(al, reg_al, false);

    generate_eflag_calculation(al, old_al, imm_opnd, pir1, true);
    return true;
#endif
}

bool translate_daa(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_daa(pir1);
#else
    IR1_OPND *reg_al = &al_ir1_opnd;
    IR2_OPND temp_opnd = ra_alloc_itemp();
    IR2_OPND imm_opnd = ra_alloc_itemp();
    IR2_OPND not_clear_af = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND not_clear_cf = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_cf = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_zf = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_clear_zf = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND old_eax = load_ireg_from_ir1(reg_al, ZERO_EXTENSION, false);
    IR2_OPND new_eax = ra_alloc_itemp();

    /* af */
    la_append_ir2_opnd1i_em(LISA_X86MFFLAG, temp_opnd, 0x4);
    /* af == 1 ? */
    la_append_ir2_opnd2i_em(LISA_ANDI, temp_opnd, temp_opnd, 0x10);
    la_append_ir2_opnd3(LISA_BNE, temp_opnd, zero_ir2_opnd, not_clear_af);
    /* (AL AND 0FH) > 9 ? */
    la_append_ir2_opnd2i_em(LISA_ANDI, temp_opnd, old_eax, 0xf);
    la_append_ir2_opnd2i_em(LISA_ORI, imm_opnd, zero_ir2_opnd, 0xa);
    la_append_ir2_opnd3(LISA_BGE, temp_opnd, imm_opnd, not_clear_af);
    /* clear af */
    la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x4);
    la_append_ir2_opnd3_em(LISA_OR, new_eax, zero_ir2_opnd, old_eax);
    la_append_ir2_opnd1(LISA_B, label_cf);
    /* not clear af */
    la_append_ir2_opnd1(LISA_LABEL, not_clear_af);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x4);
    la_append_ir2_opnd2i_em(LISA_ADDI_W, new_eax, old_eax, 0x6);

    /* cf */
    la_append_ir2_opnd1(LISA_LABEL, label_cf);
    la_append_ir2_opnd1i_em(LISA_X86MFFLAG, temp_opnd, 0x1);
    /* cf == 1 ? */
    la_append_ir2_opnd2i_em(LISA_ANDI, temp_opnd, temp_opnd, 0x1);
    la_append_ir2_opnd3(LISA_BNE, temp_opnd, zero_ir2_opnd, not_clear_cf);
    /* old_AL > 99H ? */
    la_append_ir2_opnd2i_em(LISA_ORI, imm_opnd, zero_ir2_opnd, 0x9a);
    la_append_ir2_opnd3(LISA_BGE, old_eax, imm_opnd, not_clear_cf);
    /* clear cf */
    la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x1);
    la_append_ir2_opnd1(LISA_B, label_zf);
    /* not clear cf */
    la_append_ir2_opnd1(LISA_LABEL, not_clear_cf);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x1);
    la_append_ir2_opnd2i_em(LISA_ADDI_W, new_eax, new_eax, 0x60);
    /* zf */
    la_append_ir2_opnd1(LISA_LABEL, label_zf);
    load_ireg_from_imm32(temp_opnd, 0xffff, ZERO_EXTENSION);
    la_append_ir2_opnd3_em(LISA_AND, temp_opnd, temp_opnd, new_eax);
    la_append_ir2_opnd3(LISA_BNE, temp_opnd, zero_ir2_opnd, label_clear_zf);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x8);
    la_append_ir2_opnd1(LISA_B, label_exit);
    la_append_ir2_opnd1(LISA_LABEL, label_clear_zf);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x8);
    /* exit */
    la_append_ir2_opnd1(LISA_LABEL, label_exit);
    store_ireg_to_ir1(new_eax, reg_al, false);
    generate_eflag_calculation(new_eax, old_eax, old_eax, pir1, true);

    return true;
#endif
}

bool translate_add(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_add(pir1);
#else
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR2_OPND src_opnd0;
    IR2_OPND src_opnd1 = load_ireg_from_ir1(opnd1, SIGN_EXTENSION, false);

    if (ir1_is_prefix_lock(pir1)) {
        /* lock add m, imm/r */
        IR2_OPND lat_lock_addr;
        IR2_OPND dest_opnd = ra_alloc_itemp();
        IR2_OPND mem_opnd = mem_ir1_to_ir2_opnd(opnd0, false);
        int imm = ir2_opnd_imm(&mem_opnd);
        mem_opnd._type = IR2_OPND_IREG;
        lat_lock_addr = tr_lat_spin_lock(mem_opnd, imm);
        src_opnd0 = load_ireg_from_ir1(opnd0, SIGN_EXTENSION, false);
        la_append_ir2_opnd3_em(LISA_ADD_W, dest_opnd, src_opnd0, src_opnd1);
#ifdef CONFIG_LATX_FLAG_PATTERN
        fp_save_src_opnd(pir1, src_opnd0, src_opnd1);
#endif
        generate_eflag_calculation(dest_opnd, src_opnd0, src_opnd1, pir1, true);
        store_ireg_to_ir1(dest_opnd, opnd0, false);
        tr_lat_spin_unlock(lat_lock_addr);
        ra_free_temp(dest_opnd);
    } else if (ir1_opnd_is_gpr(opnd0) && ir1_opnd_size(opnd0) == 32) {
        /* lock add gpr32, imm/r/m */
        int opnd0_gpr_num = ir1_opnd_base_reg_num(opnd0);
        src_opnd0 = ra_alloc_gpr(opnd0_gpr_num);
        generate_eflag_calculation(src_opnd0, src_opnd0, src_opnd1, pir1, true);
        la_append_ir2_opnd3(LISA_ADD_W, src_opnd0, src_opnd0, src_opnd1);
        ir2_opnd_set_em(&src_opnd0, SIGN_EXTENSION, 32);
    } else {
        IR2_OPND dest_opnd = ra_alloc_itemp();
        src_opnd0 = load_ireg_from_ir1(opnd0, SIGN_EXTENSION, false);
        la_append_ir2_opnd3_em(LISA_ADD_W, dest_opnd, src_opnd0, src_opnd1);
#ifdef CONFIG_LATX_FLAG_PATTERN
        fp_save_src_opnd(pir1, src_opnd0, src_opnd1);
#endif
        generate_eflag_calculation(dest_opnd, src_opnd0, src_opnd1, pir1, true);
        store_ireg_to_ir1(dest_opnd, opnd0, false);
        ra_free_temp(dest_opnd);
    }

    return true;
#endif
}

bool translate_adc(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_adc(pir1);
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
    load_ireg_from_cf_opnd(&dest_opnd);
    la_append_ir2_opnd3_em(LISA_ADD_W, dest_opnd, dest_opnd, src_opnd_0);
    la_append_ir2_opnd3_em(LISA_ADD_W, dest_opnd, dest_opnd, src_opnd_1);
    generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    if (ir1_is_prefix_lock(pir1)) {
        tr_lat_spin_unlock(lat_lock_addr);
    }

    ra_free_temp(dest_opnd);
    return true;
#endif
}

bool translate_inc(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_inc(pir1);
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
    la_append_ir2_opnd2i_em(LISA_ADDI_W, dest_opnd, src_opnd_0, 1);
    IR2_OPND t_imm1 = ir2_opnd_new(IR2_OPND_IMM, 1);
    generate_eflag_calculation(dest_opnd, src_opnd_0, t_imm1, pir1, true);
    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    if (ir1_is_prefix_lock(pir1)) {
        tr_lat_spin_unlock(lat_lock_addr);
    }

    ra_free_temp(dest_opnd);
    return true;
#endif
}

bool translate_dec(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_dec(pir1);
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
    la_append_ir2_opnd2i_em(LISA_ADDI_W, dest_opnd, src_opnd_0, -1);
    IR2_OPND t_imm1 = ir2_opnd_new(IR2_OPND_IMM, 1);
    generate_eflag_calculation(dest_opnd, src_opnd_0, t_imm1, pir1, true);
    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    if (ir1_is_prefix_lock(pir1)) {
        tr_lat_spin_unlock(lat_lock_addr);
    }

    ra_free_temp(dest_opnd);
    return true;
#endif
}

bool translate_sub(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_sub(pir1);
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
#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);
#endif
    la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, src_opnd_0, src_opnd_1);
    generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    if (ir1_is_prefix_lock(pir1)) {
        tr_lat_spin_unlock(lat_lock_addr);
    }

    ra_free_temp(dest_opnd);
    return true;
#endif
}

bool translate_sbb(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_sbb(pir1);
#else
    IR2_OPND dest_opnd = ra_alloc_itemp();
    IR2_OPND src_opnd_1 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
    IR2_OPND lat_lock_addr;

    if (ir1_is_prefix_lock(pir1)) {
        IR2_OPND mem_opnd = mem_ir1_to_ir2_opnd(ir1_get_opnd(pir1, 0), false);
        int imm = ir2_opnd_imm(&mem_opnd);
        mem_opnd._type = IR2_OPND_IREG;
        lat_lock_addr = tr_lat_spin_lock(mem_opnd, imm);
	}

    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    load_ireg_from_cf_opnd(&dest_opnd);
    if (ir2_opnd_cmp(&src_opnd_0, &src_opnd_1)) {
        la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, zero_ir2_opnd, dest_opnd);
    } else {
        la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, src_opnd_0, dest_opnd);
        la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, dest_opnd, src_opnd_1);
    }
    generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    if (ir1_is_prefix_lock(pir1)) {
        tr_lat_spin_unlock(lat_lock_addr);
    }

    ra_free_temp(dest_opnd);
    return true;
#endif
}

bool translate_neg(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_neg(pir1);
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
    la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, zero_ir2_opnd, src_opnd_0);
    generate_eflag_calculation(dest_opnd, zero_ir2_opnd, src_opnd_0, pir1,
                               true);
    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    if (ir1_is_prefix_lock(pir1)) {
        tr_lat_spin_unlock(lat_lock_addr);
    }

    ra_free_temp(dest_opnd);
    return true;
#endif
}

bool translate_cmp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cmp(pir1);
#else
    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    IR2_OPND src_opnd_1 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);

    IR2_OPND dest_opnd = ra_alloc_itemp();
#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);
#endif
    la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, src_opnd_0, src_opnd_1);

    generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);

    ra_free_temp(dest_opnd);

    return true;
#endif
}

bool translate_mul(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_mul(pir1);
#else
    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
    IR2_OPND dest_opnd = ra_alloc_itemp();
    IR2_OPND src_opnd_1;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8) {
        src_opnd_1 = load_ireg_from_ir1(&al_ir1_opnd, ZERO_EXTENSION, false);
        generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
        ir2_opnd_set_em(&src_opnd_0,ZERO_EXTENSION,16);
        ir2_opnd_set_em(&src_opnd_1,ZERO_EXTENSION,16);
        la_append_ir2_opnd3_em(LISA_MUL_W, dest_opnd, src_opnd_0, src_opnd_1);
        store_ireg_to_ir1(dest_opnd, &ax_ir1_opnd, false);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {
        src_opnd_1 = load_ireg_from_ir1(&ax_ir1_opnd, ZERO_EXTENSION, false);
        generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
        la_append_ir2_opnd3_em(LISA_MUL_W, dest_opnd, src_opnd_0, src_opnd_1);
        store_ireg_to_ir1(dest_opnd, &ax_ir1_opnd, false);
        la_append_ir2_opnd2i_em(LISA_SRLI_W, dest_opnd, dest_opnd, 16);
        store_ireg_to_ir1(dest_opnd, &dx_ir1_opnd, false);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        src_opnd_1 = load_ireg_from_ir1(&eax_ir1_opnd, ZERO_EXTENSION, false);
        /*
         * NOTE: for X86 MUL insn, we have to make sure src_opnd_0 data is zero
         * extension, otherwise, mul data will be incorrect.
         */
        la_append_ir2_opnd3_em(LISA_AND, src_opnd_0, src_opnd_0, n1_ir2_opnd);
        generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
        la_append_ir2_opnd3_em(LISA_MUL_D, dest_opnd, src_opnd_0, src_opnd_1);
        store_ireg_to_ir1(dest_opnd, &eax_ir1_opnd, false);
        la_append_ir2_opnd2i_em(LISA_SRLI_D, dest_opnd, dest_opnd, 32);
        store_ireg_to_ir1(dest_opnd, &edx_ir1_opnd, false);
    } else {
        lsassertm(0, "64-bit translate_imul_1_opnd is unimplemented.\n");
        /* NOT_IMPLEMENTED; */
    }


    ra_free_temp(dest_opnd);
    return true;
#endif
}

#ifndef CONFIG_SOFTMMU
static bool translate_imul_1_opnd(IR1_INST *pir1)
{
    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    IR2_OPND dest = ra_alloc_itemp();
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8) {
        IR2_OPND src_opnd_1 =
            load_ireg_from_ir1(&al_ir1_opnd, SIGN_EXTENSION, false);
        la_append_ir2_opnd3_em(LISA_MUL_W, dest, src_opnd_1, src_opnd_0);
        /* calculate eflag */
        generate_eflag_calculation(dest, src_opnd_0, src_opnd_1, pir1, true);
        store_ireg_to_ir1(dest, &ax_ir1_opnd, false);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {
        IR2_OPND src_opnd_1 =
            load_ireg_from_ir1(&ax_ir1_opnd, SIGN_EXTENSION, false);
        la_append_ir2_opnd3_em(LISA_MUL_W, dest, src_opnd_1, src_opnd_0);
        /* calculate eflag */
        generate_eflag_calculation(dest, src_opnd_0, src_opnd_1, pir1, true);
        store_ireg_to_ir1(dest, &ax_ir1_opnd, false);
        la_append_ir2_opnd2i_em(LISA_SRLI_D, dest, dest, 16);
        store_ireg_to_ir1(dest, &dx_ir1_opnd, false);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd_1 =
            load_ireg_from_ir1(&eax_ir1_opnd, SIGN_EXTENSION, false);
        la_append_ir2_opnd3_em(LISA_MUL_D, dest, src_opnd_1, src_opnd_0);
        /* calculate eflag */
        generate_eflag_calculation(dest, src_opnd_0, src_opnd_1, pir1, true);
        store_ireg_to_ir1(dest, &eax_ir1_opnd, false);
        la_append_ir2_opnd2i_em(LISA_SRLI_D, dest, dest, 32);
        store_ireg_to_ir1(dest, &edx_ir1_opnd, false);
    } else {
        /* IR2_OPND dest_opnd_0 = */
        /* load_ireg_from_ir1(pir1->get_dest_opnd_implicit(0), SIGN_EXTENSION,
         */
        /* false); IR2_OPND dest_opnd_1 = */
        /* load_ireg_from_ir1(pir1->get_dest_opnd_implicit(1), SIGN_EXTENSION,
         */
        /* false); append_ir2_opnd2(mips_dmult, src_opnd_1, src_opnd_0); */
        /* append_ir2_opnd1(mips_mflo, dest_opnd_0); */
        /* append_ir2_opnd1(mips_mfhi, dest_opnd_1); */
        lsassertm(0, "64-bit translate_imul_1_opnd is unimplemented.\n");
    }
    ra_free_temp(dest);
    return true;
}
#endif

bool translate_imul(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_imul(pir1);
#else
    if (ir1_opnd_num(pir1) == 1)
        return translate_imul_1_opnd(pir1);

    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_src_opnd(pir1, 0), SIGN_EXTENSION, false);
    IR2_OPND src_opnd_1 =
        load_ireg_from_ir1(ir1_get_src_opnd(pir1, 1), SIGN_EXTENSION, false);

    IR2_OPND dest_opnd = ra_alloc_itemp();
    lsassertm(ir1_opnd_size(ir1_get_opnd(pir1, 0)) != 64,
              "64-bit translate_imul is unimplemented.\n");
    IR2_OPCODE opcode;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
        opcode = LISA_MUL_D;
    else
        opcode = LISA_MUL_W;
    la_append_ir2_opnd3_em(opcode, dest_opnd, src_opnd_1, src_opnd_0);
    generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);

    if (ir1_opnd_num(pir1) == 3)
        store_ireg_to_ir1(dest_opnd, ir1_get_dest_opnd(pir1, 0), false);
    else
        store_ireg_to_ir1(dest_opnd, ir1_get_src_opnd(pir1, 0), false);
    ra_free_temp(dest_opnd);
    return true;
#endif
}

bool translate_div(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_div(pir1);
#else
    IR2_OPND small_opnd =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
    IR2_OPND result = ra_alloc_itemp();
    IR2_OPND result_remainder = ra_alloc_itemp();

    IR2_OPND label_z = ir2_opnd_new_type(IR2_OPND_LABEL);

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8) {
        IR2_OPND large_opnd =
            load_ireg_from_ir1(&ax_ir1_opnd, ZERO_EXTENSION, false);

        la_append_ir2_opnd3(LISA_DIV_DU, result, large_opnd, small_opnd);
	la_append_ir2_opnd3(LISA_BNE, small_opnd, zero_ir2_opnd, label_z);
	la_append_ir2_opndi(LISA_BREAK, 0x7);
	la_append_ir2_opnd1(LISA_LABEL, label_z);

        la_append_ir2_opnd3(LISA_MOD_DU, result_remainder, large_opnd, small_opnd);
        /*  result larger than uint8 would raise an exception */
        ir2_opnd_set_em(&result, ZERO_EXTENSION, 8); 
        ir2_opnd_set_em(&result_remainder, ZERO_EXTENSION, 8); 

        /* set AL and AH at the same time */
        la_append_ir2_opnd2i_em(LISA_SLLI_D, result_remainder, result_remainder, 8);
        la_append_ir2_opnd3_em(LISA_OR, result, result, result_remainder);
        store_ireg_to_ir1(result, &ax_ir1_opnd, false);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {
        IR2_OPND large_opnd =
            load_ireg_from_ir1(&ax_ir1_opnd, ZERO_EXTENSION, false);
        IR2_OPND large_opnd_high_bits =
            load_ireg_from_ir1(&dx_ir1_opnd, ZERO_EXTENSION, false);
        la_append_ir2_opnd2i_em(LISA_SLLI_D, large_opnd_high_bits, large_opnd_high_bits, 16);
        la_append_ir2_opnd3_em(LISA_OR, large_opnd, large_opnd_high_bits, large_opnd);

        la_append_ir2_opnd3(LISA_DIV_DU, result, large_opnd, small_opnd);
	la_append_ir2_opnd3(LISA_BNE, small_opnd, zero_ir2_opnd, label_z);
	la_append_ir2_opndi(LISA_BREAK, 0x7);
	la_append_ir2_opnd1(LISA_LABEL, label_z);

        la_append_ir2_opnd3(LISA_MOD_DU, result_remainder, large_opnd, small_opnd);
        /*  result larger than uint16 would raise an exception */
        ir2_opnd_set_em(&result, ZERO_EXTENSION, 16);
        ir2_opnd_set_em(&result_remainder, ZERO_EXTENSION, 16);

        store_ireg_to_ir1(result, &ax_ir1_opnd, false);
        store_ireg_to_ir1(result_remainder, &dx_ir1_opnd, false);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND large_opnd =
            load_ireg_from_ir1(&eax_ir1_opnd, ZERO_EXTENSION, false);
        IR2_OPND large_opnd_high_bits =
            load_ireg_from_ir1(&edx_ir1_opnd, UNKNOWN_EXTENSION, false);
        la_append_ir2_opnd2i_em(LISA_SLLI_D, large_opnd_high_bits,
                          large_opnd_high_bits, 32);
        la_append_ir2_opnd3_em(LISA_OR, large_opnd, large_opnd_high_bits, large_opnd);

        IR2_OPND ir2_eax = ra_alloc_gpr(ir1_opnd_base_reg_num(&eax_ir1_opnd));
        IR2_OPND ir2_edx = ra_alloc_gpr(ir1_opnd_base_reg_num(&edx_ir1_opnd));

        /* load_ireg_from_ir1() may return the old register */
        if (ir2_eax._reg_num == large_opnd._reg_num) {
            IR2_OPND temp_large = ra_alloc_itemp();
            la_append_ir2_opnd3_em(LISA_OR, temp_large, large_opnd, zero_ir2_opnd);
            large_opnd = temp_large;
        }

        la_append_ir2_opnd3(LISA_DIV_DU, ir2_eax, large_opnd, small_opnd);
	la_append_ir2_opnd3(LISA_BNE, small_opnd, zero_ir2_opnd, label_z);
	la_append_ir2_opndi(LISA_BREAK, 0x7);
	la_append_ir2_opnd1(LISA_LABEL, label_z);
        la_append_ir2_opnd3(LISA_MOD_DU, ir2_edx, large_opnd, small_opnd);
        ir2_opnd_set_em(&ir2_eax, ZERO_EXTENSION, 32);
        ir2_opnd_set_em(&ir2_edx, ZERO_EXTENSION, 32);
    } else
        lsassertm(0, "64-bit translate_div is unimplemented.\n");

    ra_free_temp(result);
    ra_free_temp(result_remainder);
    return true;
#endif
}

bool translate_idiv(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_idiv(pir1);
#else
    IR2_OPND dest_opnd = ra_alloc_itemp();
    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);

    IR2_OPND label_z = ir2_opnd_new_type(IR2_OPND_LABEL);

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8) {
        IR2_OPND src_opnd_1 =
            load_ireg_from_ir1(&ax_ir1_opnd, SIGN_EXTENSION, false);
        la_append_ir2_opnd3(LISA_MOD_D, dest_opnd, src_opnd_1, src_opnd_0);
	la_append_ir2_opnd3(LISA_BNE, src_opnd_0, zero_ir2_opnd, label_z);
	la_append_ir2_opndi(LISA_BREAK, 0x7);
	la_append_ir2_opnd1(LISA_LABEL, label_z);
        la_append_ir2_opnd3(LISA_DIV_D, src_opnd_1, src_opnd_1, src_opnd_0);
        store_ireg_to_ir1(src_opnd_1, &al_ir1_opnd, false);
        store_ireg_to_ir1(dest_opnd, &ah_ir1_opnd, false);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {
        IR2_OPND src_opnd_1 =
            load_ireg_from_ir1(&ax_ir1_opnd, ZERO_EXTENSION, false);
        IR2_OPND src_opnd_2 =
            load_ireg_from_ir1(&dx_ir1_opnd, UNKNOWN_EXTENSION, false);
        IR2_OPND temp_src = ra_alloc_itemp();
        IR2_OPND temp1_opnd = ra_alloc_itemp();

        la_append_ir2_opnd2i_em(LISA_SLLI_W, temp_src, src_opnd_2, 16);
        la_append_ir2_opnd3(LISA_OR, temp_src, temp_src, src_opnd_1);
        la_append_ir2_opnd3(LISA_DIV_D, temp1_opnd, temp_src, src_opnd_0);
	la_append_ir2_opnd3(LISA_BNE, src_opnd_0, zero_ir2_opnd, label_z);
	la_append_ir2_opndi(LISA_BREAK, 0x7);
	la_append_ir2_opnd1(LISA_LABEL, label_z);
        store_ireg_to_ir1(temp1_opnd, &ax_ir1_opnd, false);
        la_append_ir2_opnd3(LISA_MOD_D, temp1_opnd, temp_src, src_opnd_0);
        store_ireg_to_ir1(temp1_opnd, &dx_ir1_opnd, false);

        ra_free_temp(temp_src);
        ra_free_temp(temp1_opnd);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd_1 =
            load_ireg_from_ir1(&eax_ir1_opnd, ZERO_EXTENSION, false);
        IR2_OPND src_opnd_2 =
            load_ireg_from_ir1(&edx_ir1_opnd, UNKNOWN_EXTENSION, false);
        IR2_OPND temp_src = ra_alloc_itemp();
        IR2_OPND temp1_opnd = ra_alloc_itemp();

        la_append_ir2_opnd2i_em(LISA_SLLI_D, temp_src, src_opnd_2, 32);
        la_append_ir2_opnd3_em(LISA_OR, temp_src, temp_src, src_opnd_1);
        la_append_ir2_opnd3(LISA_DIV_D, temp1_opnd, temp_src, src_opnd_0);
	la_append_ir2_opnd3(LISA_BNE, src_opnd_0, zero_ir2_opnd, label_z);
	la_append_ir2_opndi(LISA_BREAK, 0x7);
	la_append_ir2_opnd1(LISA_LABEL, label_z);
        store_ireg_to_ir1(temp1_opnd, &eax_ir1_opnd, false);
        la_append_ir2_opnd3(LISA_MOD_D, temp1_opnd, temp_src, src_opnd_0);
        store_ireg_to_ir1(temp1_opnd, &edx_ir1_opnd, false);

        ra_free_temp(temp_src);
        ra_free_temp(temp1_opnd);
    } else
        lsassertm(0, "64-bit translate_idiv is unimplemented.\n");

    ra_free_temp(dest_opnd);
    return true;
#endif
}

bool translate_xadd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_xadd(pir1);
#else
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR2_OPND src_opnd_1 =
        load_ireg_from_ir1(opnd1, SIGN_EXTENSION, false);
    IR2_OPND dest_opnd = ra_alloc_itemp();
    IR2_OPND lat_lock_addr;

    if (ir1_is_prefix_lock(pir1)) {
        IR2_OPND mem_opnd = mem_ir1_to_ir2_opnd(opnd0, false);
        int imm = ir2_opnd_imm(&mem_opnd);
        mem_opnd._type = IR2_OPND_IREG;
        lat_lock_addr = tr_lat_spin_lock(mem_opnd, imm);
	}

    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(opnd0, SIGN_EXTENSION, false);
    la_append_ir2_opnd3_em(LISA_ADD_W, dest_opnd, src_opnd_0, src_opnd_1);
    generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);

    if (ir1_opnd_is_gpr(opnd0)) {
        store_ireg_to_ir1(src_opnd_0, opnd1, false);
        store_ireg_to_ir1(dest_opnd, opnd0, false);
    } else {
        lsassert(ir1_opnd_is_mem(opnd0));
        store_ireg_to_ir1(dest_opnd, opnd0, false);
        store_ireg_to_ir1(src_opnd_0, opnd1, false);
    }

    if (ir1_is_prefix_lock(pir1)) {
        tr_lat_spin_unlock(lat_lock_addr);
    }

    ra_free_temp(dest_opnd);

    return true;
#endif
}
