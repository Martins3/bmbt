#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>

void latxs_sys_misc_register_ir1(void)
{
    latxs_register_ir1(X86_INS_INVALID);
    latxs_register_ir1(X86_INS_LJMP);
    latxs_register_ir1(X86_INS_CALL);
    latxs_register_ir1(X86_INS_JMP);
    latxs_register_ir1(X86_INS_CLI);
    latxs_register_ir1(X86_INS_STI);
    latxs_register_ir1(X86_INS_LIDT);
    latxs_register_ir1(X86_INS_SIDT);
    latxs_register_ir1(X86_INS_LGDT);
    latxs_register_ir1(X86_INS_SGDT);
    latxs_register_ir1(X86_INS_LLDT);
    latxs_register_ir1(X86_INS_SLDT);
    latxs_register_ir1(X86_INS_LTR);
    latxs_register_ir1(X86_INS_STR);
    latxs_register_ir1(X86_INS_PUSH);
    latxs_register_ir1(X86_INS_POP);
    latxs_register_ir1(X86_INS_RET);
    latxs_register_ir1(X86_INS_CPUID);
    latxs_register_ir1(X86_INS_XCHG);
    latxs_register_ir1(X86_INS_CMPXCHG);
    latxs_register_ir1(X86_INS_CMPXCHG8B);
    latxs_register_ir1(X86_INS_CMPXCHG16B);
    latxs_register_ir1(X86_INS_RSM);
    latxs_register_ir1(X86_INS_INVD);
    latxs_register_ir1(X86_INS_WBINVD);
    latxs_register_ir1(X86_INS_NOP);
    latxs_register_ir1(X86_INS_PAUSE);
    latxs_register_ir1(X86_INS_IRET);
    latxs_register_ir1(X86_INS_IRETD);
    latxs_register_ir1(X86_INS_IRETQ);
    latxs_register_ir1(X86_INS_INT);
    latxs_register_ir1(X86_INS_INT1);
    latxs_register_ir1(X86_INS_INT3);
    latxs_register_ir1(X86_INS_INTO);
    latxs_register_ir1(X86_INS_RETF);
    latxs_register_ir1(X86_INS_RETFQ);
    latxs_register_ir1(X86_INS_CWD);
    latxs_register_ir1(X86_INS_CDQ);
    latxs_register_ir1(X86_INS_CQO);
    latxs_register_ir1(X86_INS_CWDE);
    latxs_register_ir1(X86_INS_CDQE);
    latxs_register_ir1(X86_INS_CBW);
    latxs_register_ir1(X86_INS_LCALL);
    latxs_register_ir1(X86_INS_POPAW);
    latxs_register_ir1(X86_INS_POPAL);
    latxs_register_ir1(X86_INS_PUSHAW);
    latxs_register_ir1(X86_INS_PUSHAL);
    latxs_register_ir1(X86_INS_LDS);
    latxs_register_ir1(X86_INS_LES);
    latxs_register_ir1(X86_INS_LFS);
    latxs_register_ir1(X86_INS_LGS);
    latxs_register_ir1(X86_INS_LSS);
    latxs_register_ir1(X86_INS_ENTER);
    latxs_register_ir1(X86_INS_LEAVE);
    latxs_register_ir1(X86_INS_RDTSC);
    latxs_register_ir1(X86_INS_RDTSCP);
    latxs_register_ir1(X86_INS_RDPMC);
    latxs_register_ir1(X86_INS_HLT);
    latxs_register_ir1(X86_INS_RDMSR);
    latxs_register_ir1(X86_INS_WRMSR);
    latxs_register_ir1(X86_INS_INVLPG);
    latxs_register_ir1(X86_INS_INVLPGA);
    latxs_register_ir1(X86_INS_LFENCE);
    latxs_register_ir1(X86_INS_MFENCE);
    latxs_register_ir1(X86_INS_SFENCE);
    latxs_register_ir1(X86_INS_PREFETCHNTA);
    latxs_register_ir1(X86_INS_SYSENTER);
    latxs_register_ir1(X86_INS_SYSEXIT);
    latxs_register_ir1(X86_INS_SYSCALL);
    latxs_register_ir1(X86_INS_SYSRET);
    latxs_register_ir1(X86_INS_PREFETCHT0);
    latxs_register_ir1(X86_INS_PREFETCHT1);
    latxs_register_ir1(X86_INS_PREFETCHT2);
    latxs_register_ir1(X86_INS_PREFETCHW);
    latxs_register_ir1(X86_INS_PREFETCH);
    latxs_register_ir1(X86_INS_TZCNT);

    latxs_register_ir1(X86_INS_UD2);
    latxs_register_ir1(X86_INS_ENDBR32);
    latxs_register_ir1(X86_INS_ENDBR64);
    latxs_register_ir1(X86_INS_SWAPGS);

    latxs_register_ir1(X86_INS_LMSW);

    latxs_register_ir1(X86_INS_VERR);
    latxs_register_ir1(X86_INS_VERW);
}

int latxs_get_sys_stack_addr_size(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
#ifdef TARGET_X86_64
    /* in 64-bit mode, the size of the stack pointer is always 64 bits. */
    if (td->sys.code64) {
        return 8;
    }
#endif
    if (td->sys.ss32) {
        return 4;
    } else {
        return 2;
    }
}

static void translate_jmp_far_pe_imm(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /*
     * void helper_ljmp_protected(
     *     CPUX86State     *env,
     *     int             new_cs,
     *     target_ulong    new_eip,
     *     target_ulong    next_eip)
     */
    IR2_OPND *env = &latxs_env_ir2_opnd;
    IR2_OPND *arg0 = &latxs_arg0_ir2_opnd;
    IR2_OPND *arg1 = &latxs_arg1_ir2_opnd;
    IR2_OPND *arg2 = &latxs_arg2_ir2_opnd;
    IR2_OPND *arg3 = &latxs_arg3_ir2_opnd;

    int   new_cs   = 0;
    ADDRX new_eip  = 0;
    ADDRX next_eip = ir1_addr_next(pir1);

    /*
     *                  ptr16:16 ptr16:32
     * opnd[0]:selector    16       16
     * opnd[1]:offset      16       32
     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    int opnd1_size  = ir1_opnd_size(opnd1);

    new_cs  = ir1_opnd_simm(opnd0);
    new_eip = ir1_opnd_uimm(opnd1);

    /* 1. save native context */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* 2. parameters */

    /* 2.1 arg0: env */
    latxs_append_ir2_opnd2_(lisa_mov, arg0, env);

    /* 2.2 arg1: new_cs, 16-bits */
    latxs_load_imm32_to_ir2(arg1, new_cs & 0xffff, EXMode_Z);

    /* 2.3 arg2: new_eip, 16/32-bits */
    if (opnd1_size == 16) {
        latxs_load_imm32_to_ir2(arg2, new_eip & 0xffff, EXMode_Z);
    } else { /* opnd1_size == 32 */
        latxs_load_imm32_to_ir2(arg2, new_eip & 0xffffffff, EXMode_Z);
    }

    /* 2.4 arg3: next_eip, 32-bits */
    latxs_load_imm32_to_ir2(arg3, next_eip, EXMode_Z);

    /* 3. call helper */
    latxs_tr_gen_call_to_helper((ADDR)helper_ljmp_protected);

    /* 4. restore native context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* 5. context switch */
    lsenv->tr_data->ignore_eip_update = 1;
    latxs_tr_generate_exit_tb(pir1, 1);
}

static void translate_jmp_far_pe_mem(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /*
     * void helper_ljmp_protected(
     *     CPUX86State     *env,
     *     int             new_cs,
     *     target_ulong    new_eip,
     *     target_ulong    next_eip)
     */

    /*
     *           m16:16     m16:32
     * offset      16         32     MEM(addr)
     * selector    16         16     MEM(addr+2/4)
     */
    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd0, -1);

    /* 1. load offset and selector */
    IR2_OPND offset = latxs_ra_alloc_itemp();
    IR2_OPND selector = latxs_ra_alloc_itemp();

    int opnd_size = ir1_opnd_size(opnd0);

    switch (opnd_size) {
    case 32: /* m16:16 */
        gen_ldst_softmmu_helper(LISA_LD_WU, &offset, &mem_opnd, 0);
        latxs_append_ir2_opnd2i(LISA_SRAI_D, &selector, &offset, 0x10);
        latxs_append_ir2_opnd2_(lisa_mov16z, &offset, &offset);
        break;
    case 48: /* m16:32 */
        /* load 32-bit offset */
        gen_ldst_softmmu_helper(LISA_LD_WU, &offset, &mem_opnd, 1);
        /* memory address += 4 */
        IR2_OPND mem_opnd_4 =
            latxs_convert_mem_ir2_opnd_plus_4(&mem_opnd);
        /* load 16-bit selector */
        gen_ldst_softmmu_helper(LISA_LD_HU, &selector, &mem_opnd_4, 1);
        break;
    default:
        lsassertm_illop(ir1_addr(pir1), 0,
                "unsupported opnd size %d in ljmp mem.\n", opnd_size);
        break;
    }

    latxs_ra_free_temp(&mem_opnd);

    /* 2. save native context here */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* 3. parameters */
    /* 3.1 arg1: new_cs, 16-bits */
    /* 3.2 arg2: new_eip, 16/32-bits */
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &selector);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg2_ir2_opnd, &offset);
    /* 3.4 arg3: next_eip, 32-bits */
    ADDRX next_eip = ir1_addr_next(pir1);
    latxs_load_imm32_to_ir2(&latxs_arg3_ir2_opnd, next_eip, EXMode_Z);
    /* 3.5 arg0: env */
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                      &latxs_env_ir2_opnd);

    /* 4. call helper */
    latxs_tr_gen_call_to_helper((ADDR)helper_ljmp_protected);

    /* 5. restore native context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* 6. context switch */
    lsenv->tr_data->ignore_eip_update = 1;
    latxs_tr_generate_exit_tb(pir1, 1);
}

static void translate_jmp_far_real_imm(IR1_INST *pir1, IR1_OPND *opnd0)
{
    uint32_t selector;
    ADDRX base;

    IR2_OPND tmp0 = latxs_ra_alloc_itemp();
    IR2_OPND tmp1 = latxs_ra_alloc_itemp();

    /*                  ptr16:16 ptr16:32 */
    /* opnd[0]:selector    16       16    */
    /* opnd[1]:offset      16       32    */
    selector = ir1_opnd_uimm(opnd0);
    base     = selector << 0x4;

    latxs_load_imm32_to_ir2(&tmp0, selector, EXMode_Z);
    latxs_load_addrx_to_ir2(&tmp1, base);

    latxs_append_ir2_opnd2i(LISA_ST_W, &tmp0, &latxs_env_ir2_opnd,
            lsenv_offset_of_seg_selector(lsenv, R_CS));
    latxs_append_ir2_opnd2i(LISA_ST_W, &tmp1, &latxs_env_ir2_opnd,
            lsenv_offset_of_seg_base(lsenv, R_CS));

    latxs_tr_generate_exit_tb(pir1, 1);
}

static void translate_jmp_far_real_mem(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /* m16:16/m16:32/m16:64 */
    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd0, -1);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    IR2_OPND next_eip = latxs_ra_alloc_itemp();

    /* m16:16 */
    if (ir1_opnd_size(opnd0) == 32) {
        IR2_OPND tmp0 = latxs_ra_alloc_itemp();

        gen_ldst_softmmu_helper(LISA_LD_W, &dest, &mem_opnd, 1);

        latxs_append_ir2_opnd2_(lisa_mov16z, &next_eip, &dest);

        latxs_append_ir2_opnd2i(LISA_SRLI_W, &tmp0, &dest, 0x10);
        latxs_append_ir2_opnd2_(lisa_mov16z, &tmp0, &tmp0);
        latxs_append_ir2_opnd2i(LISA_ST_W, &tmp0, &latxs_env_ir2_opnd,
                lsenv_offset_of_seg_selector(lsenv, R_CS));

        latxs_append_ir2_opnd2i(LISA_SLLI_W, &tmp0, &tmp0, 0x4);
        latxs_append_ir2_opnd2i(LISA_ST_W, &tmp0, &latxs_env_ir2_opnd,
                lsenv_offset_of_seg_base(lsenv, R_CS));

        latxs_ra_free_temp(&tmp0);
    } else if (ir1_opnd_size(opnd0) == 48) { /* m16:32 */
        /* load 32-bit offset */
        gen_ldst_softmmu_helper(LISA_LD_WU, &dest, &mem_opnd, 1);
        latxs_append_ir2_opnd2_(lisa_mov, &next_eip, &dest);

        /* load 16-bit selector */
        IR2_OPND mem_opnd_adjusted =
            latxs_convert_mem_ir2_opnd_plus_4(&mem_opnd);

        gen_ldst_softmmu_helper(LISA_LD_HU, &dest, &mem_opnd_adjusted, 1);
        latxs_append_ir2_opnd2i(LISA_ST_W, &dest, &latxs_env_ir2_opnd,
                lsenv_offset_of_seg_selector(lsenv, R_CS));

        latxs_append_ir2_opnd2i(LISA_SLLI_D, &dest, &dest, 0x4);
        latxs_append_ir2_opnd2i(LISA_ST_W, &dest, &latxs_env_ir2_opnd,
                lsenv_offset_of_seg_base(lsenv, R_CS));
    } else { /* m16:64 */
        lsassertm_illop(ir1_addr(pir1), 0,
            "unsupported opnd size %d in ljmp mem.\n", ir1_opnd_size(opnd0));
    }

    latxs_ra_free_temp(&dest);

#ifdef TARGET_X86_64
    latxs_append_ir2_opnd2i(LISA_ST_D, &next_eip, &latxs_env_ir2_opnd,
                            lsenv_offset_of_eip(lsenv));
#else
    latxs_append_ir2_opnd2i(LISA_ST_W, &next_eip, &latxs_env_ir2_opnd,
                            lsenv_offset_of_eip(lsenv));
#endif

    latxs_tr_generate_exit_tb(pir1, 1);
}

bool latxs_translate_jmp_far(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    /* protected mode && not vm86 mode */
    if (td->sys.pe && !td->sys.vm86) {
        if (ir1_opnd_is_imm(opnd0)) {
            translate_jmp_far_pe_imm(pir1, opnd0);
        } else if (ir1_opnd_is_mem(opnd0)) {
            translate_jmp_far_pe_mem(pir1, opnd0);
        } else {
            lsassert(0);
        }
        return true;
    }

    /* Real-Address mode || vm86 mode */
    if (ir1_opnd_is_imm(opnd0)) {
        translate_jmp_far_real_imm(pir1, opnd0);
    } else if (ir1_opnd_is_mem(opnd0)) {
        translate_jmp_far_real_mem(pir1, opnd0);
    }

    return true;
}

bool latxs_translate_call(IR1_INST *pir1)
{
    if (latxs_ir1_has_prefix_addrsize(pir1)) {
        ir1_dump(pir1);
    }
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        lsassert(!latxs_ir1_has_prefix_opsize(pir1));
    }
