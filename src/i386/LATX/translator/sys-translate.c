#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>
#include "qemu/cacheflush.h"

/* Main Translation Process */

void latxs_tr_init(TranslationBlock *tb)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    lsassert(td);

    /* 1. set current tb and ir1 */
    latxs_tr_tb_init(td, tb);

    /* 2. register allocation init */
    latxs_tr_ra_init(td);

    /* 3. reset all registers extension mode */
    latxs_tr_em_init(td);

    /* 4. reset ir2 array */
    latxs_tr_ir2_array_init(td);

    /* 5. set up lsfpu */
    latxs_tr_fpu_init(td, tb);

    if (tb) {
        td->x86_ins_nr = 0;
        td->exitreq_label = latxs_ir2_opnd_new_label();
        td->end_with_exception = 0;
    }
}

void latxs_tr_tb_init(TRANSLATION_DATA *td, TranslationBlock *tb)
{
    td->curr_tb = tb;
    td->curr_ir1_inst = NULL;
}

void latxs_tr_ir2_array_init(TRANSLATION_DATA *td)
{
    if (td->ir2_inst_array == NULL) {
        td->ir2_inst_array = mm_calloc(512, sizeof(IR2_INST));
        td->ir2_inst_num_max = 512;
    }
    td->ir2_inst_num_current = 0;
    td->real_ir2_inst_num = 0;

    td->first_ir2 = NULL;
    td->last_ir2 = NULL;

    td->label_num = 0;
}

void latxs_tr_fini(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    lsassert(td != NULL);

    td->curr_tb = NULL;
    td->curr_ir1_inst = NULL;

    td->ir2_inst_num_current = 0;
    td->real_ir2_inst_num = 0;

    td->first_ir2 = NULL;
    td->last_ir2 = NULL;

    td->label_num = 0;
    td->itemp_num = 32;
    td->ftemp_num = 32;

    if (!option_lsfpu && !option_soft_fpu) {
        latxs_td_fpu_set_top(0);
    }
}

void latxs_label_dispose(const void *code_buffer)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    int label_nr = td->label_num;

    /* 1. record the positions of label */
    int *label_pos = (int *)alloca(label_nr * 4 + 20);
    memset(label_pos , -1, label_nr * 4 + 20);

    int ir2_num = 0;
    IR2_INST *ir2_current = td->first_ir2;
    IR2_OPCODE ir2_opc = latxs_ir2_opcode(ir2_current);

    while (ir2_current != NULL) {
        ir2_opc = latxs_ir2_opcode(ir2_current);

        if (ir2_opc  == LISA_LABEL) {
            int label_id = latxs_ir2_opnd_label_id(&ir2_current->_opnd[0]);
            lsassertm(label_pos[label_id] == -1,
                      "label %d is in multiple positions\n", label_id);
            label_pos[label_id] = ir2_num;
        } else if (ir2_opc == LISA_X86_INST) {
            /* will not be assembled */
        } else {
            ir2_num++;
        }

        ir2_current = latxs_ir2_next(ir2_current);
    }

    /*
     * 2. resolve the offset of successor linkage code
     * @jmp_target_arg recoed the jmp  inst position
     * @jmp_reset_offset record the successor inst of jmp(exclude delay slot)
     *                   when the tb is removed from buffer. the jmp inst use
     *                   this position to revert the original "fall through"
     */
    {
        TranslationBlock *tb = td->curr_tb;
        /* prologue/epilogue has no tb */
        if (tb) {
            /* ctx->jmp_insn_offset point to tb->jmp_target_arg */
            int label_id_0 = tb->jmp_reset_offset[0];
            if (label_id_0 != TB_JMP_RESET_OFFSET_INVALID) {
                tb->jmp_reset_offset[0] =
                    (label_pos[label_id_0] << 2) + 8;
                tb->jmp_target_arg[0] = (label_pos[label_id_0] << 2);
            }
            int label_id_1 = tb->jmp_reset_offset[1];
            if (label_id_1 != TB_JMP_RESET_OFFSET_INVALID) {
                tb->jmp_reset_offset[1] =
                    (label_pos[label_id_1] << 2) + 8;
                tb->jmp_target_arg[1] = (label_pos[label_id_1] << 2);
            }
        }
    }

    /* 3. resolve the branch instructions */
    ir2_num = 0;
    ir2_current = td->first_ir2;

    while (ir2_current != NULL) {
        ir2_opc = latxs_ir2_opcode(ir2_current);
        if (latxs_ir2_opcode_is_branch(ir2_opc)) {

            IR2_OPND *label_opnd = latxs_ir2_branch_get_label(ir2_current);

            /* LISA_B and LISA_BL could contain label or imm */
            if (label_opnd && latxs_ir2_opnd_is_label(label_opnd)) {
                int label_id = latxs_ir2_opnd_label_id(label_opnd);
                lsassert(label_id > 0 && label_id <= label_nr);

                int label_pos_ir2_num = label_pos[label_id];

                lsassertm(label_pos_ir2_num != -1,
                        "label %d is not inserted\n",
                         label_id);

                latxs_ir2_opnd_convert_label_to_imm(
                        label_opnd, label_pos_ir2_num - ir2_num);
            }
        }

        if (ir2_opc != LISA_LABEL && ir2_opc != LISA_X86_INST) {
            ir2_num++;
        }

        ir2_current = latxs_ir2_next(ir2_current);
    }
}

