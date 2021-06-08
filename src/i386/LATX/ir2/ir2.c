#include "common.h"
#include "ir2.h"
#include "ir1/ir1.h"
#include "env.h"
#include "reg_alloc.h"
#include <string.h>

#define REG_ALLOC_DEFINE_IR2(ir2name, ir2type, ir2val) \
IR2_OPND ir2name = {._type = ir2type, .val = ir2val}

#define REG_ALLOC_DEFINE_IR2_GPR(ir2name, ir2val) \
REG_ALLOC_DEFINE_IR2(ir2name, IR2_OPND_GPR, ir2val);

/* Integer Register */
REG_ALLOC_DEFINE_IR2_GPR(zero_ir2_opnd,    0); /* ZERO */
REG_ALLOC_DEFINE_IR2_GPR(sp_ir2_opnd,      3); /* SP */
REG_ALLOC_DEFINE_IR2_GPR(fp_ir2_opnd,     22); /* FP */
REG_ALLOC_DEFINE_IR2_GPR(eflags_ir2_opnd, 22); /* FP */
REG_ALLOC_DEFINE_IR2_GPR(env_ir2_opnd,    23); /* S0 */

/* Floating Point  Register */
REG_ALLOC_DEFINE_IR2(f32_ir2_opnd, IR2_OPND_FPR, 8);
REG_ALLOC_DEFINE_IR2(fcsr_ir2_opnd, IR2_OPND_FCSR, 0);
REG_ALLOC_DEFINE_IR2(fcc0_ir2_opnd, IR2_OPND_CC, 0);
REG_ALLOC_DEFINE_IR2(fcc1_ir2_opnd, IR2_OPND_CC, 1);
REG_ALLOC_DEFINE_IR2(fcc2_ir2_opnd, IR2_OPND_CC, 2);
REG_ALLOC_DEFINE_IR2(fcc3_ir2_opnd, IR2_OPND_CC, 3);
REG_ALLOC_DEFINE_IR2(fcc4_ir2_opnd, IR2_OPND_CC, 4);
REG_ALLOC_DEFINE_IR2(fcc5_ir2_opnd, IR2_OPND_CC, 5);
REG_ALLOC_DEFINE_IR2(fcc6_ir2_opnd, IR2_OPND_CC, 6);
REG_ALLOC_DEFINE_IR2(fcc7_ir2_opnd, IR2_OPND_CC, 7);

/* Helper Arguments Register */
REG_ALLOC_DEFINE_IR2_GPR(arg0_ir2_opnd,  4); /* a0 */
REG_ALLOC_DEFINE_IR2_GPR(arg1_ir2_opnd,  5); /* a1 */
REG_ALLOC_DEFINE_IR2_GPR(arg2_ir2_opnd,  6); /* a2 */
REG_ALLOC_DEFINE_IR2_GPR(arg3_ir2_opnd,  7); /* a3 */
REG_ALLOC_DEFINE_IR2_GPR(arg4_ir2_opnd,  8); /* a4/stmp1      */
REG_ALLOC_DEFINE_IR2_GPR(arg5_ir2_opnd,  9); /* a5/stmp2      */
REG_ALLOC_DEFINE_IR2_GPR(argg_ir2_opnd, 10); /* a6/guest-base */
REG_ALLOC_DEFINE_IR2_GPR(arg7_ir2_opnd, 11); /* a7/eip        */
REG_ALLOC_DEFINE_IR2_GPR(ret0_ir2_opnd,  4); /* v0 */
REG_ALLOC_DEFINE_IR2_GPR(ret1_ir2_opnd,  5); /* v1 */
REG_ALLOC_DEFINE_IR2_GPR(ra_ir2_opnd,    1); /* ra */

/* Static Temp Registers */
REG_ALLOC_DEFINE_IR2_GPR(stmp1_ir2_opnd, 8); /* a4 */
REG_ALLOC_DEFINE_IR2_GPR(stmp2_ir2_opnd, 9); /* a5 */

#include "ir2_la_name.h"

/* Consist value */
REG_ALLOC_DEFINE_IR2(invalid_ir2_opnd, IR2_OPND_INV, 0);

