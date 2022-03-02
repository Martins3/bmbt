#ifndef _IR2_H_
#define _IR2_H_

#include "common.h"
#include "la-ir2.h"

#define IR2_ITEMP_MAX 32

void ir2_opnd_build(IR2_OPND *, IR2_OPND_TYPE, int value);
void ir2_opnd_build2(IR2_OPND *, IR2_OPND_TYPE, int base, int16 offset);
void ir2_opnd_build_none(IR2_OPND *);
void ir2_opnd_build_type(IR2_OPND *, IR2_OPND_TYPE);

IR2_OPND ir2_opnd_new(IR2_OPND_TYPE, int value);
IR2_OPND ir2_opnd_new2(IR2_OPND_TYPE, int base, int16 offset);
IR2_OPND ir2_opnd_new_none(void);
IR2_OPND ir2_opnd_new_type(IR2_OPND_TYPE);

int16 ir2_opnd_imm(IR2_OPND *);
int ir2_opnd_base_reg_num(IR2_OPND *);
IR2_OPND_TYPE ir2_opnd_type(IR2_OPND *);
EXTENSION_MODE ir2_opnd_default_em(IR2_OPND *);
uint32 ir2_opnd_addr(IR2_OPND *);
int ir2_opnd_eb(IR2_OPND *);
EXTENSION_MODE ir2_opnd_em(IR2_OPND *);
int32 ir2_opnd_label_id(IR2_OPND *);

int ir2_opnd_cmp(IR2_OPND *, IR2_OPND *);

int ir2_opnd_is_ireg(IR2_OPND *);
int ir2_opnd_is_freg(IR2_OPND *);
int ir2_opnd_is_creg(IR2_OPND *);
int ir2_opnd_is_itemp(IR2_OPND *);
int ir2_opnd_is_ftemp(IR2_OPND *);
int ir2_opnd_is_mem_base_itemp(IR2_OPND *);
int ir2_opnd_is_mem(IR2_OPND *);
int ir2_opnd_is_imm(IR2_OPND *);
int ir2_opnd_is_label(IR2_OPND *);
int ir2_opnd_is_sx(IR2_OPND *, int bits); /* default bits = 32 */
int ir2_opnd_is_zx(IR2_OPND *, int bits); /* default bits = 32 */
int ir2_opnd_is_bx(IR2_OPND *, int bits); /* default bits = 32 */
int ir2_opnd_is_ax(IR2_OPND *, int bits); /* default bits = 32 */
int ir2_opnd_is_address(IR2_OPND *);
int ir2_opnd_is_x86_address(IR2_OPND *);
int ir2_opnd_is_mips_address(IR2_OPND *);

void ir2_opnd_set_em(IR2_OPND *, EXTENSION_MODE,
                     int bits); /* default bits = 32 */
void ir2_opnd_set_em_add(IR2_OPND *, IR2_OPND *src0, EXTENSION_MODE em1,
                         int eb1);
void ir2_opnd_set_em_add2(IR2_OPND *, IR2_OPND *src0, IR2_OPND *src1);
void ir2_opnd_set_em_sub(IR2_OPND *, IR2_OPND *src0, EXTENSION_MODE em1,
                         int eb1);
void ir2_opnd_set_em_sub2(IR2_OPND *, IR2_OPND *src0, IR2_OPND *src1);
void ir2_opnd_set_em_xor(IR2_OPND *, IR2_OPND *src0, EXTENSION_MODE em1,
                         int eb1);
void ir2_opnd_set_em_xor2(IR2_OPND *, IR2_OPND *src0, IR2_OPND *src1);
void ir2_opnd_set_em_or(IR2_OPND *, IR2_OPND *src0, EXTENSION_MODE em1,
                        int eb1);
void ir2_opnd_set_em_or2(IR2_OPND *, IR2_OPND *src0, IR2_OPND *src1);
void ir2_opnd_set_em_and(IR2_OPND *, IR2_OPND *src0, EXTENSION_MODE em1,
                         int eb1);
void ir2_opnd_set_em_and2(IR2_OPND *, IR2_OPND *src0, IR2_OPND *src1);
void ir2_opnd_set_em_nor(IR2_OPND *, IR2_OPND *src0, EXTENSION_MODE em1,
                         int eb1);
