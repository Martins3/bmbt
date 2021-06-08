#include "common.h"
#include "reg_alloc.h"
#include "env.h"
#include "flag_lbt.h"
#include "x86tomips-options.h"

#ifndef CONFIG_SOFTMMU
/* pop in user-mode */
bool translate_pop(IR1_INST *pir1)
{
    lsassertm(0, "pop to be implemented in LoongArch.\n");
//    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
//
//    int esp_increment = 4;
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16)
//        esp_increment = 2;
//
//    /* 1. if esp is used or this pir1 is the last pushpop, We should update esp
//     */
//    /* first; o.w, increase curr_esp_need_decrease */
//    IR1_OPCODE next_opcode = ir1_opcode((IR1_INST *)(pir1 + 1));
//    if (ir1_opnd_is_gpr_used(ir1_get_opnd(pir1, 0), esp_index) ||
//        (next_opcode != X86_INS_PUSH && next_opcode != X86_INS_POP &&
//         next_opcode != X86_INS_PUSHF && next_opcode != X86_INS_POPF)) {
//        if (esp_increment - lsenv->tr_data->curr_esp_need_decrease != 0) {
//            append_ir2_opnd2i(mips_addi_addrx, &esp_opnd, &esp_opnd,
//                esp_increment - lsenv->tr_data->curr_esp_need_decrease);
//        }
//        lsenv->tr_data->curr_esp_need_decrease = 0;
//    } else {
//        lsenv->tr_data->curr_esp_need_decrease -= esp_increment;
//    }
//
//    /*  2. pop value from mem */
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {
//        IR1_OPND mem_ir1_opnd;
//        // ir1_opnd_build_mem(&mem_ir1_opnd, IR1_OPND_MEM, 16, 4,
//        //                -esp_increment - lsenv->tr_data->curr_esp_need_decrease);
//        ir1_opnd_build_mem(&mem_ir1_opnd, 16, X86_REG_ESP, 
//                        -esp_increment - lsenv->tr_data->curr_esp_need_decrease);
//
//        if (ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0)) &&
//            ir1_opnd_is_zx(ir1_get_opnd(pir1, 0), 16)) {
//            /* when dest is gpr, and high 16 bits are zero, load into gpr */
//            /* directly */
//            IR2_OPND dest_opnd;
//            load_ir1_to_ir2_ptr(&dest_opnd, ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
//            load_ir1_to_ir2(&dest_opnd, &mem_ir1_opnd, ZERO_EXTENSION, false);
//        }
//        else {
//            /* load value */
//            IR2_OPND value_opnd = ra_alloc_itemp();
//            /* Stack Addr size is default 4 in user-mode */
//            load_ir1_mem_to_ir2(&value_opnd, &mem_ir1_opnd, ZERO_EXTENSION, false, 4);
//            store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
//        }
//    } else {
//        IR1_OPND mem_ir1_opnd;
//        // ir1_opnd_build(&mem_ir1_opnd, IR1_OPND_MEM, 32, 4,
//        //                -esp_increment - lsenv->tr_data->curr_esp_need_decrease);
//        ir1_opnd_build_mem(&mem_ir1_opnd, 32, X86_REG_ESP,
//                       -esp_increment - lsenv->tr_data->curr_esp_need_decrease);
//
//        if (ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0))) {
//            /* when dest is gpr, load into gpr directly */
//            IR2_OPND dest_opnd;
//            load_ir1_to_ir2_ptr(&dest_opnd, ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
//            EXTENSION_MODE dest_em = SIGN_EXTENSION;
//            if (ir2_opnd_default_em(&dest_opnd) != SIGN_EXTENSION)
//                dest_em = ZERO_EXTENSION;
//
//            load_ir1_to_ir2(&dest_opnd, &mem_ir1_opnd, dest_em, false);
//        } else {
//            /* dest is mem, as normal */
//            IR2_OPND value_opnd = ra_alloc_itemp();
//            /* Stack Addr size is default 4 in user-mode */
//            load_ir1_mem_to_ir2(&value_opnd, &mem_ir1_opnd, SIGN_EXTENSION, false, 4);
//            store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
//        }
//    }
//
    return true;
}
#endif

#ifndef CONFIG_SOFTMMU
/* push in user-mode */
bool translate_push(IR1_INST *pir1)
{
    lsassertm(0, "push to be implemented in LoongArch.\n");
//    /* 1. if esp is used by push, update esp now */
//    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
//    if ((ir1_opnd_is_gpr_used(ir1_get_opnd(pir1, 0), esp_index) != 0) &&
//        (lsenv->tr_data->curr_esp_need_decrease != 0)) {
//        append_ir2_opnd2i(mips_addi_addrx, &esp_opnd, &esp_opnd,
//                          -lsenv->tr_data->curr_esp_need_decrease);
//        lsenv->tr_data->curr_esp_need_decrease = 0;
//    }
//
//    int esp_decrement = 4;
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16)
//        esp_decrement = 2;
//
//    /* 2. push value onto stack */
//    IR1_OPND mem_ir1_opnd;
//    // ir1_opnd_build(&mem_ir1_opnd, IR1_OPND_MEM, esp_decrement << 3, 4,
//    //                -esp_decrement - lsenv->tr_data->curr_esp_need_decrease);
//    ir1_opnd_build_mem(&mem_ir1_opnd, esp_decrement << 3, X86_REG_ESP,
//                    -esp_decrement - lsenv->tr_data->curr_esp_need_decrease);
//
//    IR2_OPND value_opnd = ra_alloc_itemp();
//    load_ir1_to_ir2(&value_opnd, ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
//    store_ir2_to_ir1(&value_opnd, &mem_ir1_opnd, false);
//
//    /* 3. adjust esp */
//    IR1_OPCODE next_opcode = ir1_opcode(((IR1_INST *)(pir1 + 1)));
//    if (next_opcode != X86_INS_PUSH && next_opcode != X86_INS_POP &&
//        next_opcode != X86_INS_PUSHF && next_opcode != X86_INS_POPF) {
//        append_ir2_opnd2i(mips_addi_addrx, &esp_opnd, &esp_opnd,
//            -esp_decrement - lsenv->tr_data->curr_esp_need_decrease);
//        lsenv->tr_data->curr_esp_need_decrease = 0;
//    } else
//        lsenv->tr_data->curr_esp_need_decrease += esp_decrement;

    return true;
}
#endif