IR2_OPND ir2_opnd_new_inv(void)
{
    IR2_OPND opnd;
    opnd._type = IR2_OPND_INV;
    return opnd;
}

IR2_OPND ir2_opnd_new_label(void)
{
    IR2_OPND opnd;
    opnd._type = IR2_OPND_LABEL;
    opnd.val   = ++(lsenv->tr_data->label_num);
    return opnd;
}

void ir2_opnd_build_mem(IR2_OPND *opnd, int base, int offset)
{
    opnd->_type = IR2_OPND_MEM;
    opnd->val = base;
    opnd->imm = offset;
}

void ir2_opnd_build(IR2_OPND *opnd, IR2_OPND_TYPE t, int value)
{
    opnd->_type = t;
    opnd->val   = value;
}
IR2_OPND ir2_opnd_new(IR2_OPND_TYPE type, int value)
{
    IR2_OPND opnd;
    ir2_opnd_build(&opnd, type, value);
    return opnd;
}

IR2_OPND ir2_opnd_mem_get_base(IR2_OPND *opnd)
{
    lsassert(ir2_opnd_is_mem(opnd));
    IR2_OPND ir2_opnd =
        ir2_opnd_new(IR2_OPND_GPR, ir2_opnd_reg(opnd));
    return ir2_opnd;
}
int ir2_opnd_mem_get_offset(IR2_OPND *opnd)
{
    lsassertm(ir2_opnd_is_mem(opnd),
            "ir2 opnd offset could be used for IR2 OPND MEM only\n");
    return opnd->imm;
}
void ir2_opnd_mem_set_base(IR2_OPND *mem, IR2_OPND *base)
{
    lsassert(ir2_opnd_is_mem(mem));
    mem->val = ir2_opnd_reg(base);
}
void ir2_opnd_mem_adjust_offset(IR2_OPND *mem, int offset)
{
    lsassert(ir2_opnd_is_mem(mem));
    mem->imm += offset;
}

IR2_OPND_TYPE ir2_opnd_type(IR2_OPND *opnd) {return opnd->_type;}
int ir2_opnd_reg(IR2_OPND *opnd) {return opnd->val;}
int ir2_opnd_imm(IR2_OPND *opnd) {return opnd->val;}
int ir2_opnd_label_id(IR2_OPND *opnd) {return opnd->val;}
int ir2_opnd_addr(IR2_OPND *opnd) { return opnd->val; }
int ir2_opnd_offset(IR2_OPND *opnd)
{
    lsassert(ir2_opnd_is_mem(opnd));
    return opnd->imm;
}

int ir2_opnd_is_inv(IR2_OPND *opnd)   { return opnd->_type == IR2_OPND_INV; }
int ir2_opnd_is_gpr(IR2_OPND *opnd)   { return opnd->_type == IR2_OPND_GPR; }
int ir2_opnd_is_fpr(IR2_OPND *opnd)   { return opnd->_type == IR2_OPND_FPR; }
int ir2_opnd_is_scr(IR2_OPND *opnd)   { return opnd->_type == IR2_OPND_SCR; }
int ir2_opnd_is_fcsr(IR2_OPND *opnd)  { return opnd->_type == IR2_OPND_FCSR; }
int ir2_opnd_is_cc(IR2_OPND *opnd)    { return opnd->_type == IR2_OPND_CC; }
int ir2_opnd_is_label(IR2_OPND *opnd) { return opnd->_type == IR2_OPND_LABEL; }
int ir2_opnd_is_mem(IR2_OPND *opnd)   { return opnd->_type == IR2_OPND_MEM; }
int ir2_opnd_is_immd(IR2_OPND *opnd)  { return opnd->_type == IR2_OPND_IMMD; }
int ir2_opnd_is_immh(IR2_OPND *opnd)  { return opnd->_type == IR2_OPND_IMMH; }
int ir2_opnd_is_imm(IR2_OPND *opnd)   { return opnd->_type == IR2_OPND_IMMD ||
                                               opnd->_type == IR2_OPND_IMMH; }

