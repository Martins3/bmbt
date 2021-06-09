#ifndef _IR2_H_
#define _IR2_H_

#include "../include/common.h"

/* IR2_INS_TYPE: LISA_ADD_W ...
 * IR2_OPND_TYPE: IR2_OPND_GPR ...
 * IR2_OPND { _type, val } */
#include "../ir2/la_ir2.h"

#define IR2_ITEMP_MAX 0x4000

/* Functions to build IR2_OPND */
IR2_OPND ir2_opnd_new_inv(void);
IR2_OPND ir2_opnd_new(IR2_OPND_TYPE, int value);
void ir2_opnd_build(IR2_OPND*, IR2_OPND_TYPE, int value);
void ir2_opnd_build_mem(IR2_OPND*, int base, int offset);
IR2_OPND ir2_opnd_new_label(void);

/* Functions to access IR2_OPND's fields */
IR2_OPND_TYPE   ir2_opnd_type(IR2_OPND *);
int             ir2_opnd_reg(IR2_OPND *);
int             ir2_opnd_imm(IR2_OPND *);
int             ir2_opnd_label_id(IR2_OPND *);
int             ir2_opnd_addr(IR2_OPND *opnd);
int             ir2_opnd_offset(IR2_OPND *opnd);

/* Functions to modify IR2_OPND_MEM */
IR2_OPND ir2_opnd_mem_get_base(IR2_OPND *mem);
void     ir2_opnd_mem_set_base(IR2_OPND *mem, IR2_OPND *base);
int      ir2_opnd_mem_get_offset(IR2_OPND *);
void     ir2_opnd_mem_adjust_offset(IR2_OPND *mem, int offset);

int ir2_opnd_cmp(IR2_OPND *, IR2_OPND *);

/* Functions to identify IR2_OPND's type/attribute */
int ir2_opnd_is_inv(IR2_OPND *);
int ir2_opnd_is_gpr(IR2_OPND *);
int ir2_opnd_is_fpr(IR2_OPND *);
int ir2_opnd_is_scr(IR2_OPND *);
int ir2_opnd_is_fcsr(IR2_OPND *);
int ir2_opnd_is_cc(IR2_OPND *);
int ir2_opnd_is_imm(IR2_OPND *);
int ir2_opnd_is_immd(IR2_OPND *);
int ir2_opnd_is_immh(IR2_OPND *);
int ir2_opnd_is_label(IR2_OPND *);
int ir2_opnd_is_mem(IR2_OPND *);

int ir2_opnd_is_itemp(IR2_OPND *);
int ir2_opnd_is_ftemp(IR2_OPND *);
int ir2_opnd_is_mem_base_itemp(IR2_OPND *);
int ir2_opnd_is_reg_temp(IR2_OPND *);

int ir2_opnd_to_string(IR2_OPND *, char *, bool);

/* Function used during label disposing */
void ir2_opnd_convert_label_to_imm(IR2_OPND *, int imm);

typedef struct IR2_INST {
    ADDR _addr;
    int _opcode;
    int _id;
    int _prev;
    int _next;
    int op_count;
    IR2_OPND _opnd[4];
} IR2_INST;

/* Fucntion to build IR2_INST */
void ir2_build(IR2_INST*, IR2_OPCODE, IR2_OPND*, IR2_OPND*, IR2_OPND*, IR2_OPND*);
void ir2_build0(IR2_INST*, IR2_OPCODE);
void ir2_build1(IR2_INST*, IR2_OPCODE, IR2_OPND*);
void ir2_build2(IR2_INST*, IR2_OPCODE, IR2_OPND*, IR2_OPND*);
void ir2_build3(IR2_INST*, IR2_OPCODE, IR2_OPND*, IR2_OPND*, IR2_OPND*);
void ir2_build4(IR2_INST*, IR2_OPCODE, IR2_OPND*, IR2_OPND*, IR2_OPND*, IR2_OPND*);

/* Fucntions ot access IR2_INST's fields */
void        ir2_set_id(IR2_INST *, int);
int         ir2_get_id(IR2_INST *);
IR2_OPCODE  ir2_opcode(IR2_INST *);
ADDR        ir2_addr(IR2_INST *);
void        ir2_set_addr(IR2_INST *, ADDR a);
IR2_OPND*   ir2_branch_get_label(IR2_INST*);
int         ir2_branch_label_index(IR2_INST*);

