#include "include/common.h"
#include "include/env.h"
#include "include/etb.h"
#include "include/flag_lbt.h"
#include "include/flag_pattern.h"
#include "include/reg_alloc.h"
#include "ir1/ir1.h"
#include "ir2/ir2.h"
#include "x86tomips-config.h"
#include "x86tomips-options.h"
#include <string.h>

#include "../../../include/qemu/option.h"
#include "include/test-def.h"

TESTCFG tt_cfg;
TESTENV tt_env_reset;
char   *tt_inst_str;

void x86_to_mips_parse_tests(QemuOpts *opts);

#define TT_RESET_REG(name) do { \
    env->regs[ name ## _index ] = \
        tt_env_reset.regs[ name ## _index ] ;\
} while(0) \

#define TT_RESET_SEG(name) do { \
    env->segs[ name ## _index].base = \
        tt_env_reset.segs[ name ## _index ].base ;\
} while(0) \

void tt_reset_cpu(CPUState *cpu)
{
    CPUX86State *env = cpu->env_ptr;

    TT_RESET_REG(eax);
    TT_RESET_REG(ecx);
    TT_RESET_REG(edx);
    TT_RESET_REG(ebx);
    TT_RESET_REG(esp);
    TT_RESET_REG(ebp);
    TT_RESET_REG(esi);
    TT_RESET_REG(edi);

    TT_RESET_SEG(es);
    TT_RESET_SEG(cs);
    TT_RESET_SEG(ss);
    TT_RESET_SEG(ds);
    TT_RESET_SEG(fs);
    TT_RESET_SEG(gs);

    uint32_t eflags = tt_env_reset.eflags;
    env->eflags = eflags & ~0x8d5;
    env->cc_src = env->eflags & 0x8d5;
    env->cc_op  = CC_OP_EFLAGS;

    /* PE */
    if (tt_env_reset.pe) {
        env->cr[0]  |= CR0_PE_MASK;
    }
    else {
        env->cr[0]  &= ~CR0_PE_MASK;
    }

    /* VM86 */
    if (tt_env_reset.vm86) {
        env->eflags |= HF_VM_MASK;
    }
    else {
        env->eflags &= ~HF_VM_MASK;
    }

    /* CS32 and SS32 */
    if (tt_env_reset.pe && !tt_env_reset.vm86) {
        env->segs[R_CS].flags |= DESC_B_MASK;
        env->segs[R_SS].flags |= DESC_B_MASK;
    }
    else {
        env->segs[R_CS].flags &= ~DESC_B_MASK;
        env->segs[R_SS].flags &= ~DESC_B_MASK;
    }

    /* Update hflags */
    x86_update_hflags(env);

    env->eip = 0;
}

void *tt_test(CPUState *cpu)
{
    switch (option_test_type) {
    case XTM_TEST_TYPE_TRANSLATION:
        return tt_test_dump_translate_inst(cpu);
        break;
    case XTM_TEST_TYPE_RUN_TB:
        return tt_test_run_on_tb(cpu);
        break;
    default:
        fprintf(stderr, "[XTM_TEST] unknown test type: %d.\n", option_test_type);
        exit(-1);
        break;
    }

    return NULL;
}

void tt_exit(CPUState *cpu)
{
    CPUX86State *env = cpu->env_ptr;

    if (env->exit_test) {
        exit(-1);
    }
}

#define TEST_DEF_REG(regname) \
        { \
            .name = regname, \
            .type = QEMU_OPT_NUMBER, \
            .help = "set the " regname " register", \
        },

#define TEST_DEF_SEG(segname) \
        { \
            .name = segname , \
            .type = QEMU_OPT_NUMBER, \
            .help = "set the " segname " base", \
        },

#define TEST_DEF_BOOL(bname) \
        { \
            .name = bname , \
            .type = QEMU_OPT_BOOL, \
            .help = "set the " bname " of env", \
        },

#define TEST_DEF_INT(iname) \
        { \
            .name = iname , \
            .type = QEMU_OPT_NUMBER, \
            .help = "set the " iname " of env", \
        },

#define TEST_DEF_CFG_NUM(iname) \
        { \
            .name = iname , \
            .type = QEMU_OPT_NUMBER, \
            .help = "set the " iname " for testing", \
        },

QemuOptsList qemu_xtm_tests = {
    .name = "xtestm",
    .implied_opt_name="type",
    .head = QTAILQ_HEAD_INITIALIZER(qemu_xtm_tests.head),
    .desc = {
        {
            .name = "type",
            .type = QEMU_OPT_STRING,
            .help = "choose test type: dump, runtb",
        },
        TEST_DEF_REG("eax")
        TEST_DEF_REG("ecx")
        TEST_DEF_REG("edx")
        TEST_DEF_REG("ebx")
        TEST_DEF_REG("esp")
        TEST_DEF_REG("ebp")
        TEST_DEF_REG("esi")
        TEST_DEF_REG("edi")
        TEST_DEF_SEG("es")
        TEST_DEF_SEG("cs")
        TEST_DEF_SEG("ss")
        TEST_DEF_SEG("ds")
        TEST_DEF_SEG("fs")
        TEST_DEF_SEG("gs")
        TEST_DEF_BOOL("pe")
        TEST_DEF_BOOL("vm86")
        TEST_DEF_INT("eflags")
        TEST_DEF_CFG_NUM("instmax")
        {
            .name = "insts",
            .type = QEMU_OPT_STRING,
            .help = "set the insts' bytes with hex format",
        }
    }
};

#define TESTENV_PARSE_REG(regname, regstr) do { \
    opt = qemu_opt_find(opts, regstr ); \
    if (opt) { \
        tt_env_reset.regs[ regname ## _index ] = opt->value.uint; \
        printf("[ENV] reset " regstr " as %#lx\n", opt->value.uint); \
    } \
} while(0)

#define TESTENV_PARSE_SEG(segname, segstr) do { \
    opt = qemu_opt_find(opts, segstr ); \
    if (opt) { \
        tt_env_reset.segs[ segname ## _index ].base = opt->value.uint; \
        printf("[ENV] reset " segstr " base as %#lx\n", opt->value.uint); \
    } \
} while(0)

#define TESTENV_PARSE_BOOL(bname, bstr) do { \
    opt = qemu_opt_find(opts, bstr); \
    if (opt) { \
        tt_env_reset.bname = opt->value.boolean; \
        printf("[ENV] reset " bstr " as %#x\n", opt->value.boolean); \
    } \
} while(0)

#define TESTENV_PARSE_INT(iname, istr) do { \
    opt = qemu_opt_find(opts, istr); \
    if (opt) { \
        tt_env_reset.iname = opt->value.uint; \
        printf("[ENV] reset " istr " as %#lx\n", opt->value.uint); \
    } \
} while(0)