void ir2_opnd_set_em_nor2(IR2_OPND *, IR2_OPND *src0, IR2_OPND *src1);
void ir2_opnd_set_em_dsra(IR2_OPND *, IR2_OPND *src, int shift);
void ir2_opnd_set_em_dsll(IR2_OPND *, IR2_OPND *src, int shift);
void ir2_opnd_set_em_mov(IR2_OPND *, IR2_OPND *src);
void ir2_opnd_set_em_movcc(IR2_OPND *, IR2_OPND *src0, IR2_OPND *src1);
void ir2_opnd_set_em_dsrl(IR2_OPND *, IR2_OPND *src, int shift);
void ir2_opnd_set_em_dmult_g(IR2_OPND *, IR2_OPND *src0, IR2_OPND *src1);
void ir2_opnd_set_em_dmultu_g(IR2_OPND *, IR2_OPND *src0, IR2_OPND *src1);

int ir2_opnd_to_string(IR2_OPND *, char *, bool);

void ir2_opnd_convert_label_to_imm(IR2_OPND *, int imm);

#ifndef CONFIG_SOFTMMU
typedef struct IR2_INST {
    ADDR _addr;
    int16 _opcode;
    int16 _id;
    int16 _prev;
    int16 _next;
    int op_count;
    IR2_OPND _opnd[4]; /*LA has 4 opnds*/
} IR2_INST;
#else
typedef struct IR2_INST {
    ADDR _addr;
    int _opcode;
    int _id;
    int _prev;
    int _next;
    int op_count;
    IR2_OPND _opnd[4];
} IR2_INST;
#endif

void ir2_build(IR2_INST *, IR2_OPCODE, IR2_OPND, IR2_OPND, IR2_OPND);

void ir2_set_id(IR2_INST *, int);
int ir2_get_id(IR2_INST *);
IR2_OPCODE ir2_opcode(IR2_INST *);
void ir2_set_opcode(IR2_INST *ir2, IR2_OPCODE type);
int ir2_dump(IR2_INST *);
int ir2_to_string(IR2_INST *, char *);
IR2_INST *ir2_prev(IR2_INST *);
IR2_INST *ir2_next(IR2_INST *);
ADDR ir2_addr(IR2_INST *);
void ir2_set_addr(IR2_INST *, ADDR a);

void ir2_append(IR2_INST *);
void ir2_remove(IR2_INST *);
void ir2_insert_before(IR2_INST *ir2, IR2_INST *next);
void ir2_insert_after(IR2_INST *ir2, IR2_INST *prev);

uint32 ir2_assemble(IR2_INST *);
bool ir2_op_check(IR2_INST *);


bool ir2_opcode_is_branch(IR2_OPCODE);
bool ir2_opcode_is_branch_with_3opnds(IR2_OPCODE);
bool ir2_opcode_is_f_branch(IR2_OPCODE opcode);
bool ir2_opcode_is_convert(IR2_OPCODE opcode);
bool ir2_opcode_is_fcmp(IR2_OPCODE opcode);

extern IR2_OPND zero_ir2_opnd;
extern IR2_OPND env_ir2_opnd;
extern IR2_OPND sp_ir2_opnd;
extern IR2_OPND n1_ir2_opnd;
extern IR2_OPND f32_ir2_opnd;
extern IR2_OPND fcsr_ir2_opnd;
extern IR2_OPND fp_ir2_opnd;
extern IR2_OPND a0_ir2_opnd;
extern IR2_OPND t5_ir2_opnd;
extern IR2_OPND ra_ir2_opnd;
extern IR2_OPND fcc0_ir2_opnd;
extern IR2_OPND fcc1_ir2_opnd;
extern IR2_OPND fcc2_ir2_opnd;
extern IR2_OPND fcc3_ir2_opnd;
extern IR2_OPND fcc4_ir2_opnd;
extern IR2_OPND fcc5_ir2_opnd;
extern IR2_OPND fcc6_ir2_opnd;
extern IR2_OPND fcc7_ir2_opnd;

IR2_OPND create_ir2_opnd(IR2_OPND_TYPE type, int val);
IR2_INST *la_append_ir2_opnd0(IR2_OPCODE type);
IR2_INST *la_append_ir2_opndi(IR2_OPCODE type, int imm);
IR2_INST *la_append_ir2_opnd1(IR2_OPCODE type, IR2_OPND op0);
IR2_INST *la_append_ir2_opnd1i(IR2_OPCODE type, IR2_OPND op0, int imm);
IR2_INST *la_append_ir2_opnd2(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1);
IR2_INST *la_append_ir2_opnd2i(IR2_OPCODE type, IR2_OPND op0,IR2_OPND op1, int imm);
IR2_INST *la_append_ir2_opnd2ii(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1, int imm0, int imm1);
IR2_INST *la_append_ir2_opnd3(IR2_OPCODE type, IR2_OPND op0,IR2_OPND op1, IR2_OPND op2);
IR2_INST *la_append_ir2_opnd3i(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1, IR2_OPND op2, int imm0);
IR2_INST *la_append_ir2_opnd4(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1, IR2_OPND op2, IR2_OPND op3);

