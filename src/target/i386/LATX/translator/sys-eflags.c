#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>

void latxs_sys_eflags_register_ir1(void)
{
    latxs_register_ir1(X86_INS_CLD);
    latxs_register_ir1(X86_INS_STD);
    latxs_register_ir1(X86_INS_CLC);
    latxs_register_ir1(X86_INS_STC);
    latxs_register_ir1(X86_INS_CMC);
    latxs_register_ir1(X86_INS_CLAC);
    latxs_register_ir1(X86_INS_STAC);
    latxs_register_ir1(X86_INS_PUSHF);
    latxs_register_ir1(X86_INS_PUSHFD);
    latxs_register_ir1(X86_INS_POPF);
    latxs_register_ir1(X86_INS_POPFD);
    latxs_register_ir1(X86_INS_PUSHFQ);
    latxs_register_ir1(X86_INS_POPFQ);

    latxs_register_ir1(X86_INS_CLTS);

    latxs_register_ir1(X86_INS_SAHF);
    latxs_register_ir1(X86_INS_LAHF);
}

/*
 * always set CPUX86State.cc_src == eflags
 *
 * Only called in :
 * 1. context switch native to bt
 * 2. call to helper prologue
 */
void latxs_tr_save_eflags(void)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *env = &latxs_env_ir2_opnd;

    /* 1. store OF/SF/ZF/AF/PF/CF in env->cc_src */
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_X86MFFLAG, &temp, 0x3f);
    latxs_append_ir2_opnd2i(LISA_ST_W, &temp, env,
            lsenv_offset_of_cc_src(lsenv));

    /* 2. set env->cc_op = 1 */
    latxs_append_ir2_opnd2i(LISA_ORI, &temp, zero, 1);
    latxs_append_ir2_opnd2i(LISA_ST_W, &temp, env,
            lsenv_offset_of_cc_op(lsenv));
    latxs_ra_free_temp(&temp);
}

/*
 * Only called in :
 * 1. context switch bt to native
 * 2. call to helper epilogue
 */
void latxs_tr_load_eflags(void)
{
    /* Load cc_src(OF/SF/ZF/AF/PF/CF) into temp register */
    IR2_OPND cc_src = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_W, &cc_src, &latxs_env_ir2_opnd,
            lsenv_offset_of_cc_src(lsenv));

    /* Move OF/SF/ZF/AF/PF/CF into CPU */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &cc_src, 0x3f);
    latxs_ra_free_temp(&cc_src);
}

/*
 * Only called in :
 * 1. static context switch prologue
 */
void latxs_tr_gen_static_save_eflags(void)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *env = &latxs_env_ir2_opnd;

    /* 1. store OF/SF/ZF/AF/PF/CF in env->cc_src */
    IR2_OPND temp = latxs_stmp1_ir2_opnd;
    latxs_append_ir2_opnd1i(LISA_X86MFFLAG, &temp, 0x3f);
    latxs_append_ir2_opnd2i(LISA_ST_W, &temp, env,
            lsenv_offset_of_cc_src(lsenv));

    /* 2. set env->cc_op = 1 */
    latxs_append_ir2_opnd2i(LISA_ORI, &temp, zero, 1);
    latxs_append_ir2_opnd2i(LISA_ST_W, &temp, env,
            lsenv_offset_of_cc_op(lsenv));
}

/*
 * Only called in :
 * 1. static context switch epilogue
 */
void latxs_tr_gen_static_load_eflags(int simple)
{
    /* Load cc_src(OF/SF/ZF/AF/PF/CF) into temp register */
    IR2_OPND cc_src = latxs_stmp1_ir2_opnd;
    latxs_append_ir2_opnd2i(LISA_LD_W, &cc_src, &latxs_env_ir2_opnd,
            lsenv_offset_of_cc_src(lsenv));

    /* Move OF/SF/ZF/AF/PF/CF into CPU */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &cc_src, 0x3f);
}

bool latxs_translate_cld(IR1_INST *pir1)
{
    /*
     * df was placed in env->df
     * env->df : if DF: env->df = 1 else env->df = -1
     */
    IR2_OPND tmp = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ORI, &tmp,
            &latxs_zero_ir2_opnd, 1);
    latxs_append_ir2_opnd2i(LISA_ST_W, &tmp,
            &latxs_env_ir2_opnd, lsenv_offset_of_df(lsenv));
    latxs_ra_free_temp(&tmp);

    return true;
}

