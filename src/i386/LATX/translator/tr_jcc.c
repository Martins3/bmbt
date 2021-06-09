#include "../include/common.h"
#include "../include/reg_alloc.h"
#include "../include/env.h"
#include "../x86tomips-options.h"

#ifndef CONFIG_SOFTMMU
/* Apply flag pattern only in user-mode */
#include "../include/flag_pattern.h"
#endif

extern void get_eflag_condition(IR2_OPND *value, IR1_INST *pir1);

#ifdef CONFIG_SOFTMMU

#ifdef CONFIG_XTM_FAST_CS
/* SOFTMMU with FastCS configured: always load TB addr */
#define LOAD_TB_PTR_TO_DBT_ARG1                                         \
    do {                                                                \
        IR2_OPND tb_ptr_opnd = ra_alloc_dbt_arg1(); /* a6($10) */       \
        tr_gen_exit_tb_load_tb_addr(&tb_ptr_opnd, (ADDR)lsenv->tr_data->curr_tb); \
    } while (0)

#else
/* SOFTMMU without FASTCS: depends on LSFPU  */
#define LOAD_TB_PTR_TO_DBT_ARG1                                             \
    do {                                                                    \
        if (!option_lsfpu) {                                                \
            IR2_OPND tb_ptr_opnd = ra_alloc_dbt_arg1(); /* a6($10) */       \
            tr_gen_exit_tb_load_tb_addr(&tb_ptr_opnd, (ADDR)lsenv->tr_data->curr_tb); \
        }                                                                   \
   } while (0)
#endif

#define GEN_EOB tr_gen_eob()

#else /* no SOFTMMU */
#define LOAD_TB_PTR_TO_DBT_ARG1
#define GEN_EOB
#endif

#ifdef CONFIG_SOFTMMU
#define SET_JCC_AND_RECORD_MIPS_BRANCH_INST                                 \
    do {                                                                    \
        if(xtm_branch_opt()){                                               \
            lsenv->tr_data->curr_tb->extra_tb->end_with_jcc = 1;            \
            IR2_OPND goto_label_opnd = ir2_opnd_new_label();                \
            append_ir2_opnd1(LISA_LABEL, &goto_label_opnd);                 \
            lsenv->tr_data->curr_tb->extra_tb->mips_branch_inst_offset =    \
                ir2_opnd_label_id(&goto_label_opnd);                        \
        }                                                                   \
    } while (0)
#else
#define SET_JCC_AND_RECORD_MIPS_BRANCH_INST
#endif


bool translate_jz(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = ir2_opnd_new_label(); 

//#ifndef CONFIG_SOFTMMU
//    if (fp_translate_pattern_tail(pir1, target_label_opnd)) {
//        goto _GEN_EXIT_;
//    }
//#endif

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    if (option_lbt) {
        IR2_OPND cond = ra_alloc_itemp();
        append_ir2_opnd1i(LISA_SETX86J, &cond, COND_E);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
        ra_free_temp(&cond);
    } else {
        IR2_OPND zf_opnd = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ANDI, &zf_opnd, &eflags_ir2_opnd, ZF_BIT);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &zf_opnd, &target_label_opnd);
        ra_free_temp(&zf_opnd);
    }

//#ifndef CONFIG_SOFTMMU
//_GEN_EXIT_:
//#endif
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);
    return true;
}

bool translate_jnz(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = ir2_opnd_new_label();

//#ifndef CONFIG_SOFTMMU
//    if (fp_translate_pattern_tail(pir1, target_label_opnd)) {
//        goto _GEN_EXIT_;
//    }
//#endif

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    if (option_lbt) {
        IR2_OPND cond = ra_alloc_itemp();
        append_ir2_opnd1i(LISA_SETX86J, &cond, COND_NE);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
        ra_free_temp(&cond);
    } else {
        IR2_OPND zf_opnd = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ANDI, &zf_opnd, &eflags_ir2_opnd, ZF_BIT);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BEQZ, &zf_opnd, &target_label_opnd);
        ra_free_temp(&zf_opnd);
    }

//#ifndef CONFIG_SOFTMMU
//_GEN_EXIT_:
//#endif
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);
    return true;
}