/* End of TB in system-mode
 * > mov to cr/dr
 * > mov to es/cs/ss/ds (mov to cs will triger exception) */
bool translate_mov(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    TranslationBlock *tb = lsenv->tr_data->curr_tb;
    if (tb->cflags & CF_USE_ICOUNT) {
        if (ir1_is_mov_to_cr(pir1) || ir1_is_mov_from_cr(pir1)){
            tr_gen_io_start();
        }
    }

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    if (ir1_opnd_is_seg(opnd0) &&
        option_monitor_tbf &&
        ir1_opnd_base_reg_num(opnd0) == ss_index) {
        /* mark is mov operation, used in helper */
        IR2_OPND tmp = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ORI, &tmp, &zero_ir2_opnd, 0x1);
        append_ir2_opnd2i(LISA_ST_B, &tmp, &env_ir2_opnd,
                lsenv_offset_of_pf_data_tbf_is_mov(lsenv));
        ra_free_temp(&tmp);
    }
#endif

#endif

    if (option_by_hand) {
        bool success = translate_mov_byhand(pir1);
#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
        /* clear this mark which is used in helpr */
        if (option_monitor_tbf) {
            append_ir2_opnd2i(LISA_ST_B, &zero_ir2_opnd, &env_ir2_opnd,
                    lsenv_offset_of_pf_data_tbf_is_mov(lsenv));
        }
#endif
        return success;
    }

    IR2_OPND source_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&source_opnd, ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);
    store_ir2_to_ir1(&source_opnd, ir1_get_opnd(pir1, 0), false);

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
    /* clear this mark which is used in helpr */
    if (option_monitor_tbf) {
        append_ir2_opnd2i(LISA_ST_B, &zero_ir2_opnd, &env_ir2_opnd,
                lsenv_offset_of_pf_data_tbf_is_mov(lsenv));
    }
#endif

    return true;
}

bool translate_movzx(IR1_INST *pir1)
{
    if (option_by_hand) return translate_movzx_byhand(pir1);

    IR2_OPND source_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&source_opnd, ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);
    store_ir2_to_ir1(&source_opnd, ir1_get_opnd(pir1, 0), false);
    return true;
}

bool translate_movsx(IR1_INST *pir1)
{
    if (option_by_hand) return translate_movsx_byhand(pir1);

    IR2_OPND source_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&source_opnd, ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);
    store_ir2_to_ir1(&source_opnd, ir1_get_opnd(pir1, 0), false);
    return true;
}

#ifndef CONFIG_SOFTMMU
void load_string_step_to_ir2(IR2_OPND *p_step_opnd, IR1_INST *pir1)
{
    lsassertm(0, "load string step to be implemented in LoongArch.\n");
//    IR2_OPND df_opnd = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_andi, &df_opnd, &eflags_ir2_opnd, 0x400);
//
//    int bytes = ir1_opnd_size(ir1_get_opnd(pir1, 0)) >> 3;
//    int bits = 0;
//    switch (ir1_opnd_size(ir1_get_opnd(pir1, 0))) {
//    case 8:
//        bits = 9;
//        break;
//    case 16:
//        bits = 8;
//        break;
//    case 32:
//        bits = 7;
//        break;
//    }
//    lsassert(bits != 0);
//    IR2_OPND tmp_step = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_sra, &tmp_step, &df_opnd, bits);
//    append_ir2_opnd2i(mips_addiu, &*p_step_opnd, &tmp_step, 0 - bytes);
//
//    ra_free_temp(&df_opnd);
//    ra_free_temp(&tmp_step);
}
#endif

#ifndef CONFIG_SOFTMMU
bool translate_movs(IR1_INST *pir1)
{
    lsassertm(0, "movs to be implemented in LoongArch.\n");
//    BITS_SET(pir1->flags, FI_MDA);
//
//    /* 1. exit when initial count is zero */
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND ecx_opnd;
//    if (ir1_prefix(pir1) != 0) {
//        ecx_opnd = ra_alloc_itemp();
//        load_ir1_gpr_to_ir2(&ecx_opnd, &ecx_ir1_opnd, SIGN_EXTENSION);
//        append_ir2_opnd3(mips_beq, &ecx_opnd, &zero_ir2_opnd, &label_exit);
//    }
//
//    /* 2. preparations outside the loop */
//    IR2_OPND step_opnd = ra_alloc_itemp();
//    load_string_step_to_ir2(&step_opnd, pir1);
//
//    /* 3. loop starts */
//    IR2_OPND label_loop_begin = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_label, &label_loop_begin);
//
//    /* 3.1 load memory value at ESI, and store into memory at EDI */
//    IR2_OPND esi_mem_value = ra_alloc_itemp();
//    load_ir1_to_ir2(&esi_mem_value, ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);
//    store_ir2_to_ir1(&esi_mem_value, ir1_get_opnd(pir1, 0), false);
//
//    /* 3.2 adjust ESI and EDI */
//    IR2_OPND esi_opnd = ra_alloc_gpr(esi_index);
//    IR2_OPND edi_opnd = ra_alloc_gpr(edi_index);
//    append_ir2_opnd3(mips_sub_addrx, &esi_opnd, &esi_opnd, &step_opnd);
//    append_ir2_opnd3(mips_sub_addrx, &edi_opnd, &edi_opnd, &step_opnd);
//
//    /* 4. loop ends? when ecx==0 */
//    if (ir1_prefix(pir1) != 0) {
//        lsassert(ir1_prefix(pir1) == X86_PREFIX_REP);
//        append_ir2_opnd2i(mips_addiu, &ecx_opnd, &ecx_opnd, -1);
//        append_ir2_opnd3(mips_bne, &ecx_opnd, &zero_ir2_opnd, &label_loop_begin);
//    }
//
//    /* 5. exit */
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&step_opnd);
//    return true;
}
#endif

