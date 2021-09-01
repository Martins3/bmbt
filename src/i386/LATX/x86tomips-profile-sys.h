#ifndef _X86toMIPS_PROFILE_SYS_H_
#define _X86toMIPS_PROFILE_SYS_H_

#include <capstone/capstone.h>

struct ps_entry {
    uintptr_t guest_pc;
    uint64_t  count;
    int dump_flag;
};

typedef struct xtm_profile_sys {
    /* Simple Counter */
    uint64_t num_tb_exec;
    uint64_t num_tb_use_fpu_exec;
    uint64_t num_tb_use_xmm_exec;
    uint64_t num_tb_cpl0_exec;
    uint64_t num_tb_cpl1_exec;
    uint64_t num_tb_cpl2_exec;
    uint64_t num_tb_cpl3_exec;
    uint64_t num_cs_in;  /* context switch bt to native */
    uint64_t num_cs_out; /* context switch native to bt */
    uint64_t num_cs_in_fpu;  /* load FPU */
    uint64_t num_cs_in_xmm;  /* load XMM */
    uint64_t num_helper;
    uint64_t num_helper_allgpr;
    uint64_t num_helper_fpu;
    uint64_t num_helper_xmm;
    uint64_t num_tlb_hit;
    uint64_t num_tlb_miss;
    uint64_t num_fpu_rotate_native;
    uint64_t num_fpu_rotate_bt;
    uint64_t num_excp;
    uint64_t num_fastcs_glue_01;
    uint64_t num_fastcs_glue_10;
    uint64_t num_fastcs_glue_11;
    uint64_t num_staticcs;
    uint64_t num_cpc_hit;
    uint64_t num_cpc_miss;

    /* Jmp Cache Counter */
    uint64_t num_jc_clear;
    uint64_t num_jc_clear_page;

    uint64_t num_jc_lookup;
    uint64_t num_jc_lookup_hit;

#define XTM_PF_JC_TYPE_DEF(type) \
    uint64_t num_jc_lookup_ ## type; \
    uint64_t num_jc_lookup_ ## type ## _hit; \
    uint64_t num_jc_lookup_ ## type ## _in_last_1000; \
    uint64_t num_jc_lookup_ ## type ## _in_last_10000; \
    uint64_t num_jc_lookup_ ## type ## _in_last_100000; \
    uint64_t num_jc_lookup_ ## type ## _in_last_1000_hit; \
    uint64_t num_jc_lookup_ ## type ## _in_last_10000_hit; \
    uint64_t num_jc_lookup_ ## type ## _in_last_100000_hit;

    XTM_PF_JC_TYPE_DEF(jmpdr)
    XTM_PF_JC_TYPE_DEF(jmpin)
    XTM_PF_JC_TYPE_DEF(sys_eob)
    XTM_PF_JC_TYPE_DEF(excp)

    uint64_t num_jc_cam;
    uint64_t num_jc_cam_hit;

    uint64_t num_jc_njc;
    uint64_t num_jc_njc_hit;

    /* TB Hash Table Stat */
    uint64_t num_tbht_lookup;
    uint64_t num_tbht_lookup_hit;
    uint64_t num_tbht_lookup_hash[4];
    uint64_t num_tbht_lookup_hit_hash[4];

    uint64_t num_tbht_lookup_hit_in_last_1000;
    uint64_t num_tbht_lookup_hit_in_last_10000;
    uint64_t num_tbht_lookup_hit_in_last_100000;

    uint64_t num_tbht_lookup_hash_in_last_1000[4];
    uint64_t num_tbht_lookup_hash_in_last_10000[4];
    uint64_t num_tbht_lookup_hash_in_last_100000[4];

    uint64_t num_tbht_lookup_hit_hash_in_last_1000[4];
    uint64_t num_tbht_lookup_hit_hash_in_last_10000[4];
    uint64_t num_tbht_lookup_hit_hash_in_last_100000[4];

    /* Time counter */
    struct {
#define XTM_PF_TIMECOUNTER_DEF(name) \
        uint64_t name ## _ss; \
        uint64_t name ## _ns;
        /* exeuction time counter */
        struct {
            XTM_PF_TIMECOUNTER_DEF(all)
            XTM_PF_TIMECOUNTER_DEF(tb)
        } exe;
        /* translation time counter */
        struct {
            XTM_PF_TIMECOUNTER_DEF(all)
            XTM_PF_TIMECOUNTER_DEF(disasm)
            XTM_PF_TIMECOUNTER_DEF(disasm_readbytes)
            XTM_PF_TIMECOUNTER_DEF(disasm_capstone)
            XTM_PF_TIMECOUNTER_DEF(translate)
            XTM_PF_TIMECOUNTER_DEF(assemble)
        } tr;
    } tc;

    /* Register Stat */
    struct {
        uint64_t ra_alloc_int_nr;
        uint64_t ra_alloc_fp_nr;
        uint64_t ra_free_int_nr;
        uint64_t ra_free_fp_nr;
        int ra_int_used_max;
        int ra_fp_used_max;
    } rs;

    /* Translation Stat */
    struct {
        uint64_t ins_translated;
        uint64_t tb_translated;
    } ts;

    /* Complex Stat */
#define XTM_PF_CS_ENTRY_DEFINE(num) \
    float expand_ratio_ ## num; \
    float expand_ratio_ ## num ## _cpl0; \
    float expand_ratio_ ## num ## _cpl3; \
    uint64_t last_ ## num ## _cpl0_tb_nr; \
    uint64_t last_ ## num ## _cpl3_tb_nr;

    XTM_PF_CS_ENTRY_DEFINE(1000)
    XTM_PF_CS_ENTRY_DEFINE(10000)
    XTM_PF_CS_ENTRY_DEFINE(100000)

    /* Privilege Switch CPL3 to CPL0 */
    struct ps_entry ps30[16];
    int ps30_nr;
#define XTMPF_PS30_MAXNUM 16

    /* TB.flags related */
    struct {
        uint64_t ljmp_pe;
        uint64_t lcall_pe;
        uint64_t ret_pe;
        uint64_t interrupt;
        uint64_t rsm;
        uint64_t pop_ss;
        uint64_t mov_ss;
        uint64_t switch_tss;
        uint64_t sys_enter;
        uint64_t sys_exit;
        uint64_t smm;
        uint64_t cpu_reset;
    } op_count;
    struct {
        uint64_t unchange_nr;
        uint64_t change_nr;
    } tbf_cs32;
    struct {
        uint64_t unchange_nr;
        uint64_t change_nr;
    } tbf_ss32;

} xtm_profile_sys_t ;