#ifdef REG_ALLOC_ALG_IMM
int ir2_opnd_is_itemp(IR2_OPND *opnd)
{
    return ir2_opnd_is_gpr(opnd) && ir2_opnd_is_reg_temp(opnd);
}
int ir2_opnd_is_ftemp(IR2_OPND *opnd)
{
    return ir2_opnd_is_fpr(opnd) && ir2_opnd_is_reg_temp(opnd);
}
int ir2_opnd_is_mem_base_itemp(IR2_OPND *opnd)
{
    return ir2_opnd_is_mem(opnd) && ir2_opnd_is_reg_temp(opnd);
}
int ir2_opnd_is_reg_temp(IR2_OPND *opnd)
{
    int reg;
    switch (ir2_opnd_type(opnd)) {
    case IR2_OPND_MEM:
    case IR2_OPND_GPR: {
        reg = ir2_opnd_reg(opnd);
        return 12 <= reg  && reg <= 19; /* T0 - T7 */
    }
    case IR2_OPND_FPR: {
        reg = ir2_opnd_reg(opnd);
        return 9 <= reg  && reg <= 15; /* F9 - F15 */
    }
    default: lsassert(0);
        break;
    }
}
#else
int ir2_opnd_is_itemp(IR2_OPND *opnd)
{
    return ir2_opnd_is_ireg(opnd) && ir2_opnd_base_reg_num(opnd) > 31;
}

int ir2_opnd_is_ftemp(IR2_OPND *opnd)
{
    return ir2_opnd_is_freg(opnd) && ir2_opnd_base_reg_num(opnd) > 31;
}

int ir2_opnd_is_mem_base_itemp(IR2_OPND *opnd)
{
    return ir2_opnd_is_mem(opnd) && ir2_opnd_base_reg_num(opnd) > 31;
}
#endif


int ir2_opnd_cmp(IR2_OPND *opnd1, IR2_OPND *opnd2)
{
    return opnd1->_type == opnd2->_type &&
           opnd1->val   == opnd2->val;
}

void ir2_opnd_convert_label_to_imm(IR2_OPND *opnd, int imm)
{
    lsassert(ir2_opnd_is_label(opnd));
    opnd->_type = IR2_OPND_IMMH;
    opnd->val = imm;
}

int ir2_opnd_to_string(IR2_OPND *opnd, char *str, bool hex)
{
    int reg_num = ir2_opnd_reg(opnd);

    switch (ir2_opnd_type(opnd)) {
    case IR2_OPND_INV: return 0;
    case IR2_OPND_GPR: {
        strcpy(str, ir2_name(reg_num));
        return strlen(str);
    }
    case IR2_OPND_FPR: {
        strcpy(str, ir2_name(40 + reg_num));
        return strlen(str);
    }
    case IR2_OPND_FCSR: {
        return sprintf(str, "$c%d", reg_num);
    }
    case IR2_OPND_CC: {
        return sprintf(str, "$c%d", reg_num);
    }
    case IR2_OPND_IMMD: {
        return sprintf(str, "%d", ir2_opnd_imm(opnd));
    }
    case IR2_OPND_IMMH: {
        if (hex) {
            return sprintf(str, "0x%x", (uint16_t)ir2_opnd_imm(opnd));
        } else {
            return sprintf(str, "%d", ir2_opnd_imm(opnd));
        }
    }
    case IR2_OPND_LABEL:
        return sprintf(str, "LABEL %d", ir2_opnd_imm(opnd));
    case IR2_OPND_MEM: {
        return sprintf(str, "0x%x(%s)",
                       ir2_opnd_imm(opnd),
                       ir2_name(reg_num));
    }
    default:
        lsassertm(0, "type = %d\n", ir2_opnd_type(opnd));
        return 0;
    }
}

