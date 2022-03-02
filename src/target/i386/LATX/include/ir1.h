#ifndef _IR1_H_
#define _IR1_H_

#include "../capStone/include/capstone/capstone.h"
#include "common.h"

extern csh handle;
#ifdef CONFIG_SOFTMMU
extern csh handle16;
#ifdef TARGET_X86_64
extern csh handle64;
#endif
#endif

#define eax_index 0
#define ecx_index 1
#define edx_index 2
#define ebx_index 3
#define esp_index 4
#define ebp_index 5
#define esi_index 6
#define edi_index 7
#ifdef TARGET_X86_64
#define rax_index eax_index
#define rcx_index ecx_index
#define rdx_index edx_index
#define rbx_index ebx_index
#define rsp_index esp_index
#define rbp_index ebp_index
#define rsi_index esi_index
#define rdi_index edi_index
#define r8_index  8
#define r9_index  9
#define r10_index 10
#define r11_index 11
#define r12_index 12
#define r13_index 13
#define r14_index 14
#define r15_index 15
#endif

#define es_index 0
#define cs_index 1
#define ss_index 2
#define ds_index 3
#define fs_index 4
#define gs_index 5

typedef enum {
    IR1_EFLAG_FIRST = 96,
    IR1_EFLAG_CF = IR1_EFLAG_FIRST,
    IR1_EFLAG_PF,
    IR1_EFLAG_AF,
    IR1_EFLAG_ZF,
    IR1_EFLAG_SF,
    IR1_EFLAG_OF,
    IR1_EFLAG_DF,
    IR1_EFLAG_LAST,
} IR1_EFLAG;

typedef struct {
    int8 top_increment;       /* top increment */
    int8 opnd_min_fpr_num[3]; /* [i]: fpu inst has i operands */
} FPU_NEED_MIN_FPR;

#define CF_USEDEF_BIT_INDEX 0
#define PF_USEDEF_BIT_INDEX 1
#define AF_USEDEF_BIT_INDEX 2
#define ZF_USEDEF_BIT_INDEX 3
#define SF_USEDEF_BIT_INDEX 4
#define OF_USEDEF_BIT_INDEX 5
#define DF_USEDEF_BIT_INDEX 6

#define CF_USEDEF_BIT (1 << CF_USEDEF_BIT_INDEX)
#define PF_USEDEF_BIT (1 << PF_USEDEF_BIT_INDEX)
#define AF_USEDEF_BIT (1 << AF_USEDEF_BIT_INDEX)
#define ZF_USEDEF_BIT (1 << ZF_USEDEF_BIT_INDEX)
#define SF_USEDEF_BIT (1 << SF_USEDEF_BIT_INDEX)
#define OF_USEDEF_BIT (1 << OF_USEDEF_BIT_INDEX)
#define DF_USEDEF_BIT (1 << DF_USEDEF_BIT_INDEX)

#define EAX_USEDEF_BIT (1 << eax_index)
#define ECX_USEDEF_BIT (1 << ecx_index)
#define EDX_USEDEF_BIT (1 << edx_index)
#define EBX_USEDEF_BIT (1 << ebx_index)
#define ESP_USEDEF_BIT (1 << esp_index)
#define EBP_USEDEF_BIT (1 << ebp_index)
#define ESI_USEDEF_BIT (1 << esi_index)
#define EDI_USEDEF_BIT (1 << edi_index)

#define FI_MDA (1 << 0)
#define FI_HIGH_ADDR (1 << 1)
#define FI_ZX_ADDR_USED (1 << 2)
#define FI_TU_ENTRY (1 << 3)
#define FI_TB_ENTRY (1 << 4)

typedef x86_insn IR1_OPCODE;
typedef x86_op_type IR1_OPND_TYPE;
typedef cs_x86_op IR1_OPND;
typedef x86_prefix IR1_PREFIX;
typedef x86_insn_group IR1_OPCODE_TYPE;

#define FOREACH_OPND_INIT(name) int name = 0;

