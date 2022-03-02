#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>

static int latxs_is_system_eob(IR1_INST *ir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    ADDRX pc_first = td->sys.pc;
    ADDRX pc_next  = ir1_addr_next(ir1) + td->sys.cs_base;

    if (td->sys.tf || (td->sys.flags & HF_INHIBIT_IRQ_MASK)) {
        return 1;
    } else if (td->sys.cflags & CF_USE_ICOUNT) {
        if (((pc_next & TARGET_PAGE_MASK)
                   != ((pc_next + 15)
                       & TARGET_PAGE_MASK)
                   || (pc_next & ~TARGET_PAGE_MASK) == 0)) {
            return 1;
        } else {
            /* io instructions is eob in icount mode */
            switch (ir1_opcode(ir1)) {
            case X86_INS_RDTSC:
            case X86_INS_RDTSCP:

            case X86_INS_OUTSB:
            case X86_INS_OUTSD:
            case X86_INS_OUTSW:

            case X86_INS_INSB:
            case X86_INS_INSD:
            case X86_INS_INSW:

            case X86_INS_IN:
            case X86_INS_OUT:
                return 1;
            default:
                return 0;
            }
        }
    } else if ((pc_next - pc_first) >= (TARGET_PAGE_SIZE - 32)) {
        return 1;
    }

    return 0;
}

static int latxs_ir1_is_hit_breakpoint(IR1_INST *ir1)
{
    CPUState *cpu = current_cpu;
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (current_cpu->singlestep_enabled) {
        return 1;
    }

    target_ulong pc = (target_ulong)(ir1_addr(ir1) + td->sys.cs_base);

    if (unlikely(!QTAILQ_EMPTY(&cpu->breakpoints))) {
        CPUBreakpoint *bp;
        QTAILQ_FOREACH(bp, &cpu->breakpoints, entry) {
            if (bp->pc == pc) {
                td->sys.bp_hit = 1;
                return 1;
            }
        }
    }

    if (cpu->watchpoint_hit) {
        td->sys.bp_hit = 1;
        return 1;
    }

    return 0;
}

static int latxs_ir1_is_lxx_seg(IR1_INST *ir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    switch (ir1_opcode(ir1)) {
    case X86_INS_LDS:
    case X86_INS_LES:
        if (td->sys.pe && !td->sys.vm86 && td->sys.code32) {
            return 1;
        } else {
            return 0;
        }
    case X86_INS_LSS:
        return 1;
    case X86_INS_LFS:
    case X86_INS_LGS:
    default:
        return 0;
    }
}

static int latxs_ir1_is_pop_seg_eob(IR1_INST *ir1)
{
    if (ir1_opcode(ir1) != X86_INS_POP) {
        return 0;
    }

    IR1_OPND *opnd0 = ir1_get_opnd(ir1, 0);
    if (!ir1_opnd_is_seg(opnd0)) {
        return 0;
    }

    TRANSLATION_DATA *td = lsenv->tr_data;
    int seg_num = ir1_opnd_base_reg_num(opnd0);

    switch (seg_num) {
    case ds_index:
    case es_index:
        if (td->sys.pe && !td->sys.vm86 && td->sys.code32) {
            return 1;
        } else {
            return 0;
        }
    case ss_index:
        return 1;
    case fs_index:
    case gs_index:
    default:
        return 0;
    }
}

