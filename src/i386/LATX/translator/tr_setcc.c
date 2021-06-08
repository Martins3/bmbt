#include "common.h"
#include "reg_alloc.h"
#include "env.h"
#include "x86tomips-options.h"
#include "flag_lbt.h"

bool translate_setz(IR1_INST *pir1)
{
    IR2_OPND value_opnd = ra_alloc_itemp();
    IR2_OPND zf_opnd = ra_alloc_itemp();

    get_eflag_condition(&zf_opnd, pir1);
    append_ir2_opnd2i(LISA_SRLI_W, &value_opnd,
                                   &zf_opnd, ZF_BIT_INDEX);
    ra_free_temp(&zf_opnd);

    store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
    ra_free_temp(&value_opnd);

    return true;
}

bool translate_setnz(IR1_INST *pir1)
{
    IR2_OPND value_opnd = ra_alloc_itemp();
    IR2_OPND zf_opnd = ra_alloc_itemp();

    get_eflag_condition(&zf_opnd, pir1);
    append_ir2_opnd2i(LISA_SLTUI, &value_opnd, &zf_opnd, 1);
    ra_free_temp(&zf_opnd);

    store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
    ra_free_temp(&value_opnd);

    return true;
}

bool translate_seto(IR1_INST *pir1)
{
    IR2_OPND value_opnd = ra_alloc_itemp();
    IR2_OPND of_opnd = ra_alloc_itemp();

    get_eflag_condition(&of_opnd, pir1);
    append_ir2_opnd2i(LISA_SRLI_W, &value_opnd,
                                   &of_opnd, OF_BIT_INDEX);
    ra_free_temp(&of_opnd);

    store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
    ra_free_temp(&value_opnd);

    return true;
}

bool translate_setno(IR1_INST *pir1)
{
    IR2_OPND value_opnd = ra_alloc_itemp();
    IR2_OPND of_opnd = ra_alloc_itemp();

    get_eflag_condition(&of_opnd, pir1);
    append_ir2_opnd2i(LISA_SLTUI, &value_opnd, &of_opnd, 1);
    ra_free_temp(&of_opnd);

    store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
    ra_free_temp(&value_opnd);

    return true;
}

bool translate_setb(IR1_INST *pir1)
{
    IR2_OPND value_opnd = ra_alloc_itemp();

    get_eflag_condition(&value_opnd, pir1);

    store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
    ra_free_temp(&value_opnd);
    return true;
}

bool translate_setae(IR1_INST *pir1)
{
    IR2_OPND value_opnd = ra_alloc_itemp();
    IR2_OPND cf_opnd = ra_alloc_itemp();

    get_eflag_condition(&cf_opnd, pir1);
    append_ir2_opnd2i(LISA_SLTUI, &value_opnd, &cf_opnd, 1);
    ra_free_temp(&cf_opnd);

    store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
    ra_free_temp(&value_opnd);

    return true;
}

bool translate_setbe(IR1_INST *pir1)
{
    IR2_OPND value_opnd = ra_alloc_itemp();
    IR2_OPND cfzf_opnd = ra_alloc_itemp();

    get_eflag_condition(&cfzf_opnd, pir1); /* cf==1 || zf==1 */
    append_ir2_opnd3(LISA_SLTU, &value_opnd,
                                &zero_ir2_opnd, &cfzf_opnd);
    ra_free_temp(&cfzf_opnd);

    store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
    ra_free_temp(&value_opnd);

    return true;
}

bool translate_seta(IR1_INST *pir1)
{
    IR2_OPND value_opnd = ra_alloc_itemp();
    IR2_OPND cfzf_opnd = ra_alloc_itemp();

    get_eflag_condition(&cfzf_opnd, pir1); /* cf==0 && zf==0 */
    append_ir2_opnd2i(LISA_SLTUI, &value_opnd, &cfzf_opnd, 1);
    ra_free_temp(&cfzf_opnd);

    store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
    ra_free_temp(&value_opnd);

    return true;
}

