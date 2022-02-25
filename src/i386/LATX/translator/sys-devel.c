#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>

static uint8_t *ir1_in_sys;

int latxs_is_ir1_ok(IR1_INST *pir1)
{
    IR1_OPCODE opc = ir1_opcode(pir1);
    lsassertm(X86_INS_INVALID <= opc && opc < X86_INS_ENDING,
            "unknown ir1 id %d\n", opc);
    return ir1_in_sys[opc];
}

void latxs_register_ir1(IR1_OPCODE opc)
{
    lsassertm(X86_INS_INVALID <= opc && opc < X86_INS_ENDING,
            "unknown ir1 id %d\n", opc);
    ir1_in_sys[opc] = 1;
}

static void __attribute__((__constructor__)) sys_devel_init(void)
{
    int num = (int)X86_INS_ENDING;
    int size = (num + 1) * sizeof(uint8_t);

    ir1_in_sys = mm_malloc(size);

    memset(ir1_in_sys, 0, size);

    latxs_sys_misc_register_ir1();
    latxs_sys_io_register_ir1();
    latxs_sys_arith_register_ir1();
    latxs_sys_branch_register_ir1();
    latxs_sys_logic_register_ir1();
    latxs_sys_mov_register_ir1();
    latxs_sys_eflags_register_ir1();
    latxs_sys_setcc_register_ir1();
    latxs_sys_string_register_ir1();
    latxs_sys_fp_register_ir1();
    latxs_sys_fctrl_register_ir1();
    latxs_sys_farith_register_ir1();
    latxs_sys_fldst_register_ir1();
    latxs_sys_fcmovcc_register_ir1();
    latxs_sys_simd_register_ir1();
}
