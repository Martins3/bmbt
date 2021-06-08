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

static void tr_set_xtm_fpu_top_in_env(IR1_INST *ir1)
{
    if (option_lsfpu) lsassert(0);

    IR2_OPND value = ra_alloc_itemp();
    int val = 0;
    switch (ir1_opcode(ir1)) {
        case X86_INS_FRSTOR:
        case X86_INS_FXRSTOR:
        case X86_INS_XRSTOR:
            /* Both FPU state (top) and
             * FPRs is loaded, NO rotate */
            append_ir2_opnd2i(LISA_ST_W, &zero_ir2_opnd, &env_ir2_opnd,
                    lsenv_offset_of_top_bias(lsenv));
            /* fall through */
        case X86_INS_FLDENV:
            /* Only FPU state is loaded,
             * FPRs should rotate back */
            val |= XTM_FPU_LOAD_ST_MASK;    /* 0x80 */
            break;
        case X86_INS_FNINIT:
        case X86_INS_FNSAVE:
            /* Only FPU state is reseted,
             * FPRs should rotate back */
            val |= XTM_FPU_RESET_ST_MASK;   /* 0x40 */
            break;
        default:
            return;
    }
    append_ir2_opnd2i(LISA_ORI, &value, &zero_ir2_opnd, val);

    IR2_OPND fpu_top = ra_alloc_itemp();

    append_ir2_opnd2i(LISA_LD_WU, &fpu_top, &env_ir2_opnd, offsetof(CPUX86State, fpstt));
    append_ir2_opnd2i(LISA_ANDI,  &fpu_top, &fpu_top,      0x7);
    if (XTM_FPU_TOP_BIT)
        append_ir2_opnd2i(LISA_SLLI_D, &fpu_top, &fpu_top, XTM_FPU_TOP_BIT);

    append_ir2_opnd3 (LISA_OR,   &value, &value,       &fpu_top);
    append_ir2_opnd2i(LISA_ST_W, &value, &env_ir2_opnd, offsetof(CPUX86State, xtm_fpu));

    ra_free_temp(&fpu_top);

    lsenv->tr_data->ignore_top_update = 1;
}

static IR2_OPND tr_get_real_curr_top(void)
{
    IR2_OPND top = ra_alloc_itemp();
    if (!option_lsfpu) {
        /* 1. Load top-bias from env */
        IR2_OPND top_bias = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_LD_W, &top_bias, &env_ir2_opnd,
                lsenv_offset_of_top_bias(lsenv));
        /* 2. Real Current Top = (lsenv->curr_top + top_bias) & 0x7 */
        append_ir2_opnd2i(LISA_ORI,   &top, &zero_ir2_opnd,  td_fpu_get_top());
        append_ir2_opnd3 (LISA_ADD_D, &top, &top,           &top_bias);
        append_ir2_opnd2i(LISA_ANDI,  &top, &top,            0x7);
    }
    else {
        append_ir2_opnd1 (LISA_X86MFTOP, &top);
        append_ir2_opnd2i(LISA_ANDI,  &top, &top,  0x7);
    }
    return top;
}

bool translate_fisttp(IR1_INST *pir1) {return false;}
bool translate_fedisi(IR1_INST *pir1) {return false;}
bool translate_fbld(IR1_INST *pir1) {return false;}
bool translate_fbstp(IR1_INST *pir1) {return false;}

bool translate_fsetpm(IR1_INST *pir1)
{
    if (tr_gen_fp_common_excp_check(pir1)) return true;
    return true; /* fsetpm (287 only, just do nop here) */
}

bool translate_feni(IR1_INST *pir1)
{
    if (tr_gen_fp_common_excp_check(pir1)) return true;
    return true; /* feni (287 only, just do nop here) */
}