void xtm_pf_clear_all(void);

typedef struct xtm_profile_sys_tc {

#define XTM_PF_TIMESPEC_DEF(name) \
    struct timespec name ## _st, name ## _ed;

    /* exeuction time counter */
    struct {
        XTM_PF_TIMESPEC_DEF(all)
        XTM_PF_TIMESPEC_DEF(tb)
    } exe;
    /* translation time counter */
    struct {
        XTM_PF_TIMESPEC_DEF(all)
        XTM_PF_TIMESPEC_DEF(disasm)
        XTM_PF_TIMESPEC_DEF(disasm_readbytes)
        XTM_PF_TIMESPEC_DEF(disasm_capstone)
        XTM_PF_TIMESPEC_DEF(translate)
        XTM_PF_TIMESPEC_DEF(assemble)
    } tr;
} xtm_profile_sys_tc_t;

#define XTM_PF_TIMESPEC_FN_DEF(group, name) \
void xtm_pf_tc_ ## group ## _ ## name ## _st(void); \
void xtm_pf_tc_ ## group ## _ ## name ## _ed(void); \

XTM_PF_TIMESPEC_FN_DEF(exe, all)
XTM_PF_TIMESPEC_FN_DEF(exe, tb)
XTM_PF_TIMESPEC_FN_DEF(tr, all)
XTM_PF_TIMESPEC_FN_DEF(tr, disasm)
XTM_PF_TIMESPEC_FN_DEF(tr, disasm_readbytes)
XTM_PF_TIMESPEC_FN_DEF(tr, disasm_capstone)
XTM_PF_TIMESPEC_FN_DEF(tr, translate)
XTM_PF_TIMESPEC_FN_DEF(tr, assemble)

extern xtm_profile_sys_t xtm_pf_data;

/* tr_xx()  called during translation, works during EXECUTION!
 * xtm_xx() called during translation, works during translation */

/* Function used to generate profile code in native */

