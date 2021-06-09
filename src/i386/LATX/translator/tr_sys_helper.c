/* X86toMIPS headers */
#include "../include/common.h"
#include "../include/reg_alloc.h"
#include "../include/env.h"
#include "../ir2/ir2.h"
#include "../x86tomips-options.h"

/* QEMU headers */
#include "../include/qemu-def.h"

#ifdef CONFIG_XTM_PROFILE
#include "../x86tomips-profile-sys.h"
#endif

helper_cfg_t all_helper_cfg = {
    .sv_allgpr = 1,
    .sv_eflags = 1,
    .cvt_fp80  = 1
};
helper_cfg_t zero_helper_cfg = {
    .sv_allgpr = 0,
    .sv_eflags = 0,
    .cvt_fp80  = 0
};
helper_cfg_t default_helper_cfg = {
    .sv_allgpr = 1,
    .sv_eflags = 1,
    .cvt_fp80  = 0
};

/* return 1: cfg1 == cfg2
 * return 0: cfg1 != cfg2 */
int cmp_helper_cfg(helper_cfg_t cfg1, helper_cfg_t cfg2)
{
    return cfg1.sv_allgpr == cfg2.sv_allgpr &&
           cfg1.sv_eflags == cfg2.sv_eflags &&
           cfg1.cvt_fp80  == cfg2.cvt_fp80;
}

int context_save_state(helper_cfg_t cfg)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (!td->in_context_saved) {
        return NATIVE_CONTEXT_OUT;
    }
    else {
        if (!cmp_helper_cfg(cfg, td->helper_cfg)) {
            return NATIVE_CONTEXT_DIFF;
        }
    }

    return NATIVE_CONTEXT_IN;
}

/* return 1 : context already saved, no need to save again
 * return 0 : first time ot save, do need to save context  */
static int context_save(helper_cfg_t cfg)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    int state = context_save_state(cfg);

    switch (state) {
    case NATIVE_CONTEXT_IN:
        /* context is already saved */
        td->context_save_depth += 1;
        return 1;
    case NATIVE_CONTEXT_OUT:
        td->in_context_saved    = 1;
        td->context_save_depth  = 1;
        td->helper_cfg          = cfg;
        return 0;
    case NATIVE_CONTEXT_DIFF:
        lsassertm(0, "inconsistency of nested context save.\n");
        break;
    default:
        lsassertm(0, "wrong state of context save.\n");
        break;
    }

    /* should never reach here */
    return -1;
}

/* return 1 : nested context saved, decrease depth
 * return 0 : last context saved, need to restore context  */
static int context_restore(helper_cfg_t cfg)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    int state = context_save_state(cfg);

    switch (state) {
    case NATIVE_CONTEXT_IN:
        td->context_save_depth -= 1;
        if (td->context_save_depth) {
            return 1;
        } else {
            td->in_context_saved = 0;
            return 0;
        }
    case NATIVE_CONTEXT_OUT:
        lsassertm(0, "restore before context save.\n");
        break;
    case NATIVE_CONTEXT_DIFF:
        lsassertm(0, "inconsistency of nested context save.\n");
        break;
    default:
        lsassertm(0, "wrong state of context save.\n");
        break;
    }

    /* should never reach here */
    return -1;
}

/* Prologue and Epilogue for helper function in system-mode 
 * 
 * If the helper requires complete ENV context, set @all_gpr to save
 * all the mapping registers.
 *
 * In the following situations, complete ENV is required:
 *  > helper that reads/writes all the GPRs...
 *      > helper_rsm
 *  > helper that might not return
 *      > helper_raise_exception: always no return
 *      > helper_raise_interrupt: always no return
 *      > helper that might triger interrupt/exception
 *          > many helper for system instruction
 *            - softmmu's helper : load/store
 *            - helper_ltr   : ltr
 *            - helper_lldt  : lldt
 *            - helper_pause : nop
 *  */