int latxs_tr_check_buffer_overflow(
        const void *code_start,
        TRANSLATION_DATA *td)
{
    uint64_t code_highwater = (uint64_t)td->code_highwater;
    uint64_t code_tb_end = (uint64_t)code_start;

    int code_nr = td->ir2_inst_num_current;
    code_nr -= td->label_num; /* lisa_label    */
    code_nr -= td->ir1_nr;    /* lisa_x86_inst */

    code_tb_end += code_nr << 2;

    if (code_tb_end >= code_highwater) {
        return 1;
    }

    return 0;
}

int latxs_tr_ir2_assemble(const void *code_base)
{
    /* 1. label dispose */
    latxs_label_dispose(code_base);

    /* 2. assemble */
    IR2_INST *pir2 = lsenv->tr_data->first_ir2;
    void *code_ptr = (void *)(ADDR)code_base;
    int code_nr = 0;

    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;

    /* Buffer Overflow Check */
    if (tb) {
        /* Do not check buffer overflow when generating */
        /* context switch (prologue/epilogue) */
        if (latxs_tr_check_buffer_overflow(code_base, td)) {
            return -1;
        }
    }

    /* Counter for encode search */
    int x86ins_idx  = 0;
    int x86ins_nr  = -1;
    int x86ins_hcnt = 0; /* host instruction count */
    int x86ins_size = 0;

    while (pir2 != NULL) {
        IR2_OPCODE ir2_opc = latxs_ir2_opcode(pir2);

        if (ir2_opc == LISA_X86_INST) {

            if (option_dump_host) {
                fprintf(stderr, "IR2 at %p LISA X86Ins = %x\n",
                        code_ptr, latxs_ir2_opnd_imm(&pir2->_opnd[0]));
            }

            if (x86ins_nr >= 0) {
                td->x86_ins_idx[x86ins_nr] = x86ins_idx;
                td->x86_ins_lisa_nr[x86ins_nr] = x86ins_hcnt;
            }
            x86ins_idx = latxs_ir2_get_id(pir2);
            x86ins_nr += 1;
            /* calculate TB size */
            IR1_INST *pir1 = &td->ir1_inst_array[x86ins_nr];
            x86ins_size += latxs_ir1_inst_size(pir1);

            goto _NEXT_IR2_;
        }

        if (ir2_opc != LISA_LABEL) {
            uint32_t ir2_binary = latxs_ir2_assemble(pir2);
            if (option_dump_host) {
                fprintf(stderr, "IR2 at %p LISA Binary = 0x%08x ",
                        code_ptr, ir2_binary);
                latxs_ir2_dump(pir2);
            }

            *(uint32_t *)code_ptr = ir2_binary;
            code_ptr = code_ptr + 4;
            code_nr += 1;

            x86ins_hcnt += 1;
        }

_NEXT_IR2_:
        pir2 = latxs_ir2_next(pir2);
    }

    if (td->curr_tb) {
        td->x86_ins_idx[x86ins_nr] = x86ins_idx;
        td->x86_ins_lisa_nr[x86ins_nr] = x86ins_hcnt;
        td->x86_ins_nr = x86ins_nr + 1;
        td->x86_ins_size = x86ins_size;

        lsassertm(td->x86_ins_nr == td->ir1_nr,
                "x86 inst %d not equal to IR1 %d.\n",
                td->x86_ins_nr, td->ir1_nr);

        tb->icount = td->x86_ins_nr;
        tb->size = x86ins_size;
    }

    return code_nr;
}

