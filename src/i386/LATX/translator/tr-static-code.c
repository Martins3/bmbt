#include "../include/common.h"
#include "../include/env.h"
#include "../include/etb.h"
#include "../ir2/ir2.h"
#include "../ir1/ir1.h"
#include "../include/reg-alloc.h"

#include "../x86tomips-options.h"
#include "../x86tomips-config.h"

#include "../include/flag-lbt.h"

#include "../include/ibtc.h"
#include "../include/profile.h"
#include "../include/flag-pattern.h"
#include "../include/shadow-stack.h"

#include <string.h>

ADDR ss_match_fail_native;

/*To reuse qemu's tb chaining code, we take the same way of epilogue treating,
 *context_switch_native_to_bt_ret_0 and context_switch_native_to_bt share code.
 *
 * context_swtich_native_to_bt_ret_0:
 *    mov v0, zer0
 * context_switch_native_to_bt:
 *    other instructions
 *    ...
 * to support chaining, we generate code roughly like below:
 *
 * 1. unconditional jmps and normal branches
 *
 *   <tb_ptr = tb | <succ_id> or exit flag>
 *   j <pc> + 8 #will be rewritten in tb_add_jump to target tb's native code
 *   nop
 *   j context_switch_native_to_bt
 *   mov v0, tb_ptr
 *
 * 2. indirection jmps and ret
 *
 *   <save pc to env>
 *   <save mapped regs>
 *   load t9, &helper_lookup_tb_ptr
 *   jalr t9
 *   nop
 *   <allocate tmp reg>
 *   mov tmp, v0
 *   <recover mapped regs>
 *   jr tmp #tmp == target tb code addr or context_switch_native_bt_ret_0
 *   nop
 *
 *When context_switch_native_to_bt return to qemu code, ret address will be:
 * 1. if chaining is ok: last executed TB | flags
 * 2. zero | flags
 *
 * flags is <succid> for now
 */

ADDR context_switch_bt_to_native;
ADDR context_switch_native_to_bt_ret_0;
ADDR context_switch_native_to_bt;

#ifdef CONFIG_SOFTMMU
/* Static codes to context switch for helper in system-mode
 * Temp registers will be used to save/restore context.
 *
 * Usage: jalr $t9 */
ADDR sys_helper_prologue_default;
ADDR sys_helper_epilogue_default;
#endif

ADDR xqm_break_point_code;

/* usage: native_rotate_fpu_by(step, return_address) */
ADDR native_rotate_fpu_by;
ADDR native_jmp_glue_0;
ADDR native_jmp_glue_1;
ADDR native_jmp_glue_2;
#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_FAST_CS)
ADDR jmp_glue_fastcs_fpu_0;
ADDR jmp_glue_fastcs_fpu_1;
ADDR jmp_glue_fastcs_xmm_0;
ADDR jmp_glue_fastcs_xmm_1;
#endif

ADDR fpu_enable_top;
ADDR fpu_disable_top;
ADDR fpu_get_top;
ADDR fpu_set_top;
ADDR fpu_inc_top;
ADDR fpu_dec_top;

/* Enter context switch bt_to_native use function call:
 * $a0: TB's generate code start address
 * $a1: Address of CPUX86State */
void generate_context_switch_bt_to_native(void *code_buf)
{
//    lsassertm(0, "gen context switch to be implemented in LoongArch.\n");
    int i = 0;
    /* 1. save DBT's context in stack, so allocate space on the stack */
    append_ir2_opnd2i(LISA_ADDI_D, &sp_ir2_opnd,
                                   &sp_ir2_opnd, -256);

    /* NOTE: leave some space in the dbt stack, in case that dbt functions */
    /*       called by native code need to store function arguments onto stack. */
    const int extra_space = 40;

    /* 1.1 save callee-saved registers. s0-s7 ($23-$31) */
    IR2_OPND reg_opnd = ir2_opnd_new_inv();
    for (i = 0; i <= 8; ++i) {
        reg_opnd = ir2_opnd_new(IR2_OPND_GPR, i + 23);
        append_ir2_opnd2i(LISA_ST_D, &reg_opnd,
                                     &sp_ir2_opnd, extra_space + (i << 3));
    }
    /* 1.2 save fp($22), and ra($1) */
    append_ir2_opnd2i(LISA_ST_D, &fp_ir2_opnd,
                                 &sp_ir2_opnd, extra_space + 72);
    append_ir2_opnd2i(LISA_ST_D, &ra_ir2_opnd,
                                 &sp_ir2_opnd, extra_space + 80);

    /* 1.3. save DBT FCSR (#31) */
    IR2_OPND fcsr_value_opnd = ra_alloc_itemp();
    append_ir2_opnd2(LISA_MOVFCSR2GR, &fcsr_value_opnd, &fcsr_ir2_opnd);
    append_ir2_opnd2i(LISA_ST_D, &fcsr_value_opnd,
                                 &sp_ir2_opnd, extra_space + 88);

    /* 2. move the arguments a0 and a1 to its places */

//    /* native address of target block is passed in $a0, while it might be used
//     * as temp register later. So we just save it to $t9.
//     *
//     * use $s2 to save the address of CPUX86State, which is store in $a1 */
//
//    /* 2.1 t9: TB's start address in code cache */
//    IR2_OPND native_addr_opnd = ir2_opnd_new(IR2_OPND_IREG, 25);
//    _reg_opnd = ir2_opnd_new(IR2_OPND_IREG, 4);
//    append_ir2_opnd2(mips_mov64, &native_addr_opnd, &_reg_opnd);
    /* 2.2 s0: CPUX86State */
    append_ir2_opnd3(LISA_OR, &env_ir2_opnd,
                              &arg1_ir2_opnd, &zero_ir2_opnd);
//    ir2_opnd_set_em(&env_ir2_opnd, EM_MIPS_ADDRESS, 32);

    /* 3. set native codes' execution context */

    /* 3.1 set native code FCSR (#31) */
    IR2_OPND temp_opnd = ra_alloc_itemp();
//    append_ir2_opnd2i(LISA_ADDI_D, &temp_opnd, &zero_ir2_opnd, 0);
    append_ir2_opnd3(LISA_OR, &fcsr_value_opnd, &fcsr_value_opnd, &zero_ir2_opnd);
    append_ir2_opnd2(LISA_MOVGR2FCSR, &fcsr_ir2_opnd, &fcsr_value_opnd);
//    append_ir2_opnd1i(mips_lui, &temp_opnd, 1 << 8);
//    append_ir2_opnd3(mips_or, &fcsr_value_opnd, &fcsr_value_opnd, &temp_opnd);
//    append_ir2_opnd2(mips_ctc1, &fcsr_value_opnd, &fcsr_ir2_opnd);

//    /* 3.2 set s0 to 0xffffffff and set f3 = 32 */
//    load_imm32_to_ir2(&n1_ir2_opnd, -1, ZERO_EXTENSION);
    load_imm32_to_ir2(&temp_opnd, 32, SIGN_EXTENSION);
    append_ir2_opnd2(LISA_MOVGR2FR_D, &f32_ir2_opnd, &temp_opnd);

//    /* 3.3 load guest base into t2 */
//#ifndef CONFIG_SOFTMMU /* guset-base is needed in user-mode only */
//    ADDR guest_base = cpu_get_guest_base();
//    if (guest_base != 0) {
//        IR2_OPND gbase_opnd = ra_alloc_guest_base();
//        load_addr_to_ir2(&gbase_opnd, guest_base);
//    } else {
//        /* store high 32bit of the code buffer address, we will use it to
//           extend 32 bit tb into 64 bits
//         */
//    #ifdef N64
//        ADDR tb_high32 = ((ADDR)code_buf) >> 32;
//        IR2_OPND gbase_opnd = ra_alloc_guest_base();
//        load_imm32_to_ir2(&gbase_opnd, tb_high32, UNKNOWN_EXTENSION);
//        append_ir2_opnd2i(mips_dsll32, &gbase_opnd, &gbase_opnd, 0);
//    #endif
//    }
//    if (option_dump)
//        fprintf(stderr, "[X86toMIPS] guest base = %p\n", (void *)guest_base);
//#endif
//
    /* 3.4 load x86 registers from env. top, eflags, and ss */
    tr_load_registers_from_env(0xff, 0xff, 1, 0xff, 0xff, 0x00);

    /* 3.5. load eflags */
#ifndef CONFIG_SOFTMMU
//    /* Load eflags here in user-mode. */
//    if (option_lbt) {
//        append_ir2_opnd2i(mips_lw, &eflags_ir2_opnd, &env_ir2_opnd, lsenv_offset_of_eflags(lsenv));
//        append_ir2_opnd1i(mips_mtflag, &eflags_ir2_opnd, 0x3f);
//        append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x400);
//    } else {
//        append_ir2_opnd2i(mips_lw, &eflags_ir2_opnd, &env_ir2_opnd,
//                          lsenv_offset_of_eflags(lsenv));
//    }
#else
    /* Load eflags here in system-mode. */
    tr_load_eflags(1);
#endif

#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
    tr_pf_inc_cs_in();
#endif

    /* 4. jump to native code address (saved in a0) */
    append_ir2_opnd2i(LISA_JIRL, &zero_ir2_opnd, &arg0_ir2_opnd, 0);
}