#endif
    if (ir1_is_indirect_call(pir1)) {
        return latxs_translate_callin(pir1);
    } else if (ir1_addr_next(pir1) == ir1_target_addr(pir1)) {
        return latxs_translate_callnext(pir1);
    }

#ifdef TARGET_X86_64
    if (ir1_target_addr(pir1) >> 32 && latxs_ir1_has_prefix_addrsize(pir1)) {
        lsassert(0);
    }
#endif

    bool ss32 = lsenv->tr_data->sys.ss32;
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    int data_size = latxs_ir1_data_size(pir1);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        data_size = (data_size == 16) ? 16 : 64;
    }
#endif
    int opnd_size = ir1_opnd_size(opnd0);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        lsassert(opnd_size == 64);
    } else {
        lsassert(opnd_size == 16 || opnd_size == 32);
    }
#else
    lsassert(opnd_size == 16 || opnd_size == 32);
#endif
    lsassert(data_size == opnd_size);
    (void)data_size; /* to avoid compile warning */

    int ss_addr_size = latxs_get_sys_stack_addr_size();

    /* 1. get return address */
    IR2_OPND return_addr_opnd = latxs_ra_alloc_itemp();
    latxs_load_addrx_to_ir2(&return_addr_opnd, ir1_addr_next(pir1));

    /* 2. save return address at MEM(SS:ESP - 2/4)*/
    IR1_OPND mem_ir1_opnd;
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, opnd_size,
            X86_REG_SS, X86_REG_ESP, 0 - (opnd_size >> 3), 0, 0);
    latxs_store_ir2_to_ir1_mem(&return_addr_opnd,
            &mem_ir1_opnd, ss_addr_size);
    latxs_ra_free_temp(&return_addr_opnd);

    /* 3. update ESP */
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd,
                          0 - (opnd_size >> 3));
        if (option_by_hand) {
            lsassert(0);
        }
    } else
#endif
    if (ss32) {
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &esp_opnd, &esp_opnd,
                          0 - (opnd_size >> 3));
#ifdef TARGET_X86_64
        latxs_append_ir2_opnd2_(lisa_mov32z, &esp_opnd, &esp_opnd);
#else
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
        }
#endif
    } else {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &tmp, &esp_opnd,
                          0 - (opnd_size >> 3));
        latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
        latxs_ra_free_temp(&tmp);
    }

    /* 4. exit TB */
    latxs_tr_generate_exit_tb(pir1, 0);

    return true;
}

bool latxs_translate_callnext(IR1_INST *pir1)
{
    bool ss32 = lsenv->tr_data->sys.ss32;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    int data_size = latxs_ir1_data_size(pir1);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        data_size = (data_size == 16) ? 16 : 64;
    }
#endif
    int opnd_size = ir1_opnd_size(opnd0);
    lsassert(opnd_size == 16 || opnd_size == 32);
    lsassert(data_size == opnd_size);
    (void)data_size; /* to avoid compile warning */

    int ss_addr_size = latxs_get_sys_stack_addr_size();

    /* 1. get return address */
    IR2_OPND next_addr_opnd = latxs_ra_alloc_itemp();
    latxs_load_addrx_to_ir2(&next_addr_opnd, ir1_addr_next(pir1));

    /* 2. save return address at MEM(SS:ESP - 2/4)*/
    IR1_OPND mem_ir1_opnd;
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, opnd_size,
            X86_REG_SS, X86_REG_ESP, 0 - (opnd_size >> 3), 0, 0);
    latxs_store_ir2_to_ir1_mem(&next_addr_opnd,
            &mem_ir1_opnd, ss_addr_size);
    latxs_ra_free_temp(&next_addr_opnd);

    /* 3. update ESP */
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
    if (ss32) {
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &esp_opnd,
                &esp_opnd, 0 - (opnd_size >> 3));
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
        }
    } else {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &tmp,
                &esp_opnd, 0 - (opnd_size >> 3));
        latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
        latxs_ra_free_temp(&tmp);
    }

    return true;
}

bool latxs_translate_callin(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    bool ss32 = td->sys.ss32;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    int data_size = latxs_ir1_data_size(pir1);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        data_size = (data_size == 16) ? 16 : 64;
    }
#endif
    int opnd_size = ir1_opnd_size(opnd0);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        lsassert(opnd_size == 16 || opnd_size == 64);
    } else {
        lsassert(opnd_size == 16 || opnd_size == 32);
    }
#else
    lsassert(opnd_size == 16 || opnd_size == 32);
#endif
    lsassert(data_size == opnd_size);
    (void)data_size; /* to avoid compile warning */

    int ss_addr_size = latxs_get_sys_stack_addr_size();

    /* 1. prepare successor x86 address */
    IR2_OPND next_eip_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&next_eip_opnd, opnd0, EXMode_Z);
    if (opnd_size == 16) {
        latxs_append_ir2_opnd2_(lisa_mov16z, &next_eip_opnd,
                                             &next_eip_opnd);
    }

    /* 2. get return address */
    IR2_OPND return_addr_opnd = latxs_ra_alloc_itemp();
    latxs_load_addrx_to_ir2(&return_addr_opnd, ir1_addr_next(pir1));

    /* 3. save return address at MEM(SS:ESP - 2/4)*/
    IR1_OPND mem_ir1_opnd;
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, opnd_size, /* 16 or 32 */
            X86_REG_SS, X86_REG_ESP, 0 - (opnd_size >> 3), 0, 0);
    latxs_store_ir2_to_ir1_mem(&return_addr_opnd,
            &mem_ir1_opnd, ss_addr_size);
    latxs_ra_free_temp(&return_addr_opnd);

    /* 4. update ESP */
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd,
                                0 - (opnd_size >> 3));
        if (option_by_hand) {
            lsassert(0);
        }
    } else
#endif
    if (ss32) {
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &esp_opnd,
                &esp_opnd, 0 - (opnd_size >> 3));
#ifdef TARGET_X86_64
        latxs_append_ir2_opnd2_(lisa_mov32z, &esp_opnd, &esp_opnd);
#else
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
        }
#endif
    } else {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &tmp, &esp_opnd,
                          0 - (opnd_size >> 3));
        latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
        latxs_ra_free_temp(&tmp);
    }

    /* 5. go to next TB */
#ifdef TARGET_X86_64
    latxs_append_ir2_opnd2i(LISA_ST_D, &next_eip_opnd, &latxs_env_ir2_opnd,
                            lsenv_offset_of_eip(lsenv));
#else
    latxs_append_ir2_opnd2i(LISA_ST_W, &next_eip_opnd, &latxs_env_ir2_opnd,
                            lsenv_offset_of_eip(lsenv));
#endif

    latxs_tr_generate_exit_tb(pir1, 0);

    return true;
}

bool latxs_translate_jmp(IR1_INST *pir1)
{
    if (ir1_is_indirect_jmp(pir1)) {
        return latxs_translate_jmpin(pir1);
    }

    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_jmpin(IR1_INST *pir1)
{
    /* 1. set successor x86 address */
    IR2_OPND next_eip = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&next_eip,
            ir1_get_opnd(pir1, 0), EXMode_Z);

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {
        latxs_append_ir2_opnd2_(lisa_mov16z, &next_eip, &next_eip);
    }
#ifdef TARGET_X86_64
    latxs_append_ir2_opnd2i(LISA_ST_D, &next_eip, &latxs_env_ir2_opnd,
                            lsenv_offset_of_eip(lsenv));
#else
    latxs_append_ir2_opnd2i(LISA_ST_W, &next_eip, &latxs_env_ir2_opnd,
                            lsenv_offset_of_eip(lsenv));
#endif
    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_cli(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_CLI(pir1);

    IR2_OPND *zero = &latxs_zero_ir2_opnd;

    IR2_OPND eflags = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_WU, &eflags, &latxs_env_ir2_opnd,
            lsenv_offset_of_eflags(lsenv));

    IR2_OPND mask = latxs_ra_alloc_itemp();

    /* helper_cli */
    latxs_append_ir2_opnd2i(LISA_ORI, &mask, zero, IF_BIT);
    latxs_append_ir2_opnd3(LISA_NOR, &mask, zero, &mask);
    latxs_append_ir2_opnd3(LISA_AND, &eflags, &eflags, &mask);
    latxs_append_ir2_opnd2i(LISA_ST_W, &eflags, &latxs_env_ir2_opnd,
            lsenv_offset_of_eflags(lsenv));

    latxs_ra_free_temp(&eflags);
    latxs_ra_free_temp(&mask);

    return true;
}

/* End of TB in system-mode */
bool latxs_translate_sti(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_CLI(pir1);

    IR2_OPND eflags = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_WU, &eflags, &latxs_env_ir2_opnd,
            lsenv_offset_of_eflags(lsenv));
    IR2_OPND *zero = &latxs_zero_ir2_opnd;

    IR2_OPND mask = latxs_ra_alloc_itemp();

    /* helper_sti */
    latxs_append_ir2_opnd2i(LISA_ORI, &mask, zero, IF_BIT);
    latxs_append_ir2_opnd3(LISA_OR, &eflags, &eflags, &mask);
    latxs_append_ir2_opnd2i(LISA_ST_W, &eflags, &latxs_env_ir2_opnd,
            lsenv_offset_of_eflags(lsenv));

    latxs_ra_free_temp(&eflags);
    latxs_ra_free_temp(&mask);

    /* sti is EOB in system-mode */
    lsenv->tr_data->inhibit_irq = 1;

    return true;
}

bool latxs_translate_lidt(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_LIDT(pir1);

    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1),
            ir1_opnd_is_mem(opnd),
            "not a valid LIDT insn: pir1 = %p\n", (void *)pir1);

    /*
     * In i386: always load 6 bytes
     *  >  ir1_opnd_size(opnd) == 6, not a regular load operation
     *
     * 2 bytes for limit, and limit is always 16-bits long
     *
     * 4 bytes for base address, and base assress is 24-bits long
     * in real-address mode and vm86 mode, 32-bits long in PE mode.
     */
    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd, -1);

    IR2_OPND limit = latxs_ra_alloc_itemp();
    IR2_OPND base  = latxs_ra_alloc_itemp();

    int save_temp = 1;

    /* 1. load 2 bytes for limit at MEM(addr)*/
    gen_ldst_softmmu_helper(LISA_LD_HU, &limit, &mem_opnd, save_temp);

    /* 2. load 4 bytes for base address at MEM(addr + 2)*/
    IR2_OPND mem = latxs_convert_mem_ir2_opnd_plus_2(&mem_opnd);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        gen_ldst_softmmu_helper(LISA_LD_D, &base, &mem, save_temp);
    } else {
        gen_ldst_softmmu_helper(LISA_LD_WU, &base, &mem, save_temp);
    }
#else
    gen_ldst_softmmu_helper(LISA_LD_WU, &base, &mem, save_temp);
#endif

    /* 3. 24-bits long base address in Real-Address mode and vm86 mode */
    if (latxs_ir1_data_size(pir1) == 16) {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        IR2_OPND tmp1 = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_SRAI_D, &tmp, &base, 0x10);
        latxs_append_ir2_opnd2i(LISA_ANDI, &tmp, &tmp, 0xff);
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &tmp, &tmp, 0x10);
        latxs_append_ir2_opnd2_(lisa_mov16z, &tmp1, &base);
        latxs_append_ir2_opnd3(LISA_OR, &tmp, &tmp, &tmp1);
        base = tmp;
    }

    /* 4. store limit/base into IDTR */
#ifdef TARGET_X86_64
    latxs_append_ir2_opnd2i(LISA_ST_D, &base, &latxs_env_ir2_opnd,
            lsenv_offset_of_idtr_base(lsenv));
#else
    latxs_append_ir2_opnd2i(LISA_ST_W, &base, &latxs_env_ir2_opnd,
            lsenv_offset_of_idtr_base(lsenv));
#endif

    latxs_append_ir2_opnd2i(LISA_ST_W, &limit, &latxs_env_ir2_opnd,
            lsenv_offset_of_idtr_limit(lsenv));

    return true;
}

bool latxs_translate_sidt(IR1_INST *pir1)
{
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1),
            ir1_opnd_is_mem(opnd),
            "not a valid SIDT insn: pir1 = %p\n", (void *)pir1);

    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd, -1);

    /* 1. load limit/base into temp register */
    IR2_OPND base = latxs_ra_alloc_itemp();
    IR2_OPND limit = latxs_ra_alloc_itemp();

#ifdef TARGET_X86_64
    latxs_append_ir2_opnd2i(LISA_LD_D, &base, &latxs_env_ir2_opnd,
            lsenv_offset_of_idtr_base(lsenv));
#else
    latxs_append_ir2_opnd2i(LISA_LD_W, &base, &latxs_env_ir2_opnd,
            lsenv_offset_of_idtr_base(lsenv));
#endif
    latxs_append_ir2_opnd2i(LISA_LD_W, &limit, &latxs_env_ir2_opnd,
            lsenv_offset_of_idtr_limit(lsenv));

    int save_temp = 1;
    /* 2. store 2 bytes limit at MEM(addr) */
    gen_ldst_softmmu_helper(LISA_ST_H, &limit, &mem_opnd, save_temp);

    /* 3. store 4 bytes base at MEM(addr + 2) */
    IR2_OPND mem = latxs_convert_mem_ir2_opnd_plus_2(&mem_opnd);
    if (latxs_ir1_addr_size(pir1) == 2) {
        latxs_append_ir2_opnd2_(lisa_mov24z, &base, &base);
    }
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        gen_ldst_softmmu_helper(LISA_ST_D, &base, &mem, save_temp);
    } else {
        gen_ldst_softmmu_helper(LISA_ST_W, &base, &mem, save_temp);
    }
#else
    gen_ldst_softmmu_helper(LISA_ST_W, &base, &mem, save_temp);
#endif

    latxs_ra_free_temp(&base);
    latxs_ra_free_temp(&limit);
    return true;
}

bool latxs_translate_lgdt(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_LGDT(pir1);

    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1),
            ir1_opnd_is_mem(opnd),
            "not a valid LGDT insn: pir1 = %p\n", (void *)pir1);

    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd, -1);

    IR2_OPND limit = latxs_ra_alloc_itemp();
    IR2_OPND base = latxs_ra_alloc_itemp();

    int save_temp = 1;

    /* 1. load 2 bytes for limit at MEM(addr)*/
    gen_ldst_softmmu_helper(LISA_LD_HU, &limit, &mem_opnd, save_temp);

    /* 2. load 4 bytes for base address at MEM(addr + 2)*/
    IR2_OPND mem = latxs_convert_mem_ir2_opnd_plus_2(&mem_opnd);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        gen_ldst_softmmu_helper(LISA_LD_D, &base, &mem, save_temp);
    } else {
        gen_ldst_softmmu_helper(LISA_LD_WU, &base, &mem, save_temp);
    }