int latxs_tr_translate_tb(TranslationBlock *tb, int *search_size)
{
    int code_nr = 0;
    int code_size = 0;
    int is_buffer_overflow = 0;

    latxs_tr_init(tb);

    bool translation_done = latxs_tr_ir2_generate(tb);

    if (translation_done) {
        code_nr = latxs_tr_ir2_assemble(tb->tc.ptr);
    }

    if (likely(code_nr > 0)) {
        code_size = code_nr * 4;
        counter_mips_tr += code_nr;
        is_buffer_overflow = 0;
    } else {
        /* Buffer Overflow */
        code_size = -1;
        is_buffer_overflow = 1;
    }

    if (tb && !is_buffer_overflow) {
        void *search_block = (void *)(ADDR)tb->tc.ptr + code_size;
        *search_size = latxs_tb_encode_search(tb, search_block);
    }

    latxs_tr_fini();

    if (tb) {
        /* 0. IR1 will not be used again, free it */
        int i = 0;
        for (i = 0; i < tb->icount; ++i) {
            latxs_ir1_free_info(tb->_ir1_instructions + i);
        }
        /* 1. check buffer overflow */
        if (unlikely(is_buffer_overflow)) {
            return -1;
        }
        /* 2. TODO check TB overflow */
        /* if (code_nr >= 0x3fff) { */
            /* return -2; */
        /* } */
        /* 3. flush icache range */
        ADDR s = (ADDR)tb->tc.ptr;
        ADDR e = s + code_size;
        flush_idcache_range((uintptr_t)s, (uintptr_t)s, (uintptr_t)e);
    }

    return code_size;
}

void latxs_tr_init_translate_ir1(TranslationBlock *tb, int index)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    IR1_INST *ir1_list = td->ir1_inst_array;
    int ir1_nr = td->ir1_nr;

    (void)ir1_nr; /* to avoid warning */
    lsassert(index >= 0 && ir1_nr >= 0 && index < ir1_nr);

    /* 1. initialize global data */
    IR1_INST *pir1 = ir1_list + index;
    lsenv->tr_data->curr_ir1_inst = pir1;

    /* 2. Append mips_x86_inst into IR2 list to mark */
    /*    the start of each IR1 */
    latxs_append_ir2_opnda(LISA_X86_INST, ir1_addr(pir1));

    /* 3. clear temp register mapping */
    td->itemp_mask = 0;
    td->ftemp_mask = 0;
    td->itemp_mask_bk = 0;
}

