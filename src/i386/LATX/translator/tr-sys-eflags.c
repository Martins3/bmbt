/* X86toMIPS headers */
#include "../include/common.h"
#include "../include/reg-alloc.h"
#include "../include/env.h"
#include "../ir2/ir2.h"
#include "../x86tomips-options.h"

/* QEMU headers */
#include "../include/qemu-def.h"

/* Macro : Exception Check for specific instructions */
#include "../translator/tr-excp.h"

/* End of TB in system-mode */
bool translate_popf(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_POPF(pir1);

    int data_size = ir1_data_size(pir1);
    lsassert(data_size == 16 || data_size == 32);

    /* popf
     * ----------------------
     * > eflags <= MEM(SS:ESP)
     * > ESP    <= ESP + 2/4
     * ----------------------
     * 1. tmp   <= MEM(SS:ESP) : softmmu
     * 2. tmp   => eflags : helper_write_eflags
     * 3. ESP   <= ESP + 2/4                    */
    /* 1.1 build MEM(SS:ESP) */
    IR1_OPND mem_ir1_opnd;
    ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
            X86_REG_SS, X86_REG_ESP, 0, 0, 0);

    /* 1.2 read data from stack   : might generate exception */
    IR2_OPND tmp = ra_alloc_itemp();
    int ss_addr_size = get_sys_stack_addr_size();
    load_ir1_mem_to_ir2(&tmp, &mem_ir1_opnd, ZERO_EXTENSION, false, ss_addr_size);

    /* 2. write into eflags
     *
     * void helper_write_eflags(
     *      CPUX86State *env,
     *      target_ulong t0,
     *      uint32_t update_mask)   */
    IR2_OPND tmp2 = ra_alloc_itemp();

    /* 2.0 save native context */
    int use_fp = 0;
    tr_gen_call_to_helper_prologue(use_fp);
    /* 2.1 tmp2: mask for eflags */
    uint32 eflags_mask = 0;
    if (td->sys.cpl == 0) {
        eflags_mask = TF_MASK | AC_MASK | ID_MASK | NT_MASK |
                      IF_MASK | IOPL_MASK;
    } else {
        if (td->sys.cpl <= td->sys.iopl) {
            eflags_mask = TF_MASK | AC_MASK | ID_MASK | NT_MASK |
                          IF_MASK;
        } else {
            eflags_mask = TF_MASK | AC_MASK | ID_MASK | NT_MASK;
        }
    }
    if (data_size == 16) eflags_mask &= 0xffff;
    load_imm32_to_ir2(&tmp2, eflags_mask, ZERO_EXTENSION);
    /* 2.2 arg1: data to write */
    append_ir2_opnd2_(lisa_mov, &arg1_ir2_opnd, &tmp);
    /* 2.3 arg2: eflags mask */
    append_ir2_opnd2_(lisa_mov, &arg2_ir2_opnd, &tmp2);
    /* 2.4 arg0: env */
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    /* 2.5 call helper_write_eflags : might generate exception */
    tr_gen_call_to_helper((ADDR)helper_write_eflags);
    /* 2.6 restore context */
    tr_gen_call_to_helper_epilogue(use_fp);

    /* 3. update ESP */
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    if (lsenv->tr_data->sys.ss32) {
        append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd, (data_size >> 3));
    } else {
        IR2_OPND tmp = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd, (data_size >> 3));
        store_ir2_to_ir1(&tmp, &sp_ir1_opnd, false);
        ra_free_temp(&tmp);
    }

    return true;
}

