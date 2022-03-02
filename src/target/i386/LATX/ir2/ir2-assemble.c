#include <assert.h>
#include "ir2.h"
#include "translate.h"
#include "ir2-la-assemble.h"

void set_operand_into_instruction(GM_OPERAND_TYPE operand_type,
                                  IR2_OPND *p_opnd, uint32 *result);

/*
 * FIXME: This is a intial port code, there is no any verification!!!
 */
uint32 ir2_assemble(IR2_INST *ir2){
    GM_LA_OPCODE_FORMAT format = lisa_format_table[ir2->_opcode - LISA_INVALID];
    assert(format.type == ir2->_opcode);
    
    uint32_t ins = format.opcode;
    for (int i = 0; i < 4; i++) {
        GM_OPERAND_TYPE opnd_type = format.opnd[i];
        if (opnd_type == OPD_INVALID)
            break;

        GM_OPERAND_PLACE_RELATION bit_field = bit_field_table[opnd_type];
        assert(opnd_type == bit_field.type);

        int start = bit_field.bit_range_0.start;
        int end = bit_field.bit_range_0.end;
        int bit_len = end - start + 1;
        //FIXME: this is a unoin here.
        int val = ir2->_opnd[i].val;
        int mask = (1 << bit_len) - 1;

        ins |= (val & mask) << start;

        if (bit_field.bit_range_1.start >= 0) {
            val = val >> bit_len;
            start = bit_field.bit_range_1.start;
            end = bit_field.bit_range_1.end;
            bit_len = end - start + 1;
            mask = (1 << bit_len) - 1;
            ins |= (val & mask) << start;
        }
    }

    return ins;
}
bool ir2_op_check(IR2_INST  *ir2){
    /* get the instrution format in LA*/
    if(ir2->_opcode - LISA_INVALID < 0) {return 1;}
    GM_LA_OPCODE_FORMAT format = lisa_format_table[ir2->_opcode - LISA_INVALID];
    assert(format.type == ir2->_opcode);
    /* check every opnd */
    for (int i = 0; i < 4; i++) {
        GM_OPERAND_TYPE opnd_type = format.opnd[i];
        IR2_OPND opnd = ir2->_opnd[i];
        if (opnd_type == OPD_INVALID)
            break;
        GM_LA_OPERAND_PLACE_TYPE place = ir2_type_check_table[opnd_type];
        /*
         * There are three main operand type in LA.
         *
         * 1. Regsiter.
         *    Include GPR, FPR, SCR, FCSR and FCC. The check of them is rigorous.
         * 2. Immediate num.
         *    Include IMMD, IMMH. Either of them can pass the check.
         * 3. Invalid
         *    The operand type is not real invalid, but I don't know how to
         *    define them, please ues these type carefully.
         *    The check will pass, if the type of place is invalid.
         *
         */
        if(place.type == IR2_OPND_INV)
            break;
        else if(place.type == IR2_OPND_IMMH)
            return(opnd._type == IR2_OPND_LABEL ||
                   opnd._type == IR2_OPND_IMMD  ||
                   opnd._type == IR2_OPND_IMMH);
        else
            return(opnd._type == place.type);
    }
    return 1;
}

#ifdef CONFIG_SOFTMMU

uint32_t latxs_ir2_assemble(IR2_INST *pir2)
{
    return ir2_assemble(pir2);
}

