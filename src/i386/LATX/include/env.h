#ifndef _ENV_H_
#define _ENV_H_

#include "../include/common.h"
#include "../include/flag_pattern.h"
#include "../translator/translate.h"
#include "../x86tomips-config.h"
#include "../include/shadow_stack.h"

#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
#include "../x86tomips-profile-sys.h"
#endif

#include "../include/cross-page-check.h"

typedef struct ENV {
    CPUX86State *cpu_state;     /* from QEMU,CPUArchState */
    TRANSLATION_DATA *tr_data;  /* from X86toMIPS */
    FLAG_PATTERN_DATA *fp_data; /* from X86toMIPS */
#ifdef CONFIG_SOFTMMU
    int after_exec_tb_fixed; /* mainly for FPU TOP */
    struct {
        TranslationBlock *tb_unlinked;
    } sigint_data;
    struct {
        uint64_t total_tb_executed;
        int last_tb_cpl;
    } monitor_data;
    struct {
        cpt_t cpt; /* Code Page Table */
    } cpc_data;
#endif
#ifdef CONFIG_XTM_FLAG_INT
    /* Flag to help printing only one line info about that
     * this CPU is handling interrupt or exception. */
    int   trace_is_in_int;
#endif
} ENV;

#ifdef BMBT
extern __thread ENV *lsenv;
#else
extern ENV *lsenv;
#endif

/* Functions to access ENV's attributes */
/* Integer Registers */
int lsenv_offset_of_mips_regs(ENV *lsenv,int i);
int lsenv_offset_of_mips_iregs(ENV *lsenv,int i);
int lsenv_offset_of_gpr(ENV *lsenv, int i);
/* Eflags */
int lsenv_offset_of_eflags(ENV *lsenv);
int lsenv_offset_of_cc_src(ENV *lsenv);
int lsenv_offset_of_cc_op(ENV *lsenv);
/* FPU */
int lsenv_offset_of_top(ENV *lsenv);
int lsenv_offset_of_status_word(ENV *lsenv);
int lsenv_offset_of_control_word(ENV *lsenv);
int lsenv_offset_of_tag_word(ENV *lsenv);
int lsenv_offset_of_fptags(ENV *lsenv);
int lsenv_offset_of_fpr(ENV *lsenv, int i);
int lsenv_offset_of_mxcsr(ENV *lsenv);
int   lsenv_get_top(ENV *lsenv);
void  lsenv_set_top(ENV *lsenv, int new_fpstt);
FPReg lsenv_get_fpregs(ENV *lsenv, int i);
void  lsenv_set_fpregs(ENV *lsenv, int i, FPReg new_value);
int   lsenv_get_fpu_control_word(ENV *lsenv);
/* Virtual Registers */
int lsenv_offset_of_vreg(ENV *lsenv, int i);
int lsenv_offset_of_guest_base(ENV *lsenv);
int lsenv_offset_of_last_executed_tb(ENV *lsenv);
int lsenv_offset_of_next_eip(ENV *lsenv);
int lsenv_offset_of_top_bias(ENV *lsenv);
ADDR lsenv_get_vreg(ENV *lsenv, int i);
void lsenv_set_vreg(ENV *lsenv, int i, ADDR val);
ADDR  lsenv_get_guest_base(ENV *lsenv);
void  lsenv_set_guest_base(ENV *lsenv, ADDR gbase);
ADDR  lsenv_get_last_executed_tb(ENV *lsenv);
void  lsenv_set_last_executed_tb(ENV *lsenv, ADDR tb);
ADDRX lsenv_get_next_eip(ENV *lsenv);
void  lsenv_set_next_eip(ENV *lsenv, ADDRX eip);
int   lsenv_get_top_bias(ENV *lsenv);
void  lsenv_set_top_bias(ENV *lsenv, int top_bias);
int   lsenv_get_fast_cs_mask(ENV *lsenv);
void  lsenv_set_fast_cs_mask(ENV *lsenv, int mask);
/* Segment Registers */
int lsenv_offset_of_seg_base(ENV *lsenv, int i);
int lsenv_offset_of_seg_selector(ENV *lsenv, int i);
int lsenv_offset_of_seg_limit(ENV *lsenv, int i);
int lsenv_offset_of_seg_flags(ENV *lsenv, int i);
int lsenv_offset_of_gdt_base(ENV *lsenv);
int lsenv_offset_of_gdt_limit(ENV *lsenv);
/* SIMD */
int lsenv_offset_of_mmx(ENV *lsenv, int i);
int lsenv_offset_of_mmx_t0(ENV *lsenv);
int lsenv_offset_of_xmm_t0(ENV *lsenv);
int lsenv_offset_of_xmm(ENV *lsenv, int i);
int lsenv_offset_of_ss(ENV *lsenv);
int lsenv_offset_of_fast_cs_mask(ENV *lsenv);
/* Others */
int lsenv_offset_of_eip(ENV *lsenv);
int lsenv_offset_of_df(ENV *lsenv);
int lsenv_offset_of_tr_data(ENV *lsenv);
int lsenv_get_last_executed_tb_top_out(ENV *lsenv);

/* Functions to access ENV's interrupt/exception attributes
 * Mainly in user-mode to simulate the 'int' instruction */