void tr_pf_inc_tb_exec(void);           /* [+] tr_sys.c */
void tr_pf_inc_tb_use_fpu_exec(void);   /* [+] tr_sys.c */
void tr_pf_inc_tb_use_xmm_exec(void);   /* [+] tr_sys.c */
void tr_pf_inc_tb_cpl0_exec(void);      /* [+] tr_sys.c */
void tr_pf_inc_tb_cpl1_exec(void);      /* [+] tr_sys.c */
void tr_pf_inc_tb_cpl2_exec(void);      /* [+] tr_sys.c */
void tr_pf_inc_tb_cpl3_exec(void);      /* [+] tr_sys.c */
void tr_pf_inc_cs_in(void);             /* [+] translate.c */
void tr_pf_inc_cs_out(void);            /* [+] translate.c */
void tr_pf_inc_cs_in_fpu(void);         /* [ ] */
void tr_pf_inc_cs_in_xmm(void);         /* [ ] */
void tr_pf_inc_helper(void);            /* [+] tr_sys_helper.c */
void tr_pf_inc_helper_allgpr(void);     /* [+] tr_sys_helper.c */
void tr_pf_inc_helper_fpu(void);        /* [+] tr_sys_helper.c */
void tr_pf_inc_helper_xmm(void);        /* [+] tr_sys_helper.c */
void tr_pf_inc_tlb_hit(void);           /* [+] tr_softmmu.c */
void tr_pf_inc_tlb_miss(void);          /* [+] tr_softmmu.c */
void tr_pf_inc_fpu_rotate_native(void); /* [+] translate.c */
void tr_pf_inc_excp(void);              /* [+] tr_excp.c */
void tr_pf_inc_fastcs_glue_01(void);
void tr_pf_inc_fastcs_glue_10(void);
void tr_pf_inc_fastcs_glue_11(void);
void tr_pf_inc_staticcs(void);
void tr_pf_inc_cpc_hit(void);
void tr_pf_inc_cpc_miss(void);

void tr_pf_inc_jc_cam(void *tmp1, void *tmp2);
void tr_pf_inc_jc_cam_hit(void *tmp1, void *tmp2);
void tr_pf_inc_jc_njc(void *tmp1, void *tmp2);
void tr_pf_inc_jc_njc_hit(void *tmp1, void *tmp2);

void xtm_pf_inc_tb_exec(void);
void xtm_pf_inc_tb_use_fpu_exec(void);
void xtm_pf_inc_tb_use_xmm_exec(void);
void xtm_pf_inc_tb_cpl0_exec(void);
void xtm_pf_inc_tb_cpl1_exec(void);
void xtm_pf_inc_tb_cpl2_exec(void);
void xtm_pf_inc_tb_cpl3_exec(void);
void xtm_pf_inc_cs_in(void);
void xtm_pf_inc_cs_out(void);
void xtm_pf_inc_cs_in_fpu(void);        /* [+] x86tomips-config.c */
void xtm_pf_inc_cs_in_xmm(void);        /* [+] x86tomips-config.c */
void xtm_pf_inc_helper(void);
void xtm_pf_inc_helper_allgpr(void);
void xtm_pf_inc_helper_fpu(void);
void xtm_pf_inc_helper_xmm(void);
void xtm_pf_inc_tlb_hit(void);
void xtm_pf_inc_tlb_miss(void);
void xtm_pf_inc_fpu_rotate_bt(void);    /* [+] translate.c */
void xtm_pf_inc_excp(void);

void xtm_pf_inc_rs_ra_alloc_int_nr(void);
void xtm_pf_inc_rs_ra_alloc_fp_nr(void);
void xtm_pf_inc_rs_ra_free_int_nr(void);
void xtm_pf_inc_rs_ra_free_fp_nr(void);

void xtm_pf_inc_rs_ra_int_used_max(int num);
void xtm_pf_inc_rs_ra_fp_used_max(int num);

void xtm_pf_inc_ts_ins_translated(void);
void xtm_pf_inc_ts_tb_translated(void);

/* Record TB execution trace for complex stat */
void xtm_pf_tb_exec_record(void *tb);

/* DWE: Dump While Execution Support */
int xtm_pf_need_dwe(void);
void tr_pf_dwe(void);
void xtm_pf_dwe_helper(CPUX86State *env);
/* Helper for JC to dump information */
void xtm_pf_jc_dwe_helper(CPUX86State*, uint64_t);
/* Helper for TBHT to dump information */
void xtm_pf_tbht_dwe_helper(CPUX86State*, uint64_t);

