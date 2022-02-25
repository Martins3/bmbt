#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>

void latxs_sys_string_register_ir1(void)
{
    latxs_register_ir1(X86_INS_INSB);
    latxs_register_ir1(X86_INS_INSD);
    latxs_register_ir1(X86_INS_INSW);

    latxs_register_ir1(X86_INS_OUTSB);
    latxs_register_ir1(X86_INS_OUTSD);
    latxs_register_ir1(X86_INS_OUTSW);

    latxs_register_ir1(X86_INS_MOVSB);
    latxs_register_ir1(X86_INS_MOVSD);
    latxs_register_ir1(X86_INS_MOVSW);
    latxs_register_ir1(X86_INS_MOVSQ);

    latxs_register_ir1(X86_INS_SCASB);
    latxs_register_ir1(X86_INS_SCASD);
    latxs_register_ir1(X86_INS_SCASW);
    latxs_register_ir1(X86_INS_SCASQ);

    latxs_register_ir1(X86_INS_CMPSB);
    latxs_register_ir1(X86_INS_CMPSD);
    latxs_register_ir1(X86_INS_CMPSW);
    latxs_register_ir1(X86_INS_CMPSQ);

    latxs_register_ir1(X86_INS_LODSB);
    latxs_register_ir1(X86_INS_LODSD);
    latxs_register_ir1(X86_INS_LODSW);
    latxs_register_ir1(X86_INS_LODSQ);

    latxs_register_ir1(X86_INS_STOSB);
    latxs_register_ir1(X86_INS_STOSD);
    latxs_register_ir1(X86_INS_STOSW);
    latxs_register_ir1(X86_INS_STOSQ);
}

static void latxs_load_string_step_to_ir2(IR2_OPND *setp,
        IR1_INST *pir1, int opidx)
{
    IR2_OPND df_opnd = latxs_ra_alloc_itemp();
    /* env->df : if DF: env->df = 1 else env->df = -1 */
    latxs_append_ir2_opnd2i(LISA_LD_WU, &df_opnd, &latxs_env_ir2_opnd,
            lsenv_offset_of_df(lsenv));
    latxs_append_ir2_opnd2i(LISA_ANDI, &df_opnd,
            &df_opnd, 0x400);

    lsassert(!(opidx >> 1)); /* opidx == 0 || opidx == 1 */
    IR1_OPND *opnd = ir1_get_opnd(pir1, opidx);
    int opnd_size  = ir1_opnd_size(opnd);

    int bytes = opnd_size >> 3;
    int bits  = 0;
    switch (opnd_size) {
    case 8:
        bits = 9;
        break;
    case 16:
        bits = 8;
        break;
    case 32:
        bits = 7;
        break;
#ifdef TARGET_X86_64
    case 64:
        bits = 6;
        break;
#endif
    default:
        lsassert(0);
        break;
    }
    lsassert(bits != 0);

    IR2_OPND tmp_step = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_SRAI_W, &tmp_step, &df_opnd, bits);
    latxs_append_ir2_opnd2i(LISA_ADDI_W, setp, &tmp_step, 0 - bytes);

    latxs_ra_free_temp(&df_opnd);
    latxs_ra_free_temp(&tmp_step);
}

