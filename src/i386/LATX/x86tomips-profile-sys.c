#if 0
/* X86toMIPS headers */
#include "common.h"
#include "reg_alloc.h"
#include "env.h"
#include "ir2/ir2.h"
#include "x86tomips-options.h"
#include <capstone/capstone.h>

/* QEMU headers */
#include "qemu-def.h"
#include "qemu/option.h"
#include "qemu/option_int.h"

#include "x86tomips-profile-sys.h"
#include "qemu/qemu-print.h"
#include "x86tomips-options.h"

#include <time.h>

#ifdef CONFIG_XTM_PROFILE

#define _XTM_MPRINT_(str, name) do { \
    qemu_printf( str " %lld\n", \
            (unsigned long long)(pf->name)); \
} while(0)

#define _XTM_MPRINT_DIFF_(str, name) do { \
    qemu_printf( str " %lld\n", \
            (unsigned long long)(pf->name - pr->name)); \
} while(0)

#define _XTM_MPRINT_PS_(str, pc, count) do { \
    qemu_printf( str " %#-16llx %lld\n", \
            (unsigned long long)(pf->pc), \
            (unsigned long long)(pf->count)); \
} while(0)

#define _XTM_MPRINT_PS_DIFF_(str, pc, count) do { \
    qemu_printf( str " %#-16llx %lld\n", \
            (unsigned long long)(pf->pc), \
            (unsigned long long)(pf->count - pr->count)); \
} while(0)

#define _XTM_MPRINT_IDX_(str, name, idx) do { \
    qemu_printf( str " %lld\n", \
            (unsigned long long)(pf->name[idx])); \
} while(0)

#define _XTM_MPRINT_IDX_DIFF_(str, name, idx) do { \
    qemu_printf( str " %lld\n", \
            (unsigned long long)(pf->name[idx] - pr->name[idx])); \
} while(0)

#define _XTM_MPRINTFP_(str, name) do { \
    qemu_printf( str " %.4f\n", \
            (double)(pf->name)); \
} while(0)

#define _XTM_MPRINT_GP_(str, gp, name) do { \
    qemu_printf( str " %lld\n", \
            (unsigned long long)(pf->gp.name)); \
} while(0)

#define _XTM_MPRINT_GP_DIFF_(str, gp, name) do { \
    qemu_printf( str " %lld\n", \
            (unsigned long long)(pf->gp.name - pr->gp.name)); \
} while(0)

xtm_profile_sys_t xtm_pf_data;
xtm_profile_sys_t xtm_pf_data_rcd;

void xtm_pf_clear_all(void)
{
    memset(&xtm_pf_data, 0, sizeof(xtm_profile_sys_t));
    memset(&xtm_pf_data_rcd, 0, sizeof(xtm_profile_sys_t));
}

typedef struct {
    int valid;
    uint64_t target_ins_nr;
    uint64_t target_size;
    uint64_t host_ins_nr;
    uint64_t host_size;
    uint64_t flags;
} xtm_pf_cs_data_entry_t;

#define CSPF_NUM_MASK ((1 << 17) - 1)

typedef struct {
    uint64_t tb_exec_nr;
    xtm_pf_cs_data_entry_t data[131072]; /* 1 << 17 */
    int index;
#define CSPF_STAT_DATA_DEFINE(num) \
    uint64_t sum_ ## num ## _target_ins_nr; \
    uint64_t sum_ ## num ## _host_ins_nr; \
    uint64_t sum_ ## num ## _target_ins_nr_cpl0; \
    uint64_t sum_ ## num ## _host_ins_nr_cpl0; \
    uint64_t sum_ ## num ## _target_ins_nr_cpl3; \
    uint64_t sum_ ## num ## _host_ins_nr_cpl3;

    CSPF_STAT_DATA_DEFINE(1000)
    CSPF_STAT_DATA_DEFINE(10000)
    CSPF_STAT_DATA_DEFINE(100000)

} xtm_pf_cs_data_t;

xtm_pf_cs_data_t xtm_pf_cs_data;

typedef struct {
    uint8_t is_valid;
    uint8_t is_jmpdr;
    uint8_t is_jmpin;
    uint8_t is_sys_eob;
    uint8_t is_excp;
    uint8_t is_hit;
} xtm_pf_jmp_cache_entry_t;

#define JCCPF_NUM_MASK ((1 << 17) - 1)

typedef struct {
    xtm_pf_jmp_cache_entry_t data[131072]; /* 1 << 17 */
    int index;
} xtm_pf_jmp_cache_data_t;

xtm_pf_jmp_cache_data_t xtm_pf_jcc_data;

typedef struct {
    int is_valid;
    int is_hit;
    int hash_cmp_nr;
} xtm_pf_tbht_entry_t;

#define TBHTPF_NUM_MASK ((1 << 17) - 1)

typedef struct {
    xtm_pf_tbht_entry_t data[131072]; /* 1 << 17 */
    int index;
} xtm_pf_tbht_data_t;

xtm_pf_tbht_data_t xtm_pf_tbht_data;

static void __attribute__((__constructor__)) xtm_profile_init(void)
{
    memset(&xtm_pf_data, 0, sizeof(xtm_profile_sys_t));
    memset(&xtm_pf_data_rcd, 0, sizeof(xtm_profile_sys_t));
    memset(&xtm_pf_cs_data, 0, sizeof(xtm_pf_cs_data_t));
    memset(&xtm_pf_jcc_data, 0, sizeof(xtm_pf_jmp_cache_data_t));
    memset(&xtm_pf_tbht_data, 0, sizeof(xtm_pf_tbht_data_t));
}

/* ------------------- Timer Counter ----------------------- */

xtm_profile_sys_tc_t xtm_pf_tc;