void generate_context_switch_native_to_bt(void *code_buf)
{
//    lsassertm(0, "gen context switch to be implemented in LoongArch.\n");
    /* 0. by default set v0 to zero. context_switch_native_to_bt_ret_0 points here */
    append_ir2_opnd3(LISA_OR, &ret0_ir2_opnd, &zero_ir2_opnd, &zero_ir2_opnd);
//    IR2_OPND mips_ret_opnd = ir2_opnd_new(IR2_OPND_IREG, 2); /* v0 */
//    append_ir2_opnd2(mips_mov64, &mips_ret_opnd, &zero_ir2_opnd);

    /* context_switch_native_to_bt point to this position,
     * make sure $10(a6)/$11(a7) is set
     *
     * $10(a6) : this TB
     * $11(a7) : next EIP
     * */

    /* 1. store the last executed TB (int $10) into env */
    IR2_OPND tb_ptr_opnd = ra_alloc_dbt_arg1();
    append_ir2_opnd2i(LISA_ST_D, &tb_ptr_opnd, &env_ir2_opnd,
                      lsenv_offset_of_last_executed_tb(lsenv));

    /* 2. store eip (in $25) into env */
    IR2_OPND eip_opnd = ra_alloc_dbt_arg2();
    append_ir2_opnd2i(LISA_ST_W, &eip_opnd, &env_ir2_opnd,
                      lsenv_offset_of_eip(lsenv));

    int save_top = 0;
    if (option_lsfpu) save_top = 1;
    tr_save_registers_to_env(0xff, 0xff, save_top, 0xff, 0xff, 0x00);

    /* 4. save eflags */
#ifndef CONFIG_SOFTMMU
    /* Save eflags here in user-mode. */
    if (option_lbt) {
        IR2_OPND eflags_temp = ra_alloc_itemp();
        append_ir2_opnd1i(LISA_X86MFFLAG, &eflags_temp, 0x3f);
        append_ir2_opnd3(LISA_OR, &eflags_ir2_opnd, &eflags_ir2_opnd, &eflags_temp);
        ra_free_temp(&eflags_temp);
    }
    append_ir2_opnd2i(LISA_ST_W, &eflags_ir2_opnd, &env_ir2_opnd,
                      lsenv_offset_of_eflags(lsenv));
#else
    /* Save eflags here in system-mode. */
    tr_save_eflags();
#endif

    /* 5. restore bt's context */
    const int extra_space = 40;
    /* See the note in generate_context_switch_bt_to_native */

    /* 5.1. restore DBT FCSR (#31) */
    IR2_OPND fcsr_value_opnd = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_LD_D, &fcsr_value_opnd,
                                 &sp_ir2_opnd, extra_space + 88);
    append_ir2_opnd2(LISA_MOVGR2FCSR, &fcsr_ir2_opnd, &fcsr_value_opnd);

    /* 5.2 restore fp($22) and ra($1) */
    append_ir2_opnd2i(LISA_LD_D, &ra_ir2_opnd,
                                 &sp_ir2_opnd, extra_space + 80);
    append_ir2_opnd2i(LISA_LD_D, &fp_ir2_opnd,
                                 &sp_ir2_opnd, extra_space + 72);

    /* 5.3 restore callee-saved registers. s0-s7 ($16-$23) */
    IR2_OPND reg_opnd = ir2_opnd_new_inv();
    int i = 0;
    for (i = 0; i <= 8; ++i) {
        reg_opnd = ir2_opnd_new(IR2_OPND_GPR, i + 23);
        append_ir2_opnd2i(LISA_LD_D, &reg_opnd,
                                     &sp_ir2_opnd, extra_space + (i << 3));
    }

    /* 5.4 restore sp */
    append_ir2_opnd2i(LISA_ADDI_D, &sp_ir2_opnd, &sp_ir2_opnd, 256);

#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
    tr_pf_inc_cs_out();
#endif

    /* 5.5 return */
    append_ir2_opnd2i(LISA_JIRL, &zero_ir2_opnd, &ra_ir2_opnd, 0);
}

/* code_buf: start code address
 * n = 0: direct fall through jmp
 * n = 1: direct taken jmp
 * n = 2: indirect jmps
 */
