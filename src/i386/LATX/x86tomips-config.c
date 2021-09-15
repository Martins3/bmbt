#include "include/common.h"
#include "x86tomips-config.h"
#include "ir1/ir1.h"
#include "ir2/ir2.h"
#include "include/env.h"
#include "include/etb.h"
#include "include/reg_alloc.h"
#include "x86tomips-options.h"
#include "include/etb.h"
#include "include/shadow_stack.h"
#include "include/profile.h"
#include <signal.h>
#include <ucontext.h>
#include <string.h>
#include <pthread.h>

#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
#include "x86tomips-profile-sys.h"
#endif

/* For optimization sigint in system-mode */
void xtm_tb_unlink(TranslationBlock *ctb);
void xtm_tb_relink(TranslationBlock *utb);

// #include "trace.h" FIXME comment it temporary
// it cause scrpit/change-latx-header.py
int whether_print_cpu_info(int cpu_index)
{
    return option_cpusinfo & (1<<cpu_index);
}

/* Main function to do binary translation.
 *
 * @max_insns, @code_highwater, @search_size is used in system-mode ONLY */
int target_x86_to_mips_host(
        CPUState *cpu,
        TranslationBlock *tb,
        int max_insns,
        void *code_highwater,
        int *search_size)
{
    CPUArchState *env = cpu->env_ptr;
    counter_tb_tr += 1;

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
    xtm_pf_step_translate_tb(tb);
#endif

#ifdef CONFIG_SOFTMMU
    if(option_break_point &&
       xqm_break_point_code &&
       tb->pc == option_break_point_addrx)
    {
        ((void(*)(void))xqm_break_point_code)();
    }
#endif

    // fuck_trace_xtm_tr_tb((void *)tb, (void *)tb->tc.ptr,
                    // (void *)(unsigned long long)tb->pc);

    if (option_dump && whether_print_cpu_info(cpu->cpu_index)) {
        fprintf(stderr, "=====================================\n");
        fprintf(stderr, "|| TB translation : %14p ||\n", tb);
        fprintf(stderr, "=====================================\n");
#ifndef CONFIG_SOFTMMU
        fprintf(stderr, "Guest Base = 0x%lx\n", (unsigned long)guest_base);
#endif
        fprintf(stderr, "=====================================\n");
    }

#ifdef CONFIG_SOFTMMU
    if (option_dump) {
        fprintf(stderr, "CPU ID     = %d\n", cpu->cpu_index);
    }
    tr_sys_init(tb, max_insns, code_highwater);
#endif

    /* target => IR1
     * IR1 stored in lsenv->tr_data
     *
     * disassemble by capstone */
    tr_disasm(tb);

#ifndef CONFIG_SOFTMMU
    if (option_flag_reduction) {
        tb_flag_reduction((void *)tb);
    } else {
        tb_flag_usedef((void *)tb);
    }
#else
    tb_flag_usedef((void *)tb);
#endif

    if (!option_lsfpu) {
        // FIXME how etb works ?
        etb_check_top_in(tb->extra_tb, env->fpstt);
    }

    /* IR1 => IR2 => host
     * IR2 stored in lsenv->tr_data
     * host binary will be writen into code cache (at TB.tc.ptr) */
    return tr_translate_tb(tb, search_size);
}

/* This helper is used in user-mode to simulate 'int' instruction */
void helper_raise_int(void)
{
    set_CPUX86State_error_code(lsenv, 0);
    set_CPUX86State_exception_is_int(lsenv, 1);
    set_CPUState_can_do_io(lsenv, 1);
    siglongjmp_cpu_jmp_env();
}
void set_CPUX86State_error_code(ENV *lsenv, int error_code)
{
    CPUX86State *env = lsenv->cpu_state;
    env->error_code = error_code;
}
void set_CPUX86State_exception_is_int(ENV *lsenv, int exception_is_int)
{
    CPUX86State *env = lsenv->cpu_state;
    env->exception_is_int = exception_is_int;
}
int lsenv_offset_exception_index(ENV *lsenv)
{
    CPUX86State *env = lsenv->cpu_state;
    CPUState *cs = env_cpu(env);
    return (int)((ADDR)(&cs->exception_index) - (ADDR)lsenv->cpu_state);
}
int lsenv_offset_exception_next_eip(ENV *lsenv)
{
    return offsetof(CPUX86State, exception_next_eip);
}
void set_CPUState_can_do_io(ENV *lsenv, int can_do_io)
{
    CPUX86State *env = lsenv->cpu_state;
    CPUState *cs = env_cpu(env);
    cs->can_do_io = can_do_io;
}
void siglongjmp_cpu_jmp_env(void)
{
    CPUX86State *env = lsenv->cpu_state;

    /* siglongjmp will skip the execution of x86_to_mips_after_exec_tb
     * which is expected to reset top_bias/top
     */
    TranslationBlock *last_tb = (TranslationBlock *)lsenv_get_last_executed_tb(lsenv);
    x86_to_mips_after_exec_tb(env, last_tb);

    CPUState *cs = env_cpu(env);
    siglongjmp(cs->jmp_env, 1);
}

