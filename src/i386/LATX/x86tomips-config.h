#ifndef _X86toMIPS_CONFIG_H_
#define _X86toMIPS_CONFIG_H_

#include "include/types.h"
#include "include/qemu-def.h"
#include "include/xtm-qemu-config.h"

int whether_print_cpu_info(int cpu_index);

/* lsenv is a thread-local global variable */
void x86_to_mips_alloc_lsenv(void);
void x86_to_mips_free_lsenv(void);
void x86_to_mips_init_env(CPUArchState *env);
void x86_to_mips_init_thread_signal(CPUState *cpu);

int target_x86_to_mips_host(
        CPUState *cpu,
        TranslationBlock *tb,
        int max_insns,
        void *code_hightwater,
        int *search_size);

int target_x86_to_mips_break_point_code(void *code_buf_addr);
#ifdef CONFIG_XTM_USE_LOONGEXT
int target_x86_to_mips_check_loongext(void *code_buf);
#endif

int target_x86_to_mips_static_codes(void *code_buf);

void x86_to_mips_tb_set_jmp_target(TranslationBlock *, int, TranslationBlock *);

void x86_to_mips_before_exec_tb(CPUState *cpu, struct TranslationBlock *tb);
void x86_to_mips_after_exec_tb(CPUArchState *env, struct TranslationBlock *tb);

#ifdef CONFIG_SOFTMMU
void x86_to_mips_fix_after_excp_or_int(void);
int xtm_get_top_bias_from_env(CPUX86State *env);
#endif

extern ADDR context_switch_bt_to_native;
extern ADDR context_switch_native_to_bt_ret_0;
extern ADDR context_switch_native_to_bt;
extern ADDR xqm_break_point_code;

extern ADDR ss_match_fail_native;
#ifdef CONFIG_SOFTMMU
extern ADDR sys_helper_prologue_default;
extern ADDR sys_helper_epilogue_default;
#endif

void trace_tb_execution(CPUState *cpu, struct TranslationBlock *tb);

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_BTMMU)
void* build_func_xqm(void);
void* build_data_xqm(void);
#endif

ADDRX linear_address(ADDRX pc);
#endif /* _X86toMIPS_CONFIG_H_ */