bool translate_fxsave(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) return true;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    lsassertm(ir1_opnd_is_mem(opnd0), "fxsave opnd0 is not mem.\n");

    /* 1. get memory address */
    IR2_OPND mem_opnd;
    convert_mem_opnd(&mem_opnd, opnd0, -1);
    int mem_no_offset_new_tmp = 0;
    IR2_OPND mem_no_offset = convert_mem_ir2_opnd_no_offset(&mem_opnd,
            &mem_no_offset_new_tmp);
    IR2_OPND addr_opnd = ir2_opnd_mem_get_base(&mem_no_offset);
    if (mem_no_offset_new_tmp) ra_free_temp(&mem_opnd);

    /* 2. save complete context */
    helper_cfg_t hp_cfg;
    hp_cfg.sv_allgpr = 1;
    hp_cfg.sv_eflags = 1;
    hp_cfg.cvt_fp80  = 1;
    tr_sys_gen_call_to_helper_prologue_cfg(hp_cfg);
    /* 3. call helper_fxsave
     *
     * target/i386/fpu_helper.c
     * void helper_fxsave(
     *      CPUX86State *env,
     *      target_ulong ptr)
     * >> All floating point registers will be used
     * >> 80-bit FP will be used
     */
    /* 3.1 arg1 : memory address */
    append_ir2_opnd2_(lisa_mov, &arg1_ir2_opnd, &addr_opnd);
    /* 3.2 arg0 : env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    /* 3.3 call helper */
    tr_gen_call_to_helper((ADDR)helper_fxsave);
    /* 4. restore context */
    tr_sys_gen_call_to_helper_epilogue_cfg(hp_cfg);
    return true;
}

/* End of TB in system-mode */
bool translate_fxrstor(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) return true;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    lsassertm(ir1_opnd_is_mem(opnd0), "fxrstor opnd0 is not mem.\n");

    /* 1. get memory address */
    IR2_OPND mem_opnd;
    convert_mem_opnd(&mem_opnd, opnd0, -1);
    int mem_no_offset_new_tmp = 0;
    IR2_OPND mem_no_offset = convert_mem_ir2_opnd_no_offset(&mem_opnd,
            &mem_no_offset_new_tmp);
    IR2_OPND addr_opnd = ir2_opnd_mem_get_base(&mem_no_offset);
    if (mem_no_offset_new_tmp) ra_free_temp(&mem_opnd);

    /* 2. save complete context */
    helper_cfg_t hp_cfg;
    hp_cfg.sv_allgpr = 1;
    hp_cfg.sv_eflags = 1;
    hp_cfg.cvt_fp80  = 1;
    tr_sys_gen_call_to_helper_prologue_cfg(hp_cfg);
    /* 3. call helper_fxrstor
     *
     * target/i386/fpu_helper.c
     * void helper_fxrstor(
     *      CPUX86State *env,
     *      target_ulong ptr)
     * >> All floating point registers will be used
     * >> 80-bit FP will be used
     */
    /* 3.1 arg1 : memory address */
    append_ir2_opnd2_(lisa_mov, &arg1_ir2_opnd, &addr_opnd);
    /* 3.2 arg0 : env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    /* 3.3 call helper */
    tr_gen_call_to_helper((ADDR)helper_fxrstor);
    /* 4. restore context */
    tr_sys_gen_call_to_helper_epilogue_cfg(hp_cfg);
    /* 5. set flag in env */
    if (!option_lsfpu) {
        tr_set_xtm_fpu_top_in_env(pir1);
    }

    return true;
}

