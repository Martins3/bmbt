/* X86toMIPS headers */
#include "common.h"
#include "reg_alloc.h"
#include "env.h"
#include "ir2/ir2.h"
#include "x86tomips-options.h"

/* QEMU headers */
#include "qemu-def.h"
#include "qemu/option.h"
#include "qemu/option_int.h"

void load_string_step_to_ir2(
        IR2_OPND *step_opnd,
        IR1_INST *pir1,
        int      opidx)
{
    IR2_OPND df_opnd = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ANDI, &df_opnd, &eflags_ir2_opnd, 0x400);

    lsassert(!(opidx >> 1)); /* opidx == 0 || opidx == 1 */
    IR1_OPND *opnd = ir1_get_opnd(pir1, opidx);
    int opnd_size  = ir1_opnd_size(opnd);

    int bytes = opnd_size >> 3;
    int bits  = 0;
    switch (opnd_size) {
    case 8:  bits = 9;    break;
    case 16: bits = 8;    break;
    case 32: bits = 7;    break;
    default: lsassert(0); break;
    }
    lsassert(bits != 0);

    IR2_OPND tmp_step = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_SRAI_W, &tmp_step, &df_opnd, bits);
    append_ir2_opnd2i(LISA_ADDI_W, step_opnd, &tmp_step, 0 - bytes);

    ra_free_temp(&df_opnd);
    ra_free_temp(&tmp_step);
}

static void tr_gen_string_loop_start(
        IR1_INST *pir1,
        IR2_OPND *loop,
        IR2_OPND *exit,
        int load_step, IR2_OPND *step)
{
    /* 1. exit when initial count is zero */
    IR2_OPND label_exit = ir2_opnd_new_label();
    if (ir1_has_prefix_rep(pir1) || ir1_has_prefix_repne(pir1)) {
        IR2_OPND ecx_opnd = ra_alloc_gpr(ecx_index);
        int addr_size = ir1_addr_size(pir1);
        if (addr_size == 4) { /* ECX */
            append_ir2_opnd3(LISA_BEQ, &ecx_opnd, &zero_ir2_opnd, &label_exit);
        }
        else { /* CX */
            IR2_OPND cond = ra_alloc_itemp();
            append_ir2_opnd2_(lisa_mov16z, &cond, &ecx_opnd);
//            append_ir2_opnd2i(mips_andi, &cond, &ecx_opnd, 0xffff);
            append_ir2_opnd3(LISA_BEQ, &cond, &zero_ir2_opnd, &label_exit);
            ra_free_temp(&cond);
        }
    }
    *exit = label_exit;

    /* 2. preparations outside the loop */
    if (load_step) {
        IR2_OPND step_opnd = ra_alloc_itemp();
        load_string_step_to_ir2(&step_opnd, pir1, 0);
        *step = step_opnd;
    }

    /* 3. loop starts */
    IR2_OPND label_loop = ir2_opnd_new_label();
    append_ir2_opnd1(LISA_LABEL, &label_loop);
    *loop = label_loop;
}

