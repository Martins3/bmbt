#include "../include/common.h"
#include "../include/env.h"
#include "../include/etb.h"
#include "../ir2/ir2.h"
#include "../ir1/ir1.h"
#include "../include/reg_alloc.h"

#include "../include/flag_lbt.h"
#include "../x86tomips-options.h"

#include "../include/ibtc.h"
#include "../include/profile.h"
#include "../include/flag_pattern.h"
#include "../include/shadow_stack.h"

#ifdef CONFIG_SOFTMMU
#  ifdef CONFIG_XTM_PROFILE
#  include "x86tomips-profile-sys.h"
#  endif
#endif

#include <string.h>

/* campi rd, rs     : GPR[rd]   <- lookup(GPR[rs]).index
 * campv rd, rs     : GPR[rd]   <- lookup(GPR[rs]).value
 * camwi rd, rs, rt : idx              <- GPR[rd]
 *                    Table[idx].Key   <- GPR[rs]
 *                    Table[idx].value <- GPR[rt]
 * ramri rd, rs     : GPR[rd]   <- Table[GPR[rs]].value
 */

uint64_t cam_clear_func; /* func() */
ADDR cam_insert_func;
uint64_t cam_clear_key_func; /* func(key) */

ADDR cam_lookup_k2v;
ADDR cam_lookup_k2i;

static
void __attribute__((__constructor__)) x86_to_mips_cam_init(void)
{
    cam_clear_func = 0;
}

int cam_enabled(void)
{
    return option_tb_lookup;
}

void cam_insert(TranslationBlock *tb)
{
    if (!tb) return;
    if (!cam_enabled()) return;

    static int cam_index = 0;

    int64_t index = ((int64_t(*)(uint64_t))cam_lookup_k2i)((uint64_t)tb->pc);

    if (index < 0) {
        ((void(*)(uint64_t, uint64_t, int))cam_insert_func)(
            (uint64_t)(tb->pc), (uint64_t)tb, (int)cam_index++);
    } else {
        ((void(*)(uint64_t, uint64_t, int))cam_insert_func)(
            (uint64_t)(tb->pc), (uint64_t)tb, index);
    }
}

static
int generate_native_cam_lookup_k2v(void *code_buffer)
{
    lsassertm(0, "CAM to be implemented in LoongArch.\n");
    int mips_num = 0;
//    tr_init(NULL);
//
//    /* a0: key
//     * v0: value */
//    append_ir2_opnd2(mips_campv, &arg0_ir2_opnd, &ret_ir2_opnd);
//
//    IR2_OPND ra = ir2_opnd_new(IR2_OPND_IREG, 31);
//    append_ir2_opnd1(mips_jr, &ra);
//
//    mips_num = tr_ir2_assemble(code_buffer);
//    tr_fini(false);
    return mips_num;
}

static
int generate_native_cam_lookup_k2i(void *code_buffer)
{
    lsassertm(0, "CAM to be implemented in LoongArch.\n");
    int mips_num = 0;
//    tr_init(NULL);
//
//    /* a0: key
//     * v0: index */
//    append_ir2_opnd2(mips_campi, &arg0_ir2_opnd, &ret_ir2_opnd);
//
//    IR2_OPND ra = ir2_opnd_new(IR2_OPND_IREG, 31);
//    append_ir2_opnd1(mips_jr, &ra);
//
//    mips_num = tr_ir2_assemble(code_buffer);
//    tr_fini(false);
    return mips_num;
}

static
int generate_native_cam_insert(void *code_buffer)
{
    lsassertm(0, "CAM to be implemented in LoongArch.\n");
    int mips_num = 0;
//    tr_init(NULL);
//
//    /* a0: key:   TB's PC
//     * a1: value: TB's tc.ptr
//     * a2: index */
//
//    append_ir2_opnd3(mips_camwi,
//            &arg0_ir2_opnd,
//            &arg1_ir2_opnd,
//            &arg2_ir2_opnd);
//
//    IR2_OPND ra = ir2_opnd_new(IR2_OPND_IREG, 31);
//    append_ir2_opnd1(mips_jr, &ra);
//
//    mips_num = tr_ir2_assemble(code_buffer);
//    tr_fini(false);
    return mips_num;
}

