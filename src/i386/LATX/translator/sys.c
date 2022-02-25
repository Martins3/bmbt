#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>

/*
 * Execute after every ir1_disasm to fix something
 *
 * 1> A little bug of capstone
 *    Under the following situction :
 *    - address size is 2
 *    - modrm is used and
 *      - mod is 0
 *      - rm is 6
 *    The disp should be considered as unsigned 16-bit value.
 *
 * 2. Set mem opnd's default segment
 *
 * 3. opnd size for rep string operation
 *
 */
void latxs_fix_up_ir1(IR1_INST *ir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    int i = 0;

    /* 1. A little bug of capstone */
    if (latxs_ir1_addr_size(ir1) == 2) {
        uint8 modrm = ir1->info->detail->x86.modrm;
        uint8 mod = (modrm >> 6) & 0x3;
        uint8 rm  = (modrm >> 0) & 0x7;
        if (mod == 0 && rm == 6) {
            /* disp should be unsigend */
            int64 disp_old = ir1->info->detail->x86.disp;
            int64 disp_new = disp_old & 0xffff;
            ir1->info->detail->x86.disp = disp_new;
            for (i = 0; i < ir1->info->detail->x86.op_count; ++i) {
                IR1_OPND *opnd = ir1_get_opnd(ir1, i);
                if (ir1_opnd_is_mem(opnd)) {
                    opnd->mem.disp = disp_new;
                    goto fix_up_2;
                }
            }
        }
    }

fix_up_2:
    /*
     * 2. set mem opnd's default segment
     * --------------------------------------------
     * seg |    situction       | who deals with it
     * --------------------------------------------
     *  CS | instruction fetch  | QEMU
     * --------------------------------------------
     *  SS | stack push pop     | translate_pop/push
     *     | base = EBP or ESP  | here !!!
     * --------------------------------------------
     *  DS | All data reference | here !!!
     * --------------------------------------------
     *  ES | strint operation's | translate_ins/outs
     *     | destination        | translate_movs
     * --------------------------------------------
     */
    if (td->sys.addseg && !latxs_ir1_is_lea(ir1)) {
        for (i = 0; i < ir1->info->detail->x86.op_count; ++i) {
            IR1_OPND *opnd = ir1_get_opnd(ir1, i);
            if (ir1_opnd_is_mem(opnd) && !ir1_opnd_has_seg(opnd)) {
                int base = ir1_opnd_base_reg_num(opnd);
                if (base == esp_index || base == ebp_index) {
                    opnd->mem.segment = X86_REG_SS;
                } else {
                    opnd->mem.segment = X86_REG_DS;
                }
            }
        }
    }

    /*
     * 3. opnd size for 'rep movs/outs/ins... string operation'
     *
     * When inst is prefixed with '0x66 0xf3', only the '0xf3' (rep)
     * will be disassembled. But if it is prefixed with '0xf3 0x66',
     * the '0x66' (opnd size) will also be disassembled.
     *
     * This bug only exists in 32-bit mode between 16 and 32.
     *
     * -------------------------------------
     *   opc |      default opnd size
     * -------------------------------------
     *  inst | 8-bits |  16-bits | 32-bits
     * -------------------------------------
     *   ins |  0x6c  |   0x6d   |  0x6d
     *  outs |  0x6e  |   0x6f   |  0x6f
     *  movs |  0xa4  |   0xa5   |  0xa5
     *  cmps |  0xa6  |   0xa7   |  0xa7
     *  stos |  0xaa  |   0xab   |  0xab
     *  lods |  0xac  |   0xad   |  0xad
     *  scas |  0xae  |   0xaf   |  0xaf
     * -------------------------------------
     */
    if (latxs_ir1_is_string_op(ir1)) {
        uint8_t *opbytes = latxs_ir1_inst_opbytes(ir1);
        if (td->sys.code32 && opbytes[0] & 1) {
            int data_size  = latxs_ir1_data_size(ir1) >> 3;
            IR1_OPND *opnd  = NULL;
            IR1_OPND *opnd2 = NULL;
            switch (opbytes[0]) {
            case 0x6d: /* ins  */
                opnd = ir1_get_opnd(ir1, 0);
                break;
            case 0x6f: /* outs */
                opnd = ir1_get_opnd(ir1, 1);
                break;
            case 0xa5: /* movs */
            case 0xa7: /* cmps */
            case 0xab: /* stos */
            case 0xad: /* lods */
            case 0xaf: /* scas */
                opnd  = ir1_get_opnd(ir1, 0);
                opnd2 = ir1_get_opnd(ir1, 1);
                break;
            default:
                lsassert(0);
                break;
            }
            if (opnd) {
                opnd->size  = data_size;
            }
            if (opnd2) {
                opnd2->size = data_size;
            }
        }
    }

#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        if (ir1_opcode(ir1) == X86_INS_MOVD) {
            IR1_OPND *opnd0 = ir1_get_opnd(ir1, 0);
            IR1_OPND *opnd1 = ir1_get_opnd(ir1, 1);
            if ((ir1_opnd_is_gpr(opnd0) && ir1_opnd_size(opnd0) == 64) ||
                (ir1_opnd_is_gpr(opnd1) && ir1_opnd_size(opnd1) == 64)) {
                ir1->info->id = X86_INS_MOVQ;
                ir1_dump(ir1);
                return;
            }
        }
    }
#endif

}