bool latxs_translate_std(IR1_INST *pir1)
{
    /*
     * df was placed in env->df
     * env->df : if DF: env->df = 1 else env->df = -1
     */
    IR2_OPND tmp = latxs_ra_alloc_itemp();
    latxs_load_imm64_to_ir2(&tmp, -1);
    latxs_append_ir2_opnd2i(LISA_ST_W, &tmp,
            &latxs_env_ir2_opnd, lsenv_offset_of_df(lsenv));
    latxs_ra_free_temp(&tmp);

    return true;
}

bool latxs_translate_clc(IR1_INST *pir1)
{
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &latxs_zero_ir2_opnd, 0x1);
    return true;
}

bool latxs_translate_stc(IR1_INST *pir1)
{
    IR2_OPND cf_opnd = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ORI, &cf_opnd, &latxs_zero_ir2_opnd, 0x1);
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &cf_opnd, 0x1);
    latxs_ra_free_temp(&cf_opnd);
    return true;
}

bool latxs_translate_cmc(IR1_INST *pir1)
{
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_X86MFFLAG, &temp, 0x1);
    latxs_append_ir2_opnd2i(LISA_XORI, &temp, &temp, 0x1);
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &temp, 0x1);
    latxs_ra_free_temp(&temp);
    return true;
}

bool latxs_translate_clac(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_CLAC(pir1);

    IR2_OPND eflags = latxs_ra_alloc_itemp();
    IR2_OPND mask = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_WU, &eflags, &latxs_env_ir2_opnd,
            lsenv_offset_of_eflags(lsenv));

    latxs_load_imm32_to_ir2(&mask, AC_MASK, EXMode_Z);
    latxs_append_ir2_opnd3(LISA_NOR, &mask, &mask, &latxs_zero_ir2_opnd);
    latxs_append_ir2_opnd3(LISA_AND, &eflags, &eflags, &mask);
    latxs_append_ir2_opnd2i(LISA_ST_W, &eflags, &latxs_env_ir2_opnd,
            lsenv_offset_of_eflags(lsenv));
    return true;
}

bool latxs_translate_stac(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_STAC(pir1);

    IR2_OPND eflags = latxs_ra_alloc_itemp();
    IR2_OPND mask = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_WU, &eflags, &latxs_env_ir2_opnd,
            lsenv_offset_of_eflags(lsenv));
    latxs_load_imm32_to_ir2(&mask, AC_MASK, EXMode_Z);
    latxs_append_ir2_opnd3(LISA_OR, &eflags, &eflags, &mask);
    latxs_append_ir2_opnd2i(LISA_ST_W, &eflags, &latxs_env_ir2_opnd,
            lsenv_offset_of_eflags(lsenv));
    return true;
}

/* End of TB in system-mode */
bool latxs_translate_popf(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_POPF(pir1);

    int data_size = latxs_ir1_data_size(pir1);

#ifdef TARGET_X86_64
    if (td->sys.code64) {
        data_size = (data_size == 16) ? 16 : 64;
        lsassert(data_size == 16 || data_size == 64);
    } else {
        lsassert(data_size == 16 || data_size == 32);
    }
#else
    lsassert(data_size == 16 || data_size == 32);
#endif

    /*
     * popf
     * ----------------------
     * > eflags <= MEM(SS:ESP)
     * > ESP    <= ESP + 2/4
     * ----------------------
     * 1. tmp   <= MEM(SS:ESP) : softmmu
     * 2. tmp   => eflags : helper_write_eflags
     * 3. ESP   <= ESP + 2/4
     */

    /* 1.1 build MEM(SS:ESP) */
    IR1_OPND mem_ir1_opnd;
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
            X86_REG_SS, X86_REG_ESP, 0, 0, 0);

    /* 1.2 read data from stack   : might generate exception */
    IR2_OPND tmp = latxs_ra_alloc_itemp();
    int ss_addr_size = latxs_get_sys_stack_addr_size();
    latxs_load_ir1_mem_to_ir2(&tmp,
            &mem_ir1_opnd, EXMode_Z, ss_addr_size);

    /*
     * 2. write into eflags
     *
     * void helper_write_eflags(
     *      CPUX86State *env,
     *      target_ulong t0,
     *      uint32_t update_mask)
     */
    IR2_OPND tmp2 = latxs_ra_alloc_itemp();

    /* 2.0 save native context */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

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
    if (data_size == 16) {
        eflags_mask &= 0xffff;
    }
    latxs_load_imm32_to_ir2(&tmp2, eflags_mask, EXMode_Z);
    /* 2.2 arg1: data to write */
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &tmp);
    /* 2.3 arg2: eflags mask */
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg2_ir2_opnd, &tmp2);
    /* 2.4 arg0: env */
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                      &latxs_env_ir2_opnd);
    /* 2.5 call helper_write_eflags : might generate exception */
    latxs_tr_gen_call_to_helper((ADDR)helper_write_eflags);
    /* 2.6 restore context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* 3. update ESP */
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        latxs_append_ir2_opnd2i(LISA_ADDI_D,
                &esp_opnd, &esp_opnd, (data_size >> 3));
    } else