bool translate_pushf(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_PUSHF(pir1);

    int data_size = ir1_data_size(pir1);
    lsassert(data_size == 16 || data_size == 32);

    /* pushf
     * ----------------------
     * >  ESP    <= ESP - 2/4
     * >  eflags => MEM(SS:ESP)
     * ----------------------
     * 1. eflags <= eflags : mapping register
     * 2. tmp    => MEM(SS:ESP - 2/4) : softmmu
     * 3. ESP    <= ESP - 2/4                    */
    /* 1. get the eflags
     *    since we sync the eflags in context switch and
     *    x86_to_mips_before_exec_tb(), the eflags mapping
     *    registers always contains the up-to-date eflags */
    IR2_OPND *eflags = &eflags_ir2_opnd;
    IR2_OPND ls_eflags;
    if (option_lbt) {
        ls_eflags = ra_alloc_itemp();
        append_ir2_opnd2_(lisa_mov,       &ls_eflags, &zero_ir2_opnd);
        append_ir2_opnd1i(LISA_X86MFFLAG, &ls_eflags,  0x3f);
        append_ir2_opnd3 (LISA_OR,        &ls_eflags, eflags, &ls_eflags);
    }

    /* 2.1 build MEM(SS:ESP - 2/4) */
    IR1_OPND mem_ir1_opnd;
    ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
            X86_REG_SS, X86_REG_ESP, 0 - (data_size >> 3), 0, 0);
    /* 2.2 write eflags into stack : might generate exception
     *     pushf will not write VM(17) and RF(16) */
    IR2_OPND eflags_to_push = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ORI,    &eflags_to_push, &zero_ir2_opnd,  0x3);
    append_ir2_opnd2i(LISA_SLLI_D, &eflags_to_push, &eflags_to_push, 0x10);
    append_ir2_opnd2_(lisa_not, &eflags_to_push, &eflags_to_push);
    if (option_lbt) {
        append_ir2_opnd3(LISA_AND, &eflags_to_push, &ls_eflags, &eflags_to_push);
    } else {
        append_ir2_opnd3(LISA_AND, &eflags_to_push, eflags, &eflags_to_push);
    }
    int ss_addr_size = get_sys_stack_addr_size();
    store_ir2_to_ir1_mem(&eflags_to_push, &mem_ir1_opnd, false, ss_addr_size);
    ra_free_temp(&eflags_to_push);

    /* 3. update ESP */
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    if (lsenv->tr_data->sys.ss32){
        append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd, 0 - (data_size >> 3));
    } else {
        IR2_OPND tmp = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd, 0 - (data_size >> 3));
        store_ir2_to_ir1(&tmp, &sp_ir1_opnd, false);
        ra_free_temp(&tmp);
    }

    return true;
}

bool translate_clac(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_CLAC(pir1);

    IR2_OPND mask = ra_alloc_itemp();
    load_imm32_to_ir2(&mask, AC_MASK, ZERO_EXTENSION);
    append_ir2_opnd3(LISA_NOR, &mask, &mask, &zero_ir2_opnd);
    append_ir2_opnd3(LISA_AND, &eflags_ir2_opnd, &eflags_ir2_opnd, &mask);
    return true;
}

bool translate_stac(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_STAC(pir1);

    IR2_OPND mask = ra_alloc_itemp();
    load_imm32_to_ir2(&mask, AC_MASK, ZERO_EXTENSION);
    append_ir2_opnd3(LISA_OR, &eflags_ir2_opnd, &eflags_ir2_opnd, &mask);
    return true;
}

/* always set as cc_src == eflags
 *
 * Only called in :
 * 1. context switch native to bt
 * 2. call to helper prologue */
