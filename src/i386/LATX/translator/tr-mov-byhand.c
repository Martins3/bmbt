#include "../include/common.h"
#include "../translator/translate.h"
#include "../include/reg-alloc.h"
#include "../x86tomips-options.h"

static inline
bool translate_mov_byhand_8_16_32(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_imm0(opnd1)) {
        store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
        return true;
    }

    if (ir1_opnd_size(opnd0) == 32 && ir1_opnd_is_gpr(opnd0)) {
        IR2_OPND dest_opnd =
            ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        load_ir1_to_ir2(&dest_opnd, opnd1, UNKNOWN_EXTENSION, false);
    }
    else if(ir1_opnd_is_gpr(opnd1) && ir1_opnd_size(opnd1) == 32) {
        IR2_OPND src_opnd =
            ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
        store_ir2_to_ir1(&src_opnd, opnd0, false);
    }
    else {
        IR2_OPND src_opnd = ra_alloc_itemp();
        load_ir1_to_ir2(&src_opnd, opnd1, UNKNOWN_EXTENSION, false);
        store_ir2_to_ir1(&src_opnd, opnd0, false);
    }

    return true;
}

bool translate_mov_byhand(IR1_INST *pir1)
{
    return translate_mov_byhand_8_16_32(pir1);
}

static inline
bool translate_movsx_byhand_32(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND dest_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
    load_ir1_to_ir2(&dest_opnd, opnd1, SIGN_EXTENSION, false);
    return true;
}

static inline
bool translate_movsx_byhand_8_16(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND src_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&src_opnd, opnd1, SIGN_EXTENSION, false);
    store_ir2_to_ir1(&src_opnd, opnd0, false);
    return true;
}

bool translate_movsx_byhand(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_size(opnd0) == 32) {
        return translate_movsx_byhand_32(opnd0, opnd1);
    } else {
        return translate_movsx_byhand_8_16(opnd0, opnd1);
    }
}

static inline
bool translate_movzx_byhand_32(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND dest_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
    load_ir1_to_ir2(&dest_opnd, opnd1, ZERO_EXTENSION, false);
    return true;
}

static inline
bool translate_movzx_byhand_8_16(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND src_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&src_opnd, opnd1, ZERO_EXTENSION, false);
    store_ir2_to_ir1(&src_opnd, opnd0, false);
    return true;
}

bool translate_movzx_byhand(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_size(opnd0) == 32) {
        return translate_movzx_byhand_32(opnd0, opnd1);
    } else {
        return translate_movzx_byhand_8_16(opnd0, opnd1);
    }
}
