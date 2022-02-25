/* X86toMIPS headers */
#include "../include/common.h"
#include "../include/reg-alloc.h"
#include "../include/env.h"
#include "../ir2/ir2.h"
#include "../x86tomips-options.h"

/* QEMU headers */
#include "../include/qemu-def.h"

void tr_gen_hflags_set(uint32 mask);
void tr_gen_hflags_clr(uint32 mask);

int is_system_eob(IR1_INST *ir1);

static int ir1_is_hit_breakpoint(IR1_INST *ir1)
{
    CPUState *cpu = current_cpu;
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (current_cpu->singlestep_enabled) return 1;

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

static int ir1_is_lxx_seg(IR1_INST *ir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    IR1_OPCODE opc = ir1_opcode(ir1);

    switch (opc) {
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

static int ir1_is_pop_seg_eob(IR1_INST *ir1)
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

int ir1_is_eob_in_sys(IR1_INST *ir1)
{
    if (lsenv->tr_data->sys.cflags & CF_USE_ICOUNT) {
        if(ir1_is_mov_from_cr(ir1)){
            return true;
        }
    }
    return ir1_is_mov_to_cr(ir1) ||         /* mov cr, src */
           ir1_is_mov_to_dr(ir1) ||         /* mov dr, src */
           ir1_is_pop_eflags(ir1) ||        /* popf */
           ir1_is_sysenter(ir1) ||          /* sysenter */
           ir1_is_sysexit(ir1) ||           /* sysexit */
           ir1_is_rsm(ir1) ||               /* rsm */
           ir1_is_sti(ir1) ||               /* sti */
           ir1_is_mov_to_seg_eob(ir1) ||    /* mov es/cs/ss/ds, src */
           ir1_is_pop_seg_eob(ir1) ||       /* pop ds/es/ss, src */
           ir1_is_repz_nop(ir1) ||          /* pause */
           ir1_is_pause(ir1) ||             /* pause */
           ir1_is_iret(ir1) ||              /* iret */
           ir1_is_lmsw(ir1) ||              /* lmsw */
           ir1_is_retf(ir1) ||              /* ret far */
           ir1_is_call_far(ir1) ||          /* call far */
           ir1_is_clts(ir1) ||              /* clts */
           ir1_is_invlpg(ir1) ||            /* invlpg */
           ir1_is_lxx_seg(ir1) ||           /* lss, lds/les */
           ir1_contains_fldenv(ir1) ||      /* fldenv, frstor, fxrstor, xrstor */
           ir1_is_fninit(ir1) ||            /* fninit: reset all to zero */
           ir1_is_fnsave(ir1) ||            /* fnsave: reset all to zero after save */
           ir1_is_xsetbv(ir1) ||            /* xsetbv */
           ir1_is_mwait(ir1) ||             /* mwait */
           ir1_is_vmrun(ir1) ||             /* vmrun */
           ir1_is_stgi(ir1) ||              /* stgi */
           is_system_eob(ir1) ||
           ir1_is_hit_breakpoint(ir1);
}

void tr_gen_hflags_set(uint32 mask)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (td->sys.flags & mask)
        return;

    IR2_OPND mask_reg   = ra_alloc_itemp();
    IR2_OPND hflags_reg = ra_alloc_itemp();

    load_imm32_to_ir2(&mask_reg, mask, UNKNOWN_EXTENSION);

    append_ir2_opnd2i(LISA_LD_W, &hflags_reg, &env_ir2_opnd,
                      lsenv_offset_of_hflags(lsenv));
    append_ir2_opnd3(LISA_OR, &hflags_reg, &hflags_reg, &mask_reg);
    append_ir2_opnd2i(LISA_ST_W, &hflags_reg, &env_ir2_opnd,
                      lsenv_offset_of_hflags(lsenv));
}

void tr_gen_hflags_clr(uint32 mask)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (!(td->sys.flags & mask))
        return;

    IR2_OPND mask_reg   = ra_alloc_itemp();
    IR2_OPND hflags_reg = ra_alloc_itemp();

    load_imm32_to_ir2(&mask_reg, ~mask, UNKNOWN_EXTENSION);

    append_ir2_opnd2i(LISA_LD_W, &hflags_reg, &env_ir2_opnd,
                      lsenv_offset_of_hflags(lsenv));
    append_ir2_opnd3(LISA_AND, &hflags_reg, &hflags_reg, &mask_reg);
    append_ir2_opnd2i(LISA_ST_W, &hflags_reg, &env_ir2_opnd,
                      lsenv_offset_of_hflags(lsenv));
}

void tr_gen_eob(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (!td->need_eob) {
        return;
    }

    if (td->inhibit_irq &&
        !(td->sys.flags & HF_INHIBIT_IRQ_MASK)) {
        tr_gen_hflags_set(HF_INHIBIT_IRQ_MASK);
    } else {
        tr_gen_hflags_clr(HF_INHIBIT_IRQ_MASK);
    }

    /* > ignore_top_update
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
    if (!(td->ignore_top_update) && !option_lsfpu) {
        /* Func tr_gen_save_curr_top is controled by the flag
         * td->is_top_saved. Here we should always save TOP
         * so we clear this flag. */
        td->is_top_saved = 0;
        tr_gen_save_curr_top();
    }
}