/* Functions to access QEMU's TB */
void *qm_tb_get_jmp_target_arg(TranslationBlock *tb) { return &(tb->jmp_target_arg[0]); }
void *qm_tb_get_jmp_reset_offset(TranslationBlock *tb) { return &(tb->jmp_reset_offset[0]); }

#ifdef CONFIG_SOFTMMU
inline ADDRX linear_address(ADDRX pc) { return lsenv->tr_data->sys.cs_base + pc; }
#else
inline ADDRX linear_address(ADDRX pc) { return pc; }
#endif

#ifdef CONFIG_SOFTMMU
ADDR cpu_get_guest_base(void) { lsassert(0); return 0; }
#else
ADDR cpu_get_guest_base(void) { return guest_base; }
#endif

// FIXME this is called after tb_find
// x86tomips-config.c is a wired name
// maybe changed to xqm-hacking.c, our extra hacking for qemu
void trace_tb_execution(CPUState *cpu, TranslationBlock *tb)
{
    if (!xtm_trace_enabled()) return;

    lsassert(tb != NULL);
    CPUX86State *env = lsenv->cpu_state;

#ifdef CONFIG_SOFTMMU
    static unsigned long long break_point_tb_exec_count = 0;
    if(option_break_point && xqm_break_point_code &&
       tb->pc == option_break_point_addrx)
    {
        break_point_tb_exec_count += 1;
        fprintf(stderr, "[debug] break point TB exec %#x. cnt = %lld.\n",
                tb->pc, break_point_tb_exec_count);
        if (break_point_tb_exec_count >= option_break_point_count) {
            ((void(*)(void))xqm_break_point_code)();
        }
    }
#endif

    /* start to print trace after @num of TB's execution */
    static unsigned long long trace_tb_count = 0;
    if (trace_tb_count < option_trace_start_nr) {
        trace_tb_count += 1;
        return;
    }
    trace_tb_count += 1;

    /* start to print trace after a given TB's execution */
    if (option_trace_start_tb_set) {
        static int trace_start_tb_ok = 0;
        if (tb->pc == option_trace_start_tb) {
            trace_start_tb_ok = 1;
        }
        if (!trace_start_tb_ok) {
            return;
        }
    }

#ifdef CONFIG_XTM_FLAG_INT
    if (env->xtm_flags.is_in_int) { /* CPU is handling int/excp */
        if (option_trace_simple &&
            !(lsenv->trace_is_in_int) &&    /* This is the first TB to handle int/excp */
            whether_print_cpu_info(cpu->cpu_index))
        {
            fprintf(stderr, "[tracesp] ");
            if(cpu->nr_cores * cpu->nr_threads > 1)
                fprintf(stderr, "CPU%d/%d ", cpu->cpu_index, cpu->nr_cores*cpu->nr_threads);
            fprintf(stderr, "handling interrupt or exception.\n");
        }
        /* Set the flag to avoid printing "CPU is handling interrupt or exception"
         * until it finishes handling int/excp */
        lsenv->trace_is_in_int = 1;
    }
    else {
        lsenv->trace_is_in_int = 0;
    }
#endif

    if (option_trace_simple && whether_print_cpu_info(cpu->cpu_index)) {
        uint32 eflags = helper_read_eflags(lsenv->cpu_state);
        fprintf(stderr, "[tracesp] ");
        if (cpu->nr_cores*cpu->nr_threads > 1) {
            /* if there are more than one thread/cpu */
            fprintf(stderr, "CPU%d/%d ", cpu->cpu_index, cpu->nr_cores*cpu->nr_threads);
        }
        fprintf(stderr, "PC=%#x / ",      tb->pc);
        fprintf(stderr, "CS_BASE=%#x / ", tb->cs_base);
        fprintf(stderr, "FLAGS=%#x / ",   tb->flags);
        fprintf(stderr, "EFLAGS=%#x / ",  eflags);
        switch (option_trace_simple) {
        case 2: /* Print with FPU state */
            fprintf(stderr, "FPSTT=%d / ",  env->fpstt);
            fprintf(stderr, "TOPIN=%d / ",  tb->extra_tb->_top_in);
            fprintf(stderr, "TOPOUT=%d / ", tb->extra_tb->_top_out);
#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_FAST_CS)
            fprintf(stderr, "FASTCS=%x / ", tb->extra_tb->fast_cs_mask);
#endif
            fprintf(stderr, "FREGS=%#lx,%#lx,%#lx,%#lx,%#lx,%#lx,%#lx,%#lx / ",
                    env->fpregs[0].d.low, env->fpregs[1].d.low,
                    env->fpregs[2].d.low, env->fpregs[3].d.low,
                    env->fpregs[4].d.low, env->fpregs[5].d.low,
                    env->fpregs[6].d.low, env->fpregs[7].d.low);
            break;
        default:
            break;
        }
        fprintf(stderr, "REGS=%#x,%#x,%#x,%#x,%#x,%#x,%#x,%#x",
                env->regs[0], env->regs[1], env->regs[2], env->regs[3],
                env->regs[4], env->regs[5], env->regs[6], env->regs[7]);
        fprintf(stderr, "\n");
        return;
    }

    if (!option_trace_tb && !option_trace_tb_ir1) {
        return;
    }

    fprintf(stderr, "[trace] ========================\n");
    fprintf(stderr, "[trace] TB to execute\n");

    if (option_trace_tb) {
        fprintf(stderr, "[trace] ========================\n");
        fprintf(stderr, "[trace] TB      = %-18p , TB's address\n", (void *)tb);
        fprintf(stderr, "[trace] Counter = %-18lld , TB's execution\n",
                counter_tb_exec);
        fprintf(stderr, "[trace] Counter = %-18lld , TB's translation\n",
                counter_tb_tr);
        fprintf(stderr, "[trace] Counter = %-18lld , IR1 translated\n",
                counter_ir1_tr);
        fprintf(stderr, "[trace] Counter = %-18lld , MIPS generated\n",
                counter_mips_tr);
        fprintf(stderr, "[trace] PC      = %-18p , target's virtual address\n",
                (void *)(unsigned long)tb->pc);
        fprintf(stderr, "[trace] csbase  = %-18p , target's CS segment base\n",
                (void *)(unsigned long)tb->cs_base);
        fprintf(stderr, "[trace] size    = %-18ld , TB's target code size\n",
                (unsigned long)tb->size);
        fprintf(stderr, "[trace] tc.ptr  = %-18p , TB's host code size\n",
                (void *)tb->tc.ptr);
        fprintf(stderr, "[trace] tc.size = %-18ld , TB's host code size\n",
                (unsigned long)tb->tc.size);
        fprintf(stderr, "[trace] host nr = %-18ld , TB's host code number\n",
                (unsigned long)tb->tc.size / 4);
    }

    ETB *etb = tb->extra_tb;

    IR1_INST *ir1_list = etb->_ir1_instructions;
    IR1_INST *pir1 = NULL;
    int ir1_nr = etb->_ir1_num;

    int i = 0;

    if (option_trace_tb_ir1) {
        fprintf(stderr, "[trace] ========================\n");
        fprintf(stderr, "[trace] ir1_nr  = %-18ld , TB's IR1 code size\n",
                (unsigned long)ir1_nr);
        for (i = 0; i < ir1_nr; ++i) {
            pir1 = ir1_list + i;
            fprintf(stderr, "[trace] ");
            ir1_dump(pir1);
        }
    }

    fprintf(stderr, "[trace] ========================\n");
}

