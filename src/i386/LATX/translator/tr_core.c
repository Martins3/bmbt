#include "../include/common.h"
#include "../include/env.h"
#include "../include/etb.h"
#include "../ir2/ir2.h"
#include "../ir1/ir1.h"
#include "../include/reg_alloc.h"

#include "../include/flag_lbt.h"
#include "../x86tomips-options.h"

#include "../include/ibtc.h"
#include "../include/profile.h"
#include "../include/flag_pattern.h"
#include "../include/shadow_stack.h"
#include <assert.h>

#ifdef CONFIG_SOFTMMU
#include "../debug/lockstep.h"
#  ifdef CONFIG_XTM_PROFILE
#  include "x86tomips-profile-sys.h"
#  endif
#endif

#include <string.h>

/*
 * tr_diasm() in tr_disasm.c
 *
 * tr_translate_tb()
 * /- tr_init()
 * /- tr_ir2_generate()
 * |  /- tr_init_translate_ir1()
 * |  /- ir1_translate()
 * /- tr_ir2_optimize() in ir2_optimization.c
 * /- tr_ir2_assemble()
 * |  /- label_dispose()
 * |  /- ir2_assemble() in ir2_assemble.c
 * /- tr_fini()
 *
 */

/* Generate all the data of TranslationBlock:
 * > translate from IR1 into IR2
 *   > IR1 will no longer be available after translation
 *   > IR2 is only available during translation
 * > generated host binary code from IR2 into code cache
 * > set the necessary fields of TranslationBlock
 * > check for code cache's buffer overflow
 * > prepare the encoded search data for precise exception */
int tr_translate_tb(TranslationBlock *tb, int *search_size)
{
    if (option_dump)
        fprintf(stderr, "[X86toMIPS] start translation.\n");

    /* Initialization */
    tr_init(tb);
    if (option_dump)
        fprintf(stderr, "tr_init OK. ready to translation.\n");

    /* generate IR2 from IR1 */
    bool translation_done = tr_ir2_generate(tb);

    int code_nr = 0;

    if (translation_done) {
        /* optimize ir2 */
        /*tr_ir2_optimize();*/

        /* assemble ir2 to native code */
        code_nr = tr_ir2_assemble(tb->tc.ptr);
    }

    /* Check code cache's buffer overflow. */
#ifndef CONFIG_SOFTMMU /* user-mode */
    int code_size = code_nr * 4;
    counter_mips_tr += code_nr;
#else /* system-mode */
    int code_size = 0;
    int is_buffer_overflow = 0;
    if (likely(code_nr > 0)) {
        code_size = code_nr * 4;
        counter_mips_tr += code_nr;
        is_buffer_overflow = 0;
    } else {
        /* Buffer Overflow */
        code_size = -1;
        is_buffer_overflow = 1;
    }
#endif

    /* Encoded search for precise exception.
     * Only useful in system-mode.*/
#ifdef CONFIG_SOFTMMU
    if (tb && !is_buffer_overflow) {
        void *search_block = tb->tc.ptr + code_size;
        *search_size = tb_encode_search(tb, search_block);
    }
#else
    *search_size = 0;
#endif

    /* finalize */
    tr_fini(translation_done);
    if (option_dump)
        fprintf(stderr, "tr_fini OK. translation done.\n");

#ifdef CONFIG_SOFTMMU /* system-mode */
    if (tb) {
        /* 0. IR1 will not be used again, free it */
        qm_tb_clr_ir1(tb);
        /* 1. check buffer overflow */
        if (unlikely(is_buffer_overflow)) {
            return -1;
        }
        /* 2. check TB overflow */
        if (code_nr >= 0x3fff) {
            return -2;
        }
        /* 3. flush icache range */
        ADDR s = (ADDR)tb->tc.ptr;
        ADDR e = s + code_size;
        qm_flush_icache(s, e);
    }
#endif

#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
    xtm_pf_inc_ts_tb_translated();
#endif

    return code_size;
}

int GPR_USEDEF_TO_SAVE      = 0x7;
int FPR_USEDEF_TO_SAVE      = 0xff;
int XMM_LO_USEDEF_TO_SAVE   = 0xf;
int XMM_HI_USEDEF_TO_SAVE   = 0xf;

/* Paired with tr_fini, it marks a translated code session start. 
 * Every time translation will use a pair of tr_init/tr_fini to 
 * identifying an session. 
 * */