bool ir2_opcode_is_load(IR2_OPCODE opcode)
{
    if (opcode >= LISA_LD_B && opcode <=  LISA_LD_D) {
        return true;
    }
    if (opcode >=  LISA_LD_BU && opcode <=  LISA_LD_WU) {
        return true;
    }
    if (opcode == LISA_LL_W || opcode == LISA_LL_D) {
        return true;
    }
    if (opcode == LISA_LDPTR_W || opcode == LISA_LDPTR_D) {
        return true;
    }
    if (opcode == LISA_FLD_S || opcode == LISA_FLD_D) {
        return true;
    }
    if (opcode == LISA_VLD || opcode == LISA_XVLD){
        return true;
    }
    if (opcode >= LISA_LDL_W && opcode <= LISA_LDR_D){
        return true;
    }
    if (opcode >= LISA_VLDREPL_D && opcode <= LISA_VLDREPL_B){
        return true;
    }
    if (opcode >= LISA_XVLDREPL_D && opcode <= LISA_XVLDREPL_B){
        return true;
    }
    if (opcode == LISA_PRELD) {
        return true;
    }
    return false;
}

bool ir2_opcode_is_store(IR2_OPCODE opcode)
{
    if (opcode >= LISA_ST_B && opcode <= LISA_ST_D) {
        return true;
    }
    if (opcode >= LISA_STL_W && opcode <= LISA_STR_D) {
        return true;
    }
    if (opcode == LISA_SC_D || opcode == LISA_SC_W) {
        return true;
    }
    if (opcode == LISA_STPTR_W || opcode == LISA_STPTR_D) {
        return true;
    }
    if (opcode == LISA_FST_S || opcode == LISA_FST_D) {
        return true;
    }
    if (opcode == LISA_XVST || opcode == LISA_VST){
        return true;
    }
    return false;
}

bool ir2_opcode_is_branch(IR2_OPCODE opcode)
{
    return (opcode >= LISA_BEQZ && opcode <= LISA_BNEZ) ||
           (opcode >= LISA_BCEQZ && opcode <= LISA_BCNEZ) ||
           (opcode >= LISA_B && opcode <= LISA_BGEU);
}

int ir2_branch_label_index(IR2_INST *pir2)
{
    IR2_OPCODE opc = ir2_opcode(pir2);
    lsassert(ir2_opcode_is_branch(opc));

    switch (opc) {
    case LISA_BEQZ:
    case LISA_BNEZ: return 1; break;
    case LISA_B:
    case LISA_BL:   return 0; break;
    case LISA_BEQ:
    case LISA_BNE:
    case LISA_BLT:
    case LISA_BGE:
    case LISA_BLTU:
    case LISA_BGEU: return 2; break;
    case LISA_BCEQZ:
    case LISA_BCNEZ: return 1; break;
    default: return -1; break;
    }

    return -1;
}

IR2_OPND* ir2_branch_get_label(IR2_INST *pir2)
{
    int index = ir2_branch_label_index(pir2);

    if (index >= 0) {
        IR2_OPND* label = &pir2->_opnd[index];
        if (ir2_opnd_is_label(label)) return label;
    }

    return NULL;
}

bool ir2_opcode_is_convert(IR2_OPCODE opcode)
{
    switch (opcode) {
    case LISA_FCVT_D_S:
    case LISA_FCVT_S_D:
    case LISA_FFINT_D_W:
    case LISA_FFINT_D_L:
    case LISA_FFINT_S_W:
    case LISA_FFINT_S_L:
    case LISA_FTINT_L_D:
    case LISA_FTINT_L_S:
    case LISA_FTINT_W_D:
    case LISA_FTINT_W_S:
        return true;
    default:
        return false;
    }
}

bool ir2_opcode_is_fcmp(IR2_OPCODE opcode)
{
    return (opcode == LISA_FCMP_COND_S ||
            opcode == LISA_FCMP_COND_D);
}

void ir2_set_id(IR2_INST *ir2, int id) { ir2->_id = id; }

int ir2_get_id(IR2_INST *ir2) { return ir2->_id; }

IR2_OPCODE ir2_opcode(IR2_INST *ir2) { return (ir2->_opcode); }

ADDR ir2_addr(IR2_INST *ir2) { return ir2->_addr; }

void ir2_set_addr(IR2_INST *ir2, ADDR a) { ir2->_addr = a; }