void tr_save_eflags(void)
{
    /* 1. get eflags */
    if (option_lbt) {
        /* Clear the OF/SF/ZF/AF/PF/CF of eflags */
        IR2_OPND cc_mask = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ORI, &cc_mask,     &zero_ir2_opnd,  0x8d5);
        append_ir2_opnd2_(lisa_not, &cc_mask,     &cc_mask);
        append_ir2_opnd3 (LISA_AND, &eflags_ir2_opnd, &eflags_ir2_opnd,   &cc_mask);
        ra_free_temp(&cc_mask);
        /* Read  OF/SF/ZF/AF/PF/CF from CPU */
        IR2_OPND eflags_temp = ra_alloc_itemp();
        append_ir2_opnd2_(lisa_mov,  &eflags_temp, &zero_ir2_opnd);
        append_ir2_opnd1i(LISA_X86MFFLAG, &eflags_temp,  0x3f);
        /* Set the OF/SF/ZF/AF/PF/CF of eflags */
        append_ir2_opnd3 (LISA_OR, &eflags_ir2_opnd, &eflags_ir2_opnd, &eflags_temp);
        ra_free_temp(&eflags_temp);
    }
    /* 2. save eflags into env
     *    env->cc_src : OF/SF/ZF/AF/PF/CF
     *    env->eflags : every thing else except DF
     *    env->df     : if DF: env->df = 1 else env->df = -1
     *    env->cc_op  : CC_OP_EFLAGS = 1  */
    IR2_OPND cc = ra_alloc_itemp();
    IR2_OPND cc_mask = ra_alloc_itemp();
    /* 2.1 build mask for OF/SF/ZF/AF/PF/CF */
    append_ir2_opnd2i(LISA_ORI, &cc_mask, &zero_ir2_opnd, 0x8d5);
    /* 2.2 get them */
    append_ir2_opnd3(LISA_AND, &cc, &eflags_ir2_opnd, &cc_mask);
    /* 2.3 store them into cc_src */
    append_ir2_opnd2i(LISA_ST_W, &cc, &env_ir2_opnd,
                      lsenv_offset_of_cc_src(lsenv));
    /* 2.4 get other flags : reuse cc_mask */
    append_ir2_opnd2i(LISA_ORI, &cc_mask, &cc_mask, DF_BIT);
    append_ir2_opnd3(LISA_NOR, &cc_mask, &zero_ir2_opnd, &cc_mask);
    append_ir2_opnd3(LISA_AND, &cc_mask, &eflags_ir2_opnd, &cc_mask);
    /* 2.5 store into eflags */
    append_ir2_opnd2i(LISA_ST_W, &cc_mask, &env_ir2_opnd,
                      lsenv_offset_of_eflags(lsenv));
    /* 2.6 set cc_op : reuse cc_mask */
    append_ir2_opnd2i(LISA_ORI, &cc_mask, &zero_ir2_opnd, 1);
    append_ir2_opnd2i(LISA_ST_W, &cc_mask, &env_ir2_opnd,
                      lsenv_offset_of_cc_op(lsenv));
    /* 2.7 set env->df according to eflags: reuse cc and cc_mask */
    append_ir2_opnd2i(LISA_ANDI, &cc_mask, &eflags_ir2_opnd, DF_BIT);
    IR2_OPND df_label = ir2_opnd_new_label();
    IR2_OPND exit_label = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BEQ, &cc_mask, &zero_ir2_opnd, &df_label);
    /* not branch: df = 1 , set env->df = -1 */
    append_ir2_opnd2i(LISA_ADDI_D, &cc, &zero_ir2_opnd, 1);
    append_ir2_opnd1(LISA_B, &exit_label);
    /* branch: df = 0 , set env->df = 1 */
    append_ir2_opnd1(LISA_LABEL, &df_label);
    append_ir2_opnd2i(LISA_ORI, &cc, &zero_ir2_opnd, 1);
    /* update env->df */
    append_ir2_opnd1(LISA_LABEL, &exit_label);
    append_ir2_opnd2i(LISA_ST_W, &cc, &env_ir2_opnd,
            lsenv_offset_of_df(lsenv));

    ra_free_temp(&cc);
    ra_free_temp(&cc_mask);
}

/* if (simple) : simply read the env->eflags
 * else        : read through helper_read_eflags
 *
 * Only called in :
 * 1. context switch bt to native : simple = 1
 * 2. call to helper epilogue     : simple = 0 */
