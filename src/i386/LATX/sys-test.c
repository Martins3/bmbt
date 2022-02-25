#include "common.h"
#include "lsenv.h"
#include "translate.h"
#include "ir1.h"
#include "ir2.h"
#include "flag-reduction.h"
#include "latx-options.h"
#include "qemu/osdep.h"

#include "latx-test-sys.h"
#include "qemu/option.h"
#include "qemu/option_int.h"

int option_test_sys;
int option_test_type;

int latx_test_sys_enabled(void)
{
    return option_test_sys;
}

#define LATX_TEST_TRANSLATION   0
#define LATX_TEST_RUNTB         1

typedef struct latxs_test_env {
    target_ulong regs[CPU_NB_REGS];
    target_ulong eflags;
    uint8_t pe;
    uint8_t vm86;
} latxs_test_env_t;

#define LATXS_TESTENV_PARSE_REG(regname, regstr) do {                       \
    opt = qemu_opt_find(opts, regstr);                                      \
    if (opt) {                                                              \
        tt_env_reset.regs[regname ## _index] = opt->value.uint;           \
        printf("[TEST] reset CPU " regstr " as 0x%lx\n", opt->value.uint);  \
    }                                                                       \
} while (0)

#define LATXS_TESTENV_PARSE_BOOL(bname, bstr) do {                          \
    opt = qemu_opt_find(opts, bstr);                                        \
    if (opt) {                                                              \
        tt_env_reset.bname = opt->value.boolean;                            \
        printf("[TEST] reset CPU " bstr " as %d\n", opt->value.boolean);   \
    }                                                                       \
} while (0)

#define LATXS_TESTENV_PARSE_NUM(regname, regstr) do {                       \
    opt = qemu_opt_find(opts, regstr);                                      \
    if (opt) {                                                              \
        tt_env_reset.regname = opt->value.uint;                             \
        printf("[TEST] reset CPU " regstr " as 0x%lx\n", opt->value.uint);  \
    }                                                                       \
} while (0)

latxs_test_env_t tt_env_reset;

char *tt_inst_str;

static uint8_t tt_convert_2char_to_1byte(char *c, int i)
{
    uint8_t res = 0;

    if ('0' <= c[i] && c[i] <= '9') {
        res += 0x10 * (c[i] - '0');
    } else if ('a' <= c[i] && c[i] <= 'f') {
        res += 0x10 * (c[i] - 'a') + 0xa0;
    } else {
        fprintf(stderr, "[test] bad inst char %c\n", c[i]);
        exit(-1);
    }

    if ('0' <= c[i + 1] && c[i + 1] <= '9') {
        res += 0x1 * (c[i + 1] - '0');
    } else if ('a' <= c[i + 1] && c[i + 1] <= 'f') {
        res += 0x1 * (c[i + 1] - 'a') + 0xa;
    } else {
        fprintf(stderr, "[test] bad inst char %c\n", c[i + 1]);
        exit(-1);
    }

    return res;
}

static int tt_get_ir1_array(IR1_INST *ir1_array, int ir1_max)
{
    /*
     * Example: str = "984a6b\0"
     * len   = 6
     * bytes = 3
     * inst  = {0x98, 0x4a, 0x6b}
     */

    lsassertm(tt_inst_str, "No inst specificed.\n");

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
        pc = latxs_ir1_disasm(&ir1[ir1_nr], inst + pc, pc, &error, ir1_nr);
        if (error) {
            fprintf(stderr, "Cannot disasm at %"PRIADDRX".\n", pc);
            exit(-1);
        }
        ir1_dump(&ir1[ir1_nr]);
        ir1_nr += 1;
    } while (pc < bytes && ir1_nr < ir1_max);

    ir1_make_ins_JMP(&ir1[ir1_nr], pc + 5, 0);
    ir1_nr += 1;

    return ir1_nr;
}

static void *latx_test_sys_dump(CPUState *cpu)
{
    CPUX86State *env = cpu->env_ptr;

    void *highwater = NULL;
    TranslationBlock *tb = latx_test_sys_alloc_tb(cpu, &highwater);

    option_dump       = 1;
    option_dump_ir1   = 1;
    option_dump_ir2   = 1;
    option_dump_host  = 1;

    latxs_tr_sys_init(tb, 64, highwater);

    IR1_INST *ir1 = mm_calloc(256, sizeof(IR1_INST));
    int ir1_nr = tt_get_ir1_array(ir1, 256);

    tb->_ir1_instructions = ir1;
    tb->icount = ir1_nr;

    lsenv->tr_data->ir1_inst_array = ir1;
    lsenv->tr_data->ir1_nr = ir1_nr;
    lsenv->tr_data->curr_ir1_inst = NULL;

    tb_flag(tb);

    tb->_top_in = env->fpstt;

    int search_size;
    latxs_tr_translate_tb(tb, &search_size);

    exit(-1);

    return tb;
}