void tr_init(TranslationBlock *tb)
{
    TRANSLATION_DATA *t = lsenv->tr_data;

    if (option_xmm128map){
        XMM_LO_USEDEF_TO_SAVE = 0xff;
        XMM_HI_USEDEF_TO_SAVE = 0xff;
    }

    /* 1. set current tb and ir1 */
    tr_tb_init(t, tb);

    /* 2. register allocation init */
    tr_ra_init(t);

    /* 3. reset all registers extension mode */
    tr_em_init(t);

    /* 4. reset ir2 array */
    tr_ir2_array_init(t);

    /* 5. set up lsfpu */
    tr_fpu_init(t, tb);

#ifndef CONFIG_SOFTMMU
    /* 6. set up flag pattern */
    if (tb != NULL && option_flag_pattern) {
        tr_flag_pattern_init(tb);
    }

    /* delay push/pop esp update : ONLY in user-mode */
    t->curr_esp_need_decrease = 0;
#else
    /* only used in system-mode */
    t->x86_ins_nr = 0;
    t->exitreq_label = ir2_opnd_new_label();
    t->end_with_exception = 0;
#endif
}

void tr_tb_init(TRANSLATION_DATA *t, TranslationBlock *tb)
{
    lsassertm(t->curr_tb == NULL,
              "trying to translate (TB*)%p before (TB*)%p finishes.\n",
              (void*)tb, (void*)(t->curr_tb));
    t->curr_tb = tb;
    t->curr_ir1_inst = NULL;
}

void tr_ir2_array_init(TRANSLATION_DATA *t)
{
#define XTM_IR2_ARRAY_INIT 400
    if (t->ir2_inst_array == NULL) {
        t->ir2_inst_array = mm_calloc(XTM_IR2_ARRAY_INIT, sizeof(IR2_INST));
        t->ir2_inst_num_max = XTM_IR2_ARRAY_INIT;
    }
    t->ir2_inst_num_current = 0;
    t->real_ir2_inst_num = 0;

    t->first_ir2 = NULL;
    t->last_ir2 = NULL;

    t->label_num = 0;
}

void tr_ra_init(TRANSLATION_DATA *t)
{
    t->itemp_num = 32;
    t->ftemp_num = 32;
    t->itemp_mask = 0;
    t->ftemp_mask = 0;
#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
    t->curr_ir1_itemp_num = 0;
    t->curr_ir1_ftemp_num = 0;
#endif
}

void tr_fpu_init(TRANSLATION_DATA *t, TranslationBlock *tb)
{
    if (tb != NULL) {
        if (!option_lsfpu) {
            td_fpu_set_top(etb_get_top_in(tb->extra_tb));
            assert(t->curr_top != -1);
        }
    } else {
        if (!option_lsfpu) td_fpu_set_top(0);
    }
}

#ifndef CONFIG_SOFTMMU
void tr_flag_pattern_init(TranslationBlock *tb)
{
    fp_init();
    tb_find_flag_pattern(tb);
}
#endif

void tr_em_init(TRANSLATION_DATA *t)
{
//    int i = 0;
//
//    for (i = 0; i < 32; ++i) {
//        t->ireg_em[i] = SIGN_EXTENSION;
//        t->ireg_eb[i] = 32;
//    }
//
//    ir2_opnd_set_em(&n1_ir2_opnd, ZERO_EXTENSION, 32);
//    ir2_opnd_set_em(&zero_ir2_opnd, ZERO_EXTENSION, 0);
//
// #ifdef N64 /* validate address */
//    ir2_opnd_set_em(&sp_ir2_opnd, EM_MIPS_ADDRESS, 32);
//    ir2_opnd_set_em(&env_ir2_opnd, EM_MIPS_ADDRESS, 32);
//    IR2_OPND ss_opnd = ra_alloc_ss();
//    ir2_opnd_set_em(&ss_opnd, EM_MIPS_ADDRESS, 32);
//    for (i = 0; i < 7; ++i) {
//        IR2_OPND gpr_opnd = ra_alloc_gpr(i);
//        ir2_opnd_set_em(&gpr_opnd, ir2_opnd_default_em(&gpr_opnd), 32);
//    }
//#endif
//
//    t->hi_em = UNKNOWN_EXTENSION;
//    t->lo_em = UNKNOWN_EXTENSION;
}