#ifndef CONFIG_SOFTMMU
bool translate_stos(IR1_INST *pir1)
{
    lsassertm(0, "stos to be implemented in LoongArch.\n");
//    BITS_SET(pir1->flags, FI_MDA);
//
//    /* 1. exit when initial count is zero */
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND ecx_opnd;
//    if (ir1_prefix(pir1) != 0) {
//        ecx_opnd = ra_alloc_itemp();
//        load_ir1_gpr_to_ir2(&ecx_opnd, &ecx_ir1_opnd, SIGN_EXTENSION);
//        append_ir2_opnd3(mips_beq, &ecx_opnd, &zero_ir2_opnd, &label_exit);
//    }
//
//    /* 2. preparations outside the loop */
//    IR2_OPND eax_value_opnd = ra_alloc_itemp();
//    load_ir1_to_ir2(&eax_value_opnd, ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);
//    IR2_OPND step_opnd = ra_alloc_itemp();
//    load_string_step_to_ir2(&step_opnd, pir1);
//
//    /* 3. loop starts */
//    IR2_OPND label_loop_begin = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_label, &label_loop_begin);
//
//    /* 3.1 store EAX into memory at EDI */
//    store_ir2_to_ir1(&eax_value_opnd, ir1_get_opnd(pir1, 0), false);
//
//    /* 3.2 adjust EDI */
//    IR2_OPND edi_opnd = ra_alloc_gpr(edi_index);
//    append_ir2_opnd3(mips_sub_addrx, &edi_opnd, &edi_opnd, &step_opnd);
//
//    /* 4. loop ends? when ecx==0 */
//    if (ir1_prefix(pir1) != 0) {
//        lsassert(ir1_prefix(pir1) == X86_PREFIX_REP);
//        append_ir2_opnd2i(mips_addiu, &ecx_opnd, &ecx_opnd, -1);
//        append_ir2_opnd3(mips_bne, &ecx_opnd, &zero_ir2_opnd, &label_loop_begin);
//    }
//
//    /* 5. exit */
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&step_opnd);
    return true;
}
#endif

#ifndef CONFIG_SOFTMMU
bool translate_lods(IR1_INST *pir1)
{
    lsassertm(0, "lods to be implemented in LoongArch.\n");
//    BITS_SET(pir1->flags, FI_MDA);
//    BITS_SET(pir1->flags, FI_MDA);
//
//    /* 1. exit when initial count is zero */
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND ecx_opnd;
//    if (ir1_prefix(pir1) != 0) {
//        ecx_opnd = ra_alloc_itemp();
//        load_ir1_gpr_to_ir2(&ecx_opnd, &ecx_ir1_opnd, SIGN_EXTENSION);
//        append_ir2_opnd3(mips_beq, &ecx_opnd, &zero_ir2_opnd, &label_exit);
//    }
//
//    /* 2. preparations outside the loop */
//    IR2_OPND step_opnd = ra_alloc_itemp();
//    load_string_step_to_ir2(&step_opnd, pir1);
//
//    /* 3. loop starts */
//    IR2_OPND label_loop_begin = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_label, &label_loop_begin);
//
//    /* 3.1 load memory value at ESI */
//    EXTENSION_MODE em = SIGN_EXTENSION;
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) < 32)
//        em = ZERO_EXTENSION;
//    IR2_OPND esi_mem_value = ra_alloc_itemp();
//    load_ir1_to_ir2(&esi_mem_value, ir1_get_opnd(pir1, 1), em, false);
//
//    /* 3.2 adjust ESI */
//    IR2_OPND esi_opnd = ra_alloc_gpr(esi_index);
//    append_ir2_opnd3(mips_sub_addrx, &esi_opnd, &esi_opnd, &step_opnd);
//
//    /* 4. loop ends? when ecx==0 */
//    if (ir1_prefix(pir1) != 0) {
//        lsassert(ir1_prefix(pir1) == X86_PREFIX_REP);
//        append_ir2_opnd2i(mips_addiu, &ecx_opnd, &ecx_opnd, -1);
//        append_ir2_opnd3(mips_bne, &ecx_opnd, &zero_ir2_opnd, &label_loop_begin);
//    }
//
//    store_ir2_to_ir1(&esi_mem_value, ir1_get_opnd(pir1, 0), false);
//
//    /* 5. exit */
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&step_opnd);
    return true;
}
#endif