static void latxs_tr_gen_string_loop_start(IR1_INST *pir1,
        IR2_OPND *loop, IR2_OPND *exit,
        int load_step, IR2_OPND *step)
{
    /* 1. exit when initial count is zero */
    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    if (latxs_ir1_has_prefix_rep(pir1) || latxs_ir1_has_prefix_repne(pir1)) {
        IR2_OPND ecx_opnd = latxs_ra_alloc_gpr(ecx_index);
        int addr_size = latxs_ir1_addr_size(pir1);
#ifdef TARGET_X86_64
        if (addr_size == 8) {
            /* RCX */
            latxs_append_ir2_opnd3(LISA_BEQ,
                    &ecx_opnd, &latxs_zero_ir2_opnd, &label_exit);
        } else
#endif
        if (addr_size == 4) {
            /* ECX */
            latxs_append_ir2_opnd3(LISA_BEQ,
                    &ecx_opnd, &latxs_zero_ir2_opnd, &label_exit);
        } else {
            /* CX */
            IR2_OPND cond = latxs_ra_alloc_itemp();
            latxs_append_ir2_opnd2_(lisa_mov16z, &cond, &ecx_opnd);
            latxs_append_ir2_opnd3(LISA_BEQ,
                    &cond, &latxs_zero_ir2_opnd, &label_exit);
            latxs_ra_free_temp(&cond);
        }
    }
    *exit = label_exit;

    /* 2. preparations outside the loop */
    if (load_step) {
        IR2_OPND setp_opnd = latxs_ra_alloc_itemp();
        latxs_load_string_step_to_ir2(&setp_opnd, pir1, 0);
        *step = setp_opnd;
    }

    /* 3. loop starts */
    IR2_OPND label_loop = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd1(LISA_LABEL, &label_loop);
    *loop = label_loop;
}