bool translate_fnstenv(IR1_INST *pir1)
{
    if (tr_gen_fp_common_excp_check(pir1)) return true;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1), ir1_opnd_is_mem(opnd0),
            "fnstenv operand is not mem.\n");

    int data_size = ir1_data_size(pir1);
    int data32 = data_size == 32 ? 1 : 0;
    /* 1. get memory address */
    IR2_OPND mem_opnd ;
    convert_mem_opnd(&mem_opnd, opnd0, -1);
    int mem_no_offset_new_tmp = 0;
    IR2_OPND mem_no_offset = convert_mem_ir2_opnd_no_offset(&mem_opnd,
            &mem_no_offset_new_tmp);
    IR2_OPND address = ir2_opnd_mem_get_base(&mem_no_offset);
    if (mem_no_offset_new_tmp) ra_free_temp(&mem_opnd);

    /* 2. save context */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);
    /* 3. call helper_fstenv
     *
     * target/i386/fpu_helper.c
     * void helper_fstenv(
     *      CPUX86State *env,
     *      target_ulong ptr,
     *      int data32)
     * >> Control Word : env->fpuc        => 16-bit
     * >> Status  Word : env->fpus        => 16-bit
     * >> Tag     Word : env->fptags[0:7] => 16-bit tag word
     * >> FIP,FOP,.... : zero
     */
    /* 3.1 arg1: address */
    append_ir2_opnd2_(lisa_mov, &arg1_ir2_opnd, &address);
    /* 3.2 arg0: env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    /* 3.3 arg2: data32 */
    append_ir2_opnd2i(LISA_ORI, &arg2_ir2_opnd, &zero_ir2_opnd, data32);
    /* 3.4 call helper_frstor */
    tr_gen_call_to_helper((ADDR)helper_fstenv);
    /* 4. restore context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
    return true;
}

/* End of TB in system-mode */
bool translate_fldenv(IR1_INST *pir1)
{
    if (tr_gen_fp_common_excp_check(pir1)) return true;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1), ir1_opnd_is_mem(opnd0),
            "fldtenv operand is not mem.\n");

    int data_size = ir1_data_size(pir1);
    int data32 = data_size == 32 ? 1 : 0;
    /* 1. get memory address */
    IR2_OPND mem_opnd ;
    convert_mem_opnd(&mem_opnd, opnd0, -1);
    int mem_no_offset_new_tmp = 0;
    IR2_OPND mem_no_offset = convert_mem_ir2_opnd_no_offset(&mem_opnd,
            &mem_no_offset_new_tmp);
    IR2_OPND address = ir2_opnd_mem_get_base(&mem_no_offset);
    if (mem_no_offset_new_tmp) ra_free_temp(&mem_opnd);

    /* 2. save context */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);
    /* 3. call helper_fldenv
     *
     * target/i386/fpu_helper.c
     * void helper_fldenv(
     *      CPUX86State *env,
     *      target_ulong ptr,
     *      int data32)
     * >> Control Word : env->fpuc        => 16-bit
     * >> Status  Word : env->fpus        => 16-bit
     * >> Tag     Word : env->fptags[0:7] => 16-bit tag word
     * >> FIP,FOP,.... : zero
     */
    /* 3.1 arg1: address */
    append_ir2_opnd2_(lisa_mov, &arg1_ir2_opnd, &address);
    /* 3.2 arg0: env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    /* 3.3 arg2: data32 */
    append_ir2_opnd2i(LISA_ORI, &arg2_ir2_opnd, &zero_ir2_opnd, data32);
    /* 3.4 call helper_frstor */
    tr_gen_call_to_helper((ADDR)helper_fldenv);
    /* 4. restore context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
    /* 5. set flag in env */
    if (!option_lsfpu) {
        tr_set_xtm_fpu_top_in_env(pir1);
    }

    return true;
}

/* End of TB in system-mode */
bool translate_fnsave(IR1_INST *pir1)
{
    if (tr_gen_fp_common_excp_check(pir1)) return true;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1), ir1_opnd_is_mem(opnd0),
            "fnsave operand is not mem.\n");

    int data_size = ir1_data_size(pir1);
    int data32 = data_size == 32 ? 1 : 0;
    /* 1. get memory address */
    IR2_OPND mem_opnd ;
    convert_mem_opnd(&mem_opnd, opnd0, -1);
    int mem_no_offset_new_tmp = 0;
    IR2_OPND mem_no_offset = convert_mem_ir2_opnd_no_offset(&mem_opnd,
            &mem_no_offset_new_tmp);
    IR2_OPND address = ir2_opnd_mem_get_base(&mem_no_offset);
    if (mem_no_offset_new_tmp) ra_free_temp(&mem_opnd);

    /* 2. save context, convert to 80fp */
    helper_cfg_t cfg = all_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);
    /* call helper_fsave
     *
     * void helper_fsave(
     *      CPUX86State *env,
     *      target_ulong ptr,
     *      int data32)
     * >> Control Word : env->fpuc        => 16-bit
     * >> Status  Word : env->fpus        => 16-bit
     * >> Tag     Word : env->fptags[0:7] => 16-bit tag word
     * >> FIP,FOP,.... : zero
     * >> 80-bit  FPRs : env->fpregs[0:7] => 80-bit * 8
     */
    /* 3.1 arg1: address */
    append_ir2_opnd2_(lisa_mov, &arg1_ir2_opnd, &address);
    /* 3.2 arg0: env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    /* 3.3 arg2: data32 */
    append_ir2_opnd2i(LISA_ORI, &arg2_ir2_opnd, &zero_ir2_opnd, data32);
    /* 3.4 call helper_frstor */
    tr_gen_call_to_helper((ADDR)helper_fsave);
    /* 4. restore context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
    /* 5. set flag in env */
    if (!option_lsfpu) {
        tr_set_xtm_fpu_top_in_env(pir1);
    }

    return true;
}