#define FOREACH_SRC_OPND(name, pInst, opnd) \
    opnd = ir1_get_src_opnd(pInst, 0);      \
    for (name = 0; opnd != NULL; opnd = ir1_get_src_opnd(pInst, ++name))

#define FOREACH_DEST_OPND(name, pInst, opnd) \
    opnd = ir1_get_dest_opnd(pInst, 0);      \
    for (name = 0; opnd != NULL; opnd = ir1_get_dest_opnd(pInst, ++name))

#define MAX_IR1_NUM_PER_TB 255

typedef struct IR1_INST {
    cs_insn *info;
    uint8_t info_count;
    uint8_t _eflag_use;
    uint8_t _eflag_def;
    uint8_t _native_inst_num;
    uint8_t flags;
} IR1_INST;

extern IR1_OPND al_ir1_opnd;
extern IR1_OPND ah_ir1_opnd;
extern IR1_OPND ax_ir1_opnd;
extern IR1_OPND eax_ir1_opnd;
extern IR1_OPND rax_ir1_opnd;

extern IR1_OPND bl_ir1_opnd;
extern IR1_OPND bh_ir1_opnd;
extern IR1_OPND bx_ir1_opnd;
extern IR1_OPND ebx_ir1_opnd;
extern IR1_OPND rbx_ir1_opnd;

extern IR1_OPND cl_ir1_opnd;
extern IR1_OPND ch_ir1_opnd;
extern IR1_OPND cx_ir1_opnd;
extern IR1_OPND ecx_ir1_opnd;
extern IR1_OPND rcx_ir1_opnd;

extern IR1_OPND dl_ir1_opnd;
extern IR1_OPND dh_ir1_opnd;
extern IR1_OPND dx_ir1_opnd;
extern IR1_OPND edx_ir1_opnd;
extern IR1_OPND rdx_ir1_opnd;

extern IR1_OPND sp_ir1_opnd;
extern IR1_OPND esp_ir1_opnd;
extern IR1_OPND rsp_ir1_opnd;

extern IR1_OPND bp_ir1_opnd;
extern IR1_OPND ebp_ir1_opnd;
extern IR1_OPND rbp_ir1_opnd;

extern IR1_OPND si_ir1_opnd;
extern IR1_OPND esi_ir1_opnd;
extern IR1_OPND rsi_ir1_opnd;

extern IR1_OPND di_ir1_opnd;
extern IR1_OPND edi_ir1_opnd;
extern IR1_OPND rdi_ir1_opnd;

extern IR1_OPND eax_mem8_ir1_opnd;
extern IR1_OPND ecx_mem8_ir1_opnd;
extern IR1_OPND edx_mem8_ir1_opnd;
extern IR1_OPND ebx_mem8_ir1_opnd;
extern IR1_OPND esp_mem8_ir1_opnd;
extern IR1_OPND ebp_mem8_ir1_opnd;
extern IR1_OPND esi_mem8_ir1_opnd;
extern IR1_OPND edi_mem8_ir1_opnd;

extern IR1_OPND eax_mem16_ir1_opnd;
extern IR1_OPND ecx_mem16_ir1_opnd;
extern IR1_OPND edx_mem16_ir1_opnd;
extern IR1_OPND ebx_mem16_ir1_opnd;
extern IR1_OPND esp_mem16_ir1_opnd;
extern IR1_OPND ebp_mem16_ir1_opnd;
extern IR1_OPND esi_mem16_ir1_opnd;
extern IR1_OPND edi_mem16_ir1_opnd;

extern IR1_OPND eax_mem32_ir1_opnd;
extern IR1_OPND ecx_mem32_ir1_opnd;
extern IR1_OPND edx_mem32_ir1_opnd;
extern IR1_OPND ebx_mem32_ir1_opnd;
extern IR1_OPND esp_mem32_ir1_opnd;
extern IR1_OPND ebp_mem32_ir1_opnd;
extern IR1_OPND esi_mem32_ir1_opnd;
extern IR1_OPND edi_mem32_ir1_opnd;

ADDRX ir1_disasm(IR1_INST *ir1, uint8_t *addr, ADDRX t_pc,
                    int ir1_num, void *pir1_base);