#else
    gen_ldst_softmmu_helper(LISA_LD_WU, &base, &mem, save_temp);
#endif

    /* 3. 24-bits long base address in Real-Address mode and vm86 mode */
    if (latxs_ir1_data_size(pir1) == 16) {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        IR2_OPND tmp1 = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_SRAI_D, &tmp, &base, 0x10);
        latxs_append_ir2_opnd2i(LISA_ANDI, &tmp, &tmp, 0xff);
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &tmp, &tmp, 0x10);
        latxs_append_ir2_opnd2_(lisa_mov16z, &tmp1, &base);
        latxs_append_ir2_opnd3(LISA_OR, &tmp, &tmp, &tmp1);
        base = tmp;
    }

    /* 4. store limit/base into GDTR */
#ifdef TARGET_X86_64
    latxs_append_ir2_opnd2i(LISA_ST_D, &base, &latxs_env_ir2_opnd,
            lsenv_offset_of_gdtr_base(lsenv));
#else
    latxs_append_ir2_opnd2i(LISA_ST_W, &base, &latxs_env_ir2_opnd,
            lsenv_offset_of_gdtr_base(lsenv));
#endif
    latxs_append_ir2_opnd2i(LISA_ST_W, &limit, &latxs_env_ir2_opnd,
            lsenv_offset_of_gdtr_limit(lsenv));

    return true;
}

bool latxs_translate_sgdt(IR1_INST *pir1)
{
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1),
            ir1_opnd_is_mem(opnd),
            "not a valid SGDT insn: pir1 = %p\n", (void *)pir1);

    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd, -1);

    /* 0. mem_opnd might be temp register */
    int save_temp = 1;

    /* 1. load gdtr.limit from env */
    IR2_OPND limit = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_W, &limit, &latxs_env_ir2_opnd,
            lsenv_offset_of_gdtr_limit(lsenv));
    /* 2. store 16-bits limit at MEM(addr) */
    gen_ldst_softmmu_helper(LISA_ST_H, &limit, &mem_opnd, save_temp);
    latxs_ra_free_temp(&limit);

    /* 3. load gdtr.base  from env */
    IR2_OPND base = latxs_ra_alloc_itemp();
#ifdef TARGET_X86_64
    latxs_append_ir2_opnd2i(LISA_LD_D, &base, &latxs_env_ir2_opnd,
            lsenv_offset_of_gdtr_base(lsenv));
#else
    latxs_append_ir2_opnd2i(LISA_LD_W, &base, &latxs_env_ir2_opnd,
            lsenv_offset_of_gdtr_base(lsenv));
#endif

    /* 4. store 32-bit base at MEM(addr + 2) */
    IR2_OPND mem = latxs_convert_mem_ir2_opnd_plus_2(&mem_opnd);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        gen_ldst_softmmu_helper(LISA_ST_D, &base, &mem, save_temp);
    } else {
        gen_ldst_softmmu_helper(LISA_ST_W, &base, &mem, save_temp);
    }
#else
    gen_ldst_softmmu_helper(LISA_ST_W, &base, &mem, save_temp);
#endif

    latxs_ra_free_temp(&base);

    return true;
}

static void latxs_translate_lldt_gpr(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /* helper might cause exception, save complete CPUX86State */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* void helper_lldt(CPUX86State  *env, int selector) */

    /* 1. get selector into arg1 (a1) */
    latxs_load_ir1_gpr_to_ir2(&latxs_arg1_ir2_opnd, opnd0, EXMode_Z);

    /* 2. call helper */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);
    latxs_tr_gen_call_to_helper((ADDR)helper_lldt);

    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
}

static void latxs_translate_lldt_mem(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /* 0. load selector value */
    IR2_OPND selector = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&selector, opnd0, EXMode_Z, -1);

    /* helper might cause exception, save complete CPUX86State */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* void helper_lldt(CPUX86State  *env, int selector) */

    /* 1. get selector into arg1 (a1) */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg1_ir2_opnd,
            &selector, &latxs_zero_ir2_opnd);

    /* 2. call helper */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);
    latxs_tr_gen_call_to_helper((ADDR)helper_lldt);

    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
}

bool latxs_translate_lldt(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_LLDT(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR or MEM */

    if (ir1_opnd_is_gpr(opnd0)) {
        latxs_translate_lldt_gpr(pir1, opnd0);
    } else {
        lsassertm_illop(ir1_addr(pir1),
                ir1_opnd_is_mem(opnd0),
                "not a valid LLDT insn: pir1 = %p\n", (void *)pir1);

        latxs_translate_lldt_mem(pir1, opnd0);
    }

    return true;
}

bool latxs_translate_sldt(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_SLDT(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR or MEM */

    /* load ldtr.selector from env */
    IR2_OPND selector = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_W, &selector, &latxs_env_ir2_opnd,
            lsenv_offset_of_ldtr_selector(lsenv));

    if (ir1_opnd_is_mem(opnd0)) {
        lsassert(ir1_opnd_size(opnd0) == 16);
    }

    latxs_store_ir2_to_ir1(&selector, opnd0);

    return true;
}

static void latxs_translate_ltr_gpr(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /* helper might cause exception, save complete CPUX86State */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* void helper_ltr(CPUX86State  *env, int selector) */

    /* 1. get selector into arg1 (a1) */
    latxs_load_ir1_gpr_to_ir2(&latxs_arg1_ir2_opnd, opnd0, EXMode_Z);

    /* 2. call helper */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);
    latxs_tr_gen_call_to_helper((ADDR)helper_ltr);

    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
}

static void latxs_translate_ltr_mem(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /* helper might cause exception, save complete CPUX86State */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* void helper_ltr(CPUX86State  *env, int selector) */

    /* 1. get selector into arg1 (a1) */
    IR2_OPND selector = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&selector, opnd0, EXMode_Z, -1);
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg1_ir2_opnd,
            &selector, &latxs_zero_ir2_opnd);

    /* 2. call helper */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);
    latxs_tr_gen_call_to_helper((ADDR)helper_ltr);

    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
}

bool latxs_translate_ltr(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_LTR(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR or MEM */

    if (ir1_opnd_is_gpr(opnd0)) {
        latxs_translate_ltr_gpr(pir1, opnd0);
    } else {
        lsassertm_illop(ir1_addr(pir1),
                ir1_opnd_is_mem(opnd0),
                "not a valid LTR insn: pir1 = %p\n", (void *)pir1);
        latxs_translate_ltr_mem(pir1, opnd0);
    }

    return true;
}

bool latxs_translate_str(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_STR(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    /* load tr.selector from env */
    IR2_OPND selector = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_W, &selector, &latxs_env_ir2_opnd,
            lsenv_offset_of_tr_selector(lsenv));

    latxs_store_ir2_to_ir1(&selector, opnd0);

    return true;
}

/*
 * push/pop
 *
 * In system-mode, for precise exception, the effect of instruction
 * should be executed at the end of this instruction's translated code.
 *
 * For pop, these two things need to be done at the end:
 *  1> save the read value into destination
 *  2> update esp register
 *
 * For push, since the write is done by softmmu, and the exception can
 * only be generated by the softmmu helper, only one thing need to be
 * done at the end:
 *  1> update esp register
 */

/* End of TB in system-mode : pop es/ss/ds */
bool latxs_translate_pop(IR1_INST *pir1)
{
    /*
     * pop
     * ----------------------
     * >  dest <= MEM(SS:ESP)
     * >  ESP  <= ESP + 2/4
     * ----------------------
     * 1. tmp  <= MEM(SS:ESP) : softmmu
     * 2. ESP  <= ESP + 2/4
     * 3. tmp  => dest : gpr/mem/seg
     */
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    TRANSLATION_DATA *td = lsenv->tr_data;

    bool is_gpr_esp = 0;
    bool is_mem_esp = 0;
    bool ss32 = lsenv->tr_data->sys.ss32;

    if (ir1_opnd_is_gpr(opnd0)) {
        if (ir1_opnd_base_reg_num(opnd0) == esp_index) {
            is_gpr_esp = 1;
        }
    }

    if (ir1_opnd_is_mem(opnd0)) {
        if (ir1_opnd_base_reg_num(opnd0) == esp_index) {
            is_mem_esp = 1;
        }
    }

    int data_size = latxs_ir1_data_size(pir1);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        data_size = (data_size == 16) ? 16 : 64;
    }
#endif
    int esp_inc   = data_size >> 3;

    int opnd_size = ir1_opnd_size(opnd0);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        lsassert(opnd_size == 16 || opnd_size == 64);
    } else {
        lsassert(opnd_size == 16 || opnd_size == 32);
    }
#else
    lsassert(opnd_size == 16 || opnd_size == 32);
#endif
    if (!ir1_opnd_is_seg(opnd0)) {
        lsassert(data_size == opnd_size);
    }

    lsassert(data_size >= opnd_size);
    (void)data_size; /* to avoid compile warning */
    (void)opnd_size; /* to avoid compile warning */

    /*
     * TODO
    if (option_by_hand && lsenv->tr_data->sys.ss32 &&
        !lsenv->tr_data->sys.addseg && ir1_opnd_is_gpr(opnd0) &&
        ir1_opnd_num(pir1) == 1 && !is_gpr_esp && data_size == 32) {
        IR2_OPND esp_mem_opnd;
        IR2_OPND esp = ra_alloc_gpr(esp_index);
        ir2_opnd_build2(&esp_mem_opnd, IR2_OPND_MEM, esp._reg_num, 0);
        IR2_OPND dest_reg = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        gen_ldst_softmmu_helper(
            ir1_opnd_default_em(opnd0) == SIGN_EXTENSION ? mips_lw : mips_lwu,
            &dest_reg, &esp_mem_opnd, 0);
        IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
        append_ir2_opnd2i(mips_addi_addrx, &esp_opnd, &esp_opnd, esp_inc);
        return true;
    }
    */

    /* 1.1 build MEM(SS:ESP) */
    IR1_OPND mem_ir1_opnd;
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
            X86_REG_SS, X86_REG_ESP, 0, 0, 0);

    /* 1.2 read data from stack   : might generate exception */
    IR2_OPND tmp = latxs_ra_alloc_itemp();
    int ss_addr_size = latxs_get_sys_stack_addr_size();
    latxs_load_ir1_mem_to_ir2(&tmp, &mem_ir1_opnd, EXMode_Z,
             ss_addr_size);

    /* 2. update ESP */
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
    if (is_gpr_esp) {
#ifdef TARGET_X86_64
        if (lsenv->tr_data->sys.code64) {
            latxs_append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd, esp_inc);
        } else
#endif
        if (ss32) {
            latxs_append_ir2_opnd2i(LISA_ADDI_W,
                    &esp_opnd, &esp_opnd, esp_inc);
#ifdef TARGET_X86_64
            latxs_append_ir2_opnd2_(lisa_mov32z, &esp_opnd, &esp_opnd);
#else
            if (option_by_hand) {
                latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
            }
#endif
        } else {
            IR2_OPND tmp = latxs_ra_alloc_itemp();
            latxs_append_ir2_opnd2i(LISA_ADDI_D,
                    &tmp, &esp_opnd, esp_inc);
            latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
            latxs_ra_free_temp(&tmp);
        }
    }

    /* 3. write into destination  : might generate exception */
    if (is_mem_esp) {
        td->sys.popl_esp_hack = data_size >> 3;
        latxs_store_ir2_to_ir1_mem(&tmp, opnd0, ss_addr_size);
        td->sys.popl_esp_hack = 0;
    } else {
        latxs_store_ir2_to_ir1(&tmp, opnd0);
    }

    /*
     * Order is important for pop %esp
     * The %esp is increased first. Then the pop value is loaded into %esp.
     * This executing order is the same as that in TCG.
     */
    if (!is_gpr_esp) {
#ifdef TARGET_X86_64
        if (lsenv->tr_data->sys.code64) {
            latxs_append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd, esp_inc);
            return true;
        }
#endif
        if (ss32) {
            latxs_append_ir2_opnd2i(LISA_ADDI_D,
                    &esp_opnd, &esp_opnd, esp_inc);
#ifdef TARGET_X86_64
            latxs_append_ir2_opnd2_(lisa_mov32z, &esp_opnd, &esp_opnd);
#endif
        } else {
            IR2_OPND tmp = latxs_ra_alloc_itemp();
            latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd, esp_inc);
            latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
            latxs_ra_free_temp(&tmp);
        }
    }

    return true;
}

bool latxs_translate_push(IR1_INST *pir1)
{
    /*
     * push
     * ----------------------
     * >  ESP  <= ESP - 2/4
     * >  src  => MEM(SS:ESP)
     * ----------------------
     * 1. tmp  <= source : gpr/mem/seg
     * 2. tmp  => MEM(SS:ESP - 2/4) : softmmu
     * 3. ESP  <= ESP - 2/4
     */
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    bool ss32 = lsenv->tr_data->sys.ss32;

    int data_size = latxs_ir1_data_size(pir1);

#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        data_size = (data_size == 16) ? 16 : 64;
    }
#endif

    int esp_dec   = 0 - (data_size >> 3);

    int opnd_size = ir1_opnd_size(opnd0);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        lsassert(opnd_size == 16 || opnd_size == 64);
    } else {
        lsassert(opnd_size == 16 || opnd_size == 32);
    }
#else
    lsassert(opnd_size == 16 || opnd_size == 32);
#endif
    if (!ir1_opnd_is_seg(opnd0)) {
#ifdef TARGET_X86_64
        /*
         * TODO: capstone bug
         * 66 68 00
         * push imm with 0x66 prefix, operand size mistake
         */
        if (!ir1_opnd_is_imm(opnd0)) {
            lsassert(data_size == opnd_size);
        }
#else
        lsassert(data_size == opnd_size);
#endif
    }

    (void)opnd_size; /* to avoid compile warning */

    /*
     * TODO
    if (option_by_hand && lsenv->tr_data->sys.ss32 &&
        !lsenv->tr_data->sys.addseg && ir1_opnd_is_gpr(opnd0) &&
        ir1_opnd_num(pir1) == 1 && data_size == 32) {
        IR2_OPND esp_mem_opnd;
        IR2_OPND esp = ra_alloc_gpr(esp_index);
        ir2_opnd_build2(&esp_mem_opnd, IR2_OPND_MEM, esp._reg_num, esp_dec);
        IR2_OPND src_reg = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        gen_ldst_softmmu_helper(mips_sw, &src_reg, &esp_mem_opnd, 0);
        IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
        append_ir2_opnd2i(mips_addi_addrx, &esp_opnd, &esp_opnd, esp_dec);
        return true;
    }
    */

    /* 1. load source data   : might generate exception */
    IR2_OPND tmp = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&tmp, opnd0, EXMode_N);

    /* 2.1 build MEM(SS:ESP - 2/4)*/
    IR1_OPND mem_ir1_opnd;
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
            X86_REG_SS, X86_REG_ESP, esp_dec, 0, 0);

    /* 2.2 write data into stack   : might generate exception */
    int ss_addr_size = latxs_get_sys_stack_addr_size();
    latxs_store_ir2_to_ir1_mem(&tmp, &mem_ir1_opnd,
             ss_addr_size);

    /* 3. update ESP */
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd, esp_dec);
    } else
