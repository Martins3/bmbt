#include "x86tomips-options.h"
#include "include/error.h"
#include "ir1/ir1.h"
#include "include/common.h"
#include <string.h>

#if 0
int option_by_hand;
int option_flag_pattern;
int option_flag_reduction;
int option_dump;
int option_dump_host;
int option_dump_ir1;
int option_dump_ir2;
int option_dump_itemp;
int option_trace_tb;
int option_trace_tb_ir1;
int option_trace_ir1_type;
uint8 *sys_trace_ir1_type;
int option_trace_ir1_id;
int option_trace_simple;
unsigned long long option_trace_start_nr;
unsigned long long option_trace_start_tb;
int option_trace_start_tb_set;
int option_check;
int option_tb_link;
int option_tb_lookup;
int option_lbt;
int option_shadow_stack;
int option_lsfpu;
int option_staticcs;
int option_risk;
int option_branch_opt;
int option_sigint;
int option_njc;
int option_profile;
#ifdef CONFIG_DEBUG_CHECK
int option_debugcall;
#endif
int option_xmm128map;
int option_break_point;
unsigned long long option_break_point_addrx;
unsigned long long option_break_point_count;
int option_test;
int option_test_type;

int option_cross_page_check;

int option_monitor_sc; /* Simple Counter */
int option_monitor_tc; /* Timer  Counter */
int option_monitor_rs; /* Register    Stat */
int option_monitor_ts; /* Translation Stat */

int option_monitor_cs; /* Complex Stat */
int option_monitor_jc; /* Jmp Cache Stat */
int option_monitor_tbht; /* TB Hash Table Stat */
int option_monitor_ps; /* Prilivage Switch */

int option_monitor_tbf; /* TB.flags */

unsigned long long counter_tb_exec;
unsigned long long counter_tb_tr;

unsigned long long counter_ir1_tr;
unsigned long long counter_mips_tr;

int option_cpusinfo = -1;

uint8 __sys_trace_ir1_type[X86_INS_ENDING + 1] = {0};

int xtm_trace_enabled(void)
{
    return option_trace_simple ||
           option_break_point ||
           option_trace_tb ||
           option_trace_tb_ir1;
}

int xtm_risk_opt(void)
{
    return option_risk;
}

int xtm_branch_opt(void)
{
    return option_branch_opt;
}

int xtm_tblookup_opt(void)
{
    return option_tb_lookup;
}

int xtm_sigint_opt(void)
{
    return option_sigint;
}

int xtm_njc_opt(void)
{
    return option_njc;
}

int xtm_lsfpu_opt(void)
{
    return option_lsfpu;
}

int xtm_cpc_enabled(void)
{
    return option_cross_page_check;
}

void options_init(void)
{
    option_dump_host = 0;
    option_dump_ir1 = 0;
    option_dump_ir2 = 0;
    option_dump = 0;
    option_dump_itemp = 0;

    option_trace_tb = 0;
    option_trace_tb_ir1 = 0;
    option_trace_ir1_type = 0;
    option_trace_ir1_id = 0;
    sys_trace_ir1_type = __sys_trace_ir1_type;

    option_trace_simple = 0;
    option_trace_start_nr = 0;
    option_trace_start_tb = 0;
    option_trace_start_tb_set = 0;

    option_monitor_sc = 0;
    option_monitor_tc = 0;
    option_monitor_rs = 0;
    option_monitor_ts = 0;
    option_monitor_cs = 0;
    option_monitor_jc = 0;
    option_monitor_tbht = 0;
    option_monitor_ps = 0;
    option_monitor_tbf = 0;

    option_flag_pattern = 0;
    option_flag_reduction = 0;
    option_shadow_stack = 0;

    option_xmm128map = 1;
    option_by_hand = 0;
    option_tb_link = 0;
    option_tb_lookup = 0;
    option_lbt = 0;
    option_lsfpu = 0;
    option_staticcs = 0;
    option_risk = 0;
    option_branch_opt = 0;
    option_sigint = 0;

    /* TODO: should enable this by default */
    option_cross_page_check = 0;

    option_check = 0;
    option_break_point = 0;
    option_break_point_addrx = 0;
    option_break_point_count = 1;

    option_test = 1;

    counter_tb_exec = 0;
    counter_tb_tr = 0;

    counter_ir1_tr = 0;
    counter_mips_tr = 0;
}