void x86_to_mips_alloc_lsenv(void)
{
    if (lsenv) return;

    lsenv = malloc(sizeof(*lsenv));
    // same to malloc then memset to zero
    lsenv->tr_data = calloc(1, sizeof(*(lsenv->tr_data)));
    lsenv->fp_data = calloc(1, sizeof(*(lsenv->fp_data)));

//    TRANSLATION_DATA *td = lsenv->tr_data;

//    memcpy(td->itemp_status, itemp_status_default, sizeof(itemp_status_default));
//    memcpy(td->ftemp_status, ftemp_status_default, sizeof(ftemp_status_default));
//#ifdef REG_ALLOC_ALG_IMM
//    memcpy(td->itemp_status_bak, itemp_status_default, sizeof(itemp_status_default));
//#endif

#ifdef CONFIG_SOFTMMU
    /* init fields used for FPU top fix */
    lsenv->after_exec_tb_fixed = 1;

    lsenv->tr_data->slow_path_rcd_max = 4;
    lsenv->tr_data->slow_path_rcd_nr  = 0;
    lsenv->tr_data->slow_path_rcd = mm_calloc(4, sizeof(softmmu_sp_rcd_t));
#endif
}

void x86_to_mips_free_lsenv(void)
{
    if (lsenv) {
        if (lsenv->tr_data) {
            mm_free(lsenv->tr_data);
        }
        mm_free(lsenv->fp_data);
        mm_free(lsenv);
    }
    lsenv = NULL;
}

void x86_to_mips_init_env(CPUX86State *env)
{
    x86_to_mips_alloc_lsenv();
    lsenv->cpu_state = env;
    env->vregs[4] = (uint64_t)ss._ssi_current;
    if (option_dump) {
        fprintf(stderr, "[X86toMIPS] lsenv@%p env init : %p\n", lsenv, lsenv->cpu_state);
    }
#ifdef CONFIG_XTM_FLAG_INT
    env->xtm_flags.is_in_int   = 0;
    env->xtm_flags.is_int_inst = 0;
    env->xtm_flags.is_top_int_inst = 0;
    lsenv->trace_is_in_int = 0;
#endif
    env->xtm_fpu = XTM_FPU_RESET_VALUE;
#ifdef CONFIG_SOFTMMU
    env->cpt_ptr = &lsenv->cpc_data.cpt;
#endif
}

 /* This function is needed to be execute every time before the native
  * code proceeds. Mainly adjust the fpu top ptr and synchornized eflags.
  */
