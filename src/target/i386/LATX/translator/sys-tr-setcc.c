#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "flag-lbt.h"
#include "sys-excp.h"
#include <string.h>

void latxs_sys_setcc_register_ir1(void)
{
    latxs_register_ir1(X86_INS_SETE);
    latxs_register_ir1(X86_INS_SETNE);
    latxs_register_ir1(X86_INS_SETO);
    latxs_register_ir1(X86_INS_SETNO);
    latxs_register_ir1(X86_INS_SETB);
    latxs_register_ir1(X86_INS_SETAE);
    latxs_register_ir1(X86_INS_SETBE);
    latxs_register_ir1(X86_INS_SETA);
    latxs_register_ir1(X86_INS_SETS);
    latxs_register_ir1(X86_INS_SETNS);
    latxs_register_ir1(X86_INS_SETP);
    latxs_register_ir1(X86_INS_SETNP);
    latxs_register_ir1(X86_INS_SETL);
    latxs_register_ir1(X86_INS_SETGE);
    latxs_register_ir1(X86_INS_SETLE);
    latxs_register_ir1(X86_INS_SETG);

    latxs_register_ir1(X86_INS_BSF);
    latxs_register_ir1(X86_INS_BSR);
    latxs_register_ir1(X86_INS_BT);
    latxs_register_ir1(X86_INS_BTC);
    latxs_register_ir1(X86_INS_BTR);
    latxs_register_ir1(X86_INS_BTS);
}

bool latxs_translate_setz(IR1_INST *pir1)
{
    IR2_OPND value_opnd = latxs_ra_alloc_itemp();
    IR2_OPND zf_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&zf_opnd, pir1);
    latxs_append_ir2_opnd2i(LISA_SRLI_W, &value_opnd,
                                         &zf_opnd, ZF_BIT_INDEX);
    latxs_ra_free_temp(&zf_opnd);

    latxs_store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0));
    latxs_ra_free_temp(&value_opnd);

    return true;
}

bool latxs_translate_setnz(IR1_INST *pir1)
{
    IR2_OPND value_opnd = latxs_ra_alloc_itemp();
    IR2_OPND zf_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&zf_opnd, pir1);
    latxs_append_ir2_opnd2i(LISA_SLTUI, &value_opnd,
                                        &zf_opnd, 1);
    latxs_ra_free_temp(&zf_opnd);

    latxs_store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0));
    latxs_ra_free_temp(&value_opnd);

    return true;
}

bool latxs_translate_seto(IR1_INST *pir1)
{
    IR2_OPND value_opnd = latxs_ra_alloc_itemp();
    IR2_OPND of_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&of_opnd, pir1);
    latxs_append_ir2_opnd2i(LISA_SRLI_W, &value_opnd,
                                         &of_opnd, OF_BIT_INDEX);
    latxs_ra_free_temp(&of_opnd);

    latxs_store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0));
    latxs_ra_free_temp(&value_opnd);

    return true;
}

bool latxs_translate_setno(IR1_INST *pir1)
{
    IR2_OPND value_opnd = latxs_ra_alloc_itemp();
    IR2_OPND of_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&of_opnd, pir1);
    latxs_append_ir2_opnd2i(LISA_SLTUI, &value_opnd,
                                        &of_opnd, 1);
    latxs_ra_free_temp(&of_opnd);

    latxs_store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0));
    latxs_ra_free_temp(&value_opnd);

    return true;
}

bool latxs_translate_setb(IR1_INST *pir1)
{
    IR2_OPND value_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&value_opnd, pir1);

    latxs_store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0));
    latxs_ra_free_temp(&value_opnd);
    return true;
}

bool latxs_translate_setae(IR1_INST *pir1)
{
    IR2_OPND value_opnd = latxs_ra_alloc_itemp();
    IR2_OPND cf_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&cf_opnd, pir1);
    latxs_append_ir2_opnd2i(LISA_SLTUI, &value_opnd,
                                        &cf_opnd, 1);
    latxs_ra_free_temp(&cf_opnd);

    latxs_store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0));
    latxs_ra_free_temp(&value_opnd);

    return true;
}

bool latxs_translate_setbe(IR1_INST *pir1)
{
    IR2_OPND value_opnd = latxs_ra_alloc_itemp();
    IR2_OPND cfzf_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&cfzf_opnd, pir1); /* cf==1 || zf==1 */
    latxs_append_ir2_opnd3(LISA_SLTU, &value_opnd,
                                      &latxs_zero_ir2_opnd, &cfzf_opnd);
    latxs_ra_free_temp(&cfzf_opnd);

    latxs_store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0));
    latxs_ra_free_temp(&value_opnd);

    return true;
}