void tr_sys_gen_call_to_helper_prologue(int sv_allgpr, int sv_eflags)
{
    helper_cfg_t cfg;
    cfg.sv_allgpr = sv_allgpr;
    cfg.sv_eflags = sv_eflags;
    cfg.cvt_fp80  = default_helper_cfg.cvt_fp80;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);
}
void tr_sys_gen_call_to_helper_prologue_cfg(helper_cfg_t cfg)
{
    if (context_save(cfg)) {
        return;
    }

    /* Use static helper prologue for default */
    if (staticcs_enabled() &&
        cmp_helper_cfg(cfg, default_helper_cfg))
    {
        void *code_buf = lsenv->tr_data->curr_tb->tc.ptr;
        int offset = lsenv->tr_data->real_ir2_inst_num << 2;
        append_ir2_opnda(LISA_BL, (sys_helper_prologue_default
                    - (ADDR)code_buf - offset) >> 2);
        if (!option_lsfpu) tr_gen_save_curr_top();
        return;
    }

#ifdef CONFIG_XTM_PROFILE
    tr_pf_inc_helper();
#endif

    if (likely(cfg.sv_allgpr)) {
//        tr_save_em_eb();
        tr_save_registers_to_env(0xff, 0xff, 1, 0xff, 0xff, 0x7|options_to_save()); 
#ifdef CONFIG_XTM_PROFILE
        tr_pf_inc_helper_allgpr();
#ifdef CONFIG_XTM_FAST_CS
        TranslationBlock *tb = lsenv->tr_data->curr_tb;
        uint8_t fast_cs_mask = tb->extra_tb->fast_cs_mask;
        if (fast_cs_mask & XTM_FAST_CS_MASK_FPU) {
            tr_pf_inc_helper_fpu();
        }
        if (fast_cs_mask & XTM_FAST_CS_MASK_XMM) {
            tr_pf_inc_helper_xmm();
        }
#else
        tr_pf_inc_helper_fpu();
        tr_pf_inc_helper_xmm();
#endif
#endif
    } else {
        tr_save_registers_to_env(
                GPR_USEDEF_TO_SAVE,
                FPR_USEDEF_TO_SAVE, 1,
                XMM_LO_USEDEF_TO_SAVE,
                XMM_HI_USEDEF_TO_SAVE,
                0x7|options_to_save()); 
    }

    if (likely(cfg.sv_eflags))
        tr_save_eflags();

    if (unlikely(cfg.cvt_fp80)) {
        tr_save_temp_register();
        tr_cvt_fp64_to_80();
        tr_restore_temp_register();
    }
}

void tr_sys_gen_call_to_helper_epilogue(int sv_allgpr, int sv_eflags)
{
    helper_cfg_t cfg;
    cfg.sv_allgpr = sv_allgpr;
    cfg.sv_eflags = sv_eflags;
    cfg.cvt_fp80  = default_helper_cfg.cvt_fp80;
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
}
void tr_sys_gen_call_to_helper_epilogue_cfg(helper_cfg_t cfg)
{
    if (context_restore(cfg)) {
        return;
    }

    /* Use static helper epilogue for default */
    if (staticcs_enabled() &&
        cmp_helper_cfg(cfg, default_helper_cfg))
    {
        void *code_buf = lsenv->tr_data->curr_tb->tc.ptr;
        int offset = lsenv->tr_data->real_ir2_inst_num << 2;
        append_ir2_opnda(LISA_BL, (sys_helper_epilogue_default
                    - (ADDR)code_buf - offset) >> 2);
        return;
    }

    if (unlikely(cfg.cvt_fp80))
        tr_cvt_fp80_to_64();

    if (likely(cfg.sv_eflags))
        tr_load_eflags(0);

    if (likely(cfg.sv_allgpr)) {
        tr_load_registers_from_env(0xff, 0xff, 1, 0xff, 0xff, 0x7|options_to_save());
//        tr_load_em_eb();
    } else {
        tr_load_registers_from_env(
                GPR_USEDEF_TO_SAVE,
                FPR_USEDEF_TO_SAVE, 1,
                XMM_LO_USEDEF_TO_SAVE,
                XMM_HI_USEDEF_TO_SAVE,
                0x7|options_to_save());
    }
}

/* helper with zero arg */
void tr_sys_gen_call_to_helper0(ADDR func, int all_gpr, int eflags)
{
    helper_cfg_t cfg;
    cfg.sv_allgpr = all_gpr;
    cfg.sv_eflags = eflags;
    cfg.cvt_fp80  = default_helper_cfg.cvt_fp80;
    tr_sys_gen_call_to_helper0_cfg(func, cfg);
}
void tr_sys_gen_call_to_helper0_cfg(ADDR func, helper_cfg_t cfg)
{
    IR2_OPND func_addr_opnd = ra_alloc_itemp();

    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    load_addr_to_ir2(&func_addr_opnd, (ADDR)func);
    append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    ra_free_temp(&func_addr_opnd);

    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
}