void x86_to_mips_before_exec_tb(CPUState *cpu, struct TranslationBlock *tb)
{
    CPUX86State *env = cpu->env_ptr;

#ifdef CONFIG_SOFTMMU
    env->cpt_ptr = &lsenv->cpc_data.cpt;
#endif

    if (option_trace_tb)
    {
        fprintf(stderr, "[X86toMIPS] ");
        if (cpu->nr_cores*cpu->nr_threads > 1) {
            /* if there are more than one thread/cpu */
            fprintf(stderr, "CPU%d/%d ", cpu->cpu_index, cpu->nr_cores*cpu->nr_threads);
        }
        fprintf(stderr, "before executing TB {PC = %p, code at %p}.\n",
                (void *)(unsigned long)tb->pc, (void *)tb->tc.ptr);
    }
    counter_tb_exec += 1;

    if (!option_lsfpu) {
        lsassert(lsenv_get_top_bias(lsenv) == 0);
        xtm_fpu_fix_before_exec_tb(env, tb);
    }

#ifdef CONFIG_SOFTMMU
    /* sync the eflags
     *
     * collaborate with tr_load_eflags() in context switch
     * to load the newest elfags into native context */
    env->eflags = helper_read_eflags(env);
    env->cc_src = env->eflags;
    env->cc_op  = CC_OP_EFLAGS;

#ifdef CONFIG_XTM_FAST_CS
    lsenv_set_fast_cs_mask(lsenv, tb->extra_tb->fast_cs_mask);
#endif

#ifdef CONFIG_XTM_PROFILE
    xtm_pf_step_before_exec_tb(tb);
#endif

    lsenv->after_exec_tb_fixed = 0;
#ifdef CONFIG_XTM_TEST
    fprintf(stderr, "[test-trace] ");
    fprintf(stderr, "EIP=%#x / CS_BASE=%#x / FLAGS=%#x / EFLAGS=%#x / REGS=%#x,%#x,%#x,%#x,%#x,%#x,%#x,%#x\n",
            env->eip, env->segs[1].base, env->hflags, env->eflags,
            env->regs[0], env->regs[1], env->regs[2], env->regs[3],
            env->regs[4], env->regs[5], env->regs[6], env->regs[7]);
#endif
#endif
}

void x86_to_mips_after_exec_tb(CPUX86State *env, TranslationBlock *tb)
{
#ifndef CONFIG_SOFTMMU
    if (option_profile)
        context_switch_time++;
#else /* system-mode profile : monitor */
#ifdef CONFIG_XTM_PROFILE
    xtm_pf_step_after_exec_tb();
#endif
#endif
    if (option_trace_tb)
        fprintf(stderr, "[X86toMIPS] after  executing TB {PC = %p, code at %p}.\n",
                (void *)(unsigned long)tb->pc, (void *)tb->tc.ptr);

    if (!option_lsfpu) {
        /* if tb linked to other tbs, last executed tb might not be current tb
         * if last_executed_tb is null, it is not linked indirect jmps
         */
        TranslationBlock *last_executed_tb =
            (TranslationBlock *)(lsenv_get_last_executed_tb(lsenv));
#ifdef N64
        if(last_executed_tb) {
            last_executed_tb =(TranslationBlock*)((uintptr_t)last_executed_tb |\
                    ((uintptr_t)tb & 0xffffffff00000000));
            //lsenv_set_last_executed_tb(lsenv,(ADDR)last_executed_tb);
        }
#endif

        xtm_fpu_fix_after_exec_tb(env, tb, last_executed_tb);
    }

#ifdef CONFIG_SOFTMMU
    lsenv->after_exec_tb_fixed = 1;

    if (xtm_sigint_opt()) {
        /* relink the TB that was unlinked by sigint */
        TranslationBlock *utb = lsenv->sigint_data.tb_unlinked;
        xtm_tb_relink(utb);
        lsenv->sigint_data.tb_unlinked = NULL;
    }

#ifdef CONFIG_XTM_TEST
    uint32_t eflags = helper_read_eflags(env);
    fprintf(stderr, "[test-trace] ");
    fprintf(stderr, "EIP=%#x / CS_BASE=%#x / FLAGS=%#x / EFLAGS=%#x / REGS=%#x,%#x,%#x,%#x,%#x,%#x,%#x,%#x\n",
            env->eip, env->segs[1].base, env->hflags, eflags,
            env->regs[0], env->regs[1], env->regs[2], env->regs[3],
            env->regs[4], env->regs[5], env->regs[6], env->regs[7]);
#endif
#endif
}