/* End of TB in system-mode */
bool translate_frstor(IR1_INST *pir1)
{
    if (tr_gen_fp_common_excp_check(pir1)) return true;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1), ir1_opnd_is_mem(opnd0),
            "frstor operand is not mem.\n");

    int data_size = ir1_data_size(pir1);
    int data32 = data_size == 32 ? 1 : 0;
    /* 1. get memory address */
    IR2_OPND mem_opnd ;
    convert_mem_opnd(&mem_opnd, opnd0, -1);
    int mem_no_offset_new_tmp = 0;
    IR2_OPND mem_no_offset = convert_mem_ir2_opnd_no_offset(&mem_opnd,
            &mem_no_offset_new_tmp);
    IR2_OPND address = ir2_opnd_mem_get_base(&mem_no_offset);
    if (mem_no_offset_new_tmp) ra_free_temp(&mem_opnd);

    /* 2. save complete context, along with fp80 convert */
    helper_cfg_t cfg = all_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);
    /* 3. call helper_frstor
     *
     * target/i386/fpu_helper.c
     * void helper_frstor(
     *      CPUX86State *env,
     *      target_ulong ptr,
     *      int data32)
     * >> Control Word : env->fpuc        => 16-bit
     * >> Status  Word : env->fpus        => 16-bit
     * >> Tag     Word : env->fptags[0:7] => 16-bit tag word
     * >> FIP,FOP,.... : zero
     * >> 80-bit  FPRs : env->fpregs[0:7] => 80-bit * 8
     */
    /* 3.1 arg1: address */
    append_ir2_opnd2_(lisa_mov, &arg1_ir2_opnd, &address);
    /* 3.2 arg0: env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    /* 3.3 arg2: data32 */
    append_ir2_opnd2i(LISA_ORI, &arg2_ir2_opnd, &zero_ir2_opnd, data32);
    /* 3.4 call helper_frstor */
    tr_gen_call_to_helper((ADDR)helper_frstor);
    /* 4. restore context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
    /* 5. set flag in env */
    if (!option_lsfpu) {
        tr_set_xtm_fpu_top_in_env(pir1);
    }

    return true;
}

bool translate_xgetbv(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) return true;

    IR2_OPND ecx_opnd = ra_alloc_gpr(ecx_index);
    /* 1. call helper_xgetbv
     *
     * target/i386/fpu_helper.c
     * uint64_t helper_xgetbv(
     *      CPUX86State *env,
     *      uint32_t ecx)
     * > Exception might be generated */
    helper_cfg_t cfg = default_helper_cfg;
    /* 1.1 save context */
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);
    /* 1.2 arg1 : ecx */
    append_ir2_opnd2_(lisa_mov32z, &arg1_ir2_opnd, &ecx_opnd);
    /* 1.3 arg0 : env */
    append_ir2_opnd2_(lisa_mov,    &arg0_ir2_opnd, &env_ir2_opnd);
    /* 1.4 call helper */
    tr_gen_call_to_helper((ADDR)helper_xgetbv);
    /* 1.5 restore context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
    /* 2. get return value EDX:EAX */
    IR2_OPND eax_value = ra_alloc_itemp();
    append_ir2_opnd2_(lisa_mov32z, &eax_value, &ret0_ir2_opnd);
    IR2_OPND edx_value = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_SRLI_D, &ret0_ir2_opnd, &ret0_ir2_opnd, 32);
    append_ir2_opnd2_(lisa_mov32z, &edx_value, &ret0_ir2_opnd);
    /* 3. save into eax and edx */
    store_ir2_to_ir1(&eax_value, &eax_ir1_opnd, false);
    store_ir2_to_ir1(&edx_value, &edx_ir1_opnd, false);
    return true;
}