static void tr_gen_string_loop_end(
        IR1_INST *pir1,
        IR2_OPND label_loop,
        IR2_OPND label_exit,
        IR2_OPND *result)
{
    /* 1. loop ends */
    if (!result) {
        /* 1.1 ends when ecx == 0*/
        if (ir1_has_prefix_rep(pir1)) {
            IR2_OPND ecx_opnd = ra_alloc_gpr(ecx_index);
            int addr_size = ir1_addr_size(pir1);
            if (addr_size == 4) { /* ECX */
                append_ir2_opnd2i(LISA_ADDI_D, &ecx_opnd, &ecx_opnd, -1);
                append_ir2_opnd3(LISA_BNE, &ecx_opnd, &zero_ir2_opnd, &label_loop);
            }
            else { /* CX */
                IR2_OPND cx_temp = ra_alloc_itemp();
                append_ir2_opnd2i(LISA_ADDI_D, &cx_temp, &ecx_opnd, -1);
                store_ir2_to_ir1_gpr(&cx_temp, &cx_ir1_opnd);
                append_ir2_opnd2_(lisa_mov16z, &cx_temp, &cx_temp);
//                append_ir2_opnd2i(mips_andi, &cx_temp, &cx_temp, 0xffff);
                append_ir2_opnd3(LISA_BNE, &cx_temp, &zero_ir2_opnd, &label_loop);
                ra_free_temp(&cx_temp);
            }

        }
    } else if (ir1_has_prefix_repe(pir1)  ||
               ir1_has_prefix_repne(pir1) ) {
        /* 1.2 loop ends when one of the following condition is true
         *     1> result != 0 (repe)
         *     2> result == 0 (repne)
         *     3>    ecx == 0 (always)   */
        IR2_OPND condition  = ra_alloc_itemp(); /* <3>       */
        IR2_OPND condition2 = ra_alloc_itemp(); /* <1> & <2> */
         
        /*     3>    ecx == 0            */
        IR2_OPND ecx_opnd = ra_alloc_gpr(ecx_index);
        int addr_size = ir1_addr_size(pir1);
        if (addr_size == 4) { /* ECX */
            append_ir2_opnd2i(LISA_ADDI_D, &ecx_opnd, &ecx_opnd, -1);
            append_ir2_opnd2i(LISA_SLTUI,  &condition, &ecx_opnd, 1);
        }
        else { /* CX */
            IR2_OPND cx_temp = ra_alloc_itemp();
            append_ir2_opnd2_(lisa_mov16z, &cx_temp, &ecx_opnd);
//            append_ir2_opnd2i(mips_andi, &cx_temp, &ecx_opnd, 0xffff);
            append_ir2_opnd2i(LISA_ADDI_D, &cx_temp, &cx_temp, -1);
            store_ir2_to_ir1_gpr(&cx_temp, &cx_ir1_opnd);
            append_ir2_opnd2i(LISA_SLTUI,  &condition, &cx_temp, 1);
            ra_free_temp(&cx_temp);
        }

        /*     1> result != 0 (repe)
         *     2> result == 0 (repne)    */
        if (ir1_has_prefix_repe(pir1)) {
            /* set 1 when 0 < result, i.e., result!=0 */
            append_ir2_opnd3(LISA_SLTU, &condition2, &zero_ir2_opnd, result); 
        }
        if (ir1_has_prefix_repne(pir1)) {
            /* set 1 when result < 1, i.e., result==0 */
            append_ir2_opnd2i(LISA_SLTUI, &condition2, result, 1);
        }

        /* 4.3 when none of the two conditions is satisfied, the loop continues*/
        append_ir2_opnd3(LISA_OR,  &condition, &condition, &condition2);
        append_ir2_opnd3(LISA_BEQ, &condition, &zero_ir2_opnd, &label_loop);

        ra_free_temp(&condition);
        ra_free_temp(&condition2);
    }

    /* 2. exit label */
    if (!result)
        append_ir2_opnd1(LISA_LABEL, &label_exit);
}