static void latxs_tr_gen_string_loop_end(IR1_INST *pir1,
        IR2_OPND label_loop, IR2_OPND label_exit,
        IR2_OPND *result)
{
    /* 1. loop ends when .. */
    if (!result) {
        /* 1.1 ends when ecx == 0*/
        if (latxs_ir1_has_prefix_rep(pir1)) {
            IR2_OPND ecx_opnd = latxs_ra_alloc_gpr(ecx_index);
            int addr_size = latxs_ir1_addr_size(pir1);
#ifdef TARGET_X86_64
            if (addr_size == 8) {
                /* RCX */
                latxs_append_ir2_opnd2i(LISA_ADDI_D, &ecx_opnd, &ecx_opnd, -1);
                latxs_append_ir2_opnd3(LISA_BNE,
                        &ecx_opnd, &latxs_zero_ir2_opnd, &label_loop);
                if (option_by_hand) {
                    latxs_ir2_opnd_set_emb(&ecx_opnd, EXMode_N, 64);
                }
            } else
#endif
            if (addr_size == 4) {
                /* ECX */
                latxs_append_ir2_opnd2i(LISA_ADDI_W, &ecx_opnd, &ecx_opnd, -1);
#ifdef TARGET_X86_64
                latxs_append_ir2_opnd2_(lisa_mov32z, &ecx_opnd, &ecx_opnd);
#endif
                latxs_append_ir2_opnd3(LISA_BNE,
                        &ecx_opnd, &latxs_zero_ir2_opnd, &label_loop);
#ifndef TARGET_X86_64
                if (option_by_hand) {
                    latxs_ir2_opnd_set_emb(&ecx_opnd, EXMode_S, 32);
                }
#endif
            } else {
                /* CX */
                IR2_OPND cx_temp = latxs_ra_alloc_itemp();
                latxs_append_ir2_opnd2i(LISA_ADDI_W, &cx_temp, &ecx_opnd, -1);
                if (option_by_hand) {
                    latxs_store_ir2_to_ir1_gpr_em(&cx_temp, &cx_ir1_opnd);
                } else {
                    latxs_store_ir2_to_ir1_gpr(&cx_temp, &cx_ir1_opnd);
                }
                latxs_append_ir2_opnd2_(lisa_mov16z, &cx_temp, &cx_temp);
                latxs_append_ir2_opnd3(LISA_BNE,
                        &cx_temp, &latxs_zero_ir2_opnd, &label_loop);
                latxs_ra_free_temp(&cx_temp);
            }
        }
    } else if (latxs_ir1_has_prefix_repe(pir1) ||
               latxs_ir1_has_prefix_repne(pir1)) {
        /*
         * 1.2 loop ends when one of the following condition is true
         *     1> result != 0 (repe)
         *     2> result == 0 (repne)
         *     3>    ecx == 0 (always)
         */
        IR2_OPND condition  = latxs_ra_alloc_itemp(); /* <3>       */
        IR2_OPND condition2 = latxs_ra_alloc_itemp(); /* <1> & <2> */

        /*     3>    ecx == 0            */
        IR2_OPND ecx_opnd = latxs_ra_alloc_gpr(ecx_index);
        int addr_size = latxs_ir1_addr_size(pir1);
#ifdef TARGET_X86_64
        if (addr_size == 8) {
                /* RCX */
            latxs_append_ir2_opnd2i(LISA_ADDI_D, &ecx_opnd, &ecx_opnd, -1);
            latxs_append_ir2_opnd2i(LISA_SLTUI,  &condition, &ecx_opnd, 1);
                if (option_by_hand) {
                    lsassert(0);
                }
        } else
#endif
        if (addr_size == 4) {
            /* ECX */
            latxs_append_ir2_opnd2i(LISA_ADDI_W, &ecx_opnd, &ecx_opnd, -1);
#ifdef TARGET_X86_64
            latxs_append_ir2_opnd2_(lisa_mov32z, &ecx_opnd, &ecx_opnd);
#endif
            latxs_append_ir2_opnd2i(LISA_SLTUI,  &condition, &ecx_opnd, 1);
#ifndef TARGET_X86_64
            if (option_by_hand) {
                latxs_ir2_opnd_set_emb(&ecx_opnd, EXMode_S, 32);
            }
#endif
        } else {
            /* CX */
            IR2_OPND cx_temp = latxs_ra_alloc_itemp();
            latxs_append_ir2_opnd2_(lisa_mov16z, &cx_temp, &ecx_opnd);
            latxs_append_ir2_opnd2i(LISA_ADDI_D, &cx_temp, &cx_temp, -1);
            if (option_by_hand) {
                latxs_store_ir2_to_ir1_gpr_em(&cx_temp, &cx_ir1_opnd);
            } else {
                latxs_store_ir2_to_ir1_gpr(&cx_temp, &cx_ir1_opnd);
            }
            latxs_append_ir2_opnd2i(LISA_SLTUI,  &condition, &cx_temp, 1);
            latxs_ra_free_temp(&cx_temp);
        }

        /*     1> result != 0 (repe)     */
        /*     2> result == 0 (repne)    */
        if (latxs_ir1_has_prefix_repe(pir1)) {
            /* set 1 when 0 < result, i.e., result!=0 */
            latxs_append_ir2_opnd3(LISA_SLTU,
                    &condition2, &latxs_zero_ir2_opnd, result);
        }
        if (latxs_ir1_has_prefix_repne(pir1)) {
            /* set 1 when result < 1, i.e., result==0 */
            latxs_append_ir2_opnd2i(LISA_SLTUI,
                    &condition2, result, 1);
        }

        /* 4.3 when two conditions are not satisfied, the loop continues*/
        latxs_append_ir2_opnd3(LISA_OR, &condition, &condition, &condition2);
        latxs_append_ir2_opnd3(LISA_BEQ,
                &condition, &latxs_zero_ir2_opnd, &label_loop);

        latxs_ra_free_temp(&condition);
        latxs_ra_free_temp(&condition2);
    }

    /* 2. exit label */
    if (!result) {
        latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);
    }
}