static int generate_native_jmp_glue(void *code_buf, int n)
{
//    lsassertm(0, "xxx to be implemented in LoongArch.\n");
    int lisa_num = 0;

    /* to calculate the offset of LISA_B/BL */
    int start = (lsenv->tr_data->real_ir2_inst_num << 2);
    int offset = 0;

    tr_init(NULL);

    IR2_OPND tb = ra_alloc_dbt_arg1();
//#ifndef CONFIG_SOFTMMU
//    if (cpu_get_guest_base() == 0) {
//        IR2_OPND tb_high32 = ra_alloc_guest_base();
//        append_ir2_opnd3(mips_or, &tb, &tb, &tb_high32);
//        ir2_opnd_set_em(&tb, EM_MIPS_ADDRESS, 32);
//    }
//#endif

    IR2_OPND tmp_opnd = ra_alloc_itemp();
    /* Next EIP passed to context switch */
    IR2_OPND eip_opnd = ra_alloc_dbt_arg2();
    /* @param0: next TB's top_out
     * @param1: this TB's top_in  */
    IR2_OPND param0_opnd = ra_alloc_itemp();
    IR2_OPND param1_opnd = ra_alloc_itemp();

    /* arguments passed to native rotate FPU
     * DBT arg 1 : FPU rotate step
     * DBT arg 2 : next TB's native codes
     */
    IR2_OPND step_opnd = ra_alloc_dbt_arg1();
    IR2_OPND tb_nc_opnd = ra_alloc_dbt_arg2();
//#ifdef CONFIG_SOFTMMU
//#if defined(CONFIG_XTM_FAST_CS) || defined(CONFIG_XTM_PROFILE)
//    IR2_OPND cmp_opnd = ir2_opnd_new(IR2_OPND_IREG, 6); /* For fast-cs-mask compare */
//#endif
//#endif

    /* @ret_opnd: next TB
     *
     * > for direct jump, next TB is loaded from this TB's ETB
     * > for indirect jump, next TB is obtained through:
     *   > CAM lookup
     *   > Native Jmp Cache lookup
     *   > helper_lookup_tb()
     */
    IR2_OPND ret_opnd = ret0_ir2_opnd;
    if (n == 0 || n == 1) {
        /* load tb->extra_tb.next_tb[n] into v0. */
//        ir2_opnd_set_em(&tb, EM_MIPS_ADDRESS, 32);
        /* 1. load extra_tb's address */
        append_ir2_opnd2i(LISA_LD_D, &tmp_opnd, &tb,
                          offsetof(TranslationBlock, extra_tb));
//        ir2_opnd_set_em(&tmp_opnd, EM_MIPS_ADDRESS, 32);
        /* 2. load next_tb */
        append_ir2_opnd2i(LISA_LD_D, &ret_opnd, &tmp_opnd,
                              offsetof(struct ExtraBlock, next_tb) +
                              n * sizeof(void *));
//#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_FAST_CS)
//#ifdef CONFIG_XTM_PROFILE
//        tr_pf_inc_fastcs_glue_11();
//#endif
//#endif
    } else {
        /* Indirect jmp lookup TB */
        IR2_OPND label_next_tb_exist = ir2_opnd_new_label();
//#ifndef CONFIG_SOFTMMU /* enable IBTC in user-mode only */
//        IR2_OPND label_miss = ir2_opnd_new_type(IR2_OPND_LABEL);
//        IR2_OPND label_hit  = ir2_opnd_new_type(IR2_OPND_LABEL);
//        IR2_OPND ibt_idx = ra_alloc_itemp();
//        IR2_OPND addr_opnd = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_andi, &ibt_idx, &tmp_opnd, IBTC_MASK);
//        append_ir2_opnd2i(mips_dsll, &ibt_idx, &ibt_idx, 4);
//        load_addr_to_ir2(&addr_opnd, (ADDR)ibtc_table);
//        append_ir2_opnd3(mips_add_addr, &addr_opnd, &addr_opnd, &ibt_idx);
//        IR2_OPND eip_opnd = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_lwu, &eip_opnd, &addr_opnd, 0);
//        append_ir2_opnd3(mips_bne, &eip_opnd, &tmp_opnd, &label_miss);
//        if (option_profile) {
//            void *p_execution_times = &ibtc_hit_cnt;
//            IR2_OPND exec_count_addr_opnd = ra_alloc_itemp();
//            load_addr_to_ir2(&exec_count_addr_opnd, (ADDR)p_execution_times);
//            IR2_OPND exec_count_value_opnd = ra_alloc_itemp();
//            append_ir2_opnd2i(mips_ld, &exec_count_value_opnd, &exec_count_addr_opnd, 0);
//            append_ir2_opnd2i(mips_daddiu, &exec_count_value_opnd, &exec_count_value_opnd, 1);
//            append_ir2_opnd2i(mips_sdi, &exec_count_value_opnd, &exec_count_addr_opnd, 0);
//            ra_free_temp(&exec_count_addr_opnd);
//            ra_free_temp(&exec_count_value_opnd);
//        }
//        append_ir2_opnd2i(mips_load_addr, &ret_opnd, &addr_opnd, 8);
//        append_ir2_opnd1(mips_b, &label_hit);
//        append_ir2_opnd1(mips_label, &label_miss);
//#endif

#ifdef CONFIG_SOFTMMU
        /* $a6: prev TB           @tb         @step_opnd
         * $a7: EIP for next TB   @eip_opnd
         *
         * tmp: free to use here  @param0_opnd
         * tmp: free to use here  @param1_opnd
         * tmp: free to use here  @cmp_opnd
         *      > used to verify next TB
         *      > used to record monitor data
         *
         * $v0: next TB (lookup result)       @ret_opnd
         *      > valid after TB lookup
         */

//        IR2_OPND check_tb_valid = ir2_opnd_new_type(IR2_OPND_LABEL);
        IR2_OPND njc_lookup_miss = ir2_opnd_new_label();
        int need_check_tb = xtm_tblookup_opt() || xtm_njc_opt();

        /* L1: CAM lookup. Enabled by option */
        if (!xtm_tblookup_opt()) goto _TB_LOOKUP_L2_;

        lsassertm(0, "CAM not supported in LoongArch.\n");

//#ifdef CONFIG_XTM_PROFILE
//        if (option_monitor_jc) {
//            tr_pf_inc_jc_cam(&param0_opnd, &param1_opnd);
//            /* Mark to indicate this is an CAM lookup */
//            append_ir2_opnd2i(mips_ori, &cmp_opnd, &zero_ir2_opnd, 0x0);
//        }
//#endif
//
//        /* CAM lookup through campv : get the next TB's address */
//        append_ir2_opnd2(mips_campv, &tmp_opnd, &ret_opnd);
//        /* CAM lookup fail, go to L2 lookup */
//        IR2_OPND cam_lookup_miss = ir2_opnd_new_type(IR2_OPND_LABEL);
//        if (xtm_njc_opt()) {
//            append_ir2_opnd3(mips_beq, &ret_opnd, &zero_ir2_opnd,
//                    &cam_lookup_miss);
//            append_ir2_opnd1(mips_b, &check_tb_valid);
//        } else {
//            append_ir2_opnd3(mips_beq, &ret_opnd, &zero_ir2_opnd,
//                    &njc_lookup_miss);
//        }
//
//        append_ir2_opnd1(mips_label, &cam_lookup_miss);

_TB_LOOKUP_L2_:

        /* L2: Native Jmp Cache lookup, Enabled by option */
        if (!xtm_njc_opt()) goto _CHECL_TB_VALID_;

//#ifdef CONFIG_XTM_PROFILE
//        if (option_monitor_jc) {
//            tr_pf_inc_jc_njc(&param0_opnd, &param1_opnd);
//            /* Mark to indicate this is a NJC lookup */
//            append_ir2_opnd2i(mips_ori, &cmp_opnd, &zero_ir2_opnd, 0x1);
//        }
//#endif

        load_addr_to_ir2(&tmp_opnd, njc_lookup_tb);
        append_ir2_opnd1_(lisa_call, &tmp_opnd);
        append_ir2_opnd3(LISA_BEQ, &ret_opnd, &zero_ir2_opnd, &njc_lookup_miss);

        /* L2: check if PC == TB.PC */
//        ir2_opnd_set_em(&ret_opnd, EM_MIPS_ADDRESS, 32);
        append_ir2_opnd2i(LISA_LD_WU, &param0_opnd, &ret_opnd, offsetof(TranslationBlock, pc));
        append_ir2_opnd3(LISA_BNE, &param0_opnd, &eip_opnd, &njc_lookup_miss);

_CHECL_TB_VALID_:
//        /* check tb valid */
//        if (xtm_njc_opt())
//            append_ir2_opnd1(LISA_LABEL, &check_tb_valid);

        if (!need_check_tb) goto _TB_LOOKUP_LL_;

        /* check next TB cflags CF_INVALID
         *
         * For normal indirect jmp, it can's modify cs_base, hflags, ...
         * So we only check if TB is invalid */
        append_ir2_opnd2i(LISA_LD_WU, &param0_opnd, &ret_opnd,
                offsetof(TranslationBlock, cflags));
        append_ir2_opnd2i(LISA_SRAI_D, &param0_opnd, &param0_opnd, 16);
        append_ir2_opnd2i(LISA_ANDI, &param0_opnd, &param0_opnd, (CF_INVALID >> 16));
        append_ir2_opnd3(LISA_BNE, &param0_opnd, &zero_ir2_opnd, &njc_lookup_miss);

        /* check next TB's CSBASE */
        append_ir2_opnd2i(LISA_LD_WU, &param0_opnd, &ret_opnd,
                offsetof(TranslationBlock, cs_base));
        append_ir2_opnd2i(LISA_LD_WU, &param1_opnd, &env_ir2_opnd,
                offsetof(CPUX86State, segs[R_CS].base));
        append_ir2_opnd3(LISA_BNE, &param0_opnd, &param1_opnd, &njc_lookup_miss);

        /* check next TB's flags */
        /* *flags = env->hflags |
        (env->eflags & (IOPL_MASK | TF_MASK | RF_MASK | VM_MASK | AC_MASK)); */
        load_imm64_to_ir2(&param0_opnd, (IOPL_MASK | TF_MASK | RF_MASK | VM_MASK | AC_MASK));
        append_ir2_opnd3(LISA_AND, &param0_opnd, &eflags_ir2_opnd, &param0_opnd);
        append_ir2_opnd2i(LISA_LD_WU, &param1_opnd, &env_ir2_opnd,
                offsetof(CPUX86State, hflags));
        append_ir2_opnd3(LISA_OR, &param1_opnd, &param1_opnd, &param0_opnd);
        /* tb->flags */
        append_ir2_opnd2i(LISA_LD_WU, &param0_opnd, &ret_opnd,
                offsetof(TranslationBlock, flags));
        append_ir2_opnd3(LISA_BNE, &param0_opnd, &param1_opnd, &njc_lookup_miss);

//#ifdef CONFIG_XTM_PROFILE
//        /* @cmp_opnd == 0 : CAM lookup
//         *           == 1 : NJC lookup */
//        if (option_monitor_jc) {
//            IR2_OPND xtm_pf_not_cam_lookup_label = ir2_opnd_new_type(IR2_OPND_LABEL);
//            append_ir2_opnd3(mips_bne, &cmp_opnd, &zero_ir2_opnd, &xtm_pf_not_cam_lookup_label);
//
//            tr_pf_inc_jc_cam_hit(&param0_opnd, &param1_opnd);
//            append_ir2_opnd1(mips_b, &label_next_tb_exist);
//
//            append_ir2_opnd1(mips_label, &xtm_pf_not_cam_lookup_label);
//
//            tr_pf_inc_jc_njc_hit(&param0_opnd, &param1_opnd);
//        }
//#endif

        /* TB lookup success, finish lookup */
        append_ir2_opnd1(LISA_B, &label_next_tb_exist);

        append_ir2_opnd1(LISA_LABEL, &njc_lookup_miss);

_TB_LOOKUP_LL_:
#endif /* CONFIG_SOFTMMU */

        /* LL: helper_lookup_tb
         *
         * we know that no fp will be touched in this helper, so we save
         * only gpr and vpr(guest base/$24)
         */
//        if (option_shadow_stack) {
//            tr_save_registers_to_env(GPR_USEDEF_TO_SAVE, 0, 0, 0, 0, 0x13);
//        } else {
            if (option_lsfpu) {
                tr_gen_save_curr_top();
                tr_fpu_disable_top_mode();
            }
            /* not save curr_top since we are static codes
             * curr_top will be updated dynamically before jmp_glue_2 */
            tr_save_registers_to_env(0xff, 0xff, 0, 0xff, 0xff, 0x3);
//        }
#ifdef CONFIG_SOFTMMU
        tr_save_eflags();
#endif
        /* calling helper_lookup_tb */
//        /* this call migth trash a0, so put a0 load in delay slot */
        load_addr_to_ir2(&tmp_opnd, (ADDR)helper_lookup_tb);
        append_ir2_opnd2_(lisa_mov,  &arg0_ir2_opnd, &env_ir2_opnd);
        append_ir2_opnd2i(LISA_JIRL, &ra_ir2_opnd, &tmp_opnd, 0);
//        append_ir2_opnd1_not_nop(mips_jalr, &tmp_opnd);
//        /* arg0: env * */
//        append_ir2_opnd2(mips_mov64, &param0_opnd, &env_ir2_opnd);
//        /* returned next_tb is in v0, which will not be destroyed by this load
//         */
//        if (option_shadow_stack) {
//            tr_load_registers_from_env(GPR_USEDEF_TO_SAVE, 0, 0, 0, 0, 0x13);
//        } else {
            tr_load_registers_from_env(0xff, 0xff, 1, 0xff, 0xff, 0x3);
//        }
        append_ir2_opnd3(LISA_BNE, &ret_opnd, &zero_ir2_opnd, &label_next_tb_exist);
        /* if next_tb == NULL, jump to epilogue */
//        /* clear v0 */
//        append_ir2_opnd2i(mips_ori, &ret_opnd, &zero_ir2_opnd, 0);
//        /* load back eip to t9 */
        append_ir2_opnd2i(LISA_LD_WU, &eip_opnd, &env_ir2_opnd,
                          lsenv_offset_of_eip(lsenv));
        offset = (lsenv->tr_data->real_ir2_inst_num << 2) - start;
        append_ir2_opnda(LISA_B, (context_switch_native_to_bt_ret_0
                                  - (ADDR)code_buf - offset) >> 2);
        /* else compare tb->top_out and next_tb->top_in */
        /* next_tb_exist: */
        append_ir2_opnd1(LISA_LABEL, &label_next_tb_exist);
//
//#ifndef CONFIG_SOFTMMU /* enable IBTC in user-mode only */
//        append_ir2_opnd2i(mips_load_addr, &tmp_opnd, &env_ir2_opnd,
//                          lsenv_offset_of_eip(lsenv));
//        append_ir2_opnd2i(mips_andi, &ibt_idx, &tmp_opnd, IBTC_MASK);
//        append_ir2_opnd2i(mips_dsll, &ibt_idx, &ibt_idx, 4);
//        load_addr_to_ir2(&addr_opnd, (ADDR)ibtc_table);
//        append_ir2_opnd3(mips_add_addr, &addr_opnd, &addr_opnd, &ibt_idx);
//        append_ir2_opnd2i(mips_sdi, &tmp_opnd, &addr_opnd, 0);
//        append_ir2_opnd2i(mips_sdi, &ret_opnd, &addr_opnd, 8);
//        append_ir2_opnd1(mips_label, &label_hit);
//#endif
//        /* ensure is address. tr_save/load will destroy t8's status while keep its content */
//        ir2_opnd_set_em(&tb, EM_MIPS_ADDRESS, 32);
//        ir2_opnd_set_em(&ret_opnd, EM_MIPS_ADDRESS, 32);
    }
    if (!option_lsfpu) {
//#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_FAST_CS)
//        /* jmp glue for fast-cs that allows all kinds of TB-link
//         *
//         * TB1 -> TB2
//         * TB1: $t8, @tb
//         * TB2: $v0, @ret_opnd
//         *
//         * 1. read fast-cs-mask
//         *    mask1: TB1.fast-cs-mask: @param1_opnd
//         *    mask2: TB2.fast-cs-mask: @param0_opnd
//         * 2. load XMM if mask1[1] == 0 && mask2[1] == 1
//         * 3. load FPU if mask1[0] == 0 && mask2[0] == 1 and FPU rotate check, goto 5.
//         * 4. FPU rotate check according to TB1.top_out and TB2.top_in
//         * 5. goto TB2 */
//
//        /* 1.1 mask1: param1_opnd = TB1.fast_cs_mask = CPUX86State.vreg[5] */
//        append_ir2_opnd2i(mips_lbu, &param1_opnd, &env_ir2_opnd,
//                lsenv_offset_of_fast_cs_mask(lsenv));
//        append_ir2_opnd2i(mips_andi, &param1_opnd, &param1_opnd, XTM_FAST_CS_MASK);
//        /* 1.2 mask2: param0_opnd = TB2.fast_cs_mask = TB2->extra_tb->fast_cs_mask */
//        ir2_opnd_set_em(&ret_opnd, EM_MIPS_ADDRESS, 32);
//        append_ir2_opnd2i(mips_ld, &tmp_opnd, &ret_opnd, offsetof(TranslationBlock, extra_tb));
//        ir2_opnd_set_em(&tmp_opnd, EM_MIPS_ADDRESS, 32);
//        append_ir2_opnd2i(mips_lbu, &param0_opnd, &tmp_opnd, offsetof(ETB, fast_cs_mask));
//        append_ir2_opnd2i(mips_andi, &param0_opnd, &param0_opnd, XTM_FAST_CS_MASK);
//        /* 1.3 new fast-cs-mask = mask1 | mask2 */
//        append_ir2_opnd3(mips_or, &cmp_opnd, &param1_opnd, &param0_opnd);
//        append_ir2_opnd2i(mips_sb, &cmp_opnd, &env_ir2_opnd,
//                lsenv_offset_of_fast_cs_mask(lsenv));
//
//        /* 2. load XMM if mask1[1] == 0 && mask2[1] == 1 */
//        IR2_OPND no_load_xmm = ir2_opnd_new_type(IR2_OPND_LABEL);
//        append_ir2_opnd2i(mips_andi,  &cmp_opnd,  &param1_opnd,    XTM_FAST_CS_MASK_XMM);
//        append_ir2_opnd3 (mips_bne,   &cmp_opnd,  &zero_ir2_opnd,  &no_load_xmm);
//        append_ir2_opnd2i(mips_andi,  &cmp_opnd,  &param0_opnd,    XTM_FAST_CS_MASK_XMM);
//        append_ir2_opnd3 (mips_beq,   &cmp_opnd,  &zero_ir2_opnd,  &no_load_xmm);
//        /* 2.1 not branch, load XMM */
//        tr_load_xmms_from_env(0xff, 0xff);
//        /* 2.2 load XMM done or no need to load */
//        append_ir2_opnd1(mips_label, &no_load_xmm);
//
//        /* 3. load FPU if mask1[0] == 0 && mask2[0] == 1 */
//        IR2_OPND no_load_fpu = ir2_opnd_new_type(IR2_OPND_LABEL);
//        append_ir2_opnd2i(mips_andi,  &cmp_opnd,  &param1_opnd,    XTM_FAST_CS_MASK_FPU);
//        append_ir2_opnd3 (mips_bne,   &cmp_opnd,  &zero_ir2_opnd,  &no_load_fpu);
//        append_ir2_opnd2i(mips_andi,  &cmp_opnd,  &param0_opnd,    XTM_FAST_CS_MASK_FPU);
//        append_ir2_opnd3 (mips_beq,   &cmp_opnd,  &zero_ir2_opnd,  &no_load_fpu);
//        /* 3.1 not branch, load FPU */
//        tr_load_fprs_from_env(0xff, 0); /* lsfpu not supported! It will use temp registers. Be careful! */
//        /* 3.2 check FPU rotate and jmp to TB2 */
//        IR2_OPND fast_cs_fpu_rotate = ir2_opnd_new_type(IR2_OPND_LABEL);
//        append_ir2_opnd1(mips_b, &fast_cs_fpu_rotate);
//        /* 3.3 not load FPU or FPU already loaded before */
//        append_ir2_opnd1(mips_label, &no_load_fpu);
//
//        /* 4. rotate FPU if mask1[0] == 1 && mask2[0] == 1 :     11
//         *    after <3>   : mask1[0] != 0 || mask2[0] != 1 : 10, 11, 00 >> only need to check mask2[0] */
//        IR2_OPND no_rotate_fpu = ir2_opnd_new_type(IR2_OPND_LABEL);
//        append_ir2_opnd2i(mips_andi,  &cmp_opnd,  &param0_opnd,    XTM_FAST_CS_MASK_FPU);
//        append_ir2_opnd3 (mips_beq,   &cmp_opnd,  &zero_ir2_opnd,  &no_rotate_fpu);
//        /*
//         * 4.1 not branch, check FPU rotate
//         *     current top(usually TB1.top_out) and TB2.top_in
//         */
//        append_ir2_opnd1(mips_label, &fast_cs_fpu_rotate);
//        /*
//         * 4.1.1 load current top: read from CPUX86State.fpstt
//         *
//         * Why not TB1.top_out? Besides the situation that FPU is just loaded, there is another
//         * specical situation:     TB0(use fpu) -> jump to TB1 directly
//         *                      -> TB1(no  FPU) -> check FPU rotate with TB1.top_out (NOT OK!)
//         *                      -> TB2(use FPU)
//         *      TB1 does not use FPU but CPUX86State.vreg[5] indicates FPU is already loaded.
//         *      There might be one TB0 that use FPU and jump to TB1 via TB-link, then the real
//         *      top_out should be TB0.top_out not TB1.top_out
//         *
//         * current implement:      TB0(use fpu) -> sync top_out into fpstt in EOB worker
//         *                                      -> jump to TB1 directly
//         *                      -> TB1(no  fpu) -> check FPU rotate with fpstt      (OK)
//         *                      -> TB2(use fpu)
//         */
//        append_ir2_opnd2i(mips_lw, &param1_opnd, &env_ir2_opnd, lsenv_offset_of_top(lsenv));
//        /* 4.1.2 load TB2.top_in */
//        ir2_opnd_set_em(&ret_opnd, EM_MIPS_ADDRESS, 32);
//        append_ir2_opnd2i(mips_ld, &tmp_opnd, &ret_opnd, offsetof(TranslationBlock, extra_tb));
//        ir2_opnd_set_em(&tmp_opnd, EM_MIPS_ADDRESS, 32);
//        append_ir2_opnd2i(mips_lbu, &param0_opnd, &tmp_opnd, offsetof(ETB, _top_in));
//        /* 4.1.3: calculate step = fpstt - TB2.top_in */
//        append_ir2_opnd3(mips_subu, &step_opnd, &param1_opnd, &param0_opnd);
//        append_ir2_opnd3(mips_beq, &step_opnd, &zero_ir2_opnd, &no_rotate_fpu);
//        /* 4.1.4: load TB2.tc.ptr */
//        ir2_opnd_set_em(&ret_opnd, EM_MIPS_ADDRESS, 32);
//        append_ir2_opnd2i(mips_ld, &tmp_opnd, &ret_opnd,
//                offsetof(TranslationBlock, tc) + offsetof(struct tb_tc, ptr));
//        /* 4.1.5: native_fpu_rotate($t8:step, $t9:TB.tc.ptr) */
//        append_ir2_opnda(mips_j, native_rotate_fpu_by); /* -----------> FPU rotate ---> TB2.tc.ptr */
//
//        /* 4.2 no need rotate FPU */
//        append_ir2_opnd1(mips_label, &no_rotate_fpu);
//
//        /* 5. jump to TB2 */
//        ir2_opnd_set_em(&ret_opnd, EM_MIPS_ADDRESS, 32);
//        append_ir2_opnd2i(mips_ld, &tmp_opnd, &ret_opnd,
//                offsetof(TranslationBlock, tc) + offsetof(struct tb_tc, ptr));
//        append_ir2_opnd1(mips_jr, &tmp_opnd);
//#else
        /* jmp glue for normal TB-link without fast-cs
         *
         * rotate FPU according to TB1.top_out and TB2.top_in */

        /* 1. tb->extra_tb._top_out */
//        ir2_opnd_set_em(&tb, EM_MIPS_ADDRESS, 32);
        append_ir2_opnd2i(LISA_LD_D, &tmp_opnd, &tb,
                offsetof(TranslationBlock, extra_tb));
//        ir2_opnd_set_em(&tmp_opnd, EM_MIPS_ADDRESS, 32);
        append_ir2_opnd2i(LISA_LD_BU, &param1_opnd, &tmp_opnd,
                offsetof(ETB, _top_out));
        /* 2. next_tb->extra_tb._top_in */
//        ir2_opnd_set_em(&ret_opnd, EM_MIPS_ADDRESS, 32);
        append_ir2_opnd2i(LISA_LD_D, &tmp_opnd, &ret_opnd,
                offsetof(TranslationBlock, extra_tb));
//        ir2_opnd_set_em(&tmp_opnd, EM_MIPS_ADDRESS, 32);
        append_ir2_opnd2i(LISA_LD_BU, &param0_opnd, &tmp_opnd,
                offsetof(ETB, _top_in));
        /* 3. calculate top_bias, store rotate step in arg1 */
        append_ir2_opnd3(LISA_SUB_D, &step_opnd, &param1_opnd, &param0_opnd);
        /* for direct jmps, if no need to rotate, we will make direct link without
         * this glue */
        if (n == 2) {
            IR2_OPND label_rotate = ir2_opnd_new_label();
            append_ir2_opnd3(LISA_BNE, &step_opnd, &zero_ir2_opnd, &label_rotate);
            /* bias ==0, no need to ratate */
            /* fetch native address of next_tb to arg2 */
            append_ir2_opnd2i(LISA_LD_D, &tmp_opnd, &ret_opnd,
                    offsetof(TranslationBlock, tc) + offsetof(struct tb_tc, ptr));
            append_ir2_opnd2i(LISA_JIRL, &zero_ir2_opnd, &tmp_opnd, 0);
            append_ir2_opnd1(LISA_LABEL, &label_rotate);
        }
        /* top_bias != 0, need to rotate, step is in arg1 */
        /* fetch native address of next_tb to arg2 */
//        ir2_opnd_set_em(&ret_opnd, EM_MIPS_ADDRESS, 32);
        append_ir2_opnd2i(LISA_LD_D, &tb_nc_opnd, &ret_opnd,
                offsetof(TranslationBlock, tc) + offsetof(struct tb_tc, ptr));
        offset = (lsenv->tr_data->real_ir2_inst_num << 2) - start;
        append_ir2_opnda(LISA_B, (native_rotate_fpu_by
                                  - (ADDR)code_buf - offset) >> 2);
//#endif
    } else {
        /* With LSFPU enabled! */
//#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_FAST_CS)
//        /* When LSFPU is enabled with FastCS, we need to check
//         * wether should we load FPU or XMM context for next TB
//         *
//         * NO more FPU rotate ! */
//
//        /* 1.1 mask1: param1_opnd = TB1.fast_cs_mask = CPUX86State.vreg[5] */
//        append_ir2_opnd2i(mips_lbu, &param1_opnd, &env_ir2_opnd,
//                lsenv_offset_of_fast_cs_mask(lsenv));
//        append_ir2_opnd2i(mips_andi, &param1_opnd, &param1_opnd, XTM_FAST_CS_MASK);
//        /* 1.2 mask2: param0_opnd = TB2.fast_cs_mask = TB2->extra_tb->fast_cs_mask */
//        ir2_opnd_set_em(&ret_opnd, EM_MIPS_ADDRESS, 32);
//        append_ir2_opnd2i(mips_ld, &tmp_opnd, &ret_opnd, offsetof(TranslationBlock, extra_tb));
//        ir2_opnd_set_em(&tmp_opnd, EM_MIPS_ADDRESS, 32);
//        append_ir2_opnd2i(mips_lbu, &param0_opnd, &tmp_opnd, offsetof(ETB, fast_cs_mask));
//        append_ir2_opnd2i(mips_andi, &param0_opnd, &param0_opnd, XTM_FAST_CS_MASK);
//        /* 1.3 new fast-cs-mask = mask1 | mask2 */
//        append_ir2_opnd3(mips_or, &cmp_opnd, &param1_opnd, &param0_opnd);
//        append_ir2_opnd2i(mips_sb, &cmp_opnd, &env_ir2_opnd,
//                lsenv_offset_of_fast_cs_mask(lsenv));
//
//        /* 2. load XMM if mask1[1] == 0 && mask2[1] == 1 */
//        IR2_OPND no_load_xmm = ir2_opnd_new_type(IR2_OPND_LABEL);
//        append_ir2_opnd2i(mips_andi,  &cmp_opnd,  &param1_opnd,    XTM_FAST_CS_MASK_XMM);
//        append_ir2_opnd3 (mips_bne,   &cmp_opnd,  &zero_ir2_opnd,  &no_load_xmm);
//        append_ir2_opnd2i(mips_andi,  &cmp_opnd,  &param0_opnd,    XTM_FAST_CS_MASK_XMM);
//        append_ir2_opnd3 (mips_beq,   &cmp_opnd,  &zero_ir2_opnd,  &no_load_xmm);
//        /* 2.1 not branch, load XMM */
//        tr_load_xmms_from_env(0xff, 0xff);
//        /* 2.2 load XMM done or no need to load */
//        append_ir2_opnd1(mips_label, &no_load_xmm);
//
//        /* 3. load FPU if mask1[0] == 0 && mask2[0] == 1 */
//        IR2_OPND no_load_fpu = ir2_opnd_new_type(IR2_OPND_LABEL);
//        append_ir2_opnd2i(mips_andi,  &cmp_opnd,  &param1_opnd,    XTM_FAST_CS_MASK_FPU);
//        append_ir2_opnd3 (mips_bne,   &cmp_opnd,  &zero_ir2_opnd,  &no_load_fpu);
//        append_ir2_opnd2i(mips_andi,  &cmp_opnd,  &param0_opnd,    XTM_FAST_CS_MASK_FPU);
//        append_ir2_opnd3 (mips_beq,   &cmp_opnd,  &zero_ir2_opnd,  &no_load_fpu);
//        /* 3.1 not branch, load FPU */
//        tr_load_fprs_from_env(0xff, 0);
//        tr_load_lstop_from_env(&cmp_opnd);
//        tr_fpu_enable_top_mode();
//        /* 3.2 load FPU doen or no need to load */
//        append_ir2_opnd1(mips_label, &no_load_fpu);
//
//        /* With LSFPU enabled, no more FPU rotate here  */
//
//        /* 4. direct jump to TB2 */
//        ir2_opnd_set_em(&ret_opnd, EM_MIPS_ADDRESS, 32);
//        append_ir2_opnd2i(mips_ld, &tmp_opnd, &ret_opnd,
//                offsetof(TranslationBlock, tc) + offsetof(struct tb_tc, ptr));
//        append_ir2_opnd1(mips_jr, &tmp_opnd);
//#else
        /* When LSFPU is enabled without FastCS:
         * we can direct jump to next TB.
         *
         * In fact, the jmp glue 0 and jmp glue 1 will never be
         * used in TB-Link when LSFPU is enabled without FastCS.
         *
         * The jmp glue 2 for indirect jmp will be used.
         */
        append_ir2_opnd2i(LISA_LD_D, &tmp_opnd, &ret_opnd,
                offsetof(TranslationBlock, tc) + offsetof(struct tb_tc, ptr));
        append_ir2_opnd2i(LISA_JIRL, &zero_ir2_opnd, &tmp_opnd, 0);
//#endif
    }

    lisa_num = tr_ir2_assemble(code_buf) + 1;

    tr_fini(false);

    return lisa_num;
}