// TODO : avx_bcast
void ir1_opnd_build_reg(IR1_OPND *opnd, int size, x86_reg reg);
void ir1_opnd_build_imm(IR1_OPND *opnd, int size, int64_t imm);
void ir1_opnd_build_mem(IR1_OPND *opnd, int size, x86_reg base, int64_t disp);
void ir1_opnd_build_fp(IR1_OPND *, IR1_OPND_TYPE opnd_type, int size,
                       double fp);  // not used current
// not used current
// IR1_OPND ir1_opnd_new(IR1_OPND_TYPE opnd_type, int size, int reg_num, longx
// imm);
void ir1_opnd_check(IR1_OPND *);
IR1_OPND_TYPE ir1_opnd_type(IR1_OPND *opnd);
int ir1_opnd_size(IR1_OPND *opnd);
int ir1_opnd_base_reg_bits_start(IR1_OPND *opnd);
x86_reg ir1_opnd_index_reg(IR1_OPND *opnd);
x86_reg ir1_opnd_base_reg(IR1_OPND *opnd);
int ir1_opnd_scale(IR1_OPND *opnd);
longx ir1_opnd_simm(IR1_OPND *opnd);
ulongx ir1_opnd_uimm(IR1_OPND *opnd);
x86_reg ir1_opnd_seg_reg(IR1_OPND *opnd);
int ir1_opnd_is_imm(IR1_OPND *opnd);
int ir1_opnd_is_8h(IR1_OPND *opnd);
int ir1_opnd_is_8l(IR1_OPND *opnd);
int ir1_opnd_is_16l(IR1_OPND *opnd);
int ir1_opnd_is_gpr(IR1_OPND *opnd);
int ir1_opnd_is_uimm_within_16bit(IR1_OPND *opnd);
int ir1_opnd_is_simm_within_16bit(IR1_OPND *opnd);
int ir1_opnd_is_gpr_used(IR1_OPND *opnd, uint8 gpr_index);
int ir1_opnd_is_mem(IR1_OPND *opnd);
int ir1_opnd_is_fpr(IR1_OPND *opnd);
int ir1_opnd_is_seg(IR1_OPND *opnd);
int ir1_opnd_is_mmx(IR1_OPND *opnd);
int ir1_opnd_is_xmm(IR1_OPND *opnd);
int ir1_opnd_is_ymm(IR1_OPND *opnd);
#ifdef TARGET_X86_64
int ir1_opnd_is_pc_relative(IR1_OPND *opnd);
#endif
int ir1_opnd_has_base(IR1_OPND *opnd);
int ir1_opnd_has_index(IR1_OPND *opnd);
int ir1_opnd_has_seg(IR1_OPND *opnd);
int ir1_opnd_get_seg_index(IR1_OPND *opnd);
IR1_PREFIX ir1_prefix(IR1_INST *ir1);
int ir1_opnd_num(IR1_INST *ir1);
ADDRX ir1_addr(IR1_INST *ir1);
ADDRX ir1_addr_next(IR1_INST *ir1);
ADDRX ir1_target_addr(IR1_INST *ir1);
IR1_OPCODE ir1_opcode(IR1_INST *ir1);

int ir1_is_branch(IR1_INST *ir1);
int ir1_is_jump(IR1_INST *ir1);
int ir1_is_call(IR1_INST *ir1);
int ir1_is_return(IR1_INST *ir1);
int ir1_is_indirect(IR1_INST *ir1);
int ir1_is_syscall(IR1_INST *ir1);
bool ir1_is_tb_ending(IR1_INST *ir1);

bool ir1_is_cf_use(IR1_INST *ir1);
bool ir1_is_pf_use(IR1_INST *ir1);
bool ir1_is_af_use(IR1_INST *ir1);
bool ir1_is_zf_use(IR1_INST *ir1);
bool ir1_is_sf_use(IR1_INST *ir1);
bool ir1_is_of_use(IR1_INST *ir1);

