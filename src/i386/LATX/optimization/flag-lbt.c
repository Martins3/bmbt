#include "env.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "flag-lbt.h"

bool generate_eflag_by_lbt(IR2_OPND dest, IR2_OPND src0, IR2_OPND src1, IR1_INST *pir1, bool is_sx) {
    switch(ir1_opcode(pir1)) {
        case X86_INS_XADD:
        case X86_INS_ADD: {
            GENERATE_EFLAG_IR2_2(LISA_X86ADD_B);
            break;
        }
        case X86_INS_ADC: {
            GENERATE_EFLAG_IR2_2(LISA_X86ADC_B);
            break;
        }
        case X86_INS_INC: {
            GENERATE_EFLAG_IR2_1(LISA_X86INC_B);
            break;
        }
        case X86_INS_DEC: {
            GENERATE_EFLAG_IR2_1(LISA_X86DEC_B);
            break;
        }
        case X86_INS_CMPSB:
        case X86_INS_CMPSW:
        case X86_INS_CMPSD:
        case X86_INS_SCASB:
        case X86_INS_SCASW:
        case X86_INS_SCASD:
        case X86_INS_CMPXCHG:
        case X86_INS_CMPXCHG8B:
        case X86_INS_NEG:
        case X86_INS_CMP:
        case X86_INS_SUB: {
            GENERATE_EFLAG_IR2_2(LISA_X86SUB_B);
            break;
        }
        case X86_INS_SBB: {
            GENERATE_EFLAG_IR2_2(LISA_X86SBC_B);
            break;
        }
        case X86_INS_XOR: {
            GENERATE_EFLAG_IR2_2(LISA_X86XOR_B);
            break;
        }
        case X86_INS_TEST:
        case X86_INS_AND: {
            GENERATE_EFLAG_IR2_2(LISA_X86AND_B);
            break;
        }
        case X86_INS_OR: {
            GENERATE_EFLAG_IR2_2(LISA_X86OR_B);
            break;
        }
        case X86_INS_SAL:
        case X86_INS_SHL: {
            GENERATE_EFLAG_IR2_2_R(LISA_X86SLL_B);
            break;
        }
        case X86_INS_SHR: {
            GENERATE_EFLAG_IR2_2_R(LISA_X86SRL_B);
            break;
        }
        case X86_INS_SAR: {
            GENERATE_EFLAG_IR2_2_R(LISA_X86SRA_B);
            break;
        }
        case X86_INS_RCL: {
            GENERATE_EFLAG_IR2_2_R(LISA_X86ROTL_B);
            break;
        }
        case X86_INS_RCR: {
            GENERATE_EFLAG_IR2_2_R(LISA_X86RCR_B);
            break;
        }
        case X86_INS_MUL: {
            /*
             * 3A500 has new MUL insn which could leverage directly.
             */
            GENERATE_EFLAG_IR2_2_R(LISA_X86MUL_BU);
            break;
        }
        case X86_INS_IMUL: {
            GENERATE_EFLAG_IR2_2_R(LISA_X86MUL_B);
            break;
        }
	case X86_INS_ROR:
	case X86_INS_ROL: {
	    break;
	}
	case X86_INS_AAM:
        case X86_INS_AAD:
	case X86_INS_AAA:
	case X86_INS_DAA:
	case X86_INS_DAS:
	case X86_INS_SHLD:
	case X86_INS_SHRD: {
	    return false;
        }
        default:
           lsassertm(0, "%s is not implemented in %s\n", pir1->info->mnemonic, __FUNCTION__);
           return false;
    }
    return true;
}

