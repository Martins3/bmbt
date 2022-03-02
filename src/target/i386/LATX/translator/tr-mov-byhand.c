#include "common.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"

static bool translate_mov_byhand_8_16_32(IR1_INST *pir1);
bool translate_movsx_byhand_32(IR1_INST *pir1);
bool translate_movsx_byhand_8_16(IR1_INST *pir1);
bool translate_movzx_byhand_32(IR1_INST *pir1);
bool translate_movzx_byhand_8_16(IR1_INST *pir1);

static bool translate_mov_byhand_8_16_32(IR1_INST *pir1)
{
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32 &&
        ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0))) {
        IR2_OPND dest_opnd =
            ra_alloc_gpr(ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)));
        load_ireg_from_ir1_2(dest_opnd, ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION,
                             false);
    } else {
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);
        store_ireg_to_ir1(src_opnd, ir1_get_opnd(pir1, 0), false);
    }

    return true;
}

bool translate_mov_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        return translate_mov_byhand_8_16_32(pir1);
    } else {
        return translate_mov(pir1);
    }
}

bool translate_movsx_byhand_32(IR1_INST *pir1)
{
    IR2_OPND dest_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)));
    load_ireg_from_ir1_2(dest_opnd, ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);
    return true;
}

bool translate_movsx_byhand_8_16(IR1_INST *pir1)
{
    IR2_OPND src_opnd =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);
    store_ireg_to_ir1(src_opnd, ir1_get_opnd(pir1, 0), false);
    return true;
}

bool translate_movsx_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
            return translate_movsx_byhand_32(pir1);
        } else {
            return translate_movsx_byhand_8_16(pir1);
        }
    } else {
        return translate_movsx(pir1);
    }
}

bool translate_movzx_byhand_32(IR1_INST *pir1)
{
    IR2_OPND dest_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)));
    load_ireg_from_ir1_2(dest_opnd, ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);
    return true;
}

bool translate_movzx_byhand_8_16(IR1_INST *pir1)
{
    IR2_OPND src_opnd =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);
    store_ireg_to_ir1(src_opnd, ir1_get_opnd(pir1, 0), false);
    return true;
}

bool translate_movzx_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
            return translate_movzx_byhand_32(pir1);
        } else {
            return translate_movzx_byhand_8_16(pir1);
        }
    } else {
        return translate_movzx(pir1);
    }
}
