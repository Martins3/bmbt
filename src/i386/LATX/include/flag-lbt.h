#ifndef _FLAG_LBT_H_
#define _FLAG_LBT_H_

#include "../include/etb.h"
#include "../ir1/ir1.h"
#include "../ir2/ir2.h"

bool generate_eflag_by_lbt(IR2_OPND *dest, IR2_OPND *src0, IR2_OPND *src1, IR1_INST *pir1, bool is_sx);

void get_eflag_condition(IR2_OPND *value, IR1_INST *pir1);

#define GENERATE_EFLAG_IR2_1(ir2_opcode) do{ \
    if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8){ \
        append_ir2_opnd1(ir2_opcode, src0); \
    } \
    else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16){ \
        append_ir2_opnd1(ir2_opcode + 1, src0); \
    } \
    else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32){ \
        append_ir2_opnd1(ir2_opcode + 2, src0); \
    } \
} while(0)

#define GENERATE_EFLAG_IR2_2(ir2_opcode) do{ \
    IR2_OPND *_src1 = src1; \
    if (src1 && ir2_opnd_is_imm(src1)) { \
        lsassertm(0, "LBT eflags accept no imm opnd\n"); \
        /*_src1 = ra_alloc_itemp();*/ \
        /*load_ir1_to_ir2(_src1, ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);*/ \
    } \
    if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8){ \
        append_ir2_opnd2(ir2_opcode, src0, _src1); \
    } \
    else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16){ \
        append_ir2_opnd2(ir2_opcode + 1, src0, _src1); \
    } \
    else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32){ \
        append_ir2_opnd2(ir2_opcode + 2, src0, _src1); \
    } \
    if (ir2_opnd_is_imm(_src1)) { \
        ra_free_temp(_src1); \
    } \
} while(0)

#define GENERATE_EFLAG_IR2_2_R(ir2_opcode) do{ \
    IR2_OPND *_src1 = src1; \
    if (src1 && ir2_opnd_is_imm(src1)) { \
        lsassertm(0, "LBT eflags accept no imm opnd\n"); \
        /*_src1 = ra_alloc_itemp();*/ \
        /*load_ir1_to_ir2(_src1, ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);*/  \
    } \
    if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8){ \
        append_ir2_opnd2(ir2_opcode, _src1, src0); \
    } \
    else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16){ \
        append_ir2_opnd2(ir2_opcode + 1, _src1, src0); \
    } \
    else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32){ \
        append_ir2_opnd2(ir2_opcode + 2, _src1, src0); \
    } \
    if (ir2_opnd_is_imm(_src1)) { \
        ra_free_temp(_src1); \
    } \
} while(0)

#endif