int lsenv_offset_exception_index(ENV *lsenv);
int lsenv_offset_exception_next_eip(ENV *lsenv);
void set_CPUX86State_error_code(ENV *lsenv, int error_code);
void set_CPUX86State_exception_is_int(ENV *lsenv, int exception_is_int);
void set_CPUState_can_do_io(ENV *lsenv, int can_do_io);
void helper_raise_int(void);
void siglongjmp_cpu_jmp_env(void);

/* Functions to access QEMU's data */
uint8_t cpu_read_code_via_qemu(CPUX86State *env, ADDRX pc);
ADDR cpu_get_guest_base(void);

/* Functions to access QEMU's TB */
void *qm_tb_get_jmp_reset_offset(TranslationBlock *tb);
void *qm_tb_get_jmp_target_arg(TranslationBlock *tb);

void x86tomips_fini(void);
/* eflags mask */
#define CF_BIT (1 << 0)
#define PF_BIT (1 << 2)
#define AF_BIT (1 << 4)
#define ZF_BIT (1 << 6)
#define SF_BIT (1 << 7)
#define OF_BIT (1 << 11)
#define DF_BIT (1 << 10)
#define IF_BIT (1 << 9)

#define CF_BIT_INDEX 0
#define PF_BIT_INDEX 2
#define AF_BIT_INDEX 4
#define ZF_BIT_INDEX 6
#define SF_BIT_INDEX 7
#define OF_BIT_INDEX 11
#define DF_BIT_INDEX 10
#define IF_BIT_INDEX 9

/* fcsr */
#define FCSR_OFF_EN_I           0
#define FCSR_OFF_EN_U           1
#define FCSR_OFF_EN_O           2
#define FCSR_OFF_EN_Z           3
#define FCSR_OFF_EN_V           4
#define FCSR_OFF_FLAGS_I        16
#define FCSR_OFF_FLAGS_U        17
#define FCSR_OFF_FLAGS_O        18
#define FCSR_OFF_FLAGS_Z        19
#define FCSR_OFF_FLAGS_V        20
#define FCSR_OFF_CAUSE_I        24
#define FCSR_OFF_CAUSE_U        25
#define FCSR_OFF_CAUSE_O        26
#define FCSR_OFF_CAUSE_Z        27
#define FCSR_OFF_CAUSE_V        28
#define FCSR_ENABLE_SET         0x1f
#define FCSR_ENABLE_CLEAR       (~0x1f)
/* RM */
#define FCSR_OFF_RM             8
#define FCSR_RM_CLEAR           (~(0x3 << FCSR_OFF_RM))
#define FCSR_RM_RD              0x3
#define FCSR_RM_RU              0x2
#define FCSR_RM_RZ              0x1

/* x87 FPU Status Register */
#define X87_SR_OFF_IE           0
#define X87_SR_OFF_DE           1
#define X87_SR_OFF_ZE           2
#define X87_SR_OFF_OE           3
#define X87_SR_OFF_UE           4
#define X87_SR_OFF_PE           5
#define X87_SR_OFF_SF           6
#define X87_SR_OFF_ES           7
#define X87_SR_OFF_C0           8
#define X87_SR_OFF_C1           9
#define X87_SR_OFF_C2           10
#define X87_SR_OFF_C3           14

/* x87 FPU Control Register */
#define X87_CR_OFF_IM           0
#define X87_CR_OFF_DM           1
#define X87_CR_OFF_ZM           2
#define X87_CR_OFF_OM           3
#define X87_CR_OFF_UM           4
#define X87_CR_OFF_PM           5
#define X87_CR_OFF_PC           8
#define X87_CR_OFF_X            12
/* Rounding Control */
#define X87_CR_OFF_RC           10
#define X87_CR_RC_RN            0x0
#define X87_CR_RC_RD            0x1
#define X87_CR_RC_RU            0x2
#define X87_CR_RC_RZ            0x3

#ifdef CONFIG_SOFTMMU
int lsenv_offset_of_idtr_base(ENV *lsenv);
int lsenv_offset_of_idtr_limit(ENV *lsenv);
int lsenv_offset_of_gdtr_base(ENV *lsenv);
int lsenv_offset_of_gdtr_limit(ENV *lsenv);
int lsenv_offset_of_ldtr_selector(ENV *lsenv);
int lsenv_offset_of_tr_selector(ENV *lsenv);
int lsenv_offset_of_cr(ENV *lsenv, int i);
int lsenv_offset_of_hflags(ENV *lsenv);
uint32 lsenv_get_hflags(ENV* lsenv);
ADDRX lsenv_get_eip(ENV *lsenv);
void qm_flush_icache(ADDR s, ADDR e);
void qm_tb_clr_ir1(TranslationBlock *tb);
#ifdef CONFIG_XTM_PROFILE

#define XTM_PF_LSENV_OFFSET_FUNC_DEF(group, name) \
int lsenv_offset_of_pf_data_ ## group ## _ ## name (ENV *lsenv);

XTM_PF_LSENV_OFFSET_FUNC_DEF(jc, is_jmpdr)
XTM_PF_LSENV_OFFSET_FUNC_DEF(jc, is_jmpin)
XTM_PF_LSENV_OFFSET_FUNC_DEF(jc, is_sys_eob)
XTM_PF_LSENV_OFFSET_FUNC_DEF(jc, is_excp)

XTM_PF_LSENV_OFFSET_FUNC_DEF(tbf, is_mov)
XTM_PF_LSENV_OFFSET_FUNC_DEF(tbf, is_pop)

#endif
#endif

#endif