int latxs_ir1_is_eob_in_sys(IR1_INST *ir1)
{
    if (lsenv->tr_data->sys.cflags & CF_USE_ICOUNT) {
        if (latxs_ir1_is_mov_from_cr(ir1)) {
            return true;
        }
    }

    return latxs_ir1_is_mov_to_cr(ir1) ||         /* mov cr, src */
           latxs_ir1_is_mov_to_dr(ir1) ||         /* mov dr, src */
           latxs_ir1_is_pop_eflags(ir1) ||        /* popf */
           latxs_ir1_is_sysenter(ir1) ||          /* sysenter */
           latxs_ir1_is_sysexit(ir1) ||           /* sysexit */
           latxs_ir1_is_syscall(ir1) ||           /* syscall */
           latxs_ir1_is_sysret(ir1) ||            /* sysret */
           latxs_ir1_is_rsm(ir1) ||               /* rsm */
           latxs_ir1_is_sti(ir1) ||               /* sti */
           latxs_ir1_is_mov_to_seg_eob(ir1) ||    /* mov es/cs/ss/ds, src */
           latxs_ir1_is_pop_seg_eob(ir1) ||       /* pop ds/es/ss, src */
           latxs_ir1_is_repz_nop(ir1) ||          /* pause */
           latxs_ir1_is_pause(ir1) ||             /* pause */
           latxs_ir1_is_iret(ir1) ||              /* iret */
           latxs_ir1_is_lmsw(ir1) ||              /* lmsw */
           latxs_ir1_is_retf(ir1) ||              /* ret far */
           latxs_ir1_is_call_far(ir1) ||          /* call far */
           latxs_ir1_is_clts(ir1) ||              /* clts */
           latxs_ir1_is_invlpg(ir1) ||            /* invlpg */
           latxs_ir1_is_lxx_seg(ir1) ||           /* lss, lds/les */
           /* fldenv, frstor, fxrstor, xrstor */
           latxs_ir1_contains_fldenv(ir1) ||
           /* fninit: reset all to zero */
           latxs_ir1_is_fninit(ir1) ||
           /* fnsave: reset all to zero after save */
           latxs_ir1_is_fnsave(ir1) ||
           latxs_ir1_is_xsetbv(ir1) ||            /* xsetbv */
           latxs_ir1_is_mwait(ir1) ||             /* mwait */
           latxs_ir1_is_vmrun(ir1) ||             /* vmrun */
           latxs_ir1_is_stgi(ir1) ||              /* stgi */
           latxs_is_system_eob(ir1) ||
           latxs_ir1_is_hit_breakpoint(ir1);
}

int latxs_ir1_is_mov_to_seg_eob(IR1_INST *ir1)
{
    if (ir1->info->id != X86_INS_MOV) {
        return 0;
    }

    IR1_OPND *opnd0 = ir1_get_opnd(ir1, 0);
    if (!ir1_opnd_is_seg(opnd0)) {
        return 0;
    }

    TRANSLATION_DATA *td = lsenv->tr_data;
    int seg_index = ir1_opnd_base_reg_num(opnd0);

    if (td->sys.pe && !td->sys.vm86) {
        if (seg_index == ss_index ||
           (td->sys.code32 && seg_index < fs_index)) {
            return 1;
        }
    } else{
        if (seg_index == ss_index) {
            return 1;
        }
    }

    return 0;
}

static void latxs_tr_gen_hflags_set(uint32_t mask)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (td->sys.flags & mask) {
        return;
    }

    IR2_OPND mask_reg   = latxs_ra_alloc_itemp();
    IR2_OPND hflags_reg = latxs_ra_alloc_itemp();

    latxs_load_imm32_to_ir2(&mask_reg, mask, EXMode_N);

    latxs_append_ir2_opnd2i(LISA_LD_W, &hflags_reg, &latxs_env_ir2_opnd,
            lsenv_offset_of_hflags(lsenv));
    latxs_append_ir2_opnd3(LISA_OR, &hflags_reg, &hflags_reg, &mask_reg);
    latxs_append_ir2_opnd2i(LISA_ST_W, &hflags_reg, &latxs_env_ir2_opnd,
            lsenv_offset_of_hflags(lsenv));

    latxs_ra_free_temp(&mask_reg);
    latxs_ra_free_temp(&hflags_reg);
}