#endif
    if (lsenv->tr_data->sys.ss32) {
        latxs_append_ir2_opnd2i(LISA_ADDI_W,
                &esp_opnd, &esp_opnd, (data_size >> 3));
#ifdef TARGET_X86_64
        latxs_append_ir2_opnd2_(lisa_mov32z, &esp_opnd, &esp_opnd);
#else
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
        }
#endif
    } else {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ADDI_D,
                &tmp, &esp_opnd, (data_size >> 3));
        latxs_store_ir2_to_ir1(&tmp, &sp_ir1_opnd);
        latxs_ra_free_temp(&tmp);
    }

    return true;
}

bool latxs_translate_pushf(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_PUSHF(pir1);

    int data_size = latxs_ir1_data_size(pir1);

#ifdef TARGET_X86_64
    if (td->sys.code64) {
        data_size = (data_size == 16) ? 16 : 64;
        lsassert(data_size == 16 || data_size == 64);
    } else {
        lsassert(data_size == 16 || data_size == 32);
    }
#else
    lsassert(data_size == 16 || data_size == 32);
#endif

    /*
     * pushf
     * ----------------------
     * >  ESP    <= ESP - 2/4
     * >  eflags => MEM(SS:ESP)
     * ----------------------
     * 1. eflags <= eflags : mapping register
     * 2. tmp    => MEM(SS:ESP - 2/4) : softmmu
     * 3. ESP    <= ESP - 2/4
     */

    /*
     * 1. get the eflags
     *    env->cc_src : OF/SF/ZF/AF/PF/CF (mapped to la flag register)
     *    env->df     : if DF: env->df = 1 else env->df = -1
     *    env->eflags : every thing else except DF and OF/SF/ZF/AF/PF/CF
     */
    IR2_OPND ls_eflags = latxs_ra_alloc_itemp();
    /* 1.1 get OF/SF/ZF/AF/PF/CF */
    latxs_append_ir2_opnd1i(LISA_X86MFFLAG, &ls_eflags,  0x3f);
    /* 1.2 get env->eflags */
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_WU, &temp, &latxs_env_ir2_opnd,
            lsenv_offset_of_eflags(lsenv));
    IR2_OPND mask = latxs_ra_alloc_itemp();
    /* clear df and oszapc flags, just avoid potential risk */
    latxs_append_ir2_opnd2i(LISA_ORI, &mask, &latxs_zero_ir2_opnd, 0xcd5);
    latxs_append_ir2_opnd3(LISA_NOR, &mask, &latxs_zero_ir2_opnd, &mask);
    latxs_append_ir2_opnd3(LISA_AND, &temp, &temp, &mask);
    latxs_ra_free_temp(&mask);
    latxs_append_ir2_opnd3(LISA_OR, &ls_eflags, &temp, &ls_eflags);
    /* 1.2 get DF */
    latxs_append_ir2_opnd2i(LISA_LD_WU, &temp, &latxs_env_ir2_opnd,
            lsenv_offset_of_df(lsenv));
    latxs_append_ir2_opnd2i(LISA_ANDI, &temp, &temp, 0x400);
    latxs_append_ir2_opnd3(LISA_OR, &ls_eflags, &temp, &ls_eflags);


    /* 2.1 build MEM(SS:ESP - 2/4) */
    IR1_OPND mem_ir1_opnd;
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
            X86_REG_SS, X86_REG_ESP, 0 - (data_size >> 3), 0, 0);

    /*
     * 2.2 write eflags into stack : might generate exception
     *     pushf will not write VM(17) and RF(16)
     */
    IR2_OPND eflags_to_push = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ORI, &eflags_to_push,
                                      &latxs_zero_ir2_opnd,  0x3);
    latxs_append_ir2_opnd2i(LISA_SLLI_D, &eflags_to_push,
                                         &eflags_to_push, 0x10);
    latxs_append_ir2_opnd2_(lisa_not, &eflags_to_push, &eflags_to_push);

    latxs_append_ir2_opnd3(LISA_AND,
            &eflags_to_push, &ls_eflags, &eflags_to_push);

    int ss_addr_size = latxs_get_sys_stack_addr_size();
    latxs_store_ir2_to_ir1_mem(&eflags_to_push,
            &mem_ir1_opnd, ss_addr_size);
    latxs_ra_free_temp(&eflags_to_push);

    /* 3. update ESP */
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        latxs_append_ir2_opnd2i(LISA_ADDI_D,
                &esp_opnd, &esp_opnd, 0 - (data_size >> 3));
    } else