#define OPTIONS_DUMP_FUNC 0
#define OPTIONS_DUMP_IR1 1
#define OPTIONS_DUMP_IR2 2
#define OPTIONS_DUMP_HOST 3
#define OPTIONS_DUMP_ITEMP 4

void options_parse_dump(const char *bits)
{
    if (!bits) {
        return;
    }

    int len = strlen(bits);

    if (len && bits[OPTIONS_DUMP_FUNC] == '1') {
        option_dump = 1;
    } else if (bits[OPTIONS_DUMP_FUNC] == '0') {
        option_dump = 0;
    } else {
        lsassertm(0, "wrong options for dump func.");
    }

    if (len - 1 && bits[OPTIONS_DUMP_IR1] == '1') {
        option_dump_ir1 = 1;
    } else if (bits[OPTIONS_DUMP_IR1] == '0') {
        option_dump_ir1 = 0;
    } else {
        lsassertm(0, "wrong options for dump ir1.");
    }

    if (len - 2 && bits[OPTIONS_DUMP_IR2] == '1') {
        option_dump_ir2 = 1;
    } else if (bits[OPTIONS_DUMP_IR2] == '0') {
        option_dump_ir2 = 0;
    } else {
        lsassertm(0, "wrong options for dump ir2.");
    }

    if (len - 3 && bits[OPTIONS_DUMP_HOST] == '1') {
        option_dump_host = 1;
    } else if (bits[OPTIONS_DUMP_HOST] == '0') {
        option_dump_host = 0;
    } else {
        lsassertm(0, "wrong options for dump host.");
    }

    if (len - 4 && bits[OPTIONS_DUMP_ITEMP] == '1') {
        option_dump_itemp = 1;
    } else if (bits[OPTIONS_DUMP_ITEMP] == '0') {
        option_dump_itemp = 0;
    } else {
        lsassertm(0, "wrong options for dump itemp.");
    }
}

#define OPTIONS_TRACE_SIMPLE 0
#define OPTIONS_TRACE_TB  1
#define OPTIONS_TRACE_TB_IR1 2
#define OPTIONS_TRACE_IR1_TYPE 3

void options_parse_trace(const char *bits)
{
    if (!bits) {
        return;
    }

    int len = strlen(bits);

    if (len >= 1) {
        char b = bits[OPTIONS_TRACE_SIMPLE];
        switch (b) {
        case '2': /* Print with FPU state */
        case '1': /* CPU and GPR state */
        case '0': /* Not enable */
            option_trace_simple = b - '0';
            break;
        default:
            lsassertm(0, "wrong options for trace simple %c.", b);
            break;
        }
    }

    if (len >= 2) {
        if (bits[OPTIONS_TRACE_TB] == '1') {
            option_trace_tb = 1;
        } else if (bits[OPTIONS_TRACE_TB] == '0') {
            option_trace_tb = 0;
        } else {
            lsassertm(0, "wrong options for trace tb.");
        }
    }

    if (len >= 3) {
        if (bits[OPTIONS_TRACE_TB_IR1] == '1') {
            option_trace_tb_ir1 = 1;
        } else if (bits[OPTIONS_TRACE_TB_IR1] == '0') {
            option_trace_tb_ir1 = 0;
        } else {
            lsassertm(0, "wrong options for trace ir1 .");
        }
    }

    if (len >= 4) {
        if (bits[OPTIONS_TRACE_IR1_TYPE] == '1') {
            option_trace_ir1_type = 1;
        } else if (bits[OPTIONS_TRACE_IR1_TYPE] == '0') {
            option_trace_ir1_type = 0;
        } else {
            lsassertm(0, "wrong options for trace ir1 .");
        }
    }
}