bool latxs_translate_seta(IR1_INST *pir1)
{
    IR2_OPND value_opnd = latxs_ra_alloc_itemp();
    IR2_OPND cfzf_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&cfzf_opnd, pir1); /* cf==0 && zf==0 */
    latxs_append_ir2_opnd2i(LISA_SLTUI, &value_opnd,
                                        &cfzf_opnd, 1);
    latxs_ra_free_temp(&cfzf_opnd);

    latxs_store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0));
    latxs_ra_free_temp(&value_opnd);

    return true;
}

bool latxs_translate_sets(IR1_INST *pir1)
{
    IR2_OPND value_opnd = latxs_ra_alloc_itemp();
    IR2_OPND sf_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&sf_opnd, pir1);
    latxs_append_ir2_opnd2i(LISA_SRLI_W, &value_opnd,
                                         &sf_opnd, SF_BIT_INDEX);
    latxs_ra_free_temp(&sf_opnd);

    latxs_store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0));
    latxs_ra_free_temp(&value_opnd);

    return true;
}

bool latxs_translate_setns(IR1_INST *pir1)
{
    IR2_OPND value_opnd = latxs_ra_alloc_itemp();
    IR2_OPND sf_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&sf_opnd, pir1);
    latxs_append_ir2_opnd2i(LISA_SLTUI, &value_opnd,
                                        &sf_opnd, SF_BIT_INDEX);
    latxs_ra_free_temp(&sf_opnd);

    latxs_store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0));
    latxs_ra_free_temp(&value_opnd);

    return true;
}

bool latxs_translate_setp(IR1_INST *pir1)
{
    IR2_OPND value_opnd = latxs_ra_alloc_itemp();
    IR2_OPND pf_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&pf_opnd, pir1);
    latxs_append_ir2_opnd2i(LISA_SRLI_W, &value_opnd,
                                         &pf_opnd, PF_BIT_INDEX);
    latxs_ra_free_temp(&pf_opnd);

    latxs_store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0));
    latxs_ra_free_temp(&value_opnd);

    return true;
}

bool latxs_translate_setnp(IR1_INST *pir1)
{
    IR2_OPND value_opnd = latxs_ra_alloc_itemp();
    IR2_OPND pf_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&pf_opnd, pir1);
    latxs_append_ir2_opnd2i(LISA_SLTUI, &value_opnd,
                                        &pf_opnd, 1);
    latxs_ra_free_temp(&pf_opnd);

    latxs_store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0));
    latxs_ra_free_temp(&value_opnd);

    return true;
}

bool latxs_translate_setl(IR1_INST *pir1)
{
    IR2_OPND value_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&value_opnd, pir1); /* sf != of */

    latxs_store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0));
    latxs_ra_free_temp(&value_opnd);

    return true;
}

bool latxs_translate_setge(IR1_INST *pir1)
{
    IR2_OPND value_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&value_opnd, pir1); /* sf == of */
    latxs_append_ir2_opnd2i(LISA_SLTUI, &value_opnd,
                                        &value_opnd, 1);

    latxs_store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0));
    latxs_ra_free_temp(&value_opnd);

    return true;
}

bool latxs_translate_setle(IR1_INST *pir1)
{
    IR2_OPND value_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&value_opnd, pir1); /* zf==1 || sf!=of */
    latxs_append_ir2_opnd3(LISA_SLTU, &value_opnd,
                                      &latxs_zero_ir2_opnd, &value_opnd);

    latxs_store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0));
    latxs_ra_free_temp(&value_opnd);

    return true;
}

bool latxs_translate_setg(IR1_INST *pir1)
{
    IR2_OPND value_opnd = latxs_ra_alloc_itemp();

    latxs_get_eflag_condition(&value_opnd, pir1); /* zf==0 && sf==of */
    latxs_append_ir2_opnd2i(LISA_SLTUI, &value_opnd,
                                        &value_opnd, 1);

    latxs_store_ir2_to_ir1(&value_opnd, ir1_get_opnd(pir1, 0));
    latxs_ra_free_temp(&value_opnd);

    return true;
}