#endif
    if (ss32) {
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &esp_opnd, &esp_opnd, esp_dec);
#ifdef TARGET_X86_64
        latxs_append_ir2_opnd2_(lisa_mov32z, &esp_opnd, &esp_opnd);
#else
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
        }
#endif
    } else {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd, esp_dec);
        latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
        latxs_ra_free_temp(&tmp);
    }

    return true;
}

bool latxs_translate_ret(IR1_INST *pir1)
{
    /*
     * ESP update   according to dflag(opnd size)
     * Value size   according to dflag(opnd size)
     * Address size according to aflag(addr size) : load ret addr
     *
     * code32 || (code16 && prefix_data) : dflag = 4
     * code16 || (code32 && prefix_data) : dflag = 2
     *
     * aflag = ir1_addr_size(pir1) : provided by capstone
     * code32 || (code16 && prefix_addr) : aflag = 4
     * code16 || (code32 && prefix_addr) : aflag = 2
     *
     * Usually the dflag(opnd size) stores in inst's operand.
     * But 'ret' is allowed to have zero operand. And if it has
     * one operand, it must be 'imm16' which must has 16-bit opnd size.
     * So there is no way to get the dflag from pir1 itself.
     *
     * In normal situation, the capstone will translate zero operand with
     * different opnd size into serival instruction, such as 'ins'.
     * But near 'ret' has only one instruction in capstone......
     */

    int data_size = latxs_ir1_data_size(pir1);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        data_size = (data_size == 16) ? 16 : 64;
    }
#endif
    int addr_size = latxs_ir1_addr_size(pir1);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        lsassert(data_size == 64);
        lsassert(addr_size == 8);
    } else {
        lsassert(data_size == 16 || data_size == 32);
        lsassert(addr_size == 2 || addr_size == 4);
    }
#else
    lsassert(data_size == 16 || data_size == 32);
    lsassert(addr_size == 2 || addr_size == 4);
#endif

    /* 1. load ret_addr into $25 from MEM(SS:ESP) */
    IR1_OPND mem_ir1_opnd;
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
            X86_REG_SS, X86_REG_ESP, 0, 0, 0);
    IR2_OPND return_addr_opnd = latxs_ra_alloc_itemp();
    int ss_addr_size = latxs_get_sys_stack_addr_size();
    latxs_load_ir1_mem_to_ir2(&return_addr_opnd,
            &mem_ir1_opnd, EXMode_Z, ss_addr_size);

    /* 2. apply address size */
    if (data_size == 32 && addr_size == 2) {
        latxs_append_ir2_opnd2_(lisa_mov16z, &return_addr_opnd,
                                             &return_addr_opnd);
    }
#ifdef TARGET_X86_64
    latxs_append_ir2_opnd2i(LISA_ST_D, &return_addr_opnd, &latxs_env_ir2_opnd,
                            lsenv_offset_of_eip(lsenv));
#else
    latxs_append_ir2_opnd2i(LISA_ST_W, &return_addr_opnd, &latxs_env_ir2_opnd,
                            lsenv_offset_of_eip(lsenv));
#endif

    /* 3. update ESP */
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
    if (pir1 != NULL && ir1_opnd_num(pir1) &&
            ir1_opnd_is_imm(ir1_get_opnd(pir1, 0))) {
#ifdef TARGET_X86_64
        if (lsenv->tr_data->sys.code64) {
            latxs_append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd,
                ir1_opnd_uimm(ir1_get_opnd(pir1, 0)) + (data_size >> 3));
            if (option_by_hand) {
                lsassert(0);
            }
        } else
#endif
        if (lsenv->tr_data->sys.ss32) {
            latxs_append_ir2_opnd2i(LISA_ADDI_W, &esp_opnd, &esp_opnd,
                ir1_opnd_uimm(ir1_get_opnd(pir1, 0)) + (data_size >> 3));
#ifdef TARGET_X86_64
            latxs_append_ir2_opnd2_(lisa_mov32z, &esp_opnd, &esp_opnd);
#else
            if (option_by_hand) {
                latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
            }
#endif
        } else {
            IR2_OPND tmp = latxs_ra_alloc_itemp();
            latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd,
                ir1_opnd_uimm(ir1_get_opnd(pir1, 0)) + (data_size >> 3));
            latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
            latxs_ra_free_temp(&tmp);
        }
    } else {
#ifdef TARGET_X86_64
        if (lsenv->tr_data->sys.code64) {
            latxs_append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd,
                              (data_size >> 3));
            if (option_by_hand) {
                lsassert(0);
            }
        } else
#endif
        if (lsenv->tr_data->sys.ss32) {
            latxs_append_ir2_opnd2i(LISA_ADDI_W, &esp_opnd, &esp_opnd,
                              (data_size >> 3));
#ifdef TARGET_X86_64
            latxs_append_ir2_opnd2_(lisa_mov32z, &esp_opnd, &esp_opnd);
#else
            if (option_by_hand) {
                latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
            }
#endif
        } else {
            IR2_OPND tmp = latxs_ra_alloc_itemp();
            latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd,
                              (data_size >> 3));
            latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
            latxs_ra_free_temp(&tmp);
        }
    }

    latxs_tr_generate_exit_tb(pir1, 0);

    return true;
}

bool latxs_translate_cpuid(IR1_INST *pir1)
{
    /* 0. save next instruciton's EIP to env */
    latxs_tr_gen_save_curr_eip();

    /*
     * 1. call helper_cpuid
     *
     * void helper_cpuid(
     *      CPUX86State *env)
     * >> load new EAX/ECX/EDX/EBX
     */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_cpuid, cfg);

    return true;
}

bool latxs_translate_xchg(IR1_INST *pir1)
{
    /* processor’s locking protocol is automatically implemented  */
    if ((lsenv->tr_data->sys.cflags & CF_PARALLEL) &&
        ir1_opnd_is_mem(ir1_get_opnd(pir1, 0))) {
        return latxs_translate_lock_xchg(pir1);
    }
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    /* if two src is the same reg, do nothing */
    if (ir1_opnd_is_gpr(opnd0) && ir1_opnd_is_gpr(opnd1)) {
        if ((ir1_opnd_size(opnd0) ==
             ir1_opnd_size(opnd1)) &&
            (ir1_opnd_base_reg_num(opnd0) ==
             ir1_opnd_base_reg_num(opnd1)) &&
            (ir1_opnd_base_reg_bits_start(opnd0) ==
             ir1_opnd_base_reg_bits_start(opnd1))) {
#ifdef TARGET_X86_64
            if (ir1_opnd_size(opnd0) == 32) {
                IR2_OPND reg_opnd =
                    latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
                latxs_append_ir2_opnd2_(lisa_mov32z, &reg_opnd, &reg_opnd);
            }
#endif
            return true;
        }
    }

    IR2_OPND src_opnd_0 = latxs_ra_alloc_itemp();
    IR2_OPND src_opnd_1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src_opnd_0, opnd0, EXMode_N);
    latxs_load_ir1_to_ir2(&src_opnd_1, opnd1, EXMode_N);

    /* Do memory access first for precise exception */
    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&src_opnd_1, opnd0);
        latxs_store_ir2_to_ir1(&src_opnd_0, opnd1);
    } else {
        latxs_store_ir2_to_ir1(&src_opnd_0, opnd1);
        latxs_store_ir2_to_ir1(&src_opnd_1, opnd0);
    }

    return true;
}

bool latxs_translate_cmpxchg(IR1_INST *pir1)
{
    if (latxs_ir1_has_prefix_lock(pir1) &&
        (lsenv->tr_data->sys.cflags & CF_PARALLEL)) {
        return latxs_translate_lock_cmpxchg(pir1);
    }
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR1_OPND *reg_ir1 = NULL;

    int opnd_size = ir1_opnd_size(opnd0);
    switch (opnd_size) {
    case 8:
        reg_ir1 = &al_ir1_opnd;
        break;
    case 16:
        reg_ir1 = &ax_ir1_opnd;
        break;
    case 32:
        reg_ir1 = &eax_ir1_opnd;
        break;
#ifdef TARGET_X86_64
    case 64:
        lsassert(lsenv->tr_data->sys.code64);
        reg_ir1 = &rax_ir1_opnd;
        break;
#endif
    default:
        lsassert(0);
        break;
    }

    IR2_OPND src_opnd_0 = latxs_ra_alloc_itemp();
    IR2_OPND src_opnd_1 = latxs_ra_alloc_itemp();
    IR2_OPND eax_opnd = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src_opnd_0, opnd0, EXMode_S);
    latxs_load_ir1_to_ir2(&src_opnd_1, opnd1, EXMode_S);
    latxs_load_ir1_to_ir2(&eax_opnd, reg_ir1, EXMode_S);

    IR2_OPND dest_opnd = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SUB_D,
            &dest_opnd, &eax_opnd, &src_opnd_0);

    IR2_OPND label_unequal = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BNE,
            &src_opnd_0, &eax_opnd, &label_unequal);

    /* equal */
    latxs_store_ir2_to_ir1(&src_opnd_1, opnd0);
    latxs_ra_free_temp(&src_opnd_1);

    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd1(LISA_B, &label_exit);

    /* unequal */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_unequal);
    latxs_store_ir2_to_ir1_gpr(&src_opnd_0, reg_ir1);
    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);

    /* calculate elfags after compare and exchange(store) */
    latxs_generate_eflag_calculation(&dest_opnd,
            &eax_opnd, &src_opnd_0, pir1, true);

    latxs_ra_free_temp(&dest_opnd);

    return true;
}

bool latxs_translate_cmpxchg8b(IR1_INST *pir1)
{
    if (latxs_ir1_has_prefix_lock(pir1) &&
        (lsenv->tr_data->sys.cflags & CF_PARALLEL)) {
        return latxs_translate_lock_cmpxchg8b(pir1);
    }
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_ARPL(pir1);

    /* 1. check illegal operation exception */
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    lsassert_illop(ir1_addr(pir1), ir1_opnd_is_mem(opnd0));

    /* 2. get memory address */
    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd0, -1);
    int mem_no_offset_new_tmp = 0;
    IR2_OPND mem_no_offset = latxs_convert_mem_ir2_opnd_no_offset(
            &mem_opnd, &mem_no_offset_new_tmp);
    IR2_OPND address = latxs_ir2_opnd_mem_get_base(&mem_no_offset);
    if (mem_no_offset_new_tmp) {
        latxs_ra_free_temp(&mem_opnd);
    }

    /* 3. select helper function */
    ADDR helper_addr = 0;
    if (latxs_ir1_has_prefix_lock(pir1) &&
            td->sys.cflags & CF_PARALLEL) {
        /*
         * target/i386/mem_helper.c
         * void helper_cmpxchg8b(
         *      CPUX86State *env,
         *      target_ulong a0)
         * >> EAX, ECX is used
         */
        helper_addr = (ADDR)helper_cmpxchg8b;
    } else {
        /*
         * target/i386/mem_helper.c
         * void helper_cmpxchg8b_unlocked(
         *      CPUX86State *env,
         *      target_ulong a0)
         * >> EAX, ECX is used
         */
        helper_addr = (ADDR)helper_cmpxchg8b_unlocked;
    }

    /* 4. call that helper */

    /* 4.1 save context */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    /* 4.2 arg1: address */
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd,
                                      &address);
    /* 4.3 arg0: env */
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                      &latxs_env_ir2_opnd);
    /* 4.4 call helper */
    latxs_tr_gen_call_to_helper(helper_addr);
    /* 4.5 restore context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    return true;
}

bool latxs_translate_cmpxchg16b(IR1_INST *pir1)
{
#ifdef TARGET_X86_64
    if (latxs_ir1_has_prefix_lock(pir1) &&
        (lsenv->tr_data->sys.cflags & CF_PARALLEL)) {
        return latxs_translate_lock_cmpxchg16b(pir1);
    }
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_ARPL(pir1);

    /* 1. check illegal operation exception */
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    lsassert_illop(ir1_addr(pir1), ir1_opnd_is_mem(opnd0));

    /* 2. get memory address */
    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd0, -1);
    int mem_no_offset_new_tmp = 0;
    IR2_OPND mem_no_offset = latxs_convert_mem_ir2_opnd_no_offset(
            &mem_opnd, &mem_no_offset_new_tmp);
    IR2_OPND address = latxs_ir2_opnd_mem_get_base(&mem_no_offset);
    if (mem_no_offset_new_tmp) {
        latxs_ra_free_temp(&mem_opnd);
    }

    /* 3. select helper function */
    ADDR helper_addr = 0;
    if (latxs_ir1_has_prefix_lock(pir1) &&
            td->sys.cflags & CF_PARALLEL) {
        /*
         * target/i386/mem_helper.c
         * void helper_cmpxchg8b(
         *      CPUX86State *env,
         *      target_ulong a0)
         * >> EAX, ECX is used
         */
        helper_addr = (ADDR)helper_cmpxchg16b;
    } else {
        /*
         * target/i386/mem_helper.c
         * void helper_cmpxchg8b_unlocked(
         *      CPUX86State *env,
         *      target_ulong a0)
         * >> EAX, ECX is used
         */
        helper_addr = (ADDR)helper_cmpxchg16b_unlocked;
    }

    /* 4. call that helper */

    /* 4.1 save context */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);
    /* 4.2 arg1: address */
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd,
                                      &address);
    /* 4.3 arg0: env */
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                      &latxs_env_ir2_opnd);
    /* 4.4 call helper */
    latxs_tr_gen_call_to_helper(helper_addr);
    /* 4.5 restore context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    return true;
#else
    return false;
#endif
}

/* End of TB in system-mode */
bool latxs_translate_rsm(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_RSM(pir1);

    /* 1. save next instruciton's EIP to env */
    latxs_tr_gen_save_next_eip();

    /*
     * 2. helper_rsm
     *
     * target/i386/smm_helper.c
     * void helper_rsm(
     *      CPUX86State *env)
     */
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_rsm, default_helper_cfg);

    /* the eip is already updated in helper_rsm */
    lsenv->tr_data->ignore_eip_update = 1;

    return true;
}

bool latxs_translate_invd(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_INVD(pir1);
    /* nothing to do */
    return true;
}

bool latxs_translate_wbinvd(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_WBINVD(pir1);
    /* nothing to do */
    return true;
}