/* End of TB in system-mode */
bool translate_xsetbv(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) return true;

    IR2_OPND ecx_opnd = ra_alloc_gpr(ecx_index);
    IR2_OPND eax_opnd = ra_alloc_gpr(eax_index);
    IR2_OPND edx_opnd = ra_alloc_gpr(edx_index);
    /* 1. get EDX:EAX */
    IR2_OPND mask = ra_alloc_itemp();
    IR2_OPND eax_opnd32 = ra_alloc_itemp();
    
    append_ir2_opnd2_(lisa_mov32z, &mask, &edx_opnd);
    append_ir2_opnd2i(LISA_SLLI_D, &mask, &mask, 32);
    append_ir2_opnd2_(lisa_mov32z, &eax_opnd32, &eax_opnd);
    append_ir2_opnd3 (LISA_AND,    &mask, &mask, &eax_opnd32);
    /* 2. call helper_xsetbv
     *
     * target/i386/fpu_helper.c
     * void helper_xsetbv(
     *      CPUX86State *env,
     *      uint32_t ecx,
     *      uint64_t mask)
     * > hflags might be changed, so this is an EOB */
    helper_cfg_t cfg = default_helper_cfg;
    /* 2.1 save context */
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);
    /* 2.2 arg2: mask */
    append_ir2_opnd2_(lisa_mov, &arg2_ir2_opnd, &mask);
    /* 2.3 arg1: ecx */
    append_ir2_opnd2_(lisa_mov32z, &arg1_ir2_opnd, &ecx_opnd);
    /* 2.4 arg0 : env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    /* 2.5 call helper */
    tr_gen_call_to_helper((ADDR)helper_xsetbv);
    /* 2.6 restore context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
    return true;
}

bool translate_xsave(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) return true;

    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1), ir1_opnd_is_mem(opnd),
            "xsave operand is not MEM.\n");

    /* 1. get memory address from operand */
    IR2_OPND mem_opnd;
    convert_mem_opnd(&mem_opnd, opnd, -1);
    int mem_no_offset_new_tmp = 0;
    IR2_OPND mem_no_offset = convert_mem_ir2_opnd_no_offset(&mem_opnd,
            &mem_no_offset_new_tmp);
    IR2_OPND mem_addr_opnd = ir2_opnd_mem_get_base(&mem_no_offset);
    if (mem_no_offset_new_tmp) ra_free_temp(&mem_opnd);

    /* 2. get rfbm from eax and edx */
    IR2_OPND rfbm_opnd = ra_alloc_itemp();
    IR2_OPND eax_opnd = ra_alloc_gpr(eax_index);
    IR2_OPND edx_opnd = ra_alloc_gpr(edx_index);
    IR2_OPND eax_opnd32 = ra_alloc_itemp(); /* eax might be sign extension */

    append_ir2_opnd2i(LISA_SLLI_D, &rfbm_opnd, &edx_opnd, 32);
    append_ir2_opnd2_(lisa_mov32z, &eax_opnd32, &eax_opnd);
    append_ir2_opnd3(LISA_OR, &rfbm_opnd, &eax_opnd32, &rfbm_opnd);
    /* 3. call helper_xsave
     *
     * void helper_xsave(
     *      CPUX86State *env,
     *      target_ulong ptr,
     *      uint64_t rfbm)
     * >> FPU might be saved */
    helper_cfg_t cfg = all_helper_cfg;
    /* 3.1 save context */
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);
    /* 3.2 arg2 : rfbm */
    append_ir2_opnd2_(lisa_mov, &arg2_ir2_opnd, &rfbm_opnd);
    /* 3.3 arg0 : env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    /* 3.4 arg1 : memory address */
    append_ir2_opnd2_(lisa_mov, &arg1_ir2_opnd, &mem_addr_opnd);
    /* 3.5 call helper xsave */
    tr_gen_call_to_helper((ADDR)helper_xsave);
    /* 3.6 restore context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
    return true;
}

/* End of TB in system-mode */
bool translate_xrstor(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) return true;

    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1), ir1_opnd_is_mem(opnd),
            "xsave operand is not MEM.\n");

    /* 1. get memory address from operand */
    IR2_OPND mem_opnd;
    convert_mem_opnd(&mem_opnd, opnd, -1);
    int mem_no_offset_new_tmp = 0;
    IR2_OPND mem_no_offset = convert_mem_ir2_opnd_no_offset(&mem_opnd,
            &mem_no_offset_new_tmp);
    IR2_OPND mem_addr_opnd = ir2_opnd_mem_get_base(&mem_no_offset);
    if (mem_no_offset_new_tmp) ra_free_temp(&mem_opnd);

    /* 2. get rfbm from eax and edx */
    IR2_OPND rfbm_opnd = ra_alloc_itemp();
    IR2_OPND eax_opnd = ra_alloc_gpr(eax_index);
    IR2_OPND edx_opnd = ra_alloc_gpr(edx_index);
    IR2_OPND eax_opnd32 = ra_alloc_itemp(); /* eax might be sign extension */

    append_ir2_opnd2i(LISA_SLLI_D, &rfbm_opnd, &edx_opnd, 32);
    append_ir2_opnd2_(lisa_mov32z, &eax_opnd32, &eax_opnd);
    append_ir2_opnd3(LISA_OR, &rfbm_opnd, &eax_opnd32, &rfbm_opnd);
    /* 3. call helper_xrstor
     *
     * void helper_xrstor(
     *      CPUX86State *env,
     *      target_ulong ptr,
     *      uint64_t rfbm)
     * >> FPU might be restored
     * >> XRSTOR is how MPX is enabled, we need to end the TB */
    helper_cfg_t cfg = all_helper_cfg;
    /* 3.1 save context */
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);
    /* 3.2 arg2 : rfbm */
    append_ir2_opnd2_(lisa_mov, &arg2_ir2_opnd, &rfbm_opnd);
    /* 3.3 arg0 : env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    /* 3.4 arg1 : memory address */
    append_ir2_opnd2_(lisa_mov, &arg1_ir2_opnd, &mem_addr_opnd);
    /* 3.5 call helper xsave */
    tr_gen_call_to_helper((ADDR)helper_xrstor);
    /* 3.6 restore context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
    /* 4. set flag in env */
    if (!option_lsfpu) {
        tr_set_xtm_fpu_top_in_env(pir1);
    }

    return true;
}