bool latxs_translate_bsf(IR1_INST *pir1)
{
    if (ir1_opcode(pir1) == X86_INS_TZCNT) {
        return latxs_translate_tzcnt(pir1);
    }

    IR2_OPND count = latxs_ra_alloc_itemp();
    IR2_OPND t_opnd = latxs_ra_alloc_itemp();
    IR2_OPND src_opnd = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src_opnd,
            ir1_get_opnd(pir1, 1), EXMode_Z);

    if (ir1_is_zf_def(pir1)) {
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ORI, &temp, &latxs_zero_ir2_opnd, 0xfff);
        latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &temp, 0x8);
        latxs_ra_free_temp(&temp);
    }

    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &src_opnd,
                                     &latxs_zero_ir2_opnd, &label_exit);

    latxs_append_ir2_opnd2i(LISA_ORI, &count, &latxs_zero_ir2_opnd, 0);

    /* ------------------ LABEL 1 ---------------------- */
    IR2_OPND label_1 = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd1(LISA_LABEL, &label_1);

    latxs_append_ir2_opnd3(LISA_SRL_D, &t_opnd, &src_opnd, &count);
    latxs_append_ir2_opnd2i(LISA_ANDI, &t_opnd, &t_opnd, 1);
    IR2_OPND label_2 = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BNE, &t_opnd,
                                     &latxs_zero_ir2_opnd, &label_2);
    /* ===> LABEL 2 */

    latxs_append_ir2_opnd2i(LISA_ADDI_D, &count, &count, 1);

    latxs_append_ir2_opnd1(LISA_B, &label_1);
    /* ===> LABEL 1 */

    /* ------------------ LABEL 2 ---------------------- */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_2);

    latxs_store_ir2_to_ir1(&count, ir1_get_opnd(pir1, 0));
    if (ir1_is_zf_def(pir1)) {
        latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &latxs_zero_ir2_opnd, 0x8);
    }

    /* ------------------ LABEL exit ------------------- */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);

    latxs_ra_free_temp(&count);
    latxs_ra_free_temp(&t_opnd);
    return true;
}

bool latxs_translate_bsr(IR1_INST *pir1)
{
    IR2_OPND count = latxs_ra_alloc_itemp();
    IR2_OPND t_opnd = latxs_ra_alloc_itemp();
    IR2_OPND src_opnd = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src_opnd,
            ir1_get_opnd(pir1, 1), EXMode_Z);

    if (ir1_is_zf_def(pir1)) {
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ORI, &temp, &latxs_zero_ir2_opnd, 0xfff);
        latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &temp, 0x8);
        latxs_ra_free_temp(&temp);
    }

    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &src_opnd,
                                     &latxs_zero_ir2_opnd, &label_exit);

    latxs_append_ir2_opnd2i(LISA_ORI, &count, &latxs_zero_ir2_opnd,
                      ir1_opnd_size(ir1_get_opnd(pir1, 1)) - 1);

    /* ------------------ LABEL 1 ---------------------- */
    IR2_OPND label_1 = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd1(LISA_LABEL, &label_1);

    latxs_append_ir2_opnd3(LISA_SRL_D, &t_opnd, &src_opnd, &count);
    latxs_append_ir2_opnd2i(LISA_ANDI, &t_opnd, &t_opnd, 1);
    IR2_OPND label_2 = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BNE, &t_opnd,
                                     &latxs_zero_ir2_opnd, &label_2);
    /* ===> LABEL 2 */

    latxs_append_ir2_opnd2i(LISA_ADDI_D, &count, &count, -1);
    latxs_append_ir2_opnd1(LISA_B, &label_1);
    /* ===> LABEL 1 */

    /* ------------------ LABEL 2 ---------------------- */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_2);

    latxs_store_ir2_to_ir1(&count, ir1_get_opnd(pir1, 0));
    if (ir1_is_zf_def(pir1)) {
        latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &latxs_zero_ir2_opnd, 0x8);
    }

    /* ------------------ LABEL exit ------------------- */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);

    latxs_ra_free_temp(&count);
    latxs_ra_free_temp(&t_opnd);
    return true;
}

