#ifndef _LATX_OPTIONS_H_
#define _LATX_OPTIONS_H_

#include "latx-types.h"

extern int option_by_hand;
#ifdef CONFIG_LATX_FLAG_PATTERN
extern int option_flag_pattern;
#endif
#ifdef CONFIG_LATX_FLAG_REDUCTION
extern int option_flag_reduction;
#endif
extern int option_dump;
extern int option_dump_host;
extern int option_dump_ir1;
extern int option_dump_ir2;
extern int option_trace_tb;
extern int option_trace_ir1;
extern int option_check;
extern int option_tb_link;
extern int option_shadow_stack;
extern int option_lsfpu;
extern int option_xmm128map;
extern int option_ibtc;

extern unsigned long long counter_tb_exec;
extern unsigned long long counter_tb_tr;

extern unsigned long long counter_ir1_tr;
extern unsigned long long counter_mips_tr;

void options_init(void);
void options_parse_dump(const char *bits);
void options_parse_trace(const char *bits);
uint8 options_to_save(void);

#ifdef CONFIG_SOFTMMU

/* For debug in softmmu */
extern int option_smmu_slow;
extern int option_break_point;
extern unsigned long long option_break_point_addrx;
extern unsigned long long option_break_point_count;
extern int option_native_printer;
extern int option_test_sys;
extern int option_tb_max_insns;
int latxs_tb_max_insns(void);

/* For generate TB execution trace */
extern int option_trace_simple;
extern unsigned long long option_trace_start_nr;
extern unsigned long long option_trace_start_tb;
extern int option_trace_start_tb_set;

/* Optimization in softmmu */
extern int option_staticcs;
extern int option_large_code_cache;
extern int option_soft_fpu;
extern int option_fast_fpr_ldst;
extern int option_njc;
extern int option_sigint;
extern int option_cross_page_check;
extern int option_cross_page_jmp_link;
extern int option_intb_link;
extern int option_by_hand_64;

/* For QEMU monitor in softmmu */
extern int option_monitor_sc; /* Simple Counter */
extern int option_monitor_tc; /* Timer  Counter */
extern int option_monitor_rs; /* Register    Stat */
extern int option_monitor_ts; /* Translation Stat */

extern int option_monitor_cs; /* Complex Stat */
extern int option_monitor_jc; /* Jmp Cache Stat */
extern int option_monitor_tbht; /* TB Hash Table Stat */
extern int option_monitor_ps; /* Prilivage Switch */

extern int option_monitor_tbf; /* TB.flags */

int sigint_enabled(void);

#endif /* SOFTMMU */

#endif