#ifndef CONFIG_SOFTMMU
bool translate_cmps(IR1_INST *pir1)
{
    lsassertm(0, "cmps to be implemented in LoongArch.\n");
//    BITS_SET(pir1->flags, FI_MDA);
//
//    /* 1. exit when initial count is zero */
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND ecx_opnd;
//    if (ir1_prefix(pir1) != 0) {
//        ecx_opnd = ra_alloc_itemp();
//        load_ir1_gpr_to_ir2(&ecx_opnd, &ecx_ir1_opnd, SIGN_EXTENSION);
//        append_ir2_opnd3(mips_beq, &ecx_opnd, &zero_ir2_opnd, &label_exit);
//    }
//
//    /* 2. preparations outside the loop */
//    IR2_OPND step_opnd = ra_alloc_itemp();
//    load_string_step_to_ir2(&step_opnd, pir1);
//
//    /* 3. loop starts */
//    IR2_OPND label_loop_begin = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_label, &label_loop_begin);
//
//    /* 3.1 load memory value at ESI and EDI */
//    IR2_OPND esi_mem_value = ra_alloc_itemp();
//    IR2_OPND edi_mem_value = ra_alloc_itemp();
//    load_ir1_to_ir2(&esi_mem_value, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//    load_ir1_to_ir2(&edi_mem_value, ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);
//
//    /* 3.2 adjust ESI and EDI */
//    IR2_OPND esi_opnd = ra_alloc_gpr(esi_index);
//    IR2_OPND edi_opnd = ra_alloc_gpr(edi_index);
//    append_ir2_opnd3(mips_sub_addrx, &esi_opnd, &esi_opnd, &step_opnd);
//    append_ir2_opnd3(mips_sub_addrx, &edi_opnd, &edi_opnd, &step_opnd);
//
//    /* 3.3 compare */
//    IR2_OPND cmp_result = ra_alloc_itemp();
//    append_ir2_opnd3(mips_subu, &cmp_result, &esi_mem_value, &edi_mem_value);
//
//    /* 4. loop ends? */
//    if (ir1_prefix(pir1) != 0) {
//        append_ir2_opnd2i(mips_addiu, &ecx_opnd, &ecx_opnd, -1);
//        /* 4.1. loop ends when ecx==0 */
//        IR2_OPND condition = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_sltiu, &condition, &ecx_opnd,
//                          1); /* set 1 when ecx==0 */
//
//        /* 4.2 loop ends when result != 0 (repe), and when result==0 (repne) */
//        IR2_OPND condition2 = ra_alloc_itemp();
//        if (ir1_prefix(pir1) == X86_PREFIX_REP)
//            append_ir2_opnd3(mips_sltu, &condition2, &zero_ir2_opnd, &cmp_result); /* set 1 when 0<result, i.e., result!=0 */
//        else
//            append_ir2_opnd2i(mips_sltiu, &condition2, &cmp_result,
//                              1); /* set 1 when result<1, i.e., result==0 */
//
//        /* 4.3 when none of the two conditions is satisfied, the loop continues
//         */
//        append_ir2_opnd3(mips_or, &condition, &condition, &condition2);
//        append_ir2_opnd3(mips_beq, &condition, &zero_ir2_opnd, &label_loop_begin);
//        ra_free_temp(&condition);
//        ra_free_temp(&condition2);
//    }
//
//    /* 5. calculate eflags */
//    generate_eflag_calculation(&cmp_result, &esi_mem_value, &edi_mem_value, pir1, true);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&step_opnd);
//    ra_free_temp(&edi_mem_value);
//    ra_free_temp(&cmp_result);
    return true;
}
#endif

#ifndef CONFIG_SOFTMMU
bool translate_scas(IR1_INST *pir1)
{
    lsassertm(0, "scas to be implemented in LoongArch.\n");
//    BITS_SET(pir1->flags, FI_MDA);
//
//    /* 1. exit when initial count is zero */
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND ecx_opnd;
//    if (ir1_prefix(pir1) != 0) {
//        ecx_opnd = ra_alloc_itemp();
//        load_ir1_gpr_to_ir2(&ecx_opnd, &ecx_ir1_opnd, SIGN_EXTENSION);
//        append_ir2_opnd3(mips_beq, &ecx_opnd, &zero_ir2_opnd, &label_exit);
//    }
//
//    /* 2. preparations outside the loop */
//    IR2_OPND eax_value_opnd = ra_alloc_itemp();
//    load_ir1_to_ir2(&eax_value_opnd, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//    IR2_OPND step_opnd = ra_alloc_itemp();
//    load_string_step_to_ir2(&step_opnd, pir1);
//
//    /* 3. loop starts */
//    IR2_OPND label_loop_begin = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_label, &label_loop_begin);
//
//    /* 3.1 load memory value at EDI */
//    IR2_OPND edi_mem_value = ra_alloc_itemp();
//    load_ir1_to_ir2(&edi_mem_value, ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);
//    /* 3.2 adjust edi */
//    IR2_OPND edi_opnd = ra_alloc_gpr(edi_index);
//    append_ir2_opnd3(mips_sub_addrx, &edi_opnd, &edi_opnd, &step_opnd);
//
//    /* 3.3 compare */
//    IR2_OPND cmp_result = ra_alloc_itemp();
//    append_ir2_opnd3(mips_subu, &cmp_result, &eax_value_opnd, &edi_mem_value);
//
//    /* 4. loop ends? */
//    if (ir1_prefix(pir1) != 0) {
//        append_ir2_opnd2i(mips_addiu, &ecx_opnd, &ecx_opnd, -1);
//
//        /* 4.1. loop ends when ecx==0 */
//        IR2_OPND condition = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_sltiu, &condition, &ecx_opnd,
//                          1); /* set 1 when ecx==0 */
//
//        /* 4.2 loop ends when result != 0 (repe), and when result==0 (repne) */
//        IR2_OPND condition2 = ra_alloc_itemp();
//        if (ir1_prefix(pir1) == X86_PREFIX_REP)
//            append_ir2_opnd3(mips_sltu, &condition2, &zero_ir2_opnd, &cmp_result); /* set 1 when 0<result, i.e., result!=0 */
//        else
//            append_ir2_opnd2i(mips_sltiu, &condition2, &cmp_result,
//                              1); /* set 1 when result<1, i.e., result==0 */
//
//        /* 4.3 when none of the two conditions is satisfied, the loop continues
//         */
//        append_ir2_opnd3(mips_or, &condition, &condition, &condition2);
//        append_ir2_opnd3(mips_beq, &condition, &zero_ir2_opnd, &label_loop_begin);
//        ra_free_temp(&condition);
//        ra_free_temp(&condition2);
//    }
//
//    /* 5. calculate eflags */
//    generate_eflag_calculation(&cmp_result, &eax_value_opnd, &edi_mem_value, pir1,true);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&step_opnd);
//    ra_free_temp(&edi_mem_value);
//    ra_free_temp(&cmp_result);
    return true;
}
#endif