IR2_INST *ir2_prev(IR2_INST *ir2)
{
    if (ir2->_prev == -1) {
        return NULL;
    } else {
        return lsenv->tr_data->ir2_inst_array + ir2->_prev;
    }
}

IR2_INST *ir2_next(IR2_INST *ir2)
{
    if (ir2->_next == -1) {
        return NULL;
    } else {
        return lsenv->tr_data->ir2_inst_array + ir2->_next;
    }
}

IR2_INST *ir2_get(int id)
{
    lsassert(id >= 0 && id < lsenv->tr_data->ir2_inst_num_current);
    return lsenv->tr_data->ir2_inst_array + id;
}

int ir2_to_string(IR2_INST *ir2, char *str)
{
    int length = 0;
    int i = 0;
    bool hex = false;

    length = sprintf(str, "%-8s  ", ir2_name(ir2_opcode(ir2)));

    if (ir2_opcode(ir2) == LISA_ANDI ||
        ir2_opcode(ir2) == LISA_ORI  ||
        ir2_opcode(ir2) == LISA_XORI ||
        ir2_opcode(ir2) == LISA_LU12I_W) {
        hex = true;
    }

    for (i = 0; i < 4; ++i) {
        IR2_OPND *opnd = &ir2->_opnd[i];
        if (ir2_opnd_type(opnd) == IR2_OPND_INV) {
            return length;
        } else {
            if (i > 0) {
                strcat(str, ",");
                length += 1;
            }
            length += ir2_opnd_to_string(opnd, str + length, hex);
        }
    }

    return length;
}

int ir2_dump(IR2_INST *ir2)
{
    char str[64];
    int size = 0;

    if (ir2_opcode(ir2) == 0) {
        /* an empty IR2_INST was inserted into the ir2
         * list, but not assigned yet. */
        return 0;
    }

    size = ir2_to_string(ir2, str);

    fprintf(stderr, "[%03d] %s\n", ir2->_id, str);

    return size;
}

void ir2_build(IR2_INST *ir2, IR2_OPCODE opcode,
               IR2_OPND *opnd0, IR2_OPND *opnd1,
               IR2_OPND *opnd2, IR2_OPND *opnd3)
{
    ir2->_opcode  = opcode;
    ir2->_opnd[0] = opnd0 ? *opnd0 : invalid_ir2_opnd;
    ir2->_opnd[1] = opnd1 ? *opnd1 : invalid_ir2_opnd;
    ir2->_opnd[2] = opnd2 ? *opnd2 : invalid_ir2_opnd;
    ir2->_opnd[3] = opnd3 ? *opnd3 : invalid_ir2_opnd;
}
void ir2_build0(IR2_INST *pir2, IR2_OPCODE opcode)
{
    ir2_build(pir2, opcode, NULL, NULL, NULL, NULL);
}
void ir2_build1(IR2_INST *pir2, IR2_OPCODE opcode,
                IR2_OPND *opnd0)
{
    ir2_build(pir2, opcode, opnd0, NULL, NULL, NULL);
}
void ir2_build2(IR2_INST *pir2, IR2_OPCODE opcode,
                IR2_OPND *opnd0, IR2_OPND *opnd1)
{
    ir2_build(pir2, opcode, opnd0, opnd1, NULL, NULL);
}
void ir2_build3(IR2_INST *pir2, IR2_OPCODE opcode,
                IR2_OPND *opnd0, IR2_OPND *opnd1,
                IR2_OPND *opnd2)
{
    ir2_build(pir2, opcode, opnd0, opnd1, opnd2, NULL);
}
void ir2_build4(IR2_INST *pir2, IR2_OPCODE opcode,
                IR2_OPND *opnd0, IR2_OPND *opnd1,
                IR2_OPND *opnd2, IR2_OPND *opnd3)
{
    ir2_build(pir2, opcode, opnd0, opnd1, opnd2, opnd3);
}