IR2_INST *la_append_ir2_opnd1i_em(IR2_OPCODE opcode, IR2_OPND op0, int imm);
IR2_INST *la_append_ir2_opnd2_em(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1);
IR2_INST *la_append_ir2_opnd2i_em(IR2_OPCODE type, IR2_OPND op0,IR2_OPND op1, int imm);
IR2_INST *la_append_ir2_opnd3_em(IR2_OPCODE type, IR2_OPND op0,IR2_OPND op1, IR2_OPND op2);
IR2_INST *la_append_ir2_opnda(IR2_OPCODE opcode, ADDR addr);
bool la_ir2_opcode_is_store(IR2_OPCODE opcode);
bool la_ir2_opcode_is_load(IR2_OPCODE opcode);

/* from ir2-optimization.c */
void tr_ir2_optimize(void);

#ifdef CONFIG_SOFTMMU

/* Global variables */
extern IR2_OPND latxs_zero_ir2_opnd;
extern IR2_OPND latxs_sp_ir2_opnd;
extern IR2_OPND latxs_fp_ir2_opnd;
extern IR2_OPND latxs_env_ir2_opnd;

extern IR2_OPND latxs_f32_ir2_opnd;
extern IR2_OPND latxs_fcsr_ir2_opnd;
extern IR2_OPND latxs_fcsr1_ir2_opnd;
extern IR2_OPND latxs_fcsr2_ir2_opnd;
extern IR2_OPND latxs_fcsr3_ir2_opnd;
extern IR2_OPND latxs_fcc0_ir2_opnd;
extern IR2_OPND latxs_fcc1_ir2_opnd;
extern IR2_OPND latxs_fcc2_ir2_opnd;
extern IR2_OPND latxs_fcc3_ir2_opnd;
extern IR2_OPND latxs_fcc4_ir2_opnd;
extern IR2_OPND latxs_fcc5_ir2_opnd;
extern IR2_OPND latxs_fcc6_ir2_opnd;
extern IR2_OPND latxs_fcc7_ir2_opnd;

/* host helper  latxs_arguments */
extern IR2_OPND latxs_arg0_ir2_opnd;
extern IR2_OPND latxs_arg1_ir2_opnd;
extern IR2_OPND latxs_arg2_ir2_opnd;
extern IR2_OPND latxs_arg3_ir2_opnd;
extern IR2_OPND latxs_arg4_ir2_opnd;
extern IR2_OPND latxs_arg5_ir2_opnd;
extern IR2_OPND latxs_arg6_ir2_opnd;
extern IR2_OPND latxs_arg7_ir2_opnd;
extern IR2_OPND latxs_ret0_ir2_opnd;
extern IR2_OPND latxs_ret1_ir2_opnd;
extern IR2_OPND latxs_ra_ir2_opnd;

/* static temp  latxs_registers */
extern IR2_OPND latxs_stmp1_ir2_opnd;
extern IR2_OPND latxs_stmp2_ir2_opnd;

extern IR2_OPND latxs_invalid_ir2_opnd;

extern IR2_OPND latxs_scr0_ir2_opnd;
extern IR2_OPND latxs_scr1_ir2_opnd;
extern IR2_OPND latxs_scr2_ir2_opnd;
extern IR2_OPND latxs_scr3_ir2_opnd;

/* Functions to build IR2_OPND */
IR2_OPND latxs_ir2_opnd_new(IR2_OPND_TYPE, int value);
IR2_OPND latxs_ir2_opnd_new_inv(void);
IR2_OPND latxs_ir2_opnd_new_label(void);
void latxs_ir2_opnd_build(IR2_OPND*, IR2_OPND_TYPE, int value);
void latxs_ir2_opnd_build_mem(IR2_OPND*, int base, int offset);