static void latxs_tr_gen_hflags_clr(uint32_t mask)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (!(td->sys.flags & mask)) {
        return;
    }

    IR2_OPND mask_reg   = latxs_ra_alloc_itemp();
    IR2_OPND hflags_reg = latxs_ra_alloc_itemp();

    latxs_load_imm32_to_ir2(&mask_reg, ~mask, EXMode_N);

    latxs_append_ir2_opnd2i(LISA_LD_W, &hflags_reg, &latxs_env_ir2_opnd,
            lsenv_offset_of_hflags(lsenv));
    latxs_append_ir2_opnd3(LISA_AND, &hflags_reg, &hflags_reg, &mask_reg);
    latxs_append_ir2_opnd2i(LISA_ST_W, &hflags_reg, &latxs_env_ir2_opnd,
            lsenv_offset_of_hflags(lsenv));

    latxs_ra_free_temp(&mask_reg);
    latxs_ra_free_temp(&hflags_reg);
}

void latxs_tr_gen_eob(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (!td->need_eob) {
        return;
    }

    if (td->inhibit_irq &&
        !(td->sys.flags & HF_INHIBIT_IRQ_MASK)) {
        latxs_tr_gen_hflags_set(HF_INHIBIT_IRQ_MASK);
    } else {
        latxs_tr_gen_hflags_clr(HF_INHIBIT_IRQ_MASK);
    }

    /*
     * > ignore_top_update
     *
     * is used by special system FP instructions that load TOP
     * from memory and save it into env->fpstt in helper
     *
     * Since the env->fpstt already contains the correct TOP,
     * there is no need to update again. If we do update TOP
     * here we will ruin the correct TOP value.
     *
     * > with LSFPU enabled
     *
     * No need to update TOP after every TB's execution
     * 1. in context switch the TOP will be readed
     * 2. TB-Link no more need to do FPU rotate
     *
     * For those special FP instructions, the TOP will be loaded
     * again after helper finished. So the context switch can
     * read the correct TOP value and save it again.
     */
    if (!(td->ignore_top_update) && !option_lsfpu && !option_soft_fpu) {
        /*
         * Func tr_gen_save_curr_top is controled by the flag
         * td->is_top_saved. Here we should always save TOP
         * so we clear this flag.
         */
        latxs_tr_gen_save_curr_top();
    }

    /* reset to 32s for TB-Link */
#ifndef TARGET_X86_64
    latxs_tr_reset_extmb(0xFF);
#endif
}

/*
 * Generate exit tb for system EOB situation, no TB-Link.
 *
 * Called after all IR1s' translation and
 * translation is not ended with exception.
 * > Normal TB-End: pir1 will be NULL, do nothing
 * > system EOB situation: pir1 will be the last IR1
 *   > special system instruction: pir1 will be that IR1
 *                                 generate exit tb to next
 *                                 x86 instruction
 *   > system eob situation
 *     > If this IR1 is already TB-end instruction, do nothing
 *     > If not, generate exit tb to next x86 instruction
 *   > breakpoint: generate exit tb to next x86 instruction
 */
void latxs_tr_gen_sys_eob(IR1_INST *pir1)
{
    if (!pir1) {
        return;
    }

    if (ir1_is_tb_ending(pir1)) {
        /*
         * If the system EOB instruction is already
         * one normal TB-end instruction,
         * no more exit tb is needed.
         */
        return;
    }

    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;

    lsassertm(!tb->tb_too_large_pir1,
            "system EOB should not exist with TB too large.\n");

    /* EOB worker */
    latxs_tr_gen_eob();

    IR2_OPND tbptr = latxs_ra_alloc_dbt_arg1(); /* a6($10) */

    /* t8: This TB's address */
    latxs_tr_gen_exit_tb_load_tb_addr(&tbptr, (ADDR)tb);

    /* t9: next x86 instruction's address */
    ADDRX next_eip = ir1_addr_next(pir1);
#ifdef TARGET_X86_64
    latxs_tr_gen_exit_tb_load_next_eip(td->ignore_eip_update, next_eip, 64);
#else
    latxs_tr_gen_exit_tb_load_next_eip(td->ignore_eip_update, next_eip, 32);
#endif

    /* jump to context switch */
    latxs_tr_gen_exit_tb_j_context_switch(NULL, 0, 0);
}