static
int generate_native_cam_clear_key(void *code_buffer)
{
    lsassertm(0, "CAM to be implemented in LoongArch.\n");
    int mips_num = 0;
//    tr_init(NULL);
//
//    /* a0: key = TB's PC */
//    IR2_OPND *tmp1 = &stmp1_ir2_opnd;
//    IR2_OPND *zero = &zero_ir2_opnd;
//
//    IR2_OPND cam_lookup_miss = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2i(mips_ori, &ret_ir2_opnd, &zero_ir2_opnd, 0);
//
//    append_ir2_opnd2(mips_campi, &arg0_ir2_opnd, tmp1);
//    append_ir2_opnd2(mips_bltz, tmp1, &cam_lookup_miss);
//
//    append_ir2_opnd3(mips_camwi, zero, zero, tmp1);
//    append_ir2_opnd2i(mips_ori, &ret_ir2_opnd, &zero_ir2_opnd, 1);
//
//    append_ir2_opnd1(mips_label, &cam_lookup_miss);
//
//    IR2_OPND ra = ir2_opnd_new(IR2_OPND_IREG, 31);
//    append_ir2_opnd1(mips_jr, &ra);
//
//    mips_num = tr_ir2_assemble(code_buffer);
//    tr_fini(false);
    return mips_num;
}

static
int generate_native_cam_clear(void *code_buffer)
{
    lsassertm(0, "CAM to be implemented in LoongArch.\n");
    int mips_num = 0;
//    tr_init(NULL);
//
//    IR2_OPND *tmp1 = &stmp1_ir2_opnd;
//    IR2_OPND *tmp2 = &stmp2_ir2_opnd;
//    IR2_OPND *zero = &zero_ir2_opnd;
//
//    append_ir2_opnd2i(mips_ori, tmp1, zero, 0);
//    append_ir2_opnd2i(mips_ori, tmp2, zero, 64);
//
//    IR2_OPND loop = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_label, &loop);
//
//    append_ir2_opnd3(mips_camwi, zero, zero, tmp1);
//    append_ir2_opnd2i(mips_daddiu, tmp1, tmp1, 0x1);
//    append_ir2_opnd3(mips_bne, tmp1, tmp2, &loop);
//
//    IR2_OPND ra = ir2_opnd_new(IR2_OPND_IREG, 31);
//    append_ir2_opnd1(mips_jr, &ra);
//
//    mips_num = tr_ir2_assemble(code_buffer);
//    tr_fini(false);
    return mips_num;
}

int generate_native_cam(void *code_buffer)
{
    lsassertm(0, "CAM to be implemented in LoongArch.\n");
    void *code_buf = code_buffer;
    int mips_num = 0;

    cam_clear_func = (ADDR)code_buf;
    mips_num += generate_native_cam_clear(code_buf);
    code_buf = code_buffer + (mips_num << 2);

    cam_insert_func = (ADDR)code_buf;
    mips_num += generate_native_cam_insert(code_buf);
    code_buf = code_buffer + (mips_num << 2);

    cam_clear_key_func = (ADDR) code_buf;
    mips_num += generate_native_cam_clear_key(code_buf);
    code_buf = code_buffer + (mips_num << 2);

    cam_lookup_k2v = (ADDR)code_buf;
    mips_num += generate_native_cam_lookup_k2v(code_buf);
    code_buf = code_buffer + (mips_num << 2);

    cam_lookup_k2i = (ADDR)code_buf;
    mips_num += generate_native_cam_lookup_k2i(code_buf);
    code_buf = code_buffer + (mips_num << 2);

    ((void(*)(void))cam_clear_func)();

    return mips_num << 2;
}