void options_parse_cpusinfo(const char *bits)
{
    if (!bits) {
        return;
    }

    int len = strlen(bits);
    option_cpusinfo = 0;
    int i;
    int mask = 1;
    for(i=0; i<len; i++)
    {
        if(bits[i]=='1')
            option_cpusinfo |= mask;
        mask = mask << 1;
    }
}

void options_parse_monitor(const char *bits)
{
    if (!bits) {
        return;
    }

    int len = strlen(bits);

#define OPTION_MONITOR_PARSE(idx, var) \
    if (len >= (idx+ 1)) { \
        switch (bits[idx]) { \
        case '7': var = 7; break; \
        case '6': var = 6; break; \
        case '5': var = 5; break; \
        case '4': var = 4; break; \
        case '3': var = 3; break; \
        case '2': var = 2; break; \
        case '1': var = 1; break; \
        case '0': var = 0; break; \
        default: break; \
        } \
    }
OPTION_MONITOR_PARSE(0, option_monitor_sc)
OPTION_MONITOR_PARSE(1, option_monitor_tc)
OPTION_MONITOR_PARSE(2, option_monitor_rs)
OPTION_MONITOR_PARSE(3, option_monitor_ts)

OPTION_MONITOR_PARSE(4, option_monitor_cs)
OPTION_MONITOR_PARSE(5, option_monitor_jc)
OPTION_MONITOR_PARSE(6, option_monitor_tbht)
OPTION_MONITOR_PARSE(7, option_monitor_ps)

OPTION_MONITOR_PARSE(8, option_monitor_tbf)
}

void options_parse_bpc(unsigned long long addrx)
{
    option_break_point = 1;
    option_break_point_addrx = addrx;
}

void options_parse_bpn(unsigned long long tbnum)
{
    option_break_point_count = tbnum;
}

void options_parse_tracestart_nr(unsigned long long ts)
{
    option_trace_start_nr = ts;
}

void options_parse_tracestart_tb(unsigned long long tbpc)
{
    option_trace_start_tb = tbpc;
    option_trace_start_tb_set = 1;
}

void options_parse_trace_ir1_id(unsigned long long id)
{
    option_trace_ir1_id = id;
}

static const char *xtm_test_type_str[XTM_TEST_TYPE_MAX] = {
    "dump-inst-translation",
    "running-ont-tb"
};

uint8 options_to_save(void)
{
    uint8 option_bitmap = 0;
    if (option_shadow_stack)
        option_bitmap |= 0x10;
    return option_bitmap;
}

void option_trace_ir1(void *_pir1)
{
    IR1_INST *pir1 = (IR1_INST *)_pir1;

    IR1_OPCODE id = ir1_opcode(pir1);

    if (option_trace_ir1_type) {
        if (!sys_trace_ir1_type[id]) {
            sys_trace_ir1_type[id] = 1;
            ir1_dump(pir1);
        }
    }

    if (option_trace_ir1_id) {
        if (id == option_trace_ir1_id) {
            ir1_dump(pir1);
        }
    }
}