bool latxs_translate_ins(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* destination : MEM(ES:(E)DI) */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* I/O port : DX */

    int data_size = ir1_opnd_size(opnd0);

    /* Always check IO before any IO operation */
    latxs_tr_gen_io_check(pir1, opnd1, data_size);

    /* 1. loop starts */
    IR2_OPND label_loop = latxs_ir2_opnd_new_inv();
    IR2_OPND label_exit = latxs_ir2_opnd_new_inv();
    latxs_tr_gen_string_loop_start(pir1, &label_loop, &label_exit, 0, NULL);

    /* tr_gen_io_start(); */

    int addr_size = latxs_ir1_addr_size(pir1);

    /* 2. I/O read from I/O port DX : helper_inb/w/l */
    /* 3. write to MEM(ES:(E)DI)    : softmmu helper */
    /* 4. adjust EDI                : according to opnd size */

    /*
     * Note: we must do this dummy write first to be restartable in
     * case of page fault.
     */
    latxs_store_ir2_to_ir1_mem(&latxs_zero_ir2_opnd, opnd0, addr_size);
    /* 2.0 save native context */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    /* 2.1 prepare parameter for IO read helper */
    /*    > arg0 : CPUX86State *env */
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                      &latxs_env_ir2_opnd);
    /*    > arg1 ; uint32_t    port */
    IR2_OPND port_reg = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
    latxs_append_ir2_opnd2_(lisa_mov16z, &latxs_arg1_ir2_opnd, &port_reg);
    /* 2.2 call the helper_inb/inw/inl */
    switch (data_size) {
    /* target/i386/misc_helper.c */
    case 8:
        latxs_tr_gen_call_to_helper((ADDR)helper_inb);
        break;
    case 16:
        latxs_tr_gen_call_to_helper((ADDR)helper_inw);
        break;
    case 32:
        latxs_tr_gen_call_to_helper((ADDR)helper_inl);
        break;
    default:
        lsassertm_illop(ir1_addr(pir1), 0,
                "Unsupported X86_INS_INS data size %d.\n", data_size);
        break;
    }
    /* 2.3 resotre the naive context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
    /* 2.4 get the read IO value at $v0 */
    IR2_OPND io_value_reg = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2_(lisa_mov, &io_value_reg, &latxs_ret0_ir2_opnd);

    /* 3. write to mem */
    latxs_store_ir2_to_ir1_mem(&io_value_reg, opnd0, addr_size);
    latxs_ra_free_temp(&io_value_reg);

    /* tr_gen_io_end(); */

    /* 4. adjust (E)DI */
    IR2_OPND setp = latxs_ra_alloc_itemp();
    latxs_load_string_step_to_ir2(&setp, pir1, 0);
    if (addr_size == 4) {
        /* adjust EDI */
        IR2_OPND edi = latxs_ra_alloc_gpr(edi_index);
        latxs_append_ir2_opnd3(LISA_SUB_W, &edi, &edi, &setp);
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&edi, EXMode_S, 32);
        }
    } else {
        /* adjust DI */
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        IR2_OPND edi = latxs_ra_alloc_gpr(edi_index);
        latxs_append_ir2_opnd3(LISA_SUB_W, &tmp, &edi, &setp);
        if (option_by_hand) {
            latxs_store_ir2_to_ir1_gpr_em(&tmp, &di_ir1_opnd);
        } else {
            latxs_store_ir2_to_ir1_gpr(&tmp, &di_ir1_opnd);
        }
        latxs_ra_free_temp(&tmp);
    }

    /* 5. breakpoint IO */
    latxs_tr_gen_io_bpt(pir1, opnd1, data_size >> 3);

    /* 6. loop ends */
    latxs_tr_gen_string_loop_end(pir1, label_loop, label_exit, NULL);

    latxs_ra_free_temp(&setp);

    return true;
}