/* End of TB in system-mode : reps nop */
bool latxs_translate_nop(IR1_INST *pir1)
{
    if (latxs_ir1_is_repz_nop(pir1)) {

        /* 1. save full context */
        helper_cfg_t cfg = default_helper_cfg;
        latxs_tr_gen_call_to_helper_prologue_cfg(cfg);

        /*
         * 2. call helper_pause
         *
         * target/i386/misc_helper.c
         * void helper_pause(
         *      CPUX86State *env,
         *      int next_eip_addend)
         */

        /* 2.1 arg0: env */
        latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
                &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);
        /* 2.2 arg1: this inst's size, 12-bit is enough */
        int size = latxs_ir1_inst_size(pir1);
        latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg1_ir2_opnd,
                &latxs_zero_ir2_opnd, size & 0xfff);
        /* 2.3 call helper pause */
        latxs_tr_gen_call_to_helper((ADDR)helper_pause);

        /* 3. This helper never return */
        latxs_tr_gen_infinite_loop();

    } else {
        if (pir1->info->size == 4 && pir1->info->bytes[0] == 0x0f &&
            pir1->info->bytes[1] == 0x1f && pir1->info->bytes[2] == 0x40) {
            int func = pir1->info->bytes[3];
            if (func >= 0xa0) {
                do_func(ir1_addr(pir1), func);
            }
        }
    }

    return true;
}

/* End of TB in system-mode */
bool latxs_translate_pause(IR1_INST *pir1)
{
    /* 0. save current instruciton's EIP to env */
    latxs_tr_gen_save_curr_eip();

    /* 1. save full context */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper_prologue_cfg(cfg);

    /*
     * 2. call helper_pause
     *
     * target/i386/misc_helper.c
     * void helper_pause(
     *      CPUX86State *env,
     *      int next_eip_addend)
     */

    /* 2.1 arg0: env */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);
    /* 2.2 arg1: this inst's size, 16-bit is enough */
    int size = latxs_ir1_inst_size(pir1);
    latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg1_ir2_opnd,
            &latxs_zero_ir2_opnd, size & 0xfff);
    /* 2.3 call helper pause */
    latxs_tr_gen_call_to_helper((ADDR)helper_pause);

    /* 3. This helper never return */
    latxs_tr_gen_infinite_loop();

    return true;
}

static bool latxs_do_translate_iret(IR1_INST *pir1, int size)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_IRET(pir1);

#ifdef TARGET_X86_64
        if (lsenv->tr_data->sys.code64) {
            lsassert(size == 0 || size == 1 || size == 2);
        } else {
            lsassert(size == 0 || size == 1);
        }
#else
    lsassert(size == 0 || size == 1);
#endif

    /* 1. save complete context */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper_prologue_cfg(cfg);

    if (td->sys.pe && !td->sys.vm86) {
        /*
         * 2. protected mode iret
         *
         * target/i386/seg_helper.c
         * void helper_iret_protected(
         *      CPUX86State *env,
         *      int shift,
         *      int next_eip)
         */
        ADDRX next_eip = ir1_addr_next(pir1);
        /* 2.1 arg2: next eip */
        latxs_load_addrx_to_ir2(&latxs_arg2_ir2_opnd, next_eip);
        /* 2.2 arg1: size */
        latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg1_ir2_opnd,
                &latxs_zero_ir2_opnd, size);
        /* 2.3 arg0: env */
        latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
                &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);
        /* 2.4 call helper_iret_protected : might generate exception  */
        latxs_tr_gen_call_to_helper((ADDR)helper_iret_protected);
    } else {
        /*
         * 2. real and vm86 mode iret
         *
         * target/i386/seg_helper.c
         * void helper_iret_real(
         *      CPUX86State *env,
         *      int shift)
         */
        /* 2.1 arg0: env */
        latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
                &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);
        /* 2.2 arg1: size */
        latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg1_ir2_opnd,
                &latxs_zero_ir2_opnd, size);
        /* 2.3 call helper_iret_real : ESP is updated, */
        /* so we must save all gpr */
        latxs_tr_gen_call_to_helper((ADDR)helper_iret_real);
    }

    /* 3. restore native context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(cfg);

    /* 5. disable eip update, since the helper modify eip */
    td->ignore_eip_update = 1;

    return true;
}

/* End of TB in system-mode */
bool latxs_translate_iret(IR1_INST *pir1)
{
    IR1_OPCODE opc = ir1_opcode(pir1);

    if (opc == X86_INS_IRET) {
        /* 16-bit opnd size */
        return latxs_do_translate_iret(pir1, 0);
    } else if (opc == X86_INS_IRETD) {
        /* 32-bit opnd size */
        return latxs_do_translate_iret(pir1, 1);
    } else if (opc == X86_INS_IRETQ) {
#ifdef TARGET_X86_64
        if (lsenv->tr_data->sys.code64) {
            return latxs_do_translate_iret(pir1, 2);
        } else {
            lsassert(0);
        }
#else
        lsassert(0);
#endif
    } else {
        lsassert(0);
    }

    return false;
}

bool latxs_translate_int(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_INT(pir1);

    /* 0. save current instruciton's EIP to env */
    latxs_tr_gen_save_curr_eip();

    /* 1. save complete context */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper_prologue_cfg(cfg);

    /*
     * 2. raise interrupt
     *
     *  target/i386/excp_helper.c
     *  void helper_raise_interrupt(
     *      CPUX86State *env,
     *      int intno,
     *      int next_eip_addend)
     */

    /* 2.1 arg1: intno */
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    int intno = ir1_opnd_simm(opnd);
    latxs_load_imm32_to_ir2(&latxs_arg1_ir2_opnd, intno, EXMode_Z);
    /* 2.2 arg2 : next eip addend : instruction size */
    int size = latxs_ir1_inst_size(pir1);
    latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg2_ir2_opnd,
            &latxs_zero_ir2_opnd, size & 0xfff);
    /* 2.3 arg0 : env */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);
    /* 2.4 call helper_raise_interrupt: generate interrupt */
    latxs_tr_gen_call_to_helper((ADDR)helper_raise_interrupt);

    /* 3. This helper never return */
    latxs_tr_gen_infinite_loop();

    return true;
}

bool latxs_translate_int_3(IR1_INST *pir1)
{
    /* 0. save current instruciton's EIP to env */
    latxs_tr_gen_save_curr_eip();

    /* 1. save complete context */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper_prologue_cfg(cfg);

    /*
     * 2. raise interrupt
     *
     *  target/i386/excp_helper.c
     *  void helper_raise_interrupt(
     *      CPUX86State *env,
     *      int intno,
     *      int next_eip_addend)
     */

    /* 2.1 arg1: intno = EXCP03_INT3 */
    latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg1_ir2_opnd,
            &latxs_zero_ir2_opnd, EXCP03_INT3);
    /* 2.2 arg2: next eip addend */
    int size = latxs_ir1_inst_size(pir1);
    latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg2_ir2_opnd,
            &latxs_zero_ir2_opnd, size & 0xfff);
    /* 2.3 arg0 : env */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);
    /* 2.4 call helper_raise_interrupt: generate interrupt */
    latxs_tr_gen_call_to_helper((ADDR)helper_raise_interrupt);

    /* 3. This helper never return */
    latxs_tr_gen_infinite_loop();

    return true;
}

bool latxs_translate_into(IR1_INST *pir1)
{
    /* 0. save current instruciton's EIP to env */
    latxs_tr_gen_save_curr_eip();

    /* 1. save complete context */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper_prologue_cfg(cfg);

    /*
     * 2. helper_into
     *
     *  target/i386/misc_helper.c
     *  void helper_into(
     *      CPUX86State *env,
     *      int next_eip_addend)
     */

    /* 2.1 arg2: next eip addend */
    int size = latxs_ir1_inst_size(pir1);
    latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg1_ir2_opnd,
            &latxs_zero_ir2_opnd, size & 0xfff);
    /* 2.2 arg0 : env */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);
    /* 2.3 call helper_raise_interrupt: generate interrupt */
    latxs_tr_gen_call_to_helper((ADDR)helper_into);

    /* 3. restore context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(cfg);

    return true;
}

bool latxs_translate_int1(IR1_INST *pir1)
{
    /* 0. save current instruciton's EIP to env */
    latxs_tr_gen_save_curr_eip();

    /*
     * 1. helper_debug
     *
     * target/i386/misc_helper.c
     * void helper_debug(
     *      CPUX86State *env)
     */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_debug, cfg);

    /* 2. helper debug never return */
    latxs_tr_gen_infinite_loop();

    return true;
}

static void latxs_do_translate_ret_far_pe(
        IR1_INST *pir1, int size, int esp_inc)
{
    /* 0. save current instruciton's EIP to env */
    latxs_tr_gen_save_curr_eip();

    /* 1. save complete context */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* 2. prepare arguments */

    /* arg0 : env */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);
    /* arg1 : shift = opnd size (32-bit:1 ; 16-bit:0)*/
    latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg1_ir2_opnd,
            &latxs_zero_ir2_opnd, size);
    /* arg2 : addend to esp: esp inc */
    latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg2_ir2_opnd,
            &latxs_zero_ir2_opnd, esp_inc);

    /*
     * 3. call helper_lret_protected
     *    > might genrate exception
     *    > eflags is updated in helper
     *    > EIP is updated in helper
     */
    latxs_tr_gen_call_to_helper((ADDR)helper_lret_protected);
    /*
     * > protected mode
     *
     * target/i386/seg_helper.c
     * void helper_lret_protected(
     *      CPUX86State *env,
     *      int shift,
     *      int addend)
     */

    /* 4. restore context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* 5. disable eip update in context switch */
    lsenv->tr_data->ignore_eip_update = 1;
}

static void latxs_do_translate_ret_far_real(
        IR1_INST *pir1, int opnd_size, int esp_inc)
{
    int ss_addr_size = latxs_get_sys_stack_addr_size();
    /* 1. build MEM(SS:ESP) */
    IR1_OPND mem_ir1_opnd;
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, opnd_size,
            X86_REG_SS, X86_REG_ESP, 0, 0, 0);

    /* 2. load new EIP from MEM(SS:ESP) */
    IR2_OPND tmp_new_eip = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&tmp_new_eip,
            &mem_ir1_opnd, EXMode_Z, ss_addr_size);

    /* 3. load new CS from MEM(SS:ESP + 2/4) */
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, opnd_size,
            X86_REG_SS, X86_REG_ESP, (opnd_size >> 3), 0, 0);
    IR2_OPND tmp_new_cs = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&tmp_new_cs,
            &mem_ir1_opnd, EXMode_Z, ss_addr_size);

    /* 4. update env->eip */
#ifdef TARGET_X86_64
    latxs_append_ir2_opnd2i(LISA_ST_D, &tmp_new_eip, &latxs_env_ir2_opnd,
            lsenv_offset_of_eip(lsenv));
#else
    latxs_append_ir2_opnd2i(LISA_ST_W, &tmp_new_eip, &latxs_env_ir2_opnd,
            lsenv_offset_of_eip(lsenv));
#endif

    /* 5. update env->cs.base, env->cs.selector */
    latxs_append_ir2_opnd2i(LISA_ST_W, &tmp_new_cs, &latxs_env_ir2_opnd,
            lsenv_offset_of_seg_selector(lsenv, cs_index));
    latxs_append_ir2_opnd2i(LISA_SLLI_D, &tmp_new_cs, &tmp_new_cs, 0x4);
    latxs_append_ir2_opnd2i(LISA_ST_W, &tmp_new_cs, &latxs_env_ir2_opnd,
            lsenv_offset_of_seg_base(lsenv, cs_index));

    /* 6. update esp */
    esp_inc += opnd_size >> 2; /* (opnd_size >> 3) << 1 */

    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd, esp_inc);
        if (option_by_hand) {
            lsassert(0);
        }
    } else
#endif
    if (lsenv->tr_data->sys.ss32) {
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &esp_opnd, &esp_opnd, esp_inc);
#ifdef TARGET_X86_64
        latxs_append_ir2_opnd2_(lisa_mov32z, &esp_opnd, &esp_opnd);
#endif
    } else {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd, esp_inc);
        latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
        latxs_ra_free_temp(&tmp);
    }

    /* 7. disable eip update in context switch */
    /*    Since we already update eip here */
    lsenv->tr_data->ignore_eip_update = 1;
}

static bool latxs_do_translate_ret_far(
        IR1_INST *pir1, int size, int opnd_size)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    int esp_inc = 0;
    if (ir1_opnd_num(pir1)) {
        IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
        esp_inc = ir1_opnd_simm(opnd);
    }

    if (td->sys.pe && !td->sys.vm86) {
        latxs_do_translate_ret_far_pe(pir1, size, esp_inc);
    } else {
    /* > real mode or vm86 */
        latxs_do_translate_ret_far_real(pir1, opnd_size, esp_inc);
    }

    return true;
}

/* End of TB in system-mode */
bool latxs_translate_retf(IR1_INST *pir1)
{
    int data_size = latxs_ir1_data_size(pir1);
#ifdef TARGET_X86_64
    if (data_size == 64) {
        return latxs_do_translate_ret_far(pir1, 2, 64);
    } else
#endif
    if (data_size == 16) {
        /* 16-bit opnd size */
        return latxs_do_translate_ret_far(pir1, 0, 16);
    } else {
        /* 32-bit opnd size */
        return latxs_do_translate_ret_far(pir1, 1, 32);
    }
}

bool latxs_translate_cwd(IR1_INST *pir1)
{
    IR2_OPND val = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&val, &ax_ir1_opnd, EXMode_S);
    latxs_append_ir2_opnd2i(LISA_SRAI_W, &val, &val, 15);
    latxs_store_ir2_to_ir1(&val, &dx_ir1_opnd);
    latxs_ra_free_temp(&val);
    return true;
}

bool latxs_translate_cdq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (latxs_ir1_data_size(pir1) == 16) {
        return latxs_translate_cwd(pir1);
    }
#endif

    IR2_OPND eax = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&eax, &eax_ir1_opnd, EXMode_S);
    IR2_OPND edx = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2_(lisa_mov32s, &edx, &eax);
    latxs_append_ir2_opnd2i(LISA_SRAI_W, &edx, &edx, 31);
    latxs_store_ir2_to_ir1(&edx, &edx_ir1_opnd);
    latxs_ra_free_temp(&eax);
    latxs_ra_free_temp(&edx);
    return true;
}

bool latxs_translate_cqo(IR1_INST *pir1)
{
    IR2_OPND rax = latxs_ra_alloc_gpr(eax_index);
    IR2_OPND rdx = latxs_ra_alloc_gpr(edx_index);
    latxs_append_ir2_opnd2i(LISA_SRAI_D, &rdx, &rax, 63);
    return true;
}

bool latxs_translate_cwde(IR1_INST *pir1)
{
    /*
     * In system-mode, since the capstone will always
     * generate CWDE ignoring the opnd size.
     * so we must do it by ourself.
     */
    int data_size = latxs_ir1_data_size(pir1);
    if (data_size == 16) {
        /* AX = signed extension(al) */
        IR2_OPND al_opnd = latxs_ra_alloc_itemp();
        latxs_load_ir1_gpr_to_ir2(&al_opnd, &al_ir1_opnd, EXMode_S);
        latxs_store_ir2_to_ir1(&al_opnd, &ax_ir1_opnd);
    } else if (data_size == 32) {
        /* EAX = signed extension(ax) */
        IR2_OPND eax_opnd = latxs_ra_alloc_gpr(eax_index);
        latxs_append_ir2_opnd2_(lisa_mov16s, &eax_opnd, &eax_opnd);
#ifdef TARGET_X86_64
        latxs_append_ir2_opnd2_(lisa_mov32z, &eax_opnd, &eax_opnd);
#endif
    } else {
        lsassertm(0, "unknown data size of cwde.\n");
    }

    return true;
}

