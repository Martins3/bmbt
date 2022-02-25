#include "../include/common.h"
#include "../include/reg-alloc.h"
#include "../include/env.h"
#include "../x86tomips-options.h"

#ifndef CONFIG_SOFTMMU
/* popf in user-mode */
bool translate_popf(IR1_INST *pir1)
{
    lsassertm(0, "popf to be implemented in LoongArch.\n");
//    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
//
//    if (cpu_get_guest_base() != 0) {
//        IR2_OPND tmp = ra_alloc_itemp();
//        IR2_OPND gbase = ra_alloc_guest_base();
//#ifdef N64
//        if (!ir2_opnd_is_address(&esp_opnd)) {
//            append_ir2_opnd2(mips_mov_addrx, &esp_opnd, &esp_opnd);
//        }
//        append_ir2_opnd3(mips_add_addr, &tmp, &esp_opnd, &gbase);
//#else
//        append_ir2_opnd3(mips_addu, &tmp, &esp_opnd, &gbase);
//#endif
//        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
//        append_ir2_opnd2i(mips_lw, &eflags_ir2_opnd, &tmp,
//                -lsenv->tr_data->curr_esp_need_decrease);
//        ra_free_temp(&tmp);
//        ra_free_temp(&gbase);
//    } else {
//        append_ir2_opnd2i(mips_lw, &eflags_ir2_opnd, &esp_opnd,
//                -lsenv->tr_data->curr_esp_need_decrease);
//    }
//
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &eflags_ir2_opnd, 0x3f);
//        append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd,  0x400);
//    }
//
//    append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x202);
//
//    // IR1_OPCODE next_opcode = ((IR1_INST *)(pir1 + 1))->_opcode;
//    IR1_OPCODE next_opcode = ir1_opcode((IR1_INST *)(pir1 + 1));
//
//    if (next_opcode != X86_INS_PUSH && next_opcode != X86_INS_POP &&
//        next_opcode != X86_INS_PUSHF && next_opcode != X86_INS_POPF) {
//        if (4 - lsenv->tr_data->curr_esp_need_decrease != 0)
//            append_ir2_opnd2i(mips_addi_addrx, &esp_opnd, &esp_opnd,
//                              4 - lsenv->tr_data->curr_esp_need_decrease);
//        lsenv->tr_data->curr_esp_need_decrease = 0;
//    } else
//        lsenv->tr_data->curr_esp_need_decrease -= 4;
//
    return true;
}
#endif

#ifndef CONFIG_SOFTMMU
/* pushf in user-mode */
bool translate_pushf(IR1_INST *pir1)
{
    lsassertm(0, "pushf to be implemented in LoongArch.\n");
//    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
//
//    if (option_lbt) {
//        IR2_OPND temp   = ra_alloc_itemp();
//        append_ir2_opnd1i(mips_mfflag, &temp, 0x3f);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &temp);
//        ra_free_temp(&temp);
//    }
//
//    if (cpu_get_guest_base() != 0) {
//        IR2_OPND tmp = ra_alloc_itemp();
//        IR2_OPND gbase = ra_alloc_guest_base();
//#ifdef N64
//        if (!ir2_opnd_is_address(&esp_opnd)) {
//            append_ir2_opnd2(mips_mov_addrx, &esp_opnd, &esp_opnd);
//        }
//        append_ir2_opnd3(mips_add_addr, &tmp, &esp_opnd, &gbase);
//#else
//        append_ir2_opnd3(mips_addu, &tmp, &esp_opnd, &gbase);
//#endif
//        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
//        append_ir2_opnd2i(mips_sw, &eflags_ir2_opnd, &tmp,
//                -4 - lsenv->tr_data->curr_esp_need_decrease);
//        ra_free_temp(&tmp);
//        ra_free_temp(&gbase);
//    } else {
//        append_ir2_opnd2i(mips_sw, &eflags_ir2_opnd, &esp_opnd,
//                -4 - lsenv->tr_data->curr_esp_need_decrease);
//    }
//
//    IR1_OPCODE next_opcode = ir1_opcode((IR1_INST *)(pir1 + 1));
//    if (next_opcode != X86_INS_PUSH && next_opcode != X86_INS_POP &&
//        next_opcode != X86_INS_PUSHF && next_opcode != X86_INS_POPF) {
//        if (-4 - lsenv->tr_data->curr_esp_need_decrease != 0)
//            append_ir2_opnd2i(mips_addi_addrx, &esp_opnd, &esp_opnd,
//                              -4 - lsenv->tr_data->curr_esp_need_decrease);
//        lsenv->tr_data->curr_esp_need_decrease = 0;
//    } else
//        lsenv->tr_data->curr_esp_need_decrease += 4;
//
    return true;
}
#endif

bool translate_clc(IR1_INST *pir1)
{
    if (option_lbt) {
        append_ir2_opnd1i(LISA_X86MTFLAG, &zero_ir2_opnd, 0x1);
    } else {
        IR2_OPND mask = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ORI,   &mask,    &zero_ir2_opnd,  CF_BIT);
        append_ir2_opnd2_(lisa_not,   &mask,    &mask);
        append_ir2_opnd3 (LISA_AND,   &eflags_ir2_opnd,  &eflags_ir2_opnd, &mask);
    }

    return true;
}

bool translate_cld(IR1_INST *pir1)
{
    IR2_OPND mask = ra_alloc_itemp();

    append_ir2_opnd2i(LISA_ORI,  &mask,    &zero_ir2_opnd,  0x400);
    append_ir2_opnd2_(lisa_not,  &mask,    &mask);
    append_ir2_opnd3 (LISA_AND,  &eflags_ir2_opnd,  &eflags_ir2_opnd, &mask);

#ifdef CONFIG_SOFTMMU
    IR2_OPND tmp = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ORI,  &tmp, &zero_ir2_opnd, 1);
    append_ir2_opnd2i(LISA_ST_W, &tmp, &env_ir2_opnd,
                      lsenv_offset_of_df(lsenv));
#endif

    return true;
}

bool translate_stc(IR1_INST *pir1)
{
    if (option_lbt) {
        IR2_OPND cf_opnd = ra_alloc_itemp();

        append_ir2_opnd2i(LISA_ORI, &cf_opnd, &zero_ir2_opnd, 0x1);
        append_ir2_opnd1i(LISA_X86MTFLAG, &cf_opnd, 0x1);
        ra_free_temp(&cf_opnd);
    } else {
        append_ir2_opnd2i(LISA_ORI, &eflags_ir2_opnd, &eflags_ir2_opnd, 1);
    }

    return true;
}

bool translate_std(IR1_INST *pir1)
{
    append_ir2_opnd2i(LISA_ORI, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x400);

#ifdef CONFIG_SOFTMMU
    IR2_OPND tmp = ra_alloc_itemp();
    load_imm64_to_ir2(&tmp, -1);
    append_ir2_opnd2i(LISA_ST_W, &tmp, &env_ir2_opnd,
                      lsenv_offset_of_df(lsenv));
#endif

    return true;
}