#define TESTENV_PARSE_CFG(iname, istr) do { \
    opt = qemu_opt_find(opts, istr); \
    if (opt) { \
        tt_cfg.iname = opt->value.uint; \
        printf("[test-config] set " istr " as %#lx\n", opt->value.uint); \
    } \
} while(0)


void x86_to_mips_parse_tests(QemuOpts *opts)
{
    QemuOpt *opt;
    const char  *str;

    memset(&tt_env_reset, 0, sizeof(TESTENV));

    opt = qemu_opt_find(opts, "type");
    str = opt->str;
    if (strncmp(str, "dump", 4) == 0) {
        option_test = 1;
        option_test_type = XTM_TEST_TYPE_TRANSLATION;
    }
    else if (strncmp(str, "runtb", 5) == 0) {
        option_test = 1;
        option_test_type = XTM_TEST_TYPE_RUN_TB;
    }
    else {
        lsassertm(0, "type must be one of {dump, runtb}\n");
    }

    TESTENV_PARSE_REG(eax, "eax");
    TESTENV_PARSE_REG(ecx, "ecx");
    TESTENV_PARSE_REG(edx, "edx");
    TESTENV_PARSE_REG(ebx, "ebx");
    TESTENV_PARSE_REG(esp, "esp");
    TESTENV_PARSE_REG(ebp, "ebp");
    TESTENV_PARSE_REG(esi, "esi");
    TESTENV_PARSE_REG(edi, "edi");

    TESTENV_PARSE_SEG(es, "es");
    TESTENV_PARSE_SEG(cs, "cs");
    TESTENV_PARSE_SEG(ss, "ss");
    TESTENV_PARSE_SEG(ds, "ds");
    TESTENV_PARSE_SEG(fs, "fs");
    TESTENV_PARSE_SEG(gs, "gs");

    TESTENV_PARSE_BOOL(pe, "pe");
    TESTENV_PARSE_BOOL(vm86, "vm86");

    TESTENV_PARSE_INT(eflags, "eflags");

    tt_cfg.inst_max_num = TT_INST_MAX_DEFAULT;
    TESTENV_PARSE_CFG(inst_max_num, "instmax");

    opt = qemu_opt_find(opts, "insts");
    if (opt) {
        tt_inst_str = mm_malloc(strlen(opt->str) + 2);
        sprintf(tt_inst_str, "%s", opt->str);
    }
}

uint8_t tt_convert_2char_to_1byte(char *c, int i)
{
    uint8_t res = 0;

    if ('0' <= c[i] && c[i] <= '9') {
        res += 0x10 * (c[i] - '0');
    }
    else if ('a' <= c[i] && c[i] <= 'f') {
        res += 0x10 * (c[i] - 'a') + 0xa0;
    }
    else {
        fprintf(stderr, "[test] bad inst char %c\n", c[i]);
        exit(-1);
    }

    if ('0' <= c[i+1] && c[i+1] <= '9') {
        res += 0x1 * (c[i+1] - '0');
    }
    else if ('a' <= c[i+1] && c[i+1] <= 'f') {
        res += 0x1 * (c[i+1] - 'a') + 0xa;
    }
    else {
        fprintf(stderr, "[test] bad inst char %c\n", c[i+1]);
        exit(-1);
    }

    return res;
}

int tt_get_ir1_array(IR1_INST *ir1_array, int ir1_max)
{
    /* Example:
     *
     * str   = "984a6b\0"
     * len   = 6
     * bytes = 3
     * inst  = {0x98, 0x4a, 0x6b}
     */

    lsassertm(tt_inst_str,
            "No inst specificed. please input with -xtestm [type],insts=value\n");

    int len = strlen(tt_inst_str);
    int bytes = len >> 1;
    uint8_t *inst = mm_malloc(bytes);

    int i = 0;
    for (i = 0; i < len >> 1; ++i) {
        inst[i] = tt_convert_2char_to_1byte(tt_inst_str, i << 1);
    }

    IR1_INST *ir1 = ir1_array;
    ADDRX pc = 0;
    int ir1_nr = 0;
    int error = 0;

    do {
        pc = ir1_disasm(&ir1[ir1_nr], inst + pc, pc, &error);
        if (error) {
            fprintf(stderr, "Cannot disasm at %d.\n", pc);
            exit(-1);
        }
        ir1_dump(&ir1[ir1_nr]);
        ir1_nr += 1;
    } while(pc < bytes && ir1_nr < ir1_max);

    ir1_make_ins_JMP(&ir1[ir1_nr], pc + 5, 0);
    ir1_nr += 1;

    return ir1_nr;
}