bool translate_sets(IR1_INST *pir1)
{
    IR2_OPND value_opnd = ra_alloc_itemp();
    IR2_OPND sf_opnd = ra_alloc_itemp();

    get_eflag_condition(&sf_opnd, pir1);
    append_ir2_opnd2i(LISA_SRLI_W, &value_opnd,
                                   &sf_opnd, SF_BIT_INDEX);
    ra_free_temp(&sf_opnd);

    store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
    ra_free_temp(&value_opnd);

    return true;
}

bool translate_setns(IR1_INST *pir1)
{
    IR2_OPND value_opnd = ra_alloc_itemp();
    IR2_OPND sf_opnd = ra_alloc_itemp();

    get_eflag_condition(&sf_opnd, pir1);
    append_ir2_opnd2i(LISA_SLTUI, &value_opnd,
                                  &sf_opnd, SF_BIT_INDEX);
    ra_free_temp(&sf_opnd);

    store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
    ra_free_temp(&value_opnd);

    return true;
}

bool translate_setp(IR1_INST *pir1)
{
    IR2_OPND value_opnd = ra_alloc_itemp();
    IR2_OPND pf_opnd = ra_alloc_itemp();

    get_eflag_condition(&pf_opnd, pir1);
    append_ir2_opnd2i(LISA_SRLI_W, &value_opnd,
                                   &pf_opnd, PF_BIT_INDEX);
    ra_free_temp(&pf_opnd);

    store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
    ra_free_temp(&value_opnd);

    return true;
}

bool translate_setnp(IR1_INST *pir1)
{
    IR2_OPND value_opnd = ra_alloc_itemp();
    IR2_OPND pf_opnd = ra_alloc_itemp();

    get_eflag_condition(&pf_opnd, pir1);
    append_ir2_opnd2i(LISA_SLTUI, &value_opnd, &pf_opnd, 1);
    ra_free_temp(&pf_opnd);

    store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
    ra_free_temp(&value_opnd);

    return true;
}

bool translate_setl(IR1_INST *pir1)
{
    IR2_OPND value_opnd = ra_alloc_itemp();

    get_eflag_condition(&value_opnd, pir1); /* sf != of */

    store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
    ra_free_temp(&value_opnd);

    return true;
}

bool translate_setge(IR1_INST *pir1)
{
    IR2_OPND value_opnd = ra_alloc_itemp();

    get_eflag_condition(&value_opnd, pir1); /* sf == of */
    append_ir2_opnd2i(LISA_SLTUI, &value_opnd, &value_opnd, 1);

    store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
    ra_free_temp(&value_opnd);

    return true;
}

bool translate_setle(IR1_INST *pir1)
{
    IR2_OPND value_opnd = ra_alloc_itemp();

    get_eflag_condition(&value_opnd, pir1); /* zf==1 || sf!=of */
    append_ir2_opnd3(LISA_SLTU, &value_opnd,
                                &zero_ir2_opnd, &value_opnd);

    store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
    ra_free_temp(&value_opnd);

    return true;
}

bool translate_setg(IR1_INST *pir1)
{
    IR2_OPND value_opnd = ra_alloc_itemp();

    get_eflag_condition(&value_opnd, pir1); /* zf==0 && sf==of */
    append_ir2_opnd2i(LISA_SLTUI, &value_opnd, &value_opnd, 1);

    store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0), false);
    ra_free_temp(&value_opnd);

    return true;
}