void tr_load_eflags(int simple)
{
    if (simple) {
        append_ir2_opnd2i(LISA_LD_W, &eflags_ir2_opnd, &env_ir2_opnd,
                          lsenv_offset_of_eflags(lsenv));
    }
    else {
        /* 1. save return value */
        append_ir2_opnd2i(LISA_ST_D, &ret0_ir2_opnd, &env_ir2_opnd,
                          lsenv_offset_of_mips_iregs(lsenv, ir2_opnd_reg(&ret0_ir2_opnd)));
        /* 2. read eflags via helper_read_eflags */
        append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
        tr_gen_call_to_helper((ADDR)helper_read_eflags);
        append_ir2_opnd2_(lisa_mov, &eflags_ir2_opnd, &ret0_ir2_opnd);
        /* 3. restore return value */
        append_ir2_opnd2i(LISA_LD_D, &ret0_ir2_opnd, &env_ir2_opnd,
                          lsenv_offset_of_mips_iregs(lsenv, ir2_opnd_reg(&ret0_ir2_opnd)));
    }
    if (option_lbt) {
        /* Move  OF/SF/ZF/AF/PF/CF into CPU */
        append_ir2_opnd1i(LISA_X86MTFLAG, &eflags_ir2_opnd, 0x3f);
        /* Clear OF/SF/ZF/AF/PF/CF of elfgas */
        IR2_OPND cc_mask = ra_alloc_itemp();
        load_imm32_to_ir2(&cc_mask, 0x8d5, ZERO_EXTENSION);
        append_ir2_opnd2_(lisa_not, &cc_mask, &cc_mask);
        append_ir2_opnd3(LISA_AND, &eflags_ir2_opnd, &eflags_ir2_opnd, &cc_mask);
        ra_free_temp(&cc_mask);
    }
}

void tr_gen_static_save_eflags(void)
{
    IR2_OPND *stmp1 = &stmp1_ir2_opnd;
    IR2_OPND *stmp2 = &stmp2_ir2_opnd;
    /* 1. get eflags */
    if (option_lbt) {
        /* Clear the OF/SF/ZF/AF/PF/CF of eflags */
        append_ir2_opnd2i(LISA_ORI, stmp1, &zero_ir2_opnd, 0x8d5);
        append_ir2_opnd3 (LISA_NOR, stmp1, &zero_ir2_opnd, stmp1);
        append_ir2_opnd3 (LISA_AND, &eflags_ir2_opnd, &eflags_ir2_opnd, stmp1);
        /* Read  OF/SF/ZF/AF/PF/CF from CPU */
        append_ir2_opnd2_(lisa_mov,  stmp1, &zero_ir2_opnd);
        append_ir2_opnd1i(LISA_X86MFFLAG, stmp1,  0x3f);
        /* Set the OF/SF/ZF/AF/PF/CF of eflags */
        append_ir2_opnd3 (LISA_OR, &eflags_ir2_opnd, &eflags_ir2_opnd, stmp1);
    }
    /* 2. save eflags into env
     *    env->cc_src : OF/SF/ZF/AF/PF/CF
     *    env->eflags : every thing else except DF
     *    env->df     : if DF: env->df = 1 else env->df = -1
     *    env->cc_op  : CC_OP_EFLAGS = 1  */
    IR2_OPND *cc = stmp1;
    IR2_OPND *cc_mask = stmp2;
    /* 2.1 build mask for OF/SF/ZF/AF/PF/CF */
    append_ir2_opnd2i(LISA_ORI, cc_mask, &zero_ir2_opnd, 0x8d5);
    /* 2.2 get them */
    append_ir2_opnd3(LISA_AND, cc, &eflags_ir2_opnd, cc_mask);
    /* 2.3 store them into cc_src */
    append_ir2_opnd2i(LISA_ST_W, cc, &env_ir2_opnd,
                      lsenv_offset_of_cc_src(lsenv));
    /* 2.4 get other flags : reuse cc_mask */
    append_ir2_opnd2i(LISA_ORI, cc_mask, cc_mask, DF_BIT);
    append_ir2_opnd3 (LISA_NOR, cc_mask, &zero_ir2_opnd, cc_mask);
    append_ir2_opnd3 (LISA_AND, cc_mask, &eflags_ir2_opnd, cc_mask);
    /* 2.5 store into eflags */
    append_ir2_opnd2i(LISA_ST_W, cc_mask, &env_ir2_opnd,
                      lsenv_offset_of_eflags(lsenv));
    /* 2.6 set cc_op : reuse cc_mask */
    append_ir2_opnd2i(LISA_ORI, cc_mask, &zero_ir2_opnd, 1);
    append_ir2_opnd2i(LISA_ST_W, cc_mask, &env_ir2_opnd,
                      lsenv_offset_of_cc_op(lsenv));
    /* 2.7 set env->df according to eflags: reuse cc and cc_mask */
    append_ir2_opnd2i(LISA_ANDI, cc_mask, &eflags_ir2_opnd, DF_BIT);
    IR2_OPND df_label = ir2_opnd_new_label();
    IR2_OPND exit_label = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BEQ, cc_mask, &zero_ir2_opnd, &df_label);
    /* not branch: df = 1 , set env->df = -1 */
    append_ir2_opnd2i(LISA_ADDI_D, cc, &zero_ir2_opnd, -1);
    append_ir2_opnd1 (LISA_B, &exit_label);
    /* branch: df = 0 , set env->df = 1 */
    append_ir2_opnd1 (LISA_LABEL, &df_label);
    append_ir2_opnd2i(LISA_ORI, cc, &zero_ir2_opnd, 1);
    /* update env->df */
    append_ir2_opnd1(LISA_LABEL, &exit_label);
    append_ir2_opnd2i(LISA_ST_W, cc, &env_ir2_opnd,
            lsenv_offset_of_df(lsenv));
}