void dump_xtm_options(void)
{
    /* opt = 0 : not enabled
     * opt = 1 : enabled
     * opt = X : not supported */
    printf("[XTM-OPT] by_hand = %d\n", option_by_hand);
    printf("[XTM-OPT] flag_pattern = %d\n", option_flag_pattern);
    printf("[XTM-OPT] flag_reduction = %d\n", option_flag_reduction);
    printf("[XTM-OPT] dump = %d\n", option_dump);
    printf("[XTM-OPT] dump_host = %d\n", option_dump_host);
    printf("[XTM-OPT] dump_ir1 = %d\n", option_dump_ir1);
    printf("[XTM-OPT] dump_ir2 = %d\n", option_dump_ir2);
    printf("[XTM-OPT] trace_simple = %d\n", option_trace_simple);
    printf("[XTM-OPT] trace_tb = %d\n", option_trace_tb);
    printf("[XTM-OPT] trace_tb_ir1 = %d\n", option_trace_tb_ir1);
    printf("[XTM-OPT] trace_ir1_type = %d\n", option_trace_ir1_type);
    printf("[XTM-OPT] trace_ir1_id = %d\n", option_trace_ir1_id);
    printf("[XTM-OPT] trace_start_nr = %lld\n", option_trace_start_nr);
    printf("[XTM-OPT] trace_start_tb = %#llx\n", option_trace_start_tb);
    printf("[XTM-OPT] check = %d\n", option_check);
    printf("[XTM-OPT] tb_link = %d\n", option_tb_link);
    printf("[XTM-OPT] tb_lookup = %d\n", option_tb_lookup);
    printf("[XTM-OPT] lbt = %d\n", option_lbt);
    printf("[XTM-OPT] shadow_stack = %d\n", option_shadow_stack);
    printf("[XTM-OPT] lsfpu = %d\n", option_lsfpu);
    printf("[XTM-OPT] staticcs = %d\n", option_staticcs);
    printf("[XTM-OPT] risk opt = %d\n", option_risk);
    printf("[XTM-OPT] branch opt = %d\n", option_branch_opt);
    printf("[XTM-OPT] signal interrupt = %d\n", option_sigint);
#ifndef CONFIG_SOFTMMU
    printf("[XTM-OPT] profile = %d\n", option_profile);
#else
    printf("[XTM-OPT] profile = X\n");
    printf("[XTM-OPT] monitor 1 simple    counter = %d\n", option_monitor_sc);
    printf("[XTM-OPT] monitor 2 timer     counter = %d\n", option_monitor_tc);
    printf("[XTM-OPT] monitor 3 register    stat  = %d\n", option_monitor_rs);
    printf("[XTM-OPT] monitor 4 translation stat  = %d\n", option_monitor_ts);
    printf("[XTM-OPT] monitor 5 complex     stat  = %d\n", option_monitor_cs);
    printf("[XTM-OPT] monitor 6 jmp cache profile = %d\n", option_monitor_jc);
    printf("[XTM-OPT] monitor 7 TB hash table     = %d\n", option_monitor_tbht);
    printf("[XTM-OPT] monitor 8 prilivage switch  = %d\n", option_monitor_ps);
    printf("[XTM-OPT] monitor 9 TB flags change   = %d\n", option_monitor_tbf);
#endif
#ifdef CONFIG_DEBUG_CHECK
    printf("[XTM-OPT] debugcall = %d\n", option_debugcall);
#endif
    printf("[XTM-OPT] break point = %d\n", option_break_point);
    printf("[XTM-OPT] break point TB = %#llx\n", option_break_point_addrx);
    printf("[XTM-OPT] break point nr = %lld\n", option_break_point_count);
    printf("[XTM-OPT] test type = %s\n", xtm_test_type_str[option_test_type]);
    printf("[XTM-OPT] cross page check = %d\n", option_cross_page_check);
}

#if defined(CONFIG_X86toMIPS) && defined(CONFIG_SOFTMMU)

#include "include/qemu-def.h"

/* these functions are used only in this file */
void x86_to_mips_parse_options(QemuOpts *opts);
void parse_options_bool(int index, bool set);
void set_options(int index);

