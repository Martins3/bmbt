#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "flag-lbt.h"
#include "sys-excp.h"
#include <string.h>

void latxs_sys_branch_register_ir1(void)
{
    latxs_register_ir1(X86_INS_JE);
    latxs_register_ir1(X86_INS_JNE);
    latxs_register_ir1(X86_INS_JS);
    latxs_register_ir1(X86_INS_JNS);
    latxs_register_ir1(X86_INS_JB);
    latxs_register_ir1(X86_INS_JAE);
    latxs_register_ir1(X86_INS_JO);
    latxs_register_ir1(X86_INS_JNO);
    latxs_register_ir1(X86_INS_JBE);
    latxs_register_ir1(X86_INS_JA);
    latxs_register_ir1(X86_INS_JP);
    latxs_register_ir1(X86_INS_JNP);
    latxs_register_ir1(X86_INS_JL);
    latxs_register_ir1(X86_INS_JGE);
    latxs_register_ir1(X86_INS_JLE);
    latxs_register_ir1(X86_INS_JG);
    latxs_register_ir1(X86_INS_JCXZ);
    latxs_register_ir1(X86_INS_JECXZ);
    latxs_register_ir1(X86_INS_JRCXZ);

    latxs_register_ir1(X86_INS_LOOP);
    latxs_register_ir1(X86_INS_LOOPE);
    latxs_register_ir1(X86_INS_LOOPNE);
}

#ifdef CONFIG_SOFTMMU

#define LOAD_TB_PTR_TO_DBT_ARG1 do {                        \
    if (!option_lsfpu && !option_soft_fpu) {                \
        IR2_OPND tb_ptr_opnd = latxs_ra_alloc_dbt_arg1();   \
        latxs_tr_gen_exit_tb_load_tb_addr(&tb_ptr_opnd,     \
                (ADDR)lsenv->tr_data->curr_tb);             \
    }                                                       \
} while (0)

#define GEN_EOB latxs_tr_gen_eob()

#else

#define LOAD_TB_PTR_TO_DBT_ARG1
#define GEN_EOB

#endif

bool latxs_translate_jz(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND cond = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_SETX86J, &cond, COND_E);

    latxs_append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
    latxs_ra_free_temp(&cond);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);

    return true;
}

bool latxs_translate_jnz(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND cond = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_SETX86J, &cond, COND_NE);

    latxs_append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
    latxs_ra_free_temp(&cond);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_js(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND cond = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_SETX86J, &cond, COND_S);

    latxs_append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
    latxs_ra_free_temp(&cond);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_jns(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND cond = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_SETX86J, &cond, COND_NS);

    latxs_append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
    latxs_ra_free_temp(&cond);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_jb(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND cond = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_SETX86J, &cond, COND_B);

    latxs_append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
    latxs_ra_free_temp(&cond);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_jae(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND cond = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_SETX86J, &cond, COND_AE);

    latxs_append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
    latxs_ra_free_temp(&cond);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_jo(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND cond = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_SETX86J, &cond, COND_O);

    latxs_append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
    latxs_ra_free_temp(&cond);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_jno(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND cond = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_SETX86J, &cond, COND_NO);

    latxs_append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
    latxs_ra_free_temp(&cond);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_jbe(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND cond = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_SETX86J, &cond, COND_BE);

    latxs_append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
    latxs_ra_free_temp(&cond);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_ja(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND cond = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_SETX86J, &cond, COND_A);

    latxs_append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
    latxs_ra_free_temp(&cond);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_jp(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND cond = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_SETX86J, &cond, COND_PE);

    latxs_append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
    latxs_ra_free_temp(&cond);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_jnp(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND cond = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_SETX86J, &cond, COND_PO);

    latxs_append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
    latxs_ra_free_temp(&cond);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_jl(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND cond = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_SETX86J, &cond, COND_L);

    latxs_append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
    latxs_ra_free_temp(&cond);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_jge(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND cond = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_SETX86J, &cond, COND_GE);

    latxs_append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
    latxs_ra_free_temp(&cond);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_jle(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND cond = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_SETX86J, &cond, COND_LE);

    latxs_append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
    latxs_ra_free_temp(&cond);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_jg(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND cond = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_SETX86J, &cond, COND_G);

    latxs_append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
    latxs_ra_free_temp(&cond);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_jcxz(IR1_INST *pir1)
{
    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND cx_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&cx_opnd, &cx_ir1_opnd, EXMode_Z);

    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd2(LISA_BEQZ, &cx_opnd, &target_label_opnd);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_jecxz(IR1_INST *pir1)
{
    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND ecx_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&ecx_opnd, &ecx_ir1_opnd, EXMode_Z);

    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd2(LISA_BEQZ, &ecx_opnd, &target_label_opnd);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_jrcxz(IR1_INST *pir1)
{
    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND rcx_opnd = latxs_ra_alloc_gpr(ecx_index);

    IR2_OPND target_label_opnd = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd2(LISA_BEQZ, &rcx_opnd, &target_label_opnd);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_loopnz(IR1_INST *pir1)
{
    IR2_OPND ir2_xcx;
    int ir2_xcx_tmp = 0;

    switch (latxs_ir1_addr_size(pir1)) {
    case 2:
        ir2_xcx = latxs_ra_alloc_itemp();
        ir2_xcx_tmp = 1;
        latxs_load_ir1_to_ir2(&ir2_xcx, &cx_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &ir2_xcx, &ir2_xcx, -1);
        latxs_store_ir2_to_ir1(&ir2_xcx, &cx_ir1_opnd);
        break;
    case 4:
        ir2_xcx = latxs_ra_alloc_itemp();
        ir2_xcx_tmp = 1;
        latxs_load_ir1_to_ir2(&ir2_xcx, &ecx_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &ir2_xcx, &ir2_xcx, -1);
        latxs_store_ir2_to_ir1(&ir2_xcx, &ecx_ir1_opnd);
        break;
#ifdef TARGET_X86_64
    case 8:
        ir2_xcx = latxs_ra_alloc_gpr(ecx_index);
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &ir2_xcx, &ir2_xcx, -1);
        break;
#endif
    default:
        lsassert(0);
        break;
    }

    IR2_OPND target_label = latxs_ir2_opnd_new_label();

    IR2_OPND temp_zf = latxs_ra_alloc_itemp();
    latxs_get_eflag_condition(&temp_zf, pir1);

    /* By now, if zf =0, then temp_zf =0 else temp_zf = 0xFFFFFFFFFFFFFFFF */
    latxs_append_ir2_opnd3(LISA_NOR, &temp_zf, &temp_zf, &latxs_zero_ir2_opnd);
    latxs_append_ir2_opnd3(LISA_AND, &temp_zf, &temp_zf, &ir2_xcx);

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    latxs_append_ir2_opnd2(LISA_BNEZ, &temp_zf, &target_label);
    latxs_ra_free_temp(&temp_zf);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label);
    latxs_tr_generate_exit_tb(pir1, 1);

    if (ir2_xcx_tmp) {
        latxs_ra_free_temp(&ir2_xcx);
    }

    return true;
}