void ir2_append(IR2_INST *ir2)
{
    TRANSLATION_DATA *t = lsenv->tr_data;
    IR2_INST *former_last = t->last_ir2;

    if (former_last != NULL) {
        lsassert(t->first_ir2 != NULL);
        ir2->_prev = ir2_get_id(former_last);
        ir2->_next = -1;
        t->last_ir2 = ir2;
        former_last->_next = ir2_get_id(ir2);
    } else {
        lsassert(t->first_ir2 == NULL);
        ir2->_prev = -1;
        ir2->_next = -1;
        t->last_ir2 = ir2;
        t->first_ir2 = ir2;
    }

    if(ir2->_opcode >= LISA_GR2SCR)
        t->real_ir2_inst_num++;
}

void ir2_remove(IR2_INST *ir2)
{
    lsassert(0);
    TRANSLATION_DATA *t = lsenv->tr_data;

    IR2_INST *next = ir2_next(ir2);
    IR2_INST *prev = ir2_prev(ir2);

    if (t->first_ir2 == ir2) {
        if (t->last_ir2 == ir2) { /* head and tail */
            t->first_ir2 = NULL;
            t->last_ir2 = NULL;
        } else { /* head but not tail */
            t->first_ir2 = next;
            next->_prev = -1;
        }
    } else if (t->last_ir2 == ir2) { /* tail but not head */
        t->last_ir2 = prev;
        prev->_next = -1;
    } else {
        prev->_next = ir2_get_id(next);
        next->_prev = ir2_get_id(prev);
    }

    ir2->_prev = -1;
    ir2->_next = -1;
}

void ir2_insert_before(IR2_INST *ir2, IR2_INST *next)
{
    TRANSLATION_DATA *t = lsenv->tr_data;

    if (t->first_ir2 == next) {
        t->first_ir2 = ir2;
        ir2->_prev = -1;
        ir2->_next = ir2_get_id(next);
        next->_prev = ir2_get_id(ir2);
    } else {
        IR2_INST *prev = ir2_prev(next);

        ir2->_prev = ir2_get_id(prev);
        prev->_next = ir2_get_id(ir2);

        ir2->_next = ir2_get_id(next);
        next->_prev = ir2_get_id(ir2);
    }
}

void ir2_insert_after(IR2_INST *ir2, IR2_INST *prev)
{
    TRANSLATION_DATA *t = lsenv->tr_data;

    if (t->last_ir2 == prev) {
        t->last_ir2 = ir2;
        ir2->_next = -1;
        ir2->_prev = ir2_get_id(prev);
        prev->_next = ir2_get_id(ir2);
    } else {
        IR2_INST *next = ir2_next(prev);

        ir2->_next = ir2_get_id(next);
        next->_prev = ir2_get_id(ir2);

        ir2->_prev = ir2_get_id(prev);
        prev->_next = ir2_get_id(ir2);
    }
}

IR2_INST *ir2_allocate(void)
{
    TRANSLATION_DATA *t = lsenv->tr_data;

    /* 1. make sure we have enough space */
    if (t->ir2_inst_num_current == t->ir2_inst_num_max) {
        int bytes = sizeof(IR2_INST) * t->ir2_inst_num_max;
        /* double the array */
        t->ir2_inst_num_max *= 2;
        IR2_INST *back_ir2_inst_array = t->ir2_inst_array;
        t->ir2_inst_array = (IR2_INST *)mm_realloc(t->ir2_inst_array, bytes << 1);
        t->first_ir2 = (IR2_INST *)((ADDR)t->first_ir2 - (ADDR)back_ir2_inst_array +
                                    (ADDR)t->ir2_inst_array);
        t->last_ir2  = (IR2_INST *)((ADDR)t->last_ir2  - (ADDR)back_ir2_inst_array +
                                    (ADDR)t->ir2_inst_array);
    }

    /* 2. allocate one */
    IR2_INST *p = t->ir2_inst_array + t->ir2_inst_num_current;
    ir2_set_id(p, t->ir2_inst_num_current);
    t->ir2_inst_num_current++;

    return p;
}