/* Profile Jmp Cache */
void xtm_pf_jmp_cache(void *cpu, void *tb);
void xtm_pf_jmp_cache_print(void);
#ifndef _X86toMIPS_PROFILE_SYS_INC_JC_CLEAR_
void xtm_pf_inc_jc_clear(void *cpu);
#endif
void xtm_pf_inc_jc_clear_page(void *cpu, uint64_t page_addr);

/* Profile TB Hash Table */
void xtm_pf_tb_hash_table_print(void);
void xtm_pf_tb_hash_table(void *cpu, void *tb, int hash_cmp_nr);

/* Privilege Switch CPL3 to CPL0 */
extern int xtm_pf_ps30_dump_crl;
#define XTM_PF_PS_DUMP_NONE  0
#define XTM_PF_PS_DUMP_START 1
#define XTM_PF_PS_DUMP_DONE  2

/* TB flags */
void tr_pf_inc_tbf_ljmp_pe(void);
void tr_pf_inc_tbf_rsm(void);
void tr_pf_inc_tbf_pop_ss(void);
void tr_pf_inc_tbf_mov_ss(void);
void tr_pf_inc_tbf_smm(void);
void tr_pf_inc_tbf_cpu_reset(void);

#ifndef CONFIG_XTM_PROFILE_TBF
#define CONFIG_XTM_PROFILE_TBF
void xtm_pf_tbf_clr_prefix(void);
int xtm_pf_tbf_has_prefix(void);

void xtm_pf_inc_tbf_ljmp_pe(void);
void xtm_pf_inc_tbf_lcall_pe(void);
void xtm_pf_inc_tbf_ret_pe(void);
void xtm_pf_inc_tbf_interrupt(void);
void xtm_pf_inc_tbf_rsm(void);
void xtm_pf_inc_tbf_smm(void);
void xtm_pf_inc_tbf_cpu_reset(void);
void xtm_pf_inc_tbf_switch_tss(void);
void xtm_pf_inc_tbf_sys_enter(void);
void xtm_pf_inc_tbf_sys_exit(void);
void xtm_pf_inc_tbf_pop_ss(void);
void xtm_pf_inc_tbf_mov_ss(void);

void xtm_pf_inc_tbf_cs32(int oldv, int newv);
void xtm_pf_inc_tbf_ss32(int oldv, int newv);
void xtm_pf_dump_tbf_short(void);
#endif

/* Function for QEMU monitor */
void dump_xtm_profile(void);
void dump_xtmdiff_profile(void);

void dump_xtm_sc_profile(xtm_profile_sys_t *pf);
void dump_xtm_tc_profile(xtm_profile_sys_t *pf);
void dump_xtm_rs_profile(xtm_profile_sys_t *pf);
void dump_xtm_ts_profile(xtm_profile_sys_t *pf);
void dump_xtm_cs_profile(xtm_profile_sys_t *pf);
void dump_xtm_jc_profile(xtm_profile_sys_t *pf);
void dump_xtm_tbht_profile(xtm_profile_sys_t *pf);
void dump_xtm_ps_profile(xtm_profile_sys_t *pf);
void dump_xtm_tbf_profile(xtm_profile_sys_t *pf);

void dump_xtm_sc_profile_diff(xtm_profile_sys_t *pf, xtm_profile_sys_t *pr);
void dump_xtm_tc_profile_diff(xtm_profile_sys_t *pf, xtm_profile_sys_t *pr);
void dump_xtm_rs_profile_diff(xtm_profile_sys_t *pf, xtm_profile_sys_t *pr);
void dump_xtm_ts_profile_diff(xtm_profile_sys_t *pf, xtm_profile_sys_t *pr);
void dump_xtm_tbht_profile_diff(xtm_profile_sys_t *pf, xtm_profile_sys_t *pr);
void dump_xtm_ps_profile_diff(xtm_profile_sys_t *pf, xtm_profile_sys_t *pr);

/* Function to be called in different places */
void xtm_pf_step_translate_tb(void *tb);
void xtm_pf_step_before_exec_tb(void *tb);
void xtm_pf_step_after_exec_tb(void);
void xtm_pf_step_cpu_loop_exit(void);

void xtm_pf_step_gen_tb_start(void);

#endif