int ir1_is_mov_to_seg_eob(IR1_INST *ir1)
{
    if (ir1->info->id != X86_INS_MOV)
        return 0;

    IR1_OPND *opnd0 = ir1_get_opnd(ir1, 0);
    if (!ir1_opnd_is_seg(opnd0))
        return 0;

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

int is_system_eob(IR1_INST *ir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    ADDRX pc_first = td->sys.pc;
    ADDRX pc_next  = ir1_addr_next(ir1) + td->sys.cs_base;

    if (td->sys.tf ||
       (td->sys.flags & HF_INHIBIT_IRQ_MASK) ) {
        return 1;
    }
    else if (td->sys.cflags & CF_USE_ICOUNT) {
        if (((pc_next & TARGET_PAGE_MASK)
                   != ((pc_next + TARGET_MAX_INSN_SIZE - 1)
                       & TARGET_PAGE_MASK)
                   || (pc_next & ~TARGET_PAGE_MASK) == 0)) {
            return 1;
	}
	else
	{
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

    }
    else if ((pc_next - pc_first) >= (TARGET_PAGE_SIZE - 32)) {
        return 1;
    }

    return 0;
}

/* Generate exit tb for system EOB situation, no TB-Link.
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
 *   > breakpoint: generate exit tb to next x86 instruction */
void tr_gen_sys_eob(IR1_INST *pir1)
{
    if (!pir1) return;

    if (ir1_is_tb_ending(pir1)) {
        /* If the system EOB instruction is already
         * one normal TB-end instruction,
         * no more exit tb is needed. */
        return;
    }

//    tr_adjust_em();

    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;

    lsassertm(!tb->extra_tb->tb_too_large_pir1,
            "system EOB should not exist with TB too large.\n");

    /* EOB worker */
    tr_gen_eob();

    IR2_OPND tbptr = ra_alloc_dbt_arg1(); /* t8($24) */
    IR2_OPND next_eip_opnd = ra_alloc_dbt_arg2(); /* t9($25) */

    /* t8: This TB's address */
    tr_gen_exit_tb_load_tb_addr(&tbptr, (ADDR)tb);

    /* t9: next x86 instruction's address */
    ADDRX next_eip = ir1_addr_next(pir1);
    tr_gen_exit_tb_load_next_eip(td->ignore_eip_update,
            &next_eip_opnd, next_eip, 32);

#ifdef CONFIG_XTM_PROFILE
    if (option_monitor_jc) {
        /* monitor JC mark syseob lookup */
    lsassertm(0, "monitor jc to be implemented in LoongArch.\n");
//        append_ir2_opnd2i(mips_ori, &arg0_ir2_opnd, &zero_ir2_opnd, 0x1);
//        append_ir2_opnd2i(mips_sb, &arg0_ir2_opnd, &env_ir2_opnd,
//                lsenv_offset_of_pf_data_jc_is_sys_eob(lsenv));
    }
#endif

    /* jump to context switch */
    tr_gen_exit_tb_j_context_switch(NULL, 0, 0);
}