bool ir1_is_cf_def(IR1_INST *ir1);
bool ir1_is_pf_def(IR1_INST *ir1);
bool ir1_is_af_def(IR1_INST *ir1);
bool ir1_is_zf_def(IR1_INST *ir1);
bool ir1_is_sf_def(IR1_INST *ir1);
bool ir1_is_of_def(IR1_INST *ir1);

uint8 ir1_get_eflag_use(IR1_INST *ir1);
uint8 ir1_get_eflag_def(IR1_INST *ir1);
void ir1_set_eflag_use(IR1_INST *, uint8 use);
void ir1_set_eflag_def(IR1_INST *, uint8 def);

IR1_OPND *ir1_get_src_opnd(IR1_INST *, int i);
IR1_OPND *ir1_get_dest_opnd(IR1_INST *, int i);
IR1_OPND *ir1_get_src_opnd_implicit(IR1_INST *, int i);
IR1_OPND *ir1_get_dest_opnd_implicit(IR1_INST *, int i);

void ir1_make_ins_JMP(IR1_INST *ir1, ADDRX addr, int32 off);
void ir1_make_ins_NOP(IR1_INST *ir1, ADDRX addr);
void ir1_make_ins_RET(IR1_INST *ir1, ADDRX addr);
void ir1_make_ins_LIBFUNC(IR1_INST *ir1, ADDRX addr);
int ir1_opnd_index_reg_num(IR1_OPND *opnd);
int ir1_opnd_base_reg_num(IR1_OPND *opnd);

EXTENSION_MODE ir1_opnd_em(IR1_OPND *opnd);
int ir1_opnd_eb(IR1_OPND *opnd);
int ir1_dump(IR1_INST *ir1);
int ir1_opcode_dump(IR1_INST *ir1);
const char * ir1_name(IR1_OPCODE op);
const char * ir1_group_name(x86_insn_group grp);

int ir1_opnd_is_x86_address(IR1_OPND *opnd);
int ir1_opnd_is_sx(IR1_OPND *opnd, int bits);
int ir1_opnd_is_zx(IR1_OPND *opd, int bits);
int ir1_opnd_is_bx(IR1_OPND *opnd, int bits);
int ir1_opnd_is_ax(IR1_OPND *opnd, int bits);

bool ir1_need_calculate_of(IR1_INST *ir1);
bool ir1_need_calculate_cf(IR1_INST *ir1);
bool ir1_need_calculate_pf(IR1_INST *ir1);
bool ir1_need_calculate_af(IR1_INST *ir1);
bool ir1_need_calculate_zf(IR1_INST *ir1);
bool ir1_need_calculate_sf(IR1_INST *ir1);
bool ir1_need_calculate_any_flag(IR1_INST *ir1);

bool ir1_translate(IR1_INST *ir1);

uint8 ir1_get_eflag_inherent_use(IR1_INST *ir1);
uint8 ir1_get_eflag_inherent_def(IR1_INST *ir1);
uint8 ir1_get_eflag_inherent_undef(IR1_INST *ir1);

uint8_t ir1_get_opnd_num(IR1_INST *);
IR1_OPND *ir1_get_opnd(IR1_INST *, int i);

bool ir1_is_indirect_call(IR1_INST *);
bool ir1_is_indirect_jmp(IR1_INST *);
bool ir1_is_prefix_lock(IR1_INST *ir1);

const char *ir1_reg_name(x86_reg reg);

#ifdef CONFIG_SOFTMMU

ADDRX latxs_ir1_disasm(IR1_INST *ir1,
        uint8_t *addr, ADDRX t_pc, int *error, int ir1_num);

void latxs_ir1_opnd_build_full_mem(IR1_OPND *opnd, int size,
        x86_reg seg, x86_reg base, int64_t disp,
        int64_t index, int64_t scale);

uint8_t *latxs_ir1_inst_opbytes(IR1_INST *ir1);

int latxs_ir1_mem_opnd_is_base_imm(IR1_OPND *opnd);

int latxs_ir1_opnd_is_cr(IR1_OPND *opnd);
int latxs_ir1_opnd_is_dr(IR1_OPND *opnd);
int latxs_ir1_opnd_get_cr_num(IR1_OPND *opnd, int *flag);
int latxs_ir1_opnd_get_dr_num(IR1_OPND *opnd);