bool latxs_translate_cdqe(IR1_INST *pir1)
{
    IR2_OPND eax_opnd = latxs_ra_alloc_gpr(eax_index);
    latxs_append_ir2_opnd2_(lisa_mov32s, &eax_opnd, &eax_opnd);
    return true;
}

bool latxs_translate_cbw(IR1_INST *pir1)
{
    IR2_OPND val = latxs_ra_alloc_itemp();
    latxs_load_ir1_gpr_to_ir2(&val, &al_ir1_opnd, EXMode_S);
    latxs_store_ir2_to_ir1(&val, &ax_ir1_opnd);
    return true;
}

static void latxs_translate_call_far_imm(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    /* 1. save native context */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* 2. prepare the values for parameters */

    /* 2.1 prepare parameter new_cs and new_eip */
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    /*
     * -------------------------------------------
     *  opndtype |      opnd[0]    |     opnd[1]
     * -------------------------------------------
     *  ptr16:16 |  imm16 selector |  imm16 offset
     *  ptr16:32 |  imm16 selector |  imm32 offset
     * -------------------------------------------
     */
    int data_size = latxs_ir1_data_size(pir1);
    lsassert(data_size == 16 || data_size == 32);
    lsassert(ir1_opnd_size(opnd0) == 16);
    lsassert(ir1_opnd_size(opnd1) == data_size);

    uint32_t selector = ir1_opnd_uimm(opnd0); /* new_cs  : 16-bit    */
    uint32_t offset   = ir1_opnd_uimm(opnd1); /* new_eip : 16/32-bit */

    /* 2.2 prepare parameter next_eip */
    ADDRX next_eip = ir1_addr_next(pir1);

    /*
     * 2.3 prepare shift: data size
     *     16-bit: shift = 0
     *     32-bit: shift = 1
     */
    int shift = (data_size >> 4) - 1;

    /* 3. building the parameters: take care of temp register */
    /* 3.1 arg0 : env */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);
    /* 3.2 arg1 : selector */
    /*     arg2 : offset   */
    latxs_load_imm32_to_ir2(&latxs_arg1_ir2_opnd,
            selector & 0xffff, EXMode_Z);
    if (data_size == 32) {
        latxs_load_imm32_to_ir2(&latxs_arg2_ir2_opnd, offset, EXMode_Z);
    } else {
        latxs_load_imm32_to_ir2(&latxs_arg2_ir2_opnd,
                offset & 0xffff, EXMode_Z);
    }
    /* 3.3 arg3 : shift according to data size */
    latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg3_ir2_opnd,
            &latxs_zero_ir2_opnd, shift);

    /* latxs_arg4_ir2_opnd is itemp in x86_64, please be careful */

    /* 3.4 arg4 : next eip */
    latxs_load_imm32_to_ir2(&latxs_arg4_ir2_opnd,
            (uint32_t)next_eip, EXMode_Z);

    /* 4. call the helper */
    if (td->sys.pe && !td->sys.vm86) {
        latxs_tr_gen_call_to_helper((ADDR)helper_lcall_protected);
    } else {
        latxs_tr_gen_call_to_helper((ADDR)helper_lcall_real);
    }

    /*
     *  far call in protected mode    * far call in real-mode or vm86 mode
     *  > complete cpustate is used   *  > SS and ESP (mapping) is used
     *  > CS and EIP is updated       *  > CS and EIP is updated
     *  > might generate exception    *  > might generate exception
     *
     * target/i386/seg_helper.c       * target/i386/seg_helper.c
     * void helper_lcall_protected(   * void helper_lcall_real(
     *      CPUX86State     *env,     *      CPUX86State     *env,
     *      int             new_cs,   *      int             new_cs,
     *      target_ulong    new_eip,  *      target_ulong    new_eip,
     *      int             shift,    *      int             shift,
     *      target_ulong    next_eip) *      int             next_eip)
     */

    /* 5. restore native context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* 6. disable EIP update in the later jmp */
    td->ignore_eip_update = 1;
}

static void latxs_translate_call_far_mem(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    /* 1. prepare the values for parameters */

    /* 1.1 prepare parameter new_cs and new_eip */
    IR2_OPND selector_opnd = latxs_ra_alloc_itemp();
    IR2_OPND offset_opnd   = latxs_ra_alloc_itemp();
    /*
     * -------------------------------------------
     *  opndtype |      opnd[0]    |     opnd[1]
     * -------------------------------------------
     *    m16:16 |   mem  size = 4 |       --
     *    m16:32 |   mem  size = 6 |       --
     * -------------------------------------------
     */
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd0, -1);

    /* offset at MEM(address) : size = data_size */
    int data_size = latxs_ir1_data_size(pir1);
    lsassert(data_size == 16 || data_size == 32);
    if (data_size == 32) {
        gen_ldst_softmmu_helper(LISA_LD_WU, &offset_opnd, &mem_opnd, 1);
    } else {
        gen_ldst_softmmu_helper(LISA_LD_HU, &offset_opnd, &mem_opnd, 1);
    }

    /* selector at MEM(address + 2 or 4 ) : size = 16-bits */
    if (data_size == 32) {
        IR2_OPND mem_opnd_adjusted
            = latxs_convert_mem_ir2_opnd_plus_4(&mem_opnd);
        gen_ldst_softmmu_helper(LISA_LD_HU,
                &selector_opnd, &mem_opnd_adjusted, 1);
    } else {
        IR2_OPND mem_opnd_adjusted
            = latxs_convert_mem_ir2_opnd_plus_2(&mem_opnd);
        gen_ldst_softmmu_helper(LISA_LD_HU,
                &selector_opnd, &mem_opnd_adjusted, 1);
    }

    /* 1.2 prepare parameter next_eip */
    ADDRX next_eip = ir1_addr_next(pir1);

    /*
     * 1.3 prepare shift: data size
     *     16-bit: shift = 0
     *     32-bit: shift = 1
     */
    int shift = (data_size >> 4) - 1;

    /* 2. save native context */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /*
     *  far call in protected mode    * far call in real-mode or vm86 mode
     *  > complete cpustate is used   *  > SS and ESP (mapping) is used
     *  > CS and EIP is updated       *  > CS and EIP is updated
     *  > might generate exception    *  > might generate exception
     *
     * target/i386/seg_helper.c       * target/i386/seg_helper.c
     * void helper_lcall_protected(   * void helper_lcall_real(
     *      CPUX86State     *env,     *      CPUX86State     *env,
     *      int             new_cs,   *      int             new_cs,
     *      target_ulong    new_eip,  *      target_ulong    new_eip1,
     *      int             shift,    *      int             shift,
     *      target_ulong    next_eip) *      int             next_eip)
     */

    IR2_OPND *arg0 = &latxs_arg0_ir2_opnd;
    IR2_OPND *arg1 = &latxs_arg1_ir2_opnd;
    IR2_OPND *arg2 = &latxs_arg2_ir2_opnd;
    IR2_OPND *arg3 = &latxs_arg3_ir2_opnd;
    IR2_OPND *arg4 = &latxs_arg4_ir2_opnd;

    IR2_OPND *zero = &latxs_zero_ir2_opnd;

    /* 3.1 arg1 : selector */
    /* 3.2 arg2 : offset   */
    IR2_OPND tmp = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_OR, &tmp, &offset_opnd, zero);
    latxs_append_ir2_opnd3(LISA_OR, arg1, &selector_opnd, zero);
    latxs_append_ir2_opnd3(LISA_OR, arg2, &tmp, zero);
    /* 3.3 arg3 : shift according to data size */
    latxs_append_ir2_opnd2i(LISA_ORI, arg3, zero, shift);
    /* 3.4 arg4 : next eip */
    latxs_load_imm32_to_ir2(arg4, next_eip, EXMode_Z);
    /* 3.5 arg0 : env */
    latxs_append_ir2_opnd3(LISA_OR, arg0, &latxs_env_ir2_opnd, zero);

    /* 4. call the helper */
    if (td->sys.pe && !td->sys.vm86) {
        latxs_tr_gen_call_to_helper((ADDR)helper_lcall_protected);
    } else {
        latxs_tr_gen_call_to_helper((ADDR)helper_lcall_real);
    }

    /* 5. restore native context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* 6. disable EIP update in the later jmp */
    td->ignore_eip_update = 1;
}

/* End of TB in system-mode */
bool latxs_translate_lcall(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    if (ir1_opnd_is_imm(opnd0)) {
        latxs_translate_call_far_imm(pir1);
    } else {
        lsassertm_illop(ir1_addr(pir1), ir1_opnd_is_mem(opnd0),
                "not a valid call far: pir1 = %p\n", (void *)pir1);
        latxs_translate_call_far_mem(pir1);
    }

    return true;
}

static IR1_OPND latxs_pa_get_gpr(int id, int size)
{
    lsassert(size == 2 || size == 4);
    switch (id) {
    case 0:
        return size == 2 ? ax_ir1_opnd : eax_ir1_opnd;
    case 1:
        return size == 2 ? cx_ir1_opnd : ecx_ir1_opnd;
    case 2:
        return size == 2 ? dx_ir1_opnd : edx_ir1_opnd;
    case 3:
        return size == 2 ? bx_ir1_opnd : ebx_ir1_opnd;
    case 4:
        return size == 2 ? sp_ir1_opnd : esp_ir1_opnd;
    case 5:
        return size == 2 ? bp_ir1_opnd : ebp_ir1_opnd;
    case 6:
        return size == 2 ? si_ir1_opnd : esi_ir1_opnd;
    case 7:
        return size == 2 ? di_ir1_opnd : edi_ir1_opnd;
    default:
        lsassert(0);
        break;
    }
    /* should never reach here */
    return eax_ir1_opnd;
}

static bool latxs_do_translate_pusha(IR1_INST *pir1, int size)
{
#ifdef TARGET_X86_64
    lsassert(!lsenv->tr_data->sys.code64);
#endif
    lsassert(size == 2 || size == 4);

    int data_size = latxs_ir1_data_size(pir1);
    lsassert(size == (data_size >> 3));

    /*
     * 1. pusha: push all 8 GRP to stack
     *
     * ===== pushal =======|====== pushaw =====
     *                     |
     * ESP      -> ------- | ESP      -> ------
     *             | EAX | |             | AX |
     * ESP - 4  -> ------- | ESP - 2  -> ------
     *             | ECX | |             | CX |
     * ESP - 8  -> ------- | ESP - 4  -> ------
     *             | EDX | |             | DX |
     * ESP - 12 -> ------- | ESP - 6  -> ------
     *             | EBX | |             | BX |
     * ESP - 16 -> ------- | ESP - 8  -> ------ push stack
     *             | ESP | |             | SP | pointer value
     * ESP - 20 -> ------- | ESP - 10 -> ------ before updating
     *             | EBP | |             | BP |
     * ESP - 24 -> ------- | ESP - 12 -> ------
     *             | ESI | |             | SI |
     * ESP - 28 -> ------- | ESP - 14 -> ------
     *             | EDI | |             | DI |
     * ESP - 32 -> ------- | ESP - 16 -> ------
     */

    IR1_OPND mem_ir1_opnd;
    IR2_OPND gpr_value = latxs_ra_alloc_itemp();
    int esp_dec = 0;
    int esp_dec_step = 0 - size;
    int i = 0;

    /* 2.1 build IR2 opnd for MEM(SS:ESP) */
    IR2_OPND mem_opnd;
    int ss_addr_size = latxs_get_sys_stack_addr_size();

    IR2_OPCODE st_ir2_op = size == 2 ? LISA_ST_H : LISA_ST_W ;

    int save_temp = 1;
    for (i = 0; i < 8; ++i) {
        /*
         * 2.2 adjust ESP
         *     i = 0 : esp_inc =  -4 or  -2
         *     i = 1 : esp_inc =  -8 or  -4
         *
         *     i = 7 : esp_inc = -32 or -16
         */
        esp_dec = esp_dec + esp_dec_step;
        /* 2.3 load GPR value from ENV */
        IR1_OPND gpr = latxs_pa_get_gpr(i, size);
        latxs_load_ir1_gpr_to_ir2(&gpr_value, &gpr, EXMode_Z);
        /* 2.4 update mem offset */
        latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
                X86_REG_SS, X86_REG_ESP, esp_dec, 0, 0);
        latxs_convert_mem_opnd(&mem_opnd, &mem_ir1_opnd, ss_addr_size);
        /* 2.5 store into stack */
        gen_ldst_softmmu_helper(st_ir2_op, &gpr_value, &mem_opnd, save_temp);
        latxs_ra_free_temp(&mem_opnd);
    }
    latxs_ra_free_temp(&gpr_value);

    /* 3. update ESP */
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
    if (lsenv->tr_data->sys.ss32) {
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &esp_opnd, &esp_opnd, -8 * size);
#ifdef TARGET_X86_64
        latxs_append_ir2_opnd2_(lisa_mov32z, &esp_opnd, &esp_opnd);
#else
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
        }
#endif
    } else {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd, -8 * size);
        latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
        latxs_ra_free_temp(&tmp);
    }

    return true;
}