/* helper with 1 default arg(CPUArchState*) */ 
void tr_sys_gen_call_to_helper1(ADDR func, int all_gpr, int eflags)
{
    helper_cfg_t cfg;
    cfg.sv_allgpr = all_gpr;
    cfg.sv_eflags = eflags;
    cfg.cvt_fp80  = default_helper_cfg.cvt_fp80;
    tr_sys_gen_call_to_helper1_cfg(func, cfg);
}
void tr_sys_gen_call_to_helper1_cfg(ADDR func, helper_cfg_t cfg)
{
    IR2_OPND func_addr_opnd = ra_alloc_itemp();

    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    load_addr_to_ir2(&func_addr_opnd, (ADDR)func);

    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    ra_free_temp(&func_addr_opnd);

    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
}

/* helper with 2 arg(CPUArchState*, int) */ 
void tr_sys_gen_call_to_helper2(ADDR func, int arg2, int all_gpr, int eflags)
{
    helper_cfg_t cfg;
    cfg.sv_allgpr = all_gpr;
    cfg.sv_eflags = eflags;
    cfg.cvt_fp80  = default_helper_cfg.cvt_fp80;
    tr_sys_gen_call_to_helper2_cfg(func, arg2, cfg);
}
void tr_sys_gen_call_to_helper2_cfg(ADDR func, int arg2, helper_cfg_t cfg)
{
    IR2_OPND func_addr_opnd = ra_alloc_itemp();

    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    load_addr_to_ir2(&func_addr_opnd, func);
    load_imm32_to_ir2(&arg1_ir2_opnd, arg2, SIGN_EXTENSION);

    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    ra_free_temp(&func_addr_opnd);

    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
}

/* helper with 3 arg(CPUArchState*, int, int) */ 
void tr_sys_gen_call_to_helper3(ADDR func, int arg2, int arg3, int all_gpr, int eflags)
{
    helper_cfg_t cfg;
    cfg.sv_allgpr = all_gpr;
    cfg.sv_eflags = eflags;
    cfg.cvt_fp80  = default_helper_cfg.cvt_fp80;
    tr_sys_gen_call_to_helper3_cfg(func, arg2, arg3, cfg);
}
void tr_sys_gen_call_to_helper3_cfg(ADDR func, int arg2, int arg3, helper_cfg_t cfg)
{
    IR2_OPND func_addr_opnd = ra_alloc_itemp();

    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    load_addr_to_ir2(&func_addr_opnd, func);
    load_imm32_to_ir2(&arg1_ir2_opnd, arg2, SIGN_EXTENSION);
    load_imm32_to_ir2(&arg2_ir2_opnd, arg3, SIGN_EXTENSION);

    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    ra_free_temp(&func_addr_opnd);

    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
}

/* helper with 3 arg(CPUArchState*, uint64, uint64) */ 
void tr_sys_gen_call_to_helper3_u64(ADDR func, uint64 arg2, uint64 arg3, int all_gpr, int eflags)
{
    helper_cfg_t cfg;
    cfg.sv_allgpr = all_gpr;
    cfg.sv_eflags = eflags;
    cfg.cvt_fp80  = default_helper_cfg.cvt_fp80;
    tr_sys_gen_call_to_helper3_u64_cfg(func, arg2, arg3, cfg);
}
void tr_sys_gen_call_to_helper3_u64_cfg(ADDR func, uint64 arg2, uint64 arg3, helper_cfg_t cfg)
{
    IR2_OPND func_addr_opnd = ra_alloc_itemp();

    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    load_addr_to_ir2(&func_addr_opnd, func);
    load_imm64_to_ir2(&arg1_ir2_opnd, arg2);
    load_imm64_to_ir2(&arg2_ir2_opnd, arg3);

    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    ra_free_temp(&func_addr_opnd);

    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
}
void tr_sys_gen_call_to_helper4_u64(ADDR func, uint64 arg2, uint64 arg3, uint64 arg4, int all_gpr, int eflags)
{
    helper_cfg_t cfg;
    cfg.sv_allgpr = all_gpr;
    cfg.sv_eflags = eflags;
    cfg.cvt_fp80  = default_helper_cfg.cvt_fp80;
    tr_sys_gen_call_to_helper4_u64_cfg(func, arg2, arg3, arg4, cfg);
}
void tr_sys_gen_call_to_helper4_u64_cfg(ADDR func, uint64 arg2, uint64 arg3, uint64 arg4, helper_cfg_t cfg)
{
    IR2_OPND func_addr_opnd = ra_alloc_itemp();

    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    load_addr_to_ir2(&func_addr_opnd, func);
    load_imm64_to_ir2(&arg1_ir2_opnd, arg2);
    load_imm64_to_ir2(&arg2_ir2_opnd, arg3);
    load_imm64_to_ir2(&arg3_ir2_opnd, arg4);

    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    ra_free_temp(&func_addr_opnd);

    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
}