bool translate_ins(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* destination : MEM(ES:(E)DI) */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* I/O port : DX */

    int data_size = ir1_opnd_size(opnd0);

    /* Always check IO before any IO operation */
    tr_gen_io_check(pir1, opnd1, data_size);

    /* 1. loop starts */
    IR2_OPND label_loop = ir2_opnd_new_inv();;
    IR2_OPND label_exit = ir2_opnd_new_inv();;
    tr_gen_string_loop_start(pir1, &label_loop, &label_exit, 0, NULL);

    tr_gen_io_start();

    int addr_size = ir1_addr_size(pir1);

    /* 2. I/O read from I/O port DX : helper_inb/w/l
     * 3. write to MEM(ES:(E)DI)    : softmmu helper
     * 4. adjust EDI                : according to opnd size */

    /* Note: we must do this dummy write first to be restartable in
       case of page fault. */
    store_ir2_to_ir1_mem(&zero_ir2_opnd, opnd0, false, addr_size);
    /* 2.0 save native context */
    tr_sys_gen_call_to_helper_prologue_cfg(zero_helper_cfg);
    /* 2.1 prepare parameter for IO read helper */
    /*    > arg0 : CPUX86State *env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    /*    > arg1 ; uint32_t    port */
    IR2_OPND port_reg = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
    append_ir2_opnd2_(lisa_mov16z, &arg1_ir2_opnd, &port_reg);
//    append_ir2_opnd2i(mips_andi, &arg1_ir2_opnd, &port_reg, 0xffff);
    /* 2.2 call the helper_inb/inw/inl */
    switch (data_size) {
        /* target/i386/misc_helper.c */
        case 8:  tr_gen_call_to_helper((ADDR)helper_inb); break;
        case 16: tr_gen_call_to_helper((ADDR)helper_inw); break;
        case 32: tr_gen_call_to_helper((ADDR)helper_inl); break;
        default:
            lsassertm_illop(ir1_addr(pir1),
                    0, "Unsupported X86_INS_INS data size %d.\n", data_size);
    }
    /* 2.3 resotre the naive context */
    tr_sys_gen_call_to_helper_epilogue_cfg(zero_helper_cfg);
    /* 2.4 get the read IO value at $v0 */
    IR2_OPND io_value_reg = ra_alloc_itemp();
    append_ir2_opnd2_(lisa_mov, &io_value_reg, &ret0_ir2_opnd);

    /* 3. write to mem */
    store_ir2_to_ir1_mem(&io_value_reg, opnd0, false, addr_size);
    ra_free_temp(&io_value_reg);

    //tr_gen_io_end();

    /* 4. adjust (E)DI */
    IR2_OPND step_opnd = ra_alloc_itemp();
    load_string_step_to_ir2(&step_opnd, pir1, 0);
    if (addr_size == 4) {
        /* adjust EDI */
        IR2_OPND edi_opnd = ra_alloc_gpr(edi_index);
        append_ir2_opnd3(LISA_SUB_D, &edi_opnd, &edi_opnd, &step_opnd);
    } else {
        /* adjust DI */
        IR2_OPND tmp = ra_alloc_itemp();
        IR2_OPND edi_opnd = ra_alloc_gpr(edi_index);
        append_ir2_opnd3(LISA_SUB_D, &tmp, &edi_opnd, &step_opnd);
        store_ir2_to_ir1_gpr(&tmp, &di_ir1_opnd);
        ra_free_temp(&tmp);
    }

    /* 5. breakpoint IO */
    tr_gen_io_bpt(pir1, opnd1, data_size >> 3);

    /* 6. loop ends */
    tr_gen_string_loop_end(pir1, label_loop, label_exit, NULL);

    ra_free_temp(&step_opnd);

    return true;
}

bool translate_outs(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* I/O port : DX  */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* src data : MEM(ES:(E/R)SI) */

    int data_size = ir1_opnd_size(opnd1);

    /* Always check IO before any IO operation */
    tr_gen_io_check(pir1, opnd0, data_size);

    /* 1. loop starts */
    IR2_OPND label_loop = ir2_opnd_new_inv();
    IR2_OPND label_exit = ir2_opnd_new_inv();
    tr_gen_string_loop_start(pir1, &label_loop, &label_exit, 0, NULL);

    tr_gen_io_start();

    /* 2. read from MEM(ES:(E)DI)  : softmmu helper 
     * 3. I/O write to I/O port DX : helper_outb/w/l
     * 4. adjust EDI               : according to opnd size */

    /* 2. load value from mem */
    IR2_OPND io_value_reg = ra_alloc_itemp();
    load_ir1_to_ir2(&io_value_reg, opnd1, UNKNOWN_EXTENSION, false);

    /* 3.0 save native context */
    tr_sys_gen_call_to_helper_prologue_cfg(zero_helper_cfg);
    /* 3.1 prepare parameter for IO read helper */
    /*    > arg2 : uint32_t    data */
    append_ir2_opnd2_(lisa_mov, &arg2_ir2_opnd, &io_value_reg);
    /*    > arg0 : CPUX86State *env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    /*    > arg1 ; uint32_t    port */
    IR2_OPND port_reg = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
    append_ir2_opnd2_(lisa_mov16z, &arg1_ir2_opnd, &port_reg);
//    append_ir2_opnd2i(mips_andi, &arg1_ir2_opnd, &port_reg, 0xffff);
    /* 3.2 call the helper_outb/inw/inl */
    switch (data_size) {
        /* target/i386/misc_helper.c */
        case 8:  tr_gen_call_to_helper((ADDR)helper_outb); break;
        case 16: tr_gen_call_to_helper((ADDR)helper_outw); break;
        case 32: tr_gen_call_to_helper((ADDR)helper_outl); break;
        default:
            lsassertm_illop(ir1_addr(pir1),
                    0, "Unsupported X86_INS_OUTS data size %d.\n", data_size);
    }
    /* 3.3 resotre the naive context */
    tr_sys_gen_call_to_helper_epilogue_cfg(zero_helper_cfg);

    //tr_gen_io_end();

    /* 4. adjust (E)SI */
    IR2_OPND step_opnd = ra_alloc_itemp();
    load_string_step_to_ir2(&step_opnd, pir1, 1);
    int addr_size = ir1_addr_size(pir1);
    if (addr_size == 4) {
        /* adjust ESI */
        IR2_OPND esi_opnd = ra_alloc_gpr(esi_index);
        append_ir2_opnd3(LISA_SUB_D, &esi_opnd, &esi_opnd, &step_opnd);
    } else {
        /* adjust SI */
        IR2_OPND tmp = ra_alloc_itemp();
        IR2_OPND esi_opnd = ra_alloc_gpr(esi_index);
        append_ir2_opnd3(LISA_SUB_D, &tmp, &esi_opnd, &step_opnd);
        store_ir2_to_ir1_gpr(&tmp, &si_ir1_opnd);
        ra_free_temp(&tmp);
    }

    /* 5. breakpoint IO */
    tr_gen_io_bpt(pir1, opnd1, data_size >> 3);

    /* 6. loop ends */
    tr_gen_string_loop_end(pir1, label_loop, label_exit, NULL);

    ra_free_temp(&step_opnd);

    return true;
}