void latx_test_sys_reset_cpu(void *_cpu)
{
    CPUState *cpu = _cpu;
    CPUX86State *env = cpu->env_ptr;

    env->regs[eax_index] = tt_env_reset.regs[eax_index];
    env->regs[ecx_index] = tt_env_reset.regs[ecx_index];
    env->regs[edx_index] = tt_env_reset.regs[edx_index];
    env->regs[ebx_index] = tt_env_reset.regs[ebx_index];
    env->regs[esp_index] = tt_env_reset.regs[esp_index];
    env->regs[ebp_index] = tt_env_reset.regs[ebp_index];
    env->regs[esi_index] = tt_env_reset.regs[esi_index];
    env->regs[edi_index] = tt_env_reset.regs[edi_index];

    uint32_t eflags = tt_env_reset.eflags;
    env->eflags = eflags & ~0x8d5;
    env->cc_src = env->eflags & 0x8d5;
    env->cc_op  = CC_OP_EFLAGS;

    /* PE */
    if (tt_env_reset.pe) {
        env->cr[0]  |= CR0_PE_MASK;
    } else {
        env->cr[0]  &= ~CR0_PE_MASK;
    }

    /* VM86 */
    if (tt_env_reset.vm86) {
        env->eflags |= HF_VM_MASK;
    } else {
        env->eflags &= ~HF_VM_MASK;
    }

    /* CS32 and SS32 */
    if (tt_env_reset.pe && !tt_env_reset.vm86) {
        env->segs[R_CS].flags |= DESC_B_MASK;
        env->segs[R_SS].flags |= DESC_B_MASK;
    } else {
        env->segs[R_CS].flags &= ~DESC_B_MASK;
        env->segs[R_SS].flags &= ~DESC_B_MASK;
    }

    /* Update hflags */
    x86_update_hflags(env);

    env->eip = 0;
}

void *latx_test_sys_start(void *cpu)
{
    switch (option_test_type) {
    case LATX_TEST_TRANSLATION:
        return latx_test_sys_dump(cpu);
        break;
    /* case LATX_TEST_RUNTB: */
        /* return latx_test_sys_runtb(cpu); */
        /* break; */
    default:
        lsassertm(0, "[TEST] unknown test type: %d.\n", option_test_type);
        exit(-1);
        break;
    }

    return NULL;
}

void latx_test_sys_parse_options(void *_opts)
{
    QemuOpts *opts = _opts;
    QemuOpt *opt = NULL;
    char *str = NULL;

    memset(&tt_env_reset, 0, sizeof(latxs_test_env_t));

    opt = qemu_opt_find(opts, "testype");
    str = opt->str;
    if (strncmp(str, "dump", 4) == 0) {
        option_test_sys = 1;
        option_test_type = LATX_TEST_TRANSLATION;
    } else if (strncmp(str, "runtb", 5) == 0) {
        option_test_sys = 1;
        option_test_type = LATX_TEST_RUNTB;
    } else {
        lsassertm(0, "test type must be one of {dump, runtb}\n");
    }

    LATXS_TESTENV_PARSE_REG(eax, "eax");
    LATXS_TESTENV_PARSE_REG(ecx, "ecx");
    LATXS_TESTENV_PARSE_REG(edx, "edx");
    LATXS_TESTENV_PARSE_REG(ebx, "ebx");
    LATXS_TESTENV_PARSE_REG(esp, "esp");
    LATXS_TESTENV_PARSE_REG(ebp, "ebp");
    LATXS_TESTENV_PARSE_REG(esi, "esi");
    LATXS_TESTENV_PARSE_REG(edi, "edi");

    LATXS_TESTENV_PARSE_NUM(eflags, "eflags");

    LATXS_TESTENV_PARSE_BOOL(pe, "pe");
    LATXS_TESTENV_PARSE_BOOL(vm86, "vm86");

    opt = qemu_opt_find(opts, "insts");
    if (opt) {
        tt_inst_str = mm_malloc(strlen(opt->str) + 2);
        sprintf(tt_inst_str, "%s", opt->str);
    }
}

QemuOptsList qemu_latx_test_opts = {
    .name = "latxtest",
    .implied_opt_name = "testype",
    .head = QTAILQ_HEAD_INITIALIZER(qemu_latx_test_opts.head),
    .desc = {
        {
            .name = "testype",
            .type = QEMU_OPT_STRING,
            .help = "dump/runtb simple LATX test interface",
        },
        {
            .name = "insts",
            .type = QEMU_OPT_STRING,
            .help = "enable/disable translate by hand",
        },
        {
            .name = "eax",
            .type = QEMU_OPT_NUMBER,
            .help = "set the eax register",
        },
        {
            .name = "ecx",
            .type = QEMU_OPT_NUMBER,
            .help = "set the ecx register",
        },
        {
            .name = "edx",
            .type = QEMU_OPT_NUMBER,
            .help = "set the edx register",
        },
        {
            .name = "ebx",
            .type = QEMU_OPT_NUMBER,
            .help = "set the ebx register",
        },
        {
            .name = "esp",
            .type = QEMU_OPT_NUMBER,
            .help = "set the esp register",
        },
        {
            .name = "ebp",
            .type = QEMU_OPT_NUMBER,
            .help = "set the ebp register",
        },
        {
            .name = "esi",
            .type = QEMU_OPT_NUMBER,
            .help = "set the esi register",
        },
        {
            .name = "edi",
            .type = QEMU_OPT_NUMBER,
            .help = "set the edi register",
        },
        {
            .name = "eflags",
            .type = QEMU_OPT_NUMBER,
            .help = "set the eflags",
        },
        {
            .name = "pe",
            .type = QEMU_OPT_BOOL,
            .help = "reset CPU to Protected Mode",
        },
        {
            .name = "vm86",
            .type = QEMU_OPT_BOOL,
            .help = "reset CPU to vm86 Mode",
        },
        { /* end of list */ }
    },
};