void latxs_tr_gen_exit_tb_load_tb_addr(IR2_OPND *tbptr, ADDR tb_addr)
{
    if (lsenv && lsenv->tr_data && lsenv->tr_data->curr_tb) {
        TranslationBlock *tb = lsenv->tr_data->curr_tb;
        ADDR code_ptr = (ADDR)tb->tc.ptr;
        ADDR code_off = (ADDR)(lsenv->tr_data->real_ir2_inst_num << 2);
        ADDR ins_pc = code_ptr + code_off;
        /* tbptr = ins_pc + offset => offset = tbptr - ins_pc */
        int offset = (tb_addr - ins_pc) >> 2;
        if (int32_in_int20(offset)) {
            latxs_append_ir2_opnd1i(LISA_PCADDI, tbptr, offset);
        } else {
            latxs_load_imm64_to_ir2(tbptr, tb_addr);
        }
    } else {
        latxs_load_imm64_to_ir2(tbptr, tb_addr);
    }
}

void latxs_tr_gen_exit_tb_j_tb_link(TranslationBlock *tb, int succ_id)
{
    /* 1. set a label for native code linkage */
    IR2_OPND goto_label_opnd = latxs_ir2_opnd_new_label();

    /* remember where the j instruction position */
    /* reuse jmp_reset_offset[], they will be rewritten in label_dispose */
    /* at that time we can calculate exact offset for this position */
    latxs_append_ir2_opnd1(LISA_LABEL, &goto_label_opnd);
    tb->jmp_reset_offset[succ_id] = goto_label_opnd.val;

    /* point to current j insn addr plus 8 by default, will resolve in */
    /* label_dispose, two instructions for pcaddu18i and jirl patch */
    latxs_append_ir2_opnda(LISA_B, 1);
    latxs_append_ir2_opnd0_(lisa_nop);
}

void latxs_tr_gen_exit_tb_load_next_eip(int reload_eip_from_env, ADDRX eip,
                                        int opnd_size)
{
    /*
     * reload_eip_from_env : pc has been stored to env
     * Marked by special instruction that reaches special EOB
     * This mark means that we should not update the eip in env.
     */
    if (!reload_eip_from_env) {
        IR2_OPND eip_opnd = latxs_ra_alloc_itemp();
        switch (opnd_size) {
        case 32:
            latxs_load_imm32_to_ir2(&eip_opnd, eip, EXMode_Z);
            break;
        case 16:
            latxs_load_imm32_to_ir2(&eip_opnd, eip & 0xffff, EXMode_Z);
            break;
#ifdef TARGET_X86_64
        case 64:
            latxs_load_imm64_to_ir2(&eip_opnd, eip);
            break;
#endif
        default:
            lsassert(0);
            break;
        }
#ifdef TARGET_X86_64
    latxs_append_ir2_opnd2i(LISA_ST_D, &eip_opnd, &latxs_env_ir2_opnd,
                            lsenv_offset_of_eip(lsenv));
#else
    latxs_append_ir2_opnd2i(LISA_ST_W, &eip_opnd, &latxs_env_ir2_opnd,
                            lsenv_offset_of_eip(lsenv));
#endif
        latxs_ra_free_temp(&eip_opnd);
    }

    /* should correctly set env->eip */
    if (lsenv->tr_data->sys.tf) {
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_single_step,
                                         default_helper_cfg);
    }
}

void latxs_tr_gen_exit_tb_j_context_switch(IR2_OPND *tbptr,
        int can_link, int succ_id)
{
    IR2_OPND *ret0 = &latxs_ret0_ir2_opnd;
    IR2_OPND *zero = &latxs_zero_ir2_opnd;

    if (can_link) {
        latxs_append_ir2_opnd2i(LISA_ORI, ret0, tbptr, succ_id);
    } else {
        latxs_append_ir2_opnd2i(LISA_ORI, ret0, zero, succ_id);
    }
    /* jump to context_switch_native_to_bt */
    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;

    ADDR code_buf = (ADDR)tb->tc.ptr;
    int offset = td->real_ir2_inst_num << 2;

    int64_t ins_offset = (context_switch_native_to_bt - code_buf - offset) >> 2;
    latxs_append_ir2_jmp_far(ins_offset, 0);
}