bool latxs_translate_outs(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* I/O port : DX  */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* src data : MEM(ES:(E/R)SI) */

    int data_size = ir1_opnd_size(opnd1);

    /* Always check IO before any IO operation */
    latxs_tr_gen_io_check(pir1, opnd0, data_size);

    /* 1. loop starts */
    IR2_OPND label_loop = latxs_ir2_opnd_new_inv();
    IR2_OPND label_exit = latxs_ir2_opnd_new_inv();
    latxs_tr_gen_string_loop_start(pir1, &label_loop, &label_exit, 0, NULL);

    /* tr_gen_io_start(); */

    /* 2. read from MEM(ES:(E)DI)  : softmmu helper  */
    /* 3. I/O write to I/O port DX : helper_outb/w/l */
    /* 4. adjust EDI               : according to opnd size */

    /* 2. load value from mem */
    IR2_OPND io_value_reg = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&io_value_reg, opnd1, EXMode_N);

    /* 3.0 save native context */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    /* 3.1 prepare parameter for IO read helper */
    /*    > arg2 : uint32_t    data */
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg2_ir2_opnd,
                                      &io_value_reg);
    /*    > arg0 : CPUX86State *env */
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                      &latxs_env_ir2_opnd);
    /*    > arg1 ; uint32_t    port */
    IR2_OPND port_reg = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
    latxs_append_ir2_opnd2_(lisa_mov16z, &latxs_arg1_ir2_opnd, &port_reg);
    /* 3.2 call the helper_outb/inw/inl */
    switch (data_size) {
    /* target/i386/misc_helper.c */
    case 8:
        latxs_tr_gen_call_to_helper((ADDR)helper_outb);
        break;
    case 16:
        latxs_tr_gen_call_to_helper((ADDR)helper_outw);
        break;
    case 32:
        latxs_tr_gen_call_to_helper((ADDR)helper_outl);
        break;
    default:
        lsassertm_illop(ir1_addr(pir1), 0,
                "Unsupported X86_INS_OUTS data size %d.\n", data_size);
        break;
    }
    /* 3.3 resotre the naive context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* tr_gen_io_end(); */

    /* 4. adjust (E)SI */
    IR2_OPND setp = latxs_ra_alloc_itemp();
    latxs_load_string_step_to_ir2(&setp, pir1, 1);
    int addr_size = latxs_ir1_addr_size(pir1);
    if (addr_size == 4) {
        /* adjust ESI */
        IR2_OPND esi = latxs_ra_alloc_gpr(esi_index);
        latxs_append_ir2_opnd3(LISA_SUB_W, &esi, &esi, &setp);
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esi, EXMode_S, 32);
        }
    } else {
        /* adjust SI */
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        IR2_OPND esi = latxs_ra_alloc_gpr(esi_index);
        latxs_append_ir2_opnd3(LISA_SUB_W, &tmp, &esi, &setp);
        if (option_by_hand) {
            latxs_store_ir2_to_ir1_gpr_em(&tmp, &si_ir1_opnd);
        } else {
            latxs_store_ir2_to_ir1_gpr(&tmp, &si_ir1_opnd);
        }
        latxs_ra_free_temp(&tmp);
    }

    /* 5. breakpoint IO */
    latxs_tr_gen_io_bpt(pir1, opnd1, data_size >> 3);

    /* 6. loop ends */
    latxs_tr_gen_string_loop_end(pir1, label_loop, label_exit, NULL);

    latxs_ra_free_temp(&setp);

    return true;
}

bool latxs_translate_movs(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* dest: MEM(ES:(E/R)DI) */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* src : MEM(DS:(E/R)SI) */

    int addr_size = latxs_ir1_addr_size(pir1);

    /* 1. loop starts */
    IR2_OPND label_loop = latxs_ir2_opnd_new_inv();
    IR2_OPND label_exit = latxs_ir2_opnd_new_inv();
    IR2_OPND setp = latxs_ir2_opnd_new_inv();
    latxs_tr_gen_string_loop_start(pir1,
            &label_loop, &label_exit, 1, &setp);

    /* 2. load memory value at ESI, and store into memory at EDI */
    IR2_OPND src_value = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src_value, opnd1, EXMode_S);
    latxs_store_ir2_to_ir1_mem(&src_value, opnd0, addr_size);

    /* 3. adjust (E)SI and (E)DI */
#ifdef TARGET_X86_64
    if (addr_size == 8) {
        /* RSI and RDI */
        IR2_OPND esi = latxs_ra_alloc_gpr(esi_index);
        IR2_OPND edi = latxs_ra_alloc_gpr(edi_index);
        latxs_append_ir2_opnd3(LISA_SUB_D, &esi, &esi, &setp);
        latxs_append_ir2_opnd3(LISA_SUB_D, &edi, &edi, &setp);
    } else