bool translate_xsaveopt(IR1_INST *pir1)
{
    if (tr_gen_excp_check(pir1)) return true;

    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1), ir1_opnd_is_mem(opnd),
            "xsaveopt operand is not memory.\n");

    /* 1. get memory address from operand */
    IR2_OPND mem_opnd;
    convert_mem_opnd(&mem_opnd, opnd, -1);
    int mem_no_offset_new_tmp = 0;
    IR2_OPND mem_no_offset = convert_mem_ir2_opnd_no_offset(&mem_opnd,
            &mem_no_offset_new_tmp);
    IR2_OPND mem_addr_opnd = ir2_opnd_mem_get_base(&mem_no_offset);
    if (mem_no_offset_new_tmp) ra_free_temp(&mem_opnd);

    /* 2. get rfbm from eax and edx */
    IR2_OPND rfbm_opnd = ra_alloc_itemp();
    IR2_OPND eax_opnd = ra_alloc_gpr(eax_index);
    IR2_OPND edx_opnd = ra_alloc_gpr(edx_index);
    IR2_OPND eax_opnd32 = ra_alloc_itemp(); /* eax might be sign extension */

    append_ir2_opnd2i(LISA_SLLI_D, &rfbm_opnd, &edx_opnd, 32);
    append_ir2_opnd2_(lisa_mov32z, &eax_opnd32, &eax_opnd);
    append_ir2_opnd3(LISA_OR, &rfbm_opnd, &eax_opnd32, &rfbm_opnd);

    /* 3. call helper_xsaveopt
     *
     * void helper_xsaveopt(
     *      CPUX86State *env,
     *      target_ulong ptr,
     *      uint64_t rfbm)
     */
    helper_cfg_t cfg = all_helper_cfg;
    /* 3.1 save context */
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);
    /* 3.2 arg2 : rfbm */
    append_ir2_opnd2_(lisa_mov, &arg2_ir2_opnd, &rfbm_opnd);
    /* 3.3 arg0 : env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    /* 3.4 arg1 : memory address */
    append_ir2_opnd2_(lisa_mov, &arg1_ir2_opnd, &mem_addr_opnd);
    /* 3.5 call helper xsave */
    tr_gen_call_to_helper((ADDR)helper_xsaveopt);
    /* 3.6 restore context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
    return true;
}

/* End of TB in system-mode */
bool translate_fninit(IR1_INST *pir1)
{
    if (tr_gen_fp_common_excp_check(pir1)) return true;

    /* 1. reset control word = 0x37f */
    IR2_OPND cw = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ORI, &cw, &zero_ir2_opnd, 0x37f);
    append_ir2_opnd2i(LISA_ST_H, &cw, &env_ir2_opnd,
            lsenv_offset_of_control_word(lsenv));
    /* 2. reset status word = 0 */
    append_ir2_opnd2i(LISA_ST_H, &zero_ir2_opnd, &env_ir2_opnd,
            lsenv_offset_of_status_word(lsenv));
    /* 3. reset top = 0 */
    append_ir2_opnd2i(LISA_ST_W, &zero_ir2_opnd, &env_ir2_opnd,
            lsenv_offset_of_top(lsenv));
    if (!option_lsfpu) td_fpu_set_top(0);
    /* 4. reset fptags = 0b11 (but = 1 in helper_fninit )*/
    IR2_OPND tag = ra_alloc_itemp();
    load_imm64_to_ir2(&tag, 0x0101010101010101);
    append_ir2_opnd2i(LISA_ST_D, &tag, &env_ir2_opnd,
            lsenv_offset_of_tag_word(lsenv));
    /* 5. set flag in env */
    if (!option_lsfpu) {
        tr_set_xtm_fpu_top_in_env(pir1);
    }

    return true;
}