/* Should always use TB-Link. */
void latxs_tr_gen_eob_if_tb_too_large(IR1_INST *pir1)
{
    if (!pir1) {
        return;
    }

    if (ir1_is_tb_ending(pir1)) {
        return;
    }

    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;

    lsassertm(!tb->sys_eob_pir1,
            "TB too large should not exist with system EOB.\n");

    int can_link = option_tb_link;

    latxs_tr_gen_eob();
    CPUState *cpu = env_cpu(lsenv->cpu_state);
    if (td->sys.bp_hit || cpu->singlestep_enabled) {
        current_cpu->exception_index = EXCP_DEBUG;
        can_link = 0;
    }

    IR2_OPND tbptr = latxs_ra_alloc_dbt_arg1(); /* a6($10) */
    int succ_id = 1;

    /* t8: This TB's address */
    latxs_tr_gen_exit_tb_load_tb_addr(&tbptr, (ADDR)tb);

    if (can_link) {
        latxs_tr_gen_exit_tb_j_tb_link(tb, succ_id);
    }

    /* t9: next x86 instruction's address */
    ADDRX next_eip = ir1_addr_next(pir1);
#ifdef TARGET_X86_64
    latxs_tr_gen_exit_tb_load_next_eip(td->ignore_eip_update, next_eip, 64);
#else
    latxs_tr_gen_exit_tb_load_next_eip(td->ignore_eip_update, next_eip, 32);
#endif

    /* jump to context switch */
    latxs_tr_gen_exit_tb_j_context_switch(&tbptr, can_link, succ_id);
}

/*
 * Details of exit-tb
 *   > EOB: End-Of-TB worker  tr_gen_eob()
 *   > Load TB addr   $t8     tr_gen_exit_tb_load_tb_addr()
 *   > Load next EIP  $t9     tr_gen_exit_tb_load_next_eip()
 *   > TB-Link j 0            tr_gen_exit_tb_j_tb_link()
 *   > Context Switch         tr_gen_exit_tb_j_context_switch()
 *
 * Every exit-tb must contains:
 *   > EOB
 *   > Context Switch
 *
 * TB-Link j 0
 *   > if can_link
 *   > if not indirect jmp(jmp/call/ret)
 *
 * Load TB addr
 *   > always gen it for context switch
 *   > when no lsfpu, gen before TB-Link for FPU ratate
 *   > when FastCS, gen before TB-Link for context check
 *   > when cross-page-check, gen before TB-Link
 *     > for cross-page direct TB-Link
 *     > for indirect jmp(jmp/call/ret)
 *     > to do cross-page check
 */