#endif
    if (addr_size == 4) {
        /* ESI and EDI */
        IR2_OPND esi = latxs_ra_alloc_gpr(esi_index);
        IR2_OPND edi = latxs_ra_alloc_gpr(edi_index);
        latxs_append_ir2_opnd3(LISA_SUB_W, &esi, &esi, &setp);
        latxs_append_ir2_opnd3(LISA_SUB_W, &edi, &edi, &setp);
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esi, EXMode_S, 32);
            latxs_ir2_opnd_set_emb(&edi, EXMode_S, 32);
        }
    } else {
        /* SI and DI */
        IR2_OPND si = latxs_ra_alloc_itemp();
        IR2_OPND di = latxs_ra_alloc_itemp();
        latxs_load_ir1_to_ir2(&si, &si_ir1_opnd, EXMode_Z);
        latxs_load_ir1_to_ir2(&di, &di_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd3(LISA_SUB_W, &si, &si, &setp);
        latxs_append_ir2_opnd3(LISA_SUB_W, &di, &di, &setp);
        if (option_by_hand) {
            latxs_store_ir2_to_ir1_gpr_em(&si, &si_ir1_opnd);
            latxs_store_ir2_to_ir1_gpr_em(&di, &di_ir1_opnd);
        } else {
            latxs_store_ir2_to_ir1_gpr(&si, &si_ir1_opnd);
            latxs_store_ir2_to_ir1_gpr(&di, &di_ir1_opnd);
        }
        latxs_ra_free_temp(&si);
        latxs_ra_free_temp(&di);
    }

    /* 4. loop ends */
    latxs_tr_gen_string_loop_end(pir1, label_loop, label_exit, NULL);

    latxs_ra_free_temp(&setp);

    return true;
}

bool latxs_translate_scas(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* src : AL, AX, EAX     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* src : MEM(ES:(E/R)DI) */

    int addr_size = latxs_ir1_addr_size(pir1);

    /* 0. preparations outside the loop */
    IR2_OPND eax = latxs_ra_alloc_itemp();
    latxs_load_ir1_gpr_to_ir2(&eax, opnd0, EXMode_S);

    /* 1. loop starts */
    IR2_OPND label_loop = latxs_ir2_opnd_new_inv();
    IR2_OPND label_exit = latxs_ir2_opnd_new_inv();
    IR2_OPND setp  = latxs_ir2_opnd_new_inv();
    latxs_tr_gen_string_loop_start(pir1,
            &label_loop, &label_exit, 1, &setp);

    /* 2.1 load memory value at MEM(ES:(E)DI) */
    IR2_OPND _edi_ = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&_edi_,
            opnd1, EXMode_S, addr_size);

    /* 2.2 adjust EDI */
#ifdef TARGET_X86_64
    if (addr_size == 8) {
        /* RDI */
        IR2_OPND edi = latxs_ra_alloc_gpr(edi_index);
        latxs_append_ir2_opnd3(LISA_SUB_D, &edi, &edi, &setp);
    } else
#endif
    if (addr_size == 4) {
        /* EDI */
        IR2_OPND edi = latxs_ra_alloc_gpr(edi_index);
        latxs_append_ir2_opnd3(LISA_SUB_W, &edi, &edi, &setp);
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&edi, EXMode_S, 32);
        }
    } else {
        /* DI */
        IR2_OPND di = latxs_ra_alloc_itemp();
        latxs_load_ir1_gpr_to_ir2(&di, &di_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd3(LISA_SUB_W, &di, &di, &setp);
        if (option_by_hand) {
            latxs_store_ir2_to_ir1_gpr_em(&di, &di_ir1_opnd);
        } else {
            latxs_store_ir2_to_ir1_gpr(&di, &di_ir1_opnd);
        }
        latxs_ra_free_temp(&di);
    }

    /* 3. compare */
    IR2_OPND cmp_result = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SUB_W, &cmp_result, &eax, &_edi_);

    /* 4. loop ends */
    latxs_tr_gen_string_loop_end(pir1, label_loop, label_exit, &cmp_result);

    /* 5. calculate eflags */
    latxs_generate_eflag_calculation(&cmp_result, &eax, &_edi_, pir1, true);
    latxs_ra_free_temp(&cmp_result);

    /* 6. loop exits */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);

    latxs_ra_free_temp(&setp);

    return true;
}