bool translate_ffree(IR1_INST *pir1)
{
    if (tr_gen_fp_common_excp_check(pir1)) return true;

    /* 1. get real current top
     *    The td->curr_top might be rotated along with the FPRs.
     *    But the tag word is not rotated.
     *    It should be fixed with top_bias. */
    IR2_OPND real_top = tr_get_real_curr_top();
    /* 2. tag_offset = (real_top + reg) & 0x7 */
    IR2_OPND tag_offset_opnd = ra_alloc_itemp();
    int reg = ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0));
    append_ir2_opnd2i(LISA_ADDI_D, &tag_offset_opnd, &real_top,         reg);
    append_ir2_opnd2i(LISA_ANDI,   &tag_offset_opnd, &tag_offset_opnd,  0x7);
    /* 3. address of tag = env + tag_offset. */
    IR2_OPND address = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ADDI_D, &address, &env_ir2_opnd,
            lsenv_offset_of_tag_word(lsenv));
    append_ir2_opnd3(LISA_ADD_D,   &address, &address, &tag_offset_opnd);
    /* 4. set tag = 1 */
    IR2_OPND tag = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ORI,  &tag, &zero_ir2_opnd, 1);
    append_ir2_opnd2i(LISA_ST_B, &tag, &address,       0);
    return true;
}

bool translate_fincstp(IR1_INST *pir1)
{
    if (tr_gen_fp_common_excp_check(pir1)) return true;

    /* 1. increase the top-of-stack pointer : td->curr_top */
    tr_fpu_inc();
    /* 2. set C1 flag to 0 */
    IR2_OPND mask = ra_alloc_itemp();
    IR2_OPND value_status = ra_alloc_itemp();

    append_ir2_opnd2i(LISA_LD_H, &value_status, &env_ir2_opnd,
                      lsenv_offset_of_status_word(lsenv)); /* status_word */
    load_imm32_to_ir2(&mask, 0xfdffULL, ZERO_EXTENSION);
    append_ir2_opnd3 (LISA_AND,  &value_status, &value_status, &mask);
    append_ir2_opnd2i(LISA_ST_H, &value_status, &env_ir2_opnd,
                      lsenv_offset_of_status_word(lsenv)); /* status_word */
    return true;
}

