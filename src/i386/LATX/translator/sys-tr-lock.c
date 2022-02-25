#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>

#ifdef TARGET_X86_64
#define LATXS_MOV_ADDR_TO_ARG(argx, src)                       \
    do {                                                       \
        if (latxs_ir1_addr_size(pir1) == 8) {                  \
            latxs_append_ir2_opnd2_(lisa_mov, &argx, &src);    \
        } else {                                               \
            latxs_append_ir2_opnd2_(lisa_mov32z, &argx, &src); \
        }                                                      \
    } while (0)
#else
#define LATXS_MOV_ADDR_TO_ARG(argx, src)                   \
    do {                                                   \
        latxs_append_ir2_opnd2_(lisa_mov32s, &argx, &src); \
    } while (0)
#endif

static inline int opsz2index(int opsz)
{
    /* 8->0, 16->1, 32->2, 64->3*/
    lsassert(opsz == 8 || opsz == 16 || opsz == 32 || opsz == 64);
    return __builtin_ctz(opsz) - 3;
}
static inline int opsz2tcgmemop(int opsz)
{
    /* sign and endian are irrelevant */
    static const int ops[] = {MO_8, MO_16, MO_32, MO_64};
    return ops[opsz2index(opsz)];
}

static ADDR atomis_add_functions[4] = {
    (ADDR)helper_atomic_fetch_addb,
    (ADDR)helper_atomic_fetch_addw_le,
    (ADDR)helper_atomic_fetch_addl_le,
    (ADDR)helper_atomic_fetch_addq_le,
};
static ADDR atomis_and_functions[4] = {
    (ADDR)helper_atomic_fetch_andb,
    (ADDR)helper_atomic_fetch_andw_le,
    (ADDR)helper_atomic_fetch_andl_le,
    (ADDR)helper_atomic_fetch_andq_le,
};
static ADDR atomis_or_functions[4] = {
    (ADDR)helper_atomic_fetch_orb,
    (ADDR)helper_atomic_fetch_orw_le,
    (ADDR)helper_atomic_fetch_orl_le,
    (ADDR)helper_atomic_fetch_orq_le,
};
static ADDR atomis_xor_functions[4] = {
    (ADDR)helper_atomic_fetch_xorb,
    (ADDR)helper_atomic_fetch_xorw_le,
    (ADDR)helper_atomic_fetch_xorl_le,
    (ADDR)helper_atomic_fetch_xorq_le,
};
static ADDR atomis_cmpxchg_functions[4] = {
    (ADDR)helper_atomic_cmpxchgb,
    (ADDR)helper_atomic_cmpxchgw_le,
    (ADDR)helper_atomic_cmpxchgl_le,
    (ADDR)helper_atomic_cmpxchgq_le,
};
static ADDR atomis_xchg_functions[4] = {
    (ADDR)helper_atomic_xchgb,
    (ADDR)helper_atomic_xchgw_le,
    (ADDR)helper_atomic_xchgl_le,
    (ADDR)helper_atomic_xchgq_le,
};
bool latxs_translate_lock_add(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    int opsz = ir1_opnd_size(opnd0);

    IR2_OPND src0_mem_addr = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&src0_mem_addr, opnd0);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, src0_mem_addr);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg2_ir2_opnd, &src1);
    latxs_load_imm64_to_ir2(
        &latxs_arg3_ir2_opnd,
        (opsz2tcgmemop(opsz) << 4) | lsenv->tr_data->sys.mem_index);

    latxs_tr_gen_call_to_helper((ADDR)atomis_add_functions[opsz2index(opsz)]);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* temp registers were not saved */
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_N);
    latxs_generate_eflag_calculation(&latxs_zero_ir2_opnd, &latxs_ret0_ir2_opnd,
                                     &src1, pir1, true);

    latxs_ra_free_temp(&src0_mem_addr);
    latxs_ra_free_temp(&src1);
    return true;
}
bool latxs_translate_lock_sub(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    int opsz = ir1_opnd_size(opnd0);

    IR2_OPND src0_mem_addr = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&src0_mem_addr, opnd0);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, src0_mem_addr);
    latxs_append_ir2_opnd3(LISA_SUB_D, &latxs_arg2_ir2_opnd,
                           &latxs_zero_ir2_opnd, &src1);
    latxs_load_imm64_to_ir2(
        &latxs_arg3_ir2_opnd,
        (opsz2tcgmemop(opsz) << 4) | lsenv->tr_data->sys.mem_index);

    latxs_tr_gen_call_to_helper((ADDR)atomis_add_functions[opsz2index(opsz)]);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* temp registers were not saved */
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_N);
    latxs_generate_eflag_calculation(&latxs_zero_ir2_opnd, &latxs_ret0_ir2_opnd,
                                     &src1, pir1, true);

    latxs_ra_free_temp(&src0_mem_addr);
    latxs_ra_free_temp(&src1);
    return true;
}
bool latxs_translate_lock_adc(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    int opsz = ir1_opnd_size(opnd0);

    IR2_OPND src0_mem_addr = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&src0_mem_addr, opnd0);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, src0_mem_addr);
    latxs_load_eflags_cf_to_ir2(&latxs_arg2_ir2_opnd);
    latxs_append_ir2_opnd3(LISA_ADD_D, &latxs_arg2_ir2_opnd,
                           &latxs_arg2_ir2_opnd, &src1);
    latxs_load_imm64_to_ir2(
        &latxs_arg3_ir2_opnd,
        (opsz2tcgmemop(opsz) << 4) | lsenv->tr_data->sys.mem_index);

    latxs_tr_gen_call_to_helper((ADDR)atomis_add_functions[opsz2index(opsz)]);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* temp registers were not saved */
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_N);
    latxs_generate_eflag_calculation(&latxs_zero_ir2_opnd, &latxs_ret0_ir2_opnd,
                                     &src1, pir1, true);

    latxs_ra_free_temp(&src0_mem_addr);
    latxs_ra_free_temp(&src1);
    return true;
}
bool latxs_translate_lock_sbb(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    int opsz = ir1_opnd_size(opnd0);

    IR2_OPND src0_mem_addr = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&src0_mem_addr, opnd0);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, src0_mem_addr);

    latxs_load_eflags_cf_to_ir2(&latxs_arg2_ir2_opnd);
    latxs_append_ir2_opnd3(LISA_SUB_D, &latxs_arg2_ir2_opnd,
                           &latxs_zero_ir2_opnd, &latxs_arg2_ir2_opnd);
    latxs_append_ir2_opnd3(LISA_SUB_D, &latxs_arg2_ir2_opnd,
                           &latxs_arg2_ir2_opnd, &src1);
    latxs_load_imm64_to_ir2(
        &latxs_arg3_ir2_opnd,
        (opsz2tcgmemop(opsz) << 4) | lsenv->tr_data->sys.mem_index);

    latxs_tr_gen_call_to_helper((ADDR)atomis_add_functions[opsz2index(opsz)]);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* temp registers were not saved */
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_N);
    latxs_generate_eflag_calculation(&latxs_zero_ir2_opnd, &latxs_ret0_ir2_opnd,
                                     &src1, pir1, true);

    latxs_ra_free_temp(&src0_mem_addr);
    latxs_ra_free_temp(&src1);
    return true;
}
bool latxs_translate_lock_inc(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    int opsz = ir1_opnd_size(opnd0);

    IR2_OPND src0_mem_addr = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&src0_mem_addr, opnd0);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, src0_mem_addr);
    latxs_load_imm64_to_ir2(&latxs_arg2_ir2_opnd, 1);
    latxs_load_imm64_to_ir2(
        &latxs_arg3_ir2_opnd,
        (opsz2tcgmemop(opsz) << 4) | lsenv->tr_data->sys.mem_index);

    latxs_tr_gen_call_to_helper((ADDR)atomis_add_functions[opsz2index(opsz)]);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    latxs_generate_eflag_calculation(&latxs_zero_ir2_opnd, &latxs_ret0_ir2_opnd,
                                     &latxs_zero_ir2_opnd, pir1, true);

    latxs_ra_free_temp(&src0_mem_addr);
    return true;
}
bool latxs_translate_lock_dec(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    int opsz = ir1_opnd_size(opnd0);

    IR2_OPND src0_mem_addr = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&src0_mem_addr, opnd0);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, src0_mem_addr);
    latxs_load_imm64_to_ir2(&latxs_arg2_ir2_opnd, -1);
    latxs_load_imm64_to_ir2(
        &latxs_arg3_ir2_opnd,
        (opsz2tcgmemop(opsz) << 4) | lsenv->tr_data->sys.mem_index);

    latxs_tr_gen_call_to_helper((ADDR)atomis_add_functions[opsz2index(opsz)]);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    latxs_generate_eflag_calculation(&latxs_zero_ir2_opnd, &latxs_ret0_ir2_opnd,
                                     &latxs_zero_ir2_opnd, pir1, true);

    latxs_ra_free_temp(&src0_mem_addr);
    return true;
}
bool latxs_translate_lock_xadd(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    int opsz = ir1_opnd_size(opnd0);

    IR2_OPND src0_mem_addr = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&src0_mem_addr, opnd0);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, src0_mem_addr);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg2_ir2_opnd, &src1);
    latxs_load_imm64_to_ir2(
        &latxs_arg3_ir2_opnd,
        (opsz2tcgmemop(opsz) << 4) | lsenv->tr_data->sys.mem_index);

    latxs_tr_gen_call_to_helper((ADDR)atomis_add_functions[opsz2index(opsz)]);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* temp registers were not saved */
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_N);
    latxs_generate_eflag_calculation(&latxs_zero_ir2_opnd, &latxs_ret0_ir2_opnd,
                                     &src1, pir1, true);
    latxs_store_ir2_to_ir1(&latxs_ret0_ir2_opnd, opnd1);
    latxs_ra_free_temp(&src0_mem_addr);
    latxs_ra_free_temp(&src1);
    return true;
}
bool latxs_translate_lock_and(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    int opsz = ir1_opnd_size(opnd0);

    IR2_OPND src0_mem_addr = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&src0_mem_addr, opnd0);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, src0_mem_addr);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg2_ir2_opnd, &src1);
    latxs_load_imm64_to_ir2(
        &latxs_arg3_ir2_opnd,
        (opsz2tcgmemop(opsz) << 4) | lsenv->tr_data->sys.mem_index);

    latxs_tr_gen_call_to_helper((ADDR)atomis_and_functions[opsz2index(opsz)]);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* temp registers were not saved */
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_N);
    latxs_generate_eflag_calculation(&latxs_zero_ir2_opnd, &latxs_ret0_ir2_opnd,
                                     &src1, pir1, true);

    latxs_ra_free_temp(&src0_mem_addr);
    latxs_ra_free_temp(&src1);
    return true;
}
bool latxs_translate_lock_or(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    int opsz = ir1_opnd_size(opnd0);

    IR2_OPND src0_mem_addr = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&src0_mem_addr, opnd0);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, src0_mem_addr);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg2_ir2_opnd, &src1);
    latxs_load_imm64_to_ir2(
        &latxs_arg3_ir2_opnd,
        (opsz2tcgmemop(opsz) << 4) | lsenv->tr_data->sys.mem_index);

    latxs_tr_gen_call_to_helper((ADDR)atomis_or_functions[opsz2index(opsz)]);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* temp registers were not saved */
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_N);
    latxs_generate_eflag_calculation(&latxs_zero_ir2_opnd, &latxs_ret0_ir2_opnd,
                                     &src1, pir1, true);

    latxs_ra_free_temp(&src0_mem_addr);
    latxs_ra_free_temp(&src1);
    return true;
}
bool latxs_translate_lock_xor(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    int opsz = ir1_opnd_size(opnd0);

    IR2_OPND src0_mem_addr = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&src0_mem_addr, opnd0);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, src0_mem_addr);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg2_ir2_opnd, &src1);
    latxs_load_imm64_to_ir2(
        &latxs_arg3_ir2_opnd,
        (opsz2tcgmemop(opsz) << 4) | lsenv->tr_data->sys.mem_index);

    latxs_tr_gen_call_to_helper((ADDR)atomis_xor_functions[opsz2index(opsz)]);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* temp registers were not saved */
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_N);
    latxs_generate_eflag_calculation(&latxs_zero_ir2_opnd, &latxs_ret0_ir2_opnd,
                                     &src1, pir1, true);

    latxs_ra_free_temp(&src0_mem_addr);
    latxs_ra_free_temp(&src1);
    return true;
}
bool latxs_translate_lock_not(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    int opsz = ir1_opnd_size(opnd0);

    IR2_OPND src0_mem_addr = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&src0_mem_addr, opnd0);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, src0_mem_addr);
    latxs_load_imm64_to_ir2(&latxs_arg2_ir2_opnd, -1);
    latxs_load_imm64_to_ir2(
        &latxs_arg3_ir2_opnd,
        (opsz2tcgmemop(opsz) << 4) | lsenv->tr_data->sys.mem_index);

    latxs_tr_gen_call_to_helper((ADDR)atomis_xor_functions[opsz2index(opsz)]);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    latxs_ra_free_temp(&src0_mem_addr);
    return true;
}