void tr_gen_infinite_loop(void)
{
    IR2_OPND label_opnd = ir2_opnd_new_label(); 
    append_ir2_opnd1(LISA_LABEL, &label_opnd);
    append_ir2_opnd1(LISA_B, &label_opnd);
    append_ir2_opnd0_(lisa_nop);
}

void tr_gen_save_curr_eip(void)
{
    lsassert(lsenv->tr_data->curr_ir1_inst);

    IR1_INST *pir1 = lsenv->tr_data->curr_ir1_inst;
    ADDRX curr_eip = ir1_addr(pir1);
    IR2_OPND eip_opnd = ra_alloc_itemp();
    load_addrx_to_ir2(&eip_opnd, curr_eip);

    append_ir2_opnd2i(LISA_ST_W, &eip_opnd, &env_ir2_opnd,
                      lsenv_offset_of_eip(lsenv));

    ra_free_temp(&eip_opnd);
}

void tr_gen_save_next_eip(void)
{
    lsassert(lsenv->tr_data->curr_ir1_inst);

    IR1_INST *pir1 = lsenv->tr_data->curr_ir1_inst;
    ADDRX next_eip = ir1_addr_next(pir1);
    IR2_OPND eip_opnd = ra_alloc_itemp();
    load_addrx_to_ir2(&eip_opnd, next_eip);

    append_ir2_opnd2i(LISA_ST_W, &eip_opnd, &env_ir2_opnd,
                      lsenv_offset_of_eip(lsenv));

    ra_free_temp(&eip_opnd);
}

#ifdef REG_ALLOC_ALG_IMM

void tr_save_temp_register_mask(int mask)
{
    TEMP_REG_STATUS  *p  = itemp_status_default;
    int i;
    for (i = 0; i < itemp_status_num; ++i) {
        if ((mask >> i) & 0x1) {
            int ireg = p[i].physical_id;
            IR2_OPND gpr = ir2_opnd_new(IR2_OPND_GPR, ireg);
            append_ir2_opnd2i(LISA_ST_D, &gpr, &env_ir2_opnd,
                lsenv_offset_of_mips_iregs(lsenv, ireg));
        }
    }
}

void tr_restore_temp_register_mask(int mask)
{
    TEMP_REG_STATUS  *p  = itemp_status_default;
    int i;
    for (i = 0; i < itemp_status_num; ++i) {
        if ((mask >> i) & 0x1) {
            int ireg = p[i].physical_id;
            IR2_OPND gpr = ir2_opnd_new(IR2_OPND_GPR, ireg);
            append_ir2_opnd2i(LISA_LD_D, &gpr, &env_ir2_opnd,
                lsenv_offset_of_mips_iregs(lsenv, ireg));
        }
    }
}


/* Only need to save current used temp registers */
void tr_save_temp_register(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
//    TEMP_REG_STATUS  *p  = td->itemp_status;
//    TEMP_REG_STATUS  *pb = td->itemp_status_bak;
    TEMP_REG_STATUS  *p  = itemp_status_default;

    int mask = td->itemp_mask;
    td->itemp_mask_bk = mask;

    lsassertm(!(td->itemp_saved),
            "Can not save temp again after save temp.\n");

    int i;
    for (i = 0; i < itemp_status_num; ++i) {
//        if ((mask >> i) & 0x1) {
            int ireg = p[i].physical_id;
            IR2_OPND gpr = ir2_opnd_new(IR2_OPND_GPR, ireg);
            append_ir2_opnd2i(LISA_ST_D, &gpr, &env_ir2_opnd,
                lsenv_offset_of_mips_iregs(lsenv, ireg));
//            pb[i].virtual_id = p[i].virtual_id;
//        }
    }

    td->itemp_saved = 1;
}

/* Only need to restore saved temp registers */
void tr_restore_temp_register(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
//    TEMP_REG_STATUS  *pb = td->itemp_status_bak;
    TEMP_REG_STATUS  *p  = itemp_status_default;

    int mask_bk = td->itemp_mask_bk;
    td->itemp_mask = mask_bk;

    lsassertm(td->itemp_saved,
            "Can not resoter temp again if not save temp.\n");

    int i;
    for (i = 0; i < itemp_status_num; ++i) {
        if ((mask_bk >> i) & 0x1) {
            int ireg = p[i].physical_id;
            IR2_OPND gpr = ir2_opnd_new(IR2_OPND_GPR, ireg);
            append_ir2_opnd2i(LISA_LD_D, &gpr, &env_ir2_opnd,
                lsenv_offset_of_mips_iregs(lsenv, ireg));
        }
    }

    td->itemp_saved = 0;
}