#ifdef CONFIG_SOFTMMU
/* Because of exception or interrupt, x86_to_mips_after_exec_tb() will no
 * be executed. We use this function to finish some necessary job.
 *
 * Current implemented instructions need this fix :
 * ========================================================================
 *   > X86_INS_INT :
 *     helper_raise_interrupt() -> raise_interrupt()
 *     -> raise_interrupt2() -> cpu_loop_exit_restore() --> cpu_loop_exit()
 *   > All Exception Check :
 *     helper_raise_exception() -> raise_exception()
 *     -> raise_interrupt2() -> cpu_loop_exit_restore() --> cpu_loop_exit()
 *   > X86_INS_PAUSE : helper_pause() -> do_pause() ------> cpu_loop_exit()
 *   > X86_INS_HLT : helper_hlt() -> do_hlt() ------------> cpu_loop_exit()
 * ========================================================================
 *
 * Other helpers might need this fix too :
 * ========================================================================
 *   > X86_INS_INTO : helper_into()
 *     -> raise_interrupt() -> raise_interrupt2() --------> cpu_loop_exit()
 *   > helpers in misc/seg_helper.c might call
 *     raise_exception_err() -> raise_interrupt2() -------> cpu_loop_exit()
 *   > helpers in bpt/excp/misc/seg_helper.c might call
 *     raise_exception_err_ra() -> raise_interrupt2() ----> cpu_loop_exit()
 *   > helpers in bpt/excp/svm_helper.c might call
 *     raise_exception() -> raise_interrupt2() -----------> cpu_loop_exit()
 *   > helpers in cc/fpu/int/mem/misc/mpx_helper.c might call
 *     raise_exception_ra() -> raise_interrupt2() --------> cpu_loop_exit()
 *   > helpers in svm/seg/misc_helper.c might call -------> cpu_loop_exit()
 *   > cpu_handle_exception() -> do_interrupt_all()
 *                            -> do_interrupt_protected/real()
 *                            -> raise_exception_err() ---> cpu_loop_exit()
 *   > all of the raise_interrupt2()
 *                -> cpu_svm_check_interrupt_param()
 *                -> cpu_vmexit() ------------------------> cpu_loop_exit()
 *   > all of the raise_interrupt2()
 *                -> check_exception() -> qemu_system_reset_request()
 *                   -> cpu_stop_current()
 *                   -> cpu_exit() : cpu->exit_request = 1
 *                -> cpu_loop_exit_restore() -------------> cpu_loop_exit()
 *   > helper_svm_check_interrupt_param() ----------------> cpu_loop_exit()
 *   > helper_svm_check_io() -> cpu_vmexit() -------------> cpu_loop_exit()
 * ========================================================================
 *
 * The execution might be :
 * ========================================================================
 * <1> native code -> context switch ---------> x86_to_mips_after_exec_tb()
 *     -> x86_to_mips_after_exec_tb()
 *     -> cpu_handle_interrupt()    // no interrupt
 *     -> next TB
 * ========================================================================
 * <2> native code -> context switch ---------> x86_to_mips_after_exec_tb()
 *     -> x86_to_mips_after_exec_tb()
 *     -> cpu_handle_interrupt()    // have interrupt
 *     -> x86_cpu_exec_interrupt()
 *     -> EIP updated -> new TB
 * ========================================================================
 * <3> native code -> context switch ---------> x86_to_mips_after_exec_tb()
 *     -> x86_to_mips_after_exec_tb()
 *     -> cpu_handle_interrupt()    // have hard interrupt
 *     -> x86_cpu_exec_interrupt()
 *     -> do_interrupt_x86_hardirq()
 *     -> do_interrupt_all()
 *     -> do_interrupt_protected/real()
 *     -> EIP updated -> new TB
 * ========================================================================
 * <4> native code -> context switch ---------> x86_to_mips_after_exec_tb()
 *     -> cpu_handle_interrupt()    // have hard interrupt
 *     -> x86_cpu_exec_interrupt()
 *     -> do_interrupt_x86_hardirq()
 *     -> do_interrupt_all()
 *     -> do_interrupt_protected/real()
 *     -> raise_exception_err() --------------------------> cpu_loop_exit()
 *     -> cpu_hadnle_exception()
 *     -> EIP updated -> new TB
 * ========================================================================
 * <5> native code -> helper ->
 *     raise_exception/interrupt() -----------------------> cpu_loop_exit()
 *     -> cpu_handle_exception()
 *     -> do_interrupt_all()
 *     -> do_interrupt_protected/real()
 *     -> EIP updated -> new TB
 * ========================================================================
 * <6> native code -> helper ->
 *     raise_exception/interrupt() -----------------------> cpu_loop_exit()
 *     -> cpu_handle_exception()
 *     -> do_interrupt_all()
 *     -> do_interrupt_protected/real()
 *     -> raise_exception_err()
 *     -> raise_interrupt2() -----------------------------> cpu_loop_exit()
 *     -> cpu_handle_exception()
 *     -> do_interrupt_all()
 *     -> do_interrupt_protected/real()
 *     -> EIP updated -> new TB
 * ========================================================================
 *
 * Plan 1: always do this in cpu_loop_exit()
 * Plan 2: do this fix in each possible execution
 *
 * Advantage of Plan 1:
 *
 *      Only need to modify one function cpu_loop_exit() to cover all the
 *      cases that need to be fix.
 *
 * Advantage of Plan 2:
 *
 *      Easy to control only exec it when necessary.
 *
 * Disadvantage of plan 1: Too many ways to call cpu_loop_exit()!
 *
 *      It is hard to tell when this fix is really needed and when it
 *      should not be executed such as exception happends and the CPU
 *      has to restore.
 *
 * Disadvantage of plan 2: Need to modity too many places.
 *
 * We choose plan 1:
 *
 *      Use the flag 'after_exec_tb_fixed' in lsenv to control not to execute
 *      it more than once.
 *
 */