/* generate native_jmp_glue_0/1/2
 * args pass in: $24: tb, $25: eip(which is also stored in env->eip)  */
int generate_native_jmp_glue_all(void *code_buffer)
{
    void *code_buf = code_buffer;
    int mips_num = 0;
    int total_mips_num = 0;

    native_jmp_glue_0 = (ADDR)code_buf;
    mips_num = generate_native_jmp_glue(code_buf, 0);
    total_mips_num += mips_num;
    code_buf += mips_num << 2;

    native_jmp_glue_1 = (ADDR)code_buf;
    mips_num = generate_native_jmp_glue(code_buf, 1);
    total_mips_num += mips_num;
    code_buf += mips_num << 2;

    native_jmp_glue_2 = (ADDR)code_buf;
    mips_num = generate_native_jmp_glue(code_buf, 2);
    total_mips_num += mips_num;
    code_buf += mips_num << 2;

    return total_mips_num << 2;
}

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_FAST_CS)

static void generate_fastcs_jmp_glue_fpu(void *code_buf, int n)
{
    lsassertm(0, "fastcs jmp glue fpu to be implemented in LoongArch.\n");
//    IR2_OPND tb = ra_alloc_dbt_arg1();
//    IR2_OPND step_opnd = ra_alloc_dbt_arg1();
//    IR2_OPND tmp_opnd = ra_alloc_dbt_arg2();
//    IR2_OPND param0_opnd = ir2_opnd_new(IR2_OPND_IREG, 4);
//    IR2_OPND param1_opnd = ir2_opnd_new(IR2_OPND_IREG, 5);
//    IR2_OPND cmp_opnd = ir2_opnd_new(IR2_OPND_IREG, 6);
//    IR2_OPND ret_opnd = ir2_opnd_new(IR2_OPND_IREG, 2);
//
//    /* load tb->extra_tb.next_tb[n] into v0. */
//    ir2_opnd_set_em(&tb, EM_MIPS_ADDRESS, 32);
//    /* 1. load extra_tb's address */
//    append_ir2_opnd2i(mips_ld, &tmp_opnd, &tb,
//                      offsetof(TranslationBlock, extra_tb));
//    ir2_opnd_set_em(&tmp_opnd, EM_MIPS_ADDRESS, 32);
//    /* 2. load next_tb */
//    append_ir2_opnd2i(mips_ld, &ret_opnd, &tmp_opnd,
//                          offsetof(struct ExtraBlock, next_tb) +
//                          n * sizeof(void *));
//#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_FAST_CS)
//#ifdef CONFIG_XTM_PROFILE
//        tr_pf_inc_fastcs_glue_01();
//#endif
//#endif
//
//    /* jmp glue for fast-cs that considers only FPU
//     *
//     * TB1 -> TB2
//     * TB1: $t8, @tb
//     * TB2: $v0, @ret_opnd
//     *
//     * 1. read fast-cs-mask
//     *    mask1: TB1.fast-cs-mask: @param1_opnd : any value
//     *    mask2: TB2.fast-cs-mask: @param0_opnd : must be 01 or 11
//     * 2. load FPU if mask1[0] == 0 and FPU rotate check, goto 4.
//     * 3. FPU rotate check according to TB1.top_out and TB2.top_in
//     * 4. goto TB2 */
//
//    /* 1.1 mask1: param1_opnd = TB1.fast_cs_mask = CPUX86State.vreg[5] */
//    append_ir2_opnd2i(mips_lbu, &param1_opnd, &env_ir2_opnd,
//            lsenv_offset_of_fast_cs_mask(lsenv));
//    append_ir2_opnd2i(mips_andi, &param1_opnd, &param1_opnd, XTM_FAST_CS_MASK);
//
//    /* 2. load FPU if mask1[0] == 0 */
//    IR2_OPND no_load_fpu = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2i(mips_andi,  &cmp_opnd,  &param1_opnd,    XTM_FAST_CS_MASK_FPU);
//    append_ir2_opnd3 (mips_bne,   &cmp_opnd,  &zero_ir2_opnd,  &no_load_fpu);
//    /* 2.1 not branch, load FPU */
//    tr_load_fprs_from_env(0xff, 0);
//    if (option_lsfpu) {
//        tr_load_lstop_from_env(&cmp_opnd);
//        tr_fpu_enable_top_mode();
//    }
//    /* 2.2 new fast-cs-mask = mask1 | 0x1 */
//    append_ir2_opnd2i(mips_ori, &cmp_opnd, &param1_opnd, 0x1);
//    append_ir2_opnd2i(mips_sb, &cmp_opnd, &env_ir2_opnd,
//            lsenv_offset_of_fast_cs_mask(lsenv));
//    /* 2.3 just load FPU or FPU already loaded before */
//    append_ir2_opnd1(mips_label, &no_load_fpu);
//
//    /* With LSFPU enabled, no more FPU rotate here  */
//    if (option_lsfpu) goto NO_ROTATE_FPU;
//
//    /* 3. rotate FPU */
//    /* 3.1.1 load current top: param1_opnd = CPUX86State.fpstt */
//    append_ir2_opnd2i(mips_lw, &param1_opnd, &env_ir2_opnd,
//            lsenv_offset_of_top(lsenv));
//    /* 3.1.2 load TB2.top_in : param0_opnd = TB2.top_in */
//    ir2_opnd_set_em(&ret_opnd, EM_MIPS_ADDRESS, 32);
//    append_ir2_opnd2i(mips_ld, &tmp_opnd, &ret_opnd, offsetof(TranslationBlock, extra_tb));
//    ir2_opnd_set_em(&tmp_opnd, EM_MIPS_ADDRESS, 32);
//    append_ir2_opnd2i(mips_lbu, &param0_opnd, &tmp_opnd, offsetof(ETB, _top_in));
//    /* 3.1.3: calculate step = fpstt - TB2.top_in */
//    IR2_OPND no_rotate_fpu = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_subu, &step_opnd, &param1_opnd, &param0_opnd);
//    append_ir2_opnd3(mips_beq, &step_opnd, &zero_ir2_opnd, &no_rotate_fpu);
//    /* 3.1.4: load TB2.tc.ptr */
//    ir2_opnd_set_em(&ret_opnd, EM_MIPS_ADDRESS, 32);
//    append_ir2_opnd2i(mips_ld, &tmp_opnd, &ret_opnd,
//            offsetof(TranslationBlock, tc) + offsetof(struct tb_tc, ptr));
//    /* 3.1.5: native_fpu_rotate($t8:step, $t9:TB.tc.ptr) */
//    append_ir2_opnda(mips_j, native_rotate_fpu_by);
//
//    /* 3.2 no need rotate FPU */
//    append_ir2_opnd1(mips_label, &no_rotate_fpu);
//
//NO_ROTATE_FPU:
//    /* 4. jump to TB2 */
//    ir2_opnd_set_em(&ret_opnd, EM_MIPS_ADDRESS, 32);
//    append_ir2_opnd2i(mips_ld, &tmp_opnd, &ret_opnd,
//            offsetof(TranslationBlock, tc) + offsetof(struct tb_tc, ptr));
//    append_ir2_opnd1(mips_jr, &tmp_opnd);
}

