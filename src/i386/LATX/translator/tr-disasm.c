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


#include <string.h>

/* Function inside X86toMIPS to read target's code */
uint8_t cpu_read_code_via_qemu(CPUX86State *env, ADDRX pc)
{
#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
    xtm_pf_tc_tr_disasm_readbytes_st();
    uint8_t byte = cpu_ldub_code(env, (target_ulong)pc);
    xtm_pf_tc_tr_disasm_readbytes_ed();
    return byte;
#else
    return cpu_ldub_code(env, (target_ulong)pc);
#endif
}

/* from target/i386/translate.c */
#define X86_MAX_INSN_LENGTH 15

/* Disassemble one instruction according to target' pc.
 * IR1_INST will be filled in with the disasm result.
 *
 * @pir1: IR1_INST to be filled
 * @pc: target pc = cs_base + eip
 *
 * #PF: read code across pages
 * #ILLOP: disasm fail
 */
static ADDRX __disasm_one_ir1(IR1_INST *pir1, ADDRX pc)
{
    lsassert(pir1 != NULL);

    CPUX86State *env = lsenv->cpu_state;

    static uint8_t inst_cache[64];
    uint8_t  *pins = inst_cache;
    pins = inst_cache;

    ADDRX next_pc; /* Next PC after current instruction */

    ADDRX pc_page = pc & TARGET_PAGE_MASK;
    ADDRX pc_read_code;
    int bytes_left_st = 0;

    int inst_size    = 8; /* 8, 16, */
    int inst_st      = 0; /* 0, 4, 8,  */

    int i = 0;
    int error = 0;

    do {
        /* We do not allow instruction to across page at the first time.
         *
         * If disasm fail, it means this instruction IS across page, so
         * we need to read the bytes left on the last time to read.
         *
         * eg.
         * ----------------------------------------------------------------
         * <1> byte [0] [1] [2] [3] inst_st = 0, inst_size = 4
         *                   ^
         *                    \ byte[2] is in another page
         *                                    byte_left_st = 2
         * ----------------------------------------------------------------
         * <2> use byte [0] [1] to disasm
         * ----------------------------------------------------------------
         * <3> If success, then disasm finish
         *     > The disasm result instruction size should not be greater
         *       than 2 (byte_left_st).
         *     If fail, then inst_st   = 0, go to next around to read codes
         *  +<-------------  inst_size = 4
         *  |           > if inst_st is greater than MAX, generate a
         *  |             general protection fault of instruction too long.
         *  |             > If the 16th byte is in another page, we should
         *  v               generate #PF before #GP. TODO
         * ----------------------------------------------------------------
         * <4> byte_left_st != 0, we need to continue to read byte [2] [3],
         *     NOT byte [4] [5] [6] [7]
         * ----------------------------------------------------------------
         */
        if (bytes_left_st) {
            pins = &inst_cache[bytes_left_st];
            for (i = bytes_left_st; i < inst_size; ++i) {
                pc_read_code = pc + i;
                *pins = cpu_read_code_via_qemu(env, pc_read_code);
                pins++;
            }
            bytes_left_st = 0;
        }
        else {
            /* 1. read [ 0,  3] st = 0, size = 4
             * 2. read [ 4,  7] st = 4, size = 8
             * 3. read [ 8, 16] st = 8, size = 16 */
            for (i = inst_st; i < inst_size; ++i) {
                pc_read_code = pc + i;
                if ((pc_read_code & TARGET_PAGE_MASK) != pc_page) {
                    pc_page = pc_read_code & TARGET_PAGE_MASK;
                    bytes_left_st = i;
                    break;
                }
                *pins = cpu_read_code_via_qemu(env, pc_read_code);
                pins++;
            }
        }

        error = 0;
        next_pc = ir1_disasm(pir1, inst_cache, pc, &error);

        int inst_real_size = inst_size;
        if (bytes_left_st) {
            inst_real_size = bytes_left_st;
        }

        if (error || ir1_inst_size(pir1) > inst_real_size) {
            ir1_free_info(pir1);
            if (!bytes_left_st) {
                /* Go to next reading round if no across page */
                inst_st   = inst_size;      /* 4, 8,  */
                inst_size = inst_size << 1; /* 8, 16, */
                if (inst_st > X86_MAX_INSN_LENGTH) {
                    /* > If the 16th byte is in another page, we should
                     *   generate #PF before #GP. TODO */
                    error = 1;
                    break;
                }
            }
        }
        else {
            error = 0;
            break;
        }

    } while(1);

    /* Cannot disasm this instruction */
    if (error) {
#ifndef CONFIG_SOFTMMU
        fprintf(stderr, "ERROR : disasm, ADDR : 0x%" PRIx64 "\n", (uint64_t)pc);
        fprintf(stderr, "bytes: ");
        for (i = 0; i < inst_size - 1; ++i) {
            fprintf(stderr, "%#x ", inst_cache[i & 0x3f]);
        }
        fprintf(stderr, "%#x\n", inst_cache[i & 0x3f]);
        exit(-1);
#else
        if (inst_cache[0] == 0xf3 &&
            inst_cache[1] == 0x0f &&
            inst_cache[2] == 0x1e &&
            inst_cache[3] == 0xfb) {
            ir1_make_ins_ILLEGAL(pir1, pc, 4, IR1_FLAGS_GENNOP);
            next_pc = pc + 4;
        } else {
            ir1_make_ins_ILLEGAL(pir1, pc, 1, IR1_FLAGS_ILLOP);
        }
#endif
    }

    return next_pc;
}