bool translate_movs(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* dest: MEM(ES:(E/R)DI) */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* src : MEM(DS:(E/R)SI) */

    int addr_size = ir1_addr_size(pir1);

    /* 1. loop starts */
    IR2_OPND label_loop = ir2_opnd_new_inv();
    IR2_OPND label_exit = ir2_opnd_new_inv();
    IR2_OPND step_opnd  = ir2_opnd_new_inv();
    tr_gen_string_loop_start(pir1, &label_loop, &label_exit, 1, &step_opnd);

    /* 2. load memory value at ESI, and store into memory at EDI */
    IR2_OPND src_value = ra_alloc_itemp();
    load_ir1_to_ir2(&src_value, opnd1, SIGN_EXTENSION, false);
    store_ir2_to_ir1_mem(&src_value, opnd0, false, addr_size);

    /* 3. adjust (E)SI and (E)DI */
    if (addr_size == 4) { /* ESI and EDI */
        IR2_OPND esi_opnd = ra_alloc_gpr(esi_index);
        IR2_OPND edi_opnd = ra_alloc_gpr(edi_index);
        append_ir2_opnd3(LISA_SUB_D, &esi_opnd, &esi_opnd, &step_opnd);
        append_ir2_opnd3(LISA_SUB_D, &edi_opnd, &edi_opnd, &step_opnd);
    }
    else { /* SI and DI */
        IR2_OPND si_opnd = ra_alloc_itemp();
        IR2_OPND di_opnd = ra_alloc_itemp();
        load_ir1_to_ir2(&si_opnd, &si_ir1_opnd, ZERO_EXTENSION, false);
        load_ir1_to_ir2(&di_opnd, &di_ir1_opnd, ZERO_EXTENSION, false);
        append_ir2_opnd3(LISA_SUB_D, &si_opnd, &si_opnd, &step_opnd);
        append_ir2_opnd3(LISA_SUB_D, &di_opnd, &di_opnd, &step_opnd);
        store_ir2_to_ir1_gpr(&si_opnd, &si_ir1_opnd);
        store_ir2_to_ir1_gpr(&di_opnd, &di_ir1_opnd);
        ra_free_temp(&si_opnd);
        ra_free_temp(&di_opnd);
    }
 
    /* 4. loop ends */
    tr_gen_string_loop_end(pir1, label_loop, label_exit, NULL);

    ra_free_temp(&step_opnd);

    return true;
}