void x86_to_mips_fix_after_excp_or_int(void)
{
#ifdef CONFIG_XTM_PROFILE
    xtm_pf_step_cpu_loop_exit();
#endif

    if (xtm_sigint_opt()) {
        TranslationBlock *utb = lsenv->sigint_data.tb_unlinked;
        xtm_tb_relink(utb);
        lsenv->sigint_data.tb_unlinked = NULL;
    }

    if (lsenv->after_exec_tb_fixed) {
        return;
    }

    if (!option_lsfpu) {
        xtm_fpu_fix_cpu_loop_exit();
    }

    lsenv->after_exec_tb_fixed = 1;
}

#endif

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_FAST_CS)

#define FCS_LINK_DRT 0
#define FCS_LINK_FPU 1
#define FCS_LINK_XMM 2
#define FCS_LINK_ALL 3

static int fastcs_tblink_flag(ETB *e1, ETB *e2)
{
    uint8_t m1 = (e1->fast_cs_mask) & 0x3;
    uint8_t m2 = (e2->fast_cs_mask) & 0x3;

    /* m1 => m2 |  00  |  01  |  10  |  11
     * ---------+------+------+------+------
     * 00       |  --  |  FP  |  MM  |  AL
     * ---------+------+------+------+------
     * 01       |  --  |  FP  |  MM  |  AL
     * ---------+------+------+------+------
     * 10       |  --  |  FP  |  --  |  FP
     * ---------+------+------+------+------
     * 11       |  --  |  FP  |  --  |  FP
     * -------------------------------------
     *
     * LSFPU    |  00  |  01  |  10  |  11
     * -------------------------------------
     * 00       |  --  |  FP  |  MM  |  AL
     * ---------+------+------+------+------
     * 01       |  --  |  --  |  MM  |  MM
     * ---------+------+------+------+------
     * 10       |  --  |  FP  |  --  |  FP
     * ---------+------+------+------+------
     * 11       |  --  |  --  |  --  |  --
     * -------------------------------------
     */

    static const uint8_t fast_cs_link_flag[4][4] = {
        {FCS_LINK_DRT, FCS_LINK_FPU, FCS_LINK_XMM, FCS_LINK_ALL},
        {FCS_LINK_DRT, FCS_LINK_FPU, FCS_LINK_XMM, FCS_LINK_ALL},
        {FCS_LINK_DRT, FCS_LINK_FPU, FCS_LINK_DRT, FCS_LINK_FPU},
        {FCS_LINK_DRT, FCS_LINK_FPU, FCS_LINK_DRT, FCS_LINK_FPU}
    };

    static const uint8_t fast_cs_link_flag_lsfpu[4][4] = {
        {FCS_LINK_DRT, FCS_LINK_FPU, FCS_LINK_XMM, FCS_LINK_ALL},
        {FCS_LINK_DRT, FCS_LINK_DRT, FCS_LINK_XMM, FCS_LINK_XMM},
        {FCS_LINK_DRT, FCS_LINK_FPU, FCS_LINK_DRT, FCS_LINK_FPU},
        {FCS_LINK_DRT, FCS_LINK_DRT, FCS_LINK_DRT, FCS_LINK_DRT}
    };

    if (option_lsfpu) {
        return fast_cs_link_flag_lsfpu[m1][m2];
    }

    uint8_t flag = fast_cs_link_flag[m1][m2];

    if (flag == FCS_LINK_FPU && (m1 & 0x1)) {
        /* Only need to check FPU rotate when: */
        if (e1->_top_out == e2->_top_in) {
            flag = FCS_LINK_DRT;
        }
    }

    if (flag == FCS_LINK_ALL && (m1 & 0x1)) {
        /* All check but fpu rotate is needed when: */
        if (e1->_top_out == e2->_top_in) {
            flag = FCS_LINK_XMM;
        }
    }

    return flag;
}

#endif