bool translate_fdecstp(IR1_INST *pir1)
{
    if (tr_gen_fp_common_excp_check(pir1)) return true;

    /* 1. decrease the top-of-stack pointer : td->curr_top */
    tr_fpu_dec();
    /* 2. set C1 flag to 0 */
    IR2_OPND mask = ra_alloc_itemp();
    IR2_OPND value_status = ra_alloc_itemp();

    append_ir2_opnd2i(LISA_LD_H, &value_status, &env_ir2_opnd,
                      lsenv_offset_of_status_word(lsenv)); /* status_word */
    load_imm32_to_ir2(&mask, 0xfdffULL, ZERO_EXTENSION);
    append_ir2_opnd3(LISA_AND, &value_status, &value_status, &mask);
    append_ir2_opnd2i(LISA_ST_H, &value_status, &env_ir2_opnd,
                      lsenv_offset_of_status_word(lsenv)); /* status_word */
    
    return true;
}

/*
 * The following instructions will never be generated
 * by capstone, but we still implement it.
 *
 * - X86_INS_FSTENV = X86_INS_WAIT + X86_INS_FNSTENV
 * - X86_INS_FCLEX  = X86_INS_WAIT + X86_INS_FNCLEX
 *
 */

/* X86_INS_FSTENV = X86_INS_WAIT + X86_INS_FNSTENV */
bool translate_fstenv(IR1_INST *pir1)
{
    /* fstenv = fwait + fnstenv */
    if(tr_gen_fp_common_excp_check(pir1)) return true;

    /* 1. fwait */
    tr_gen_fwait();
    /* 2. fnstenv */
    IR1_OPND *dst = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1), ir1_opnd_is_mem(dst),
            "fstenv operand is not mem.\n");

    int dflag_1; // dflag -1
    if(ir1_data_size(pir1)==16)
        dflag_1 = 0;
    else // ir1_data_size == 32
        dflag_1 = 1;

    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    IR2_OPND ptr, ptr_no_offset, ptr_no_offset_ireg;

    convert_mem_opnd(&ptr, dst, -1); // type:mem
    int new_tmp = 0;
    ptr_no_offset = convert_mem_ir2_opnd_no_offset(&ptr, &new_tmp); // type:mem
    ptr_no_offset_ireg = ir2_opnd_mem_get_base(&ptr_no_offset);

    append_ir2_opnd2_(lisa_mov, &arg1_ir2_opnd, &ptr_no_offset_ireg);
    load_imm32_to_ir2(&arg2_ir2_opnd, dflag_1, SIGN_EXTENSION);

    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    tr_gen_call_to_helper((ADDR)helper_fstenv);

    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
    return true;
}

/* X86_INS_FCLEX  = X86_INS_WAIT + X86_INS_FNCLEX */
bool translate_fclex(IR1_INST *pir1)
{
    /* fclex = fwait + fnclex */
    if(tr_gen_fp_common_excp_check(pir1)) return true;
    /* 1. fwait */
    tr_gen_fwait();
    /* 2. fnclex */
    /* env->fpus &= 0x7f00 */
    IR2_OPND sw_opnd = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_LD_HU, &sw_opnd, &env_ir2_opnd,
            lsenv_offset_of_status_word(lsenv));

    IR2_OPND mask = ra_alloc_itemp();
    load_imm32_to_ir2(&mask, 0x7f00, ZERO_EXTENSION);
//    append_ir2_opnd2i(mips_ori, &mask, &zero_ir2_opnd, 0x7f00);
    append_ir2_opnd3(LISA_AND, &sw_opnd, &sw_opnd, &mask);
    append_ir2_opnd2i(LISA_ST_H, &sw_opnd, &env_ir2_opnd,
            lsenv_offset_of_status_word(lsenv));

    return true;
}