/* Functions to convert IR2_INST to string */
int         ir2_dump(IR2_INST *);
int         ir2_to_string(IR2_INST *, char *);
const char *ir2_name(int value);

/* Functions to manage the linked list of IR2_INST */
IR2_INST*   ir2_allocate(void);
void        ir2_append(IR2_INST *);
void        ir2_remove(IR2_INST *);
void        ir2_insert_before(IR2_INST *ir2, IR2_INST *next);
void        ir2_insert_after(IR2_INST *ir2, IR2_INST *prev);
IR2_INST*   ir2_prev(IR2_INST *);
IR2_INST*   ir2_next(IR2_INST *);
IR2_INST*   ir2_get(int id);

uint32_t ir2_assemble(IR2_INST *);

/* Functions to generate IR2_INST and add into the linked list */
IR2_INST *append_ir2_opnd3i (IR2_OPCODE, IR2_OPND*, IR2_OPND*, IR2_OPND*, int32_t);
IR2_INST *append_ir2_opnd2ii(IR2_OPCODE, IR2_OPND*, IR2_OPND*, int32_t,   int32_t);
IR2_INST *append_ir2_opnd3  (IR2_OPCODE, IR2_OPND*, IR2_OPND*, IR2_OPND*);
IR2_INST *append_ir2_opnd2i (IR2_OPCODE, IR2_OPND*, IR2_OPND*, int32_t);
IR2_INST *append_ir2_opnd2  (IR2_OPCODE, IR2_OPND*, IR2_OPND*);
IR2_INST *append_ir2_opnd1i (IR2_OPCODE, IR2_OPND*, int32_t);
IR2_INST *append_ir2_opnd1  (IR2_OPCODE, IR2_OPND*);
IR2_INST *append_ir2_opndi  (IR2_OPCODE, int32_t);
IR2_INST *append_ir2_opnda  (IR2_OPCODE, ADDR);
IR2_INST *append_ir2_opnd0  (IR2_OPCODE);

/* for fake LISA instruction */
IR2_INST *append_ir2_opnd2_ (IR2_OPCODE, IR2_OPND*, IR2_OPND*);
IR2_INST *append_ir2_opnd1_ (IR2_OPCODE, IR2_OPND*);
IR2_INST *append_ir2_opnd0_ (IR2_OPCODE);

bool ir2_op_check(IR2_INST *pir2);

/* Functions to identify IR2_INST's type */
extern bool ir2_opcode_is_load(IR2_OPCODE);
extern bool ir2_opcode_is_store(IR2_OPCODE);
extern bool ir2_opcode_is_branch(IR2_OPCODE);
extern bool ir2_opcode_is_convert(IR2_OPCODE);
extern bool ir2_opcode_is_fcmp(IR2_OPCODE);

/* Global variables */
extern IR2_OPND zero_ir2_opnd;
extern IR2_OPND sp_ir2_opnd;
extern IR2_OPND fp_ir2_opnd;
extern IR2_OPND eflags_ir2_opnd;
extern IR2_OPND env_ir2_opnd;

extern IR2_OPND f32_ir2_opnd;
extern IR2_OPND fcsr_ir2_opnd;
extern IR2_OPND fcc0_ir2_opnd;
extern IR2_OPND fcc1_ir2_opnd;
extern IR2_OPND fcc2_ir2_opnd;
extern IR2_OPND fcc3_ir2_opnd;
extern IR2_OPND fcc4_ir2_opnd;
extern IR2_OPND fcc5_ir2_opnd;
extern IR2_OPND fcc6_ir2_opnd;
extern IR2_OPND fcc7_ir2_opnd;

/* allocate host helper arguments */
extern IR2_OPND arg0_ir2_opnd;
extern IR2_OPND arg1_ir2_opnd;
extern IR2_OPND arg2_ir2_opnd;
extern IR2_OPND arg3_ir2_opnd;
extern IR2_OPND arg4_ir2_opnd;
extern IR2_OPND arg5_ir2_opnd;
extern IR2_OPND arg6_ir2_opnd;
extern IR2_OPND arg7_ir2_opnd;
extern IR2_OPND ret0_ir2_opnd;
extern IR2_OPND ret1_ir2_opnd;
extern IR2_OPND ra_ir2_opnd;

/* static temp registers */
extern IR2_OPND stmp1_ir2_opnd;
extern IR2_OPND stmp2_ir2_opnd;

extern IR2_OPND invalid_ir2_opnd;

#endif