bool latxs_tr_ir2_generate(TranslationBlock *tb)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    int i = 0;

    IR1_INST *ir1_list = td->ir1_inst_array;
    int ir1_nr = td->ir1_nr;

    latxs_tr_gen_tb_start();

    IR1_INST *pir1 = NULL;
    for (i = 0; i < ir1_nr; ++i) {
        pir1 = ir1_list + i;

        /* option_trace_ir1(pir1); TODO for trace */

        latxs_tr_init_translate_ir1(tb, i);

        /* if ((td->max_insns == i + 1) && (td->sys.cflags & CF_LAST_IO)) { */
            /* latxs_tr_gen_io_start(); TODO for icount */
        /* } */

        if (option_dump_ir1) {
            fprintf(stderr, "ir1 translate >>> ");
            ir1_dump(pir1);
        }

#ifdef TARGET_X86_64
        if (td->sys.code64) {
            lsassert(((CPUX86State *)lsenv->cpu_state)->segs[cs_index].base ==
                     0);
            lsassert(((CPUX86State *)lsenv->cpu_state)->segs[ds_index].base ==
                     0);
            lsassert(((CPUX86State *)lsenv->cpu_state)->segs[es_index].base ==
                     0);
            lsassert(((CPUX86State *)lsenv->cpu_state)->segs[ss_index].base ==
                     0);
            if (ir1_opcode(pir1) != X86_INS_CALL) {
                if (latxs_ir1_has_prefix_addrsize(pir1)) {
                    ir1_dump(pir1);
                }
            }
            int opnd_num = ir1_opnd_num(pir1);
            for (size_t i = 0; i < opnd_num; i++) {
                IR1_OPND *op = ir1_get_opnd(pir1, i);
                if (ir1_opnd_is_mem(op) && ir1_opnd_has_seg(op) &&
                    !latxs_ir1_is_nop(pir1)) {
                    int seg_num = ir1_opnd_get_seg_index(op);
                    if (seg_num == cs_index || seg_num == ds_index ||
                        seg_num == es_index || seg_num == ss_index) {
                        lsassert(!lsenv->tr_data->sys.addseg);
                    }
                }
            }
        }
#endif

        bool translation_success = ir1_translate(pir1);

        (void)translation_success; /* to avoid warning  */
        lsassertm(translation_success,
                "translate failed for inst %s\n",
                pir1->info->mnemonic);

        if (td->end_with_exception) {
            /* 1. get real number of IR1 */
            int real_ir1_nr = i + 1;
            td->ir1_nr = real_ir1_nr;
            /* 2. Reset related fields in ETB */
            for (++i; i < ir1_nr; ++i) {
                pir1 = ir1_list + i;
                latxs_ir1_free_info(pir1);
            }
            tb->icount = real_ir1_nr;
            /* 3. TODO ICOUNT: decrease real number of IR1 */
            /* if (atomic_read(&tb->cflags) & CF_USE_ICOUNT) { */
                /* IR2_INST *pir2 = latxs_ir2_get(td->dec_icount_inst_id); */
                /* pir2->_opnd[2] = latxs_ir2_opnd_new(IR2_OPND_IMMH, */
                        /* 0 - real_ir1_nr); */
            /* } */
            break;
        }
    }

    if (!(td->end_with_exception)) {
        latxs_tr_gen_eob_if_tb_too_large(tb->tb_too_large_pir1);
        latxs_tr_gen_sys_eob(tb->sys_eob_pir1);
    }
    latxs_tr_gen_tb_end();
    tr_gen_softmmu_slow_path();

    if (!option_lsfpu && !option_soft_fpu) {
        tb->_top_out = latxs_td_fpu_get_top();
    }

    return true;
}

void latxs_tr_gen_tb_start(void)
{
    if (sigint_enabled()) {
        return;
    }

    TRANSLATION_DATA *td = lsenv->tr_data;
    IR2_OPND count = latxs_ra_alloc_itemp();

    latxs_append_ir2_opnd2i(LISA_LD_W, &count,
            &latxs_env_ir2_opnd,
            (int32_t)offsetof(X86CPU, neg.icount_decr.u32) -
            (int32_t)offsetof(X86CPU, env));

    /* TODO icount */

    latxs_append_ir2_opnd3(LISA_BLT, &count, &latxs_zero_ir2_opnd,
            &(td->exitreq_label));

    latxs_ra_free_temp(&count);
}

void latxs_tr_gen_tb_end(void)
{
    if (sigint_enabled()) {
        return;
    }

    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;

    latxs_append_ir2_opnd1(LISA_LABEL, &td->exitreq_label);

    IR2_OPND tb_ptr_opnd = latxs_ra_alloc_dbt_arg1();
    IR2_OPND eip_opnd = latxs_ra_alloc_itemp();

    ADDR tb_addr = (ADDR)tb;

    latxs_tr_gen_exit_tb_load_tb_addr(&tb_ptr_opnd, tb_addr);

    ADDRX eip = tb->pc - tb->cs_base;

#ifdef TARGET_X86_64
    latxs_load_imm64_to_ir2(&eip_opnd, eip);
    latxs_append_ir2_opnd2i(LISA_ST_D, &eip_opnd, &latxs_env_ir2_opnd,
                        lsenv_offset_of_eip(lsenv));
#else
    latxs_load_imm32_to_ir2(&eip_opnd, eip, EXMode_Z);
    latxs_append_ir2_opnd2i(LISA_ST_W, &eip_opnd, &latxs_env_ir2_opnd,
                        lsenv_offset_of_eip(lsenv));
#endif

    latxs_ra_free_temp(&eip_opnd);

    if (!option_lsfpu && !option_soft_fpu) {
        /* FPU top shoud be TB's top_in, not top_out. */
        IR2_OPND top_in = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ORI, &top_in,
                &latxs_zero_ir2_opnd, tb->_top_in & 0x7);
        latxs_append_ir2_opnd2i(LISA_ST_W, &top_in,
                &latxs_env_ir2_opnd, lsenv_offset_of_top(lsenv));
        latxs_ra_free_temp(&top_in);
    }

    latxs_append_ir2_opnd2i(LISA_ORI, &latxs_ret0_ir2_opnd,
            &tb_ptr_opnd, TB_EXIT_REQUESTED);

    ADDR code_buf = (ADDR)tb->tc.ptr;
    int offset = td->real_ir2_inst_num << 2;
    int64_t ins_offset =
        (context_switch_native_to_bt - code_buf - offset) >> 2;
    latxs_append_ir2_jmp_far(ins_offset, 0);
}