bool latxs_translate_cmps(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* src : MEM(ES:(E/R)SI) */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* src : MEM(ES:(E/R)DI) */

    int addr_size = latxs_ir1_addr_size(pir1);

    /* 1. loop starts */
    IR2_OPND label_loop = latxs_ir2_opnd_new_inv();
    IR2_OPND label_exit = latxs_ir2_opnd_new_inv();
    IR2_OPND setp  = latxs_ir2_opnd_new_inv();
    latxs_tr_gen_string_loop_start(pir1,
            &label_loop, &label_exit, 1, &setp);

    /* 2.1 load memory value at ESI and EDI */
    IR2_OPND _esi_ = latxs_ra_alloc_itemp();
    IR2_OPND _edi_ = latxs_ra_alloc_itemp();

    latxs_load_ir1_mem_to_ir2(&_esi_, opnd0, EXMode_S, addr_size);
    latxs_load_ir1_mem_to_ir2(&_edi_, opnd1, EXMode_S, addr_size);

    /* 2.2 adjust ESI and EDI */
#ifdef TARGET_X86_64
    if (addr_size == 8) {
        /* RSI and RDI */
        IR2_OPND esi = latxs_ra_alloc_gpr(esi_index);
        IR2_OPND edi = latxs_ra_alloc_gpr(edi_index);
        latxs_append_ir2_opnd3(LISA_SUB_D, &esi, &esi, &setp);
        latxs_append_ir2_opnd3(LISA_SUB_D, &edi, &edi, &setp);
    } else
#endif
    if (addr_size == 4) {
        /* ESI and EDI */
        IR2_OPND esi = latxs_ra_alloc_gpr(esi_index);
        IR2_OPND edi = latxs_ra_alloc_gpr(edi_index);
        latxs_append_ir2_opnd3(LISA_SUB_W, &esi, &esi, &setp);
        latxs_append_ir2_opnd3(LISA_SUB_W, &edi, &edi, &setp);
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esi, EXMode_S, 32);
            latxs_ir2_opnd_set_emb(&edi, EXMode_S, 32);
        }
    } else {
        /* SI and DI */
        IR2_OPND si = latxs_ra_alloc_itemp();
        IR2_OPND di = latxs_ra_alloc_itemp();
        latxs_load_ir1_gpr_to_ir2(&si, &si_ir1_opnd, EXMode_Z);
        latxs_load_ir1_gpr_to_ir2(&di, &di_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd3(LISA_SUB_W, &si, &si, &setp);
        latxs_append_ir2_opnd3(LISA_SUB_W, &di, &di, &setp);
        if (option_by_hand) {
            latxs_store_ir2_to_ir1_gpr_em(&si, &si_ir1_opnd);
            latxs_store_ir2_to_ir1_gpr_em(&di, &di_ir1_opnd);
        } else {
            latxs_store_ir2_to_ir1_gpr(&si, &si_ir1_opnd);
            latxs_store_ir2_to_ir1_gpr(&di, &di_ir1_opnd);
        }
        latxs_ra_free_temp(&si);
        latxs_ra_free_temp(&di);
    }

    /* 3. compare */
    IR2_OPND cmp_result = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SUB_W, &cmp_result, &_esi_, &_edi_);

    /* 4. loop ends */
    latxs_tr_gen_string_loop_end(pir1, label_loop, label_exit, &cmp_result);

    /* 5. calculate eflags */
    latxs_generate_eflag_calculation(&cmp_result,
            &_esi_, &_edi_, pir1, true);
    latxs_ra_free_temp(&cmp_result);

    /* 6. loop exits */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);

    latxs_ra_free_temp(&setp);

    return true;
}