static void generate_fastcs_jmp_glue_xmm(void *code_buf, int n)
{
    lsassertm(0, "fastcs jmp glue xmm to be implemented in LoongArch.\n");
//    IR2_OPND tb = ra_alloc_dbt_arg1();
//    IR2_OPND tmp_opnd = ra_alloc_dbt_arg2();
//    IR2_OPND param1_opnd = ir2_opnd_new(IR2_OPND_IREG, 5);
//    IR2_OPND cmp_opnd = ir2_opnd_new(IR2_OPND_IREG, 6);
//    IR2_OPND ret_opnd = ir2_opnd_new(IR2_OPND_IREG, 2);
//
//    /* load tb->extra_tb.next_tb[n] into v0. */
//    ir2_opnd_set_em(&tb, EM_MIPS_ADDRESS, 32);
//    /* 1. load extra_tb's address */
//    append_ir2_opnd2i(mips_ld, &tmp_opnd, &tb,
//                      offsetof(TranslationBlock, extra_tb));
//    ir2_opnd_set_em(&tmp_opnd, EM_MIPS_ADDRESS, 32);
//    /* 2. load next_tb */
//    append_ir2_opnd2i(mips_ld, &ret_opnd, &tmp_opnd,
//                          offsetof(struct ExtraBlock, next_tb) +
//                          n * sizeof(void *));
//#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_FAST_CS)
//#ifdef CONFIG_XTM_PROFILE
//        tr_pf_inc_fastcs_glue_10();
//#endif
//#endif
//
//    /* jmp glue for fast-cs that considers only XMM
//     *
//     * TB1 -> TB2
//     * TB1: $t8, @tb
//     * TB2: $v0, @ret_opnd
//     *
//     * 1. read fast-cs-mask
//     *    mask1: TB1.fast-cs-mask: @param1_opnd : any value
//     *    mask2: TB2.fast-cs-mask: must be 10
//     * 2. load XMM if mask1[1] == 0
//     * 3. goto TB2 */
//
//    /* 1.1 mask1: param1_opnd = TB1.fast_cs_mask = CPUX86State.vreg[5] */
//    append_ir2_opnd2i(mips_lbu, &param1_opnd, &env_ir2_opnd,
//            lsenv_offset_of_fast_cs_mask(lsenv));
//    append_ir2_opnd2i(mips_andi, &param1_opnd, &param1_opnd, XTM_FAST_CS_MASK);
//
//    /* 2. load XMM if mask1[1] == 0 */
//    IR2_OPND no_load_xmm = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2i(mips_andi,  &cmp_opnd,  &param1_opnd,    XTM_FAST_CS_MASK_XMM);
//    append_ir2_opnd3 (mips_bne,   &cmp_opnd,  &zero_ir2_opnd,  &no_load_xmm);
//    /* 2.1 not branch, load XMM */
//    tr_load_xmms_from_env(0xff, 0xff);
//    /* 2.2 new fast-cs-mask = mask1 | mask2 */
//    append_ir2_opnd2i(mips_ori, &cmp_opnd, &param1_opnd, 0x2);
//    append_ir2_opnd2i(mips_sb, &cmp_opnd, &env_ir2_opnd,
//            lsenv_offset_of_fast_cs_mask(lsenv));
//    /* 2.3 load XMM done or no need to load */
//    append_ir2_opnd1(mips_label, &no_load_xmm);
//
//    /* 3. jump to TB2 */
//    ir2_opnd_set_em(&ret_opnd, EM_MIPS_ADDRESS, 32);
//    append_ir2_opnd2i(mips_ld, &tmp_opnd, &ret_opnd,
//            offsetof(TranslationBlock, tc) + offsetof(struct tb_tc, ptr));
//    append_ir2_opnd1(mips_jr, &tmp_opnd);
}