/* translate functions */

void latxs_tr_save_registers_to_env(
        uint32_t gpr_to_save,
        uint8_t fpr_to_save, int save_top,
        uint32_t xmm_to_save,
        uint8_t vreg_to_save)
{
    /* 1. GPR */
    latxs_tr_save_gprs_to_env(gpr_to_save);

    /* 2. FPR (MMX) */
    latxs_tr_save_fprs_to_env(fpr_to_save, save_top);

    /* 3. XMM */
    latxs_tr_save_xmms_to_env(xmm_to_save);

    /* 4. virtual registers */
    latxs_tr_save_vreg_to_env(vreg_to_save);
}

void latxs_tr_load_registers_from_env(
        uint32_t gpr_to_load,
        uint8_t fpr_to_load, int load_top,
        uint32_t xmm_to_load,
        uint8_t vreg_to_load)
{
    /* 4. virtual registers */
    latxs_tr_load_vreg_from_env(vreg_to_load);

    /* 3. XMM */
    latxs_tr_load_xmms_from_env(xmm_to_load);

    /* 2. FPR (MMX) */
    latxs_tr_load_fprs_from_env(fpr_to_load, load_top);

    /* 1. GPR */
    latxs_tr_load_gprs_from_env(gpr_to_load);

}

void latxs_tr_save_gprs_to_env(uint32_t mask)
{
    int i = 0;
    for (i = 0; i < CPU_NB_REGS; ++i) {
        if (BITS_ARE_SET(mask, 1 << i)) {
            IR2_OPND gpr_opnd = latxs_ra_alloc_gpr(i);
#ifdef TARGET_X86_64
            latxs_append_ir2_opnd2i(LISA_ST_D, &gpr_opnd, &latxs_env_ir2_opnd,
                                    lsenv_offset_of_gpr(lsenv, i));
#else
            latxs_append_ir2_opnd2i(LISA_ST_W, &gpr_opnd, &latxs_env_ir2_opnd,
                                    lsenv_offset_of_gpr(lsenv, i));
#endif
        }
    }
}

void latxs_tr_save_fprs_to_env(uint8_t mask, int save_top)
{
    int i = 0;

    /* 1. save FPU top: curr_top is rotated */
    if (save_top) {
        lsassert(!option_soft_fpu);
        latxs_tr_gen_save_curr_top();
        latxs_tr_fpu_disable_top_mode();
    }
    /* 2. save FPRs: together with FPU top */
    for (i = 0; i < 8; i++) {
        if (BITS_ARE_SET(mask, 1 << i)) {
            IR2_OPND mmx_opnd = latxs_ra_alloc_mmx(i);
            latxs_append_ir2_opnd2i(LISA_FST_D, &mmx_opnd,
                    &latxs_env_ir2_opnd,
                    lsenv_offset_of_mmx(lsenv, i));
        }
    }

}

void latxs_tr_save_xmms_to_env(uint32_t mask)
{
    int i = 0;
    for (i = 0; i < CPU_NB_REGS; i++) {
        if (BITS_ARE_SET(mask, 1 << i)) {
            IR2_OPND xmm_opnd = latxs_ra_alloc_xmm(i);
            latxs_append_ir2_opnd2i(LISA_VST, &xmm_opnd,
                    &latxs_env_ir2_opnd,
                    lsenv_offset_of_xmm(lsenv, i));
        }
    }
}