static bool try_fast_link(TranslationBlock *tb, TranslationBlock *next_tb){
#ifndef CONFIG_LATX
    ETB *etb = tb->extra_tb;
    if (xtm_branch_opt() && etb->end_with_jcc) {
        uintptr_t mips_branch_inst_addr =
            (uintptr_t)((uintptr_t)tb->tc.ptr + etb->mips_branch_inst_offset);
        // code cache < 256MB, int32 enough
        int32_t branch_offset =
            (int32_t)(int64_t)(next_tb->tc.ptr - (mips_branch_inst_addr + 4));
        branch_offset >>= 2;
        if (int32_in_int16(branch_offset)) {
            uint32_t branch_inst = *(uint32_t *)mips_branch_inst_addr;
            etb->mips_branch_backup = branch_inst;
            // mips_bne, mips_beq and mips_x86_jcc
            if (((branch_inst & 0xf3000000) == 0x10000000) ||
                ((branch_inst & 0xf3000000) == 0x14000000)) {
                *(uint32_t *)mips_branch_inst_addr =
                    (branch_inst & 0xffff0000) | (branch_offset & 0xffff);
            } else if ((branch_inst & 0xfc00003f) == 0x70000028) {
                *(uint32_t *)mips_branch_inst_addr =
                    (branch_inst & 0xffc0003f) |
                    ((branch_offset & 0xffff) << 6);
            } else {
                lsassert(0);
            }
            flush_icache_range(mips_branch_inst_addr,
                               mips_branch_inst_addr + 4);
            etb->branch_to_target_direct_in_mips_branch = 1;
            return 1;
        }
    }
#endif
    return 0;
}

/* check fpu rotate and patch the native jump address */
void x86_to_mips_tb_set_jmp_target(TranslationBlock *tb, int n,
                                   TranslationBlock *next_tb)
{
    lsassert(next_tb != NULL);
    tb->extra_tb->next_tb[n] = next_tb;

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_FAST_CS)
    /* lsfpu is not supported yet */
    ETB *etb1 = tb->extra_tb;
    ETB *etb2 = next_tb->extra_tb;

    int flag = fastcs_tblink_flag(etb1, etb2);
    if (flag == FCS_LINK_DRT) {
        if(n == 0 || !try_fast_link(tb, next_tb)){
            tb_set_jmp_target(tb, n, (uintptr_t)next_tb->tc.ptr);
        }
    } else {
        switch (flag) {
        case FCS_LINK_ALL:
            tb_set_jmp_target(tb, n,
                    n ? native_jmp_glue_1 : native_jmp_glue_0);
            break;
        case FCS_LINK_FPU:
            tb_set_jmp_target(tb, n,
                    n ? jmp_glue_fastcs_fpu_1 : jmp_glue_fastcs_fpu_0);
            break;
        case FCS_LINK_XMM:
            tb_set_jmp_target(tb, n,
                    n ? jmp_glue_fastcs_xmm_1 : jmp_glue_fastcs_xmm_0);
            break;
        default:
            break;
        }
    }
#else
    if (option_lsfpu || tb->extra_tb->_top_out == next_tb->extra_tb->_top_in) {
#ifdef CONFIG_SOFTMMU
        /* Cross-Page-Check for direct TB-Link */
        int cpc0 = tb->extra_tb->tb_need_cpc[0];
        int cpc1 = tb->extra_tb->tb_need_cpc[1];
        if (xtm_cpc_enabled() &&
                ((tb->flags & 0x3) == 3) &&
                (cpc0 | cpc1)) {
            if (n == 0) {
                tb_set_jmp_target(tb, 0,
                        cpc0 ? native_jmp_glue_cpc_0 :
                               (uintptr_t)next_tb->tc.ptr);
            } else {
                tb_set_jmp_target(tb, 1,
                        cpc1 ? native_jmp_glue_cpc_1 :
                               (uintptr_t)next_tb->tc.ptr);
            }
        } else
#endif
        /* direct TB-Link */
        if(n == 0 || !try_fast_link(tb, next_tb)) {
            tb_set_jmp_target(tb, n, (uintptr_t)next_tb->tc.ptr);
        }
    } else {
        if (option_dump)
            fprintf(stderr, "%p %p(%x) %s to %p %p(%x)\n", tb, tb->tc.ptr,
                    tb->pc, n ? "jmp" : "fallthrough", next_tb, next_tb->tc.ptr,
                    next_tb->pc);
        if (n == 0)
            tb_set_jmp_target(tb, 0, native_jmp_glue_0);
        else
            tb_set_jmp_target(tb, 1, native_jmp_glue_1);
    }
#endif
}

#ifdef CONFIG_SOFTMMU

// FIXME comment this, vregs defined in CPUX86State ?
/* This is used in target/i386/fpu_helper.c */
int xtm_get_top_bias_from_env(CPUX86State *env) { return (int)env->vregs[3]; }

void qm_flush_icache(ADDR s, ADDR e)
{
    flush_icache_range((uintptr_t)s, (uintptr_t)e);
}

void qm_tb_clr_ir1(TranslationBlock *tb)
{
    if (tb) {
#ifdef CONFIG_XTM_MD_CAPSTONE
        xtm_reset_capstione_mem();
#else
        int i = 0;
        for (i = 0; i < tb->extra_tb->_ir1_num; ++i) {
            ir1_free_info(tb->extra_tb->_ir1_instructions + i);
        }
#endif
        tb->extra_tb->_ir1_instructions = NULL;
        tb->extra_tb->_ir1_num = 0;
    }
}

#define XTM_SIGINT_SIGNAL 63