int generate_fastcs_jmp_glue_all(void *code_buffer)
{
    int code_nr = 0;
    void *code_buf = code_buffer;

    jmp_glue_fastcs_fpu_0 = (ADDR)code_buf;
    tr_init(NULL);
    generate_fastcs_jmp_glue_fpu(code_buf, 0);
    code_nr += tr_ir2_assemble(code_buf);
    code_buf = code_buffer + (code_nr << 2);
    tr_fini(false);

    jmp_glue_fastcs_fpu_1 = (ADDR)code_buf;
    tr_init(NULL);
    generate_fastcs_jmp_glue_fpu(code_buf, 1);
    code_nr += tr_ir2_assemble(code_buf);
    code_buf = code_buffer + (code_nr << 2);
    tr_fini(false);

    if (option_dump) {
        fprintf(stderr, "[X86toMIPS] fastcs jmp glue fpu 0 = %p\n",
                (void *)jmp_glue_fastcs_fpu_0);
        fprintf(stderr, "[X86toMIPS] fastcs jmp glue fpu 1 = %p\n",
                (void *)jmp_glue_fastcs_fpu_1);
    }

    jmp_glue_fastcs_xmm_0 = (ADDR)code_buf;
    tr_init(NULL);
    generate_fastcs_jmp_glue_xmm(code_buf, 0);
    code_nr += tr_ir2_assemble(code_buf);
    code_buf = code_buffer + (code_nr << 2);
    tr_fini(false);

    jmp_glue_fastcs_xmm_1 = (ADDR)code_buf;
    tr_init(NULL);
    generate_fastcs_jmp_glue_xmm(code_buf, 1);
    code_nr += tr_ir2_assemble(code_buf);
    code_buf = code_buffer + (code_nr << 2);
    tr_fini(false);

    if (option_dump) {
        fprintf(stderr, "[X86toMIPS] fastcs jmp glue xmm 0 = %p\n",
                (void *)jmp_glue_fastcs_xmm_0);
        fprintf(stderr, "[X86toMIPS] fastcs jmp glue xmm 1 = %p\n",
                (void *)jmp_glue_fastcs_xmm_1);
    }

    return code_nr << 2;
}
#endif

/* note: native_rotate_fpu_by rotate data between mapped fp registers instead
 * of the in memory env->fpregs
 */
int generate_native_rotate_fpu_by(void *code_buf_addr)
{
//    lsassertm(0, "gen native fpu rotate to be implemented in LoongArch.\n");
    void *code_buf = code_buf_addr;

    int lisa_num = 0;
    int total_lisa_num = 0;

    static ADDR rotate_by_step_addr[15]; /* rotate -7 ~ 7 */
    /* 1. generate the rotation code for step 0 */
    ADDR *rotate_by_step_0_addr = rotate_by_step_addr + 7;
    rotate_by_step_0_addr[0] = 0;

    /* 2. generate the rotation code for step 1-7 */
    for (int step = 1; step <= 7; ++step) {
        tr_init(NULL);
        /* 2.1 load top_bias early. It will be modified later */
        IR2_OPND top_bias = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_LD_WU, &top_bias, &env_ir2_opnd,
                          lsenv_offset_of_top_bias(lsenv)); /* */
        /* 2.2 prepare for the rotation */
        IR2_OPND fpr[8];
        for (int i = 0; i < 8; ++i)
            fpr[i] = ra_alloc_st(i);
        /* 2.3 rotate! */
        IR2_OPND spilled_data = ra_alloc_ftemp();
        int spilled_index = 0;
        int number_of_moved_fpr = 0;
        while (number_of_moved_fpr < 8) {
            /* 2.3.1 spill out a register */
            append_ir2_opnd2(LISA_FMOV_D, &spilled_data, &fpr[spilled_index]);
            /* 2.3.2 rotate, until moving from the spilled register */
            int target_index = spilled_index;
            int source_index = (target_index + step) & 7;
            while (source_index != spilled_index) {
                append_ir2_opnd2(LISA_FMOV_D, &fpr[target_index], &fpr[source_index]);
                number_of_moved_fpr++;
                target_index = source_index;
                source_index = (target_index + step) & 7;
            };
            /* 2.3.3 move from the spilled data */
            append_ir2_opnd2(LISA_FMOV_D, &fpr[target_index], &spilled_data);
            number_of_moved_fpr++;
            /* 2.3.4 when step is 2, 4, or 6, rotate from the next index; */
            spilled_index++;
        }

        /* 1.4 adjust the top_bias */
        append_ir2_opnd2i(LISA_ADDI_W, &top_bias, &top_bias, step);
        append_ir2_opnd2i(LISA_ANDI,   &top_bias, &top_bias, 0x7);

#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
        tr_pf_inc_fpu_rotate_native();
#endif

        /* 1.5 jump to next TB's native code */
        IR2_OPND target_native_code_addr = ra_alloc_dbt_arg2();
        append_ir2_opnd2i(LISA_ST_W, &top_bias, &env_ir2_opnd,
                          lsenv_offset_of_top_bias(lsenv));
        append_ir2_opnd2i(LISA_JIRL, &zero_ir2_opnd, &target_native_code_addr, 0);

        lisa_num = 0;
        rotate_by_step_0_addr[step - 8] = rotate_by_step_0_addr[step] = (ADDR)code_buf;
        lisa_num = tr_ir2_assemble((void *)rotate_by_step_0_addr[step - 8]) + 1;
        tr_fini(false);

        if (option_dump)
            fprintf(stderr,
                    "[fpu rotate] rotate step(%d,%d) at %p, size = %d\n",
                    step, step - 8, code_buf, lisa_num);
        code_buf += lisa_num * 4;
        total_lisa_num += lisa_num;
    }

    /* 3. generate dispatch code. two arguments:
     *    > DBT arg 1 : rotation step
     *    > DBT arg 2 : target native address
     */
    tr_init(NULL);
    IR2_OPND rotate_fpu = ra_alloc_itemp();
    load_addr_to_ir2(&rotate_fpu, (ADDR)(rotate_by_step_0_addr));
    IR2_OPND rotate_step = ra_alloc_dbt_arg1();
//#ifdef N64
//    append_ir2_opnd2i(mips_sll, &rotation_step, &rotation_step, 3);
//#else
//    append_ir2_opnd2i(mips_sll, &rotation_step, &rotation_step, 2);
//#endif
    append_ir2_opnd2i(LISA_SLLI_W, &rotate_step, &rotate_step, 3);
    append_ir2_opnd3(LISA_ADD_D, &rotate_fpu, &rotate_fpu, &rotate_step);
    append_ir2_opnd2i(LISA_LD_D, &rotate_fpu, &rotate_fpu, 0);
    append_ir2_opnd2i(LISA_JIRL, &zero_ir2_opnd, &rotate_fpu, 0);

    lisa_num = 0;
    native_rotate_fpu_by = (ADDR)code_buf;
    lisa_num = tr_ir2_assemble((void *)native_rotate_fpu_by) + 1;
    tr_fini(false);
    if (option_dump)
        fprintf(stderr, "[fpu rotate] rotate dispatch at %p. size = %d\n",
                code_buf, lisa_num);
    total_lisa_num += lisa_num;
    code_buf += lisa_num * 4;

//    IR2_OPND a0_opnd = ir2_opnd_new(IR2_OPND_IREG, 4);
//    IR2_OPND v0_opnd = ir2_opnd_new(IR2_OPND_IREG, 2);
//    IR2_OPND ra_opnd = ir2_opnd_new(IR2_OPND_IREG, 31);
//    /* fpu_get_top() */
//    fpu_get_top = (ADDR)code_buf;
//    tr_init(NULL);
//    append_ir2_opnd1(mips_mftop, &v0_opnd);
//    append_ir2_opnd1(mips_jr, &ra_opnd);
//    mips_num = tr_ir2_assemble(code_buf) + 1;
//    total_mips_num += mips_num;
//    code_buf += mips_num * 4;
//    tr_fini(false);

//    /* fpu_inc_top() */
//    fpu_inc_top = (ADDR)code_buf;
//    tr_init(NULL);
//    append_ir2_opnd0(mips_inctop);
//    append_ir2_opnd1_not_nop(mips_jr, &ra_opnd);
//    append_ir2_opnd1(mips_mftop, &v0_opnd);
//    mips_num = tr_ir2_assemble(code_buf) + 1;
//    total_mips_num += mips_num;
//    code_buf += mips_num * 4;
//    tr_fini(false);

//    /* fpu_dec_top() */
//    fpu_dec_top = (ADDR)code_buf;
//    tr_init(NULL);
//    append_ir2_opnd0(mips_dectop);
//    append_ir2_opnd1_not_nop(mips_jr, &ra_opnd);
//    append_ir2_opnd1(mips_mftop, &v0_opnd);
//    mips_num = tr_ir2_assemble(code_buf) + 1;
//    total_mips_num += mips_num;
//    code_buf += mips_num * 4;
//    tr_fini(false);

//    /* fpu_enable_top() */
//    fpu_enable_top = (ADDR)code_buf;
//    tr_init(NULL);
//    append_ir2_opnd0(mips_setop);
//    append_ir2_opnd1_not_nop(mips_jr, &ra_opnd);
//    append_ir2_opnd2(mips_cfc1, &v0_opnd, &fcsr_ir2_opnd);
//    mips_num = tr_ir2_assemble(code_buf) + 1;
//    total_mips_num += mips_num;
//    code_buf += mips_num * 4;
//    tr_fini(false);

//    /* fpu_disable_top() */
//    fpu_disable_top = (ADDR)code_buf;
//    tr_init(NULL);
//    append_ir2_opnd0(mips_clrtop);
//    append_ir2_opnd1_not_nop(mips_jr, &ra_opnd);
//    append_ir2_opnd1(mips_mftop, &v0_opnd);
//    mips_num = tr_ir2_assemble(code_buf) + 1;
//    total_mips_num += mips_num;
//    code_buf += mips_num * 4;
//    tr_fini(false);