IR2_INST *append_ir2_opnd3(
        IR2_OPCODE opcode,
        IR2_OPND  *opnd0, IR2_OPND *opnd1, IR2_OPND *opnd2)
{
    IR2_INST *pir2 = ir2_allocate();

    ir2_build3(pir2, opcode, opnd0, opnd1, opnd2);

    lsassertm(ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *append_ir2_opnd2i(
        IR2_OPCODE opcode,
        IR2_OPND  *opnd0, IR2_OPND *opnd1, int32_t imm)
{
    if (opcode == LISA_ANDI || opcode == LISA_ORI ||
        opcode == LISA_XORI || opcode == LISA_LU52I_D)
        lsassertm((unsigned int)(imm) <= 0xfff,
                "append opnd2i IMM too large\n");
    else
        lsassertm(imm >= -2048 && imm <= 2047,
                "append opnd2i IMM too large\n");

    IR2_INST *pir2 = ir2_allocate();

    IR2_OPND imm_opnd = ir2_opnd_new(IR2_OPND_IMMH, imm); 
    ir2_build3(pir2, opcode, opnd0, opnd1, &imm_opnd);

    lsassertm(ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *append_ir2_opnd2(
        IR2_OPCODE opcode,
        IR2_OPND  *opnd0, IR2_OPND *opnd1)
{
    if (ir2_opcode_is_load(opcode) ||
        ir2_opcode_is_store(opcode)) {
        /* opnd1 could be IR2 OPND MEM */
        if (ir2_opnd_type(opnd1) == IR2_OPND_MEM) {
            IR2_OPND base = ir2_opnd_mem_get_base(opnd1);
            int offset = ir2_opnd_mem_get_offset(opnd1);
            return append_ir2_opnd2i(opcode, opnd0, &base, offset);
        }
    }

    IR2_INST *pir2 = ir2_allocate();

    ir2_build2(pir2, opcode, opnd0, opnd1);

    lsassertm(ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *append_ir2_opnd1i(
        IR2_OPCODE opcode,
        IR2_OPND  *opnd0, int32 imm)
{
    IR2_INST *pir2 = ir2_allocate();

    IR2_OPND imm_opnd = ir2_opnd_new(IR2_OPND_IMMH, imm);
    ir2_build2(pir2, opcode, opnd0, &imm_opnd);

    lsassertm(ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *append_ir2_opnd1(
        IR2_OPCODE opcode,
        IR2_OPND  *opnd0)
{
    IR2_INST *pir2 = ir2_allocate();

    ir2_build1(pir2, opcode, opnd0);

    lsassertm(ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *append_ir2_opndi(
        IR2_OPCODE opcode,
        int32_t imm)
{
    IR2_INST *pir2 = ir2_allocate();

    IR2_OPND imm_opnd = ir2_opnd_new(IR2_OPND_IMMH, imm);
    ir2_build1(pir2, opcode, &imm_opnd);

    lsassertm(ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *append_ir2_opnd0(
        IR2_OPCODE opcode)
{
    IR2_INST *pir2 = ir2_allocate();

    ir2_build0(pir2, opcode);

    lsassertm(ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *append_ir2_opnda(
        IR2_OPCODE opcode,
        ADDR addr)
{
    IR2_INST *pir2 = ir2_allocate();

    IR2_OPND imm_opnd = ir2_opnd_new(IR2_OPND_IMMH, addr);
    switch (opcode) {
        case LISA_B:
        case LISA_BL:
        case LISA_X86_INST:
        case LISA_DUP:
            ir2_build1(pir2, opcode, &imm_opnd);
            break;
        default:
            lsassertm(0, "append opnda unsupport LISA instruciton (%d)%s\n",
                    opcode, ir2_name(opcode));
            break;
    }

    lsassertm(ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *append_ir2_opnd2ii(
        IR2_OPCODE opcode,
        IR2_OPND *opnd0, IR2_OPND *opnd1,
        int imm1, int imm2)
{
    IR2_INST *pir2 = ir2_allocate();

    IR2_OPND ir2_opnd2, ir2_opnd3;
    ir2_opnd_build(&ir2_opnd2, IR2_OPND_IMMH, imm1);
    ir2_opnd_build(&ir2_opnd3, IR2_OPND_IMMH, imm2);

    switch (opcode) {
    case LISA_BSTRINS_W:
    case LISA_BSTRINS_D:
    case LISA_BSTRPICK_W:
    case LISA_BSTRPICK_D: {
        ir2_build4(pir2, opcode,
                opnd0, opnd1, &ir2_opnd2, &ir2_opnd3);
        break;
    }
    default:
        lsassertm(0, "append opnd2ii unsupport LISA instruciton (%d)%s\n",
                opcode, ir2_name(opcode));
        break;
    }

    lsassertm(ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *append_ir2_opnd3i(
        IR2_OPCODE opcode,
        IR2_OPND *opnd0, IR2_OPND *opnd1,
        IR2_OPND *opnd2, int imm)
{
    IR2_INST *pir2 = ir2_allocate();

    IR2_OPND imm_opnd = ir2_opnd_new(IR2_OPND_IMMH, imm);
    ir2_build4(pir2, opcode, opnd0, opnd1, opnd2, &imm_opnd);

    lsassertm(ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *append_ir2_opnd2_ (
        IR2_OPCODE opcode,
        IR2_OPND  *opnd0, IR2_OPND *opnd1)
{
    IR2_INST *pir2 = NULL;

    switch (opcode) {
    case lisa_mov:
        pir2 = append_ir2_opnd3(LISA_OR, opnd0, opnd1, &zero_ir2_opnd);
        break;
    case lisa_mov32z:
        pir2 = append_ir2_opnd2ii(LISA_BSTRPICK_D, opnd0, opnd1, 31, 0);
        break;
    case lisa_mov24z:
        pir2 = append_ir2_opnd2ii(LISA_BSTRPICK_D, opnd0, opnd1, 23, 0);
        break;
    case lisa_mov16z:
        pir2 = append_ir2_opnd2ii(LISA_BSTRPICK_D, opnd0, opnd1, 15, 0);
        break;
    case lisa_mov8z:
        pir2 = append_ir2_opnd2ii(LISA_BSTRPICK_D, opnd0, opnd1,  8, 0);
        break;
    case lisa_mov32s:
        pir2 = append_ir2_opnd2ii(LISA_BSTRPICK_W, opnd0, opnd1, 31, 0);
        break;
    case lisa_mov16s:
        pir2 = append_ir2_opnd2(LISA_EXT_W_H, opnd0, opnd1);
        break;
    case lisa_mov8s:
        pir2 = append_ir2_opnd2(LISA_EXT_W_B, opnd0, opnd1);
        break;
    case lisa_not:
        pir2 = append_ir2_opnd3(LISA_NOR, opnd0, opnd1, &zero_ir2_opnd);
        break;
    default:
        lsassertm(0, "append opnd2_ only for fake LISA.\n");
        break;
    }

    return pir2;
}

IR2_INST *append_ir2_opnd1_ (
        IR2_OPCODE opcode,
        IR2_OPND  *opnd0)
{
    IR2_INST *pir2 = NULL;

    switch (opcode) {
    case lisa_call:
        pir2 = append_ir2_opnd2i(LISA_JIRL, &ra_ir2_opnd, opnd0, 0);
        break;
    case lisa_jr:
        pir2 = append_ir2_opnd2i(LISA_JIRL, &zero_ir2_opnd, opnd0, 0);
        break;
    default:
        lsassertm(0, "append opnd1_ only for fake LISA.\n");
        break;
    }

    return pir2;
}

IR2_INST *append_ir2_opnd0_ (IR2_OPCODE opcode)
{
    IR2_INST *pir2 = NULL;

    switch (opcode) {
    case lisa_return:
        pir2 = append_ir2_opnd2i(LISA_JIRL, &zero_ir2_opnd,
                &ra_ir2_opnd, 0);
        break;
    case lisa_nop:
        pir2 = append_ir2_opnd2i(LISA_ANDI, &zero_ir2_opnd,
                &zero_ir2_opnd, 0);
        break;
    default:
        lsassertm(0, "append opnd0_ only for fake LISA.\n");
        break;
    }

    return pir2;
}