bool latxs_translate_lock_neg(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    int opsz = ir1_opnd_size(opnd0);

    IR2_OPND src0_mem_addr = latxs_ra_alloc_itemp();
    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND neg = latxs_ra_alloc_itemp();
    IR2_OPND label_unequal = latxs_ir2_opnd_new_label();

    latxs_append_ir2_opnd1(LISA_LABEL, &label_unequal);

    latxs_convert_mem_opnd_no_offset(&src0_mem_addr, opnd0);
    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S);
    latxs_append_ir2_opnd3(LISA_SUB_D, &neg, &latxs_zero_ir2_opnd, &src0);

    latxs_tr_save_temp_register_mask(lsenv->tr_data->itemp_mask);
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, src0_mem_addr);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg2_ir2_opnd, &src0);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg3_ir2_opnd, &neg);
    latxs_load_imm64_to_ir2(
        &latxs_arg4_ir2_opnd,
        (opsz2tcgmemop(opsz) << 4) | lsenv->tr_data->sys.mem_index);

    latxs_tr_gen_call_to_helper(
        (ADDR)atomis_cmpxchg_functions[opsz2index(opsz)]);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
    latxs_tr_restore_temp_register_mask(lsenv->tr_data->itemp_mask);

    if (opsz == 8) {
        latxs_append_ir2_opnd2_(lisa_mov8s, &latxs_ret0_ir2_opnd,
                                &latxs_ret0_ir2_opnd);
    } else if (opsz == 16) {
        latxs_append_ir2_opnd2_(lisa_mov16s, &latxs_ret0_ir2_opnd,
                                &latxs_ret0_ir2_opnd);
    }

    latxs_append_ir2_opnd3(LISA_BNE, &latxs_ret0_ir2_opnd, &src0,
                           &label_unequal);
    latxs_generate_eflag_calculation(&latxs_zero_ir2_opnd, &latxs_zero_ir2_opnd,
                                     &latxs_ret0_ir2_opnd, pir1, true);

    latxs_ra_free_temp(&src0_mem_addr);
    latxs_ra_free_temp(&src0);
    latxs_ra_free_temp(&neg);
    return true;
}
bool latxs_translate_lock_btx(IR1_INST *pir1)
{
    lsassert(ir1_opcode(pir1) != X86_INS_BT);
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* bit base   : GPR/MEM */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* bit offset : GPR/imm */
    int opsz = ir1_opnd_size(opnd0);
    lsassert(opsz == 16 || opsz == 32 || opsz == 64);

    IR2_OPND bit_off = latxs_ra_alloc_itemp();

    IR2_OPND mem = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&mem, opnd0);

    /* opnd0 is memory operand */
    if (ir1_opnd_is_imm(opnd1)) {
        latxs_load_imm64_to_ir2(&bit_off, ir1_opnd_uimm(opnd1) & (opsz - 1));
    } else {
        IR2_OPND byte_nr = latxs_ra_alloc_itemp();
        latxs_load_ir1_to_ir2(&bit_off, opnd1, EXMode_S);
        latxs_append_ir2_opnd2i(LISA_SRAI_D, &byte_nr, &bit_off,
                                __builtin_ctz(opsz));
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &byte_nr, &byte_nr,
                                opsz2index(opsz));
        latxs_append_ir2_opnd2i(LISA_ANDI, &bit_off, &bit_off, opsz - 1);

        latxs_append_ir2_opnd3(LISA_ADD_D, &mem, &mem, &byte_nr);
        latxs_ra_free_temp(&byte_nr);
    }

    IR2_OPND bit_set = latxs_ra_alloc_itemp();

    latxs_load_imm64_to_ir2(&bit_set, 1);
    latxs_append_ir2_opnd3(LISA_SLL_D, &bit_set, &bit_set, &bit_off);
    if (ir1_opcode(pir1) == X86_INS_BTR) {
        latxs_append_ir2_opnd2_(lisa_not, &bit_set, &bit_set);
    }
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, mem);

    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg2_ir2_opnd, &bit_set);
    latxs_load_imm64_to_ir2(
        &latxs_arg3_ir2_opnd,
        (opsz2tcgmemop(opsz) << 4) | lsenv->tr_data->sys.mem_index);
    /* update BitBase */
    switch (ir1_opcode(pir1)) {
    case X86_INS_BTS: /* set bit = 1 */
        latxs_tr_gen_call_to_helper(
            (ADDR)atomis_or_functions[opsz2index(opsz)]);
        break;
    case X86_INS_BTR: /* set bit = 0 */
        latxs_tr_gen_call_to_helper(
            (ADDR)atomis_and_functions[opsz2index(opsz)]);
        break;
    case X86_INS_BTC: /* set bit = not bit  */
        latxs_tr_gen_call_to_helper(
            (ADDR)atomis_xor_functions[opsz2index(opsz)]);
        break;
    default:
        lsassertm(0, "Invalid opcode in translate_btx\n");
        break;
    }
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    if (ir1_opnd_is_imm(opnd1)) {
        latxs_load_imm64_to_ir2(&bit_off, ir1_opnd_uimm(opnd1) & (opsz - 1));
    } else {
        latxs_load_ir1_to_ir2(&bit_off, opnd1, EXMode_S);
        latxs_append_ir2_opnd2i(LISA_ANDI, &bit_off, &bit_off, opsz - 1);
    }

    latxs_append_ir2_opnd3(LISA_SRL_D, &latxs_ret0_ir2_opnd,
                           &latxs_ret0_ir2_opnd, &bit_off);
    latxs_append_ir2_opnd2i(LISA_ANDI, &latxs_ret0_ir2_opnd,
                            &latxs_ret0_ir2_opnd, 1);
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &latxs_ret0_ir2_opnd, 0x1);
    latxs_ra_free_temp(&bit_set);
    latxs_ra_free_temp(&bit_off);
    return true;
}

