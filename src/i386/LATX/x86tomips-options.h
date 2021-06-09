#ifndef _X86toMIPS_OPTIONS_H_
#define _X86toMIPS_OPTIONS_H_

#include "include/types.h"

#define XTM_OPT_by_hand          0
#define XTM_OPT_flag_pattern     1
#define XTM_OPT_flag_reduction   2
#define XTM_OPT_dump             3
#define XTM_OPT_dump_host        4
#define XTM_OPT_dump_ir1         5
#define XTM_OPT_dump_ir2         6
#define XTM_OPT_trace_simple     7
#define XTM_OPT_trace_tb         8
#define XTM_OPT_trace_tb_ir1     9
#define XTM_OPT_trace_ir1_type   10
#define XTM_OPT_check            11
#define XTM_OPT_tb_link          12
#define XTM_OPT_lbt              13
#define XTM_OPT_shadow_stack     14
#define XTM_OPT_lsfpu            15
#define XTM_OPT_profile          16
#define XTM_OPT_break_point      17
#define XTM_OPT_debugcall        18
#define XTM_OPT_TEST             19
#define XTM_OPT_staticcs         20
#define XTM_OPT_risk             21
#define XTM_OPT_tb_lookup        22
#define XTM_OPT_sigint           23
#define XTM_OPT_njc              24
#define XTM_OPT_branch_opt       25
#define XTM_OPT_cross_page_check 26

extern int option_by_hand;
extern int option_flag_pattern;
extern int option_flag_reduction;
extern int option_dump;
extern int option_dump_host;
extern int option_dump_ir1;
extern int option_dump_ir2;
extern int option_dump_itemp;
extern int xtm_trace_enabled(void);
extern int option_trace_tb;
extern int option_trace_tb_ir1;
extern int option_trace_ir1_type;
extern uint8 *sys_trace_ir1_type;
extern int option_trace_ir1_id;
extern int option_trace_simple;
extern unsigned long long option_trace_start_nr;
extern unsigned long long option_trace_start_tb;
extern int option_trace_start_tb_set;
extern int option_check;
extern int option_tb_link;
extern int option_tb_lookup;
#ifndef _XTM_TBLOOKUP_OPT_
#define _XTM_TBLOOKUP_OPT_
extern int xtm_tblookup_opt(void);
#endif
extern int option_lbt;
extern int option_shadow_stack;
extern int option_lsfpu;
extern int xtm_lsfpu_opt(void);
extern int option_staticcs;
extern int option_risk;
extern int xtm_risk_opt(void);
extern int option_sigint;
extern int xtm_sigint_opt(void);
extern int xtm_njc_opt(void);
extern int branch_opt;
extern int xtm_branch_opt(void);
extern int option_profile;
#ifdef CONFIG_DEBUG_CHECK
extern int option_debugcall;
#endif
extern int option_xmm128map;
extern int option_break_point;
extern unsigned long long option_break_point_addrx;
extern unsigned long long option_break_point_count;
extern int option_test;
extern int option_test_type;

extern int option_cross_page_check;
extern int xtm_cpc_enabled(void);

#define XTM_TEST_TYPE_TRANSLATION   0
#define XTM_TEST_TYPE_RUN_TB        1
#define XTM_TEST_TYPE_MAX           2

extern unsigned long long counter_tb_exec;
extern unsigned long long counter_tb_tr;

extern unsigned long long counter_ir1_tr;
extern unsigned long long counter_mips_tr;

extern int option_cpusinfo;

extern int option_monitor_sc;
extern int option_monitor_tc;
extern int option_monitor_rs;
extern int option_monitor_ts;
extern int option_monitor_cs;
extern int option_monitor_jc;
extern int option_monitor_tbht;
extern int option_monitor_ps;
extern int option_monitor_tbf;

void options_init(void);
void options_parse_dump(const char *bits);
void options_parse_trace(const char *bits);
void options_parse_cpusinfo(const char *bits);
void options_parse_monitor(const char *bits);
void options_parse_bpc(unsigned long long addrx);
void options_parse_bpn(unsigned long long tbnum);
void options_parse_tracestart_nr(unsigned long long ts);
void options_parse_tracestart_tb(unsigned long long ts);
void options_parse_trace_ir1_id(unsigned long long id);
void options_parse_test(const char *type);
uint8 options_to_save(void);

void option_trace_ir1(void *pir1);

void dump_xtm_options(void);

#endif