#ifndef CONFIG_SOFTMMU
#define CMOVCC_ASSERT(pir1, os0, os1) \
    do { \
        lsassert((os0 == 16 || os0 == 32) && os0 == os1); \
    } while(0)
#else
#define CMOVCC_ASSERT(pir1, os0, os1) \
    do { \
        lsassertm_illop(ir1_addr(pir1), \
            (os0 == 16 || os1 == 32) && os0 == os1, \
            "cmov opnd size0 %d size1 %d is unsupported.\n", \
            os0, os1); \
    } while(0)
#endif


bool translate_cmovo(IR1_INST *pir1)
{
    IR2_OPND condition = ra_alloc_itemp(); /* OF */
#ifndef CONFIG_SOFTMMU
    if (!fp_translate_pattern_tail(pir1, condition)) {
        get_eflag_condition(&condition, pir1);
    }
#else
    get_eflag_condition(&condition, pir1);
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&src_opnd, opnd1, UNKNOWN_EXTENSION, false);

    IR2_OPND no_mov = ir2_opnd_new_label(); /* mov if OF = 1 */
    append_ir2_opnd2(LISA_BEQZ, &condition, &no_mov);
    store_ir2_to_ir1(&src_opnd, opnd0, false);
    append_ir2_opnd1(LISA_LABEL, &no_mov);

    ra_free_temp(&condition);
    return true;
}
                         
bool translate_cmovno(IR1_INST *pir1)
{
    IR2_OPND condition = ra_alloc_itemp(); /* OF */
#ifndef CONFIG_SOFTMMU
    if (!fp_translate_pattern_tail(pir1, condition)) {
        get_eflag_condition(&condition, pir1);
    }
#else
    get_eflag_condition(&condition, pir1);
#endif
                         
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&src_opnd, opnd1, UNKNOWN_EXTENSION, false);

    IR2_OPND no_mov = ir2_opnd_new_label(); /* mov if OF = 0 */
    append_ir2_opnd2(LISA_BNEZ, &condition, &no_mov);
    store_ir2_to_ir1(&src_opnd, opnd0, false);
    append_ir2_opnd1(LISA_LABEL, &no_mov);

    ra_free_temp(&condition);
    return true;
}

bool translate_cmovb(IR1_INST *pir1)
{
    IR2_OPND condition = ra_alloc_itemp(); /* CF */
#ifndef CONFIG_SOFTMMU
    if (!fp_translate_pattern_tail(pir1, condition)) {
        get_eflag_condition(&condition, pir1);
    }
#else
    get_eflag_condition(&condition, pir1);
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&src_opnd, opnd1, UNKNOWN_EXTENSION, false);

    IR2_OPND no_mov = ir2_opnd_new_label(); /* mov if CF = 1 */
    append_ir2_opnd2(LISA_BEQZ, &condition, &no_mov);
    store_ir2_to_ir1(&src_opnd, opnd0, false);
    append_ir2_opnd1(LISA_LABEL, &no_mov);

    ra_free_temp(&condition);
    return true;
}
                         
bool translate_cmovae(IR1_INST *pir1)
{
    IR2_OPND condition = ra_alloc_itemp(); /* CF */
#ifndef CONFIG_SOFTMMU
    if (!fp_translate_pattern_tail(pir1, condition)) {
        get_eflag_condition(&condition, pir1);
    }
#else
    get_eflag_condition(&condition, pir1);
#endif
                         
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&src_opnd, opnd1, UNKNOWN_EXTENSION, false);

    IR2_OPND no_mov = ir2_opnd_new_label(); /* mov if CF = 0 */
    append_ir2_opnd2(LISA_BNEZ, &condition, &no_mov);
    store_ir2_to_ir1(&src_opnd, opnd0, false);
    append_ir2_opnd1(LISA_LABEL, &no_mov);

    ra_free_temp(&condition);
    return true;
}

bool translate_cmovz(IR1_INST *pir1)
{
    IR2_OPND condition = ra_alloc_itemp(); /* ZF */
#ifndef CONFIG_SOFTMMU
    if (!fp_translate_pattern_tail(pir1, condition)) {
        get_eflag_condition(&condition, pir1);
    }
#else
    get_eflag_condition(&condition, pir1);
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd  = ra_alloc_itemp();
    load_ir1_to_ir2(&src_opnd, opnd1, UNKNOWN_EXTENSION, false);

    IR2_OPND no_mov = ir2_opnd_new_label(); /* mov if ZF = 1 */
    append_ir2_opnd2(LISA_BEQZ, &condition, &no_mov);
    store_ir2_to_ir1(&src_opnd, opnd0, false);
    append_ir2_opnd1(LISA_LABEL, &no_mov);

    ra_free_temp(&condition);
    return true;
}

bool translate_cmovnz(IR1_INST *pir1)
{
    IR2_OPND condition = ra_alloc_itemp(); /* ZF */
#ifndef CONFIG_SOFTMMU
    if (!fp_translate_pattern_tail(pir1, condition)) {
        get_eflag_condition(&condition, pir1);
    }
#else
    get_eflag_condition(&condition, pir1);
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&src_opnd, opnd1, UNKNOWN_EXTENSION, false);

    IR2_OPND no_mov = ir2_opnd_new_label(); /* mov if ZF = 0 */
    append_ir2_opnd2(LISA_BNEZ, &condition, &no_mov);
    store_ir2_to_ir1(&src_opnd, opnd0, false);
    append_ir2_opnd1(LISA_LABEL, &no_mov);

    ra_free_temp(&condition);
    return true;
}