bool translate_bsf(IR1_INST *pir1)
{
    IR2_OPND count = ra_alloc_itemp();
    IR2_OPND t_opnd = ra_alloc_itemp();
    IR2_OPND src_opnd = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd, ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);

    if (ir1_is_zf_def(pir1)) {
        if (option_lbt) {
            IR2_OPND temp = ra_alloc_itemp();
            append_ir2_opnd2i(LISA_ORI, &temp, &zero_ir2_opnd, 0xfff);
            append_ir2_opnd1i(LISA_X86MTFLAG,  &temp, 0x8);
            ra_free_temp(&temp);
        } else {
            append_ir2_opnd2i(LISA_ORI, &eflags_ir2_opnd,
                                        &eflags_ir2_opnd, ZF_BIT);
        }
    }

    IR2_OPND label_exit = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BEQ, &src_opnd, &zero_ir2_opnd, &label_exit);

    append_ir2_opnd2i(LISA_ORI, &count, &zero_ir2_opnd, 0);

    /* ------------------ LABEL 1 ---------------------- */
    IR2_OPND label_1 = ir2_opnd_new_label();
    append_ir2_opnd1(LISA_LABEL, &label_1);

    append_ir2_opnd3(LISA_SRL_D, &t_opnd, &src_opnd, &count);
    append_ir2_opnd2i(LISA_ANDI, &t_opnd, &t_opnd, 1);
    IR2_OPND label_2 = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BNE, &t_opnd,
                               &zero_ir2_opnd, &label_2); /* ===> LABEL 2 */

    append_ir2_opnd2i(LISA_ADDI_D, &count, &count, 1);

    append_ir2_opnd1(LISA_B, &label_1); /* ===> LABEL 1 */

    /* ------------------ LABEL 2 ---------------------- */
    append_ir2_opnd1(LISA_LABEL, &label_2);

    store_ir2_to_ir1(&count, ir1_get_opnd(pir1, 0), false); 
    if (ir1_is_zf_def(pir1)) {
        if (option_lbt) {
            append_ir2_opnd1i(LISA_X86MTFLAG, &zero_ir2_opnd, 0x8);
        } else {
            IR2_OPND mask = ra_alloc_itemp();
            load_imm32_to_ir2(&mask, ZF_BIT, ZERO_EXTENSION);
            append_ir2_opnd2_(lisa_not, &mask, &mask);
            append_ir2_opnd3 (LISA_AND, &eflags_ir2_opnd,
                                        &eflags_ir2_opnd, &mask);
            ra_free_temp(&mask);
        }
    }
    
    /* ------------------ LABEL exit ------------------- */
    append_ir2_opnd1(LISA_LABEL, &label_exit);

    ra_free_temp(&count);
    ra_free_temp(&t_opnd);
    return true;
}

bool translate_bsr(IR1_INST *pir1)
{
    IR2_OPND count = ra_alloc_itemp();
    IR2_OPND t_opnd = ra_alloc_itemp();
    IR2_OPND src_opnd = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd, ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);

    if (ir1_is_zf_def(pir1)) {
        if (option_lbt) {
            IR2_OPND temp = ra_alloc_itemp();
            append_ir2_opnd2i(LISA_ORI, &temp, &zero_ir2_opnd, 0xfff);
            append_ir2_opnd1i(LISA_X86MTFLAG, &temp, 0x8);
            ra_free_temp(&temp);
        } else {
            append_ir2_opnd2i(LISA_ORI, &eflags_ir2_opnd,
                                        &eflags_ir2_opnd, ZF_BIT);
        }
    }

    IR2_OPND label_exit = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BEQ, &src_opnd, &zero_ir2_opnd, &label_exit);

    append_ir2_opnd2i(LISA_ORI, &count, &zero_ir2_opnd,
                      ir1_opnd_size(ir1_get_opnd(pir1, 1)) - 1);

    /* ------------------ LABEL 1 ---------------------- */
    IR2_OPND label_1 = ir2_opnd_new_label();
    append_ir2_opnd1(LISA_LABEL, &label_1);

    append_ir2_opnd3(LISA_SRL_D, &t_opnd, &src_opnd, &count);
    append_ir2_opnd2i(LISA_ANDI, &t_opnd, &t_opnd, 1);
    IR2_OPND label_2 = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BNE, &t_opnd,
                               &zero_ir2_opnd, &label_2); /* ===> LABEL 2 */

    append_ir2_opnd2i(LISA_ADDI_D, &count, &count, -1);
    append_ir2_opnd1(LISA_B, &label_1); /* ===> LABEL 1 */

    /* ------------------ LABEL 2 ---------------------- */
    append_ir2_opnd1(LISA_LABEL, &label_2);

    store_ir2_to_ir1(&count, ir1_get_opnd(pir1, 0), false); 
    if (ir1_is_zf_def(pir1)) {
        if (option_lbt) {
            append_ir2_opnd1i(LISA_X86MTFLAG, &zero_ir2_opnd, 0x8);
        } else {
            IR2_OPND mask = ra_alloc_itemp();
            load_imm32_to_ir2(&mask, ZF_BIT, ZERO_EXTENSION);
            append_ir2_opnd2_(lisa_not, &mask, &mask);
            append_ir2_opnd3 (LISA_AND, &eflags_ir2_opnd,
                                        &eflags_ir2_opnd, &mask);
            ra_free_temp(&mask);
        }
    }

    /* ------------------ LABEL exit ------------------- */
    append_ir2_opnd1(LISA_LABEL, &label_exit);

    ra_free_temp(&count);
    ra_free_temp(&t_opnd);
    return true;
}