int latxs_ir1_data_size(IR1_INST *ir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    int prefix_opsize = latxs_ir1_has_prefix_opsize(ir1);
#ifdef TARGET_X86_64
    if (td->sys.code64) {
        /*
         * In 64-bit mode, the default data size is 32-bit.  Select 64-bit
         *  data with rex_w, and 16-bit data with 0x66; rex_w takes precedence
         * over 0x66 if both are present.
         */
        return (ir1->info->detail->x86.rex & 0x8) ? 64
               : prefix_opsize                    ? 16
                                                  : 32;
    }
#endif
    if (td->sys.code32 ^ prefix_opsize) {
        /*  code32 && !prefix_opsize  */
        /* !code32 &&  prefix_opsize  */
        return 4 << 3;
    } else {
        /*  code32 &&  prefix_opsize  */
        /* !code32 && !prefix_opsize  */
        return 2 << 3;
    }
}

void latxs_ir1_make_ins_ILLEGAL(IR1_INST *ir1,
        ADDRX addr, int size, int flags)
{
    cs_insn *info = NULL;

    TRANSLATION_DATA *td = lsenv->tr_data;
    if (td->sys.code32) {
        info = cs_malloc(handle);
    }
#ifdef TARGET_X86_64
    else if (td->sys.code64) {
        info = cs_malloc(handle64);
    }
#endif
    else {
        info = cs_malloc(handle16);
    }

    info->id = X86_INS_INVALID;
    info->address = addr - td->sys.cs_base;
    info->mnemonic[0] = 'i';
    info->mnemonic[1] = 'l';
    info->mnemonic[2] = 'l';
    info->mnemonic[3] = 'e';
    info->mnemonic[4] = 'g';
    info->mnemonic[5] = 'a';
    info->mnemonic[6] = 'l';
    info->mnemonic[7] = 0;
    info->size = size;
    info->op_str[0] = 0;
    info->detail->x86.addr_size = -1;
    info->detail->x86.op_count = 0;

    ir1->flags = flags;
    ir1->info = info;
}

int latxs_tb_max_insns(void)
{
    if (!option_tb_max_insns) {
        return MAX_IR1_NUM_PER_TB;
    }
    return option_tb_max_insns;
}

void latxs_tr_sys_init(TranslationBlock *tb,
        int max_insns, void *code_highwater)
{
    if (!tb) {
        return;
    }

    CPUX86State *env = lsenv->cpu_state;
    TRANSLATION_DATA *td = lsenv->tr_data;

    lsassert(max_insns >= 1);
    int max_nr = latxs_tb_max_insns();
    td->max_insns = max_insns < max_nr ? max_insns : max_nr;
    td->code_highwater = code_highwater;

    uint32_t flags  = tb->flags;
    uint32_t cflags = tb->cflags;

    td->slow_path_rcd_nr = 0;
    td->in_gen_slow_path = 0;

    td->sys.pe = (flags >> HF_PE_SHIFT) & 1;
    td->sys.code32 = (flags >> HF_CS32_SHIFT) & 1;
    td->sys.ss32 = (flags >> HF_SS32_SHIFT) & 1;
    td->sys.addseg = (flags >> HF_ADDSEG_SHIFT) & 1;
    td->sys.f_st = 0;
    td->sys.vm86 = (flags >> VM_SHIFT) & 1;
    td->sys.cpl = (flags >> HF_CPL_SHIFT) & 3;
    td->sys.iopl = (flags >> IOPL_SHIFT) & 3;
    td->sys.tf = (flags >> TF_SHIFT) & 1;
    td->sys.cs_base = tb->cs_base;
    td->sys.popl_esp_hack = 0;

    td->sys.mem_index = cpu_mmu_index(env, false);

    td->sys.cpuid_features = env->features[FEAT_1_EDX];
    td->sys.cpuid_ext_features = env->features[FEAT_1_ECX];
    td->sys.cpuid_ext2_features = env->features[FEAT_8000_0001_EDX];
    td->sys.cpuid_ext3_features = env->features[FEAT_8000_0001_ECX];
    td->sys.cpuid_7_0_ebx_features = env->features[FEAT_7_0_EBX];
    td->sys.cpuid_xsave_features = env->features[FEAT_XSAVE];
#ifdef TARGET_X86_64
    td->sys.lma = (flags >> HF_LMA_SHIFT) & 1;
    td->sys.code64 = (flags >> HF_CS64_SHIFT) & 1;
#endif

    td->sys.flags = flags;
    td->sys.cflags = cflags;

    td->sys.pc = tb->pc;
    td->sys.bp_hit = 0;

    if (option_dump) {
        fprintf(stderr, "SYS.PE     = %d\n", td->sys.pe);
        fprintf(stderr, "SYS.code32 = %d\n", td->sys.code32);
#ifdef TARGET_X86_64
        fprintf(stderr, "SYS.lma    = %d\n", td->sys.lma);
        fprintf(stderr, "SYS.code64 = %d\n", td->sys.code64);
#endif
        fprintf(stderr, "SYS.ss32   = %d\n", td->sys.ss32);
        fprintf(stderr, "SYS.addseg = %d\n", td->sys.addseg);
        fprintf(stderr, "SYS.vm86   = %d\n", td->sys.vm86);
        fprintf(stderr, "SYS.cpl    = %d\n", td->sys.cpl);
        fprintf(stderr, "SYS.iopl   = %d\n", td->sys.iopl);
        fprintf(stderr, "SYS.tf     = %d\n", td->sys.tf);
        fprintf(stderr, "SYS.CSBase = %p\n", (void *)(ADDR)td->sys.cs_base);
        fprintf(stderr, "SYS.MMUIdx = %d\n", td->sys.mem_index);
    }

    td->need_eob    = 1; /* always generate eob */
    td->inhibit_irq = 0;
    td->recheck_tf  = 0;
    td->ignore_eip_update = 0;
    td->ignore_top_update = 0;

    td->end_with_exception = 0;
    td->dec_icount_inst_id = 0;

    td->need_save_currtb_for_int = sigint_enabled();
}