void tr_init_translate_ir1(TranslationBlock *tb, int index)
{
    TRANSLATION_DATA *t = lsenv->tr_data;
    IR1_INST *ir1_list = t->ir1_inst_array;
    int ir1_nr = t->ir1_nr;

    lsassert(index >= 0 && ir1_nr >= 0 && index < ir1_nr);

    /* 1. initialize global data */
    IR1_INST *pir1 = ir1_list + index;
    lsenv->tr_data->curr_ir1_inst = pir1;

    /* 2. Append mips_x86_inst into IR2 list to mark
     *    the start of each IR1 */
    append_ir2_opnda(LISA_X86_INST, ir1_addr(pir1));

#ifdef REG_ALLOC_ALG_IMM
    /* 3. clear temp register mapping */
    t->itemp_mask = 0;
    t->ftemp_mask = 0;
    t->itemp_mask_bk = 0;
#endif
#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
    t->curr_ir1_itemp_num = 0;
    t->curr_ir1_ftemp_num = 0;
#endif

    t->curr_ir1_skipped_eflags = 0;
#ifndef CONFIG_SOFTMMU
    /* 4. flag pattern init
     *    curr_ir1_skipped_eflags will be updated */
    fp_init_skipped_flags(pir1);
#endif

    /* 5. validate addres */
//    em_recognize_address(pir1);

    /* 6. adjust extension mode at the last instruction (why?) */
//    if (index == ir1_nr - 1)
//        tr_adjust_em();
}



#ifdef CONFIG_SOFTMMU
int tr_check_buffer_overflow(void *code_start, TRANSLATION_DATA *td)
{
    uint64_t code_highwater = (uint64_t)td->code_highwater;
    uint64_t code_tb_end = (uint64_t)code_start;

    int code_nr = td->ir2_inst_num_current;
    code_nr -= td->label_num; /* mips_label    */
    code_nr -= td->ir1_nr;    /* mips_x86_inst */

    code_tb_end += code_nr << 2;

    if (code_tb_end >= code_highwater)
        return 1;

    return 0;
}
#endif

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_BTMMU)
void btmmu_check_setmem(void *code_addr);

void btmmu_check_setmem(void *code_addr)
{
    lsassertm(0, "BTMMU to be implemented in LoongArch.\n");
//    TRANSLATION_DATA *td = lsenv->tr_data;
//
//    int ir2_curr_id = 0;
//    IR2_INST *ir2_curr = td->first_ir2;
//
//    if (ir2_curr) {
//        ir2_curr_id = ir2_get_id(ir2_curr);
//    }
//
//    uint64_t ir2_code_addr = (uint64_t)code_addr - 4;
//
//    while (ir2_curr != NULL) {
//        if (ir2_opcode(ir2_curr) == LISA_LABEL ||
//            ir2_opcode(ir2_curr) == LISA_X86_INST) {
//            ir2_curr = ir2_next(ir2_curr);
//            if (ir2_curr)
//                ir2_curr_id = ir2_get_id(ir2_curr);
//            continue;
//        }
//
//        ir2_code_addr += 4;
//        if (ir2_opcode(ir2_curr) == mips_setmem) {
//            if ((((uint64_t)(ir2_code_addr) + sizeof(int)) % qemu_host_page_size) == 0) {
//                IR2_INST *p = ir2_allocate();
//                ir2_build(p, mips_nop, NULL, NULL, NULL);
//                ir2_curr = ir2_get(ir2_curr_id);
//                ir2_insert_before(p, ir2_curr);
//                ir2_code_addr += 4;
//            }
//        }
//
//        ir2_curr = ir2_next(ir2_curr);
//        if (ir2_curr)
//            ir2_curr_id = ir2_get_id(ir2_curr);
//    }
}
#endif