bool latxs_translate_loopz(IR1_INST *pir1)
{
    IR2_OPND ir2_xcx;
    int ir2_xcx_tmp = 0;

    switch (latxs_ir1_addr_size(pir1)) {
    case 2:
        ir2_xcx = latxs_ra_alloc_itemp();
        ir2_xcx_tmp = 1;
        latxs_load_ir1_to_ir2(&ir2_xcx, &cx_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &ir2_xcx, &ir2_xcx, -1);
        latxs_store_ir2_to_ir1(&ir2_xcx, &cx_ir1_opnd);
        break;
    case 4:
        ir2_xcx = latxs_ra_alloc_itemp();
        ir2_xcx_tmp = 1;
        latxs_load_ir1_to_ir2(&ir2_xcx, &ecx_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &ir2_xcx, &ir2_xcx, -1);
        latxs_store_ir2_to_ir1(&ir2_xcx, &ecx_ir1_opnd);
        break;
#ifdef TARGET_X86_64
    case 8:
        ir2_xcx = latxs_ra_alloc_gpr(ecx_index);
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &ir2_xcx, &ir2_xcx, -1);
        break;
#endif
    default:
        lsassert(0);
        break;
    }

    IR2_OPND target_label = latxs_ir2_opnd_new_label();

    IR2_OPND temp_zf = latxs_ra_alloc_itemp();
    latxs_get_eflag_condition(&temp_zf, pir1);

    /* By now, if zf =0, then temp_zf =0 else temp_zf = 0xFFFFFFFFFFFFFFFF */
    latxs_append_ir2_opnd3(LISA_AND, &temp_zf, &temp_zf, &ir2_xcx);

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    latxs_append_ir2_opnd2(LISA_BNEZ, &temp_zf, &target_label);
    latxs_ra_free_temp(&temp_zf);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label);
    latxs_tr_generate_exit_tb(pir1, 1);

    if (ir2_xcx_tmp) {
        latxs_ra_free_temp(&ir2_xcx);
    }

    return true;
}

bool latxs_translate_loop(IR1_INST *pir1)
{
    IR2_OPND ir2_xcx;
    int ir2_xcx_tmp = 0;

    switch (latxs_ir1_addr_size(pir1)) {
    case 2:
        ir2_xcx = latxs_ra_alloc_itemp();
        ir2_xcx_tmp = 1;
        latxs_load_ir1_to_ir2(&ir2_xcx, &cx_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &ir2_xcx, &ir2_xcx, -1);
        latxs_store_ir2_to_ir1(&ir2_xcx, &cx_ir1_opnd);
        break;
    case 4:
        ir2_xcx = latxs_ra_alloc_itemp();
        ir2_xcx_tmp = 1;
        latxs_load_ir1_to_ir2(&ir2_xcx, &ecx_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &ir2_xcx, &ir2_xcx, -1);
        latxs_store_ir2_to_ir1(&ir2_xcx, &ecx_ir1_opnd);
        break;
#ifdef TARGET_X86_64
    case 8:
        ir2_xcx = latxs_ra_alloc_gpr(ecx_index);
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &ir2_xcx, &ir2_xcx, -1);
        break;
#endif
    default:
        lsassert(0);
        break;
    }

    IR2_OPND target_label = latxs_ir2_opnd_new_label();

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    latxs_append_ir2_opnd2(LISA_BNEZ, &ir2_xcx, &target_label);

    latxs_tr_generate_exit_tb(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label);
    latxs_tr_generate_exit_tb(pir1, 1);

    if (ir2_xcx_tmp) {
        latxs_ra_free_temp(&ir2_xcx);
    }

    return true;
}
