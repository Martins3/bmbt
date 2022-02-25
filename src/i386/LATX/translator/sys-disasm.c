#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "ir1.h"
#include "ir2.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>

static __thread char latxs_insn_info[MAX_IR1_NUM_PER_TB * IR1_INST_SIZE] = {0};
static QemuMutex cs_lock;

static void __attribute__((__constructor__)) latxs_cs_lock_init(void)
{
    qemu_mutex_init(&cs_lock);
}
ADDRX latxs_ir1_disasm(IR1_INST *ir1,
        uint8_t *addr, ADDRX t_pc, int *error, int ir1_num)
{
    qemu_mutex_lock(&cs_lock);
    TRANSLATION_DATA *td = lsenv->tr_data;
    ADDRX t_eip = t_pc - td->sys.cs_base; /* EIP = PC - CS_BASE */
    int count = 0;
    cs_insn *info;

    void *pir1_base = latxs_insn_info;
#ifdef TARGET_X86_64
    if (td->sys.code64) {
        count = cs_disasm(handle64, addr, 15, (uint64_t)t_eip, 1,
                          &info, ir1_num, pir1_base);
    } else
#endif
    if (td->sys.code32) {
        count = cs_disasm(handle, addr, 15, (uint64_t)t_eip, 1,
                          &info, ir1_num, pir1_base);
    } else {
        count = cs_disasm(handle16, addr, 15, (uint64_t)t_eip, 1,
                          &info, ir1_num, pir1_base);
    }

    qemu_mutex_unlock(&cs_lock);
    ir1->info = info;
    ir1->info_count = count;

    /* Disasm error */
    if (count != 1) {
        *error = 1;
        return 0;
    }

    ir1->flags = 0;
    ir1->_eflag_def = 0;
    ir1->_eflag_use = 0;
    ir1->_native_inst_num = 0;

    latxs_fix_up_ir1(ir1);
    t_pc = ir1_addr(ir1) + td->sys.cs_base;
    return (ADDRX)(t_pc + ir1->info->size);
}

/* from target/i386/translate.c */
#define LATXS_X86_MAX_INSN_LENGTH 15

/* Function inside X86toMIPS to read target's code */
uint8_t latxs_cpu_read_code_via_qemu(CPUX86State *env, ADDRX pc)
{
    return cpu_ldub_code(env, (target_ulong)pc);
}

static inline int ir1_is_rdsspd(uint8_t *inst_cache)
{
    if (inst_cache[0] == 0xf3 && inst_cache[1] == 0x0f &&
        inst_cache[2] == 0x1e && ((inst_cache[3] & 0xf8) == 0xc8)) {
        return 4;
    } else {
        return 0;
    }
}

static inline int ir1_is_rdssp_rex(uint8_t *inst_cache)
{
    if (inst_cache[0] == 0xf3 && ((inst_cache[1] & 0xf0) == 0x40) &&
        inst_cache[2] == 0x0f && inst_cache[3] == 0x1e &&
        (((inst_cache[4] & 0xf8)) == 0xc8)) {
        return 5;
    } else {
        return 0;
    }
}

static void __latxs_disasm_do_error_sys(IR1_INST *pir1,
        uint8_t *inst_cache, ADDRX pc, ADDRX *next_pc)
{
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64 && ir1_is_rdssp_rex(inst_cache)) {
        latxs_ir1_make_ins_ILLEGAL(pir1, pc, 5, LATXS_IR1_FLAGS_GENNOP);
        *next_pc = pc + 5;
    } else if (ir1_is_rdsspd(inst_cache)) {
        latxs_ir1_make_ins_ILLEGAL(pir1, pc, 4, LATXS_IR1_FLAGS_GENNOP);
        *next_pc = pc + 4;
    } else
#else
    if (ir1_is_rdsspd(inst_cache)) {
        latxs_ir1_make_ins_ILLEGAL(pir1, pc, 4, LATXS_IR1_FLAGS_GENNOP);
        *next_pc = pc + 4;
    } else
#endif
    if (inst_cache[0] == 0xf3 && inst_cache[1] == 0x0f &&
            inst_cache[2] == 0x1e && inst_cache[3] == 0xfb) {
        /* endbr32 */
        latxs_ir1_make_ins_ILLEGAL(pir1, pc, 4, LATXS_IR1_FLAGS_GENNOP);
        *next_pc = pc + 4;
    } else {
        /* next_pc has no meaning if illegal instruction is detected */
        latxs_ir1_make_ins_ILLEGAL(pir1, pc, 1, LATXS_IR1_FLAGS_ILLOP);
    }
}

/* Success, return 0 */
static int __latxs_disasm_ont_ir1_directly(IR1_INST *pir1,
        ADDRX pc, ADDRX *next_pc, int ir1_num)
{
    CPUX86State *env = lsenv->cpu_state;
    ADDR hva = 0;

    int error = 0;
    hva = latxs_tr_cvt_cpu_addr_code(env, pc, &error);

    if (error) {
        return 1;
    }

    error = 0;
    *next_pc = latxs_ir1_disasm(pir1, (void *)hva, pc, &error, ir1_num);

    if (error) {
        __latxs_disasm_do_error_sys(pir1, (void *)hva, pc, next_pc);
    }

    return 0;
}