bool translate_js(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = ir2_opnd_new_label();

//#ifndef CONFIG_SOFTMMU
//    if (fp_translate_pattern_tail(pir1, target_label_opnd)) {
//        goto _GEN_EXIT_;
//    }
//#endif

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    if (option_lbt) {
        IR2_OPND cond = ra_alloc_itemp();
        append_ir2_opnd1i(LISA_SETX86J, &cond, COND_S);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
        ra_free_temp(&cond);
//        append_ir2_opnd1(mips_x86js, &target_label_opnd);            
    } else {
        IR2_OPND sf_opnd = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ANDI, &sf_opnd, &eflags_ir2_opnd, SF_BIT);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &sf_opnd, &target_label_opnd);
        ra_free_temp(&sf_opnd);
    }

//#ifndef CONFIG_SOFTMMU
//_GEN_EXIT_:
//#endif
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);
    return true;
}

bool translate_jns(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = ir2_opnd_new_label();

//#ifndef CONFIG_SOFTMMU
//    if (fp_translate_pattern_tail(pir1, target_label_opnd)) {
//        goto _GEN_EXIT_;
//    }
//#endif

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    if (option_lbt) {
        IR2_OPND cond = ra_alloc_itemp();
        append_ir2_opnd1i(LISA_SETX86J, &cond, COND_NS);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
        ra_free_temp(&cond);
    } else {
        IR2_OPND sf_opnd = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ANDI, &sf_opnd, &eflags_ir2_opnd, SF_BIT);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BEQZ, &sf_opnd, &target_label_opnd);
        ra_free_temp(&sf_opnd);
    }

//#ifndef CONFIG_SOFTMMU
//_GEN_EXIT_:
//#endif
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);
    return true;
}

bool translate_jb(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = ir2_opnd_new_label();

//#ifndef CONFIG_SOFTMMU
//    if (fp_translate_pattern_tail(pir1, target_label_opnd)) {
//        goto _GEN_EXIT_;
//    }
//#endif

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    if (option_lbt) {
        IR2_OPND cond = ra_alloc_itemp();
        append_ir2_opnd1i(LISA_SETX86J, &cond, COND_B);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
        ra_free_temp(&cond);
    } else {
        IR2_OPND cf_opnd = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ANDI, &cf_opnd, &eflags_ir2_opnd, CF_BIT);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &cf_opnd, &target_label_opnd);
        ra_free_temp(&cf_opnd);
    }

//#ifndef CONFIG_SOFTMMU
//_GEN_EXIT_:
//#endif
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);
    return true;
}

bool translate_jae(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = ir2_opnd_new_label();

//#ifndef CONFIG_SOFTMMU
//    if (fp_translate_pattern_tail(pir1, target_label_opnd)) {
//        goto _GEN_EXIT_;
//    }
//#endif

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    if (option_lbt) {
        IR2_OPND cond = ra_alloc_itemp();
        append_ir2_opnd1i(LISA_SETX86J, &cond, COND_AE);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
        ra_free_temp(&cond);
    } else {
        IR2_OPND cf_opnd = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ANDI, &cf_opnd, &eflags_ir2_opnd, CF_BIT);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BEQZ, &cf_opnd, &target_label_opnd);
        ra_free_temp(&cf_opnd);
    }

//#ifndef CONFIG_SOFTMMU
//_GEN_EXIT_:
//#endif
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);
    return true;
}

bool translate_jo(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = ir2_opnd_new_label();

//#ifndef CONFIG_SOFTMMU
//    if (fp_translate_pattern_tail(pir1, target_label_opnd)) {
//        goto _GEN_EXIT_;
//    }
//#endif

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    if (option_lbt) {
        IR2_OPND cond = ra_alloc_itemp();
        append_ir2_opnd1i(LISA_SETX86J, &cond, COND_O);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
        ra_free_temp(&cond);
    } else {
        IR2_OPND of_opnd = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ANDI, &of_opnd, &eflags_ir2_opnd, OF_BIT);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &of_opnd, &target_label_opnd);
        ra_free_temp(&of_opnd);
    }

//#ifndef CONFIG_SOFTMMU
//_GEN_EXIT_:
//#endif
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);
    return true;
}