bool translate_cmovbe(IR1_INST *pir1)
{
    IR2_OPND condition = ra_alloc_itemp(); /* ZF, CF */
#ifndef CONFIG_SOFTMMU
    if (!fp_translate_pattern_tail(pir1, condition)) {
        get_eflag_condition(&condition, pir1);
    }
#else
    get_eflag_condition(&condition, pir1);
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&src_opnd, opnd1, UNKNOWN_EXTENSION, false);

    IR2_OPND no_mov = ir2_opnd_new_label(); /* mov if ZF = 1 or CF = 1 */
    append_ir2_opnd2(LISA_BEQZ, &condition, &no_mov);
    store_ir2_to_ir1(&src_opnd, opnd0, false);
    append_ir2_opnd1(LISA_LABEL, &no_mov);

    ra_free_temp(&condition);
    return true;
}

bool translate_cmova(IR1_INST *pir1)
{
    IR2_OPND condition = ra_alloc_itemp(); /* ZF, CF */
#ifndef CONFIG_SOFTMMU
    if (!fp_translate_pattern_tail(pir1, condition)) {
        get_eflag_condition(&condition, pir1);
    }
#else
    get_eflag_condition(&condition, pir1);
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&src_opnd, opnd1, UNKNOWN_EXTENSION, false);

    IR2_OPND no_mov = ir2_opnd_new_label(); /* mov if ZF = 0 && CF = 0 */
    append_ir2_opnd2(LISA_BNEZ, &condition, &no_mov);
    store_ir2_to_ir1(&src_opnd, opnd0, false);
    append_ir2_opnd1(LISA_LABEL, &no_mov);

    ra_free_temp(&condition);
    return true;
}

bool translate_cmovs(IR1_INST *pir1)
{
    IR2_OPND condition = ra_alloc_itemp(); /* SF */
#ifndef CONFIG_SOFTMMU
    if (!fp_translate_pattern_tail(pir1, condition)) {
        get_eflag_condition(&condition, pir1);
    }
#else
    get_eflag_condition(&condition, pir1);
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&src_opnd, opnd1, UNKNOWN_EXTENSION, false);

    IR2_OPND no_mov = ir2_opnd_new_label(); /* mov if SF = 1 */
    append_ir2_opnd2(LISA_BEQZ, &condition, &no_mov);
    store_ir2_to_ir1(&src_opnd, opnd0, false);
    append_ir2_opnd1(LISA_LABEL, &no_mov);

    ra_free_temp(&condition);
    return true;
}

bool translate_cmovns(IR1_INST *pir1)
{
    IR2_OPND condition = ra_alloc_itemp(); /* SF */
#ifndef CONFIG_SOFTMMU
    if (!fp_translate_pattern_tail(pir1, condition)) {
        get_eflag_condition(&condition, pir1);
    }
#else
    get_eflag_condition(&condition, pir1);
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&src_opnd, opnd1, UNKNOWN_EXTENSION, false);

    IR2_OPND no_mov = ir2_opnd_new_label(); /* mov if SF = 0 */
    append_ir2_opnd2(LISA_BNEZ, &condition, &no_mov);
    store_ir2_to_ir1(&src_opnd, opnd0, false);
    append_ir2_opnd1(LISA_LABEL, &no_mov);

    ra_free_temp(&condition);
    return true;
}

bool translate_cmovp(IR1_INST *pir1)
{
    IR2_OPND condition = ra_alloc_itemp(); /* PF */
#ifndef CONFIG_SOFTMMU
    if (!fp_translate_pattern_tail(pir1, condition)) {
        get_eflag_condition(&condition, pir1);
    }
#else
    get_eflag_condition(&condition, pir1);
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&src_opnd, opnd1, UNKNOWN_EXTENSION, false);

    IR2_OPND no_mov = ir2_opnd_new_label(); /* mov if PF = 1 */
    append_ir2_opnd2(LISA_BEQZ, &condition, &no_mov);
    store_ir2_to_ir1(&src_opnd, opnd0, false);
    append_ir2_opnd1(LISA_LABEL, &no_mov);

    ra_free_temp(&condition);
    return true;
}

bool translate_cmovnp(IR1_INST *pir1)
{
    IR2_OPND condition = ra_alloc_itemp(); /* PF */
#ifndef CONFIG_SOFTMMU
    if (!fp_translate_pattern_tail(pir1, condition)) {
        get_eflag_condition(&condition, pir1);
    }
#else
    get_eflag_condition(&condition, pir1);
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&src_opnd, opnd1, UNKNOWN_EXTENSION, false);

    IR2_OPND no_mov = ir2_opnd_new_label(); /* mov if PF = 0 */
    append_ir2_opnd2(LISA_BNEZ, &condition, &no_mov);
    store_ir2_to_ir1(&src_opnd, opnd0, false);
    append_ir2_opnd1(LISA_LABEL, &no_mov);

    ra_free_temp(&condition);
    return true;
}

bool translate_cmovl(IR1_INST *pir1)
{
    /* sf != of */
    IR2_OPND condition = ra_alloc_itemp(); /* cond = SF xor OF */
#ifndef CONFIG_SOFTMMU
    if (!fp_translate_pattern_tail(pir1, condition)) {
        get_eflag_condition(&condition, pir1);
    }
#else
    get_eflag_condition(&condition, pir1);
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&src_opnd, opnd1, UNKNOWN_EXTENSION, false);

    IR2_OPND no_mov = ir2_opnd_new_label(); /* mov if cond = 1 */
    append_ir2_opnd2(LISA_BEQZ, &condition, &no_mov);
    store_ir2_to_ir1(&src_opnd, opnd0, false);
    append_ir2_opnd1(LISA_LABEL, &no_mov);

    ra_free_temp(&condition);
    return true;
}

bool translate_cmovge(IR1_INST *pir1)
{
    /* sf == of */
    IR2_OPND condition = ra_alloc_itemp(); /* cond = SF xor OF */
#ifndef CONFIG_SOFTMMU
    if (!fp_translate_pattern_tail(pir1, condition)) {
        get_eflag_condition(&condition, pir1);
    }
#else
    get_eflag_condition(&condition, pir1);
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&src_opnd, opnd1, UNKNOWN_EXTENSION, false);

    IR2_OPND no_mov = ir2_opnd_new_label(); /* mov if cond = 0 */
    append_ir2_opnd2(LISA_BNEZ, &condition, &no_mov);
    store_ir2_to_ir1(&src_opnd, opnd0, false);
    append_ir2_opnd1(LISA_LABEL, &no_mov);

    ra_free_temp(&condition);
    return true;
}

