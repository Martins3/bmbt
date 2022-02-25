#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>

static bool latxs_translate_mov_byhand_normal(
        IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND tmp = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&tmp, opnd1, EXMode_N);
    latxs_store_ir2_to_ir1(&tmp, opnd0);
    latxs_ra_free_temp(&tmp);
    return true;
}

static bool latxs_translate_mov_byhand_gpr_gpr(
        IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    int gpr_num0 = ir1_opnd_base_reg_num(opnd0);
    int gpr_num1 = ir1_opnd_base_reg_num(opnd1);

    int is_opnd0_8h = ir1_opnd_is_8h(opnd0);
    int is_opnd1_8h = ir1_opnd_is_8h(opnd1);

    /*
     * When operand size is 8:
     *
     * +------------|------- dest operand ------|
     * |    mov     +---------------------------+
     * |            |  al  |  ah  |  bl  |  bh  |
     * +------+---------------------------------+
     * | src  |  al |  1   |  2   |  3   |  2   |
     * | opnd |-----|------+------+------+------|
     * |      |  ah |  3   |  1   |  3   |  2   |
     * +------+---------------------------------+
     *
     * What should we do:
     *   > 1: do nothing
     *   > 2: pick from source and write into dest
     *   > 3: use source directly and write into dest
     */

    if (is_opnd0_8h || is_opnd1_8h) {
        /* 1: do nothing: 8: mov ah, ah */
        if (is_opnd0_8h && is_opnd1_8h && gpr_num0 == gpr_num1) {
            return true;
        }
        /*
         * 2: pick from source and write into dest
         * >  8: mov al, ah
         * >  8: mov al, bh
         * >  8: mov ah, bh
         */
        if (is_opnd1_8h) {
            return latxs_translate_mov_byhand_normal(opnd0, opnd1);
        }
        /*
         * 3: use source directly and write into dest
         * >  8: mov ah, bl
         * >  8: mov ah, al
         */
        IR2_OPND gpr1 = latxs_ra_alloc_gpr(gpr_num1);
        latxs_store_ir2_to_ir1_gpr_em(&gpr1, opnd0);
        return true;
    }

    /*
     * 1: do nothing:
     * > 32: mov eax, eax
     * > 16: mov  ax,  ax
     * >  8: mov  al,  al
     */
    if (gpr_num0 == gpr_num1) {
        return true;
    }

    /*
     * 3: use source directly and write into dest
     * > 32: mov eax, ebx
     * > 16: mov  ax,  bx
     * >  8: mov  al,  bl
     */
    IR2_OPND gpr1 = latxs_ra_alloc_gpr(gpr_num1);
    latxs_store_ir2_to_ir1_gpr_em(&gpr1, opnd0);
    return true;
}

static bool latxs_translate_mov_byhand_gpr_imm(
        IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    int os = ir1_opnd_size(opnd0);

    int gpr_num = ir1_opnd_base_reg_num(opnd0);
    IR2_OPND gpr = latxs_ra_alloc_gpr(gpr_num);

    if (os == 32) {
        /*
         * When could imm be loaded directly to GPR:
         * > 32-bit GPR: any EM
         * > 16-bit GPR: EM-bits <= 16
         * >  8-bit GPR: EM-bits <=  8
         */
        latxs_load_ir1_imm_to_ir2_em(&gpr, opnd1, EXMode_Z);
    } else {
        /*
         * When we must use temp to move data
         * > 16-bit GPR: EM-bits > 16
         * >  8-bit GPR: EM-bits >  8
         */
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_load_ir1_imm_to_ir2(&tmp, opnd1, EXMode_Z);
        latxs_store_ir2_to_ir1_gpr_em(&tmp, opnd0);
        latxs_ra_free_temp(&tmp);
    }

    return true;
}

static bool latxs_translate_mov_byhand_gpr_mem(
        IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    int os = ir1_opnd_size(opnd0);

    if (os == 32) {
        int gpr_num = ir1_opnd_base_reg_num(opnd0);
        IR2_OPND gpr = latxs_ra_alloc_gpr(gpr_num);
        latxs_load_ir1_mem_to_ir2(&gpr, opnd1, EXMode_S, -1);
        latxs_td_set_reg_extmb(gpr_num, EXMode_S, 32);
    } else {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_load_ir1_mem_to_ir2(&tmp, opnd1, EXMode_Z, -1);
        latxs_store_ir2_to_ir1_gpr_em(&tmp, opnd0);
    }

    return true;
}