#endif
    if (lsenv->tr_data->sys.ss32) {
        latxs_append_ir2_opnd2i(LISA_ADDI_W,
                &esp_opnd, &esp_opnd, 0 - (data_size >> 3));
#ifdef TARGET_X86_64
        latxs_append_ir2_opnd2_(lisa_mov32z, &esp_opnd, &esp_opnd);
#else
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
        }
#endif
    } else {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ADDI_D,
                &tmp, &esp_opnd, 0 - (data_size >> 3));
        latxs_store_ir2_to_ir1(&tmp, &sp_ir1_opnd);
        latxs_ra_free_temp(&tmp);
    }

    return true;
}

/* End of TB in system-mode */
bool latxs_translate_clts(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_CLTS(pir1);

    /* void helper_clts(CPUX86State  *env) */

    IR2_OPND cr0 = latxs_ra_alloc_itemp();
    IR2_OPND hf = latxs_ra_alloc_itemp();

    int off_cr0 = lsenv_offset_of_cr(lsenv, 0);
    int off_hf  = lsenv_offset_of_hflags(lsenv);

    IR2_OPND *env = &latxs_env_ir2_opnd;
    IR2_OPND *zero = &latxs_zero_ir2_opnd;

    latxs_append_ir2_opnd2i(LISA_LD_WU, &cr0, env, off_cr0);
    latxs_append_ir2_opnd2i(LISA_LD_WU, &hf, env, off_hf);

    IR2_OPND mask = latxs_ra_alloc_itemp();

    /*env->cr[0] &= ~CR0_TS_MASK;*/
    latxs_append_ir2_opnd2i(LISA_ORI, &mask, zero, CR0_TS_MASK);
    latxs_append_ir2_opnd3(LISA_NOR, &mask, zero, &mask);
    latxs_append_ir2_opnd3(LISA_AND, &cr0, &cr0, &mask);

    /*env->hflags &= ~HF_TS_MASK;*/
    latxs_append_ir2_opnd2i(LISA_ORI, &mask, zero, HF_TS_MASK);
    latxs_append_ir2_opnd3(LISA_NOR, &mask, zero, &mask);
    latxs_append_ir2_opnd3(LISA_AND, &hf, &hf, &mask);

    latxs_append_ir2_opnd2i(LISA_ST_W, &cr0, env, off_cr0);
    latxs_append_ir2_opnd2i(LISA_ST_W, &hf, env, off_hf);

    return true;
}

bool latxs_translate_sahf(IR1_INST *pir1)
{
    IR2_OPND ah = latxs_ra_alloc_itemp();
    latxs_load_ir1_gpr_to_ir2(&ah, &ah_ir1_opnd, EXMode_Z);
    latxs_append_ir2_opnd2i(LISA_ORI, &ah, &ah, 0x2);
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &ah, 0x1f);
    latxs_ra_free_temp(&ah);
    return true;
}

bool latxs_translate_lahf(IR1_INST *pir1)
{
    IR2_OPND ah = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_X86MFFLAG, &ah, 0x1f);
    latxs_append_ir2_opnd2i(LISA_ORI, &ah, &ah, 0x2);
    latxs_store_ir2_to_ir1_gpr(&ah, &ah_ir1_opnd);
    latxs_ra_free_temp(&ah);
    return true;
}