static bool latxs_do_translate_popa(IR1_INST *pir1, int size)
{
#ifdef TARGET_X86_64
    lsassert(!lsenv->tr_data->sys.code64);
#endif
    lsassert(size == 2 || size == 4);

    int data_size = latxs_ir1_data_size(pir1);
    lsassert(size == (data_size >> 3));

    /*
     * 1. popa: pop all 8 GRP to stack
     *
     * ===== popal ========|====== popaw ======
     *                     |
     * ESP + 32 -> ------- | ESP + 16 -> ------
     *             | EAX | |             | AX |
     * ESP + 28 -> ------- | ESP + 14 -> ------
     *             | ECX | |             | CX |
     * ESP + 24 -> ------- | ESP + 12 -> ------
     *             | EDX | |             | DX |
     * ESP + 20 -> ------- | ESP + 10 -> ------
     *             | EBX | |             | BX |
     * ESP + 16 -> ------- | ESP + 8  -> ------ NOT pop
     *             | ESP | |             | SP | stack pointer
     * ESP + 12 -> ------- | ESP + 6  -> ------
     *             | EBP | |             | BP |
     * ESP + 8  -> ------- | ESP + 4  -> ------
     *             | ESI | |             | SI |
     * ESP + 4  -> ------- | ESP + 2  -> ------
     *             | EDI | |             | DI |
     * ESP      -> ------- | ESP      -> ------
     */

    IR1_OPND mem_ir1_opnd;
    IR2_OPND gpr_value = latxs_ra_alloc_itemp();
    int esp_inc = 8 * size;
    int esp_inc_step = 0 - size;
    int i = 0;

    /* 2.1 build IR2 opnd for MEM(SS:ESP) */
    IR2_OPND mem_opnd;
    int ss_addr_size = latxs_get_sys_stack_addr_size();

    lsassert(size == 2 || size == 4);
    IR2_OPCODE ld_ir2_op = size == 2 ? LISA_LD_HU : LISA_LD_WU;

    int save_temp = 1;
    for (i = 0; i < 8; ++i) {
        /*
         * 2.2 adjust ESP
         *     i = 0 : esp_inc = 28 or 14
         *     i = 1 : esp_inc = 24 or 12
         *
         *     i = 7 : esp_inc = 0
         */
        esp_inc = esp_inc + esp_inc_step;
        /* 2.3 ignore ESP pop */
        if (i == esp_index) {
            continue;
        }
        /* 2.4 update mem offset */
        latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
                X86_REG_SS, X86_REG_ESP, esp_inc, 0, 0);
        latxs_convert_mem_opnd(&mem_opnd, &mem_ir1_opnd, ss_addr_size);
        /* 2.5 load from stack */
        gen_ldst_softmmu_helper(ld_ir2_op, &gpr_value, &mem_opnd, save_temp);
        latxs_ra_free_temp(&mem_opnd);
        /* 2.6 write poped GPR value to ENV */
        IR1_OPND gpr = latxs_pa_get_gpr(i, size);
        latxs_store_ir2_to_ir1_gpr(&gpr_value, &gpr);
    }
    latxs_ra_free_temp(&gpr_value);

    /* 4. update ESP */
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
    if (lsenv->tr_data->sys.ss32) {
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &esp_opnd, &esp_opnd, 8 * size);
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
        }
    } else {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd, 8 * size);
        latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
        latxs_ra_free_temp(&tmp);
    }

    return true;
}

static bool latxs_translate_pushaw(IR1_INST *pir1)
{
    return latxs_do_translate_pusha(pir1, 2); /* 16-bit */
}
static bool latxs_translate_pushal(IR1_INST *pir1)
{
    return latxs_do_translate_pusha(pir1, 4); /* 32-bit */
}
static bool latxs_translate_popaw(IR1_INST *pir1)
{
    return latxs_do_translate_popa(pir1, 2);  /* 16-bit */
}
static bool latxs_translate_popal(IR1_INST *pir1)
{
    return latxs_do_translate_popa(pir1, 4);  /* 32-bit */
}

bool latxs_translate_pusha(IR1_INST *pir1)
{
    IR1_OPCODE opc = ir1_opcode(pir1);
    if (opc == X86_INS_PUSHAL) {
        return latxs_translate_pushal(pir1);
    } else if (opc == X86_INS_PUSHAW) {
        return latxs_translate_pushaw(pir1);
    } else {
        return false;
    }
}

bool latxs_translate_popa(IR1_INST *pir1)
{
    IR1_OPCODE opc = ir1_opcode(pir1);
    if (opc == X86_INS_POPAL) {
        return latxs_translate_popal(pir1);
    } else if (opc == X86_INS_POPAW) {
        return latxs_translate_popaw(pir1);
    } else {
        return false;
    }
}

static bool latxs_do_translate_lxx(
        IR1_INST *pir1, IR1_OPND *seg_ir1_opnd)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* dest: GPR */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* src : MEM */

    /*
     *      data size | 16 | 32 | mem opnd
     * -------------------------------------
     *    offset size | 16 | 32 |  m16:16
     *  selector size | 16 | 16 |  m16:32
     */

    int opnd_size = ir1_opnd_size(opnd0);
    int data_size = latxs_ir1_data_size(pir1);
    lsassert(data_size == 16 || data_size == 32);
    lsassert(opnd_size == data_size);
    (void)opnd_size; /* to avoid compile warning */

    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd1, -1);
    int save_temp = 1;

    TRANSLATION_DATA *td = lsenv->tr_data;

    /* 1. load offset at MEM */
    IR2_OPND offset_opnd = latxs_ra_alloc_itemp();
    IR2_OPCODE opc = data_size == 16 ? LISA_LD_HU : LISA_LD_WU;
    gen_ldst_softmmu_helper(opc, &offset_opnd, &mem_opnd, save_temp);
    /* 1.1 save offset to not be ruined by store_ir2_to_ir1_seg */
    if (td->sys.pe && !td->sys.vm86) {
        latxs_append_ir2_opnd2i(LISA_ST_D, &offset_opnd,
                &latxs_env_ir2_opnd,
                lsenv_offset_of_mips_regs(lsenv, 0));
    }

    /* 2. load selector at MEM + 2 or 4 */
    IR2_OPND mem;
    switch (data_size >> 3) {
    case 2:
        mem = latxs_convert_mem_ir2_opnd_plus_2(&mem_opnd);
        break;
    case 4:
        mem = latxs_convert_mem_ir2_opnd_plus_4(&mem_opnd);
        break;
    default:
        break;
    }
    IR2_OPND selector_opnd = latxs_ra_alloc_itemp();
    gen_ldst_softmmu_helper(LISA_LD_HU, &selector_opnd, &mem, save_temp);

    /* 3. store selector to segment register */
    latxs_store_ir2_to_ir1_seg(&selector_opnd, seg_ir1_opnd);
    latxs_ra_free_temp(&selector_opnd);

    /* 4.0 load offset if it is saved */
    if (td->sys.pe && !td->sys.vm86) {
        latxs_append_ir2_opnd2i(LISA_LD_D, &offset_opnd,
                &latxs_env_ir2_opnd,
                lsenv_offset_of_mips_regs(lsenv, 0));
    }

    /* 4. move offset into GPR.                     */
    /*    Do this at the end for precise exception. */
    latxs_store_ir2_to_ir1_gpr(&offset_opnd, opnd0);
    latxs_ra_free_temp(&offset_opnd);

    /* 5. no inhibit irq from store_ir2_to_ir1_seg */
    td->inhibit_irq = 0;

    return true;
}

/* End of TB in system-mode : PE & !vm86 & code32 */
bool latxs_translate_lds(IR1_INST *pir1)
{
    IR1_OPND seg_ir1_opnd;
    ir1_opnd_build_reg(&seg_ir1_opnd, 16, X86_REG_DS);
    return latxs_do_translate_lxx(pir1, &seg_ir1_opnd);
}
/* End of TB in system-mode : PE & !vm86 & code32*/
bool latxs_translate_les(IR1_INST *pir1)
{
    IR1_OPND seg_ir1_opnd;
    ir1_opnd_build_reg(&seg_ir1_opnd, 16, X86_REG_ES);
    return latxs_do_translate_lxx(pir1, &seg_ir1_opnd);
}
bool latxs_translate_lfs(IR1_INST *pir1)
{
    IR1_OPND seg_ir1_opnd;
    ir1_opnd_build_reg(&seg_ir1_opnd, 16, X86_REG_FS);
    return latxs_do_translate_lxx(pir1, &seg_ir1_opnd);
}
bool latxs_translate_lgs(IR1_INST *pir1)
{
    IR1_OPND seg_ir1_opnd;
    ir1_opnd_build_reg(&seg_ir1_opnd, 16, X86_REG_GS);
    return latxs_do_translate_lxx(pir1, &seg_ir1_opnd);
}
/* End of TB in system-mode */
bool latxs_translate_lss(IR1_INST *pir1)
{
    IR1_OPND seg_ir1_opnd;
    ir1_opnd_build_reg(&seg_ir1_opnd, 16, X86_REG_SS);
    return latxs_do_translate_lxx(pir1, &seg_ir1_opnd);
}

bool latxs_translate_enter(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* esp_addend */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* level */
    int esp_addend  = ir1_opnd_uimm(opnd0);
    int level       = ir1_opnd_uimm(opnd1);

    TRANSLATION_DATA *td = lsenv->tr_data;

    int data_size = latxs_ir1_data_size(pir1);

#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        data_size = (data_size == 16) ? 16 : 64;
    }
#endif

    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
    IR2_OPND ebp_opnd = latxs_ra_alloc_gpr(ebp_index);

    IR2_OPND frametemp = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ADDI_D, &frametemp,
            &esp_opnd, 0 - (data_size >> 3));

    /* 1. push EBP into stack MEM(SS:ESP-d)*/
    IR1_OPND mem_ir1_opnd;
    /* 1.1 build MEM(SS:ESP-d) */
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
            X86_REG_SS, X86_REG_ESP, 0 - (data_size >> 3), 0, 0);
    /* 1.2 store */
    int ss_addr_size = latxs_get_sys_stack_addr_size();
    latxs_store_ir2_to_ir1_mem(&ebp_opnd,
            &mem_ir1_opnd, ss_addr_size);

    level &= 31;
    if (level) {
        int i;
        /* copy level-1 pointers from the previous frame */
        for (i = 1; i < level; ++i) {
            latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
                    X86_REG_SS, X86_REG_EBP, 0 - (data_size >> 3) * i, 0, 0);
            IR2_OPND value = latxs_ra_alloc_itemp();
            latxs_load_ir1_mem_to_ir2(&value,
                    &mem_ir1_opnd, EXMode_Z, ss_addr_size);

            latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
                    X86_REG_SS, X86_REG_ESP,
                    0 - (data_size >> 3) * (i + 1), 0, 0);
            latxs_store_ir2_to_ir1_mem(&value,
                    &mem_ir1_opnd, ss_addr_size);
            latxs_ra_free_temp(&value);
        }
        /* push current FrameTemp as the last level */
        latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
                X86_REG_SS, X86_REG_ESP,
                0 - (data_size >> 3) * (level + 1), 0, 0);
        latxs_store_ir2_to_ir1_mem(&frametemp,
                &mem_ir1_opnd, ss_addr_size);
    }

#ifdef TARGET_X86_64
    if (td->sys.code64) {
        latxs_store_ir2_to_ir1_gpr(&frametemp, &rbp_ir1_opnd);
    } else
#endif
    /* copy FrameTemp vlaue to EBP */
    if (td->sys.ss32) {
        latxs_store_ir2_to_ir1_gpr(&frametemp, &ebp_ir1_opnd);
    } else {
        latxs_store_ir2_to_ir1_gpr(&frametemp, &bp_ir1_opnd);
    }

    /* compute final value of ESP */
    IR2_OPND final_esp = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ADDI_D, &final_esp,
            &frametemp, -(esp_addend + (data_size >> 3) * level));

    latxs_ra_free_temp(&frametemp);
#ifdef TARGET_X86_64
    if (td->sys.code64) {
        latxs_store_ir2_to_ir1_gpr(&final_esp, &rsp_ir1_opnd);
    } else
#endif
    if (td->sys.ss32) {
        latxs_store_ir2_to_ir1_gpr(&final_esp, &esp_ir1_opnd);
    } else {
        latxs_store_ir2_to_ir1_gpr(&final_esp, &sp_ir1_opnd);
    }
    latxs_ra_free_temp(&final_esp);

    return true;
}

bool latxs_translate_leave(IR1_INST *pir1)
{
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
    IR2_OPND ebp_opnd = latxs_ra_alloc_gpr(ebp_index);

    int ss_addr_size = latxs_get_sys_stack_addr_size();
    int data_size = latxs_ir1_data_size(pir1);

#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        data_size = (data_size == 16) ? 16 : 64;
    }
#endif

    /* 1. load value from MEM(SS:EBP) */
    IR1_OPND mem_ir1_opnd;
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
            X86_REG_SS, X86_REG_EBP, 0, 0, 0);

    IR2_OPND new_ebp = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&new_ebp, &mem_ir1_opnd,
            EXMode_Z, ss_addr_size);

    IR2_OPND new_esp = latxs_ra_alloc_itemp();

#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &new_esp, &ebp_opnd,
                                data_size >> 3);
    } else {
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &new_esp, &ebp_opnd,
                                data_size >> 3);
    }
#else
    latxs_append_ir2_opnd2i(LISA_ADDI_W, &new_esp, &ebp_opnd, data_size >> 3);
#endif

    /* 2. update EBP according to data_size */
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        latxs_append_ir2_opnd2_(lisa_mov, &ebp_opnd, &new_ebp);
        if (option_by_hand) {
            lsassert(0);
        }
    } else
#endif
    if (data_size == 32) {
        latxs_append_ir2_opnd2_(lisa_mov, &ebp_opnd, &new_ebp);
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&ebp_opnd, EXMode_Z, 32);
        }
    } else if (data_size == 16) {
        latxs_store_ir2_to_ir1_gpr(&new_ebp, &bp_ir1_opnd);
    }

    /* 3. update ESP according to ss_addr_size */
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        latxs_append_ir2_opnd2_(lisa_mov, &esp_opnd, &new_esp);
        if (option_by_hand) {
            lsassert(0);
        }
    } else
#endif
    if (ss_addr_size == 4) {
        latxs_append_ir2_opnd2_(lisa_mov, &esp_opnd, &new_esp);
#ifdef TARGET_X86_64
        latxs_append_ir2_opnd2_(lisa_mov32z, &esp_opnd, &esp_opnd);
#else
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
        }
#endif
    } else {
        latxs_store_ir2_to_ir1_gpr(&new_esp, &sp_ir1_opnd);
    }

    return true;
}

bool latxs_translate_rdtsc(IR1_INST *pir1)
{
    /* TRANSLATION_DATA *td = lsenv->tr_data; */

    /* 0. save next instruciton's EIP to env */
    latxs_tr_gen_save_curr_eip();

    /* if (td->sys.cflags & CF_USE_ICOUNT) { */
        /* TODO latxs_tr_gen_io_start(); */
    /* } */

    /*
     * target/i386/misc_helper.c
     * void helper_rdtsc(CPUX86State *env)
     */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_rdtsc, cfg);

    /* if (td->sys.cflags & CF_USE_ICOUNT) { */
        /* TODO latxs_tr_gen_io_end(); */
    /* } */

    return true;
}

bool latxs_translate_rdtscp(IR1_INST *pir1)
{
    if (latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    /* TRANSLATION_DATA *td = lsenv->tr_data; */

    /* 0. save next instruciton's EIP to env */
    latxs_tr_gen_save_curr_eip();

    /* if (td->sys.cflags & CF_USE_ICOUNT) { */
        /* TODO latxs_tr_gen_io_start(); */
    /* } */

    /*
     * target/i386/misc_helper.c
     * void helper_rdtscp(CPUX86State *env)
     */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_rdtscp, cfg);

    /* if (td->sys.cflags & CF_USE_ICOUNT) { */
        /* TODO latxs_tr_gen_io_end(); */
    /* } */

    return true;
}

bool latxs_translate_rdpmc(IR1_INST *pir1)
{
    /* 0. save next instruciton's EIP to env */
    latxs_tr_gen_save_curr_eip();

    /*
     * target/i386/misc_helper.c
     * void helper_rdpmc(CPUX86State *env)
     * >> exception might be generated
     */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_rdpmc, cfg);

    return true;
}