static bool latxs_translate_mov_byhand_mem_gpr(
        IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    return latxs_translate_mov_byhand_normal(opnd0, opnd1);
}

static bool latxs_translate_mov_byhand_mem_imm(
        IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    return latxs_translate_mov_byhand_normal(opnd0, opnd1);
}

bool latxs_translate_mov_byhand(IR1_INST *pir1)
{
    /*
     *  MOV |  dest |  src
     * -----+-------+--------
     *      |  GPR  |  GPR
     *      |  MEM  |  GPR
     *      |  GPR  |  MEM
     * -----+-------+--------
     *      |  r/m  |  seg
     *      |  seg  |  r/m
     * -----+-------+--------
     *      |  GPR  |  imm
     *      |  MEM  |  imm
     * -----+-------+--------
     */

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_gpr(opnd0)) {
        if (ir1_opnd_is_gpr(opnd1)) {
            return latxs_translate_mov_byhand_gpr_gpr(opnd0, opnd1);
        } else if (ir1_opnd_is_mem(opnd1)) {
            return latxs_translate_mov_byhand_gpr_mem(opnd0, opnd1);
        } else if (ir1_opnd_is_imm(opnd1)) {
            return latxs_translate_mov_byhand_gpr_imm(opnd0, opnd1);
        }
    } else if (ir1_opnd_is_mem(opnd0)) {
        if (ir1_opnd_is_gpr(opnd1)) {
            return latxs_translate_mov_byhand_mem_gpr(opnd0, opnd1);
        } else if (ir1_opnd_is_imm(opnd1)) {
            return latxs_translate_mov_byhand_mem_imm(opnd0, opnd1);
        }
    }

    return latxs_translate_mov_byhand_normal(opnd0, opnd1);
}

static void tr_movsx_byhand_gpr(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND src = latxs_convert_gpr_opnd(opnd1, EXMode_S);
    if (ir1_opnd_size(opnd0) == 32) {
        IR2_OPND gpr = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        latxs_append_ir2_opnd2_(lisa_mov, &gpr, &src);
        latxs_ir2_opnd_set_emb(&gpr, EXMode_S, 32);
    } else {
        latxs_store_ir2_to_ir1_gpr_em(&src, opnd0);
    }
}

static void tr_movsx_byhand_mem(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    if (ir1_opnd_size(opnd0) == 32) {
        IR2_OPND dest = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        latxs_load_ir1_mem_to_ir2(&dest, opnd1, EXMode_S, -1);
    } else {
        IR2_OPND src = latxs_ra_alloc_itemp();
        latxs_load_ir1_mem_to_ir2(&src, opnd1, EXMode_S, -1);
        latxs_store_ir2_to_ir1_gpr_em(&src, opnd0);
    }
}

bool latxs_translate_movsx_byhand(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_gpr(opnd1)) {
        tr_movsx_byhand_gpr(opnd0, opnd1);
    } else {
        tr_movsx_byhand_mem(opnd0, opnd1);
    }
    return true;
}

static void tr_movzx_byhand_gpr(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND src = latxs_convert_gpr_opnd(opnd1, EXMode_Z);
    if (ir1_opnd_size(opnd0) == 32) {
        IR2_OPND gpr = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        latxs_append_ir2_opnd2_(lisa_mov, &gpr, &src);
        latxs_ir2_opnd_set_emb(&gpr, EXMode_Z, 32);
    } else {
        latxs_store_ir2_to_ir1_gpr_em(&src, opnd0);
    }
}

static void tr_movzx_byhand_mem(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    if (ir1_opnd_size(opnd0) == 32) {
        IR2_OPND dest = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        latxs_load_ir1_mem_to_ir2(&dest, opnd1, EXMode_Z, -1);
    } else {
        IR2_OPND src = latxs_ra_alloc_itemp();
        latxs_load_ir1_mem_to_ir2(&src, opnd1, EXMode_Z, -1);
        latxs_store_ir2_to_ir1_gpr_em(&src, opnd0);
    }
}

bool latxs_translate_movzx_byhand(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_gpr(opnd1)) {
        tr_movzx_byhand_gpr(opnd0, opnd1);
    } else {
        tr_movzx_byhand_mem(opnd0, opnd1);
    }
    return true;
}