/*
 * Disassemble one instruction according to target' pc.
 * IR1_INST will be filled in with the disasm result.
 *
 * @pir1: IR1_INST to be filled
 * @pc: target pc = cs_base + eip
 *
 * #PF: read code across pages
 * #ILLOP: disasm fail
 */
static ADDRX __latxs_disasm_one_ir1(IR1_INST *pir1, ADDRX pc, int ir1_num)
{
    lsassert(pir1 != NULL);

    CPUX86State *env = lsenv->cpu_state;

    uint8_t inst_cache[64];
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

    if (pc_page  == ((pc + 16) & TARGET_PAGE_MASK)) {
        /*
         * This instruction must be in the same page,
         * we can use HVA to read its bytes dirtectly
         */
        if (!__latxs_disasm_ont_ir1_directly(pir1, pc, &next_pc, ir1_num)) {
            return next_pc;
        }
    }

    do {
        /*
         * We do not allow instruction to across page at the first time.
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
                *pins = latxs_cpu_read_code_via_qemu(env, pc_read_code);
                pins++;
            }
            bytes_left_st = 0;
        } else {
            /*
             * 1. read [ 0,  3] st = 0, size = 4
             * 2. read [ 4,  7] st = 4, size = 8
             * 3. read [ 8, 16] st = 8, size = 16
             */
            for (i = inst_st; i < inst_size; ++i) {
                pc_read_code = pc + i;
                if ((pc_read_code & TARGET_PAGE_MASK) != pc_page) {
                    pc_page = pc_read_code & TARGET_PAGE_MASK;
                    bytes_left_st = i;
                    break;
                }
                *pins = latxs_cpu_read_code_via_qemu(env, pc_read_code);
                pins++;
            }
        }

        error = 0;
        next_pc = latxs_ir1_disasm(pir1, inst_cache, pc, &error, ir1_num);

        int inst_real_size = inst_size;
        if (bytes_left_st) {
            inst_real_size = bytes_left_st;
        }

        if (error || latxs_ir1_inst_size(pir1) > inst_real_size) {
            latxs_ir1_free_info(pir1);
            if (!bytes_left_st) {
                /* Go to next reading round if no across page */
                inst_st   = inst_size;      /* 4, 8,  */
                inst_size = inst_size << 1; /* 8, 16, */
                if (inst_st > LATXS_X86_MAX_INSN_LENGTH) {
                    /*
                     * > If the 16th byte is in another page, we should
                     *   generate #PF before #GP. TODO
                     */
                    error = 1;
                    break;
                }
            }
        } else {
            error = 0;
            break;
        }

    } while (1);

    /* Cannot disasm this instruction */
    if (error) {
        __latxs_disasm_do_error_sys(pir1, inst_cache, pc, &next_pc);
    }

    return next_pc;
}

void latxs_tr_disasm(TranslationBlock *tb)
{
    ADDRX pc = (ADDRX)tb->pc;

    int ir1_num = 0;
    IR1_INST *ir1_list = latxs_get_ir1_list(tb, pc, &ir1_num);

    TRANSLATION_DATA *td = lsenv->tr_data;

    td->ir1_inst_array = ir1_list;
    td->ir1_nr = ir1_num;
    td->curr_ir1_inst = NULL;

    tb->_ir1_instructions = ir1_list;
    tb->icount = ir1_num;
}

IR1_INST *latxs_get_ir1_list(TranslationBlock *tb, ADDRX pc, int *p_ir1_num)
{
    lsassert(lsenv->cpu_state != NULL);

    IR1_INST *ir1_list = mm_calloc(MAX_IR1_NUM_PER_TB + 1, sizeof(IR1_INST));

    IR1_INST *pir1 = ir1_list;
    int ir1_num = 0;

    do {
        /* disasemble this instruction */
        pir1 = &ir1_list[ir1_num];
        pc = __latxs_disasm_one_ir1(pir1, pc, ir1_num);
        ir1_num++;

        if (latxs_ir1_is_illegal(pir1) &&
                pir1->flags != LATXS_IR1_FLAGS_GENNOP) {
            break;
        }

        /*
         * special end of block in system-mode
         * > special system instructions
         * > system eob: inhibit irq, icount, cross page
         * > breakpoint
         */
        if (latxs_ir1_is_eob_in_sys(pir1)) {
            tb->sys_eob_pir1 = pir1;
            break;
        }

        /* check if TB is too large */
        if (ir1_num == lsenv->tr_data->max_insns &&
                !ir1_is_tb_ending(pir1)) {
            tb->tb_too_large_pir1 = pir1;
            break;
        }

    } while (!ir1_is_tb_ending(pir1));

    ir1_list = mm_realloc(ir1_list, ir1_num * sizeof(IR1_INST));
    *p_ir1_num = ir1_num;
    return ir1_list;
}