int latxs_ir1_has_prefix_lock(IR1_INST *ir1);
int latxs_ir1_has_prefix_opsize(IR1_INST *ir1);
int latxs_ir1_has_prefix_addrsize(IR1_INST *ir1);
int latxs_ir1_has_prefix_rep(IR1_INST *ir1);
int latxs_ir1_has_prefix_repe(IR1_INST *ir1);
int latxs_ir1_has_prefix_repne(IR1_INST *ir1);

int latxs_ir1_is_lea(IR1_INST *ir1);
int latxs_ir1_is_string_op(IR1_INST *ir1);
/* EOB instructions in system-mode */
int latxs_ir1_is_eob_in_sys(IR1_INST *ir1);
int latxs_ir1_is_jump_far(IR1_INST *ir1);
int latxs_ir1_is_mov_to_cr(IR1_INST *ir1);
int latxs_ir1_is_mov_from_cr(IR1_INST *ir1);
int latxs_ir1_is_mov_to_dr(IR1_INST *ir1);
int latxs_ir1_is_mov_to_seg(IR1_INST *ir1);
int latxs_ir1_is_mov_to_seg_eob(IR1_INST *ir1);
int latxs_ir1_is_pop_eflags(IR1_INST *ir1);
int latxs_ir1_is_rsm(IR1_INST *ir1);
int latxs_ir1_is_sti(IR1_INST *ir1);
int latxs_ir1_is_nop(IR1_INST *ir1);
int latxs_ir1_is_repz_nop(IR1_INST *ir1);
int latxs_ir1_is_iret(IR1_INST *ir1);
int latxs_ir1_is_lmsw(IR1_INST *ir1);
int latxs_ir1_is_retf(IR1_INST *ir1);
int latxs_ir1_is_clts(IR1_INST *ir1);
int latxs_ir1_is_call_far(IR1_INST *ir1);
int latxs_ir1_is_invlpg(IR1_INST *ir1);
int latxs_ir1_is_pause(IR1_INST *ir1);
int latxs_ir1_is_sysenter(IR1_INST *ir1);
int latxs_ir1_is_sysexit(IR1_INST *ir1);
int latxs_ir1_is_syscall(IR1_INST *ir1);
int latxs_ir1_is_sysret(IR1_INST *ir1);
int latxs_ir1_is_xrstor(IR1_INST *ir1);
int latxs_ir1_is_xsetbv(IR1_INST *ir1);
int latxs_ir1_is_mwait(IR1_INST *ir1);
int latxs_ir1_is_vmrun(IR1_INST *ir1);
int latxs_ir1_is_stgi(IR1_INST *ir1);
/* EOB instructions that might change the state of FPU */
int latxs_ir1_contains_fldenv(IR1_INST *ir1);
int latxs_ir1_is_fninit(IR1_INST *ir1);
int latxs_ir1_is_fnsave(IR1_INST *ir1);

void latxs_fix_up_ir1(IR1_INST *ir1);

void latxs_ir1_free_info(IR1_INST *ir1);

int latxs_ir1_is_illegal(IR1_INST *ir1);

int latxs_ir1_access_mmx(IR1_INST *ir1);

int latxs_ir1_inst_size(IR1_INST *ir1);
int latxs_ir1_addr_size(IR1_INST *ir1); /* Address Size */

int latxs_ir1_data_size(IR1_INST *ir1); /* Operand Size */

#define LATXS_IR1_FLAGS_ILLOP     1
#define LATXS_IR1_FLAGS_GENNOP    2
void latxs_ir1_make_ins_ILLEGAL(IR1_INST *ir1,
        ADDRX addr, int size, int flags);

/* byhand use */
int latxs_ir1_opnd_is_gpr32(IR1_OPND *opnd);
int latxs_ir1_opnd_is_gpr64(IR1_OPND *opnd);
int latxs_ir1_opnd_is_gpr_not_8h(IR1_OPND *opnd);

#endif

#endif