static
void latxs_ir2_op_check_imm(IR2_OPND *opnd, GM_OPERAND_TYPE opnd_type)
{
#ifndef TARGET_X86_64
    /* TODO: check imm on 32bit mode */
    return;
#endif
    IR2_OPND_TYPE t = ir2_opnd_type(opnd);

    if (t != IR2_OPND_IMMH && t != IR2_OPND_IMMD) {
        return;
    }

    int32_t imm = latxs_ir2_opnd_imm(opnd);
    switch (opnd_type) {
    case OPD_OFFS:
        lsassertm(int32_in_int16(imm), "IR2 check imm fail OPD_OFFS\n");
        break;
    case OPD_OFFL:
        lsassertm(int32_in_int21(imm), "IR2 check imm fail OPD_OFFL\n");
        break;
    case OPD_OFFLL:
        lsassertm(int32_in_int26(imm), "IR2 check imm fail OPD_OFFLL\n");
        break;
    case OPD_SI5:
        lsassertm(int32_in_int5(imm), "IR2 check imm fail OPD_SI5\n");
        break;
    case OPD_SI8:
        lsassertm(int32_in_int8(imm), "IR2 check imm fail OPD_SI8\n");
        break;
    case OPD_SI9:
        lsassertm(int32_in_int9(imm), "IR2 check imm fail OPD_SI9\n");
        break;
    case OPD_SI10:
        lsassertm(int32_in_int10(imm), "IR2 check imm fail OPD_SI10\n");
        break;
    case OPD_SI11:
        lsassertm(int32_in_int11(imm), "IR2 check imm fail OPD_SI11\n");
        break;
    case OPD_SI12:
        lsassertm(int32_in_int12(imm), "IR2 check imm fail OPD_SI12\n");
        break;
    case OPD_SI14:
        lsassertm(int32_in_int14(imm), "IR2 check imm fail OPD_SI14\n");
        break;
    case OPD_SI16:
        lsassertm(int32_in_int16(imm), "IR2 check imm fail OPD_SI16\n");
        break;
    case OPD_SI20:
        lsassertm(int32_in_int20(imm), "IR2 check imm fail OPD_SI20\n");
        break;
    case OPD_UI1:
        lsassertm(uint32_in_uint1(imm), "IR2 check imm fail OPD_UI1\n");
        break;
    case OPD_UI2:
        lsassertm(uint32_in_uint2(imm), "IR2 check imm fail OPD_UI2\n");
        break;
    case OPD_UI3:
        lsassertm(uint32_in_uint3(imm), "IR2 check imm fail OPD_UI3\n");
        break;
    case OPD_UI4:
        lsassertm(uint32_in_uint4(imm), "IR2 check imm fail OPD_UI4\n");
        break;
    case OPD_UI5H:
        lsassertm(uint32_in_uint5(imm), "IR2 check imm fail OPD_UI5H\n");
        break;
    case OPD_UI5L:
        lsassertm(uint32_in_uint5(imm), "IR2 check imm fail OPD_UI5L\n");
        break;
    case OPD_UI6:
        lsassertm(uint32_in_uint6(imm), "IR2 check imm fail OPD_UI6\n");
        break;
    case OPD_UI7:
        lsassertm(uint32_in_uint7(imm), "IR2 check imm fail OPD_UI7\n");
        break;
    case OPD_UI8:
        lsassertm(uint32_in_uint8(imm), "IR2 check imm fail OPD_UI8\n");
        break;
    case OPD_UI12:
        lsassertm(uint32_in_uint12(imm), "IR2 check imm fail OPD_UI12\n");
        break;
    case OPD_PTR:
        lsassertm(uint32_in_uint3(imm), "IR2 check imm fail OPD_PTR\n");
        break;
    case OPD_LSBD:
        lsassertm(uint32_in_uint6(imm), "IR2 check imm fail OPD_LSBD\n");
        break;
    case OPD_LSBW:
        lsassertm(uint32_in_uint5(imm), "IR2 check imm fail OPD_LSBW\n");
        break;
    case OPD_MSBD:
        lsassertm(uint32_in_uint6(imm), "IR2 check imm fail OPD_MSBD\n");
        break;
    case OPD_MSBW:
        lsassertm(uint32_in_uint5(imm), "IR2 check imm fail OPD_MSBW\n");
        break;

    case OPD_CONDF:
        return;
    case OPD_CONDH:
        return;
    case OPD_CONDL:
        return;

    case OPD_I13:
        lsassertm(int32_in_int13(imm), "IR2 check imm fail OPD_I13\n");
        break;

    default:
        lsassertm(0, "IR2 check imm unsupported type %d\n", opnd_type);
        break;
    }

    (void)imm; /* avoid compile warning */
}

bool latxs_ir2_op_check(IR2_INST *pir2)
{
    IR2_OPCODE ir2_opc = latxs_ir2_opcode(pir2);
    int idx = ir2_opc - LISA_INVALID;

    if (idx < 0) {
        return 1;
    }

    /* get the instrution format in LA */
    GM_LA_OPCODE_FORMAT format = lisa_format_table[idx];
    lsassertm(format.type == ir2_opc,
            "IR2 check type wrong : %s\n",
            latxs_ir2_name(ir2_opc));

    /* check every opnd */
    for (int i = 0; i < 4; i++) {
        GM_OPERAND_TYPE opnd_type = format.opnd[i];
        IR2_OPND opnd = pir2->_opnd[i];

        if (opnd_type == OPD_INVALID) {
            break;
        }

        latxs_ir2_op_check_imm(&opnd, opnd_type);

        GM_LA_OPERAND_PLACE_TYPE place
            = ir2_type_check_table[opnd_type];

        if (place.type == IR2_OPND_INV) {
            break;
        }
        if (place.type == IR2_OPND_IMMH) {
            if (!(opnd._type == IR2_OPND_LABEL || opnd._type == IR2_OPND_IMMD ||
                  opnd._type == IR2_OPND_IMMH)) {
                return 0;
            }
        } else if (opnd._type != place.type) {
            return 0;
        }
    }

    return 1;
}

#endif