QemuOptsList qemu_xtm_opts = {
    .name = "xtm",
    .implied_opt_name="optm",
    .head = QTAILQ_HEAD_INITIALIZER(qemu_xtm_opts.head),
    .desc = {
        {
            .name = "optm",
            .type = QEMU_OPT_STRING,
            .help = "none/select/all optimizations to enable",
        },{
            .name = "trbh",
            .type = QEMU_OPT_BOOL,
            .help = "enable/disable translate by hand",
        },{
            .name = "flagptn",
            .type = QEMU_OPT_BOOL,
            .help = "enable/disable flag pattern",
        },{
            .name = "flagrdtn",
            .type = QEMU_OPT_BOOL,
            .help = "enable/disable flag reduction",
        }, {
            .name = "tblink",
            .type = QEMU_OPT_BOOL,
            .help = "enable/disable TB-link(also named TB-chain)",
        }, {
            .name = "ss",
            .type = QEMU_OPT_BOOL,
            .help = "enable/disable shadow stack",
        }, {
            .name = "lbt",
            .type = QEMU_OPT_BOOL,
            .help = "enable/disable loongson binary translation support",
        }, {
            .name = "lsfpu",
            .type = QEMU_OPT_BOOL,
            .help = "enable/disable using loongson FPU",
        }, {
            .name = "staticcs",
            .type = QEMU_OPT_BOOL,
            .help = "use static generated codes for context switch",
        }, {
            .name = "risk",
            .type = QEMU_OPT_BOOL,
            .help = "apply optimizations that might get wrong in some rare situation",
        }, {
            .name = "tblookup",
            .type = QEMU_OPT_BOOL,
            .help = "optimization for indirect jmp tb lookup",
        }, {
            .name = "sigint",
            .type = QEMU_OPT_BOOL,
            .help = "use signal to info interrupt to vCPU",
        }, {
            .name = "njc",
            .type = QEMU_OPT_BOOL,
            .help = "use Native Jmp Cache lookup",
        }, {
            .name = "optbranch",
            .type = QEMU_OPT_BOOL,
            .help = "branch target directly, if possible",
        }, {
            .name = "dump",
            .type = QEMU_OPT_STRING,
            .help = "dump flags for func,ir1,ir2,host",
        }, {
            .name = "trace",
            .type = QEMU_OPT_STRING,
            .help = "trace flags for TB,ir1",
        }, {
            .name = "traceir1id",
            .type = QEMU_OPT_NUMBER,
            .help = "instruction ID to trace IR1",
        }, {
            .name = "cpusinfo",
            .type = QEMU_OPT_STRING,
            .help = "select output which cpus' trace/dump info",
        }, {
            .name = "tracestartnr",
            .type = QEMU_OPT_NUMBER,
            .help = "start to print trace after @number of TB's execution",
        }, {
            .name = "tracestarttb",
            .type = QEMU_OPT_NUMBER,
            .help = "start to print trace after TB.PC",
        }, {
            .name = "check",
            .type = QEMU_OPT_BOOL,
            .help = "check with original qemu",
        }, {
            .name = "breakpoint",
            .type = QEMU_OPT_NUMBER,
            .help = "build break point code to use",
        }, {
            .name = "bpnum",
            .type = QEMU_OPT_NUMBER,
            .help = "break point TB exec number",
        }, {
            .name = "profile",
            .type = QEMU_OPT_BOOL,
            .help = "enable/disable profiling translation",
        }, {
            .name = "cpc",
            .type = QEMU_OPT_BOOL,
            .help = "enable/disable cross page check in system",
        }, {
            .name = "monitor",
            .type = QEMU_OPT_STRING,
            .help = "enable/disable XQM's monitor information",
        }, {
            .name = "debug",
            .type = QEMU_OPT_STRING,
            .help = "debug translator by TB or PC",
        }, {
            .name = "debugshow",
            .type = QEMU_OPT_NUMBER,
            .help = "debug info for the given number TB",
        }, {
            .name = "debugcall",
            .type = QEMU_OPT_BOOL,
            .help = "print all x86 call instructions",
        }, {
            .name = "verbose",
            .type = QEMU_OPT_BOOL,
            .help = "pint the options parsing result",
        },
        { /* end of list */ }
    },
};