#else

void tr_save_temp_register(void)
{
    int temp_nr = ra_get_temp_nr();

    IR2_OPND gpr;
    int i, ireg;

    for (i = 0; i < temp_nr; ++i) {
        ireg = ra_get_temp_reg_by_idx(i);
        /* save temp register value */
        gpr = ir2_opnd_new(IR2_OPND_GPR, ireg);
        append_ir2_opnd2i(LISA_ST_D, &gpr, &env_ir2_opnd,
                lsenv_offset_of_mips_iregs(lsenv, ireg));
    }
}

void tr_restore_temp_register(void)
{
    int temp_nr = ra_get_temp_nr();

    IR2_OPND gpr;
    int i, ireg;

    for (i = 0; i < temp_nr; ++i) {
        ireg = ra_get_temp_reg_by_idx(i);
        /* restore temp register value */
        gpr = ir2_opnd_new(IR2_OPND_GPR, ireg);
        append_ir2_opnd2i(LISA_ST_D, &gpr, &env_ir2_opnd,
                lsenv_offset_of_mips_iregs(lsenv, ireg));
    }
}
#endif

int staticcs_enabled(void)
{
    return option_staticcs;
}

/* All temp registers should not be touch by static
 * generated prologue and epilogue codes.
 * Use stmp1_ir2_opnd & stmp2_ir2_opnd to mov data */

void generate_static_sys_helper_prologue_cfg(helper_cfg_t cfg)
{
    if (!staticcs_enabled()) return;

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
    tr_pf_inc_staticcs();
#endif

    int ra_reg_num = ir2_opnd_reg(&ra_ir2_opnd);
    append_ir2_opnd2i(LISA_ST_D, &ra_ir2_opnd, &env_ir2_opnd,
            lsenv_offset_of_mips_iregs(lsenv, ra_reg_num));

    /* FPU TOP will be stored outside */
    if ((cfg.sv_allgpr)) {
        tr_gen_static_save_registers_to_env(
                0xff, 0xff, 0xff, 0xff, 0x7); 
    } else {
        tr_gen_static_save_registers_to_env(
                GPR_USEDEF_TO_SAVE,
                FPR_USEDEF_TO_SAVE,
                XMM_LO_USEDEF_TO_SAVE,
                XMM_HI_USEDEF_TO_SAVE,
                0x7); 
    }

    if (cfg.sv_eflags) tr_gen_static_save_eflags();
    if (cfg.cvt_fp80) {
        lsassertm(0, "cvtfp not supported in staticcs.\n");
    }

    append_ir2_opnd2i(LISA_LD_D, &ra_ir2_opnd, &env_ir2_opnd,
            lsenv_offset_of_mips_iregs(lsenv, ra_reg_num));
    append_ir2_opnd0_(lisa_return);
}

void generate_static_sys_helper_epilogue_cfg(helper_cfg_t cfg)
{
    if (!staticcs_enabled()) return;

    int ra_reg_num = ir2_opnd_reg(&ra_ir2_opnd);
    append_ir2_opnd2i(LISA_ST_D, &ra_ir2_opnd, &env_ir2_opnd,
            lsenv_offset_of_mips_iregs(lsenv, ra_reg_num));

    if (cfg.cvt_fp80) {
        lsassertm(0, "cvtfp not supported in staticcs.\n");
    }
    if (cfg.sv_eflags) tr_gen_static_load_eflags(0);

    /* FPU TOP will be loaded outside */
    if (cfg.sv_allgpr) {
        tr_gen_static_load_registers_from_env(
                0xff, 0xff, 0xff, 0xff, 0x7);
    } else {
        tr_gen_static_load_registers_from_env(
                GPR_USEDEF_TO_SAVE,
                FPR_USEDEF_TO_SAVE,
                XMM_LO_USEDEF_TO_SAVE,
                XMM_HI_USEDEF_TO_SAVE,
                0x7);
    }

    append_ir2_opnd2i(LISA_LD_D, &ra_ir2_opnd, &env_ir2_opnd,
            lsenv_offset_of_mips_iregs(lsenv, ra_reg_num));
    append_ir2_opnd0_(lisa_return);
}