bool translate_cmovle(IR1_INST *pir1)
{
    /* zf==1 || sf!=of */
    IR2_OPND condition = ra_alloc_itemp(); /* ZF, OF xor SF */
#ifndef CONFIG_SOFTMMU
    if (!fp_translate_pattern_tail(pir1, condition)) {
        get_eflag_condition(&condition, pir1);
    }
#else
    get_eflag_condition(&condition, pir1);
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&src_opnd, opnd1, UNKNOWN_EXTENSION, false);

    IR2_OPND no_mov = ir2_opnd_new_label(); /* mov if cond != 0 */
    append_ir2_opnd2(LISA_BEQZ, &condition, &no_mov);
    store_ir2_to_ir1(&src_opnd, opnd0, false);
    append_ir2_opnd1(LISA_LABEL, &no_mov);

    ra_free_temp(&condition);
    return true;
}

bool translate_cmovg(IR1_INST *pir1)
{
    /* zf==0 && sf==of */
    IR2_OPND condition = ra_alloc_itemp(); /* ZF, OF xor SF */
#ifndef CONFIG_SOFTMMU
    if (!fp_translate_pattern_tail(pir1, condition)) {
        get_eflag_condition(&condition, pir1);
    }
#else
    get_eflag_condition(&condition, pir1);
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&src_opnd, opnd1, UNKNOWN_EXTENSION, false);

    IR2_OPND no_mov = ir2_opnd_new_label(); /* mov if cond = 0 */
    append_ir2_opnd2(LISA_BNEZ, &condition, &no_mov);
    store_ir2_to_ir1(&src_opnd, opnd0, false);
    append_ir2_opnd1(LISA_LABEL, &no_mov);

    ra_free_temp(&condition);
    return true;
}

bool translate_lea(IR1_INST *pir1)
{
//#ifdef CONFIG_SOFTMMU
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//    if (option_by_hand && lsenv->tr_data->sys.ss32 &&
//        !lsenv->tr_data->sys.addseg && ir1_opnd_is_gpr(opnd0) &&
//        ir1_addr_size(pir1) == 4) {
//        if (!ir1_opnd_has_index(opnd1) && ir1_opnd_has_base(opnd1)) {
//            longx offset = ir1_opnd_simm(opnd1);
//            IR2_OPND dst_reg = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
//            IR2_OPND src_base_reg = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
//            if (int32_in_int16(offset)) {
//                append_ir2_opnd2i(mips_addi_addrx, &dst_reg, &src_base_reg, offset);
//            } else {
//                IR2_OPND offset_opnd = ra_alloc_itemp();
//                load_imm32_to_ir2(&offset_opnd, offset, SIGN_EXTENSION);
//                append_ir2_opnd3(mips_add_addrx, &dst_reg, &src_base_reg,
//                                 &offset_opnd);
//                ra_free_temp(&offset_opnd);
//            }
//            return true;
//        }
//    }
//#endif


    IR2_OPND value_opnd = ra_alloc_itemp();

    int addr_size = ir1_addr_size(pir1);
    load_ir1_mem_addr_to_ir2(&value_opnd, ir1_get_opnd(pir1, 1), addr_size);
    store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
    return true;
}

#ifndef CONFIG_SOFTMMU
bool translate_xchg(IR1_INST *pir1)
{
    lsassertm(0, "xchg to be implemented in LoongArch.\n");
//    /* if two src is the same reg, do nothing */
//    if (ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0) + 1))
//        if ((ir1_opnd_size(ir1_get_opnd(pir1, 0)) == ir1_opnd_size(ir1_get_opnd(pir1, 0) + 1)) &&
//            (ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) ==
//             ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0) + 1)) &&
//            (ir1_opnd_base_reg_bits_start(ir1_get_opnd(pir1, 0)) ==
//             ir1_opnd_base_reg_bits_start(ir1_get_opnd(pir1, 0) + 1)))
//            return true;
//
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
//    load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);
//
//    if (ir2_opnd_is_itemp(&src_opnd_1)) {
//        store_ir2_to_ir1(&src_opnd_0, ir1_get_opnd(pir1, 1), false);
//        store_ir2_to_ir1(&src_opnd_1, ir1_get_opnd(pir1, 0), false);
//    }
//    else if (ir2_opnd_is_itemp(&src_opnd_0)) {
//        store_ir2_to_ir1(&src_opnd_1, ir1_get_opnd(pir1, 0), false);
//        store_ir2_to_ir1(&src_opnd_0, ir1_get_opnd(pir1, 1), false);
//    }
//    else { /* none src is temp reg */
//        IR2_OPND t_opnd = ra_alloc_itemp();
//        append_ir2_opnd2(mips_mov64, &t_opnd, &src_opnd_1);
//
//        store_ir2_to_ir1(&src_opnd_0, ir1_get_opnd(pir1, 1), false);
//        store_ir2_to_ir1(&t_opnd, ir1_get_opnd(pir1, 0), false);
//
//        ra_free_temp(&t_opnd);
//    }

    return true;
}
#endif