#define XTM_PF_TIMEPSEC_FN_ST(group, name) \
void xtm_pf_tc_ ## group ## _ ## name ## _st(void) \
{ \
    if (!option_monitor_tc) return; \
    clock_gettime(CLOCK_REALTIME, \
            &(xtm_pf_tc.group. name ## _st)); \
}

XTM_PF_TIMEPSEC_FN_ST(exe, all)
XTM_PF_TIMEPSEC_FN_ST(exe, tb)
XTM_PF_TIMEPSEC_FN_ST(tr, all)
XTM_PF_TIMEPSEC_FN_ST(tr, disasm)
XTM_PF_TIMEPSEC_FN_ST(tr, disasm_readbytes)
XTM_PF_TIMEPSEC_FN_ST(tr, disasm_capstone)
XTM_PF_TIMEPSEC_FN_ST(tr, translate)
XTM_PF_TIMEPSEC_FN_ST(tr, assemble)

#define XTM_PF_TIMEPSEC_FN_ED(group, name) \
void xtm_pf_tc_ ## group ## _ ## name ## _ed(void) \
{ \
    if (!option_monitor_tc) return; \
    clock_gettime(CLOCK_REALTIME, &(xtm_pf_tc.group. name ## _ed)); \
    uint64_t st_ss = xtm_pf_tc.group. name ## _st.tv_sec; \
    uint64_t st_ns = xtm_pf_tc.group. name ## _st.tv_nsec; \
    uint64_t ed_ss = xtm_pf_tc.group. name ## _ed.tv_sec; \
    uint64_t ed_ns = xtm_pf_tc.group. name ## _ed.tv_nsec; \
    xtm_pf_data.tc.group. name ## _ss += ed_ss - st_ss; \
    if (ed_ns >= st_ns) { \
        xtm_pf_data.tc.group. name ## _ns += ed_ns - st_ns; \
    } else { \
        xtm_pf_data.tc.group. name ## _ns += (1000000000 + ed_ns) - st_ns; \
        xtm_pf_data.tc.group. name ## _ss -= 1; \
    } \
    if (xtm_pf_data.tc.group. name ## _ns > 1000000000) { \
        xtm_pf_data.tc.group. name ## _ss += 1; \
        xtm_pf_data.tc.group. name ## _ns -= 1000000000; \
    } \
}

XTM_PF_TIMEPSEC_FN_ED(exe, all)
XTM_PF_TIMEPSEC_FN_ED(exe, tb)
XTM_PF_TIMEPSEC_FN_ED(tr, all)
XTM_PF_TIMEPSEC_FN_ED(tr, disasm)
XTM_PF_TIMEPSEC_FN_ED(tr, disasm_readbytes)
XTM_PF_TIMEPSEC_FN_ED(tr, disasm_capstone)
XTM_PF_TIMEPSEC_FN_ED(tr, translate)
XTM_PF_TIMEPSEC_FN_ED(tr, assemble)

#define _XTM_MONITOR_TC_PRINT_(str, gp, prefix) do { \
    qemu_printf( str " %lld s %lld ns\n", \
            (unsigned long long)(pf->tc.gp.prefix ## _ss), \
            (unsigned long long)(pf->tc.gp.prefix ## _ns)); \
} while(0)

void dump_xtm_tc_profile(xtm_profile_sys_t *pf)
{
    if (!option_monitor_tc) return;

    _XTM_MONITOR_TC_PRINT_("TC -- ALL Execution ---- ", exe, all);
    _XTM_MONITOR_TC_PRINT_("TC -- TB --------------- ", exe, tb);
    _XTM_MONITOR_TC_PRINT_("TC -- ALL Translation -- ", tr, all);
    _XTM_MONITOR_TC_PRINT_("TC -- Disasm all ------- ", tr, disasm);
    _XTM_MONITOR_TC_PRINT_("TC -- Disasm readins --- ", tr, disasm_readbytes);
    _XTM_MONITOR_TC_PRINT_("TC -- Disasm capstone -- ", tr, disasm_capstone);
    _XTM_MONITOR_TC_PRINT_("TC -- Translate -------- ", tr, translate);
    _XTM_MONITOR_TC_PRINT_("TC -- Assemble --------- ", tr, assemble);
}

#define _XTM_MONITOR_TC_PRINT_DIFF_(str, gp, prefix) do { \
    if (pf->tc.gp.prefix ## _ns > pr->tc.gp.prefix ## _ns) { \
        ss_diff = pf->tc.gp.prefix ## _ss - pr->tc.gp.prefix ## _ss; \
        ns_diff = pf->tc.gp.prefix ## _ns - pr->tc.gp.prefix ## _ns; \
    } else { \
        ss_diff = pf->tc.gp.prefix ## _ss - pr->tc.gp.prefix ## _ss - 1; \
        ns_diff = 1000000000 + pf->tc.gp.prefix ## _ns - pr->tc.gp.prefix ## _ns; \
    } \
    qemu_printf( str " %lld s %lld ns\n", \
            (unsigned long long)(ss_diff), \
            (unsigned long long)(ns_diff)); \
} while(0)

void dump_xtm_tc_profile_diff(xtm_profile_sys_t *pf, xtm_profile_sys_t *pr)
{
    if (!option_monitor_tc) return;
    
    uint64_t ss_diff, ns_diff;

    _XTM_MONITOR_TC_PRINT_DIFF_("TC -- ALL Execution ---- ", exe, all);
    _XTM_MONITOR_TC_PRINT_DIFF_("TC -- TB --------------- ", exe, tb);
    _XTM_MONITOR_TC_PRINT_DIFF_("TC -- ALL Translation -- ", tr, all);
    _XTM_MONITOR_TC_PRINT_DIFF_("TC -- Disasm ----------- ", tr, disasm);
    _XTM_MONITOR_TC_PRINT_DIFF_("TC -- Translate -------- ", tr, translate);
    _XTM_MONITOR_TC_PRINT_DIFF_("TC -- Assemble --------- ", tr, assemble);
}

/* ------------------- Simple Counter ---------------------- */

#define _XTM_PF_DO_INC_NATIVE_(addr, inc) do { \
    IR2_OPND num = ra_alloc_itemp(); \
    IR2_OPND mem = ra_alloc_itemp(); \
    load_imm64_to_ir2(&mem, addr); \
    append_ir2_opnd2i(LISA_LD_D,   &num, &mem, 0); \
    append_ir2_opnd2i(LISA_ADDI_D, &num, &num, inc); \
    append_ir2_opnd2i(LISA_ST_D,   &num, &mem, 0); \
    ra_free_temp(&num); \
    ra_free_temp(&mem); \
} while(0)

#define _XTM_PF_INC_NATIVE_(field, inc) do { \
    ADDR addr = (ADDR)(&(xtm_pf_data.field)); \
    _XTM_PF_DO_INC_NATIVE_(addr, inc); \
} while(0)

#define _XTM_PF_DEF_INC_NATIVE_(field) \
void tr_pf_inc_ ## field(void) \
{ \
    if (!option_monitor_sc) return; \
    _XTM_PF_INC_NATIVE_(num_ ## field, 0x1); \
}

_XTM_PF_DEF_INC_NATIVE_(tb_exec);
_XTM_PF_DEF_INC_NATIVE_(tb_use_fpu_exec);
_XTM_PF_DEF_INC_NATIVE_(tb_use_xmm_exec);
_XTM_PF_DEF_INC_NATIVE_(tb_cpl0_exec);
_XTM_PF_DEF_INC_NATIVE_(tb_cpl1_exec);
_XTM_PF_DEF_INC_NATIVE_(tb_cpl2_exec);
_XTM_PF_DEF_INC_NATIVE_(tb_cpl3_exec);
_XTM_PF_DEF_INC_NATIVE_(cs_in);
_XTM_PF_DEF_INC_NATIVE_(cs_out);
_XTM_PF_DEF_INC_NATIVE_(cs_in_fpu);
_XTM_PF_DEF_INC_NATIVE_(cs_in_xmm);
_XTM_PF_DEF_INC_NATIVE_(helper);
_XTM_PF_DEF_INC_NATIVE_(helper_allgpr);
_XTM_PF_DEF_INC_NATIVE_(helper_fpu);
_XTM_PF_DEF_INC_NATIVE_(helper_xmm);
_XTM_PF_DEF_INC_NATIVE_(tlb_hit);
_XTM_PF_DEF_INC_NATIVE_(tlb_miss);
_XTM_PF_DEF_INC_NATIVE_(fpu_rotate_native);
_XTM_PF_DEF_INC_NATIVE_(excp);
_XTM_PF_DEF_INC_NATIVE_(fastcs_glue_01);
_XTM_PF_DEF_INC_NATIVE_(fastcs_glue_10);
_XTM_PF_DEF_INC_NATIVE_(fastcs_glue_11);
_XTM_PF_DEF_INC_NATIVE_(cpc_hit);
_XTM_PF_DEF_INC_NATIVE_(cpc_miss);

void tr_pf_inc_staticcs(void)
{
    if (!option_monitor_sc) return;

    ADDR addr = (ADDR)(&(xtm_pf_data.num_staticcs));

    IR2_OPND *num = &stmp1_ir2_opnd;
    IR2_OPND *mem = &stmp2_ir2_opnd;

    load_imm64_to_ir2(mem, addr);

    append_ir2_opnd2i(LISA_LD_D,   num, mem, 0);
    append_ir2_opnd2i(LISA_ADDI_D, num, num, 1);
    append_ir2_opnd2i(LISA_ST_D,   num, mem, 0);
}

#define _XTM_PF_INC_(field, inc) do { \
    xtm_pf_data.field += inc; \
} while(0)

#define _XTM_PF_DEF_INC_(field) \
void xtm_pf_inc_ ## field (void) \
{ \
    if (!option_monitor_sc) return; \
    _XTM_PF_INC_(num_ ## field, 1); \
}

_XTM_PF_DEF_INC_(tb_exec);
_XTM_PF_DEF_INC_(tb_use_fpu_exec);
_XTM_PF_DEF_INC_(tb_use_xmm_exec);
_XTM_PF_DEF_INC_(tb_cpl0_exec);
_XTM_PF_DEF_INC_(tb_cpl1_exec);
_XTM_PF_DEF_INC_(tb_cpl2_exec);
_XTM_PF_DEF_INC_(tb_cpl3_exec);
_XTM_PF_DEF_INC_(cs_in);
_XTM_PF_DEF_INC_(cs_out);
_XTM_PF_DEF_INC_(cs_in_fpu);
_XTM_PF_DEF_INC_(cs_in_xmm);
_XTM_PF_DEF_INC_(helper);
_XTM_PF_DEF_INC_(helper_allgpr);
_XTM_PF_DEF_INC_(helper_fpu);
_XTM_PF_DEF_INC_(helper_xmm);
_XTM_PF_DEF_INC_(tlb_hit);
_XTM_PF_DEF_INC_(tlb_miss);
_XTM_PF_DEF_INC_(fpu_rotate_bt);
_XTM_PF_DEF_INC_(excp);

void dump_xtm_sc_profile(xtm_profile_sys_t *pf)
{
    if (!option_monitor_sc) return;

    _XTM_MPRINT_("TB executed -------------- ", num_tb_exec);
    _XTM_MPRINT_("TB using FPU executed ---- ", num_tb_use_fpu_exec);
    _XTM_MPRINT_("TB using XMM executed ---- ", num_tb_use_xmm_exec);
    _XTM_MPRINT_("TB CPL0 executed --------- ", num_tb_cpl0_exec);
    _XTM_MPRINT_("TB CPL1 executed --------- ", num_tb_cpl1_exec);
    _XTM_MPRINT_("TB CPL2 executed --------- ", num_tb_cpl2_exec);
    _XTM_MPRINT_("TB CPL3 executed --------- ", num_tb_cpl3_exec);
    _XTM_MPRINT_("Context Switch In -------- ", num_cs_in);
    _XTM_MPRINT_("Context Switch Out ------- ", num_cs_out);
    _XTM_MPRINT_("Context Switch In FPU ---- ", num_cs_in_fpu);
    _XTM_MPRINT_("Context Switch In XMM ---- ", num_cs_in_xmm);
    _XTM_MPRINT_("helper ------------------- ", num_helper);
    _XTM_MPRINT_("helper save ALL context -- ", num_helper_allgpr);
    _XTM_MPRINT_("helper save FPU ---------- ", num_helper_fpu);
    _XTM_MPRINT_("helper save XMM ---------- ", num_helper_xmm);
    _XTM_MPRINT_("QEMU TLB hit ------------- ", num_tlb_hit);
    _XTM_MPRINT_("QEMU TLB miss ------------ ", num_tlb_miss);
    _XTM_MPRINT_("FPU rotated in QEMU ------ ", num_fpu_rotate_bt);
    _XTM_MPRINT_("FPU rotated in native ---- ", num_fpu_rotate_native);
    _XTM_MPRINT_("Exception in native ------ ", num_excp);
    _XTM_MPRINT_("FastCS     FPU ----------- ", num_fastcs_glue_01);
    _XTM_MPRINT_("FastCS XMM     ----------- ", num_fastcs_glue_10);
    _XTM_MPRINT_("FastCS XMM FPU ----------- ", num_fastcs_glue_11);
    _XTM_MPRINT_("Static Context Switch ---- ", num_staticcs);
    _XTM_MPRINT_("Cross-Page Check Hit ----- ", num_cpc_hit);
    _XTM_MPRINT_("Cross-Page Check Miss ---- ", num_cpc_miss);
}

void dump_xtm_sc_profile_diff(xtm_profile_sys_t *pf, xtm_profile_sys_t *pr)
{
    if (!option_monitor_sc) return;

    _XTM_MPRINT_DIFF_("TB executed -------------- ", num_tb_exec);
    _XTM_MPRINT_DIFF_("TB using FPU executed ---- ", num_tb_use_fpu_exec);
    _XTM_MPRINT_DIFF_("TB using XMM executed ---- ", num_tb_use_xmm_exec);
    _XTM_MPRINT_DIFF_("TB CPL0 executed --------- ", num_tb_cpl0_exec);
    _XTM_MPRINT_DIFF_("TB CPL1 executed --------- ", num_tb_cpl1_exec);
    _XTM_MPRINT_DIFF_("TB CPL2 executed --------- ", num_tb_cpl2_exec);
    _XTM_MPRINT_DIFF_("TB CPL3 executed --------- ", num_tb_cpl3_exec);
    _XTM_MPRINT_DIFF_("Context Switch In -------- ", num_cs_in);
    _XTM_MPRINT_DIFF_("Context Switch Out ------- ", num_cs_out);
    _XTM_MPRINT_DIFF_("Context Switch In FPU ---- ", num_cs_in_fpu);
    _XTM_MPRINT_DIFF_("Context Switch In XMM ---- ", num_cs_in_xmm);
    _XTM_MPRINT_DIFF_("helper ------------------- ", num_helper);
    _XTM_MPRINT_DIFF_("helper save ALL context -- ", num_helper_allgpr);
    _XTM_MPRINT_DIFF_("helper save FPU ---------- ", num_helper_fpu);
    _XTM_MPRINT_DIFF_("helper save XMM ---------- ", num_helper_xmm);
    _XTM_MPRINT_DIFF_("QEMU TLB hit ------------- ", num_tlb_hit);
    _XTM_MPRINT_DIFF_("QEMU TLB miss ------------ ", num_tlb_miss);
    _XTM_MPRINT_DIFF_("FPU rotated in QEMU ------ ", num_fpu_rotate_bt);
    _XTM_MPRINT_DIFF_("FPU rotated in native ---- ", num_fpu_rotate_native);
    _XTM_MPRINT_DIFF_("Exception in native ------ ", num_excp);
    _XTM_MPRINT_DIFF_("FastCS     FPU ----------- ", num_fastcs_glue_01);
    _XTM_MPRINT_DIFF_("FastCS XMM FPU ----------- ", num_fastcs_glue_10);
    _XTM_MPRINT_DIFF_("FastCS XMM FPU ----------- ", num_fastcs_glue_11);
    _XTM_MPRINT_DIFF_("Static Context Switch ---- ", num_staticcs);
    _XTM_MPRINT_DIFF_("Cross-Page Check Hit ----- ", num_cpc_hit);
    _XTM_MPRINT_DIFF_("Cross-Page Check Miss ---- ", num_cpc_miss);
}

/* ------------------- Regsiter Stat ----------------------- */

#define _XTM_PF_DEF_INC_RS_(field, inc) \
void xtm_pf_inc_rs_ ## field (void) \
{ \
    if (!option_monitor_rs) return; \
    xtm_pf_data.rs.field += inc; \
}

_XTM_PF_DEF_INC_RS_(ra_alloc_int_nr, 1);
_XTM_PF_DEF_INC_RS_(ra_alloc_fp_nr, 1);
_XTM_PF_DEF_INC_RS_(ra_free_int_nr, 1);
_XTM_PF_DEF_INC_RS_(ra_free_fp_nr, 1);

void xtm_pf_inc_rs_ra_int_used_max(int num)
{
    if (!option_monitor_rs) return;
    if (num > xtm_pf_data.rs.ra_int_used_max) {
        xtm_pf_data.rs.ra_int_used_max = num;
    }
}
void xtm_pf_inc_rs_ra_fp_used_max(int num)
{
    if (!option_monitor_rs) return;
    if (num > xtm_pf_data.rs.ra_fp_used_max) {
        xtm_pf_data.rs.ra_fp_used_max = num;
    }
}

void dump_xtm_rs_profile(xtm_profile_sys_t *pf)
{
    if (!option_monitor_rs) return;

    _XTM_MPRINT_GP_("RS --- Int Temp Allocated num ------ ", rs, ra_alloc_int_nr);
    _XTM_MPRINT_GP_("RS --- FP  Temp Allocated num ------ ", rs, ra_alloc_fp_nr);
    _XTM_MPRINT_GP_("RS --- Int Temp Freed num ---------- ", rs, ra_free_int_nr);
    _XTM_MPRINT_GP_("RS --- FP  Temp Freed num ---------- ", rs, ra_free_fp_nr);
    _XTM_MPRINT_GP_("RS --- Int Used Max ---------------- ", rs, ra_int_used_max);
    _XTM_MPRINT_GP_("RS --- FP  Used Max ---------------- ", rs, ra_fp_used_max);
}

void dump_xtm_rs_profile_diff(xtm_profile_sys_t *pf, xtm_profile_sys_t *pr)
{
    if (!option_monitor_rs) return;

    _XTM_MPRINT_GP_DIFF_("RS --- Int Temp Allocated num ------ ", rs, ra_alloc_int_nr);
    _XTM_MPRINT_GP_DIFF_("RS --- FP  Temp Allocated num ------ ", rs, ra_alloc_fp_nr);
    _XTM_MPRINT_GP_DIFF_("RS --- Int Temp Freed num ---------- ", rs, ra_free_int_nr);
    _XTM_MPRINT_GP_DIFF_("RS --- FP  Temp Freed num ---------- ", rs, ra_free_fp_nr);
    _XTM_MPRINT_GP_DIFF_("RS --- Int Used Max ---------------- ", rs, ra_int_used_max);
    _XTM_MPRINT_GP_DIFF_("RS --- FP  Used Max ---------------- ", rs, ra_fp_used_max);
}

/* ------------------- Translation Stat -------------------- */

#define _XTM_PF_DEF_INC_TS_(field, inc) \
void xtm_pf_inc_ts_ ## field (void) \
{ \
    if (!option_monitor_ts) return; \
    xtm_pf_data.ts.field += inc; \
}

_XTM_PF_DEF_INC_TS_(ins_translated, 1)
_XTM_PF_DEF_INC_TS_(tb_translated, 1)

void dump_xtm_ts_profile(xtm_profile_sys_t *pf)
{
    if (!option_monitor_ts) return;

    _XTM_MPRINT_GP_("TS --- Ins Transalted -------------- ", ts, ins_translated);
    _XTM_MPRINT_GP_("TS --- TB  Transalted -------------- ", ts, tb_translated);
}

void dump_xtm_ts_profile_diff(xtm_profile_sys_t *pf, xtm_profile_sys_t *pr)
{
    if (!option_monitor_ts) return;

    _XTM_MPRINT_GP_DIFF_("TS --- Ins Transalted -------------- ", ts, ins_translated);
    _XTM_MPRINT_GP_DIFF_("TS --- TB  Transalted -------------- ", ts, tb_translated);
}

/* ------------------- Complex Stat ------------------------ */

void xtm_pf_tb_exec_record(void *_tb)
{
    if (!option_monitor_cs) return;

    TranslationBlock *tb = (TranslationBlock*)_tb;
    xtm_pf_cs_data_t *csd  = &xtm_pf_cs_data;
    csd->tb_exec_nr += 1;

    int index = csd->index;
    xtm_pf_cs_data_entry_t *csde = &csd->data[index];

    csde->valid = 1;
    csde->target_ins_nr = tb->icount;
    csde->target_size   = tb->size;
    csde->host_ins_nr   = tb->tc.size >> 2;
    csde->host_size     = tb->tc.size;
    csde->flags         = tb->flags;

    xtm_profile_sys_t *pf = &xtm_pf_data;

    csd->sum_1000_target_ins_nr += csde->target_ins_nr;
    csd->sum_1000_host_ins_nr   += csde->host_ins_nr;
    csd->sum_10000_target_ins_nr += csde->target_ins_nr;
    csd->sum_10000_host_ins_nr   += csde->host_ins_nr;
    csd->sum_100000_target_ins_nr += csde->target_ins_nr;
    csd->sum_100000_host_ins_nr   += csde->host_ins_nr;

    if (((csde->flags) & 0x3) == 3) {
        csd->sum_1000_target_ins_nr_cpl3 += csde->target_ins_nr;
        csd->sum_1000_host_ins_nr_cpl3   += csde->host_ins_nr;
        pf->last_1000_cpl3_tb_nr += 1;
        csd->sum_10000_target_ins_nr_cpl3 += csde->target_ins_nr;
        csd->sum_10000_host_ins_nr_cpl3   += csde->host_ins_nr;
        pf->last_10000_cpl3_tb_nr += 1;
        csd->sum_100000_target_ins_nr_cpl3 += csde->target_ins_nr;
        csd->sum_100000_host_ins_nr_cpl3   += csde->host_ins_nr;
        pf->last_100000_cpl3_tb_nr += 1;
    } else {
        csd->sum_1000_target_ins_nr_cpl0 += csde->target_ins_nr;
        csd->sum_1000_host_ins_nr_cpl0   += csde->host_ins_nr;
        pf->last_1000_cpl0_tb_nr += 1;
        csd->sum_10000_target_ins_nr_cpl0 += csde->target_ins_nr;
        csd->sum_10000_host_ins_nr_cpl0   += csde->host_ins_nr;
        pf->last_10000_cpl0_tb_nr += 1;
        csd->sum_100000_target_ins_nr_cpl0 += csde->target_ins_nr;
        csd->sum_100000_host_ins_nr_cpl0   += csde->host_ins_nr;
        pf->last_100000_cpl0_tb_nr += 1;
    }

#define CSPF_PROCESS(num) \
    if (csd->tb_exec_nr > ((num - 1))) { \
        csde = &csd->data[(index - num) & CSPF_NUM_MASK]; \
        if (csde->valid) { \
            csd->sum_ ## num ## _target_ins_nr -= csde->target_ins_nr; \
            csd->sum_ ## num ## _host_ins_nr   -= csde->host_ins_nr; \
            if ((csde->flags & 0x3) == 3) { \
                csd->sum_ ## num ## _target_ins_nr_cpl3 -= csde->target_ins_nr; \
                csd->sum_ ## num ## _host_ins_nr_cpl3   -= csde->host_ins_nr; \
                pf->last_ ## num ## _cpl3_tb_nr -= 1; \
            } else { \
                csd->sum_ ## num ## _target_ins_nr_cpl0 -= csde->target_ins_nr; \
                csd->sum_ ## num ## _host_ins_nr_cpl0   -= csde->host_ins_nr; \
                pf->last_ ## num ## _cpl0_tb_nr -= 1; \
            } \
        } \
        pf->expand_ratio_ ## num = \
            (float)(csd->sum_ ## num ## _host_ins_nr) / \
            (float)(csd->sum_ ## num ## _target_ins_nr); \
        pf->expand_ratio_ ## num ##  _cpl3 = \
            (float)(csd->sum_ ## num ## _host_ins_nr_cpl3) / \
            (float)(csd->sum_ ## num ## _target_ins_nr_cpl3); \
        pf->expand_ratio_ ## num ## _cpl0 = \
            (float)(csd->sum_ ## num ## _host_ins_nr_cpl0) / \
            (float)(csd->sum_ ## num ## _target_ins_nr_cpl0); \
    }

    CSPF_PROCESS(1000)
    CSPF_PROCESS(10000)
    CSPF_PROCESS(100000)

    csd->index += 1;
    csd->index &= CSPF_NUM_MASK;
}

void dump_xtm_cs_profile(xtm_profile_sys_t *pf)
{
    if (!option_monitor_cs) return;

#define CSPF_MPRINT(str, num) \
    _XTM_MPRINTFP_("Last " str "   TB ALL  expand ratio by ins = ", expand_ratio_ ## num); \
    _XTM_MPRINTFP_("Last " str "   TB CPL3 expand ratio by ins = ", expand_ratio_ ## num ## _cpl3); \
    _XTM_MPRINTFP_("Last " str "   TB CPL0 expand ratio by ins = ", expand_ratio_ ## num ## _cpl0); \
    _XTM_MPRINT_("Last " str "   TB CPL3 number              = ", last_ ## num ## _cpl3_tb_nr); \
    _XTM_MPRINT_("Last " str "   TB CPL0 number              = ", last_ ## num ## _cpl0_tb_nr);

    CSPF_MPRINT("1000",   1000)
    CSPF_MPRINT("10000",  10000)
    CSPF_MPRINT("100000", 100000)
}

/* ------------------- Jmp Cache Counter ------------------- */

void xtm_pf_jmp_cache(void *cpu, void *tb)
{
    if (!option_monitor_jc) return;

    CPUX86State *env = ((CPUState*)cpu)->env_ptr;

    uint8_t is_jmpdr   = env->xtm_pf_data.jc.is_jmpdr;
    uint8_t is_jmpin   = env->xtm_pf_data.jc.is_jmpin;
    uint8_t is_sys_eob = env->xtm_pf_data.jc.is_sys_eob;
    uint8_t is_excp    = env->xtm_pf_data.jc.is_excp;
    uint8_t is_hit     = tb == NULL ? 0 : 1;

    xtm_profile_sys_t *pf = &xtm_pf_data;
    xtm_pf_jmp_cache_data_t *jccd = &xtm_pf_jcc_data;
    xtm_pf_jmp_cache_entry_t *jcce;

    int index = jccd->index;
    jcce = &jccd->data[index];

    jcce->is_valid   = 1;
    jcce->is_jmpdr   = is_jmpdr;
    jcce->is_jmpin   = is_jmpin;
    jcce->is_sys_eob = is_sys_eob;
    jcce->is_excp    = is_excp;
    jcce->is_hit     = is_hit;

    pf->num_jc_lookup += 1;
    if (is_hit) pf->num_jc_lookup_hit += 1;

#define JCCPF_TYPE_PROCESS_INC(type) \
    if (is_ ## type ) { \
        pf->num_jc_lookup_ ## type += 1; \
        pf->num_jc_lookup_ ## type ## _in_last_1000 += 1; \
        pf->num_jc_lookup_ ## type ## _in_last_10000 += 1; \
        pf->num_jc_lookup_ ## type ## _in_last_100000 += 1; \
        if (is_hit) { \
            pf->num_jc_lookup_ ## type ## _hit += 1; \
            pf->num_jc_lookup_ ## type ## _in_last_1000_hit += 1; \
            pf->num_jc_lookup_ ## type ## _in_last_10000_hit += 1; \
            pf->num_jc_lookup_ ## type ## _in_last_100000_hit += 1; \
        } \
    }

    JCCPF_TYPE_PROCESS_INC(jmpdr)
    JCCPF_TYPE_PROCESS_INC(jmpin)
    JCCPF_TYPE_PROCESS_INC(sys_eob)
    JCCPF_TYPE_PROCESS_INC(excp)

#define JCCPF_TYPE_PROCESS_DEC(type, num) \
    if (jcce->is_valid && jcce->is_ ## type) { \
        pf->num_jc_lookup_ ## type ## _in_last_ ## num -= 1; \
        if (jcce->is_hit) \
            pf->num_jc_lookup_ ## type ## _in_last_ ## num ## _hit -= 1; \
    } \

#define JCCPF_PROCESS(num) do { \
    if (pf->num_jc_lookup > (num)) { \
        jcce = &jccd->data[(index - (num)) & JCCPF_NUM_MASK]; \
        JCCPF_TYPE_PROCESS_DEC(jmpdr, num) \
        JCCPF_TYPE_PROCESS_DEC(jmpin, num) \
        JCCPF_TYPE_PROCESS_DEC(sys_eob, num) \
        JCCPF_TYPE_PROCESS_DEC(excp, num) \
    } \
} while(0)

    JCCPF_PROCESS(1000);
    JCCPF_PROCESS(10000);
    JCCPF_PROCESS(100000);

    jccd->index += 1;
    jccd->index &= JCCPF_NUM_MASK;
}

#define _XTM_PF_DEF_INC_NUM_JC_NATIVE_(field) \
void tr_pf_inc_ ## field(void *tmp1, void *tmp2) \
{ \
    if (!option_monitor_jc) return; \
    ADDR addr = (ADDR)(&(xtm_pf_data.num_ ## field)); \
    IR2_OPND *num = tmp1; \
    IR2_OPND *mem = tmp2; \
    load_imm64_to_ir2(mem, addr); \
    append_ir2_opnd2i(LISA_LD_D,   num, mem, 0); \
    append_ir2_opnd2i(LISA_ADDI_D, num, num, 1); \
    append_ir2_opnd2i(LISA_ST_D,   num, mem, 0); \
}

_XTM_PF_DEF_INC_NUM_JC_NATIVE_(jc_cam)
_XTM_PF_DEF_INC_NUM_JC_NATIVE_(jc_cam_hit)
_XTM_PF_DEF_INC_NUM_JC_NATIVE_(jc_njc)
_XTM_PF_DEF_INC_NUM_JC_NATIVE_(jc_njc_hit)

void dump_xtm_jc_profile(xtm_profile_sys_t *pf)
{
    if (!option_monitor_jc) return;

    _XTM_MPRINT_("JC Clear      ", num_jc_clear);
    _XTM_MPRINT_("JC Clear Page ", num_jc_clear_page);

    _XTM_MPRINT_("JC Lookup num                  ", num_jc_lookup);
    _XTM_MPRINT_("JC Lookup num hit              ", num_jc_lookup_hit);

#define JCCPF_DUMP_TYPE(typestr, type) \
    _XTM_MPRINT_("JC Lookup num     " typestr "        ", num_jc_lookup_ ## type); \
    _XTM_MPRINT_("JC Lookup num hit " typestr "        ", num_jc_lookup_ ## type ## _hit); \
    _XTM_MPRINT_("JC Lookup num     " typestr " 1000   ", num_jc_lookup_ ## type ## _in_last_1000); \
    _XTM_MPRINT_("JC Lookup num hit " typestr " 1000   ", num_jc_lookup_ ## type ## _in_last_1000_hit); \
    _XTM_MPRINT_("JC Lookup num     " typestr " 10000  ", num_jc_lookup_ ## type ## _in_last_10000); \
    _XTM_MPRINT_("JC Lookup num hit " typestr " 10000  ", num_jc_lookup_ ## type ## _in_last_10000_hit); \
    _XTM_MPRINT_("JC Lookup num     " typestr " 100000 ", num_jc_lookup_ ## type ## _in_last_100000); \
    _XTM_MPRINT_("JC Lookup num hit " typestr " 100000 ", num_jc_lookup_ ## type ## _in_last_100000_hit);

    JCCPF_DUMP_TYPE("jmpdr",   jmpdr)
    JCCPF_DUMP_TYPE("jmpin",   jmpin)
    JCCPF_DUMP_TYPE("syt_eob", sys_eob)
    JCCPF_DUMP_TYPE("excp",    excp)

    _XTM_MPRINT_("JC Lookup cam     ", num_jc_cam);
    _XTM_MPRINT_("JC Lookup cam hit ", num_jc_cam_hit);
    _XTM_MPRINT_("JC Lookup njc     ", num_jc_njc);
    _XTM_MPRINT_("JC Lookup njc hit ", num_jc_njc_hit);
}

void xtm_pf_jmp_cache_print(void)
{
    fprintf(stderr, "%lld, %lld, \
%lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld, \
%lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld, \
%lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld, \
%lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld, \
%lld, %lld, %lld, %lld, \
%d\n", /* 0 */
    (unsigned long long)xtm_pf_data.num_jc_lookup,
    (unsigned long long)xtm_pf_data.num_jc_lookup_hit,

#define JCCPF_PRINT_TYPE_ENTRY(type) \
    (unsigned long long)xtm_pf_data.num_jc_lookup_ ## type, \
    (unsigned long long)xtm_pf_data.num_jc_lookup_ ## type ## _hit, \
    (unsigned long long)xtm_pf_data.num_jc_lookup_ ## type ## _in_last_1000, \
    (unsigned long long)xtm_pf_data.num_jc_lookup_ ## type ## _in_last_1000_hit, \
    (unsigned long long)xtm_pf_data.num_jc_lookup_ ## type ## _in_last_10000, \
    (unsigned long long)xtm_pf_data.num_jc_lookup_ ## type ## _in_last_10000_hit, \
    (unsigned long long)xtm_pf_data.num_jc_lookup_ ## type ## _in_last_100000, \
    (unsigned long long)xtm_pf_data.num_jc_lookup_ ## type ## _in_last_100000_hit,

    JCCPF_PRINT_TYPE_ENTRY(jmpdr)
    JCCPF_PRINT_TYPE_ENTRY(jmpin)
    JCCPF_PRINT_TYPE_ENTRY(sys_eob)
    JCCPF_PRINT_TYPE_ENTRY(excp)

    (unsigned long long)xtm_pf_data.num_jc_cam,
    (unsigned long long)xtm_pf_data.num_jc_cam_hit,
    (unsigned long long)xtm_pf_data.num_jc_njc,
    (unsigned long long)xtm_pf_data.num_jc_njc_hit,

    0 /* just for passing compile */
    );
}

/* Called by xtm_pf_dwe_helper */
void xtm_pf_jc_dwe_helper(CPUX86State *env, uint64_t tb_exec_nr)
{
    if (option_monitor_jc != 3) return;

    int do_print_trace = 0;
    do_print_trace = !(tb_exec_nr & ((1 << 17) - 1));

    if (do_print_trace) xtm_pf_jmp_cache_print();
}

void xtm_pf_inc_jc_clear(void *cpu)
{
    if (!option_monitor_jc) return;

    xtm_profile_sys_t *pf = &xtm_pf_data;

    pf->num_jc_clear += 1;

    if (option_monitor_jc == 4) {
        static uint64_t last_num_jc_clear_page= 0;

        static uint64_t last_num_jc_lookup = 0;
        static uint64_t last_num_jc_lookup_hit = 0;
        static uint64_t last_num_jc_lookup_jmpin = 0;
        static uint64_t last_num_jc_lookup_jmpin_hit = 0;

        fprintf(stderr, "%lld, %lld, %lld, %lld, %lld, %lld\n",
        (unsigned long long)(pf->num_jc_clear),
        (unsigned long long)(pf->num_jc_clear_page - last_num_jc_clear_page),
        (unsigned long long)(pf->num_jc_lookup - last_num_jc_lookup),
        (unsigned long long)(pf->num_jc_lookup_hit - last_num_jc_lookup_hit),
        (unsigned long long)(pf->num_jc_lookup_jmpin - last_num_jc_lookup_jmpin),
        (unsigned long long)(pf->num_jc_lookup_jmpin_hit - last_num_jc_lookup_jmpin_hit)
        );

        last_num_jc_clear_page = pf->num_jc_clear_page;

        last_num_jc_lookup = pf->num_jc_lookup;
        last_num_jc_lookup_hit = pf->num_jc_lookup_hit;
        last_num_jc_lookup_jmpin = pf->num_jc_lookup_jmpin;
        last_num_jc_lookup_jmpin_hit = pf->num_jc_lookup_jmpin_hit;
    }
}

void xtm_pf_inc_jc_clear_page(void *cpu, uint64_t page_addr)
{
    if (!option_monitor_jc) return;

    xtm_pf_data.num_jc_clear_page += 1;
}

/* ------------------- TB Hash Table Stat ------------------ */

void xtm_pf_tb_hash_table_print(void)
{
    fprintf(stderr, "%lld, %lld, %lld, %lld, %lld, \
%lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld, \
%lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld, \
%lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld, \
%lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld\n",
    (unsigned long long)xtm_pf_data.num_tbht_lookup,
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit,
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_in_last_1000,
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_in_last_10000,
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_in_last_100000,
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hash[0],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hash_in_last_1000[0],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hash_in_last_10000[0],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hash_in_last_100000[0],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hash[1],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hash_in_last_1000[1],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hash_in_last_10000[1],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hash_in_last_100000[1],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hash[2],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hash_in_last_1000[2],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hash_in_last_10000[2],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hash_in_last_100000[2],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hash[3], /* >= 4 */
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hash_in_last_1000[3],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hash_in_last_10000[3],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hash_in_last_100000[3],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_hash[0],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_hash_in_last_1000[0],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_hash_in_last_10000[0],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_hash_in_last_100000[0],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_hash[1],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_hash_in_last_1000[1],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_hash_in_last_10000[1],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_hash_in_last_100000[1],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_hash[2],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_hash_in_last_1000[2],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_hash_in_last_10000[2],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_hash_in_last_100000[2],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_hash[3],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_hash_in_last_1000[3],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_hash_in_last_10000[3],
    (unsigned long long)xtm_pf_data.num_tbht_lookup_hit_hash_in_last_100000[3]
    );
}

/* Called by xtm_pf_dwe_helper */
void xtm_pf_tbht_dwe_helper(CPUX86State *env, uint64_t tb_exec_nr)
{
    if (option_monitor_tbht != 2) return;

    int do_print_trace = 0;
    do_print_trace = !(tb_exec_nr & ((1 << 17) - 1));

    if (do_print_trace) xtm_pf_tb_hash_table_print();
}

void xtm_pf_tb_hash_table(void *cpu, void *tb, int hash_cmp_nr)
{
    if (!option_monitor_tbht) return;

    /* Should compare hash at least once */
    lsassert(hash_cmp_nr > 0);

    int is_hit = tb == NULL ? 0 : 1;
    int hash_idx = (hash_cmp_nr - 1) & 0x3; /* 0, 1, 2, 3 */
    if (hash_cmp_nr > 4) hash_idx = 3;

    xtm_profile_sys_t *pf = &xtm_pf_data;
    xtm_pf_tbht_data_t *tbhtd = &xtm_pf_tbht_data;
    xtm_pf_tbht_entry_t *tbhte;

    int index = tbhtd->index;
    tbhte = &tbhtd->data[index];

    tbhte->is_valid    = 1;
    tbhte->is_hit      = is_hit;
    tbhte->hash_cmp_nr = hash_cmp_nr;

    pf->num_tbht_lookup += 1;
    pf->num_tbht_lookup_hash[hash_idx] += 1;

    pf->num_tbht_lookup_hash_in_last_1000[hash_idx] += 1;
    pf->num_tbht_lookup_hash_in_last_10000[hash_idx] += 1;
    pf->num_tbht_lookup_hash_in_last_100000[hash_idx] += 1;

    if (is_hit) {
        pf->num_tbht_lookup_hit += 1;
        pf->num_tbht_lookup_hit_hash[hash_idx] += 1;

        pf->num_tbht_lookup_hit_in_last_1000   += 1;
        pf->num_tbht_lookup_hit_in_last_10000  += 1;
        pf->num_tbht_lookup_hit_in_last_100000 += 1;

        pf->num_tbht_lookup_hit_hash_in_last_1000[hash_idx] += 1;
        pf->num_tbht_lookup_hit_hash_in_last_10000[hash_idx] += 1;
        pf->num_tbht_lookup_hit_hash_in_last_100000[hash_idx] += 1;
    }

#define TBHTPF_PROCESS(num) do { \
    if (pf->num_tbht_lookup > (num)) { \
        tbhte = &tbhtd->data[(index - (num)) & TBHTPF_NUM_MASK]; \
        if (tbhte->is_valid) { \
            int idx = (tbhte->hash_cmp_nr - 1) & 0x3; \
            if (tbhte->hash_cmp_nr > 4) idx = 3; \
            pf->num_tbht_lookup_hash_in_last_ ## num [idx] -= 1; \
            if (tbhte->is_hit) {\
                pf->num_tbht_lookup_hit_in_last_ ## num -= 1; \
                pf->num_tbht_lookup_hit_hash_in_last_ ## num [idx] -= 1; \
            } \
        } \
    } \
} while(0)

    TBHTPF_PROCESS(1000);
    TBHTPF_PROCESS(10000);
    TBHTPF_PROCESS(100000);

    tbhtd->index += 1;
    tbhtd->index &= TBHTPF_NUM_MASK;
}

void dump_xtm_tbht_profile(xtm_profile_sys_t *pf)
{
    if (!option_monitor_tbht) return;

    _XTM_MPRINT_("TB Hash Lookup             ", num_tbht_lookup);
    _XTM_MPRINT_("TB Hash Lookup hit         ", num_tbht_lookup_hit);

    _XTM_MPRINT_IDX_("TB Hash Lookup cmp = 1     ", num_tbht_lookup_hash, 0);
    _XTM_MPRINT_IDX_("TB Hash Lookup cmp = 2     ", num_tbht_lookup_hash, 1);
    _XTM_MPRINT_IDX_("TB Hash Lookup cmp = 3     ", num_tbht_lookup_hash, 2);
    _XTM_MPRINT_IDX_("TB Hash Lookup cmp > 4     ", num_tbht_lookup_hash, 3);

    _XTM_MPRINT_IDX_("TB Hash Lookup hit cmp = 1 ", num_tbht_lookup_hit_hash, 0);
    _XTM_MPRINT_IDX_("TB Hash Lookup hit cmp = 2 ", num_tbht_lookup_hit_hash, 1);
    _XTM_MPRINT_IDX_("TB Hash Lookup hit cmp = 3 ", num_tbht_lookup_hit_hash, 2);
    _XTM_MPRINT_IDX_("TB Hash Lookup hit cmp > 4 ", num_tbht_lookup_hit_hash, 3);
}

void dump_xtm_tbht_profile_diff(xtm_profile_sys_t *pf, xtm_profile_sys_t *pr)
{
    if (!option_monitor_tbht) return;

    _XTM_MPRINT_DIFF_("TB Hash Lookup             ", num_tbht_lookup);
    _XTM_MPRINT_DIFF_("TB Hash Lookup hit         ", num_tbht_lookup_hit);

    _XTM_MPRINT_IDX_DIFF_("TB Hash Lookup cmp = 1     ", num_tbht_lookup_hash, 0);
    _XTM_MPRINT_IDX_DIFF_("TB Hash Lookup cmp = 2     ", num_tbht_lookup_hash, 1);
    _XTM_MPRINT_IDX_DIFF_("TB Hash Lookup cmp = 3     ", num_tbht_lookup_hash, 2);
    _XTM_MPRINT_IDX_DIFF_("TB Hash Lookup cmp > 4     ", num_tbht_lookup_hash, 3);

    _XTM_MPRINT_IDX_DIFF_("TB Hash Lookup hit cmp = 1 ", num_tbht_lookup_hit_hash, 0);
    _XTM_MPRINT_IDX_DIFF_("TB Hash Lookup hit cmp = 2 ", num_tbht_lookup_hit_hash, 1);
    _XTM_MPRINT_IDX_DIFF_("TB Hash Lookup hit cmp = 3 ", num_tbht_lookup_hit_hash, 2);
    _XTM_MPRINT_IDX_DIFF_("TB Hash Lookup hit cmp > 4 ", num_tbht_lookup_hit_hash, 3);
}

/* ------------- Privilege Switch CPL3 to CPL0 ------------- */

int xtm_pf_ps30_dump_ctl = 0;

static
void xtm_pf_ps_clear_dump_flag(void)
{
    int i = 0;
    xtm_profile_sys_t *pf = &xtm_pf_data;
    for (i = 0; i < pf->ps30_nr; ++i) {
        struct ps_entry *pse = &pf->ps30[i];
        pse->dump_flag = 0;
    }
}

static
void xtm_pf_ps_check_dump_ctl(void)
{
    if (unlikely(xtm_pf_ps30_dump_ctl)) {
        xtm_pf_ps_clear_dump_flag();
        xtm_pf_ps30_dump_ctl = 0;
    }
}

static
void xtm_pf_ps_check_dump_start(struct ps_entry *pse)
{
    if (option_monitor_ps != 2) return;

    /* Called when CPL3 to CPL0, mark to start dump.
     * Stop check  when CPL0 to CPL3 */
    if (pse->dump_flag == XTM_PF_PS_DUMP_NONE) {
        pse->dump_flag = XTM_PF_PS_DUMP_START;

        option_trace_simple = 1;
        option_dump = 1;
        option_dump_ir1 = 1;
        option_dump_ir2 = 1;
        option_dump_host = 1;

        fprintf(stderr, "[PS] dump start %#x\n", (unsigned int)pse->guest_pc);
    }
}

static
void xtm_pf_ps_check_dump_stop(void)
{
    if (option_monitor_ps != 2) return;

    int i = 0;
    int need_disable_dump = 0;
    xtm_profile_sys_t *pf = &xtm_pf_data;
    for (i = 0; i < pf->ps30_nr; ++i) {
        struct ps_entry *pse = &pf->ps30[i];
        if (pse->dump_flag == XTM_PF_PS_DUMP_START) {
            need_disable_dump = 1;
            pse->dump_flag = XTM_PF_PS_DUMP_DONE;
        }
    }
    if (need_disable_dump) {
        option_trace_simple = 0;
        option_dump = 0;
        option_dump_ir1 = 0;
        option_dump_ir2 = 0;
        option_dump_host = 0;
        fprintf(stderr, "[PS] dump stop\n");
    }
}

static
void xtm_pf_privilege_switch_tr(TranslationBlock *tb_to_tr)
{
    if (option_monitor_ps != 2) return;

    xtm_profile_sys_t *pf = &xtm_pf_data;

    xtm_pf_ps_check_dump_ctl();

    uintptr_t gpc = (uintptr_t)tb_to_tr->pc;
    int curr_tb_cpl = tb_to_tr->flags & 0x3;

    int i;
    struct ps_entry *pse;

    /* From CPL3 to CPL0 */
    if (lsenv->monitor_data.last_tb_cpl == 3 &&
            curr_tb_cpl == 0) {

        for (i = 0; i < pf->ps30_nr; ++i) {
            pse = &pf->ps30[i];
            if (pse->guest_pc == gpc) {
                goto do_check_ps;
            }
        }

        /* Meet a new enter point */
        if (i < (XTMPF_PS30_MAXNUM - 1)) {
            /* We still have enough sapce
             * to store one more enter point */
            pse = &pf->ps30[i];
            pse->guest_pc = gpc;
            pf->ps30_nr += 1;
        } else {
            /* All the enter points
             * that show up 'late' */
            pse = &pf->ps30[XTMPF_PS30_MAXNUM];
        }

do_check_ps:
        xtm_pf_ps_check_dump_start(pse);
    }

    /* From CPL0 to CPL3 */
    if (lsenv->monitor_data.last_tb_cpl == 0 &&
            curr_tb_cpl == 3) {
        xtm_pf_ps_check_dump_stop();
    }
}

static
void xtm_pf_privilege_switch_exec(TranslationBlock *tb_to_exec)
{
    if (!option_monitor_ps) return;

    xtm_profile_sys_t *pf = &xtm_pf_data;

    xtm_pf_ps_check_dump_ctl();

    uintptr_t gpc = (uintptr_t)tb_to_exec->pc;
    int curr_tb_cpl = tb_to_exec->flags & 0x3;

    int i;
    struct ps_entry *pse;

    /* From CPL3 to CPL0 */
    if (lsenv->monitor_data.last_tb_cpl == 3 &&
            curr_tb_cpl == 0) {

        for (i = 0; i < pf->ps30_nr; ++i) {
            pse = &pf->ps30[i];
            if (pse->guest_pc == gpc) {
                goto do_update_ps;
            }
        }

        /* Meet a new enter point */
        if (i < (XTMPF_PS30_MAXNUM - 1)) {
            /* We still have enough sapce
             * to store one more enter point */
            pse = &pf->ps30[i];
            pse->guest_pc = gpc;
            pf->ps30_nr += 1;
        } else {
            /* All the enter points
             * that show up 'late' */
            pse = &pf->ps30[XTMPF_PS30_MAXNUM];
        }

do_update_ps:
        pse->count += 1;
        xtm_pf_ps_check_dump_start(pse);
    }

    /* From CPL0 to CPL3 */
    if (lsenv->monitor_data.last_tb_cpl == 0 &&
        curr_tb_cpl == 3) {
        xtm_pf_ps_check_dump_stop();
    }

    lsenv->monitor_data.last_tb_cpl = curr_tb_cpl;
}

void dump_xtm_ps_profile(
        xtm_profile_sys_t *pf)
{
    int i = 0;

    for (i = 0; i < pf->ps30_nr; ++i) {
        _XTM_MPRINT_PS_("CPL3-CPL0 Enter ", ps30[i].guest_pc, ps30[i].count);
    }
}

void dump_xtm_ps_profile_diff(
        xtm_profile_sys_t *pf,
        xtm_profile_sys_t *pr)
{
    int i = 0;

    for (i = 0; i < pf->ps30_nr; ++i) {
        _XTM_MPRINT_PS_DIFF_("CPL3-CPL0 old Enter ", ps30[i].guest_pc, ps30[i].count);
    }
    for (; i < pr->ps30_nr; ++i) {
        _XTM_MPRINT_PS_("CPL3-CPL0 new Enter ", ps30[i].guest_pc, ps30[i].count);
    }

}

/* ------------------- TB flags ---------------------------- */

static char *tbf_prefix = NULL;
static int tbf_prefix_idx = 0;
static char tbf_name[20][20] = { "null",
    "ljmp pe", "lcall_pe", "ret_pe", "rsm", "smm",
    "cpu_reset", "switch_tss", "sys_enter", "sys_exit",
    "interrupt", "pop_ss", "mov_ss"
};
static void xtm_pf_tbf_update_prefix(void)
{
    tbf_prefix = tbf_name[tbf_prefix_idx];
}

int tbf_prefix_idx_ljmp_pe    = 1;
int tbf_prefix_idx_lcall_pe   = 2;
int tbf_prefix_idx_ret_pe     = 3;
int tbf_prefix_idx_rsm        = 4;
int tbf_prefix_idx_smm        = 5;
int tbf_prefix_idx_cpu_reset  = 6;
int tbf_prefix_idx_switch_tss = 7;
int tbf_prefix_idx_sys_enter  = 8;
int tbf_prefix_idx_sys_exit   = 9;
int tbf_prefix_idx_interrupt  = 10;
int tbf_prefix_idx_pop_ss     = 11;
int tbf_prefix_idx_mov_ss     = 12;

void xtm_pf_tbf_clr_prefix(void)
{
    if (!option_monitor_tbf) return;
    tbf_prefix_idx = 0;
    xtm_pf_tbf_update_prefix();
}

int xtm_pf_tbf_has_prefix(void)
{
    if (!option_monitor_tbf) return 0;
    if (!tbf_prefix_idx) return 0;
    return 1;
}

#define _XTM_PF_DEF_INC_TBF_OP_(field)              \
void xtm_pf_inc_tbf_ ## field(void)                 \
{                                                   \
    if (!option_monitor_tbf) return;                \
    xtm_profile_sys_t *pf = &xtm_pf_data;           \
    pf->op_count.field += 1;                        \
    tbf_prefix_idx = tbf_prefix_idx_ ## field;      \
    xtm_pf_tbf_update_prefix();                     \
}

_XTM_PF_DEF_INC_TBF_OP_(ljmp_pe)
_XTM_PF_DEF_INC_TBF_OP_(lcall_pe)
_XTM_PF_DEF_INC_TBF_OP_(ret_pe)
_XTM_PF_DEF_INC_TBF_OP_(interrupt)
_XTM_PF_DEF_INC_TBF_OP_(rsm)
_XTM_PF_DEF_INC_TBF_OP_(smm)
_XTM_PF_DEF_INC_TBF_OP_(cpu_reset)
_XTM_PF_DEF_INC_TBF_OP_(switch_tss)
_XTM_PF_DEF_INC_TBF_OP_(sys_enter)
_XTM_PF_DEF_INC_TBF_OP_(sys_exit)
_XTM_PF_DEF_INC_TBF_OP_(pop_ss)
_XTM_PF_DEF_INC_TBF_OP_(mov_ss)

#define _XTM_PF_DEF_INC_NATIVE_TBF_OP_(field)       \
void tr_pf_inc_tbf_ ## field(void)                      \
{                                                   \
    if (!option_monitor_tbf) return;                \
    xtm_profile_sys_t *pf = &xtm_pf_data;           \
    ADDR addr = (ADDR)(&(pf->op_count.field));      \
    _XTM_PF_DO_INC_NATIVE_(addr, 1);                \
}

_XTM_PF_DEF_INC_NATIVE_TBF_OP_(ljmp_pe)
_XTM_PF_DEF_INC_NATIVE_TBF_OP_(rsm)
_XTM_PF_DEF_INC_NATIVE_TBF_OP_(pop_ss)
_XTM_PF_DEF_INC_NATIVE_TBF_OP_(mov_ss)
_XTM_PF_DEF_INC_NATIVE_TBF_OP_(smm)
_XTM_PF_DEF_INC_NATIVE_TBF_OP_(cpu_reset)

void xtm_pf_inc_tbf_cs32(int oldv, int newv)
{
    if (!option_monitor_tbf) return;
    xtm_profile_sys_t *pf = &xtm_pf_data;
    
    if (oldv != newv)
        pf->tbf_cs32.change_nr += 1;
    else
        pf->tbf_cs32.unchange_nr += 1;
}

void xtm_pf_inc_tbf_ss32(int oldv, int newv)
{
    if (!option_monitor_tbf) return;
    xtm_profile_sys_t *pf = &xtm_pf_data;

    if (oldv != newv)
        pf->tbf_ss32.change_nr += 1;
    else
        pf->tbf_ss32.unchange_nr += 1;
}

/* Called in QEMU when SS32 or CS32 might change */
void xtm_pf_dump_tbf_short(void)
{
    if (option_monitor_tbf != 2) return;
    xtm_profile_sys_t *pf = &xtm_pf_data;

    fprintf(stderr, "%-10s : %lld, %lld, \
%lld, %lld, %lld, %lld, %lld, %lld, \
%lld, %lld, %lld, %lld, \
%lld, %lld, %lld, %lld\n",
    tbf_prefix,
    (unsigned long long)pf->op_count.ljmp_pe,
    (unsigned long long)pf->op_count.lcall_pe,

    (unsigned long long)pf->op_count.interrupt,
    (unsigned long long)pf->op_count.ret_pe,

    (unsigned long long)pf->op_count.smm,
    (unsigned long long)pf->op_count.rsm,

    (unsigned long long)pf->op_count.sys_enter,
    (unsigned long long)pf->op_count.sys_exit,

    (unsigned long long)pf->op_count.pop_ss,
    (unsigned long long)pf->op_count.mov_ss,

    (unsigned long long)pf->op_count.cpu_reset,
    (unsigned long long)pf->op_count.switch_tss,

    (unsigned long long)pf->tbf_cs32.change_nr,
    (unsigned long long)pf->tbf_cs32.unchange_nr,
    (unsigned long long)pf->tbf_ss32.change_nr,
    (unsigned long long)pf->tbf_ss32.unchange_nr);

}

void dump_xtm_tbf_profile(xtm_profile_sys_t *pf)
{
    if (!option_monitor_tbf) return;

    _XTM_MPRINT_("TB Flags OP Count ljmp  pe   ", op_count.ljmp_pe);
    _XTM_MPRINT_("TB Flags OP Count lcall pe   ", op_count.lcall_pe);
    _XTM_MPRINT_("TB Flags OP Count ret   pe   ", op_count.ret_pe);
    _XTM_MPRINT_("TB Flags OP Count interrupt  ", op_count.interrupt);
    _XTM_MPRINT_("TB Flags OP Count rsm        ", op_count.rsm);
    _XTM_MPRINT_("TB Flags OP Count pop ss     ", op_count.pop_ss);
    _XTM_MPRINT_("TB Flags OP Count mov ss     ", op_count.mov_ss);
    _XTM_MPRINT_("TB Flags OP Count smm        ", op_count.smm);
    _XTM_MPRINT_("TB Flags OP Count cpu reset  ", op_count.cpu_reset);
    _XTM_MPRINT_("TB Flags OP Count TSS switch ", op_count.switch_tss);
    _XTM_MPRINT_("TB Flags OP Count TSS switch ", op_count.sys_enter);
    _XTM_MPRINT_("TB Flags OP Count TSS switch ", op_count.sys_exit);

    _XTM_MPRINT_("TB Flags CS32   change       ", tbf_cs32.change_nr);
    _XTM_MPRINT_("TB Flags CS32 unchange       ", tbf_cs32.unchange_nr);
    _XTM_MPRINT_("TB Flags SS32   change       ", tbf_ss32.change_nr);
    _XTM_MPRINT_("TB Flags SS32 unchange       ", tbf_ss32.unchange_nr);
}

/* ------------------- Main Dump Function ------------------ */

void dump_xtm_profile(void)
{
    memcpy(&xtm_pf_data_rcd, &xtm_pf_data, sizeof(xtm_profile_sys_t));
    xtm_profile_sys_t *pf = &xtm_pf_data_rcd;

    dump_xtm_sc_profile(pf);
    dump_xtm_tc_profile(pf);
    dump_xtm_rs_profile(pf);
    dump_xtm_ts_profile(pf);

    dump_xtm_cs_profile(pf);
    dump_xtm_jc_profile(pf);
    dump_xtm_tbht_profile(pf);
    dump_xtm_ps_profile(pf);

    dump_xtm_tbf_profile(pf);
}

void dump_xtmdiff_profile(void)
{
    xtm_profile_sys_t *pf = &xtm_pf_data;
    xtm_profile_sys_t *pr = &xtm_pf_data_rcd;

    dump_xtm_sc_profile_diff(pf, pr);
    dump_xtm_tc_profile_diff(pf, pr);
    dump_xtm_rs_profile_diff(pf, pr);
    dump_xtm_ts_profile_diff(pf, pr);

    dump_xtm_tbht_profile_diff(pf, pr);
    dump_xtm_ps_profile_diff(pf, pr);
}

/* Dump while execution
 * 
 * Basic design:
 *   > count the number of TB's execution
 *     call specific dump function
 *     after a number of TB's execution
 *
 * Current configure:
 *   > dump function will be called
 *     after every 2^16 TB's execution
 *
 * Current dump function:
 *   > monitor JC   : xtm_pf_jc_dwe_helper
 *     >> enabled when option_monitor_jc == 3
 *   > monitor TBHT : xtm_pf_trbh_dwe_helper
 *     >> enabled when option_monitor_tbht == 2
 * */

int xtm_pf_need_dwe(void)
{
    return option_monitor_jc   == 3 ||
           option_monitor_tbht == 2;
}

/* Called from native code */
void xtm_pf_dwe_helper(CPUX86State *env)
{
    uint64_t tb_exec_nr = lsenv->monitor_data.total_tb_executed;
    xtm_pf_jc_dwe_helper(env, tb_exec_nr);
    xtm_pf_tbht_dwe_helper(env, tb_exec_nr);
}

/* Called at the beginning of tr_gen_tb_start */
void tr_pf_dwe(void)
{
    if (!xtm_pf_need_dwe()) return;

    uintptr_t counter_addr = (uintptr_t)&lsenv->monitor_data.total_tb_executed;

    IR2_OPND num = ra_alloc_itemp();
    IR2_OPND mem = ra_alloc_itemp();
    IR2_OPND cmp = ra_alloc_itemp();

    load_addr_to_ir2(&mem, (ADDR)counter_addr);
    append_ir2_opnd2i(LISA_LD_D,   &num, &mem, 0);
    append_ir2_opnd2i(LISA_ADDI_D, &num, &num, 1);
    append_ir2_opnd2i(LISA_ST_D,   &num, &mem, 0);

    IR2_OPND go_on_exec = ir2_opnd_new_label();

    append_ir2_opnd2_(lisa_mov16z, &cmp, &num);
    append_ir2_opnd2(LISA_BNEZ, &cmp, &go_on_exec);

    tr_gen_call_to_helper1((ADDR)xtm_pf_dwe_helper, 0);

    /* go on executing TB */
    append_ir2_opnd1(LISA_LABEL, &go_on_exec);

    ra_free_temp(&num);
    ra_free_temp(&mem);
    ra_free_temp(&cmp);
}

/* Function to be called in different places */

void xtm_pf_step_translate_tb(void *tb)
{
    xtm_pf_privilege_switch_tr(tb);
}

void xtm_pf_step_before_exec_tb(void *_tb)
{
    TranslationBlock *tb = _tb;

    /* Prilivage Switch */
    xtm_pf_privilege_switch_exec(tb);

    uint8_t fast_cs_mask = tb->extra_tb->fast_cs_mask;

    /* SC.context switch with FPU/XMM */
    if (fast_cs_mask & XTM_FAST_CS_MASK_FPU) {
        xtm_pf_inc_cs_in_fpu();
    }
    if (fast_cs_mask & XTM_FAST_CS_MASK_XMM) {
        xtm_pf_inc_cs_in_xmm();
    }

    /* For monitor CS */
    xtm_pf_tb_exec_record(tb);

    static uint64_t tb_exec_no_link = 0;
    tb_exec_no_link += 1;

    if (option_monitor_cs == 2) {
        if (!(tb_exec_no_link % 100000)) {

#define CSPF_TRACE_PRINT(num) do { \
fprintf(stderr, "%.4f, %.4f, %.4f, %lld, %lld, ", \
(double)xtm_pf_data.expand_ratio_ ## num, \
(double)xtm_pf_data.expand_ratio_ ## num ## _cpl0, \
(double)xtm_pf_data.expand_ratio_ ## num ## _cpl3, \
(unsigned long long)xtm_pf_data.last_ ## num ## _cpl0_tb_nr, \
(unsigned long long)xtm_pf_data.last_ ## num ## _cpl3_tb_nr); \
} while(0)

            CSPF_TRACE_PRINT(1000);
            CSPF_TRACE_PRINT(10000);
            CSPF_TRACE_PRINT(100000);
            fprintf(stderr, "\n");
        }
    }

    /* TC.exe_tb record start time */
    xtm_pf_tc_exe_tb_st();
}

void xtm_pf_step_after_exec_tb(void)
{
    /* TC.exe_tb record stop time */
    xtm_pf_tc_exe_tb_ed();
}

void xtm_pf_step_cpu_loop_exit(void)
{
    CPUX86State *env = lsenv->cpu_state;

    /* TC.exe_tb record stop time */
    xtm_pf_tc_exe_tb_ed();

    /* JC. mark next Jmp Cache Lookup is_excp */
    if (option_monitor_jc) {
        env->xtm_pf_data.jc.is_excp = 1;
    }

    xtm_pf_tbf_clr_prefix();
}

void xtm_pf_step_gen_tb_start(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;

    /* DWE support */
    tr_pf_dwe();

    /* SC.num_tb_exec */
    tr_pf_inc_tb_exec();

    /* SC.num_tb_use_fpu/xmm_exec */
    uint8_t fast_cs_mask = tb->extra_tb->fast_cs_mask;
    if (fast_cs_mask & XTM_FAST_CS_MASK_FPU) {
        tr_pf_inc_tb_use_fpu_exec();
    }
    if (fast_cs_mask & XTM_FAST_CS_MASK_XMM) {
        tr_pf_inc_tb_use_xmm_exec();
    }

    /* SC.num_tb_cplN_exec */
    switch (td->sys.cpl) {
    case 0: tr_pf_inc_tb_cpl0_exec(); break;
    case 1: tr_pf_inc_tb_cpl1_exec(); break;
    case 2: tr_pf_inc_tb_cpl2_exec(); break;
    case 3: tr_pf_inc_tb_cpl3_exec(); break;
    default: break;
    }
    
    /* JC. clear jmpin flag */
    if (option_monitor_jc) {
        append_ir2_opnd2i(LISA_ST_B, &zero_ir2_opnd, &env_ir2_opnd,
                lsenv_offset_of_pf_data_jc_is_jmpdr(lsenv));
        append_ir2_opnd2i(LISA_ST_B, &zero_ir2_opnd, &env_ir2_opnd,
                lsenv_offset_of_pf_data_jc_is_jmpin(lsenv));
        append_ir2_opnd2i(LISA_ST_B, &zero_ir2_opnd, &env_ir2_opnd,
                lsenv_offset_of_pf_data_jc_is_sys_eob(lsenv));
        append_ir2_opnd2i(LISA_ST_B, &zero_ir2_opnd, &env_ir2_opnd,
                lsenv_offset_of_pf_data_jc_is_excp(lsenv));
    }
}

#else

/* Not configure with x86tomips-profile, generate empty function */

#define XTM_PF_TIMEPSEC_FN(group, name, op) \
    void xtm_pf_tc_ ## group ## _ ## name ## _ ## op (void) {} \

XTM_PF_TIMEPSEC_FN(exe, all, st)
XTM_PF_TIMEPSEC_FN(exe, all, ed)
XTM_PF_TIMEPSEC_FN(exe, tb, st)
XTM_PF_TIMEPSEC_FN(exe, tb, ed)
XTM_PF_TIMEPSEC_FN(tr, all, st)
XTM_PF_TIMEPSEC_FN(tr, all, ed)
XTM_PF_TIMEPSEC_FN(tr, disasm, st)
XTM_PF_TIMEPSEC_FN(tr, disasm, ed)
XTM_PF_TIMEPSEC_FN(tr, translate, st)
XTM_PF_TIMEPSEC_FN(tr, translate, ed)
XTM_PF_TIMEPSEC_FN(tr, assemble, st)
XTM_PF_TIMEPSEC_FN(tr, assemble, ed)

#define _XTM_PF_DEF_INC_NATIVE_(field) \
    void tr_pf_inc_ ## field(void) { }

_XTM_PF_DEF_INC_NATIVE_(tb_exec);
_XTM_PF_DEF_INC_NATIVE_(tb_use_fpu_exec);
_XTM_PF_DEF_INC_NATIVE_(tb_use_xmm_exec);
_XTM_PF_DEF_INC_NATIVE_(tb_cpl0_exec);
_XTM_PF_DEF_INC_NATIVE_(tb_cpl1_exec);
_XTM_PF_DEF_INC_NATIVE_(tb_cpl2_exec);
_XTM_PF_DEF_INC_NATIVE_(tb_cpl3_exec);
_XTM_PF_DEF_INC_NATIVE_(cs_in);
_XTM_PF_DEF_INC_NATIVE_(cs_out);
_XTM_PF_DEF_INC_NATIVE_(cs_in_fpu);
_XTM_PF_DEF_INC_NATIVE_(cs_in_xmm);
_XTM_PF_DEF_INC_NATIVE_(helper);
_XTM_PF_DEF_INC_NATIVE_(helper_allgpr);
_XTM_PF_DEF_INC_NATIVE_(helper_fpu);
_XTM_PF_DEF_INC_NATIVE_(helper_xmm);
_XTM_PF_DEF_INC_NATIVE_(tlb_hit);
_XTM_PF_DEF_INC_NATIVE_(tlb_miss);
_XTM_PF_DEF_INC_NATIVE_(fpu_rotate_native);
_XTM_PF_DEF_INC_NATIVE_(excp);
_XTM_PF_DEF_INC_NATIVE_(fastcs_glue_01);
_XTM_PF_DEF_INC_NATIVE_(fastcs_glue_10);
_XTM_PF_DEF_INC_NATIVE_(fastcs_glue_11);

#define _XTM_PF_DEF_INC_(field) \
    void xtm_pf_inc_ ## field (void) { }

_XTM_PF_DEF_INC_(tb_exec);
_XTM_PF_DEF_INC_(tb_use_fpu_exec);
_XTM_PF_DEF_INC_(tb_use_xmm_exec);
_XTM_PF_DEF_INC_(tb_cpl0_exec);
_XTM_PF_DEF_INC_(tb_cpl1_exec);
_XTM_PF_DEF_INC_(tb_cpl2_exec);
_XTM_PF_DEF_INC_(tb_cpl3_exec);
_XTM_PF_DEF_INC_(cs_in);
_XTM_PF_DEF_INC_(cs_out);
_XTM_PF_DEF_INC_(cs_in_fpu);
_XTM_PF_DEF_INC_(cs_in_xmm);
_XTM_PF_DEF_INC_(helper);
_XTM_PF_DEF_INC_(helper_allgpr);
_XTM_PF_DEF_INC_(helper_fpu);
_XTM_PF_DEF_INC_(helper_xmm);
_XTM_PF_DEF_INC_(tlb_hit);
_XTM_PF_DEF_INC_(tlb_miss);
_XTM_PF_DEF_INC_(fpu_rotate_bt);
_XTM_PF_DEF_INC_(excp);

_XTM_PF_DEF_INC_(rs_ra_alloc_int_nr);
_XTM_PF_DEF_INC_(rs_ra_alloc_fp_nr);
_XTM_PF_DEF_INC_(rs_ra_free_int_nr);
_XTM_PF_DEF_INC_(rs_ra_free_fp_nr);

void xtm_pf_inc_rs_ra_int_used_max(int num) {}
void xtm_pf_inc_rs_ra_fp_used_max(int num) {}

_XTM_PF_DEF_INC_(ts_ins_translated);
_XTM_PF_DEF_INC_(ts_tb_translated);

void xtm_pf_tb_exec_record(void *tb) {}

void xtm_pf_jmp_cache(void *cpu, void *tb) {}
void xtm_pf_inc_jc_clear(void *cpu) {}
void xtm_pf_inc_jc_clear_page(void *cpu, uint64_t page_addr) {}

void xtm_pf_tb_hash_table(void *cpu, void *tb, int hash_cmp_nr) {}

void dump_xtm_profile(void)
{
    qemu_printf("No profile configured. Rerun configure with '--x86tomips-profile'\n");
}
void dump_xtmdiff_profile(void)
{
    qemu_printf("No profile configured. Rerun configure with '--x86tomips-profile'\n");
}

void dump_xtm_sc_profile(xtm_profile_sys_t *pf) {}
void dump_xtm_tc_profile(xtm_profile_sys_t *pf) {}
void dump_xtm_rs_profile(xtm_profile_sys_t *pf) {}
void dump_xtm_ts_profile(xtm_profile_sys_t *pf) {}
void dump_xtm_cs_profile(xtm_profile_sys_t *pf) {}
void dump_xtm_jc_profile(xtm_profile_sys_t *pf) {}

void dump_xtm_sc_profile_diff(xtm_profile_sys_t *pf, xtm_profile_sys_t *pr) {}
void dump_xtm_tc_profile_diff(xtm_profile_sys_t *pf, xtm_profile_sys_t *pr) {}
void dump_xtm_rs_profile_diff(xtm_profile_sys_t *pf, xtm_profile_sys_t *pr) {}
void dump_xtm_ts_profile_diff(xtm_profile_sys_t *pf, xtm_profile_sys_t *pr) {}

void xtm_pf_step_before_exec_tb(void *tb) {}
void xtm_pf_step_after_exec_tb(void) {}
void xtm_pf_step_cpu_loop_exit(void) {}
void xtm_pf_step_gen_tb_start(void) {}

void xtm_pf_inc_tbf_cs32(int oldv, int newv) {}
void xtm_pf_inc_tbf_ss32(int oldv, int newv) {}

#endif
#endif