void latxs_tr_save_vreg_to_env(uint8_t mask)
{
    int i = 0;
    for (i = 0; i < 8; ++i) {
        if (BITS_ARE_SET(mask, 1 << i)) {
            IR2_OPND vreg_opnd = latxs_ra_alloc_vreg(i);
            latxs_append_ir2_opnd2i(LISA_ST_D, &vreg_opnd,
                    &latxs_env_ir2_opnd,
                    lsenv_offset_of_vreg(lsenv, i));
        }
    }
}

void latxs_tr_load_gprs_from_env(uint32_t mask)
{
    int i = 0;
    for (i = 0; i < CPU_NB_REGS; ++i) {
        if (BITS_ARE_SET(mask, 1 << i)) {
            IR2_OPND gpr_opnd = latxs_ra_alloc_gpr(i);
#ifdef TARGET_X86_64
            latxs_append_ir2_opnd2i(LISA_LD_D, &gpr_opnd, &latxs_env_ir2_opnd,
                                    lsenv_offset_of_gpr(lsenv, i));
#else
            latxs_append_ir2_opnd2i(LISA_LD_W, &gpr_opnd, &latxs_env_ir2_opnd,
                                    lsenv_offset_of_gpr(lsenv, i));
#endif
        }
    }
}

void latxs_tr_load_fprs_from_env(uint8_t mask, int load_top)
{
    int i = 0;

    for (i = 0; i < 8; i++) {
        if (BITS_ARE_SET(mask, 1 << i)) {
            IR2_OPND mmx_opnd = latxs_ra_alloc_mmx(i);
            latxs_append_ir2_opnd2i(LISA_FLD_D, &mmx_opnd,
                    &latxs_env_ir2_opnd,
                    lsenv_offset_of_mmx(lsenv, i));
        }
    }

    if (load_top && option_lsfpu) {
        lsassert(!option_soft_fpu);
        IR2_OPND top = latxs_ra_alloc_itemp();
        latxs_tr_load_lstop_from_env(&top);
        latxs_ra_free_temp(&top);
        latxs_tr_fpu_enable_top_mode();
    }
}

void latxs_tr_load_xmms_from_env(uint32_t mask)
{
    int i = 0;
    for (i = 0; i < CPU_NB_REGS; i++) {
        if (BITS_ARE_SET(mask, 1 << i)) {
            IR2_OPND xmm_opnd = latxs_ra_alloc_xmm(i);
            latxs_append_ir2_opnd2i(LISA_VLD, &xmm_opnd,
                    &latxs_env_ir2_opnd,
                    lsenv_offset_of_xmm(lsenv, i));
        }
    }
}

void latxs_tr_load_vreg_from_env(uint8_t mask)
{
    int i = 0;
    for (i = 0; i < 8; ++i) {
        if (BITS_ARE_SET(mask, 1 << i)) {
            IR2_OPND vreg_opnd = latxs_ra_alloc_vreg(i);
            latxs_append_ir2_opnd2i(LISA_LD_D, &vreg_opnd,
                    &latxs_env_ir2_opnd,
                    lsenv_offset_of_vreg(lsenv, i));
        }
    }
}

void latxs_enter_mmx(void)
{
    /* same as helper_enter_mmx */
    latxs_append_ir2_opnd2i(LISA_ST_W, &latxs_zero_ir2_opnd,
                            &latxs_env_ir2_opnd, lsenv_offset_of_top(lsenv));
    latxs_append_ir2_opnd2i(LISA_ST_D, &latxs_zero_ir2_opnd,
                            &latxs_env_ir2_opnd,
                            lsenv_offset_of_tag_word(lsenv));
}

void do_func(ADDRX addr, int id)
{
    IR2_OPND eip_opnd = latxs_ra_alloc_itemp();
    if (id == 0xa0) {
        latxs_load_imm64_to_ir2(&eip_opnd, addr);
#ifdef TARGET_X86_64
        latxs_append_ir2_opnd2i(LISA_ST_D, &eip_opnd, &latxs_env_ir2_opnd,
                                lsenv_offset_of_eip(lsenv));
#else
        latxs_append_ir2_opnd2i(LISA_ST_W, &eip_opnd, &latxs_env_ir2_opnd,
                                lsenv_offset_of_eip(lsenv));
#endif
        latxs_ra_free_temp(&eip_opnd);

        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_x86_cpu_dump_state,
                                         default_helper_cfg);
    }
}