bool latxs_translate_lods(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* dest : AL, AX, EAX    */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* src  : MEM(ES:(E)SI)  */

    int addr_size = latxs_ir1_addr_size(pir1);

    /* 1. loop starts */
    IR2_OPND label_loop = latxs_ir2_opnd_new_inv();
    IR2_OPND label_exit = latxs_ir2_opnd_new_inv();
    IR2_OPND step = latxs_ir2_opnd_new_inv();
    latxs_tr_gen_string_loop_start(pir1,
            &label_loop, &label_exit, 1, &step);

    /* 2. load memory value at MEM(ES:(E)SI) */
    EXTENSION_MODE em = EXMode_S;
    if (ir1_opnd_size(opnd0) < 32) {
        em = EXMode_Z;
    }

    IR2_OPND _esi_ = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&_esi_, opnd1, em, addr_size);

    /* 3 adjust (E)SI */
#ifdef TARGET_X86_64
    if (addr_size == 8) {
        /* RSI*/
        IR2_OPND esi = latxs_ra_alloc_gpr(esi_index);
        latxs_append_ir2_opnd3(LISA_SUB_D, &esi, &esi, &step);
    } else
#endif
    if (addr_size == 4) {
        /* ESI */
        IR2_OPND esi = latxs_ra_alloc_gpr(esi_index);
        latxs_append_ir2_opnd3(LISA_SUB_W, &esi, &esi, &step);
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esi, EXMode_S, 32);
        }
    } else {
        /* SI */
        IR2_OPND si = latxs_ra_alloc_itemp();
        latxs_load_ir1_gpr_to_ir2(&si, &si_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd3(LISA_SUB_W, &si, &si, &step);
        if (option_by_hand) {
            latxs_store_ir2_to_ir1_gpr_em(&si, &si_ir1_opnd);
        } else {
            latxs_store_ir2_to_ir1_gpr(&si, &si_ir1_opnd);
        }
        latxs_ra_free_temp(&si);
    }

    /* if xcx = 0, just end */

    /* 5. store the load value to dest */
    latxs_store_ir2_to_ir1_gpr(&_esi_, opnd0);

    /* 4. loop ends */
    latxs_tr_gen_string_loop_end(pir1, label_loop, label_exit, NULL);

    latxs_ra_free_temp(&step);

    return true;
}

bool latxs_translate_stos(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* dest: MEM(ES:(E/R)DI) */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* src : AL, AX, EAX     */

    int addr_size = latxs_ir1_addr_size(pir1);

    /* 0. preparations outside the loop */
    IR2_OPND eax_value_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_gpr_to_ir2(&eax_value_opnd, opnd1, EXMode_S);

    /* 1. loop starts */
    IR2_OPND label_loop = latxs_ir2_opnd_new_inv();
    IR2_OPND label_exit = latxs_ir2_opnd_new_inv();
    IR2_OPND step = latxs_ir2_opnd_new_inv();
    latxs_tr_gen_string_loop_start(pir1,
            &label_loop, &label_exit, 1, &step);

    /* 2. store EAX into memory at EDI */
    latxs_store_ir2_to_ir1_mem(&eax_value_opnd, opnd0, addr_size);

    /* 3. adjust (E)DI */
#ifdef TARGET_X86_64
    if (addr_size == 8) {
        /* RDI*/
        IR2_OPND edi = latxs_ra_alloc_gpr(edi_index);
        latxs_append_ir2_opnd3(LISA_SUB_D, &edi, &edi, &step);
    } else
#endif
    if (addr_size == 4) {
        /* EDI */
        IR2_OPND edi = latxs_ra_alloc_gpr(edi_index);
        latxs_append_ir2_opnd3(LISA_SUB_W, &edi, &edi, &step);
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&edi, EXMode_S, 32);
        }
    } else {
        /* DI */
        IR2_OPND di = latxs_ra_alloc_itemp();
        latxs_load_ir1_gpr_to_ir2(&di, &di_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd3(LISA_SUB_D, &di, &di, &step);
        if (option_by_hand) {
            latxs_store_ir2_to_ir1_gpr_em(&di, &di_ir1_opnd);
        } else {
            latxs_store_ir2_to_ir1_gpr(&di, &di_ir1_opnd);
        }
        latxs_ra_free_temp(&di);
    }

    /* 4. loop ends */
    latxs_tr_gen_string_loop_end(pir1, label_loop, label_exit, NULL);

    latxs_ra_free_temp(&step);

    return true;
}