/* Functions to access IR2_OPND's fields */
IR2_OPND_TYPE latxs_ir2_opnd_type(IR2_OPND *);
int           latxs_ir2_opnd_reg(IR2_OPND *);
int           latxs_ir2_opnd_imm(IR2_OPND *);
int           latxs_ir2_opnd_label_id(IR2_OPND *);
int           latxs_ir2_opnd_addr(IR2_OPND *opnd);
int           latxs_ir2_opnd_offset(IR2_OPND *opnd);

/* Functions to modify IR2_OPND_MEM */
IR2_OPND latxs_ir2_opnd_mem_get_base(IR2_OPND *mem);
void     latxs_ir2_opnd_mem_set_base(IR2_OPND *mem, IR2_OPND *base);
int      latxs_ir2_opnd_mem_get_offset(IR2_OPND *);
void     latxs_ir2_opnd_mem_adjust_offset(IR2_OPND *mem, int offset);

/* Compare IR2 OPND */
int latxs_ir2_opnd_cmp(IR2_OPND *, IR2_OPND *);

/* Functions to identify IR2_OPND's type/attribute */
int latxs_ir2_opnd_is_inv(IR2_OPND *);
int latxs_ir2_opnd_is_gpr(IR2_OPND *);
int latxs_ir2_opnd_is_fpr(IR2_OPND *);
int latxs_ir2_opnd_is_scr(IR2_OPND *);
int latxs_ir2_opnd_is_fcsr(IR2_OPND *);
int latxs_ir2_opnd_is_cc(IR2_OPND *);
int latxs_ir2_opnd_is_label(IR2_OPND *);
int latxs_ir2_opnd_is_mem(IR2_OPND *);
int latxs_ir2_opnd_is_immd(IR2_OPND *);
int latxs_ir2_opnd_is_immh(IR2_OPND *);
int latxs_ir2_opnd_is_imm(IR2_OPND *);

int latxs_ir2_opnd_is_itemp(IR2_OPND *);
int latxs_ir2_opnd_is_ftemp(IR2_OPND *);
int latxs_ir2_opnd_is_mem_base_itemp(IR2_OPND *);
int latxs_ir2_opnd_is_reg_temp(IR2_OPND *);

/* Functions to convert IR2_INST to string */
int latxs_ir2_dump(IR2_INST *);
int latxs_ir2_to_string(IR2_INST *, char *);
int latxs_ir2_opnd_to_string(IR2_OPND *, char *, bool);
const char *latxs_ir2_name(int value);

/* Function used during label disposing */
void latxs_ir2_opnd_convert_label_to_imm(IR2_OPND *, int imm);

/* Functions to manage IR2 extension mode */
void latxs_ir2_opnd_set_emb(IR2_OPND *, EXMode, EXBits);
void latxs_ir2_opnd_set_em(IR2_OPND *, EXMode);
void latxs_ir2_opnd_set_eb(IR2_OPND *, EXBits);
EXMode latxs_ir2_opnd_get_em(IR2_OPND *);
EXBits latxs_ir2_opnd_get_eb(IR2_OPND *);

/* ------------------ IR2_INST ------------------ */

/* Fucntion to build IR2_INST */
void latxs_ir2_build(IR2_INST *, IR2_OPCODE, IR2_OPND *, IR2_OPND *,
                                             IR2_OPND *, IR2_OPND *);
void latxs_ir2_build0(IR2_INST *, IR2_OPCODE);
void latxs_ir2_build1(IR2_INST *, IR2_OPCODE, IR2_OPND *);
void latxs_ir2_build2(IR2_INST *, IR2_OPCODE, IR2_OPND *, IR2_OPND *);
void latxs_ir2_build3(IR2_INST *, IR2_OPCODE, IR2_OPND *, IR2_OPND *,
                                              IR2_OPND *);
void latxs_ir2_build4(IR2_INST *, IR2_OPCODE, IR2_OPND *, IR2_OPND *,
                                              IR2_OPND *, IR2_OPND *);

/* Fucntions ot access IR2_INST's fields */
void        latxs_ir2_set_id(IR2_INST *, int);
int         latxs_ir2_get_id(IR2_INST *);
IR2_OPCODE  latxs_ir2_opcode(IR2_INST *);
ADDR        latxs_ir2_addr(IR2_INST *);
void        latxs_ir2_set_addr(IR2_INST *, ADDR a);
IR2_OPND   *latxs_ir2_branch_get_label(IR2_INST *);
int         latxs_ir2_branch_label_index(IR2_INST *);