void get_eflag_condition(IR2_OPND *cond, IR1_INST *pir1) {
    switch(ir1_opcode(pir1)) {
        case X86_INS_SETAE:
        case X86_INS_SETB:
        case X86_INS_RCL:
        case X86_INS_RCR:
        case X86_INS_FCMOVNB:
        case X86_INS_FCMOVB:
        case X86_INS_CMOVAE:
        case X86_INS_CMOVB: {
            la_append_ir2_opnd1i_em(LISA_X86MFFLAG, *cond, 0x1);
            break;
        }
        case X86_INS_SETNP:
        case X86_INS_SETP:
        case X86_INS_FCMOVNU:
        case X86_INS_FCMOVU:
        case X86_INS_CMOVNP:
        case X86_INS_CMOVP: {
            la_append_ir2_opnd1i_em(LISA_X86MFFLAG, *cond, 0x2);
            break;
        }
        case X86_INS_SETNE:
        case X86_INS_SETE:
        case X86_INS_FCMOVNE:
        case X86_INS_FCMOVE:
        case X86_INS_CMOVNE:
        case X86_INS_CMOVE: {
            la_append_ir2_opnd1i_em(LISA_X86MFFLAG, *cond, 0x8);
            break;
        }
        case X86_INS_SETNS:
        case X86_INS_SETS:
        case X86_INS_CMOVNS:
        case X86_INS_CMOVS: {
            la_append_ir2_opnd1i_em(LISA_X86MFFLAG, *cond, 0x10);
            break;
        }
        case X86_INS_SETNO:
        case X86_INS_SETO:
        case X86_INS_CMOVNO:
        case X86_INS_CMOVO: {
            la_append_ir2_opnd1i_em(LISA_X86MFFLAG, *cond, 0x20);
            break;
        }
        case X86_INS_SETA:
        case X86_INS_SETBE:
        case X86_INS_CMOVBE:
        case X86_INS_CMOVA: {
            la_append_ir2_opnd1i_em(LISA_X86MFFLAG, *cond, 0x9);
            break;
        }
        case X86_INS_LOOPE:
        case X86_INS_LOOPNE: {
            la_append_ir2_opnd1i_em(LISA_X86MFFLAG, *cond, 0x8);
            la_append_ir2_opnd2i_em(LISA_SLLI_D, *cond, *cond, 63-ZF_BIT_INDEX);
            la_append_ir2_opnd2i_em(LISA_SRAI_D, *cond, *cond, 63);
            break;
        }
        case X86_INS_FCMOVNBE:
        case X86_INS_FCMOVBE: {
            IR2_OPND eflags_temp = ra_alloc_itemp();
            la_append_ir2_opnd1i_em(LISA_X86MFFLAG, eflags_temp, 0x9);
            la_append_ir2_opnd2i(LISA_SRLI_W, *cond, eflags_temp, 6);
            la_append_ir2_opnd3(LISA_OR, *cond, eflags_temp, *cond);
            la_append_ir2_opnd2i(LISA_ANDI, *cond, *cond, 0x1);
            ra_free_temp(eflags_temp);
            break;
        }
        case X86_INS_SETGE:
        case X86_INS_SETL:
        case X86_INS_CMOVGE:
        case X86_INS_CMOVL: {
            IR2_OPND sf_opnd = ra_alloc_itemp();
            IR2_OPND of_opnd = ra_alloc_itemp();
            la_append_ir2_opnd1i_em(LISA_X86MFFLAG, of_opnd, 0x30);
            la_append_ir2_opnd2i(LISA_SRLI_W, sf_opnd, of_opnd, SF_BIT_INDEX);
            la_append_ir2_opnd2i(LISA_SRLI_W, of_opnd, of_opnd, OF_BIT_INDEX);
            la_append_ir2_opnd3(LISA_XOR, sf_opnd, sf_opnd, of_opnd);
            la_append_ir2_opnd2i(LISA_ANDI, *cond, sf_opnd, 1);
            ra_free_temp(sf_opnd);
            ra_free_temp(of_opnd);
            break;
        }
        case X86_INS_SETG:
        case X86_INS_SETLE:
        case X86_INS_CMOVG:
        case X86_INS_CMOVLE: {
            IR2_OPND of_opnd = ra_alloc_itemp();
            la_append_ir2_opnd1i_em(LISA_X86MFFLAG, of_opnd, 0x20);
            la_append_ir2_opnd2i(LISA_SRLI_W, of_opnd, of_opnd, OF_BIT_INDEX-SF_BIT_INDEX);

            IR2_OPND sf_zf_opnd = ra_alloc_itemp();
            la_append_ir2_opnd1i_em(LISA_X86MFFLAG, sf_zf_opnd, 0x18);
            la_append_ir2_opnd3(LISA_XOR, *cond, of_opnd, sf_zf_opnd);

            ra_free_temp(of_opnd);
            ra_free_temp(sf_zf_opnd);
            break;
        }
        default: {
            lsassertm(0, "%s for %s is not implemented\n", __FUNCTION__,
                      ir1_name(ir1_opcode(pir1)));
        }
    }
}