bool translate_jno(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = ir2_opnd_new_label();

//#ifndef CONFIG_SOFTMMU
//    if (fp_translate_pattern_tail(pir1, target_label_opnd)) {
//        goto _GEN_EXIT_;
//    }
//#endif

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    if (option_lbt) {
        IR2_OPND cond = ra_alloc_itemp();
        append_ir2_opnd1i(LISA_SETX86J, &cond, COND_NO);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
        ra_free_temp(&cond);
    } else {
        IR2_OPND of_opnd = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ANDI, &of_opnd, &eflags_ir2_opnd, OF_BIT);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BEQZ, &of_opnd, &target_label_opnd);
        ra_free_temp(&of_opnd);
    }

//#ifndef CONFIG_SOFTMMU
//_GEN_EXIT_:
//#endif
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);
    return true;
}

bool translate_jbe(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = ir2_opnd_new_label();

//#ifndef CONFIG_SOFTMMU
//    if (fp_translate_pattern_tail(pir1, target_label_opnd)) {
//        goto _GEN_EXIT_;
//    }
//#endif

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    if (option_lbt) {
        IR2_OPND cond = ra_alloc_itemp();
        append_ir2_opnd1i(LISA_SETX86J, &cond, COND_BE);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
        ra_free_temp(&cond);
    } else {
        IR2_OPND cfzf_opnd = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ANDI, &cfzf_opnd, &eflags_ir2_opnd, CF_BIT | ZF_BIT);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &cfzf_opnd, &target_label_opnd);
        ra_free_temp(&cfzf_opnd);
    }

//#ifndef CONFIG_SOFTMMU
//_GEN_EXIT_:
//#endif
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);
    return true;
}

bool translate_ja(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = ir2_opnd_new_label();

//#ifndef CONFIG_SOFTMMU
//    if (fp_translate_pattern_tail(pir1, target_label_opnd)) {
//        goto _GEN_EXIT_;
//    }
//#endif

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    if (option_lbt) {
        IR2_OPND cond = ra_alloc_itemp();
        append_ir2_opnd1i(LISA_SETX86J, &cond, COND_A);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
        ra_free_temp(&cond);
    } else {
        IR2_OPND cfzf_opnd = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ANDI, &cfzf_opnd, &eflags_ir2_opnd, CF_BIT | ZF_BIT);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BEQZ, &cfzf_opnd, &target_label_opnd);
        ra_free_temp(&cfzf_opnd);
    }

//#ifndef CONFIG_SOFTMMU
//_GEN_EXIT_:
//#endif
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);
    return true;
}

bool translate_jp(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = ir2_opnd_new_label();

//#ifndef CONFIG_SOFTMMU
//    if (fp_translate_pattern_tail(pir1, target_label_opnd)) {
//        goto _GEN_EXIT_;
//    }
//#endif

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    if (option_lbt) {
        IR2_OPND cond = ra_alloc_itemp();
        append_ir2_opnd1i(LISA_SETX86J, &cond, COND_PE);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
        ra_free_temp(&cond);
    } else {
        IR2_OPND pf_opnd = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ANDI, &pf_opnd, &eflags_ir2_opnd, PF_BIT);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &pf_opnd, &target_label_opnd);
        ra_free_temp(&pf_opnd);
    }

//#ifndef CONFIG_SOFTMMU
//_GEN_EXIT_:
//#endif
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);
    return true;
}

bool translate_jnp(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = ir2_opnd_new_label();

//#ifndef CONFIG_SOFTMMU
//    if (fp_translate_pattern_tail(pir1, target_label_opnd)) {
//        goto _GEN_EXIT_;
//    }
//#endif

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    if (option_lbt) {
        IR2_OPND cond = ra_alloc_itemp();
        append_ir2_opnd1i(LISA_SETX86J, &cond, COND_PO);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
        ra_free_temp(&cond);
    } else {
        IR2_OPND pf_opnd = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ANDI, &pf_opnd, &eflags_ir2_opnd, PF_BIT);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BEQZ, &pf_opnd, &target_label_opnd);
        ra_free_temp(&pf_opnd);
    }

//#ifndef CONFIG_SOFTMMU
//_GEN_EXIT_:
//#endif
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);
    return true;
}