void set_options(int index)
{
    switch(index) {
    case XTM_OPT_by_hand:        option_by_hand        = 1;  break;
    case XTM_OPT_flag_pattern:   option_flag_pattern   = 1;  break;
    case XTM_OPT_flag_reduction: option_flag_reduction = 1;  break;
    case XTM_OPT_dump:           option_dump           = 1;  break;
    case XTM_OPT_dump_host:      option_dump_host      = 1;  break;
    case XTM_OPT_dump_ir1:       option_dump_ir1       = 1;  break;
    case XTM_OPT_dump_ir2:       option_dump_ir2       = 1;  break;
    case XTM_OPT_trace_tb:       option_trace_tb       = 1;  break;
    case XTM_OPT_trace_tb_ir1:   option_trace_tb_ir1   = 1;  break;
    case XTM_OPT_trace_ir1_type: option_trace_ir1_type = 1;  break;
    case XTM_OPT_trace_simple:   option_trace_simple   = 1;  break;
    case XTM_OPT_check:          option_check          = 1;  break;
    case XTM_OPT_tb_link:        option_tb_link        = 1;  break;
    case XTM_OPT_tb_lookup:      option_tb_lookup      = 1;  break;
    case XTM_OPT_lbt:            option_lbt            = 1;  break;
    case XTM_OPT_shadow_stack:   option_shadow_stack   = 1;  break;
    case XTM_OPT_lsfpu:          option_lsfpu          = 1;  break;
    case XTM_OPT_staticcs:       option_staticcs       = 1;  break;
    case XTM_OPT_risk:           option_risk           = 1;  break;
    case XTM_OPT_branch_opt:     option_branch_opt     = 1;  break;
    case XTM_OPT_sigint:         option_sigint         = 1;  break;
    case XTM_OPT_njc:            option_njc            = 1;  break;
    case XTM_OPT_profile:        option_profile        = 1;  break;
#ifdef CONFIG_DEBUG_CHECK
    case XTM_OPT_debugcall:      option_debugcall      = 1;  break;
#endif
    case XTM_OPT_break_point:    option_break_point    = 1;  break;
    case XTM_OPT_cross_page_check:
          option_cross_page_check = 1;  break;
    default: break;
    }
}

void parse_options_bool(int index, bool set)
{
    if (set) {
        set_options(index);
    }
}

static
void x86_to_mips_check_options(void)
{
    lsassertm(!option_by_hand || option_lbt,
            "[OPTION] trbh must with lbt.\n");
}