#ifdef CONFIG_SOFTMMU

bool latxs_generate_eflag_by_lbt(IR2_OPND *dest,
        IR2_OPND *src0, IR2_OPND *src1,
        IR1_INST *pir1, bool is_sx)
{
    switch (ir1_opcode(pir1)) {
    case X86_INS_XADD:
    case X86_INS_ADD: {
        LATXS_GENERATE_EFLAG_IR2_2(LISA_X86ADD_B);
        break;
    }
    case X86_INS_ADC: {
        LATXS_GENERATE_EFLAG_IR2_2(LISA_X86ADC_B);
        break;
    }
    case X86_INS_INC: {
        LATXS_GENERATE_EFLAG_IR2_1(LISA_X86INC_B);
        break;
    }
    case X86_INS_DEC: {
        LATXS_GENERATE_EFLAG_IR2_1(LISA_X86DEC_B);
        break;
    }
    case X86_INS_CMPSB:
    case X86_INS_CMPSW:
    case X86_INS_CMPSD:
    case X86_INS_CMPSQ:
    case X86_INS_SCASB:
    case X86_INS_SCASW:
    case X86_INS_SCASD:
    case X86_INS_SCASQ:
    case X86_INS_CMPXCHG:
    case X86_INS_NEG:
    case X86_INS_CMP:
    case X86_INS_SUB: {
        LATXS_GENERATE_EFLAG_IR2_2(LISA_X86SUB_B);
        break;
    }
    case X86_INS_SBB: {
        LATXS_GENERATE_EFLAG_IR2_2(LISA_X86SBC_B);
        break;
    }
    case X86_INS_XOR: {
        LATXS_GENERATE_EFLAG_IR2_2(LISA_X86XOR_B);
        break;
    }
    case X86_INS_TEST:
    case X86_INS_AND: {
        LATXS_GENERATE_EFLAG_IR2_2(LISA_X86AND_B);
        break;
    }
    case X86_INS_OR: {
        LATXS_GENERATE_EFLAG_IR2_2(LISA_X86OR_B);
        break;
    }
    case X86_INS_SAL:
    case X86_INS_SHL: {
        LATXS_GENERATE_EFLAG_IR2_2(LISA_X86SLL_B);
        break;
    }
    case X86_INS_SHR: {
        LATXS_GENERATE_EFLAG_IR2_2(LISA_X86SRL_B);
        break;
    }
    case X86_INS_SAR: {
        LATXS_GENERATE_EFLAG_IR2_2(LISA_X86SRA_B);
        break;
    }
    case X86_INS_RCL: {
        LATXS_GENERATE_EFLAG_IR2_2(LISA_X86RCL_B);
        break;
    }
    case X86_INS_RCR: {
        LATXS_GENERATE_EFLAG_IR2_2(LISA_X86RCR_B);
        break;
    }
    case X86_INS_MUL: {
        LATXS_GENERATE_EFLAG_IR2_2(LISA_X86MUL_BU);
        break;
    }
    case X86_INS_IMUL: {
        LATXS_GENERATE_EFLAG_IR2_2(LISA_X86MUL_B);
        break;
    }
    case X86_INS_ROR:
    case X86_INS_ROL:
        break;
    case X86_INS_SHLD:
    case X86_INS_SHRD:
         return false;
    default:
       lsassertm(0, "%s is not implemented in %s\n",
               pir1->info->mnemonic, __func__);
        return false;
    }

    return true;
}