bool latxs_translate_lock_cmpxchg(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    int opsz = ir1_opnd_size(opnd0);

    IR1_OPND *reg_ir1 = NULL;

    int opnd_size = ir1_opnd_size(opnd0);
    switch (opnd_size) {
    case 8:
        reg_ir1 = &al_ir1_opnd;
        break;
    case 16:
        reg_ir1 = &ax_ir1_opnd;
        break;
    case 32:
        reg_ir1 = &eax_ir1_opnd;
        break;
#ifdef TARGET_X86_64
    case 64:
        lsassert(lsenv->tr_data->sys.code64);
        reg_ir1 = &rax_ir1_opnd;
        break;
#endif
    default:
        lsassert(0);
        break;
    }

    IR2_OPND src0_mem_addr = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();
    IR2_OPND eax_opnd = latxs_ra_alloc_itemp();

    latxs_convert_mem_opnd_no_offset(&src0_mem_addr, opnd0);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);
    latxs_load_ir1_to_ir2(&eax_opnd, reg_ir1, EXMode_S);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, src0_mem_addr);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg2_ir2_opnd, &eax_opnd);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg3_ir2_opnd, &src1);
    latxs_load_imm64_to_ir2(
        &latxs_arg4_ir2_opnd,
        (opsz2tcgmemop(opsz) << 4) | lsenv->tr_data->sys.mem_index);

    latxs_tr_gen_call_to_helper(
        (ADDR)atomis_cmpxchg_functions[opsz2index(opsz)]);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* temp registers were not saved */
    latxs_load_ir1_to_ir2(&eax_opnd, reg_ir1, EXMode_S);
    latxs_generate_eflag_calculation(&latxs_zero_ir2_opnd, &eax_opnd,
                                     &latxs_ret0_ir2_opnd, pir1, true);
    latxs_store_ir2_to_ir1_gpr(&latxs_ret0_ir2_opnd, reg_ir1);

    latxs_ra_free_temp(&src0_mem_addr);
    latxs_ra_free_temp(&src1);
    return true;
}
bool latxs_translate_lock_cmpxchg8b(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR2_OPND src0_mem_addr = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&src0_mem_addr, opnd0);
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, src0_mem_addr);
    latxs_tr_gen_call_to_helper((ADDR)helper_cmpxchg8b);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
    latxs_ra_free_temp(&src0_mem_addr);
    return true;
}
bool latxs_translate_lock_cmpxchg16b(IR1_INST *pir1)
{
#ifdef TARGET_X86_64
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR2_OPND src0_mem_addr = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&src0_mem_addr, opnd0);
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, src0_mem_addr);
    latxs_tr_gen_call_to_helper((ADDR)helper_cmpxchg16b);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
    latxs_ra_free_temp(&src0_mem_addr);
    return true;
#else
    lsassert(0);
#endif
}
bool latxs_translate_lock_xchg(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    int opsz = ir1_opnd_size(opnd0);

    IR2_OPND src0_mem_addr = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();
    latxs_convert_mem_opnd_no_offset(&src0_mem_addr, opnd0);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    LATXS_MOV_ADDR_TO_ARG(latxs_arg1_ir2_opnd, src0_mem_addr);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg2_ir2_opnd, &src1);
    latxs_load_imm64_to_ir2(
        &latxs_arg3_ir2_opnd,
        (opsz2tcgmemop(opsz) << 4) | lsenv->tr_data->sys.mem_index);

    latxs_tr_gen_call_to_helper((ADDR)atomis_xchg_functions[opsz2index(opsz)]);
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    latxs_store_ir2_to_ir1(&latxs_ret0_ir2_opnd, opnd1);
    latxs_ra_free_temp(&src0_mem_addr);
    latxs_ra_free_temp(&src1);
    return true;
}