void latxs_tr_generate_exit_tb(IR1_INST *branch, int succ_id)
{
    int      next_eip_size = 32;
#ifdef TARGET_X86_64
    next_eip_size = latxs_ir1_addr_size(branch) << 3;
#endif
    ADDRX    next_eip;
    ADDRX    curr_eip;

    IR1_OPCODE opcode = ir1_opcode(branch);
    int can_link = option_tb_link;

    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;

    /*
     * EOB worker will be generated before
     * each TB's each exit (at most two exit)
     */
    if (!ir1_is_branch(branch)) {
        latxs_tr_gen_eob();
    }

    /* debug support : when hit break point, do NOT TB-link */
    CPUState *cpu = env_cpu(lsenv->cpu_state);
    if (td->sys.bp_hit || cpu->singlestep_enabled) {
        current_cpu->exception_index = EXCP_DEBUG;
        /*
         * don't allow link to next tb, we already return to qemu to allow
         * in time exception check
         */
        can_link = 0;
    }

    /* Cross Pgae Check */
    /*
     * TODO
     * tb->extra_tb->tb_need_cpc[succ_id] =
     * xtm_tb_need_cpc(tb, branch, succ_id);
     */

    /*
     * Load this TB's address into $a6
     *
     *        |       | need TB addr   |
     * FastCS | LSFPU | for jmp glue ? | Reason
     * -----------------------------------------------------
     *    X   |   X   |     YES        | FPU rotate
     *    X   |   O   |     NO         |
     *    O   |   X   |     YES        | TB's fast cs mask
     *    O   |   O   |     YES        | TB's fast cs mask
     */

    IR2_OPND tbptr = latxs_ra_alloc_dbt_arg1(); /* t8($24) */
    ADDR tb_addr = (ADDR)tb;

    int need_tb_addr_for_jmp_glue = (option_lsfpu || option_soft_fpu) ? 0 : 1;

    /*
     * TODO
     * if (xtm_cpc_enabled() && xtm_tb_need_cpc(tb, branch, succ_id)) {
     * need_tb_addr_for_jmp_glue = 1;
     * }
     */

    if (!need_tb_addr_for_jmp_glue) {
        goto IGNORE_LOAD_TB_ADDR_FOR_JMP_GLUE;
    }

    if (!ir1_is_branch(branch)) {
        latxs_tr_gen_exit_tb_load_tb_addr(&tbptr, tb_addr);
    } else {
        if (option_lsfpu &&
            /* xtm_cpc_enabled() && TODO */
            need_tb_addr_for_jmp_glue)
        {
            latxs_tr_gen_exit_tb_load_tb_addr(&tbptr, tb_addr);
        }
    }

IGNORE_LOAD_TB_ADDR_FOR_JMP_GLUE:

    /* Generate 'j 0' if not indrect jmp for TB-Link */
    if (can_link &&
            !ir1_is_indirect_jmp(branch) &&
            !ir1_is_indirect_call(branch) &&
            opcode != X86_INS_RET) {
        latxs_tr_gen_exit_tb_j_tb_link(tb, succ_id);
    }

    if (!need_tb_addr_for_jmp_glue) {
        /* Always need to load TB addr for context switch : for TB-Link */
        latxs_tr_gen_exit_tb_load_tb_addr(&tbptr, tb_addr);
    }

    int mask_cpdj = 1;
    /*
     * Standard process:
     * 1. prepare a6 and a7
     *    > DBT arg1 : a6 Current TranslationBlock's address
     *    > DBT arg2 : a7: Next TranslationBlock's EIP
     * 2. prepare the return value (a0/v0) for TB-link
     * 3. jump to context switch native to bt
     *
     * Cross Page Direct Jmp:
     * 1. check with next_eip and curr_eip
     * 2. set mask for cross page direct jmp
     *    > mask = 1 : link according to @can_link
     *      > when option_cpjl is enabled (force to allow link)
     *      > when option_cpjl is disabled and
     *             current branch is NOT a cross page direct jmp
     *    > mask = 0 : force to disable link
     *      > when option_cpjl is disabled and
     *             current branch is a cross page direct jmp
     * 3. generate exit tb according to can_link & mask_cpdj
     */
    switch (opcode) {
    case X86_INS_CALL:
        if (ir1_is_indirect_call(branch)) {
            goto indirect_call;
        }

        next_eip = ir1_target_addr(branch);
        curr_eip = ir1_addr(branch);

        if ((next_eip >> TARGET_PAGE_BITS) !=
            (curr_eip >> TARGET_PAGE_BITS)) {
            mask_cpdj = option_cross_page_jmp_link;
        }

        if (latxs_ir1_addr_size(branch) == 2 ||
            ir1_opnd_size(ir1_get_opnd(branch, 0)) == 16) {
            next_eip_size = 16;
        }

        latxs_tr_gen_exit_tb_load_next_eip(0, next_eip, next_eip_size);

        latxs_tr_gen_exit_tb_j_context_switch(&tbptr,
                can_link & mask_cpdj, succ_id);
        break;

    case X86_INS_LJMP:
        /* only ptr16:16/ptr16:32 is supported now */
        if (td->sys.pe && !td->sys.vm86) {
            lsassert(td->ignore_eip_update == 1);
            latxs_tr_gen_exit_tb_load_next_eip(1, 0, 0);
        } else if (!ir1_opnd_is_mem(ir1_get_opnd(branch, 0))) {
            next_eip = ir1_opnd_uimm(ir1_get_opnd(branch, 1));
            next_eip_size = ir1_opnd_size(ir1_get_opnd(branch, 1));
            latxs_tr_gen_exit_tb_load_next_eip(0, next_eip, next_eip_size);
        }

        /* Always no TB-link for jmp far */
        latxs_tr_gen_exit_tb_j_context_switch(NULL, 0, succ_id);
        break;

    case X86_INS_JMP:
        if (ir1_is_indirect_jmp(branch)) {
            goto indirect_jmp;
        }

        next_eip = ir1_target_addr(branch);
        curr_eip = ir1_addr(branch);

        if ((next_eip >> TARGET_PAGE_BITS) !=
            (curr_eip >> TARGET_PAGE_BITS)) {
            mask_cpdj = option_cross_page_jmp_link;
        }

        next_eip_size = ir1_opnd_size(ir1_get_opnd(branch, 0));
        latxs_tr_gen_exit_tb_load_next_eip(td->ignore_eip_update, next_eip,
                                           next_eip_size);

        latxs_tr_gen_exit_tb_j_context_switch(&tbptr,
                can_link & mask_cpdj, succ_id);
        break;

    case X86_INS_RET:
indirect_call:
indirect_jmp:

        can_link = can_link && option_intb_link;

        if (sigint_enabled()) {
            if (can_link) {
                ADDR code_buf = (ADDR)tb->tc.ptr;
                int offset = td->real_ir2_inst_num << 2;

                IR2_OPND sigint_label = latxs_ir2_opnd_new_label();
                latxs_append_ir2_opnd1(LISA_LABEL, &sigint_label);

                int64_t ins_offset =
                    (native_jmp_glue_2 - code_buf - offset) >> 2;
                latxs_append_ir2_jmp_far(ins_offset, 0);

                /* will be resolved in label_dispose() */
                tb->jmp_reset_offset[0] =
                    latxs_ir2_opnd_label_id(&sigint_label);
                latxs_append_ir2_opnd0_(lisa_nop);
                tb->is_indir_tb = 1;
            }
            latxs_tr_gen_exit_tb_j_context_switch(NULL, 0, succ_id);
        } else {
            if (can_link) {
                ADDR code_buf = (ADDR)tb->tc.ptr;
                int offset = td->real_ir2_inst_num << 2;
                int64_t ins_offset =
                    (native_jmp_glue_2 - code_buf - offset) >> 2;
                latxs_append_ir2_jmp_far(ins_offset, 0);
            } else {
                latxs_tr_gen_exit_tb_j_context_switch(NULL, 0, succ_id);
            }
        }
        break;
    case X86_INS_JE:
    case X86_INS_JNE:
    case X86_INS_JS:
    case X86_INS_JNS:
    case X86_INS_JB:
    case X86_INS_JAE:
    case X86_INS_JO:
    case X86_INS_JNO:
    case X86_INS_JBE:
    case X86_INS_JA:
    case X86_INS_JP:
    case X86_INS_JNP:
    case X86_INS_JL:
    case X86_INS_JGE:
    case X86_INS_JLE:
    case X86_INS_JG:
    case X86_INS_JCXZ:
    case X86_INS_JECXZ:
    case X86_INS_JRCXZ:
    case X86_INS_LOOP:
    case X86_INS_LOOPE:
    case X86_INS_LOOPNE:
        next_eip = succ_id ? ir1_target_addr(branch) : ir1_addr_next(branch);
        next_eip_size = ir1_opnd_size(ir1_get_opnd(branch, 0));
        curr_eip = ir1_addr(branch);

        if ((next_eip >> TARGET_PAGE_BITS) !=
            (curr_eip >> TARGET_PAGE_BITS)) {
            mask_cpdj = option_cross_page_jmp_link;
        }

        latxs_tr_gen_exit_tb_load_next_eip(0, next_eip, next_eip_size);

        latxs_tr_gen_exit_tb_j_context_switch(&tbptr,
                can_link & mask_cpdj, succ_id);
        break;
    default:
        lsassertm(0, "not implement.\n");
    }
}