bool latxs_translate_hlt(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_HLT(pir1);

    /* 0. save next instruciton's EIP to env */
    latxs_tr_gen_save_curr_eip();

    /*
     * target/i386/misc_helper.c
     * void helper_hlt(
     *      CPUX86State  *env,
     *      int           next_eip_addend)
     */
    helper_cfg_t cfg = default_helper_cfg;

    int next_eip_addend = latxs_ir1_inst_size(pir1);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_hlt,
            next_eip_addend, cfg);

    /* 2. should nevet reach here */
    latxs_tr_gen_infinite_loop();

    return true;
}

bool latxs_translate_wrmsr(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_WRMSR(pir1);

    /*
     * target/i386/misc_helper.c
     * void helper_wrmsr(CPUX86State *env)
     * >> EAX, ECX, EDX are used
     */
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_wrmsr,
            default_helper_cfg);

    return true;
}

bool latxs_translate_rdmsr(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_RDMSR(pir1);

    /*
     * target/i386/misc_helper.c
     * void helper_rdmsr(CPUX86State *env)
     * >> EAX, ECX, EDX are used
     */
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_rdmsr,
            default_helper_cfg);

    return true;
}

/* End of TB in system-mode */
bool latxs_translate_invlpg(IR1_INST *pir1)
{
    if (latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    lsassert(ir1_opnd_is_mem(opnd0));

    /* 0. save current instruciton's EIP to env */
    latxs_tr_gen_save_curr_eip();

    /* 1. get mem address */
    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd0, -1);

    int mem_no_offset_new_tmp = 0;
    IR2_OPND mem_no_offset = latxs_convert_mem_ir2_opnd_no_offset(
            &mem_opnd, &mem_no_offset_new_tmp);
    if (mem_no_offset_new_tmp) {
        latxs_ra_free_temp(&mem_opnd);
    }
    IR2_OPND addr_opnd = latxs_ir2_opnd_mem_get_base(&mem_no_offset);

    /*
     * 2. helper_invlpg
     * target/i386/misc_helper.c
     * void helper_invlpg(
     *      CPUX86State *env,
     *      target_ulong addr)
     */

    /* 2.0 save context */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper_prologue_cfg(cfg);

    /* 2.1 arg1: address */
    /* latxs_append_ir2_opnd3(LISA_OR, &latxs_arg1_ir2_opnd, */
            /* &addr_opnd, &latxs_zero_ir2_opnd); */
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &addr_opnd);
    } else {
        latxs_append_ir2_opnd2_(lisa_mov32z, &latxs_arg1_ir2_opnd, &addr_opnd);
    }
#else
    latxs_append_ir2_opnd2_(lisa_mov32s, &latxs_arg1_ir2_opnd, &addr_opnd);
#endif

    latxs_ra_free_temp(&addr_opnd);
    /* 2.2 arg0: env */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);

    /* 2.3 call helper_invlpg */
    latxs_tr_gen_call_to_helper((ADDR)helper_invlpg);

    /* 2.4 restore context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(cfg);

    return true;
}

bool latxs_translate_invlpga(IR1_INST *pir1)
{
    if (latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    int addr_size = latxs_ir1_addr_size(pir1);
    lsassert(addr_size == 2 || addr_size == 4);

    /* 0. save current instruciton's EIP to env */
    latxs_tr_gen_save_curr_eip();

    /*
     * 1. helper_invlpga
     * target/i386/svm_helper.c
     * void helper_invlpga(
     *      CPUX86State *env,
     *      int         aflag)
     */
    helper_cfg_t cfg = default_helper_cfg;

    /* 1.1 aflag */
    int aflag = addr_size == 2 ? 0 : 1;

    /* 1.2 call helper_invlpga */
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_invlpga, aflag, cfg);

    return true;
}

bool latxs_translate_lfence(IR1_INST *pir1)
{
    if (latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    /* TRANSLATION_DATA *td = lsenv->tr_data; */
    /* if (td->sys.cflags & CF_PARALLEL) { */
        /* TODO: MTTCG */
    /* } */
    return true;
}
bool latxs_translate_mfence(IR1_INST *pir1)
{
    if (latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    /* TRANSLATION_DATA *td = lsenv->tr_data; */
    /* if (td->sys.cflags & CF_PARALLEL) { */
        /* TODO: MTTCG */
    /* } */

    return true;
}
bool latxs_translate_sfence(IR1_INST *pir1)
{
    if (latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    /* TRANSLATION_DATA *td = lsenv->tr_data; */
    /* if (td->sys.cflags & CF_PARALLEL) { */
        /* TODO: MTTCG */
    /* } */

    return true;
}

/* End of TB in system-mode */
bool latxs_translate_sysenter(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_SYSENTER(pir1);

    /*
     * target/i386/seg_helper.c
     * void helper_sysenter(CPUX86State *env)
     * >> EIP is updated
     */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_sysenter, cfg);

    /* 2. disable EIP update in the added jmp */
    td->ignore_eip_update = 1;

    return true;
}

/* End of TB in system-mode */
bool latxs_translate_sysexit(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_SYSEXIT(pir1);

    int data_size = latxs_ir1_data_size(pir1);
    lsassert(data_size == 16 || data_size == 32);

    /*
     * target/i386/seg_helper.c
     * void helper_sysexit(
     *      CPUX86State *env,
     *      int dflag)
     */
    helper_cfg_t cfg = default_helper_cfg;

    /* 1.1 dflag = 0 : 16-bit */
    /*     dflag = 1 : 32-bit */
    int dflag = (data_size >> 4) - 1;
    lsassert(!(dflag >> 1));

    /* 1.2 call helper_sysexit*/
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_sysexit, dflag, cfg);

    /* 2. disable EIP update in the added jmp */
    td->ignore_eip_update = 1;

    return true;
}

bool latxs_translate_syscall(IR1_INST *pir1)
{
#ifdef TARGET_X86_64
    IR2_OPND cur_eip = latxs_ra_alloc_itemp();
    latxs_load_addrx_to_ir2(&cur_eip, ir1_addr(pir1));
    latxs_append_ir2_opnd2i(LISA_ST_D, &cur_eip, &latxs_env_ir2_opnd,
                            lsenv_offset_of_eip(lsenv));
    latxs_tr_gen_call_to_helper2_cfg(
        (ADDR)helper_syscall, latxs_ir1_inst_size(pir1), default_helper_cfg);
    lsenv->tr_data->ignore_eip_update = 1;
    return true;
#else
    return false;
#endif
}
bool latxs_translate_sysret(IR1_INST *pir1)
{
#ifdef TARGET_X86_64
    if (!lsenv->tr_data->sys.pe) {
        /* TODO: should raise EXCP0D_GPF */
        lsassert(0);
    }
    int data_size = latxs_ir1_data_size(pir1);
    latxs_tr_gen_call_to_helper2_cfg((ADDR)helper_sysret, data_size / 32,
                                     default_helper_cfg);
    lsenv->tr_data->ignore_eip_update = 1;
    return true;
#else
    return false;
#endif
}

bool latxs_translate_lmsw(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_LMSW(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND msw = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&msw, opnd0, EXMode_N);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &msw);

    latxs_tr_gen_call_to_helper((ADDR)helper_lmsw);

    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    return true;
}

bool latxs_translate_verr(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_VERR(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src, opnd0, EXMode_N);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &src);

    latxs_tr_gen_call_to_helper((ADDR)helper_verr);

    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    return true;
}

bool latxs_translate_verw(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_VERW(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src, opnd0, EXMode_N);

    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                            &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &src);

    latxs_tr_gen_call_to_helper((ADDR)helper_verw);

    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    return true;
}

bool latxs_translate_prefetchnta(IR1_INST *pir1) { return true; }
bool latxs_translate_prefetcht0(IR1_INST *pir1) { return true; }
bool latxs_translate_prefetcht1(IR1_INST *pir1) { return true; }
bool latxs_translate_prefetcht2(IR1_INST *pir1) { return true; }
bool latxs_translate_prefetchw(IR1_INST *pir1) { return true; }
bool latxs_translate_prefetch(IR1_INST *pir1) { return true; }

bool latxs_translate_tzcnt(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (!latxs_ir1_has_prefix_repe(pir1) ||
        td->sys.cpuid_ext3_features & CPUID_EXT3_ABM) {
        pir1->info->id = X86_INS_BSF;
        return latxs_translate_bsf(pir1);
    }

    lsassert(0);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* r16,   r32   */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 2); /* r/m16, r/m32 */

    int opnd_size = ir1_opnd_size(opnd0);
    lsassertm_illop(ir1_addr(pir1), opnd_size == 16 || opnd_size == 32,
            "lzcnt with unsupported opnd size %d.\n", opnd_size);

    IR2_OPND *zero = &latxs_zero_ir2_opnd;

    /* 1. load source data */
    IR2_OPND value = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&value, opnd1, EXMode_S);

    /* 2. count tailing zero */
    IR2_OPND tz_num = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2_(lisa_mov, &tz_num, zero);
    /* 2.1 load loop cnt */
    IR2_OPND cnt_opnd = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ORI, &cnt_opnd, zero, opnd_size);
    /* 2.2 loop start */
    IR2_OPND loop_label = latxs_ir2_opnd_new_label();
    IR2_OPND loop_exit  = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd1(LISA_LABEL, &loop_label);
    latxs_append_ir2_opnd3(LISA_BEQ, &cnt_opnd, zero, &loop_exit);
    /*
     * 2.3 compare the lease signifit bit and
     *     shift the source and
     *     minus the loop cnt
     */
    IR2_OPND bit = latxs_ra_alloc_itemp();
    /* bit   = value &  1 */
    latxs_append_ir2_opnd2i(LISA_ANDI, &bit, &value, 0x1);
    /* value = value >> 1 */
    latxs_append_ir2_opnd2i(LISA_SRAI_W, &value, &value, 0x1);
    /* cnt   = cnt   -  1 */
    latxs_append_ir2_opnd2i(LISA_ADDI_W, &cnt_opnd, &cnt_opnd, -1);
    latxs_append_ir2_opnd3(LISA_BNE, &bit, zero, &loop_label);

    latxs_ra_free_temp(&cnt_opnd);
    latxs_ra_free_temp(&bit);
    latxs_ra_free_temp(&value);
    /* 2.4 not branch: tz_num += 1 */
    latxs_append_ir2_opnd2i(LISA_ADDI_D, &tz_num, &tz_num, 0x1);
    /* 2.5 next loop */
    latxs_append_ir2_opnd1(LISA_B, &loop_label);
    /* 2.6 loop exit */
    latxs_append_ir2_opnd1(LISA_LABEL, &loop_exit);

    /* 2.7 store into dest */
    latxs_store_ir2_to_ir1_gpr(&tz_num, opnd0);

    IR2_OPND mask = latxs_ra_alloc_itemp();

    /* 3. if tz_num == opnd_size : CF = 1 */
    /*    else                   : CF = 0 */
    /* 3.1 prepare label and CF mask */
    latxs_append_ir2_opnd2i(LISA_ORI, &mask, zero, CF_BIT);
    IR2_OPND label_cf = latxs_ir2_opnd_new_label();
    /* 3.2 set CF = 1 */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &mask, 0x1);
    /* 3.3 branch if opnd_size == tz_num */
    IR2_OPND size_opnd = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ORI, &size_opnd, zero, opnd_size);
    latxs_append_ir2_opnd3(LISA_BEQ, &tz_num, &size_opnd, &label_cf);
    latxs_ra_free_temp(&size_opnd);
    /* 3.4 not branch: set CF = 0 */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &zero_ir2_opnd, 0x1);
    /* 3.5 branch : CF = 1 */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_cf);

    /* 4. if tz_num == 0 : ZF = 1 */
    /*    else           : ZF = 0 */
    /* 4.1 prepare label and ZF mask */
    latxs_append_ir2_opnd2i(LISA_ORI, &mask, zero, ZF_BIT);
    IR2_OPND label_zf = latxs_ir2_opnd_new_label();
    /* 4.2 set ZF = 1 */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &mask, 0x8);
    /* 4.3 branch if opnd_size == 0 */
    latxs_append_ir2_opnd3(LISA_BEQ, &tz_num, zero, &label_zf);
    latxs_ra_free_temp(&tz_num);
    /* 4.4 not branch: set ZF = 0 */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, zero, 0x8);
    latxs_ra_free_temp(&mask);
    /* 4.5 branch : CF = 1 */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_zf);

    return true;
}

bool latxs_translate_invalid(IR1_INST *pir1)
{
    switch (pir1->flags) {
    case LATXS_IR1_FLAGS_ILLOP:
        latxs_tr_gen_excp_illegal_op_addr(
                ir1_addr(pir1) - lsenv->tr_data->sys.cs_base, 1);
        break;
    case LATXS_IR1_FLAGS_GENNOP:
        /* Just do nothing */
        break;
    default:
        lsassertm(0, "unsupport invalid ir1 falgs %d\n", pir1->flags);
        break;
    }
    return true;
};

bool latxs_translate_ud0(IR1_INST *pir1)
{
    latxs_tr_gen_excp_illegal_op_addr(
            ir1_addr(pir1) - lsenv->tr_data->sys.cs_base, /* EIP */
            1 /* translation end with exception */
    );
    return true;
}

bool latxs_translate_ud2(IR1_INST *pir1)
{
    latxs_tr_gen_excp_illegal_op_addr(
            ir1_addr(pir1) - lsenv->tr_data->sys.cs_base, /* EIP */
            1 /* translation end with exception */
    );
    return true;
}

bool latxs_translate_swapgs(IR1_INST *pir1)
{
#ifdef TARGET_X86_64
    if (!lsenv->tr_data->sys.code64) {
        /* TODO: should gen illegal op */
        lsassert(0);
    } else if (lsenv->tr_data->sys.cpl != 0) {
        /* TODO: should gen EXCP0D_GPF */
        lsassert(0);
    } else {
        IR2_OPND gs_base = latxs_ra_alloc_itemp();
        IR2_OPND kernel_gs_base = latxs_ra_alloc_itemp();

        latxs_append_ir2_opnd2i(LISA_LD_D, &gs_base, &latxs_env_ir2_opnd,
                                offsetof(CPUX86State, segs[gs_index].base));
        latxs_append_ir2_opnd2i(LISA_LD_D, &kernel_gs_base,
                                &latxs_env_ir2_opnd,
                                offsetof(CPUX86State, kernelgsbase));
        latxs_append_ir2_opnd2i(LISA_ST_D, &kernel_gs_base,
                                &latxs_env_ir2_opnd,
                                offsetof(CPUX86State, segs[gs_index].base));
        latxs_append_ir2_opnd2i(LISA_ST_D, &gs_base, &latxs_env_ir2_opnd,
                                offsetof(CPUX86State, kernelgsbase));
    }
#else
        /* TODO: should gen illegal op */
        lsassert(0);
#endif
    return true;
}