bool translate_scas(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* src : AL, AX, EAX     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* src : MEM(ES:(E/R)DI) */

    int addr_size = ir1_addr_size(pir1);

    /* 0. preparations outside the loop */
    IR2_OPND eax_value_opnd = ra_alloc_itemp();
    load_ir1_gpr_to_ir2(&eax_value_opnd, opnd0, SIGN_EXTENSION);

    /* 1. loop starts */
    IR2_OPND label_loop = ir2_opnd_new_inv();
    IR2_OPND label_exit = ir2_opnd_new_inv();
    IR2_OPND step_opnd  = ir2_opnd_new_inv();
    tr_gen_string_loop_start(pir1, &label_loop, &label_exit, 1, &step_opnd);

    /* 2.1 load memory value at MEM(ES:(E)DI) */
    IR2_OPND edi_mem_value = ra_alloc_itemp();
    load_ir1_mem_to_ir2(&edi_mem_value, opnd1, SIGN_EXTENSION, false, addr_size);

    /* 2.2 adjust EDI */
    if (addr_size == 4) { /* EDI */
        IR2_OPND edi_opnd = ra_alloc_gpr(edi_index);
        append_ir2_opnd3(LISA_SUB_D, &edi_opnd, &edi_opnd, &step_opnd);
    }
    else { /* DI */
        IR2_OPND di_opnd = ra_alloc_itemp();
        load_ir1_gpr_to_ir2(&di_opnd, &di_ir1_opnd, ZERO_EXTENSION);
        append_ir2_opnd3(LISA_SUB_D, &di_opnd, &di_opnd, &step_opnd);
        store_ir2_to_ir1_gpr(&di_opnd, &di_ir1_opnd);
        ra_free_temp(&di_opnd);
    }

    /* 3. compare */
    IR2_OPND cmp_result = ra_alloc_itemp();
    append_ir2_opnd3(LISA_SUB_W, &cmp_result, &eax_value_opnd, &edi_mem_value);

    /* 4. loop ends */
    tr_gen_string_loop_end(pir1, label_loop, label_exit, &cmp_result);

    /* 5. calculate eflags */
    generate_eflag_calculation(&cmp_result, &eax_value_opnd, &edi_mem_value, pir1, true);
    ra_free_temp(&cmp_result);

    /* 6. loop exits */
    append_ir2_opnd1(LISA_LABEL, &label_exit);

    ra_free_temp(&step_opnd);

    return true;
}

bool translate_cmps(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* src : MEM(ES:(E/R)SI) */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* src : MEM(ES:(E/R)DI) */

    int addr_size = ir1_addr_size(pir1);

    /* 1. loop starts */
    IR2_OPND label_loop = ir2_opnd_new_inv();
    IR2_OPND label_exit = ir2_opnd_new_inv();
    IR2_OPND step_opnd  = ir2_opnd_new_inv();
    tr_gen_string_loop_start(pir1, &label_loop, &label_exit, 1, &step_opnd);

    /* 2.1 load memory value at ESI and EDI */
    IR2_OPND esi_mem_value = ra_alloc_itemp();
    IR2_OPND edi_mem_value = ra_alloc_itemp();

    load_ir1_mem_to_ir2(&esi_mem_value, opnd0, SIGN_EXTENSION, false, addr_size);
    load_ir1_mem_to_ir2(&edi_mem_value, opnd1, SIGN_EXTENSION, false, addr_size);

    /* 2.2 adjust ESI and EDI */
    if (addr_size == 4) { /* ESI and EDI */
        IR2_OPND esi_opnd = ra_alloc_gpr(esi_index);
        IR2_OPND edi_opnd = ra_alloc_gpr(edi_index);
        append_ir2_opnd3(LISA_SUB_D, &esi_opnd, &esi_opnd, &step_opnd);
        append_ir2_opnd3(LISA_SUB_D, &edi_opnd, &edi_opnd, &step_opnd);
    }
    else { /* SI and DI */
        IR2_OPND si_opnd = ra_alloc_itemp();
        IR2_OPND di_opnd = ra_alloc_itemp();
        load_ir1_gpr_to_ir2(&si_opnd, &si_ir1_opnd, ZERO_EXTENSION);
        load_ir1_gpr_to_ir2(&di_opnd, &di_ir1_opnd, ZERO_EXTENSION);
        append_ir2_opnd3(LISA_SUB_D, &si_opnd, &si_opnd, &step_opnd);
        append_ir2_opnd3(LISA_SUB_D, &di_opnd, &di_opnd, &step_opnd);
        store_ir2_to_ir1_gpr(&si_opnd, &si_ir1_opnd);
        store_ir2_to_ir1_gpr(&di_opnd, &di_ir1_opnd);
        ra_free_temp(&si_opnd);
        ra_free_temp(&di_opnd);
    }

    /* 3. compare */
    IR2_OPND cmp_result = ra_alloc_itemp();
    append_ir2_opnd3(LISA_SUB_W, &cmp_result, &esi_mem_value, &edi_mem_value);

    /* 4. loop ends */
    tr_gen_string_loop_end(pir1, label_loop, label_exit, &cmp_result);

    /* 5. calculate eflags */
    generate_eflag_calculation(&cmp_result, &esi_mem_value, &edi_mem_value, pir1, true);
    ra_free_temp(&cmp_result);

    /* 6. loop exits */
    append_ir2_opnd1(LISA_LABEL, &label_exit);

    ra_free_temp(&step_opnd);

    return true;
}