//    /* fpu_set_top(int top) */
//    fpu_set_top = (ADDR)code_buf;
//    tr_init(NULL);
//    IR2_OPND label_loop = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opndi(mips_mttop, 0);
//    append_ir2_opnd1(mips_label, &label_loop);
//    append_ir2_opnd0(mips_inctop);
//    append_ir2_opnd3_not_nop(mips_bne, &a0_opnd, &zero_ir2_opnd, &label_loop);
//    append_ir2_opnd2i(mips_addiu, &a0_opnd, &a0_opnd, -1);
//    /* one step back */
//    append_ir2_opnd0(mips_dectop);
//    append_ir2_opnd1_not_nop(mips_jr, &ra_opnd);
//    append_ir2_opnd1(mips_mftop, &v0_opnd);
//    mips_num = tr_ir2_assemble(code_buf) + 1;
//    total_mips_num += mips_num;
//    code_buf += mips_num * 4;
//    tr_fini(false);
//    if (option_dump)
//        fprintf(stderr, "[fpu rotate] native jump glue at %p. size = %d\n",
//                code_buf, mips_num);

    return total_lisa_num << 2;
}

void ss_generate_match_fail_native_code(void* code_buf)
{
    lsassertm(0, "shadow stack to be implemented in LoongArch.\n");
//    // ss_x86_addr is not equal to x86_addr, compare esp
//    IR2_OPND ss_opnd = ra_alloc_ss();
//    IR2_OPND ss_esp = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_load_addrx, &ss_esp, &ss_opnd, -(int)sizeof(SS_ITEM) + (int)offsetof(SS_ITEM, x86_esp));
//    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
//    IR2_OPND temp_result = ra_alloc_itemp();
//
//    // if esp < ss_esp, that indicates ss has less item
//    IR2_OPND label_exit_with_fail_match = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3_not_nop(mips_bne, &temp_result, &zero_ir2_opnd, &label_exit_with_fail_match);
//    append_ir2_opnd3(mips_sltu, &temp_result, &esp_opnd, &ss_esp);
//    // x86_addr is not equal, but esp match, it indicates that the x86_addr has been changed
//    append_ir2_opnd3_not_nop(mips_beq, &esp_opnd, &ss_esp, &label_exit_with_fail_match);
//    append_ir2_opnd2i(mips_addi_addr, &ss_opnd, &ss_opnd, -(int)sizeof(SS_ITEM));
//    // pop till find, compare esp with ss_esp each time
//    IR2_OPND label_pop_till_find = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_label, &label_pop_till_find);
//    append_ir2_opnd2i(mips_load_addrx, &ss_esp, &ss_opnd, -(int)sizeof(SS_ITEM) + (int)offsetof(SS_ITEM, x86_esp));
//    IR2_OPND label_esp_equal = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &esp_opnd, &ss_esp, &label_esp_equal);
//    append_ir2_opnd3_not_nop(mips_bne, &temp_result, &zero_ir2_opnd, &label_exit_with_fail_match);
//    append_ir2_opnd3(mips_slt, &temp_result, &esp_opnd, &ss_esp);
//    append_ir2_opnd1_not_nop(mips_b, &label_pop_till_find);
//    append_ir2_opnd2i(mips_addi_addr, &ss_opnd, &ss_opnd, -(int)sizeof(SS_ITEM));
//    ra_free_temp(&temp_result);
//    // esp equal, adjust esp with 24#reg value
//    append_ir2_opnd1(mips_label, &label_esp_equal);
//    append_ir2_opnd2i(mips_addi_addr, &ss_opnd, &ss_opnd, -(int)sizeof(SS_ITEM));
//    IR2_OPND etb_addr = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_load_addr, &etb_addr, &ss_opnd, (int)offsetof(SS_ITEM, return_tb));
//    IR2_OPND ret_tb_addr = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_load_addr, &ret_tb_addr, &etb_addr, offsetof(ETB, tb));
//    /* check if etb->tb is set */
//    IR2_OPND label_have_no_native_code = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &ret_tb_addr, &zero_ir2_opnd, &label_have_no_native_code);
//    IR2_OPND ss_x86_addr = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_load_addrx, &ss_x86_addr, &ret_tb_addr, (int)offsetof(TranslationBlock, pc));
//    IR2_OPND x86_addr = ra_alloc_dbt_arg2();
//    append_ir2_opnd3(mips_bne, &ss_x86_addr, &x86_addr, &label_exit_with_fail_match);
//    // after several ss_pop, finally match successfully
//    IR2_OPND esp_change_bytes = ra_alloc_mda();
//    append_ir2_opnd3(mips_add_addrx, &esp_opnd, &esp_opnd, &esp_change_bytes);
//    IR2_OPND ret_mips_addr = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_load_addr, &ret_mips_addr, &ret_tb_addr,
//        offsetof(TranslationBlock, tc) + offsetof(struct tb_tc, ptr));
//    //before jump to the target tb, check whether top_out and top_in are equal
//    //NOTE: last_executed_tb is already set before jumping to ss_match_fail_native
//    IR2_OPND rotate_step = ra_alloc_dbt_arg1();
//    IR2_OPND rotate_ret_addr = ra_alloc_dbt_arg2();
//    IR2_OPND label_no_rotate = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND last_executed_tb = ra_alloc_dbt_arg1();
//    IR2_OPND top_out = ra_alloc_itemp();
//    IR2_OPND top_in = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_lbu, &top_out, &last_executed_tb,
//        offsetof(TranslationBlock, extra_tb) + offsetof(ETB,_top_out));
//    append_ir2_opnd2i(mips_lbu, &top_in, &ret_tb_addr,
//        offsetof(TranslationBlock, extra_tb) + offsetof(ETB,_top_in));
//    append_ir2_opnd3(mips_beq, &top_in, &top_out, &label_no_rotate);
//    //top_in != top_out, rotate fpu
//    append_ir2_opnd3(mips_subu, &rotate_step, &top_out, &top_in);
//    append_ir2_opnda_not_nop(mips_j, native_rotate_fpu_by);
//    append_ir2_opnd2(mips_mov64, &rotate_ret_addr, &ret_mips_addr);
//    ra_free_temp(&top_in);
//    ra_free_temp(&top_out);
//    //top_in == top_out, directly go to next tb
//    append_ir2_opnd1(mips_label, &label_no_rotate);
//    append_ir2_opnd1(mips_jr, &ret_mips_addr);
//    ra_free_temp(&ret_tb_addr);
//    ra_free_temp(&ret_mips_addr);
//
//    // finally match failed: adjust esp, load last_execut_tb
//    append_ir2_opnd1(mips_label, &label_exit_with_fail_match);
//    append_ir2_opnd3(mips_add_addrx, &esp_opnd, &esp_opnd, &esp_change_bytes);
//    append_ir2_opnd1(mips_label, &label_have_no_native_code);
//    append_ir2_opnda(mips_j, context_switch_native_to_bt_ret_0);
//    //IR2_OPND indirect_lookup_code_addr = ra_alloc_itemp();
//    //load_ireg_from_addr(&indirect_lookup_code_addr, tb_look_up_native);
//    //append_ir2(mips_jr, indirect_lookup_code_addr);
//    //ra_free_temp(&indirect_lookup_code_addr);
//
//    ra_free_temp(&ss_esp);
//    ra_free_temp(&ss_x86_addr);
}

#ifdef CONFIG_XTM_USE_LOONGEXT

static int generate_check_loongext(void *code_buf)
{
    lsassertm(0, "gen check loongext to be implemented in LoongArch.\n");
//    append_ir2_opnd2(mips_cpucfg, &ret_ir2_opnd, &arg0_ir2_opnd);
//
//    IR2_OPND ra_opnd = ir2_opnd_new(IR2_OPND_IREG, 0x1f);
//    append_ir2_opnd1(mips_jr, &ra_opnd);
//
//    int code_size = tr_ir2_assemble(code_buf);
//    code_size = code_size << 2;
//
//    /* int cpucfg(int reg) */
//    int (*cpucfg_fn)(int reg) = code_buf;
//
//    int cfg1 = cpucfg_fn(0x1);
//    int cfg2 = cpucfg_fn(0x2);
//
//    printf("[CPUCFG] cpucfg 0x1 = %#x\n", cfg1);
//    printf("[CPUCFG] cpucfg 0x2 = %#x\n", cfg2);
//
//#define CPUCFG_MMI_SHIFT    4
//#define CPUCFG_LSX1_SHIFT   9
//#define CPUCFG_LSX2_SHIFT   10
//#define CPUCFG_LASX_SHIFT   11
//
//#define CPUCFG_LEXT1_SHIFT  0
//#define CPUCFG_LEXT2_SHIFT  1
//#define CPUCFG_LEXT3_SHIFT  2
//#define CPUCFG_LBT1_SHIFT   4
//#define CPUCFG_LBT2_SHIFT   5
//#define CPUCFG_LBT3_SHIFT   6
//
//#define CPUCFG_MMI_MASK     (1 << CPUCFG_MMI_SHIFT)
//#define CPUCFG_LSX1_MASK    (1 << CPUCFG_LSX1_SHIFT)
//#define CPUCFG_LSX2_MASK    (1 << CPUCFG_LSX2_SHIFT)
//#define CPUCFG_LASX_MASK    (1 << CPUCFG_LASX_SHIFT)
//
//#define CPUCFG_LEXT1_MASK   (1 << CPUCFG_LEXT1_SHIFT)
//#define CPUCFG_LEXT2_MASK   (1 << CPUCFG_LEXT2_SHIFT)
//#define CPUCFG_LEXT3_MASK   (1 << CPUCFG_LEXT3_SHIFT)
//#define CPUCFG_LBT1_MASK    (1 << CPUCFG_LBT1_SHIFT)
//#define CPUCFG_LBT2_MASK    (1 << CPUCFG_LBT2_SHIFT)
//#define CPUCFG_LBT3_MASK    (1 << CPUCFG_LBT3_SHIFT)

/*#define CPUCFG_CHECK_FEATURE(mask, cpucfg, name) do { \*/
    /*if ((cpucfg) & mask) { \*/
        /*printf("[CPUCFG] " name " = yes\n"); \*/
    /*} else { \*/
        /*printf("[CPUCFG] " name " = no\n"); \*/
    /*} \*/
/*} while(0)*/

//    CPUCFG_CHECK_FEATURE(CPUCFG_MMI_MASK,  cfg1, "mmi");
//    CPUCFG_CHECK_FEATURE(CPUCFG_LSX1_MASK, cfg1, "lsx1");
//    CPUCFG_CHECK_FEATURE(CPUCFG_LSX2_MASK, cfg1, "lsx2");
//    CPUCFG_CHECK_FEATURE(CPUCFG_LASX_MASK, cfg1, "lasx");
//
//    CPUCFG_CHECK_FEATURE(CPUCFG_LEXT1_MASK, cfg2, "lext1");
//    CPUCFG_CHECK_FEATURE(CPUCFG_LEXT2_MASK, cfg2, "lext2");
//    CPUCFG_CHECK_FEATURE(CPUCFG_LEXT3_MASK, cfg2, "lext3");
//
//    CPUCFG_CHECK_FEATURE(CPUCFG_LBT1_MASK, cfg2, "lbt1");
//    CPUCFG_CHECK_FEATURE(CPUCFG_LBT2_MASK, cfg2, "lbt2");
//    CPUCFG_CHECK_FEATURE(CPUCFG_LBT3_MASK, cfg2, "lbt3");
//
//    return code_size;
}