bool translate_jl(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = ir2_opnd_new_label();

//#ifndef CONFIG_SOFTMMU
//    if (fp_translate_pattern_tail(pir1, target_label_opnd)) {
//        goto _GEN_EXIT_;
//    }
//#endif

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    if (option_lbt) {
        IR2_OPND cond = ra_alloc_itemp();
        append_ir2_opnd1i(LISA_SETX86J, &cond, COND_L);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
        ra_free_temp(&cond);
    } else {
        IR2_OPND sf_opnd = ra_alloc_itemp();
        IR2_OPND of_opnd = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ANDI, &sf_opnd, &eflags_ir2_opnd, SF_BIT);
        append_ir2_opnd2i(LISA_ANDI, &of_opnd, &eflags_ir2_opnd, OF_BIT);
        append_ir2_opnd2i(LISA_SRLI_D, &of_opnd, &of_opnd, OF_BIT_INDEX - SF_BIT_INDEX);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd3(LISA_BNE, &sf_opnd, &of_opnd, &target_label_opnd);
        ra_free_temp(&sf_opnd);
        ra_free_temp(&of_opnd);
    }

//#ifndef CONFIG_SOFTMMU
//_GEN_EXIT_:
//#endif
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);
    return true;
}

bool translate_jge(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = ir2_opnd_new_label();

//#ifndef CONFIG_SOFTMMU
//    if (fp_translate_pattern_tail(pir1, target_label_opnd)) {
//        goto _GEN_EXIT_;
//    }
//#endif

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    if (option_lbt) {
        IR2_OPND cond = ra_alloc_itemp();
        append_ir2_opnd1i(LISA_SETX86J, &cond, COND_GE);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
        ra_free_temp(&cond);
    } else {
        IR2_OPND sf_opnd = ra_alloc_itemp();
        IR2_OPND of_opnd = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ANDI, &sf_opnd, &eflags_ir2_opnd, SF_BIT);
        append_ir2_opnd2i(LISA_ANDI, &of_opnd, &eflags_ir2_opnd, OF_BIT);
        append_ir2_opnd2i(LISA_SRLI_D, &of_opnd, &of_opnd, OF_BIT_INDEX - SF_BIT_INDEX);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd3(LISA_BEQ, &sf_opnd, &of_opnd, &target_label_opnd);
        ra_free_temp(&sf_opnd);
        ra_free_temp(&of_opnd);
    }

//#ifndef CONFIG_SOFTMMU
//_GEN_EXIT_:
//#endif
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);
    return true;
}

bool translate_jle(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = ir2_opnd_new_label();

//#ifndef CONFIG_SOFTMMU
//    if (fp_translate_pattern_tail(pir1, target_label_opnd)) {
//        goto _GEN_EXIT_;
//    }
//#endif

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    if (option_lbt) {
        IR2_OPND cond = ra_alloc_itemp();
        append_ir2_opnd1i(LISA_SETX86J, &cond, COND_LE);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
        ra_free_temp(&cond);
    } else {
        IR2_OPND sfzf_opnd = ra_alloc_itemp();
        IR2_OPND of_opnd = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ANDI, &sfzf_opnd, &eflags_ir2_opnd, SF_BIT | ZF_BIT);   //sf | zf
        append_ir2_opnd2i(LISA_ANDI, &of_opnd, &eflags_ir2_opnd, OF_BIT);              //of
        append_ir2_opnd2i(LISA_SRLI_D, &of_opnd, &of_opnd, 4);
        append_ir2_opnd3(LISA_XOR, &sfzf_opnd, &sfzf_opnd, &of_opnd);               //sfzf xor of  
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &sfzf_opnd, &target_label_opnd);
        ra_free_temp(&sfzf_opnd);
        ra_free_temp(&of_opnd);
    }

//#ifndef CONFIG_SOFTMMU
//_GEN_EXIT_:
//#endif
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);
    return true;
}

bool translate_jg(IR1_INST *pir1)
{
    IR2_OPND target_label_opnd = ir2_opnd_new_label();

//#ifndef CONFIG_SOFTMMU
//    if (fp_translate_pattern_tail(pir1, target_label_opnd)) {
//        goto _GEN_EXIT_;
//    }
//#endif

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    if (option_lbt) {
        IR2_OPND cond = ra_alloc_itemp();
        append_ir2_opnd1i(LISA_SETX86J, &cond, COND_G);
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BNEZ, &cond, &target_label_opnd);
        ra_free_temp(&cond);
    } else {
        IR2_OPND sfzf_opnd = ra_alloc_itemp();
        IR2_OPND of_opnd = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ANDI, &sfzf_opnd, &eflags_ir2_opnd,
                          SF_BIT | ZF_BIT);                         /* sf | zf */
        append_ir2_opnd2i(LISA_ANDI, &of_opnd, &eflags_ir2_opnd, OF_BIT); /* of */
        append_ir2_opnd2i(LISA_SRLI_D, &of_opnd, &of_opnd, 4);
        append_ir2_opnd3(LISA_XOR, &sfzf_opnd, &sfzf_opnd, &of_opnd); /* sfzf xor of */
        SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
        append_ir2_opnd2(LISA_BEQZ, &sfzf_opnd, &target_label_opnd);
        ra_free_temp(&sfzf_opnd);
        ra_free_temp(&of_opnd);
    }