/* Translation each IR1 and generate IR2 array */
bool tr_ir2_generate(TranslationBlock *tb)
{
#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
    xtm_pf_tc_tr_translate_st();
#endif
#ifndef CONFIG_SOFTMMU
    /* Profile in user-mode only */
    lsassertm(0, "user-mode profile to be implemented in LoongArch.\n");
//    if (option_profile) {
//        ETB *etb = tb->extra_tb;
//        void *p_execution_times = &etb->_execution_times;
//        IR2_OPND exec_count_addr_opnd = ra_alloc_itemp();
//        load_addr_to_ir2(&exec_count_addr_opnd, (ADDR)p_execution_times);
//        IR2_OPND exec_count_value_opnd = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_ld, &exec_count_value_opnd, &exec_count_addr_opnd, 0);
//        append_ir2_opnd2i(mips_daddiu, &exec_count_value_opnd, &exec_count_value_opnd, 1);
//        append_ir2_opnd2i(mips_sdi, &exec_count_value_opnd, &exec_count_addr_opnd, 0);
//        ra_free_temp(&exec_count_addr_opnd);
//        ra_free_temp(&exec_count_value_opnd);
//    }
#endif

#ifdef CONFIG_SOFTMMU
    tr_gen_tb_start();
#ifdef CONFIG_XTM_LOCKSTEP
    lsassertm(0, "lock step to be implemented in LoongArch.\n");
//    /* 2.0 save context */
//    helper_cfg_t cfg = default_helper_cfg;
//    tr_sys_gen_call_to_helper_prologue_cfg(cfg);
//    /* 2.1 arg0: env */
//    append_ir2_opnd3(mips_or, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
//    /* 2.2 call helper_invlpg */
//    tr_gen_call_to_helper((ADDR)vcpu_tb_exec);
//    /* 2.3 restore context */
//    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
#endif
#endif

    if (option_dump)
        fprintf(stderr, "[X86toMIPS] translation : generate IR2 from IR1.\n");

    int i = 0;

    TRANSLATION_DATA *t = lsenv->tr_data;
    IR1_INST *ir1_list = t->ir1_inst_array;
    int ir1_nr = t->ir1_nr;

    IR1_INST *pir1 = NULL;
    for (i = 0; i < ir1_nr; ++i) {
        pir1 = ir1_list + i;

        option_trace_ir1(pir1);

        tr_init_translate_ir1(tb, i);

#ifdef CONFIG_SOFTMMU
        TRANSLATION_DATA *td = lsenv->tr_data;
        if ((td->max_insns == i + 1)
            && (td->sys.cflags & CF_LAST_IO))
            tr_gen_io_start();
#endif
	
        bool translation_success = ir1_translate(pir1);
        if (option_dump_ir1) {
            fprintf(stderr, "ir1 translate >>> ");
            ir1_dump(pir1);
        }

        lsassertm(translation_success,
                "translate failed for inst %s\n",
                pir1->info->mnemonic);
        
#ifdef CONFIG_SOFTMMU
        if (t->end_with_exception) {
            /* 1. get real number of IR1 */
            int real_ir1_nr = i + 1;
            t->ir1_nr = real_ir1_nr;
            /* 2. Reset related fields in ETB */
            for (++i; i < ir1_nr; ++i) {
                pir1 = ir1_list + i;
                ir1_free_info(pir1);
            }
            tb->extra_tb->_ir1_num = real_ir1_nr;
            /* 3. ICOUNT: decrease real number of IR1 */
            if (atomic_read(&tb->cflags) & CF_USE_ICOUNT) {
                IR2_INST *pir2 = ir2_get(td->dec_icount_inst_id);
                pir2->_opnd[2] = ir2_opnd_new(IR2_OPND_IMMH,
                        0 - real_ir1_nr);
            }
            /* 4. dummay operation to avoid assert */
//            tr_adjust_em();
            break;
        }
#endif
    }

#ifdef CONFIG_SOFTMMU
    if (!(t->end_with_exception)) {
        tr_gen_eob_if_tb_too_large(tb->extra_tb->tb_too_large_pir1);
        tr_gen_sys_eob(tb->extra_tb->sys_eob_pir1);
    }
    tr_gen_tb_end();
    tr_gen_softmmu_slow_path();
#else
    tr_gen_eob_if_tb_too_large(tb->extra_tb->tb_too_large_pir1);
#endif

    if (option_dump_ir2) {
        fprintf(stderr, "IR2 num = %d\n", lsenv->tr_data->ir2_inst_num_current);
        IR2_INST *pir2 = t->first_ir2;
        while(pir2) {
            ir2_dump(pir2);
            pir2 = ir2_next(pir2);
        }
    }

    if (!option_lsfpu) {
        etb_check_top_out(tb->extra_tb, td_fpu_get_top());
    }

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
    xtm_pf_tc_tr_translate_ed();
#endif
    return true;
}