void tr_disasm(TranslationBlock *tb)
{
#ifndef CONFIG_SOFTMMU
    ADDRX pc = (ADDRX)tb->pc;
    ETB *etb = etb_cache_find(pc, true);
    /* update profile info */
    etb_array[tb_num++] = etb;
    lsassert(tb_num < ETB_ARRAY_SIZE);
    /* get ir1 instructions */
    IR1_INST *ir1_list = etb->_ir1_instructions;
    int ir1_num = etb->_ir1_num;
    /* ir1 hasn't been disasmbled, get it */
    if (etb->_ir1_instructions == NULL) {
        ir1_list = get_ir1_list(etb, pc, &ir1_num);

        etb->_ir1_instructions = ir1_list;
        etb->_ir1_num = ir1_num;
        etb->_tb_type = get_etb_type(ir1_list + ir1_num - 1);
    }
#else
#ifdef CONFIG_XTM_PROFILE
    xtm_pf_tc_tr_disasm_st();
#endif

    ETB *etb = etb_alloc();

    ADDRX pc = (ADDRX)tb->pc;
    etb->pc  = pc;

    int ir1_num = 0;
    IR1_INST *ir1_list = get_ir1_list(etb, pc, &ir1_num);
    etb->_ir1_instructions = ir1_list;
    etb->_ir1_num = ir1_num;
#endif

    lsenv->tr_data->ir1_inst_array = ir1_list;
    lsenv->tr_data->ir1_nr = ir1_num;
    lsenv->tr_data->curr_ir1_inst = NULL;
#ifndef CONFIG_SOFTMMU
    if (option_flag_reduction) {
        etb_add_succ(etb, 2);
        etb->flags |= SUCC_IS_SET_MASK;
    }
#endif
    tb->extra_tb = etb;

    counter_ir1_tr += ir1_num;
#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
    xtm_pf_tc_tr_disasm_ed();
#endif
}

IR1_INST *get_ir1_list(ETB* etb, ADDRX pc, int *p_ir1_num)
{

#ifdef CONFIG_SOFTMMU
#if defined(CONFIG_XTM_PROFILE) || defined(CONFIG_XTM_FAST_CS)
    etb->fast_cs_mask = XTM_FAST_CS_MASK_ZERO;
#endif
#endif

    IR1_INST *ir1_list = (IR1_INST *)mm_calloc(
            MAX_IR1_NUM_PER_TB + 1, sizeof(IR1_INST));

    IR1_INST *pir1 = ir1_list;

    int ir1_num = 0;
    do {
        lsassert(lsenv->cpu_state != NULL);
        /* disasemble this instruction */
        pir1 = &ir1_list[ir1_num];
        pc = __disasm_one_ir1(pir1, pc);
        ir1_num++;

#ifdef CONFIG_SOFTMMU
        if (ir1_is_illegal(pir1) &&
            pir1->flags != IR1_FLAGS_GENNOP) break;
#if defined(CONFIG_XTM_PROFILE) || defined(CONFIG_XTM_FAST_CS)
        etb_pref_cs_mask(etb, pir1);
#endif
#endif

        /* check if TB is too large */
        if (
#ifndef CONFIG_SOFTMMU
        ir1_num == MAX_IR1_NUM_PER_TB &&
#else
        ir1_num == lsenv->tr_data->max_insns &&
#endif
                !ir1_is_tb_ending(pir1))
        {
            etb->tb_too_large_pir1 = pir1;
            break;
        }

#ifdef CONFIG_SOFTMMU
        /* special end of block in system-mode
         * > special system instructions
         * > system eob: inhibit irq, icount, cross page
         * > breakpoint */
        if (ir1_is_eob_in_sys(pir1)) {
            etb->sys_eob_pir1 = pir1;
            break;
        }
#endif

    } while (!ir1_is_tb_ending(pir1));

    // ir1_list = (IR1_INST *)mm_realloc(ir1_list, ir1_num * sizeof(IR1_INST));
    *p_ir1_num = ir1_num;
    return ir1_list;

}