/* Functions to manage the linked list of IR2_INST */
IR2_INST   *latxs_ir2_allocate(void);
void        latxs_ir2_append(IR2_INST *);
void        latxs_ir2_remove(IR2_INST *);
void        latxs_ir2_insert_before(IR2_INST *ir2, IR2_INST *next);
void        latxs_ir2_insert_after(IR2_INST *ir2, IR2_INST *prev);
IR2_INST   *latxs_ir2_prev(IR2_INST *);
IR2_INST   *latxs_ir2_next(IR2_INST *);
IR2_INST   *latxs_ir2_get(int id);

/* Functions to generate IR2_INST and add into the linked list */
IR2_INST *latxs_append_ir2_opnd4(IR2_OPCODE, IR2_OPND *, IR2_OPND *,
                                               IR2_OPND *, IR2_OPND *);
IR2_INST *latxs_append_ir2_opnd3i(IR2_OPCODE, IR2_OPND *, IR2_OPND *,
                                              IR2_OPND *, int32_t);
IR2_INST *latxs_append_ir2_opnd2ii(IR2_OPCODE, IR2_OPND *, IR2_OPND *,
                                               int32_t,   int32_t);
IR2_INST *latxs_append_ir2_opnd3(IR2_OPCODE, IR2_OPND *, IR2_OPND *,
                                             IR2_OPND*);
IR2_INST *latxs_append_ir2_opnd2i(IR2_OPCODE, IR2_OPND *, IR2_OPND *,
                                              int32_t);
IR2_INST *latxs_append_ir2_opnd2(IR2_OPCODE, IR2_OPND *, IR2_OPND *);
IR2_INST *latxs_append_ir2_opnd1i(IR2_OPCODE, IR2_OPND *, int32_t);
IR2_INST *latxs_append_ir2_opnd1(IR2_OPCODE, IR2_OPND *);
IR2_INST *latxs_append_ir2_opndi(IR2_OPCODE, int32_t);
IR2_INST *latxs_append_ir2_opnda(IR2_OPCODE, ADDR);
IR2_INST *latxs_append_ir2_opnd0(IR2_OPCODE);

/* for fake LISA instruction */
IR2_INST *latxs_append_ir2_opnd2_(IR2_OPCODE, IR2_OPND *, IR2_OPND *);
IR2_INST *latxs_append_ir2_opnd1_(IR2_OPCODE, IR2_OPND *);
IR2_INST *latxs_append_ir2_opnd0_(IR2_OPCODE);

/*
 *  jmp_offset : instruction number
 *  link : whether save pc + c at ra
 */
#define latxs_append_ir2_jmp_far(jmp_offset, link)                             \
    do {                                                                       \
        if (option_large_code_cache) {                                         \
            int64_t upper, lower;                                              \
            upper = (((jmp_offset) + (1 << 15)) >> 16) << 44 >> 44;            \
            lower = (jmp_offset) << 48 >> 48;                                  \
            IR2_OPND tmp = latxs_ra_alloc_itemp();                             \
            latxs_append_ir2_opnd1i(LISA_PCADDU18I, &tmp, upper);              \
            if (link) {                                                        \
                latxs_append_ir2_opnd2i(LISA_JIRL, &latxs_ra_ir2_opnd, &tmp,   \
                                        lower);                                \
            } else {                                                           \
                latxs_append_ir2_opnd2i(LISA_JIRL, &latxs_zero_ir2_opnd, &tmp, \
                                        lower);                                \
            }                                                                  \
            latxs_ra_free_temp(&tmp);                                          \
        } else {                                                               \
            if (link) {                                                        \
                latxs_append_ir2_opnda(LISA_BL, (int32_t)ins_offset);          \
            } else {                                                           \
                latxs_append_ir2_opnda(LISA_B, (int32_t)ins_offset);           \
            }                                                                  \
        }                                                                      \
    } while (0)

/* Functions to identify IR2_INST's type */
bool latxs_ir2_opcode_is_load(IR2_OPCODE);
bool latxs_ir2_opcode_is_store(IR2_OPCODE);
bool latxs_ir2_opcode_is_branch(IR2_OPCODE);
bool latxs_ir2_opcode_is_convert(IR2_OPCODE);
bool latxs_ir2_opcode_is_fcmp(IR2_OPCODE);

uint32_t latxs_ir2_assemble(IR2_INST *pir2);
bool latxs_ir2_op_check(IR2_INST *pir2);

#endif

#endif