/* paired with tr_init, it marks a translated code session end */
void tr_fini(bool check_the_extension)
{
    TRANSLATION_DATA *t = lsenv->tr_data;
//    int i = 0;

    /* 1. Just check out gpr extension mode */
//    if (check_the_extension) {
//        for (i = 0; i < 8; ++i) {
//            IR2_OPND gpr_opnd = ra_alloc_gpr(i);
//            EXTENSION_MODE default_em = ir2_opnd_default_em(&gpr_opnd);
//            if (default_em == SIGN_EXTENSION) {
//                lsassertm(ir2_opnd_is_sx(&gpr_opnd, 32), "%d EM wrong \n", i);
//            } else if (default_em == ZERO_EXTENSION ||
//                       default_em == EM_X86_ADDRESS) {
//                lsassertm(ir2_opnd_is_zx(&gpr_opnd, 32), "%d EM wrong \n", i);
//            } else {
//                lsassert(0);
//            }
//        }
//    }

    /* 2. reset flags in TRANSLATION_DATA uesd by translation */
    t->curr_tb = NULL;
    t->curr_ir1_inst = NULL;

    t->ir2_inst_num_current = 0;
    t->real_ir2_inst_num = 0;

    t->first_ir2 = NULL;
    t->last_ir2 = NULL;

    t->label_num = 0;
    t->itemp_num = 32;
    t->ftemp_num = 32;

    if (!option_lsfpu) td_fpu_set_top(0);
}

int tr_ir2_assemble(void *code_start_addr)
{
#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
    xtm_pf_tc_tr_assemble_st();
#endif
    if (option_dump) {
        fprintf(stderr, "[X86toMIPS] assemble    : generate host from IR2.\n");
    }
#ifdef REG_ALLOC_ALG_AFT
    /* 1. assign temp register to physical register */
    ra_temp_register_allocation();
#endif
#if defined(CONFIG_SOFTMMU) && defined(CONFIG_BTMMU)
    /* Check if setmem locates at page edge */
    btmmu_check_setmem(code_start_addr);
#endif
    /* 2. label dispose */
    label_dispose(code_start_addr);
    /* 3. assemble */
    IR2_INST *pir2 = lsenv->tr_data->first_ir2;
    void *code_addr = code_start_addr;
    int code_nr = 0;
#ifdef CONFIG_SOFTMMU
    TRANSLATION_DATA *td = lsenv->tr_data;
    /* Buffer Overflow Check */
    if (td->curr_tb) {
        /* Do not check buffer overflow when generating
         * context switch (prologue/epilogue) */
        if (tr_check_buffer_overflow(code_start_addr, td)) {
            return -1;
        }
    }
    /* Counter for encode search */
    int x86ins_idx  = 0;
    int x86ins_nr   = -1;
    int x86ins_mcnt = 0;
    int x86ins_size = 0;
#endif
    while (pir2 != NULL) {
        IR2_OPCODE ir2_opc = ir2_opcode(pir2);
        if (ir2_opc == LISA_X86_INST) {
#ifdef CONFIG_SOFTMMU
            if (x86ins_nr >= 0) {
                td->x86_ins_idx[x86ins_nr] = x86ins_idx;
                td->x86_ins_mips_nr[x86ins_nr] = x86ins_mcnt;
            }
            x86ins_idx = ir2_get_id(pir2);
            x86ins_nr += 1;
            /* calculate TB size */
            IR1_INST *pir1 = &td->ir1_inst_array[x86ins_nr];
            x86ins_size += ir1_inst_size(pir1);
#endif
            goto _NEXT_IR2_;
        }
        if (ir2_opc != LISA_LABEL) {
            uint32 result = ir2_assemble(pir2);
            if (option_dump_host) {
                fprintf(stderr, "IR2 at %p LISA Binary = 0x%08x ", code_addr,
                        result);
                ir2_dump(pir2);
            }

            *(uint32 *)code_addr = result;
            code_addr = code_addr + 4;
            code_nr += 1;
#ifdef CONFIG_SOFTMMU
            x86ins_mcnt += 1;
#endif
        }
_NEXT_IR2_:
        pir2 = ir2_next(pir2);
    }
#ifdef CONFIG_SOFTMMU
    if (td->curr_tb) {
        td->x86_ins_idx[x86ins_nr] = x86ins_idx;
        td->x86_ins_mips_nr[x86ins_nr] = x86ins_mcnt;
        td->x86_ins_nr = x86ins_nr + 1;
        td->x86_ins_size = x86ins_size;

        lsassertm(td->x86_ins_nr == td->ir1_nr,
                "x86 inst %d not equal to IR1 %d.\n",
                td->x86_ins_nr, td->ir1_nr);

        td->curr_tb->icount = td->x86_ins_nr;
        td->curr_tb->size = x86ins_size;
    
        if (option_dump) {
            int i;;
            ADDRX x86_addr;
            fprintf(stderr, "X86_ins_nr = %d\n", td->x86_ins_nr);
            for ( i = 0; i < td->x86_ins_nr; ++i ) {
                x86_addr = ir1_addr(td->ir1_inst_array + i);
                fprintf(stderr, "X86[%3d][%p] mips = %d\n",
                        i,
                        (void*)(ADDR)x86_addr,
                        td->x86_ins_mips_nr[i]);
            }
        }
    }
#ifdef CONFIG_XTM_PROFILE
    xtm_pf_tc_tr_assemble_ed();
#endif
#endif
    return code_nr;
}

