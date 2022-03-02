#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "flag-lbt.h"
#include "sys-excp.h"

#ifdef TARGET_X86_64
bool latxs_translate_mov_byhand64(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    if (latxs_ir1_opnd_is_gpr32(opnd0)) {
        int gpr0_num = ir1_opnd_base_reg_num(opnd0);
        IR2_OPND gpr0 = latxs_ra_alloc_gpr(gpr0_num);
        if (latxs_ir1_opnd_is_gpr32(opnd1)) {
            int gpr1_num = ir1_opnd_base_reg_num(opnd1);
            IR2_OPND gpr1 = latxs_ra_alloc_gpr(gpr1_num);
            latxs_append_ir2_opnd2_(lisa_mov32z, &gpr0, &gpr1);
            return true;
        } else if (ir1_opnd_is_mem(opnd1)) {
            latxs_load_ir1_mem_to_ir2(&gpr0, opnd1, EXMode_Z, -1);
            return true;
        }
    }
    if (latxs_ir1_opnd_is_gpr64(opnd0)) {
        int gpr0_num = ir1_opnd_base_reg_num(opnd0);
        IR2_OPND gpr0 = latxs_ra_alloc_gpr(gpr0_num);
        if (latxs_ir1_opnd_is_gpr64(opnd1)) {
            int gpr1_num = ir1_opnd_base_reg_num(opnd1);
            IR2_OPND gpr1 = latxs_ra_alloc_gpr(gpr1_num);
            latxs_append_ir2_opnd2_(lisa_mov, &gpr0, &gpr1);
            return true;
        } else if (ir1_opnd_is_mem(opnd1)) {
            latxs_load_ir1_mem_to_ir2(&gpr0, opnd1, EXMode_Z, -1);
            return true;
        }
    }

    return false;
}

bool latxs_translate_xor_byhand64(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    if (latxs_ir1_opnd_is_gpr32(opnd0) && latxs_ir1_opnd_is_gpr32(opnd1) &&
        (ir1_opnd_base_reg_num(opnd0) == ir1_opnd_base_reg_num(opnd1))) {
        int gpr0_num = ir1_opnd_base_reg_num(opnd0);
        IR2_OPND gpr0 = latxs_ra_alloc_gpr(gpr0_num);
        latxs_append_ir2_opnd2(LISA_X86XOR_W, &latxs_zero_ir2_opnd,
                               &latxs_zero_ir2_opnd);
        latxs_append_ir2_opnd2_(lisa_mov, &gpr0, &latxs_zero_ir2_opnd);
        return true;
    }
    return false;
}

bool latxs_translate_cmp_byhand64(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    int opsz = ir1_opnd_size(opnd0);
    if (latxs_ir1_opnd_is_gpr_not_8h(opnd0)) {
        int gpr0_num = ir1_opnd_base_reg_num(opnd0);
        IR2_OPND reg0 = latxs_ra_alloc_gpr(gpr0_num);
        IR2_OPND reg1;
        if (latxs_ir1_opnd_is_gpr_not_8h(opnd1)) {
            int gpr1_num = ir1_opnd_base_reg_num(opnd1);
            reg1 = latxs_ra_alloc_gpr(gpr1_num);
            latxs_append_ir2_opnd2(LISA_X86SUB_B + ((opsz >> 4) - (opsz >> 6)),
                                   &reg0, &reg1);
            return true;
        } else if (ir1_opnd_is_mem(opnd1)) {
            reg1 = latxs_ra_alloc_itemp();
            latxs_load_ir1_mem_to_ir2(&reg1, opnd1, EXMode_N, -1);
            latxs_append_ir2_opnd2(LISA_X86SUB_B + ((opsz >> 4) - (opsz >> 6)),
                                   &reg0, &reg1);
            return true;
        } else if (ir1_opnd_is_imm(opnd1)) {
            if (ir1_opnd_simm(opnd1) == 0) {
                reg1 = latxs_zero_ir2_opnd;
            } else {
                reg1 = latxs_ra_alloc_itemp();
                latxs_load_ir1_imm_to_ir2(&reg1, opnd1, EXMode_S);
            }
            latxs_append_ir2_opnd2(LISA_X86SUB_B + ((opsz >> 4) - (opsz >> 6)),
                                   &reg0, &reg1);
            return true;
        }
    }
    return false;
}