bool translate_lods(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* dest : AL, AX, EAX    */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* src  : MEM(ES:(E)SI)  */

    int addr_size = ir1_addr_size(pir1);

    /* 1. loop starts */
    IR2_OPND label_loop = ir2_opnd_new_inv();
    IR2_OPND label_exit = ir2_opnd_new_inv();
    IR2_OPND step_opnd  = ir2_opnd_new_inv();
    tr_gen_string_loop_start(pir1, &label_loop, &label_exit, 1, &step_opnd);

    /* 2. load memory value at MEM(ES:(E)SI) */
    EXTENSION_MODE em = SIGN_EXTENSION;
    if (ir1_opnd_size(opnd0) < 32) em = ZERO_EXTENSION;
    IR2_OPND esi_mem_value = ra_alloc_itemp();
    load_ir1_mem_to_ir2(&esi_mem_value, opnd1, em, false, addr_size);

    /* 3 adjust (E)SI */
    if (addr_size == 4) { /* ESI */
        IR2_OPND esi_opnd = ra_alloc_gpr(esi_index);
        append_ir2_opnd3(LISA_SUB_D, &esi_opnd, &esi_opnd, &step_opnd);
    }
    else {
        IR2_OPND si_opnd = ra_alloc_itemp();
        load_ir1_gpr_to_ir2(&si_opnd, &si_ir1_opnd, ZERO_EXTENSION);
        append_ir2_opnd3(LISA_SUB_D, &si_opnd, &si_opnd, &step_opnd);
        store_ir2_to_ir1_gpr(&si_opnd, &si_ir1_opnd);
        ra_free_temp(&si_opnd);
    }

    /* 4. loop ends */
    tr_gen_string_loop_end(pir1, label_loop, label_exit, NULL);

    /* 5. store the load value to dest */
    store_ir2_to_ir1_gpr(&esi_mem_value, opnd0);

    ra_free_temp(&step_opnd);

    return true;
}

bool translate_stos(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* dest: MEM(ES:(E/R)DI) */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* src : AL, AX, EAX     */

    int addr_size = ir1_addr_size(pir1);

    /* 0. preparations outside the loop */
    IR2_OPND eax_value_opnd = ra_alloc_itemp();
    load_ir1_gpr_to_ir2(&eax_value_opnd, opnd1, SIGN_EXTENSION);

    /* 1. loop starts */
    IR2_OPND label_loop = ir2_opnd_new_inv();
    IR2_OPND label_exit = ir2_opnd_new_inv();
    IR2_OPND step_opnd  = ir2_opnd_new_inv();
    tr_gen_string_loop_start(pir1, &label_loop, &label_exit, 1, &step_opnd);

    /* 2. store EAX into memory at EDI */
    store_ir2_to_ir1_mem(&eax_value_opnd, opnd0, false, addr_size);

    /* 3. adjust (E)DI */
    if (addr_size == 4) { /* EDI */
        IR2_OPND edi_opnd = ra_alloc_gpr(edi_index);
        append_ir2_opnd3(LISA_SUB_D, &edi_opnd, &edi_opnd, &step_opnd);
    }
    else {
        IR2_OPND di_opnd = ra_alloc_itemp();
        load_ir1_gpr_to_ir2(&di_opnd, &di_ir1_opnd, ZERO_EXTENSION);
        append_ir2_opnd3(LISA_SUB_D, &di_opnd, &di_opnd, &step_opnd);
        store_ir2_to_ir1_gpr(&di_opnd, &di_ir1_opnd);
        ra_free_temp(&di_opnd);
    }

    /* 4. loop ends */
    tr_gen_string_loop_end(pir1, label_loop, label_exit, NULL);

    ra_free_temp(&step_opnd);

    return true;
}