void label_dispose(void *code_cache_addr)
{
    /* 1. record the positions of label */
    int *label_num_to_ir2_num =
        (int *)alloca(lsenv->tr_data->label_num * 4 + 20);
    memset(label_num_to_ir2_num, -1, lsenv->tr_data->label_num * 4 + 20);
    int ir2_num = 0;
    IR2_INST *ir2_current = lsenv->tr_data->first_ir2;
    while (ir2_current != NULL) {
        if (ir2_current->_opcode == LISA_LABEL) {
            int label_num = ir2_current->_opnd[0].val;
            lsassertm(label_num_to_ir2_num[label_num] == -1,
                      "label %d is in multiple positions\n", label_num);
            label_num_to_ir2_num[label_num] = ir2_num;
//            fprintf(stderr, "[Label Dispose] label[%d] = %d\n", label_num, ir2_num);
        } else if (ir2_current->_opcode == LISA_X86_INST) {
            /* will not be assembled */
        } else {
            ir2_num++;
        }
        ir2_current = ir2_next(ir2_current);
    }
    /* 2. resolve the offset of successor linkage code */
    /* @jmp_target_arg recoed the jmp  inst position */
    /* @jmp_reset_offset record the successor inst of jmp(exclude delay slot). */
    /*                   when the tb is removed from buffer. the jmp inst use */
    /*                   this position to revert the original "fall through". */
    {
        TranslationBlock *tb = lsenv->tr_data->curr_tb;
        /* prologue/epilogue has no tb */
        if (tb) {
            /* ctx->jmp_insn_offset point to tb->jmp_target_arg */
            int label_id_0 = tb->jmp_reset_offset[0];
            if (label_id_0 != TB_JMP_RESET_OFFSET_INVALID) {
                tb->jmp_reset_offset[0] =
                    (label_num_to_ir2_num[label_id_0] << 2) + 8;
                tb->jmp_target_arg[0] = (label_num_to_ir2_num[label_id_0] << 2);
            }
            int label_id_1 = tb->jmp_reset_offset[1];
            if (label_id_1 != TB_JMP_RESET_OFFSET_INVALID) {
                tb->jmp_reset_offset[1] =
                    (label_num_to_ir2_num[label_id_1] << 2) + 8;
                tb->jmp_target_arg[1] = (label_num_to_ir2_num[label_id_1] << 2);
            }

            if (xtm_branch_opt() && tb->extra_tb->end_with_jcc) {
                tb->extra_tb->mips_branch_inst_offset = (label_num_to_ir2_num[tb->extra_tb->mips_branch_inst_offset] << 2);
            }
        }
    }
    /* 3. resolve the branch instructions */
    ir2_num = 0;
    ir2_current = lsenv->tr_data->first_ir2;
    while (ir2_current != NULL) {
        IR2_OPCODE opcode = ir2_current->_opcode;
        if (ir2_opcode_is_branch(opcode)) {

            IR2_OPND *label_opnd = ir2_branch_get_label(ir2_current);

            /* LISA_B and LISA_BL could contain label or imm */
            if (label_opnd && ir2_opnd_is_label(label_opnd)) {
                int label_num = ir2_opnd_label_id(label_opnd);
                lsassert(label_num > 0 && label_num <= lsenv->tr_data->label_num);
                int target_ir2_num = label_num_to_ir2_num[label_num];

                lsassertm(target_ir2_num != -1,
                        "label %d is not inserted\n",
                         label_num);

//                fprintf(stderr, "[Label Dispose] branch to label[%d] offset = %d\n",
//                        label_num, target_ir2_num - ir2_num);
                ir2_opnd_convert_label_to_imm(label_opnd,
                                              target_ir2_num - ir2_num);
            }
        }

        if (ir2_current->_opcode != LISA_LABEL &&
            ir2_current->_opcode != LISA_X86_INST ) {
            ir2_num++;
        }

        ir2_current = ir2_next(ir2_current);
    }
}