#ifndef CONFIG_SOFTMMU
bool translate_cmpxchg(IR1_INST *pir1)
{
    lsassertm(0, "cmpxchg to be implemented in LoongArch.\n");
//    if (option_by_hand) return translate_cmpxchg_byhand(pir1);
//
//    IR1_OPND *reg_ir1 = NULL;
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
//        reg_ir1 = &eax_ir1_opnd;
//    else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
//        reg_ir1 = &al_ir1_opnd;
//    else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16)
//        reg_ir1 = &ax_ir1_opnd;
//
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//    IR2_OPND eax_opnd = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//    load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);
//    load_ir1_gpr_to_ir2(&eax_opnd, reg_ir1, SIGN_EXTENSION);
//
//    IR2_OPND t_dest_opnd = ra_alloc_itemp();
//    append_ir2_opnd3(mips_dsubu, &t_dest_opnd, &eax_opnd, &src_opnd_0);
//
//    generate_eflag_calculation(&t_dest_opnd, &eax_opnd, &src_opnd_0, pir1, true);
//
//    IR2_OPND label_unequal = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_bne, &src_opnd_0, &eax_opnd, &label_unequal);
//    /* equal */
//    store_ir2_to_ir1(&src_opnd_1, ir1_get_opnd(pir1, 0), false);
//    /* append_ir2_opnd2i(mips_ori, &eflags_opnd, &eflags_opnd, 0x40); */
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_b, &label_exit);
//
//    /* unequal */
//    append_ir2_opnd1(mips_label, &label_unequal);
//    store_ir2_to_ir1(&src_opnd_0, reg_ir1, false);
//    /* append_ir2_opnd2i(mips_andi, &eflags_opnd, &eflags_opnd, 0xffbf); */
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&t_dest_opnd);
    return true;
}
#endif

bool translate_movq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif

    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src  = ir1_get_opnd(pir1, 1);

    /* XMM */

    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_mem(src)) {
        IR2_OPND temp = load_freg_from_ir1_1(src, false, false);
        IR2_OPND xmm_dest = ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
        append_ir2_opnd2i(LISA_VCLRSTRI_V, &xmm_dest, &temp, 7);
        //append_ir2_opnd2i(mips_vclrstriv, &xmm_dest, &temp, 7);
        return true;
    }

    if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
        IR2_OPND xmm_src = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
        store_freg_to_ir1(&xmm_src, dest, false, false);
        return true;
    }

    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_xmm(src)) {
        IR2_OPND xmm_dest = ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
        IR2_OPND xmm_src  = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
        append_ir2_opnd2i(LISA_VCLRSTRI_V, &xmm_dest, &xmm_src, 7);
        //append_ir2_opnd2i(mips_vclrstriv, &xmm_dest, &xmm_src, 7);
        return true;
    }

    /* MMX */

    if (ir1_opnd_is_mmx(dest)) { /* dest mmx : src mmx/mem */
        IR2_OPND dest_opnd = load_freg_from_ir1_1(dest, false, false);
        load_freg_from_ir1_2(&dest_opnd, src, false, false);
        return true;
    }
    
    if (ir1_opnd_is_mmx(src)) { /* src mmx  : dest mmx/mem */
        IR2_OPND source_opnd = load_freg_from_ir1_1(src, false, false);
        store_freg_to_ir1(&source_opnd, dest, false, false);
        return true;
    }

    lsassert(0);

    return true;
}

bool translate_movd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif

    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src  = ir1_get_opnd(pir1, 1);

    /* XMM */

    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_mem(src)) {
        IR2_OPND temp = ra_alloc_itemp();
        load_ir1_mem_to_ir2(&temp, src, ZERO_EXTENSION, false, -1);
//        IR2_OPND temp = load_freg_from_ir1_1(src, false, false);
        IR2_OPND xmm_dest = ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
        append_ir2_opnd1i(LISA_VLDI, &xmm_dest, 0);
        append_ir2_opnd2(LISA_MOVGR2FR_D, &xmm_dest, &temp);
//        append_ir2_opnd2i(LISA_VCLRSTRI_V, &xmm_dest, &temp, 3);
        return true;
    }

    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_gpr(src)) {
        IR2_OPND gpr_src  = ra_alloc_itemp();
        load_ir1_gpr_to_ir2(&gpr_src, src, ZERO_EXTENSION);
        IR2_OPND xmm_dest = ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
        append_ir2_opnd1i(LISA_VLDI, &xmm_dest, 0);
        append_ir2_opnd2(LISA_MOVGR2FR_D, &xmm_dest, &gpr_src);
//        append_ir2_opnd2 (LISA_MOVGR2FR_W, &temp,     &gpr_src);
//        append_ir2_opnd2i(LISA_VCLRSTRI_V, &xmm_dest, &temp, 3);
        //append_ir2_opnd2(mips_mtc1, &gpr_src, &temp);
        //append_ir2_opnd2i(mips_vclrstriv, &xmm_dest, &temp, 3);
        return true;
    }

    if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
        IR2_OPND xmm_src = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
        store_freg_to_ir1(&xmm_src, dest, false, false);
        return true;
    }

    if (ir1_opnd_is_gpr(dest) && ir1_opnd_is_xmm(src)) {
        IR2_OPND gpr_dest = ra_alloc_gpr(ir1_opnd_base_reg_num(dest));
        IR2_OPND xmm_src  = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
        append_ir2_opnd2(LISA_MOVFR2GR_S, &gpr_dest, &xmm_src);
      //append_ir2_opnd2(mips_mfc1, &gpr_dest, &xmm_src);
        return true;
    }

    /* MMX */
    IR2_OPND src_tmp = ra_alloc_itemp();

    if (ir1_opnd_is_gpr(dest) || ir1_opnd_is_mem(dest)) {
        lsassert(ir1_opnd_is_mmx(src));
        load_ir1_to_ir2 (&src_tmp, src,  UNKNOWN_EXTENSION, false);
        store_ir2_to_ir1(&src_tmp, dest, false);
    }
    else if (ir1_opnd_is_gpr(src) || ir1_opnd_is_mem(src)) {
        lsassert(ir1_opnd_is_mmx(dest));
        load_ir1_to_ir2 (&src_tmp, src,  ZERO_EXTENSION, false);
        store_ir2_to_ir1(&src_tmp, dest, false);
    }

    lsassertm(0, "unknown type of SIMD movd.\n");

    return true;
}