void x86_to_mips_parse_options(QemuOpts *opts)
{
    QemuOpt     *opt;
    const char  *str;
    int         need_parse_optimizations = 1;
    int         verbose = 0;

    opt = qemu_opt_find(opts, "verbose");
    if (opt && opt->value.boolean) {
        verbose = 1;
    }

    if (verbose) {
        qemu_opts_print(opts,"\n");
        printf("\n");
    }

    /* optm=none|select|all */
    opt = qemu_opt_find(opts, "optm");
    str = opt->str;
    if (strncmp(str, "all", 3) == 0) {
        if (verbose) {
            printf("[optimization] enable all optimizations.\n");
        }
        set_options(XTM_OPT_by_hand);
        set_options(XTM_OPT_flag_pattern);
        set_options(XTM_OPT_flag_reduction);
        set_options(XTM_OPT_tb_link);
        set_options(XTM_OPT_lbt);
        set_options(XTM_OPT_shadow_stack);
        set_options(XTM_OPT_lsfpu);
        need_parse_optimizations = 0;
    }
    else if (strncmp(str, "none", 4) == 0) {
        if (verbose) {
            printf("[optimization] disable all optimizations.\n");
        }
        need_parse_optimizations = 0;
    }
    else if (strncmp(str, "select", 6) == 0) {
        if (verbose) {
            printf("[optimization] to select optimizations.\n");
        }
    }
    else{
        lsassertm(0, "optm must be one of {none, select, all}\n");
    }

    opt = qemu_opt_find(opts, "dump");
    if (opt)
        options_parse_dump(opt->str);

    opt = qemu_opt_find(opts, "trace");
    if (opt)
        options_parse_trace(opt->str);

    opt = qemu_opt_find(opts, "cpusinfo");
    if (opt)
        options_parse_cpusinfo(opt->str);

    opt = qemu_opt_find(opts, "monitor");
    if (opt)
        options_parse_monitor(opt->str);

    opt = qemu_opt_find(opts, "tracestartnr");
    if (opt)
        options_parse_tracestart_nr(opt->value.uint);

    opt = qemu_opt_find(opts, "tracestarttb");
    if (opt)
        options_parse_tracestart_tb(opt->value.uint);

    opt = qemu_opt_find(opts, "traceir1id");
    if (opt)
        options_parse_trace_ir1_id(opt->value.uint);

    opt = qemu_opt_find(opts, "cpc");
    if (opt)
        parse_options_bool(XTM_OPT_cross_page_check,
                opt->value.boolean);

#ifndef CONFIG_SOFTMMU
    opt = qemu_opt_find(opts, "profile");
    if (opt)
        parse_options_bool(XTM_OPT_profile, opt->value.boolean);
#endif

#ifdef CONFIG_DEBUG_CHECK
    opt = qemu_opt_find(opts, "debugcall");
    if (opt)
        parse_options_bool(XTM_OPT_debugcall, opt->value.boolean);
#endif

    opt = qemu_opt_find(opts, "check");
    if (opt)
        parse_options_bool(XTM_OPT_check, opt->value.boolean);

    opt = qemu_opt_find(opts, "breakpoint");
    if (opt)
        options_parse_bpc(opt->value.uint);

    opt = qemu_opt_find(opts, "bpnum");
    if (opt)
        options_parse_bpn(opt->value.uint);

    /* parse all optimizations as last! */
    if (!need_parse_optimizations) {
        goto _OUT;
    }

    opt = qemu_opt_find(opts, "trbh");
    if (opt)
        parse_options_bool(XTM_OPT_by_hand, opt->value.boolean);

    opt = qemu_opt_find(opts, "flagptn");
    if (opt)
        parse_options_bool(XTM_OPT_flag_pattern, opt->value.boolean);

    opt = qemu_opt_find(opts, "flagrdtn");
    if (opt)
        parse_options_bool(XTM_OPT_flag_reduction, opt->value.boolean);

    opt = qemu_opt_find(opts, "tblink");
    if (opt)
        parse_options_bool(XTM_OPT_tb_link, opt->value.boolean);

    opt = qemu_opt_find(opts, "ss");
    if (opt)
        parse_options_bool(XTM_OPT_shadow_stack, opt->value.boolean);

    opt = qemu_opt_find(opts, "lbt");
    if (opt)
        parse_options_bool(XTM_OPT_lbt, opt->value.boolean);

    opt = qemu_opt_find(opts, "lsfpu");
    if (opt)
        parse_options_bool(XTM_OPT_lsfpu, opt->value.boolean);

    opt = qemu_opt_find(opts, "staticcs");
    if (opt)
        parse_options_bool(XTM_OPT_staticcs, opt->value.boolean);

    opt = qemu_opt_find(opts, "risk");
    if (opt)
        parse_options_bool(XTM_OPT_risk, opt->value.boolean);
    
    opt = qemu_opt_find(opts, "optbranch");
    if (opt)
        parse_options_bool(XTM_OPT_branch_opt, opt->value.boolean);

    opt = qemu_opt_find(opts, "tblookup");
    if (opt)
        parse_options_bool(XTM_OPT_tb_lookup, opt->value.boolean);

    opt = qemu_opt_find(opts, "sigint");
    if (opt)
        parse_options_bool(XTM_OPT_sigint, opt->value.boolean);

    opt = qemu_opt_find(opts, "njc");
    if (opt)
        parse_options_bool(XTM_OPT_njc, opt->value.boolean);

_OUT:
    if (verbose) {
        dump_xtm_options();
    }

    x86_to_mips_check_options();

    return;
}
#endif

#endif