bool translate_btx(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1 ,0); /* bit base   : GPR/MEM */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1 ,1); /* bit offset : GPR/imm */

    int t_imm = 0;
    switch (ir1_opnd_size(opnd0)) {
    case 16: t_imm = 0xf;  break;
    case 32: t_imm = 0x1f; break;
    case 64: t_imm = 0x3f; break;
    default: lsassertm(0, "Invalid operand size (%d) in translate_btx.\n",
                           ir1_opnd_size(opnd0));
        break;
    }

    IR2_OPND bit_base = ra_alloc_itemp();
    IR2_OPND bit_off = ra_alloc_itemp();
    IR2_OPND mem_opnd;
#ifdef CONFIG_SOFTMMU
    IR2_OPND mem_no_offset;
#endif

    /* get bit_base and bit_off */
    if (ir1_opnd_is_gpr(opnd0)) {
        load_ir1_to_ir2(&bit_base, opnd0, ZERO_EXTENSION, false);
        load_ir1_to_ir2(&bit_off,  opnd1, ZERO_EXTENSION, false);
        append_ir2_opnd2i(LISA_ANDI, &bit_off, &bit_off, t_imm);
    } else {
        /* opnd0 is memory operand */
        load_ir1_to_ir2(&bit_off, opnd1, ZERO_EXTENSION, false);
        if (ir1_opnd_is_imm(opnd1)) {
            append_ir2_opnd2i(LISA_ANDI, &bit_off, &bit_off, t_imm);
        }

        convert_mem_opnd(&mem_opnd, opnd0, -1);
#ifdef CONFIG_SOFTMMU
        int mem_no_offset_new_tmp = 0;
        mem_no_offset = convert_mem_ir2_opnd_no_offset(&mem_opnd,
                &mem_no_offset_new_tmp);
        IR2_OPND mem = ir2_opnd_mem_get_base(&mem_no_offset);
        if (mem_no_offset_new_tmp) ra_free_temp(&mem_opnd); 
#else
        IR2_OPND mem = ir2_opnd_mem_get_base(&mem_opnd);
        if (!ir2_opnd_is_reg_temp(&mem)) {
            IR2_OPND mem_new = ra_alloc_itemp();
            ir2_opnd_mem_set_base(&mem_opnd, &mem_new);
            mem = mem_new;
        }
#endif

        IR2_OPND byte_nr = ra_alloc_itemp();

        /* bit offset
         * 31                            7       0
         * xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx 
         * \                           / \       /
         *  \  byte offset for memory /  bit offset
         *   -------------------------   inside Byte
         * */
        append_ir2_opnd2i(LISA_SRLI_D, &byte_nr, &bit_off, 3);
        append_ir2_opnd2i(LISA_ANDI,   &bit_off, &bit_off, 0x7);

        IR2_OPND off_not_zero = ir2_opnd_new_label();
        append_ir2_opnd2(LISA_BNEZ, &bit_off, &off_not_zero);
        append_ir2_opnd2i(LISA_ADDI_D, &bit_off, &bit_off, 0x8);
        append_ir2_opnd2i(LISA_ADDI_D, &byte_nr, &byte_nr, -1);
        append_ir2_opnd1(LISA_LABEL, &off_not_zero);

        append_ir2_opnd3(LISA_ADD_D, &mem, &mem, &byte_nr);
        ra_free_temp(&byte_nr);

#ifndef CONFIG_SOFTMMU
        append_ir2_opnd2(LISA_LD_HU, &bit_base, &mem_opnd);
#else
        gen_ldst_softmmu_helper(LISA_LD_HU, &bit_base, &mem_no_offset, 1);
#endif
    }

    IR2_OPND bit_eflags = ra_alloc_itemp();
    append_ir2_opnd3(LISA_SRL_D, &bit_eflags, &bit_base, &bit_off);

    /* update BitBase */
    IR2_OPND bit_set = ra_alloc_itemp();
    switch (ir1_opcode(pir1)) {
    case X86_INS_BT:
        break;
    case X86_INS_BTS: /* set bit = 1 */
        append_ir2_opnd2i(LISA_ORI,  &bit_set,  &zero_ir2_opnd, 1);
        append_ir2_opnd3(LISA_SLL_D, &bit_set,  &bit_set, &bit_off);
        append_ir2_opnd3(LISA_OR,    &bit_base, &bit_base, &bit_set);
        break;
    case X86_INS_BTR: /* set bit = 0 */
        append_ir2_opnd2i(LISA_ORI,   &bit_set,  &zero_ir2_opnd, 1);
        append_ir2_opnd3 (LISA_SLL_D, &bit_set,  &bit_set, &bit_off);
        append_ir2_opnd2_(lisa_not,   &bit_set,  &bit_set);
        append_ir2_opnd3 (LISA_AND,   &bit_base, &bit_base, &bit_set);
        break;
    case X86_INS_BTC: /* set bit = not bit  */
        append_ir2_opnd2i(LISA_ORI,   &bit_set,  &zero_ir2_opnd, 1);
        append_ir2_opnd3 (LISA_SLL_D, &bit_set,  &bit_set,  &bit_off);
        append_ir2_opnd3 (LISA_XOR,   &bit_base, &bit_base, &bit_set);
        break;
    default:
        lsassertm(0, "Invalid opcode in translate_btx\n");
        break;
    }
    ra_free_temp(&bit_set);
    ra_free_temp(&bit_off);

    if (ir1_opcode(pir1) != X86_INS_BT) {
        if (ir1_opnd_is_gpr(opnd0)) {
            store_ir2_to_ir1(&bit_base, opnd0, false);
        } else {
#ifndef CONFIG_SOFTMMU
            append_ir2_opnd2(LISA_ST_H, &bit_base, &mem_opnd);
#else
            gen_ldst_softmmu_helper(LISA_ST_H, &bit_base, &mem_no_offset, 1);
#endif
        }
    }
    ra_free_temp(&bit_base);

    /* update EFALGS */
    append_ir2_opnd2i(LISA_ANDI, &bit_eflags, &bit_eflags, 1);
    if (option_lbt) {
        append_ir2_opnd1i(LISA_X86MTFLAG, &bit_eflags, 0x1);
    } else {
        IR2_OPND mask = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ORI, &mask, &zero_ir2_opnd, CF_BIT);
        append_ir2_opnd2_(lisa_not, &mask, &mask);
        append_ir2_opnd3 (LISA_AND, &eflags_ir2_opnd, &eflags_ir2_opnd, &mask);
        append_ir2_opnd3 (LISA_OR,  &eflags_ir2_opnd, &eflags_ir2_opnd, &bit_eflags);
        ra_free_temp(&mask);
    }
    ra_free_temp(&bit_eflags);

    return true;
}