//#ifndef CONFIG_SOFTMMU
//_GEN_EXIT_:
//#endif
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);
    return true;
}

bool translate_jcxz(IR1_INST *pir1)
{
    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND cx_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&cx_opnd, &cx_ir1_opnd, ZERO_EXTENSION, false);

    IR2_OPND target_label_opnd = ir2_opnd_new_label();
    SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
    append_ir2_opnd2(LISA_BEQZ, &cx_opnd, &target_label_opnd);

    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
}

bool translate_jecxz(IR1_INST *pir1)
{
    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND ecx_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&ecx_opnd, &ecx_ir1_opnd, ZERO_EXTENSION, false);

    IR2_OPND target_label_opnd = ir2_opnd_new_label();
    SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
    append_ir2_opnd2(LISA_BEQZ, &ecx_opnd, &target_label_opnd);

    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
}

bool translate_jrcxz(IR1_INST *pir1)
{
    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;

    IR2_OPND rcx_opnd = ra_alloc_gpr(ecx_index);

    IR2_OPND target_label_opnd = ir2_opnd_new_label();
    SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
    append_ir2_opnd2(LISA_BEQZ, &rcx_opnd, &target_label_opnd);

    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label_opnd);
    tr_generate_exit_tb(pir1, 1);

    return true;
}

bool translate_loopnz(IR1_INST *pir1)
{
    IR2_OPND ir2_xcx;
    int ir2_xcx_tmp = 0;

    if (ir1_addr_size(pir1) == 2) {
        ir2_xcx = ra_alloc_itemp(); ir2_xcx_tmp = 1;
        load_ir1_to_ir2(&ir2_xcx, &cx_ir1_opnd, ZERO_EXTENSION, false);
        append_ir2_opnd2i(LISA_ADDI_W, &ir2_xcx, &ir2_xcx, -1);
        store_ir2_to_ir1(&ir2_xcx, &cx_ir1_opnd, false);
    }
    else if (ir1_addr_size(pir1) == 4) {
        ir2_xcx = ra_alloc_itemp(); ir2_xcx_tmp = 1;
        load_ir1_to_ir2(&ir2_xcx, &ecx_ir1_opnd, ZERO_EXTENSION, false);
        append_ir2_opnd2i(LISA_ADDI_W, &ir2_xcx, &ir2_xcx, -1);
        store_ir2_to_ir1(&ir2_xcx, &ecx_ir1_opnd, false);
    }
    else {
        ir2_xcx = ra_alloc_gpr(ir1_opnd_base_reg_num(&rcx_ir1_opnd));
        append_ir2_opnd2i(LISA_ADDI_D, &ir2_xcx, &ir2_xcx, -1);
    }

    IR2_OPND target_label = ir2_opnd_new_label();

    IR2_OPND temp_zf = ra_alloc_itemp();
    get_eflag_condition(&temp_zf, pir1);

    //By now, if zf =0, then temp_zf =0 else temp_zf = 0xFFFFFFFFFFFFFFFF
    append_ir2_opnd3(LISA_NOR, &temp_zf, &temp_zf, &zero_ir2_opnd);
    append_ir2_opnd3(LISA_AND, &temp_zf, &temp_zf, &ir2_xcx);

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
    append_ir2_opnd2(LISA_BNEZ, &temp_zf, &target_label);
    ra_free_temp(&temp_zf);

    /* env->tr_data->curr_tb->generate_tb_linkage(0); */
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label);

    /* env->tr_data->curr_tb->generate_tb_linkage(1); */
    tr_generate_exit_tb(pir1, 1);

    if (ir2_xcx_tmp) ra_free_temp(&ir2_xcx);

    return true;
}