static uint64_t code_buffer_lo = 0;
static uint64_t code_buffer_hi = 0;

void xtm_tb_unlink(TranslationBlock *ctb)
{
    if (!ctb) return;

    uintptr_t addr = 0;

    if (ctb->jmp_reset_offset[0] != TB_JMP_RESET_OFFSET_INVALID) {
        addr = (uintptr_t)(ctb->tc.ptr + ctb->jmp_reset_offset[0]);
        tb_set_jmp_target(ctb, 0, addr);
    }

    if (xtm_branch_opt() && ctb->extra_tb->branch_to_target_direct_in_mips_branch == 1) {
        uint32_t *addr =
            (uint32_t *)(ctb->tc.ptr + ctb->extra_tb->mips_branch_inst_offset);
        *addr = ctb->extra_tb->mips_branch_backup;
        ctb->extra_tb->mips_branch_backup = 0;
        ctb->extra_tb->branch_to_target_direct_in_mips_branch = 0;
        flush_icache_range((uintptr_t)addr, (uintptr_t)addr + 4);
        return;
    }

    if (ctb->jmp_reset_offset[1] != TB_JMP_RESET_OFFSET_INVALID) {
        addr = (uintptr_t)(ctb->tc.ptr + ctb->jmp_reset_offset[1]);
        tb_set_jmp_target(ctb, 1, addr);
    }
}

void xtm_tb_relink(TranslationBlock *utb)
{
    if (!utb) return;

    TranslationBlock *utb_next;
    utb_next = utb->extra_tb->next_tb[0];
    if (utb_next && !(tb_cflags(utb_next) & CF_INVALID)) {
        x86_to_mips_tb_set_jmp_target(utb, 0, utb_next);
    }

    utb_next = utb->extra_tb->next_tb[1];
    if (utb_next && !(tb_cflags(utb_next) & CF_INVALID)) {
        x86_to_mips_tb_set_jmp_target(utb, 1, utb_next);
    }
}

static
void xtm_interrupt_signal_handler(
        int n, siginfo_t *siginfo, void *ctx)
{
    if (!xtm_sigint_opt()) return;

// FIXME comment the code related with signal handler
// maybe we have a better way to handling this
#if 0
    ucontext_t *uc = ctx;

#if defined(CONFIG_LATX)
    uintptr_t pc = (uintptr_t)uc->uc_mcontext.__pc;
#elif defined(CONFIG_X86toMIPS)
    uintptr_t pc = (uintptr_t)uc->uc_mcontext.pc;
#else
#error should configure with x86tomips or latx
#endif

    if (code_buffer_lo <= pc && pc <= code_buffer_hi) {
        TranslationBlock *ctb = tcg_tb_lookup(pc);

        TranslationBlock *oldtb = lsenv->sigint_data.tb_unlinked;
        if (oldtb == ctb) {
            /* This TB is already unlinked */
            return;
        } else {
            /* Prev TB is unlinked and not relinked */
            xtm_tb_relink(oldtb);
        }

        lsenv->sigint_data.tb_unlinked = ctb;
        xtm_tb_unlink(ctb);
    }
#endif
}

// FIXME this is a temporary fix
#include "../../tcg/tcg.h"

// FIXME no more signal anymore
void x86_to_mips_init_thread_signal(CPUState *cpu)
{
    if (!xtm_sigint_opt()) return;

    /* 1. unblock the signal for vCPU thread */
    sigset_t set, oldset;
    sigemptyset(&set);
    sigaddset(&set, XTM_SIGINT_SIGNAL);

    int ret = pthread_sigmask(SIG_UNBLOCK, &set, &oldset);
    if (ret < 0) {
        fprintf(stderr, "[Signal Interrupt] unblock USR1 failed\n");
        exit(-1);
    }

    /* 2. set the handler for signal */

    struct sigaction act;
    struct sigaction old_act;

    memset(&act, 0, sizeof(act));
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = xtm_interrupt_signal_handler;

    ret = sigaction(XTM_SIGINT_SIGNAL, &act, &old_act);
    if (ret < 0) {
        fprintf(stderr, "[Signal Interrupt] set handler failed\n");
        exit(-1);
    }

    pthread_t tid = pthread_self();
    fprintf(stderr, "[Signal] thread %x set signal %d handler\n",
            (unsigned int)tid, XTM_SIGINT_SIGNAL);

    /* 3. set the range of code buffer */
    code_buffer_lo = (uint64_t)tcg_ctx->code_gen_buffer;
    code_buffer_hi = (uint64_t)tcg_ctx->code_gen_buffer +
                     (uint64_t)tcg_ctx->code_gen_buffer_size;

    fprintf(stderr, "[Signal] monitor code buffer %llx to %llx\n",
            (unsigned long long)code_buffer_lo,
            (unsigned long long)code_buffer_hi);
}

#else /* CONFIG_SOFTMMU */

void x86_to_mips_init_thread_signal(CPUState *cpu)
{
    /* empty function in user-mode */
}

#endif