bool latxs_translate_btx(IR1_INST *pir1)
{
    if (latxs_ir1_has_prefix_lock(pir1) &&
        (lsenv->tr_data->sys.cflags & CF_PARALLEL) &&
        ir1_opcode(pir1) != X86_INS_BT) {
        return latxs_translate_lock_btx(pir1);
    }
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* bit base   : GPR/MEM */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* bit offset : GPR/imm */

    int t_imm = 0;
    switch (ir1_opnd_size(opnd0)) {
    case 16:
        t_imm = 0xf;
        break;
    case 32:
        t_imm = 0x1f;
        break;
    case 64:
        t_imm = 0x3f;
        break;
    default:
        lsassertm(0, "Invalid operand size (%d) in translate_btx.\n",
                ir1_opnd_size(opnd0));
        break;
    }

    IR2_OPND bit_base = latxs_ra_alloc_itemp();
    IR2_OPND bit_off = latxs_ra_alloc_itemp();
    IR2_OPND mem_opnd;
    IR2_OPND mem_no_offset;

    /* get bit_base and bit_off */
    if (ir1_opnd_is_gpr(opnd0)) {
        latxs_load_ir1_to_ir2(&bit_base, opnd0, EXMode_Z);
        latxs_load_ir1_to_ir2(&bit_off,  opnd1, EXMode_Z);
        latxs_append_ir2_opnd2i(LISA_ANDI, &bit_off, &bit_off, t_imm);
    } else {
        /* opnd0 is memory operand */
        if (ir1_opnd_is_imm(opnd1)) {
            latxs_load_ir1_to_ir2(&bit_off, opnd1, EXMode_Z);
            latxs_append_ir2_opnd2i(LISA_ANDI, &bit_off, &bit_off, t_imm);
        } else {
            latxs_load_ir1_to_ir2(&bit_off, opnd1, EXMode_S);
        }

        latxs_convert_mem_opnd(&mem_opnd, opnd0, -1);
        int mem_no_offset_new_tmp = 0;
        mem_no_offset = latxs_convert_mem_ir2_opnd_no_offset(&mem_opnd,
                &mem_no_offset_new_tmp);
        IR2_OPND mem = latxs_ir2_opnd_mem_get_base(&mem_no_offset);
        if (mem_no_offset_new_tmp) {
            latxs_ra_free_temp(&mem_opnd);
        }

        IR2_OPND byte_nr = latxs_ra_alloc_itemp();

        /*
         * bit offset
         * 31                            7       0
         * xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
         * \                           / \       /
         *  \  byte offset for memory /  bit offset
         *   -------------------------   inside Byte
         */
        latxs_append_ir2_opnd2i(LISA_SRAI_D, &byte_nr, &bit_off, 3);
        latxs_append_ir2_opnd2i(LISA_ANDI, &bit_off, &bit_off, 0x7);

        IR2_OPND off_not_zero = latxs_ir2_opnd_new_label();
        latxs_append_ir2_opnd2(LISA_BNEZ, &bit_off, &off_not_zero);
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &bit_off, &bit_off, 0x8);
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &byte_nr, &byte_nr, -1);
        latxs_append_ir2_opnd1(LISA_LABEL, &off_not_zero);

        latxs_append_ir2_opnd3(LISA_ADD_D, &mem, &mem, &byte_nr);
        latxs_ra_free_temp(&byte_nr);

#ifdef TARGET_X86_64
        if (!lsenv->tr_data->sys.code64) {
            /* byte_nr is sx, make sure mem is 32zx */
            latxs_append_ir2_opnd2_(lisa_mov32z, &mem, &mem);
        }
#else
        /* byte_nr is sx, make sure mem is 32zx */
        latxs_append_ir2_opnd2_(lisa_mov32z, &mem, &mem);
#endif

        gen_ldst_softmmu_helper(LISA_LD_HU, &bit_base, &mem_no_offset, 1);
    }

    IR2_OPND bit_eflags = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SRL_D, &bit_eflags, &bit_base, &bit_off);

    /* update BitBase */
    IR2_OPND bit_set = latxs_ra_alloc_itemp();
    switch (ir1_opcode(pir1)) {
    case X86_INS_BT:
        break;
    case X86_INS_BTS: /* set bit = 1 */
        latxs_append_ir2_opnd2i(LISA_ORI, &bit_set, &latxs_zero_ir2_opnd, 1);
        latxs_append_ir2_opnd3(LISA_SLL_D, &bit_set, &bit_set, &bit_off);
        latxs_append_ir2_opnd3(LISA_OR, &bit_base, &bit_base, &bit_set);
        break;
    case X86_INS_BTR: /* set bit = 0 */
        latxs_append_ir2_opnd2i(LISA_ORI, &bit_set, &latxs_zero_ir2_opnd, 1);
        latxs_append_ir2_opnd3(LISA_SLL_D, &bit_set, &bit_set, &bit_off);
        latxs_append_ir2_opnd2_(lisa_not, &bit_set, &bit_set);
        latxs_append_ir2_opnd3(LISA_AND, &bit_base, &bit_base, &bit_set);
        break;
    case X86_INS_BTC: /* set bit = not bit  */
        latxs_append_ir2_opnd2i(LISA_ORI, &bit_set, &latxs_zero_ir2_opnd, 1);
        latxs_append_ir2_opnd3(LISA_SLL_D, &bit_set, &bit_set, &bit_off);
        latxs_append_ir2_opnd3(LISA_XOR, &bit_base, &bit_base, &bit_set);
        break;
    default:
        lsassertm(0, "Invalid opcode in translate_btx\n");
        break;
    }
    latxs_ra_free_temp(&bit_set);
    latxs_ra_free_temp(&bit_off);

    if (ir1_opcode(pir1) != X86_INS_BT) {
        if (ir1_opnd_is_gpr(opnd0)) {
            latxs_store_ir2_to_ir1(&bit_base, opnd0);
        } else {
            gen_ldst_softmmu_helper(LISA_ST_H, &bit_base, &mem_no_offset, 1);
        }
    }
    latxs_ra_free_temp(&bit_base);

    /* update EFALGS */
    latxs_append_ir2_opnd2i(LISA_ANDI, &bit_eflags, &bit_eflags, 1);
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &bit_eflags, 0x1);
    latxs_ra_free_temp(&bit_eflags);

    return true;
}