bool translate_loopz(IR1_INST *pir1)
{
    IR2_OPND ir2_xcx;
    int ir2_xcx_tmp = 0;

    if (ir1_addr_size(pir1) == 2) {
        ir2_xcx = ra_alloc_itemp(); ir2_xcx_tmp = 1;
        load_ir1_to_ir2(&ir2_xcx, &cx_ir1_opnd, ZERO_EXTENSION, false);
        append_ir2_opnd2i(LISA_ADDI_W, &ir2_xcx, &ir2_xcx, -1);
        store_ir2_to_ir1(&ir2_xcx, &cx_ir1_opnd, false);
    }
    else if (ir1_addr_size(pir1) == 4) {
        ir2_xcx = ra_alloc_itemp(); ir2_xcx_tmp = 1;
        load_ir1_to_ir2(&ir2_xcx, &ecx_ir1_opnd, ZERO_EXTENSION, false);
        append_ir2_opnd2i(LISA_ADDI_W, &ir2_xcx, &ir2_xcx, -1);
        store_ir2_to_ir1(&ir2_xcx, &ecx_ir1_opnd, false);
    }
    else {
        ir2_xcx = ra_alloc_gpr(ir1_opnd_base_reg_num(&rcx_ir1_opnd));
        append_ir2_opnd2i(LISA_ADDI_D, &ir2_xcx, &ir2_xcx, -1);
    }

    IR2_OPND target_label = ir2_opnd_new_label();

    IR2_OPND temp_zf = ra_alloc_itemp();
    get_eflag_condition(&temp_zf, pir1);

    //By now, if zf =0, then temp_zf =0 else temp_zf = 0xFFFFFFFFFFFFFFFF
    append_ir2_opnd3(LISA_AND, &temp_zf, &temp_zf, &ir2_xcx);

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
    append_ir2_opnd2(LISA_BNEZ, &temp_zf, &target_label);
    ra_free_temp(&temp_zf);

    /* env->tr_data->curr_tb->generate_tb_linkage(0); */
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label);
    /* env->tr_data->curr_tb->generate_tb_linkage(1); */
    tr_generate_exit_tb(pir1, 1);

    if (ir2_xcx_tmp) ra_free_temp(&ir2_xcx);

    return true;
}

bool translate_loop(IR1_INST *pir1)
{
    IR2_OPND ir2_xcx;
    int ir2_xcx_tmp = 0;

    if (ir1_addr_size(pir1) == 2) {
        ir2_xcx = ra_alloc_itemp(); ir2_xcx_tmp = 1;
        load_ir1_to_ir2(&ir2_xcx, &cx_ir1_opnd, ZERO_EXTENSION, false);
        append_ir2_opnd2i(LISA_ADDI_W, &ir2_xcx, &ir2_xcx, -1);
        store_ir2_to_ir1(&ir2_xcx, &cx_ir1_opnd, false);
    }
    else if (ir1_addr_size(pir1) == 4) {
        ir2_xcx = ra_alloc_itemp(); ir2_xcx_tmp = 1;
        load_ir1_to_ir2(&ir2_xcx, &ecx_ir1_opnd, ZERO_EXTENSION, false);
        append_ir2_opnd2i(LISA_ADDI_W, &ir2_xcx, &ir2_xcx, -1);
        store_ir2_to_ir1(&ir2_xcx, &ecx_ir1_opnd, false);
    }
    else {
        ir2_xcx = ra_alloc_gpr(ir1_opnd_base_reg_num(&rcx_ir1_opnd));
        append_ir2_opnd2i(LISA_ADDI_D, &ir2_xcx, &ir2_xcx, -1);
    }

    IR2_OPND target_label = ir2_opnd_new_label();

    GEN_EOB;
    LOAD_TB_PTR_TO_DBT_ARG1;
    SET_JCC_AND_RECORD_MIPS_BRANCH_INST;
    append_ir2_opnd2(LISA_BNEZ, &ir2_xcx, &target_label);

    /* env->tr_data->curr_tb->generate_tb_linkage(0); */
    tr_generate_exit_tb(pir1, 0);
    append_ir2_opnd1(LISA_LABEL, &target_label);
    /* env->tr_data->curr_tb->generate_tb_linkage(1); */
    tr_generate_exit_tb(pir1, 1);

    if (ir2_xcx_tmp) ra_free_temp(&ir2_xcx);

    return true;
}