void tr_gen_static_load_eflags(int simple)
{
    if (simple) {
        append_ir2_opnd2i(LISA_LD_W, &eflags_ir2_opnd,
                &env_ir2_opnd, lsenv_offset_of_eflags(lsenv));
    }
    else {
        int ret_reg = ir2_opnd_reg(&ret0_ir2_opnd);
        /* 1. save return value */
        append_ir2_opnd2i(LISA_ST_D, &ret0_ir2_opnd,
                &env_ir2_opnd,
                lsenv_offset_of_mips_iregs(lsenv, ret_reg));
        /* 2. read eflags via helper_read_eflags */
        append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
//        tr_gen_call_to_helper((ADDR)helper_read_eflags);
        load_addr_to_ir2(&stmp1_ir2_opnd, (ADDR)helper_read_eflags);
        append_ir2_opnd1_(lisa_call, &stmp1_ir2_opnd);
        append_ir2_opnd2_(lisa_mov, &eflags_ir2_opnd, &ret0_ir2_opnd);
        /* 3. restore return value */
        append_ir2_opnd2i(LISA_LD_D, &ret0_ir2_opnd,
                &env_ir2_opnd,
                lsenv_offset_of_mips_iregs(lsenv, ret_reg));
    }
    if (option_lbt) {
        /* Move  OF/SF/ZF/AF/PF/CF into CPU */
        append_ir2_opnd1i(LISA_X86MTFLAG, &eflags_ir2_opnd, 0x3f);
        /* Clear OF/SF/ZF/AF/PF/CF of elfgas */
        IR2_OPND *cc_mask = &stmp1_ir2_opnd;
        append_ir2_opnd2i(LISA_ORI, cc_mask, &zero_ir2_opnd, 0x8d5);
        append_ir2_opnd3 (LISA_NOR, cc_mask, &zero_ir2_opnd, cc_mask);
        append_ir2_opnd3 (LISA_AND, &eflags_ir2_opnd, &eflags_ir2_opnd, cc_mask);
    }
}