#endif

static void generate_native_break_code(void *code_buf)
{
    IR2_OPND *zero = &zero_ir2_opnd;
    append_ir2_opnd3(LISA_ADD_D, zero, zero, zero);
    append_ir2_opnd3(LISA_ADD_D, zero, zero, zero);
    append_ir2_opnd2i(LISA_JIRL, zero, &ra_ir2_opnd, 0);
}

int target_x86_to_mips_static_codes(void *code_buf)
{
    int code_nr = 0;
    int code_size = 0;
    void *code_buffer = code_buf;

    x86_to_mips_alloc_lsenv();

    /* genfn 1) only generate IR2
     *       2) no return
     *       3) get the number of IR2 via assembling */
#define XTM_STATIC_CODE_GEN_1(genfn, ...) do { \
    tr_init(NULL); \
    genfn(__VA_ARGS__); \
    code_nr += tr_ir2_assemble(code_buffer); \
    tr_fini(false); \
    code_buffer = code_buf + (code_nr << 2); \
} while(0)

    /* genfn 1) generates IR2 and assemble them
     *       2) return host binary size */
#define XTM_STATIC_CODE_GEN_2(genfn, ...) do { \
    tr_init(NULL); \
    code_size = genfn(__VA_ARGS__); \
    code_nr += code_size >> 2; \
    tr_fini(false); \
    code_buffer = code_buf + (code_nr << 2); \
} while(0)

    /* prologue: context switch BT to native */
    context_switch_bt_to_native = (ADDR)code_buffer;
    XTM_STATIC_CODE_GEN_1(
            generate_context_switch_bt_to_native,
            code_buffer);

    if (option_dump) {
        fprintf(stderr, "[X86toMIPS] context_switch_bt_to_native = %p\n",
                (void *)context_switch_bt_to_native);
    }

    /* epilogue: context switch native to BT */
    context_switch_native_to_bt_ret_0 = (ADDR)code_buffer;
    context_switch_native_to_bt = (ADDR)code_buffer + 4;
    XTM_STATIC_CODE_GEN_1(
            generate_context_switch_native_to_bt,
            code_buffer);

    if (option_dump) {
        fprintf(stderr, "[X86toMIPS] context_switch_native_to_bt = %p\n",
                (void *)context_switch_native_to_bt);
    }

    /* fpu rotate */
    XTM_STATIC_CODE_GEN_2(
            generate_native_rotate_fpu_by,
            code_buffer);

#ifdef CONFIG_SOFTMMU
    if (xtm_njc_opt()) {
        XTM_STATIC_CODE_GEN_2(
                generate_native_jmp_cache,
                code_buffer);
    }
#endif

    /* native jmp glue */
    XTM_STATIC_CODE_GEN_2(
            generate_native_jmp_glue_all,
            code_buffer);

    if (option_dump) {
        fprintf(stderr, "[X86toMIPS] native jmp glue 0 = %p\n",
                (void *)native_jmp_glue_0);
        fprintf(stderr, "[X86toMIPS] native jmp glue 1 = %p\n",
                (void *)native_jmp_glue_1);
        fprintf(stderr, "[X86toMIPS] native jmp glue 2 = %p\n",
                (void *)native_jmp_glue_2);
    }

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_FAST_CS)
    XTM_STATIC_CODE_GEN_2(
            generate_fastcs_jmp_glue_all,
            code_buffer);
#endif

    if (cam_enabled()) {
        XTM_STATIC_CODE_GEN_2(
                generate_native_cam,
                code_buffer);
    }

    /* Break Point for debuging :
     * 1. code to exetue this before the wanted TB executing
     * 2. set this as break point
     * 3. run in debug tool
     * Then the execution will stop right before the wanted TB */
    if (option_break_point) {
        xqm_break_point_code = (ADDR)code_buffer;
        XTM_STATIC_CODE_GEN_1(
                generate_native_break_code,
                code_buffer);
    }

#ifndef CONFIG_SOFTMMU
    /* Shadown Stack in user-mode */
    if (option_shadow_stack) {
        ss_match_fail_native = (ADDR)code_buffer;
        XTM_STATIC_CODE_GEN_1(
                ss_generate_match_fail_native_code,
                code_buffer);
    }
#else
    /* Static CS in sys-mode */
    if (staticcs_enabled()) {
        /* Static helper prologue  */
        sys_helper_prologue_default = (ADDR)code_buffer;
        XTM_STATIC_CODE_GEN_1(
                generate_static_sys_helper_prologue_cfg,
                default_helper_cfg);

        /* Static helper epilogue  */
        sys_helper_epilogue_default = (ADDR)code_buffer;
        XTM_STATIC_CODE_GEN_1(
                generate_static_sys_helper_epilogue_cfg,
                default_helper_cfg);
    } else {
        sys_helper_prologue_default = 0;
        sys_helper_epilogue_default = 0;
    }
#endif

#ifdef CONFIG_XTM_USE_LOONGEXT
    XTM_STATIC_CODE_GEN_2(
            generate_check_loongext,
            code_buffer);
#endif

#if defined(CONFIG_SOFTMMU)
    /* Do cross page TB-Link check in system mode */
    if (xtm_cpc_enabled()) {
        XTM_STATIC_CODE_GEN_2(generate_jmp_glue_cpc,
                code_buffer, 0);
        XTM_STATIC_CODE_GEN_2(generate_jmp_glue_cpc,
                code_buffer, 1);
    }
#endif

    x86_to_mips_free_lsenv();

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
    /* Clear monitor data to avoid counting the static codes */
    xtm_pf_clear_all();
#endif

    return code_nr;
}


/*
 * Functions to save/load mapping registers in static codes
 */

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_FAST_CS)

void tr_gen_static_save_registers_to_env(
        uint8_t gpr_to_save,
        uint8_t fpr_to_save,
        uint8_t xmm_lo_to_save,
        uint8_t xmm_hi_to_save,
        uint8_t vreg_to_save)
{
    lsassertm(0, "staticcs to be implemented in LoongArch.\n");
//    /* Copy from abrove.
//     * Use statis temp registers. */
//
//    tr_save_gprs_to_env(gpr_to_save);
//    tr_save_vreg_to_env(vreg_to_save);
//
//    IR2_OPND *fast_cs_mask = &stmp1_ir2_opnd;
//    IR2_OPND *tmp = &stmp2_ir2_opnd;
//
//    append_ir2_opnd2i(mips_lwu, fast_cs_mask, &env_ir2_opnd,
//            lsenv_offset_of_fast_cs_mask(lsenv));
//
//    IR2_OPND label_no_fpu = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2i(mips_andi, tmp, fast_cs_mask, XTM_FAST_CS_MASK_FPU);
//    append_ir2_opnd3(mips_beq, tmp, &zero_ir2_opnd, &label_no_fpu);
//    if (option_lsfpu) {
//        tr_save_lstop_to_env(&stmp1_ir2_opnd);
//        tr_fpu_disable_top_mode();
//    }
//    tr_save_fprs_to_env(fpr_to_save, 0);
//
//    append_ir2_opnd1(mips_label, &label_no_fpu);
//
//    IR2_OPND label_no_xmm = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2i(mips_andi, tmp, fast_cs_mask, XTM_FAST_CS_MASK_XMM);
//    append_ir2_opnd3(mips_beq, tmp, &zero_ir2_opnd, &label_no_xmm);
//    tr_save_xmms_to_env(xmm_lo_to_save, xmm_hi_to_save);
//
//    append_ir2_opnd1(mips_label, &label_no_xmm);
}

void tr_gen_static_load_registers_from_env(
        uint8_t gpr_to_load,
        uint8_t fpr_to_load,
        uint8_t xmm_lo_to_load,
        uint8_t xmm_hi_to_load,
        uint8_t vreg_to_load)
{
    lsassertm(0, "staticcs to be implemented in LoongArch.\n");
//    /* Copy from abrove.
//     * Use statis temp registers. */
//
//    tr_load_vreg_from_env(vreg_to_load);
//    tr_load_gprs_from_env(gpr_to_load);
//
//    IR2_OPND *fast_cs_mask = &stmp1_ir2_opnd;
//    IR2_OPND *tmp = &stmp2_ir2_opnd;
//
//    append_ir2_opnd2i(mips_lwu, fast_cs_mask, &env_ir2_opnd,
//            lsenv_offset_of_fast_cs_mask(lsenv));
//
//    IR2_OPND label_no_fpu = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2i(mips_andi, tmp, fast_cs_mask, XTM_FAST_CS_MASK_FPU);
//    append_ir2_opnd3(mips_beq, tmp, &zero_ir2_opnd, &label_no_fpu);
//    tr_load_fprs_from_env(fpr_to_load, 0);
//    if (option_lsfpu) {
//        tr_load_lstop_from_env(&stmp1_ir2_opnd);
//        tr_fpu_enable_top_mode();
//    }
//
//    append_ir2_opnd1(mips_label, &label_no_fpu);
//
//    IR2_OPND label_no_xmm = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2i(mips_andi, tmp, fast_cs_mask, XTM_FAST_CS_MASK_XMM);
//    append_ir2_opnd3(mips_beq, tmp, &zero_ir2_opnd, &label_no_xmm);
//    tr_load_xmms_from_env(xmm_lo_to_load, xmm_hi_to_load);
//
//    append_ir2_opnd1(mips_label, &label_no_xmm);
}

#else

void tr_gen_static_save_registers_to_env(
        uint8_t gpr_to_save,
        uint8_t fpr_to_save,
        uint8_t xmm_lo_to_save,
        uint8_t xmm_hi_to_save,
        uint8_t vreg_to_save)
{
    tr_save_gprs_to_env(gpr_to_save);
    if (option_lsfpu) {
        tr_save_lstop_to_env(&stmp1_ir2_opnd);
        tr_fpu_disable_top_mode();
    }
    tr_save_fprs_to_env(fpr_to_save, 0);
    tr_save_xmms_to_env(xmm_lo_to_save, xmm_hi_to_save);
    tr_save_vreg_to_env(vreg_to_save);
}

void tr_gen_static_load_registers_from_env(
        uint8_t gpr_to_load,
        uint8_t fpr_to_load,
        uint8_t xmm_lo_to_load,
        uint8_t xmm_hi_to_load,
        uint8_t vreg_to_load)
{
    tr_load_vreg_from_env(vreg_to_load);
    tr_load_xmms_from_env(xmm_lo_to_load, xmm_hi_to_load);
    tr_load_fprs_from_env(fpr_to_load, 0);
    if (option_lsfpu) {
        tr_load_lstop_from_env(&stmp1_ir2_opnd);
        tr_fpu_enable_top_mode();
    }
    tr_load_gprs_from_env(gpr_to_load);
}

#endif