void latxs_get_eflag_condition(
        IR2_OPND *cond,
        IR1_INST *pir1)
{
    switch (ir1_opcode(pir1)) {
    case X86_INS_SETAE:
    case X86_INS_SETB:
    case X86_INS_RCL:
    case X86_INS_RCR:
    case X86_INS_FCMOVNB:
    case X86_INS_FCMOVB:
    case X86_INS_CMOVAE:
    case X86_INS_CMOVB: {
        latxs_append_ir2_opnd1i(LISA_X86MFFLAG, cond, 0x1);
        break;
    }
    case X86_INS_SETNP:
    case X86_INS_SETP:
    case X86_INS_FCMOVNU:
    case X86_INS_FCMOVU:
    case X86_INS_CMOVNP:
    case X86_INS_CMOVP: {
        latxs_append_ir2_opnd1i(LISA_X86MFFLAG, cond, 0x2);
        break;
    }
    case X86_INS_SETNE:
    case X86_INS_SETE:
    case X86_INS_FCMOVNE:
    case X86_INS_FCMOVE:
    case X86_INS_CMOVNE:
    case X86_INS_CMOVE: {
        latxs_append_ir2_opnd1i(LISA_X86MFFLAG, cond, 0x8);
        break;
    }
    case X86_INS_SETNS:
    case X86_INS_SETS:
    case X86_INS_CMOVNS:
    case X86_INS_CMOVS: {
        latxs_append_ir2_opnd1i(LISA_X86MFFLAG, cond, 0x10);
        break;
    }
    case X86_INS_SETNO:
    case X86_INS_SETO:
    case X86_INS_CMOVNO:
    case X86_INS_CMOVO: {
        latxs_append_ir2_opnd1i(LISA_X86MFFLAG, cond, 0x20);
        break;
    }
    case X86_INS_SETA:
    case X86_INS_SETBE:
    case X86_INS_CMOVBE:
    case X86_INS_CMOVA: {
        latxs_append_ir2_opnd1i(LISA_X86MFFLAG, cond, 0x9);
        break;
    }
    case X86_INS_LOOPE:
    case X86_INS_LOOPNE: {
        latxs_append_ir2_opnd1i(LISA_X86MFFLAG, cond, 0x8);
        latxs_append_ir2_opnd2i(LISA_SLLI_D, cond, cond, 63 - ZF_BIT_INDEX);
        latxs_append_ir2_opnd2i(LISA_SRAI_D, cond, cond, 63);
        break;
    }
    case X86_INS_FCMOVNBE:
    case X86_INS_FCMOVBE: {
        IR2_OPND eflags_temp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd1i(LISA_X86MFFLAG, &eflags_temp, 0x9);
        latxs_append_ir2_opnd2i(LISA_SRLI_W, cond, &eflags_temp, 6);
        latxs_append_ir2_opnd3(LISA_OR, cond, &eflags_temp, cond);
        latxs_append_ir2_opnd2i(LISA_ANDI, cond, cond, 0x1);
        latxs_ra_free_temp(&eflags_temp);
        break;
    }
    case X86_INS_SETGE:
    case X86_INS_SETL:
    case X86_INS_CMOVGE:
    case X86_INS_CMOVL: {
        IR2_OPND sf_opnd = latxs_ra_alloc_itemp();
        IR2_OPND of_opnd = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd1i(LISA_X86MFFLAG, &of_opnd, 0x30);
        latxs_append_ir2_opnd2ii(LISA_BSTRPICK_W, &sf_opnd,
                &of_opnd, SF_BIT_INDEX, SF_BIT_INDEX);
        latxs_append_ir2_opnd2ii(LISA_BSTRPICK_W, &of_opnd,
                &of_opnd, OF_BIT_INDEX, OF_BIT_INDEX);
        latxs_append_ir2_opnd3(LISA_XOR, cond, &sf_opnd, &of_opnd);
        latxs_ra_free_temp(&sf_opnd);
        latxs_ra_free_temp(&of_opnd);
        break;
    }
    case X86_INS_SETG:
    case X86_INS_SETLE:
    case X86_INS_CMOVG:
    case X86_INS_CMOVLE: {
        IR2_OPND of_opnd = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd1i(LISA_X86MFFLAG, &of_opnd, 0x20);
        latxs_append_ir2_opnd2i(LISA_SRLI_W, &of_opnd,
                &of_opnd, OF_BIT_INDEX - SF_BIT_INDEX);

        IR2_OPND sf_zf_opnd = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd1i(LISA_X86MFFLAG, &sf_zf_opnd, 0x18);
        latxs_append_ir2_opnd3(LISA_XOR, cond, &of_opnd, &sf_zf_opnd);

        latxs_ra_free_temp(&of_opnd);
        latxs_ra_free_temp(&sf_zf_opnd);
        break;
    }
    default:
        lsassertm(0, "%s for %s is not implemented\n",
                __func__, ir1_name(ir1_opcode(pir1)));
        break;
    }
}

#endif