bool latxs_translate_test_byhand64(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    int opsz = ir1_opnd_size(opnd0);
    if (latxs_ir1_opnd_is_gpr_not_8h(opnd0)) {
        int gpr0_num = ir1_opnd_base_reg_num(opnd0);
        IR2_OPND reg0 = latxs_ra_alloc_gpr(gpr0_num);
        IR2_OPND reg1;
        if (latxs_ir1_opnd_is_gpr_not_8h(opnd1)) {
            int gpr1_num = ir1_opnd_base_reg_num(opnd1);
            reg1 = latxs_ra_alloc_gpr(gpr1_num);
            latxs_append_ir2_opnd2(LISA_X86AND_B + ((opsz >> 4) - (opsz >> 6)),
                                   &reg0, &reg1);
            return true;
        } else if (ir1_opnd_is_imm(opnd1)) {
            if (ir1_opnd_simm(opnd1) == 0) {
                reg1 = latxs_zero_ir2_opnd;
            } else {
                reg1 = latxs_ra_alloc_itemp();
                latxs_load_ir1_imm_to_ir2(&reg1, opnd1, EXMode_S);
            }
            latxs_append_ir2_opnd2(LISA_X86AND_B + ((opsz >> 4) - (opsz >> 6)),
                                   &reg0, &reg1);
            return true;
        } else {
            lsassert(0);
        }
    }
    return false;
}

bool latxs_translate_add_byhand64(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    if (latxs_ir1_opnd_is_gpr32(opnd0)) {
        int gpr0_num = ir1_opnd_base_reg_num(opnd0);
        IR2_OPND gpr0 = latxs_ra_alloc_gpr(gpr0_num);
        if (latxs_ir1_opnd_is_gpr32(opnd1)) {
            int gpr1_num = ir1_opnd_base_reg_num(opnd1);
            IR2_OPND gpr1 = latxs_ra_alloc_gpr(gpr1_num);
            latxs_append_ir2_opnd2(LISA_X86ADD_W, &gpr0, &gpr1);
            latxs_append_ir2_opnd3(LISA_ADD_W, &gpr0, &gpr0, &gpr1);
            latxs_append_ir2_opnd2_(lisa_mov32z, &gpr0, &gpr0);
            return true;
        } else if (ir1_opnd_is_imm(opnd1)) {
            if (int32_in_int12(ir1_opnd_simm(opnd1))) {
                IR2_OPND reg1 = latxs_ra_alloc_itemp();
                IR2_OPND temp = latxs_ra_alloc_itemp();
                latxs_load_ir1_imm_to_ir2(&reg1, opnd1, EXMode_S);
                latxs_append_ir2_opnd2i(LISA_ADDI_W, &temp, &gpr0,
                                        ir1_opnd_simm(opnd1));
                latxs_append_ir2_opnd2(LISA_X86ADD_W, &gpr0, &reg1);
                latxs_append_ir2_opnd2_(lisa_mov32z, &gpr0, &temp);
                return true;
            } else {
                return false;
            }
        }
    }

    if (latxs_ir1_opnd_is_gpr64(opnd0)) {
        int gpr0_num = ir1_opnd_base_reg_num(opnd0);
        IR2_OPND gpr0 = latxs_ra_alloc_gpr(gpr0_num);
        if (latxs_ir1_opnd_is_gpr64(opnd1)) {
            int gpr1_num = ir1_opnd_base_reg_num(opnd1);
            IR2_OPND gpr1 = latxs_ra_alloc_gpr(gpr1_num);
            latxs_append_ir2_opnd2(LISA_X86ADD_D, &gpr0, &gpr1);
            latxs_append_ir2_opnd3(LISA_ADD_D, &gpr0, &gpr0, &gpr1);
            return true;
        } else if (ir1_opnd_is_imm(opnd1)) {
            if (int32_in_int12(ir1_opnd_simm(opnd1))) {
                IR2_OPND reg1 = latxs_ra_alloc_itemp();
                latxs_load_ir1_imm_to_ir2(&reg1, opnd1, EXMode_S);
                latxs_append_ir2_opnd2(LISA_X86ADD_D, &gpr0, &reg1);
                latxs_append_ir2_opnd2i(LISA_ADDI_D, &gpr0, &gpr0,
                                        ir1_opnd_simm(opnd1));
                return true;
            } else {
                return false;
            }
        }
    }

    return false;
}

#endif
