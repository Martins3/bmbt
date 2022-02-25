#include "lsenv.h"
#include "etb.h"
#include "reg-alloc.h"
#include "flag-pattern.h"
#include "shadow-stack.h"
#include "flag-lbt.h"
#include <string.h>
#include "latx-options.h"
#include "fpu/softfloat.h"
#include "profile.h"
#include "ibtc.h"
#include "translate.h"

extern void *helper_tb_lookup_ptr(CPUArchState *);
static int ss_generate_match_fail_native_code(void* code_buf);

/*To reuse qemu's tb chaining code, we take the same way of epilogue treating,
 *context_switch_native_to_bt_ret_0 and context_switch_native_to_bt share code.
 *
 * context_swtich_native_to_bt_ret_0:
 *    mov v0, zer0
 * context_switch_native_to_bt:
 *    other instructions
 *    ...
 * to support chaining, we generate code roughly like below:
 *
 * 1. unconditional jmps and normal branches
 *
 *   <tb_ptr = tb | <succ_id> or exit flag>
 *   j <pc> + 8 #will be rewritten in tb_add_jump to target tb's native code
 *   nop
 *   j context_switch_native_to_bt
 *   mov v0, tb_ptr
 *
 * 2. indirection jmps and ret
 *
 *   <save pc to env>
 *   <save mapped regs>
 *   load t9, &helper_lookup_tb_ptr
 *   jalr t9
 *   nop
 *   <allocate tmp reg>
 *   mov tmp, v0
 *   <recover mapped regs>
 *   jr tmp #tmp == target tb code addr or context_switch_native_bt_ret_0
 *   nop
 *
 *When context_switch_native_to_bt return to qemu code, ret address will be:
 * 1. if chaining is ok: last executed TB | flags
 * 2. zero | flags
 *
 * flags is <succid> for now
 */

ADDR context_switch_bt_to_native;
ADDR context_switch_native_to_bt_ret_0;
ADDR context_switch_native_to_bt;
ADDR ss_match_fail_native;

ADDR native_rotate_fpu_by; /* native_rotate_fpu_by(step, return_address) */
ADDR native_jmp_glue_0;
ADDR native_jmp_glue_1;
ADDR native_jmp_glue_2;

ADDR fpu_enable_top;
ADDR fpu_disable_top;
ADDR fpu_get_top;
ADDR fpu_set_top;
ADDR fpu_inc_top;
ADDR fpu_dec_top;

int GPR_USEDEF_TO_SAVE = 0x7;
int FPR_USEDEF_TO_SAVE = 0xff;
int XMM_LO_USEDEF_TO_SAVE = 0xff;
int XMM_HI_USEDEF_TO_SAVE = 0xff;

struct lat_lock lat_lock[16];

void tr_init(void *tb)
{
    TRANSLATION_DATA *t = lsenv->tr_data;
    int i = 0;

    /* set current tb and ir1 */
    lsassertm(t->curr_tb == NULL,
              "trying to translate (TB*)%p before (TB*)%p finishes.\n", tb,
              t->curr_tb);
    t->curr_tb = tb;
    t->curr_ir1_inst = NULL;

    /* register allocation init */
    ra_init();

    /* reset gpr extension mode */
    for (i = 0; i < 32; ++i) {
        t->ireg_em[i] = SIGN_EXTENSION;
        t->ireg_eb[i] = 32;
    }
    ir2_opnd_set_em(&n1_ir2_opnd, ZERO_EXTENSION, 32);
    ir2_opnd_set_em(&zero_ir2_opnd, ZERO_EXTENSION, 0);
#ifdef N64 /* validate address */
    ir2_opnd_set_em(&sp_ir2_opnd, EM_MIPS_ADDRESS, 32);
    ir2_opnd_set_em(&env_ir2_opnd, EM_MIPS_ADDRESS, 32);

    IR2_OPND ss_opnd = ra_alloc_ss();
    ir2_opnd_set_em(&ss_opnd, EM_MIPS_ADDRESS, 32);
    for (i = 0; i < 7; ++i) {
        IR2_OPND gpr_opnd = ra_alloc_gpr(i);
        ir2_opnd_set_em(&gpr_opnd, ir2_opnd_default_em(&gpr_opnd), 32);
    }
#endif

    /* reset ir2 array */
    if (t->ir2_inst_array == NULL) {
        lsassert(t->ir2_inst_num_max == 0);
        t->ir2_inst_array = (IR2_INST *)mm_calloc(400, sizeof(IR2_INST));
        t->ir2_inst_num_max = 400;
    } else {
        memset((void *)t->ir2_inst_array, 0,
               t->ir2_inst_num_max * sizeof(IR2_INST));
    }
    t->ir2_inst_num_current = 0;
    t->real_ir2_inst_num = 0;

    /* reset ir2 first/last/num */
    t->first_ir2 = NULL;
    t->last_ir2 = NULL;

    /* label number */
    t->label_num = 0;
    t->itemp_num = 0;
    t->ftemp_num = 0;

    /* top in translator */
    if (tb != NULL) {
        if (!option_lsfpu) {
            t->curr_top = etb_get_top_in(tb);
            assert(t->curr_top != -1);
        }
#ifdef CONFIG_LATX_FLAG_PATTERN
        if (option_flag_pattern) {
            fp_init();
            tb_find_flag_pattern(tb);
        }
#endif
    } else
        t->curr_top = 0;
}

void tr_fini(bool check_the_extension)
{
    TRANSLATION_DATA *t = lsenv->tr_data;

    int i = 0;


    /* check gpr extension mode */
    if (check_the_extension) {
        for (i = 0; i < 8; ++i) {
            IR2_OPND gpr_opnd = ra_alloc_gpr(i);
            EXTENSION_MODE default_em = ir2_opnd_default_em(&gpr_opnd);

            if (default_em == SIGN_EXTENSION) {
                lsassertm(ir2_opnd_is_sx(&gpr_opnd, 32), "%s EM wrong \n",
                          ir1_name(i));
            } else if (default_em == ZERO_EXTENSION ||
                       default_em == EM_X86_ADDRESS) {
                lsassertm(ir2_opnd_is_zx(&gpr_opnd, 32), "%s EM wrong \n",
                          ir1_name(i));
            } else {
                lsassert(0);
            }
        }
    }

    /* set current tb and ir1 */
    t->curr_tb = NULL;
    t->curr_ir1_inst = NULL;

    /* reset ir2 array */
    t->ir2_inst_num_current = 0;
    t->real_ir2_inst_num = 0;

    /* reset ir2 first/last/num */
    t->first_ir2 = NULL;
    t->last_ir2 = NULL;

    /* label number */
    t->label_num = 0;
    t->itemp_num = 0;
    t->ftemp_num = 0;

    /* top in translator */
    t->curr_top = 0;
}

void tr_adjust_em(void)
{
    int i = 0;
    for (i = 0; i < 8; ++i) {
        IR2_OPND gpr_opnd = ra_alloc_gpr(i);
        EXTENSION_MODE default_em = ir2_opnd_default_em(&gpr_opnd);

        if (default_em == SIGN_EXTENSION) {
            if (!ir2_opnd_is_sx(&gpr_opnd, 32))
                la_append_ir2_opnd2_em(LISA_MOV32_SX, gpr_opnd, gpr_opnd);
        } else if (default_em == ZERO_EXTENSION) {
            if (!ir2_opnd_is_zx(&gpr_opnd, 32))
                la_append_ir2_opnd2_em(LISA_MOV32_ZX, gpr_opnd, gpr_opnd);
        } else if (default_em == EM_X86_ADDRESS) {
            if (!ir2_opnd_is_zx(&gpr_opnd, 32))
                la_append_ir2_opnd2_em(LISA_MOV32_ZX, gpr_opnd, gpr_opnd);
            ir2_opnd_set_em(&gpr_opnd, EM_X86_ADDRESS, 32);
        } else {
            lsassert(0);
        }
    }
}

#ifdef CONFIG_LATX_FLAG_REDUCTION
void etb_add_succ(void *petb, int depth)
{
    ETB* etb = (ETB*)petb;
    if (depth==0 || (etb->flags & SUCC_IS_SET_MASK))
        return;

    IR1_INST* pir1_last = etb->_ir1_instructions + etb->_ir1_num - 1;
    ADDRX succ[2] = {0,0};
    if (etb->_tb_type == TB_TYPE_BRANCH) {
        succ[0] = ir1_addr_next(pir1_last);
        succ[1] = ir1_target_addr(pir1_last);
    }
    else if (etb->_tb_type == TB_TYPE_JUMP || etb->_tb_type == TB_TYPE_CALL) { 
        succ[0] = ir1_target_addr(pir1_last);
    }

    for (int i=0;i<2;i++) {
        if (succ[i] == 0) 
            break;
        
        ETB* succ_etb = etb_find(succ[i]);
        etb->succ[i] = succ_etb;
      
        IR1_INST *ir1_list = succ_etb->_ir1_instructions;
        int ir1_num = succ_etb->_ir1_num;
        if (ir1_list == NULL) {
            ir1_list = get_ir1_list(succ_etb, succ[i], &ir1_num);
            succ_etb->_ir1_instructions = ir1_list;
            succ_etb->_ir1_num = ir1_num;
            succ_etb->_tb_type = get_etb_type(ir1_list + ir1_num - 1); 
        }
        etb_add_succ(succ_etb, depth-1);
    
    }
    return;
}
#endif

/* func to access QEMU's data */
static inline uint8_t cpu_read_code_via_qemu(void *cpu, ADDRX pc)
{
    return cpu_ldub_code((CPUX86State *)cpu, (target_ulong)pc);
}

/* we creat an array to fill all 255 ir1's info. */
static char insn_info[MAX_IR1_NUM_PER_TB * IR1_INST_SIZE] = {0};

IR1_INST *get_ir1_list(struct TranslationBlock *tb, ADDRX pc)
{
    IR1_INST *ir1_list = (IR1_INST *)mm_calloc(MAX_IR1_NUM_PER_TB, sizeof(IR1_INST));
    IR1_INST *pir1 = NULL;
    void *pir1_base = insn_info;
    ADDRX start_pc = pc;

    int ir1_num = 0;
    do {
        /* read 32 instructioin bytes */
        lsassert(lsenv->cpu_state != NULL);

        /* disasemble this instruction */
        pir1 = &ir1_list[ir1_num];
        /* get next pc */
        pc = ir1_disasm(pir1, (uint8_t *)((uintptr_t)pc), pc, ir1_num, pir1_base);
        ir1_num++;
        lsassert(ir1_num <= 255);

        /* check if TB is too large */
        if (ir1_num == MAX_IR1_NUM_PER_TB && !ir1_is_tb_ending(pir1)) {
            pc = ir1_addr(pir1);
            ir1_make_ins_JMP(pir1, pc, 0);
            break;
        }

    } while (!ir1_is_tb_ending(pir1));
    tb->size = pc - start_pc;

    ir1_list = (IR1_INST *)mm_realloc(ir1_list, ir1_num * sizeof(IR1_INST));
    tb->icount = ir1_num;
    return ir1_list;

}

#ifdef CONFIG_LATX_FLAG_REDUCTION
int8 get_etb_type(IR1_INST *pir1)
{
    if (ir1_is_branch(pir1))
        return (int8)TB_TYPE_BRANCH;
    else if (ir1_is_call(pir1) && !ir1_is_indirect_call(pir1))
        return (int8)TB_TYPE_CALL;
    else if (ir1_is_jump(pir1) && !ir1_is_indirect_jmp(pir1))
        return (int8)TB_TYPE_JUMP;
    else if (ir1_is_return(pir1))
        return (int8)TB_TYPE_RETURN;
    else if (ir1_opcode(pir1) == X86_INS_CALL && ir1_is_indirect_call(pir1))
        return (int8)TB_TYPE_CALLIN;
    else if (ir1_opcode(pir1) == X86_INS_JMP && ir1_is_indirect_jmp(pir1))
        return (int8)TB_TYPE_JUMPIN;
    else 
        return (int8)TB_TYPE_NONE;

}
#endif

void tr_disasm(struct TranslationBlock *ptb)
{
    ADDRX pc = ptb->pc;
    /* get ir1 instructions */
    ptb->_ir1_instructions = get_ir1_list(ptb, pc);
    lsenv->tr_data->curr_ir1_inst = NULL;
#ifdef CONFIG_LATX_FLAG_REDUCTION
        etb->_tb_type = get_etb_type(ir1_list + ir1_num - 1);
#endif
#ifdef CONFIG_LATX_FLAG_REDUCTION
    if (option_flag_reduction) {
        etb_add_succ(etb, 2);
        etb->flags |= SUCC_IS_SET_MASK;
    }
#endif
#ifdef CONFIG_LATX_DEBUG
    counter_ir1_tr += ptb->icount;
#endif
}

static void label_dispose(void)
{
    /* 1. record the positions of label */
    int *label_num_to_ir2_num =
        (int *)alloca(lsenv->tr_data->label_num * 4 + 20);
    memset(label_num_to_ir2_num, -1, lsenv->tr_data->label_num * 4 + 20);
    int ir2_num = 0;
    IR2_INST *ir2_current = lsenv->tr_data->first_ir2;
    while (ir2_current != NULL) {
        if (ir2_current->_opcode == LISA_LABEL) {
            int label_num = ir2_current->_opnd[0]._label_id;
            lsassertm(label_num_to_ir2_num[label_num] == -1,
                      "label %d is in multiple positions\n", label_num);
            label_num_to_ir2_num[label_num] = ir2_num;
        } else if (ir2_current->_opcode == LISA_X86_INST) {
            /* will not be assembled */
        } else {
            ir2_num++;
        }
        ir2_current = ir2_next(ir2_current);
    }

    /* 2. resolve the offset of successor linkage code */
    /* @jmp_target_arg recoed the jmp  inst position */
    /* @jmp_reset_offset record the successor inst of jmp(exclude delay slot).
     */
    /*                   when the tb is removed from buffer. the jmp inst use
     */
    /*                   this position to revert the original "fall through".
     */
    {
        TranslationBlock *tb = lsenv->tr_data->curr_tb;

        /* prologue/epilogue has no tb */
        if (tb) {
            /* ctx->jmp_insn_offset point to tb->jmp_target_arg */
            int label_id_0 = tb->jmp_reset_offset[0];
            if (label_id_0 != TB_JMP_RESET_OFFSET_INVALID) {
                tb->jmp_reset_offset[0] =
                    (label_num_to_ir2_num[label_id_0] << 2) + 8;
                tb->jmp_target_arg[0] = (label_num_to_ir2_num[label_id_0] << 2);
            }

            int label_id_1 = tb->jmp_reset_offset[1];
            if (label_id_1 != TB_JMP_RESET_OFFSET_INVALID) {
                tb->jmp_reset_offset[1] =
                    (label_num_to_ir2_num[label_id_1] << 2) + 8;
                tb->jmp_target_arg[1] = (label_num_to_ir2_num[label_id_1] << 2);
            }
        }
    }

    /* 3. resolve the branch instructions */
    ir2_num = 0;
    ir2_current = lsenv->tr_data->first_ir2;
    while (ir2_current != NULL) {
        IR2_OPCODE opcode = ir2_current->_opcode;
        if (ir2_opcode_is_branch(opcode) || ir2_opcode_is_f_branch(opcode)) {
            IR2_OPND *label_opnd = NULL;
            if (ir2_current->_opcode == LISA_B ||
                ir2_current->_opcode == LISA_BL)
                label_opnd = &ir2_current->_opnd[0];
            else if (ir2_opcode_is_branch_with_3opnds(opcode))
                label_opnd = &ir2_current->_opnd[2];
            else if (ir2_opcode_is_f_branch(opcode))
                label_opnd = &ir2_current->_opnd[1];
            if(label_opnd && ir2_opnd_is_label(label_opnd)) {
                int label_num = label_opnd->_label_id;
                lsassert(label_num > 0 && label_num <= lsenv->tr_data->label_num);
                int target_ir2_num = label_num_to_ir2_num[label_num];
                lsassertm(target_ir2_num != -1, "label %d is not inserted\n",
                          label_num);
                ir2_opnd_convert_label_to_imm(label_opnd,
                                              target_ir2_num - ir2_num);
            }
        }

        if (ir2_current->_opcode != LISA_LABEL &&
            ir2_current->_opcode != LISA_X86_INST) {
            ir2_num++;
        }
        ir2_current = ir2_next(ir2_current);
    }
}

int tr_ir2_assemble(const void *code_start_addr)
{
    if (option_dump) {
        fprintf(stderr, "[LATX] Assemble IR2.\n");
    }

    /* 1. assign temp register to physical register */
    /* now, temp register is physical register */

    /* 2. label dispose */
    label_dispose();

    /* 3. assemble */
    IR2_INST *pir2 = lsenv->tr_data->first_ir2;

    void *code_addr = (void *) code_start_addr;
    int code_nr = 0;

    while (pir2 != NULL) {
        if (ir2_opcode(pir2) != LISA_LABEL &&
            ir2_opcode(pir2) != LISA_X86_INST) {
            uint32 result = ir2_assemble(pir2);

            if (option_dump_host) {
                fprintf(stderr, "IR2[%03d] at %p 0x%08x ",pir2->_id, code_addr,
                        result);
                ir2_dump(pir2);
            }

            *(uint32 *)code_addr = result;
            code_addr = code_addr + 4;
            code_nr += 1;
        }
        pir2 = ir2_next(pir2);
    }

    return code_nr;
}

void tr_skip_eflag_calculation(int usedef_bits)
{
    BITS_SET(lsenv->tr_data->curr_ir1_skipped_eflags, usedef_bits);
}

bool ir1_need_calculate_cf(IR1_INST *ir1)
{
    return ir1_is_cf_def(ir1) &&
           BITS_ARE_CLEAR(lsenv->tr_data->curr_ir1_skipped_eflags,
                          1 << CF_USEDEF_BIT_INDEX);
}

bool ir1_need_calculate_pf(IR1_INST *ir1)
{
    return ir1_is_pf_def(ir1) &&
           BITS_ARE_CLEAR(lsenv->tr_data->curr_ir1_skipped_eflags,
                          1 << PF_USEDEF_BIT_INDEX);
}

bool ir1_need_calculate_af(IR1_INST *ir1)
{
    return ir1_is_af_def(ir1) &&
           BITS_ARE_CLEAR(lsenv->tr_data->curr_ir1_skipped_eflags,
                          1 << AF_USEDEF_BIT_INDEX);
}

bool ir1_need_calculate_zf(IR1_INST *ir1)
{
    return ir1_is_zf_def(ir1) &&
           BITS_ARE_CLEAR(lsenv->tr_data->curr_ir1_skipped_eflags,
                          1 << ZF_USEDEF_BIT_INDEX);
}

bool ir1_need_calculate_sf(IR1_INST *ir1)
{
    return ir1_is_sf_def(ir1) &&
           BITS_ARE_CLEAR(lsenv->tr_data->curr_ir1_skipped_eflags,
                          1 << SF_USEDEF_BIT_INDEX);
}

bool ir1_need_calculate_of(IR1_INST *ir1)
{
    return ir1_is_of_def(ir1) &&
           BITS_ARE_CLEAR(lsenv->tr_data->curr_ir1_skipped_eflags,
                          1 << OF_USEDEF_BIT_INDEX);
}

bool ir1_need_calculate_any_flag(IR1_INST *ir1)
{
    return (ir1_get_eflag_def(ir1) &
            ~(lsenv->tr_data->curr_ir1_skipped_eflags)) != 0;
}

int idx_start = __LINE__ + 5;
static bool (*translate_functions[])(IR1_INST *) = {
    //implemented , but not in X86_INS_...        
    //lods,pusha,finit,popa,fstenv,xlat,fclex,movs,jmp_far,fsave,call_far,fstcw,

    translate_invalid,   /* X86_INS_INVALID = 0, */
    translate_aaa,       //X86_INS_AAA,
    translate_aad,       //X86_INS_AAD,
    translate_aam,       //X86_INS_AAM,
    translate_aas,       //X86_INS_AAS,
    translate_fabs,      //X86_INS_FABS,
    translate_adc,       //X86_INS_ADC,
    NULL,                //X86_INS_ADCX,
    translate_add,       //X86_INS_ADD,
    translate_addpd,     //X86_INS_ADDPD,
    translate_addps,     //X86_INS_ADDPS,
    translate_addsd,     //X86_INS_ADDSD,
    translate_addss,     //X86_INS_ADDSS,
    translate_addsubpd,  //X86_INS_ADDSUBPD,
    translate_addsubps,  //X86_INS_ADDSUBPS,
    translate_fadd,      //X86_INS_FADD,
    translate_fiadd,     //X86_INS_FIADD,
    translate_faddp,     //X86_INS_FADDP,
    NULL,                //X86_INS_ADOX,
    translate_aesdeclast,//X86_INS_AESDECLAST,
    translate_aesdec,    //X86_INS_AESDEC,
    translate_aesenclast,//X86_INS_AESENCLAST,
    translate_aesenc,    //X86_INS_AESENC,
    translate_aesimc,    //X86_INS_AESIMC,
    translate_aeskeygenassist,//X86_INS_AESKEYGENASSIST,
    translate_and,       //X86_INS_AND,
    NULL,                //X86_INS_ANDN,
    translate_andnpd,    //X86_INS_ANDNPD,
    translate_andnps,    //X86_INS_ANDNPS,
    translate_andpd,     //X86_INS_ANDPD,
    translate_andps,     //X86_INS_ANDPS,
    translate_arpl,      //X86_INS_ARPL,
    NULL,                //X86_INS_BEXTR,
    NULL,                //X86_INS_BLCFILL,
    NULL,                //X86_INS_BLCI,
    NULL,                //X86_INS_BLCIC,
    NULL,                //X86_INS_BLCMSK,
    NULL,                //X86_INS_BLCS,
    translate_blendpd,   //X86_INS_BLENDPD,
    translate_blendps,   //X86_INS_BLENDPS,
    translate_blendvpd,  //X86_INS_BLENDVPD,
    translate_blendvps,  //X86_INS_BLENDVPS,
    NULL,                //X86_INS_BLSFILL,
    NULL,                //X86_INS_BLSI,
    NULL,                //X86_INS_BLSIC,
    NULL,                //X86_INS_BLSMSK,
    NULL,                //X86_INS_BLSR,
    translate_bound,     //X86_INS_BOUND,
    translate_bsf,       //X86_INS_BSF,
    translate_bsr,       //X86_INS_BSR,
    translate_bswap,     //X86_INS_BSWAP,
    translate_btx,       //X86_INS_BT,
    translate_btx,       //X86_INS_BTC,
    translate_btx,       //X86_INS_BTR,
    translate_btx,       //X86_INS_BTS,
    NULL,                //X86_INS_BZHI,
    translate_call,      //X86_INS_CALL,
    translate_cbw,       //X86_INS_CBW,
    translate_cdq,       //X86_INS_CDQ,
    translate_cdqe,      //X86_INS_CDQE,
    translate_fchs,      //X86_INS_FCHS,
    translate_clac,      /* X86_INS_CLAC, */
    translate_clc,       //X86_INS_CLC,
    translate_cld,       //X86_INS_CLD,
    translate_clflush,   //X86_INS_CLFLUSH,
    NULL,                //X86_INS_CLFLUSHOPT,
    translate_clgi,      //X86_INS_CLGI,
    translate_cli,       //X86_INS_CLI,
    translate_clts,      //X86_INS_CLTS,
    NULL,                //X86_INS_CLWB,
    translate_cmc,       //X86_INS_CMC,
    translate_cmova,     //X86_INS_CMOVA,
    translate_cmovae,    //X86_INS_CMOVAE,
    translate_cmovb,     //X86_INS_CMOVB,
    translate_cmovbe,    //X86_INS_CMOVBE,
    translate_fcmovbe,   //X86_INS_FCMOVBE,
    translate_fcmovb,    //X86_INS_FCMOVB,
    translate_cmovz,     //X86_INS_CMOVE,
    translate_fcmove,    //X86_INS_FCMOVE,
    translate_cmovg,     //X86_INS_CMOVG,
    translate_cmovge,    //X86_INS_CMOVGE,
    translate_cmovl,     //X86_INS_CMOVL,
    translate_cmovle,    //X86_INS_CMOVLE,
    translate_fcmovnbe,  //X86_INS_FCMOVNBE,
    translate_fcmovnb,   //X86_INS_FCMOVNB,
    translate_cmovnz,    //X86_INS_CMOVNE,
    translate_fcmovne,   //X86_INS_FCMOVNE,
    translate_cmovno,    //X86_INS_CMOVNO,
    translate_cmovnp,    //X86_INS_CMOVNP,
    translate_fcmovnu,   //X86_INS_FCMOVNU,
    translate_cmovns,    //X86_INS_CMOVNS,
    translate_cmovo,     //X86_INS_CMOVO,
    translate_cmovp,     //X86_INS_CMOVP,
    translate_fcmovu,    //X86_INS_FCMOVU,
    translate_cmovs,     //X86_INS_CMOVS,
    translate_cmp,       //X86_INS_CMP,
    translate_cmps,      /* X86_INS_CMPSB, */
    translate_cmps,                /* X86_INS_CMPSQ, */
    translate_cmps,      /* X86_INS_CMPSW, */
    translate_cmpxchg16b,//X86_INS_CMPXCHG16B,
    translate_cmpxchg,   //X86_INS_CMPXCHG,
    translate_cmpxchg8b, //X86_INS_CMPXCHG8B,
    translate_comisd,    //X86_INS_COMISD,
    translate_comiss,    //X86_INS_COMISS,
    translate_fcomp,     //X86_INS_FCOMP,
    translate_fcomip,    //X86_INS_FCOMIP,
    translate_fcomi,     //X86_INS_FCOMI,
    translate_fcom,      //X86_INS_FCOM,
    translate_fcos,      //X86_INS_FCOS,
    translate_cpuid,     //X86_INS_CPUID,
    translate_cqo,       //X86_INS_CQO,
    translate_crc32,     //X86_INS_CRC32,
    translate_cvtdq2pd,  //X86_INS_CVTDQ2PD,
    translate_cvtdq2ps,  //X86_INS_CVTDQ2PS,
    translate_cvtpd2dq,  //X86_INS_CVTPD2DQ,
    translate_cvtpd2ps,  //X86_INS_CVTPD2PS,
    translate_cvtps2dq,  //X86_INS_CVTPS2DQ,
    translate_cvtps2pd,  //X86_INS_CVTPS2PD,
    translate_cvtsd2si,  //X86_INS_CVTSD2SI,
    translate_cvtsd2ss,  //X86_INS_CVTSD2SS,
    translate_cvtsi2sd,  //X86_INS_CVTSI2SD,
    translate_cvtsi2ss,  //X86_INS_CVTSI2SS,
    translate_cvtss2sd,  //X86_INS_CVTSS2SD,
    translate_cvtss2si,  //X86_INS_CVTSS2SI,
    translate_cvttpd2dq, //X86_INS_CVTTPD2DQ,
    translate_cvttps2dq, //X86_INS_CVTTPS2DQ,
    translate_cvttsd2si, //X86_INS_CVTTSD2SI,
    translate_cvttss2si, //X86_INS_CVTTSS2SI,
    translate_cwd,       //X86_INS_CWD,
    translate_cwde,      //X86_INS_CWDE,
    translate_daa,       //X86_INS_DAA,
    translate_das,       //X86_INS_DAS,
    NULL,                //X86_INS_DATA16,
    translate_dec,       //X86_INS_DEC,
    translate_div,       //X86_INS_DIV,
    translate_divpd,     //X86_INS_DIVPD,
    translate_divps,     //X86_INS_DIVPS,
    translate_fdivr,     //X86_INS_FDIVR,
    translate_fidivr,    //X86_INS_FIDIVR,
    translate_fdivrp,    //X86_INS_FDIVRP,
    translate_divsd,     //X86_INS_DIVSD,
    translate_divss,     //X86_INS_DIVSS,
    translate_fdiv,      //X86_INS_FDIV,
    translate_fidiv,     //X86_INS_FIDIV,
    translate_fdivp,     //X86_INS_FDIVP,
    translate_dppd,      //X86_INS_DPPD,
    translate_dpps,      //X86_INS_DPPS,
    translate_ret,       //X86_INS_RET,
    NULL,                //X86_INS_ENCLS,
    NULL,                //X86_INS_ENCLU,
    translate_enter,     //X86_INS_ENTER,
    translate_extractps, //X86_INS_EXTRACTPS,
    translate_extrq,     //X86_INS_EXTRQ,
    translate_f2xm1,     //X86_INS_F2XM1,
    translate_call_far,  /* X86_INS_LCALL, */
    translate_jmp_far,   /* X86_INS_LJMP */
    translate_fbld,      //X86_INS_FBLD,
    translate_fbstp,     //X86_INS_FBSTP,
    translate_fcompp,    //X86_INS_FCOMPP,
    translate_fdecstp,   //X86_INS_FDECSTP,
    translate_femms,     //X86_INS_FEMMS,
    translate_ffree,     //X86_INS_FFREE,
    translate_ficom,     //X86_INS_FICOM,
    translate_ficomp,    //X86_INS_FICOMP,
    translate_fincstp,   //X86_INS_FINCSTP,
    translate_fldcw,     //X86_INS_FLDCW,
    translate_fldenv,    //X86_INS_FLDENV,
    translate_fldl2e,    //X86_INS_FLDL2E,
    translate_fldl2t,    //X86_INS_FLDL2T,
    translate_fldlg2,    //X86_INS_FLDLG2,
    translate_fldln2,    //X86_INS_FLDLN2,
    translate_fldpi,     //X86_INS_FLDPI,
    translate_fnclex,    //X86_INS_FNCLEX,
    translate_fninit,    //X86_INS_FNINIT,
    translate_fnop,      //X86_INS_FNOP,
    translate_fnstcw,    //X86_INS_FNSTCW,
    translate_fnstsw,    //X86_INS_FNSTSW,
    translate_fpatan,    //X86_INS_FPATAN,
    translate_fprem,     //X86_INS_FPREM,
    translate_fprem1,    //X86_INS_FPREM1,
    translate_fptan,     //X86_INS_FPTAN,
    NULL,                //X86_INS_FFREEP,
    translate_frndint,   //X86_INS_FRNDINT,
    translate_frstor,    //X86_INS_FRSTOR,
    translate_fnsave,    //X86_INS_FNSAVE,
    translate_fscale,    //X86_INS_FSCALE,
    translate_fsetpm,    //X86_INS_FSETPM,
    translate_fsincos,   //X86_INS_FSINCOS,
    translate_fnstenv,   //X86_INS_FNSTENV,
    translate_fxam,      //X86_INS_FXAM,
    translate_fxrstor,   //X86_INS_FXRSTOR,
    translate_fxrstor,   /* X86_INS_FXRSTOR64, */
    translate_fxsave,    //X86_INS_FXSAVE,
    translate_fxsave,    /* X86_INS_FXSAVE64, */
    translate_fxtract,   //X86_INS_FXTRACT,
    translate_fyl2x,     //X86_INS_FYL2X,
    translate_fyl2xp1,   //X86_INS_FYL2XP1,
    translate_movapd,    //X86_INS_MOVAPD,
    translate_movaps,    //X86_INS_MOVAPS,
    translate_orpd,      //X86_INS_ORPD,
    translate_orps,      //X86_INS_ORPS,
    translate_vmovapd,   //X86_INS_VMOVAPD,
    translate_vmovaps,   //X86_INS_VMOVAPS,
    translate_xorpd,     //X86_INS_XORPD,
    translate_xorps,     //X86_INS_XORPS,
    translate_getsec,    //X86_INS_GETSEC,
    translate_haddpd,    //X86_INS_HADDPD,
    translate_haddps,    //X86_INS_HADDPS,
    translate_hlt,       //X86_INS_HLT,
    translate_hsubpd,    //X86_INS_HSUBPD,
    translate_hsubps,    //X86_INS_HSUBPS,
    translate_idiv,      //X86_INS_IDIV,
    translate_fild,      //X86_INS_FILD,
    translate_imul,      //X86_INS_IMUL,
    translate_in,        //X86_INS_IN,
    translate_inc,       //X86_INS_INC,
    translate_ins,       /* X86_INS_INSB, */
    translate_insertps,  //X86_INS_INSERTPS,
    translate_insertq,   //X86_INS_INSERTQ,
    translate_ins,       /* X86_INS_INSD, */
    translate_ins,       /* X86_INS_INSW, */
    translate_int,       //X86_INS_INT,
    translate_int1,      //X86_INS_INT1,
    translate_int_3,     /* X86_INS_INT3, */
    translate_into,      //X86_INS_INTO,
    translate_invd,      //X86_INS_INVD,
    translate_invept,    //X86_INS_INVEPT,
    translate_invlpg,    //X86_INS_INVLPG,
    translate_invlpga,   //X86_INS_INVLPGA,
    NULL,                //X86_INS_INVPCID,
    translate_invvpid,   //X86_INS_INVVPID,
    translate_iret,      //X86_INS_IRET,
    translate_iret,      //X86_INS_IRETD,
    translate_iret,      /* X86_INS_IRETQ */
    translate_fisttp,    //X86_INS_FISTTP,
    translate_fist,      //X86_INS_FIST,
    translate_fistp,     //X86_INS_FISTP,
    translate_ucomisd,   //X86_INS_UCOMISD,
    translate_ucomiss,   //X86_INS_UCOMISS,
    translate_vcomisd,   //X86_INS_VCOMISD,
    translate_vcomiss,   //X86_INS_VCOMISS,
    translate_vcvtsd2ss, //X86_INS_VCVTSD2SS,
    translate_vcvtsi2sd, //X86_INS_VCVTSI2SD,
    translate_vcvtsi2ss, //X86_INS_VCVTSI2SS,
    translate_vcvtss2sd, //X86_INS_VCVTSS2SD,
    translate_vcvttsd2si,//X86_INS_VCVTTSD2SI,
    NULL,                //X86_INS_VCVTTSD2USI,
    translate_vcvttss2si,//X86_INS_VCVTTSS2SI,
    NULL,                //X86_INS_VCVTTSS2USI,
    NULL,                //X86_INS_VCVTUSI2SD,
    NULL,                //X86_INS_VCVTUSI2SS,
    translate_vucomisd,  //X86_INS_VUCOMISD,
    translate_vucomiss,  //X86_INS_VUCOMISS,
    translate_jae,       //X86_INS_JAE,
    translate_ja,        //X86_INS_JA,
    translate_jbe,       //X86_INS_JBE,
    translate_jb,        //X86_INS_JB,
    translate_jcxz,      //X86_INS_JCXZ,
    translate_jecxz,     //X86_INS_JECXZ,
    translate_jz,                //X86_INS_JE,
    translate_jge,       //X86_INS_JGE,
    translate_jg,        //X86_INS_JG,
    translate_jle,       //X86_INS_JLE,
    translate_jl,        //X86_INS_JL,
    translate_jmp,       //X86_INS_JMP,
    translate_jnz,                //X86_INS_JNE,
    translate_jno,       //X86_INS_JNO,
    translate_jnp,       //X86_INS_JNP,
    translate_jns,       //X86_INS_JNS,
    translate_jo,        //X86_INS_JO,
    translate_jp,        //X86_INS_JP,
    translate_jrcxz,     //X86_INS_JRCXZ,
    translate_js,        //X86_INS_JS,
    NULL,                //X86_INS_KANDB,
    NULL,                //X86_INS_KANDD,
    NULL,                //X86_INS_KANDNB,
    NULL,                //X86_INS_KANDND,
    NULL,                //X86_INS_KANDNQ,
    NULL,                //X86_INS_KANDNW,
    NULL,                //X86_INS_KANDQ,
    NULL,                //X86_INS_KANDW,
    NULL,                //X86_INS_KMOVB,
    NULL,                //X86_INS_KMOVD,
    NULL,                //X86_INS_KMOVQ,
    NULL,                //X86_INS_KMOVW,
    NULL,                //X86_INS_KNOTB,
    NULL,                //X86_INS_KNOTD,
    NULL,                //X86_INS_KNOTQ,
    NULL,                //X86_INS_KNOTW,
    NULL,                //X86_INS_KORB,
    NULL,                //X86_INS_KORD,
    NULL,                //X86_INS_KORQ,
    NULL,                //X86_INS_KORTESTB,
    NULL,                //X86_INS_KORTESTD,
    NULL,                //X86_INS_KORTESTQ,
    NULL,                //X86_INS_KORTESTW,
    NULL,                //X86_INS_KORW,
    NULL,                //X86_INS_KSHIFTLB,
    NULL,                //X86_INS_KSHIFTLD,
    NULL,                //X86_INS_KSHIFTLQ,
    NULL,                //X86_INS_KSHIFTLW,
    NULL,                //X86_INS_KSHIFTRB,
    NULL,                //X86_INS_KSHIFTRD,
    NULL,                //X86_INS_KSHIFTRQ,
    NULL,                //X86_INS_KSHIFTRW,
    NULL,                //X86_INS_KUNPCKBW,
    NULL,                //X86_INS_KXNORB,
    NULL,                //X86_INS_KXNORD,
    NULL,                //X86_INS_KXNORQ,
    NULL,                //X86_INS_KXNORW,
    NULL,                //X86_INS_KXORB,
    NULL,                //X86_INS_KXORD,
    NULL,                //X86_INS_KXORQ,
    NULL,                //X86_INS_KXORW,
    translate_lahf,      //X86_INS_LAHF,
    translate_lar,       //X86_INS_LAR,
    translate_lddqu,     //X86_INS_LDDQU,
    translate_ldmxcsr,   //X86_INS_LDMXCSR,
    translate_lds,       //X86_INS_LDS,
    translate_fldz,      //X86_INS_FLDZ,
    translate_fld1,      //X86_INS_FLD1,
    translate_fld,       //X86_INS_FLD,
    translate_lea,       //X86_INS_LEA,
    translate_leave,     //X86_INS_LEAVE,
    translate_les,       //X86_INS_LES,
    translate_lfence,    //X86_INS_LFENCE,
    translate_lfs,       //X86_INS_LFS,
    translate_lgdt,      //X86_INS_LGDT,
    translate_lgs,       //X86_INS_LGS,
    translate_lidt,      //X86_INS_LIDT,
    translate_lldt,      //X86_INS_LLDT,
    translate_lmsw,      //X86_INS_LMSW,
    translate_or,        //X86_INS_OR,
    translate_sub,       //X86_INS_SUB,
    translate_xor,       //X86_INS_XOR,
    translate_lods,      /* X86_INS_LODSB, */
    translate_lods,      /* X86_INS_LODSD, */
    translate_lods,      /* X86_INS_LODSQ, */
    translate_lods,      /* X86_INS_LODSW, */
    translate_loop,      //X86_INS_LOOP,
    translate_loopz,     //X86_INS_LOOPE,
    translate_loopnz,    //X86_INS_LOOPNE,
    translate_retf,      //X86_INS_RETF,
    translate_retf,      /* X86_INS_RETFQ, */
    translate_lsl,       //X86_INS_LSL,
    translate_lss,       //X86_INS_LSS,
    translate_ltr,       //X86_INS_LTR,
    translate_xadd,      //X86_INS_XADD,
    translate_lzcnt,     //X86_INS_LZCNT,
    translate_maskmovdqu,//X86_INS_MASKMOVDQU,
    translate_maxpd,     //X86_INS_MAXPD,
    translate_maxps,     //X86_INS_MAXPS,
    translate_maxsd,     //X86_INS_MAXSD,
    translate_maxss,     //X86_INS_MAXSS,
    translate_mfence,    //X86_INS_MFENCE,
    translate_minpd,     //X86_INS_MINPD,
    translate_minps,     //X86_INS_MINPS,
    translate_minsd,     //X86_INS_MINSD,
    translate_minss,     //X86_INS_MINSS,
    translate_cvtpd2pi,  //X86_INS_CVTPD2PI,
    translate_cvtpi2pd,  //X86_INS_CVTPI2PD,
    translate_cvtpi2ps,  //X86_INS_CVTPI2PS,
    translate_cvtps2pi,  //X86_INS_CVTPS2PI,
    translate_cvttpd2pi, //X86_INS_CVTTPD2PI,
    translate_cvttps2pi, //X86_INS_CVTTPS2PI,
    translate_emms,      //X86_INS_EMMS,
    translate_maskmovq,  //X86_INS_MASKMOVQ,
    translate_movd,      //X86_INS_MOVD,
    translate_movdq2q,   //X86_INS_MOVDQ2Q,
    translate_movntq,    //X86_INS_MOVNTQ,
    translate_movq2dq,   //X86_INS_MOVQ2DQ,
    translate_movq,      //X86_INS_MOVQ,
    translate_pabsb,     //X86_INS_PABSB,
    translate_pabsd,     //X86_INS_PABSD,
    translate_pabsw,     //X86_INS_PABSW,
    translate_packssdw,  //X86_INS_PACKSSDW,
    translate_packsswb,  //X86_INS_PACKSSWB,
    translate_packuswb,  //X86_INS_PACKUSWB,
    translate_paddb,     //X86_INS_PADDB,
    translate_paddd,     //X86_INS_PADDD,
    translate_paddq,     //X86_INS_PADDQ,
    translate_paddsb,    //X86_INS_PADDSB,
    translate_paddsw,    //X86_INS_PADDSW,
    translate_paddusb,   //X86_INS_PADDUSB,
    translate_paddusw,   //X86_INS_PADDUSW,
    translate_paddw,     //X86_INS_PADDW,
    translate_palignr,   //X86_INS_PALIGNR,
    translate_pandn,     //X86_INS_PANDN,
    translate_pand,      //X86_INS_PAND,
    translate_pavgb,     //X86_INS_PAVGB,
    translate_pavgw,     //X86_INS_PAVGW,
    translate_pcmpeqb,   //X86_INS_PCMPEQB,
    translate_pcmpeqd,   //X86_INS_PCMPEQD,
    translate_pcmpeqw,   //X86_INS_PCMPEQW,
    translate_pcmpgtb,   //X86_INS_PCMPGTB,
    translate_pcmpgtd,   //X86_INS_PCMPGTD,
    translate_pcmpgtw,   //X86_INS_PCMPGTW,
    translate_pextrw,    //X86_INS_PEXTRW,
    translate_phaddsw,   //X86_INS_PHADDSW,
    translate_phaddw,    //X86_INS_PHADDW,
    translate_phaddd,    //X86_INS_PHADDD,
    translate_phsubd,    //X86_INS_PHSUBD,
    translate_phsubsw,   //X86_INS_PHSUBSW,
    translate_phsubw,    //X86_INS_PHSUBW,
    translate_pinsrw,    //X86_INS_PINSRW,
    translate_pmaddubsw, //X86_INS_PMADDUBSW,
    translate_pmaddwd,   //X86_INS_PMADDWD,
    translate_pmaxsw,    //X86_INS_PMAXSW,
    translate_pmaxub,    //X86_INS_PMAXUB,
    translate_pminsw,    //X86_INS_PMINSW,
    translate_pminub,    //X86_INS_PMINUB,
    translate_pmovmskb,  //X86_INS_PMOVMSKB,
    translate_pmulhrsw,  //X86_INS_PMULHRSW,
    translate_pmulhuw,   //X86_INS_PMULHUW,
    translate_pmulhw,    //X86_INS_PMULHW,
    translate_pmullw,    //X86_INS_PMULLW,
    translate_pmuludq,   //X86_INS_PMULUDQ,
    translate_por,       //X86_INS_POR,
    translate_psadbw,    //X86_INS_PSADBW,
    translate_pshufb,    //X86_INS_PSHUFB,
    translate_pshufw,    //X86_INS_PSHUFW,
    translate_psignb,    //X86_INS_PSIGNB,
    translate_psignd,    //X86_INS_PSIGND,
    translate_psignw,    //X86_INS_PSIGNW,
    translate_pslld,     //X86_INS_PSLLD,
    translate_psllq,     //X86_INS_PSLLQ,
    translate_psllw,     //X86_INS_PSLLW,
    translate_psrad,     //X86_INS_PSRAD,
    translate_psraw,     //X86_INS_PSRAW,
    translate_psrld,     //X86_INS_PSRLD,
    translate_psrlq,     //X86_INS_PSRLQ,
    translate_psrlw,     //X86_INS_PSRLW,
    translate_psubb,     //X86_INS_PSUBB,
    translate_psubd,     //X86_INS_PSUBD,
    translate_psubq,     //X86_INS_PSUBQ,
    translate_psubsb,    //X86_INS_PSUBSB,
    translate_psubsw,    //X86_INS_PSUBSW,
    translate_psubusb,   //X86_INS_PSUBUSB,
    translate_psubusw,   //X86_INS_PSUBUSW,
    translate_psubw,     //X86_INS_PSUBW,
    translate_punpckhbw, //X86_INS_PUNPCKHBW,
    translate_punpckhdq, //X86_INS_PUNPCKHDQ,
    translate_punpckhwd, //X86_INS_PUNPCKHWD,
    translate_punpcklbw, //X86_INS_PUNPCKLBW,
    translate_punpckldq, //X86_INS_PUNPCKLDQ,
    translate_punpcklwd, //X86_INS_PUNPCKLWD,
    translate_pxor,      //X86_INS_PXOR,
    translate_monitor,   //X86_INS_MONITOR,
    NULL,                //X86_INS_MONTMUL,
    translate_mov,       //X86_INS_MOV,
    translate_mov,       /* X86_INS_MOVABS, */
    translate_movbe,     //X86_INS_MOVBE,
    translate_movddup,   //X86_INS_MOVDDUP,
    translate_movdqa,    //X86_INS_MOVDQA,
    translate_movdqu,    //X86_INS_MOVDQU,
    translate_movhlps,   //X86_INS_MOVHLPS,
    translate_movhpd,    //X86_INS_MOVHPD,
    translate_movhps,    //X86_INS_MOVHPS,
    translate_movlhps,   //X86_INS_MOVLHPS,
    translate_movlpd,    //X86_INS_MOVLPD,
    translate_movlps,    //X86_INS_MOVLPS,
    translate_movmskpd,  //X86_INS_MOVMSKPD,
    translate_movmskps,  //X86_INS_MOVMSKPS,
    translate_movntdqa,  //X86_INS_MOVNTDQA,
    translate_movntdq,   //X86_INS_MOVNTDQ,
    translate_movnti,    //X86_INS_MOVNTI,
    translate_movntpd,   //X86_INS_MOVNTPD,
    translate_movntps,   //X86_INS_MOVNTPS,
    translate_movntsd,   //X86_INS_MOVNTSD,
    translate_movntss,   //X86_INS_MOVNTSS,
    translate_movs,      /* X86_INS_MOVSB, */
    translate_movs,      /* X86_INS_MOVSD, */
    translate_movshdup,  //X86_INS_MOVSHDUP,
    translate_movsldup,  //X86_INS_MOVSLDUP,
    translate_movs,                /* X86_INS_MOVSQ, */
    translate_movss,     //X86_INS_MOVSS,
    translate_movs,      /* X86_INS_MOVSW, */
    translate_movsx,     //X86_INS_MOVSX,
    translate_movsxd,    //X86_INS_MOVSXD,
    translate_movupd,    //X86_INS_MOVUPD,
    translate_movups,    //X86_INS_MOVUPS,
    translate_movzx,     //X86_INS_MOVZX,
    translate_mpsadbw,   //X86_INS_MPSADBW,
    translate_mul,       //X86_INS_MUL,
    translate_mulpd,     //X86_INS_MULPD,
    translate_mulps,     //X86_INS_MULPS,
    translate_mulsd,     //X86_INS_MULSD,
    translate_mulss,     //X86_INS_MULSS,
    NULL,                //X86_INS_MULX,
    translate_fmul,      //X86_INS_FMUL,
    translate_fimul,     //X86_INS_FIMUL,
    translate_fmulp,     //X86_INS_FMULP,
    translate_mwait,     //X86_INS_MWAIT,
    translate_neg,       //X86_INS_NEG,
    translate_nop,       //X86_INS_NOP,
    translate_not,       //X86_INS_NOT,
    translate_out,       //X86_INS_OUT,
    translate_outs,      /* X86_INS_OUTSB, */
    translate_outs,      /* X86_INS_OUTSD, */
    translate_outs,      /* X86_INS_OUTSW, */
    translate_packusdw,  //X86_INS_PACKUSDW,
    translate_pause,     //X86_INS_PAUSE,
    translate_pavgusb,   //X86_INS_PAVGUSB,
    translate_pblendvb,  //X86_INS_PBLENDVB,
    translate_pblendw,   //X86_INS_PBLENDW,
    translate_pclmulqdq, //X86_INS_PCLMULQDQ,
    translate_pcmpeqq,   //X86_INS_PCMPEQQ,
    translate_pcmpestri, //X86_INS_PCMPESTRI,
    translate_pcmpestrm, //X86_INS_PCMPESTRM,
    translate_pcmpgtq,   //X86_INS_PCMPGTQ,
    translate_pcmpistri, //X86_INS_PCMPISTRI,
    translate_pcmpistrm, //X86_INS_PCMPISTRM,
    NULL,                //X86_INS_PCOMMIT,
    NULL,                //X86_INS_PDEP,
    NULL,                //X86_INS_PEXT,
    translate_pextrb,    //X86_INS_PEXTRB,
    translate_pextrd,    //X86_INS_PEXTRD,
    translate_pextrq,    //X86_INS_PEXTRQ,
    translate_pf2id,     //X86_INS_PF2ID,
    translate_pf2iw,     //X86_INS_PF2IW,
    translate_pfacc,     //X86_INS_PFACC,
    translate_pfadd,     //X86_INS_PFADD,
    translate_pfcmpeq,   //X86_INS_PFCMPEQ,
    translate_pfcmpge,   //X86_INS_PFCMPGE,
    translate_pfcmpgt,   //X86_INS_PFCMPGT,
    translate_pfmax,     //X86_INS_PFMAX,
    translate_pfmin,     //X86_INS_PFMIN,
    translate_pfmul,     //X86_INS_PFMUL,
    translate_pfnacc,    //X86_INS_PFNACC,
    translate_pfpnacc,   //X86_INS_PFPNACC,
    translate_pfrcpit1,  //X86_INS_PFRCPIT1,
    translate_pfrcpit2,  //X86_INS_PFRCPIT2,
    translate_pfrcp,     //X86_INS_PFRCP,
    translate_pfrsqit1,  //X86_INS_PFRSQIT1,
    translate_pfrsqrt,   //X86_INS_PFRSQRT,
    translate_pfsubr,    //X86_INS_PFSUBR,
    translate_pfsub,     //X86_INS_PFSUB,
    translate_phminposuw,//X86_INS_PHMINPOSUW,
    translate_pi2fd,     //X86_INS_PI2FD,
    translate_pi2fw,     //X86_INS_PI2FW,
    translate_pinsrb,    //X86_INS_PINSRB,
    translate_pinsrd,    //X86_INS_PINSRD,
    translate_pinsrq,    //X86_INS_PINSRQ,
    translate_pmaxsb,    //X86_INS_PMAXSB,
    translate_pmaxsd,    //X86_INS_PMAXSD,
    translate_pmaxud,    //X86_INS_PMAXUD,
    translate_pmaxuw,    //X86_INS_PMAXUW,
    translate_pminsb,    //X86_INS_PMINSB,
    translate_pminsd,    //X86_INS_PMINSD,
    translate_pminud,    //X86_INS_PMINUD,
    translate_pminuw,    //X86_INS_PMINUW,
    translate_pmovsxbd,  //X86_INS_PMOVSXBD,
    translate_pmovsxbq,  //X86_INS_PMOVSXBQ,
    translate_pmovsxbw,  //X86_INS_PMOVSXBW,
    translate_pmovsxdq,  //X86_INS_PMOVSXDQ,
    translate_pmovsxwd,  //X86_INS_PMOVSXWD,
    translate_pmovsxwq,  //X86_INS_PMOVSXWQ,
    translate_pmovzxbd,  //X86_INS_PMOVZXBD,
    translate_pmovzxbq,  //X86_INS_PMOVZXBQ,
    translate_pmovzxbw,  //X86_INS_PMOVZXBW,
    translate_pmovzxdq,  //X86_INS_PMOVZXDQ,
    translate_pmovzxwd,  //X86_INS_PMOVZXWD,
    translate_pmovzxwq,  //X86_INS_PMOVZXWQ,
    translate_pmuldq,    //X86_INS_PMULDQ,
    translate_pmulhrw,   //X86_INS_PMULHRW,
    translate_pmulld,    //X86_INS_PMULLD,
    translate_pop,       //X86_INS_POP,
    translate_popa,      /* X86_INS_POPAW, */
    translate_popa,      //X86_INS_POPAL,
    translate_popcnt,    //X86_INS_POPCNT,
    translate_popf,      //X86_INS_POPF,
    translate_popf,                //X86_INS_POPFD,
    translate_popf,                /* X86_INS_POPFQ, */
    translate_prefetch,  //X86_INS_PREFETCH,
    translate_prefetchnta,//X86_INS_PREFETCHNTA,
    translate_prefetcht0,//X86_INS_PREFETCHT0,
    translate_prefetcht1,//X86_INS_PREFETCHT1,
    translate_prefetcht2,//X86_INS_PREFETCHT2,
    translate_prefetchw, //X86_INS_PREFETCHW,
    translate_pshufd,    //X86_INS_PSHUFD,
    translate_pshufhw,   //X86_INS_PSHUFHW,
    translate_pshuflw,   //X86_INS_PSHUFLW,
    translate_pslldq,    //X86_INS_PSLLDQ,
    translate_psrldq,    //X86_INS_PSRLDQ,
    translate_pswapd,    //X86_INS_PSWAPD,
    translate_ptest,     //X86_INS_PTEST,
    translate_punpckhqdq,//X86_INS_PUNPCKHQDQ,
    translate_punpcklqdq,//X86_INS_PUNPCKLQDQ,
    translate_push,      //X86_INS_PUSH,
    translate_pusha,     /* X86_INS_PUSHAW, */
    translate_pusha,     //X86_INS_PUSHAL,
    translate_pushf,     //X86_INS_PUSHF,
    translate_pushf,                //X86_INS_PUSHFD,
    translate_pushf,                /* X86_INS_PUSHFQ, */
    translate_rcl,       //X86_INS_RCL,
    translate_rcpps,     //X86_INS_RCPPS,
    translate_rcpss,     //X86_INS_RCPSS,
    translate_rcr,       //X86_INS_RCR,
    NULL,                //X86_INS_RDFSBASE,
    NULL,                //X86_INS_RDGSBASE,
    translate_rdmsr,     //X86_INS_RDMSR,
    translate_rdpmc,     //X86_INS_RDPMC,
    NULL,                //X86_INS_RDRAND,
    NULL,                //X86_INS_RDSEED,
    translate_rdtsc,     //X86_INS_RDTSC,
    translate_rdtscp,    //X86_INS_RDTSCP,
    translate_rol,       //X86_INS_ROL,
    translate_ror,       //X86_INS_ROR,
    NULL,                //X86_INS_RORX,
    translate_roundpd,   //X86_INS_ROUNDPD,
    translate_roundps,   //X86_INS_ROUNDPS,
    translate_roundsd,   //X86_INS_ROUNDSD,
    translate_roundss,   //X86_INS_ROUNDSS,
    translate_rsm,       //X86_INS_RSM,
    translate_rsqrtps,   //X86_INS_RSQRTPS,
    translate_rsqrtss,   //X86_INS_RSQRTSS,
    translate_sahf,      //X86_INS_SAHF,
    translate_sal,       //X86_INS_SAL,
    translate_salc,      //X86_INS_SALC,
    translate_sar,       //X86_INS_SAR,
    NULL,                //X86_INS_SARX,
    translate_sbb,       //X86_INS_SBB,
    translate_scas,      /* X86_INS_SCASB, */
    translate_scas,      //X86_INS_SCASD,
    translate_scas,      /* X86_INS_SCASQ, */
    translate_scas,      /* X86_INS_SCASW, */
    translate_setae,     //X86_INS_SETAE,
    translate_seta,      //X86_INS_SETA,
    translate_setbe,     //X86_INS_SETBE,
    translate_setb,      //X86_INS_SETB,
    translate_setz,      //X86_INS_SETE,
    translate_setge,     //X86_INS_SETGE,
    translate_setg,      //X86_INS_SETG,
    translate_setle,     //X86_INS_SETLE,
    translate_setl,      //X86_INS_SETL,
    translate_setnz,     //X86_INS_SETNE,
    translate_setno,     //X86_INS_SETNO,
    translate_setnp,     //X86_INS_SETNP,
    translate_setns,     //X86_INS_SETNS,
    translate_seto,      //X86_INS_SETO,
    translate_setp,      //X86_INS_SETP,
    translate_sets,      //X86_INS_SETS,
    translate_sfence,    //X86_INS_SFENCE,
    translate_sgdt,      //X86_INS_SGDT,
    NULL,                //X86_INS_SHA1MSG1,
    NULL,                //X86_INS_SHA1MSG2,
    NULL,                //X86_INS_SHA1NEXTE,
    NULL,                //X86_INS_SHA1RNDS4,
    NULL,                //X86_INS_SHA256MSG1,
    NULL,                //X86_INS_SHA256MSG2,
    NULL,                //X86_INS_SHA256RNDS2,
    translate_shl,       //X86_INS_SHL,
    translate_shld,      //X86_INS_SHLD,
    NULL,                //X86_INS_SHLX,
    translate_shr,       //X86_INS_SHR,
    translate_shrd,      //X86_INS_SHRD,
    NULL,                //X86_INS_SHRX,
    translate_shufpd,    //X86_INS_SHUFPD,
    translate_shufps,    //X86_INS_SHUFPS,
    translate_sidt,      //X86_INS_SIDT,
    translate_fsin,      //X86_INS_FSIN,
    translate_skinit,    //X86_INS_SKINIT,
    translate_sldt,      //X86_INS_SLDT,
    translate_smsw,      //X86_INS_SMSW,
    translate_sqrtpd,    //X86_INS_SQRTPD,
    translate_sqrtps,    //X86_INS_SQRTPS,
    translate_sqrtsd,    //X86_INS_SQRTSD,
    translate_sqrtss,    //X86_INS_SQRTSS,
    translate_fsqrt,     //X86_INS_FSQRT,
    translate_stac,      /* X86_INS_STAC, */
    translate_stc,       //X86_INS_STC,
    translate_std,       //X86_INS_STD,
    translate_stgi,      //X86_INS_STGI,
    translate_sti,       //X86_INS_STI,
    translate_stmxcsr,   //X86_INS_STMXCSR,
    translate_stos,      /* X86_INS_STOSB, */
    translate_stos,      /* X86_INS_STOSD, */
    translate_stos,      //X86_INS_STOSQ,
    translate_stos,      /* X86_INS_STOSW, */
    translate_str,       //X86_INS_STR,
    translate_fst,       //X86_INS_FST,
    translate_fstp,      //X86_INS_FSTP,
    NULL,                //X86_INS_FSTPNCE,
    translate_fxch,      //X86_INS_FXCH,
    translate_subpd,     //X86_INS_SUBPD,
    translate_subps,     //X86_INS_SUBPS,
    translate_fsubr,     //X86_INS_FSUBR,
    translate_fisubr,    //X86_INS_FISUBR,
    translate_fsubrp,    //X86_INS_FSUBRP,
    translate_subsd,     //X86_INS_SUBSD,
    translate_subss,     //X86_INS_SUBSS,
    translate_fsub,      //X86_INS_FSUB,
    translate_fisub,     //X86_INS_FISUB,
    translate_fsubp,     //X86_INS_FSUBP,
    translate_swapgs,    //X86_INS_SWAPGS,
    translate_syscall,   //X86_INS_SYSCALL,
    translate_sysenter,  //X86_INS_SYSENTER,
    translate_sysexit,   //X86_INS_SYSEXIT,
    translate_sysret,    //X86_INS_SYSRET,
    NULL,                //X86_INS_T1MSKC,
    translate_test,      //X86_INS_TEST,
    translate_ud2,       //X86_INS_UD2,
    translate_ftst,      //X86_INS_FTST,
    translate_bsf,                //X86_INS_TZCNT,//remove it future
    NULL,                //X86_INS_TZMSK,
    translate_fucomip,   //X86_INS_FUCOMIP,
    translate_fucomi,    //X86_INS_FUCOMI,
    translate_fucompp,   //X86_INS_FUCOMPP,
    translate_fucomp,    //X86_INS_FUCOMP,
    translate_fucom,     //X86_INS_FUCOM,
    NULL,                //X86_INS_UD2B,
    translate_unpckhpd,  //X86_INS_UNPCKHPD,
    translate_unpckhps,  //X86_INS_UNPCKHPS,
    translate_unpcklpd,  //X86_INS_UNPCKLPD,
    translate_unpcklps,  //X86_INS_UNPCKLPS,
    translate_vaddpd,    //X86_INS_VADDPD,
    translate_vaddps,    //X86_INS_VADDPS,
    translate_vaddsd,    //X86_INS_VADDSD,
    translate_vaddss,    //X86_INS_VADDSS,
    translate_vaddsubpd, //X86_INS_VADDSUBPD,
    translate_vaddsubps, //X86_INS_VADDSUBPS,
    translate_vaesdeclast,//X86_INS_VAESDECLAST,
    translate_vaesdec,   //X86_INS_VAESDEC,
    translate_vaesenclast,//X86_INS_VAESENCLAST,
    translate_vaesenc,   //X86_INS_VAESENC,
    translate_vaesimc,   //X86_INS_VAESIMC,
    translate_vaeskeygenassist,//X86_INS_VAESKEYGENASSIST,
    NULL,                //X86_INS_VALIGND,
    NULL,                //X86_INS_VALIGNQ,
    translate_vandnpd,   //X86_INS_VANDNPD,
    translate_vandnps,   //X86_INS_VANDNPS,
    translate_vandpd,    //X86_INS_VANDPD,
    translate_vandps,    //X86_INS_VANDPS,
    NULL,                //X86_INS_VBLENDMPD,
    NULL,                //X86_INS_VBLENDMPS,
    translate_vblendpd,  //X86_INS_VBLENDPD,
    translate_vblendps,  //X86_INS_VBLENDPS,
    translate_vblendvpd, //X86_INS_VBLENDVPD,
    translate_vblendvps, //X86_INS_VBLENDVPS,
    translate_vbroadcastf128,//X86_INS_VBROADCASTF128,
    NULL,                //X86_INS_VBROADCASTI32X4,
    NULL,                //X86_INS_VBROADCASTI64X4,
    translate_vbroadcastsd,//X86_INS_VBROADCASTSD,
    translate_vbroadcastss,//X86_INS_VBROADCASTSS,
    NULL,                //X86_INS_VCOMPRESSPD,
    NULL,                //X86_INS_VCOMPRESSPS,
    translate_vcvtdq2pd, //X86_INS_VCVTDQ2PD,
    translate_vcvtdq2ps, //X86_INS_VCVTDQ2PS,
    NULL,                //X86_INS_VCVTPD2DQX,
    translate_vcvtpd2dq, //X86_INS_VCVTPD2DQ,
    NULL,                //X86_INS_VCVTPD2PSX,
    translate_vcvtpd2ps, //X86_INS_VCVTPD2PS,
    NULL,                //X86_INS_VCVTPD2UDQ,
    NULL,                //X86_INS_VCVTPH2PS,
    translate_vcvtps2dq, //X86_INS_VCVTPS2DQ,
    translate_vcvtps2pd, //X86_INS_VCVTPS2PD,
    NULL,                //X86_INS_VCVTPS2PH,
    NULL,                //X86_INS_VCVTPS2UDQ,
    translate_vcvtsd2si, //X86_INS_VCVTSD2SI,
    NULL,                //X86_INS_VCVTSD2USI,
    translate_vcvtss2si, //X86_INS_VCVTSS2SI,
    NULL,                //X86_INS_VCVTSS2USI,
    NULL,                //X86_INS_VCVTTPD2DQX,
    translate_vcvttpd2dq,//X86_INS_VCVTTPD2DQ,
    NULL,                //X86_INS_VCVTTPD2UDQ,
    translate_vcvttps2dq,//X86_INS_VCVTTPS2DQ,
    NULL,                //X86_INS_VCVTTPS2UDQ,
    NULL,                //X86_INS_VCVTUDQ2PD,
    NULL,                //X86_INS_VCVTUDQ2PS,
    translate_vdivpd,    //X86_INS_VDIVPD,
    translate_vdivps,    //X86_INS_VDIVPS,
    translate_vdivsd,    //X86_INS_VDIVSD,
    translate_vdivss,    //X86_INS_VDIVSS,
    translate_vdppd,     //X86_INS_VDPPD,
    translate_vdpps,     //X86_INS_VDPPS,
    translate_verr,      //X86_INS_VERR,
    translate_verw,      //X86_INS_VERW,
    NULL,                //X86_INS_VEXP2PD,
    NULL,                //X86_INS_VEXP2PS,
    NULL,                //X86_INS_VEXPANDPD,
    NULL,                //X86_INS_VEXPANDPS,
    translate_vextractf128,//X86_INS_VEXTRACTF128,
    NULL,                //X86_INS_VEXTRACTF32X4,
    NULL,                //X86_INS_VEXTRACTF64X4,
    NULL,                //X86_INS_VEXTRACTI128,
    NULL,                //X86_INS_VEXTRACTI32X4,
    NULL,                //X86_INS_VEXTRACTI64X4,
    translate_vextractps,//X86_INS_VEXTRACTPS,
    translate_vfmadd132pd,//X86_INS_VFMADD132PD,
    translate_vfmadd132ps,//X86_INS_VFMADD132PS,
    NULL,                //X86_INS_VFMADDPD,
    translate_vfmadd213pd,//X86_INS_VFMADD213PD,
    translate_vfmadd231pd,//X86_INS_VFMADD231PD,
    NULL,                //X86_INS_VFMADDPS,
    translate_vfmadd213ps,//X86_INS_VFMADD213PS,
    translate_vfmadd231ps,//X86_INS_VFMADD231PS,
    NULL,                //X86_INS_VFMADDSD,
    translate_vfmadd213sd,//X86_INS_VFMADD213SD,
    translate_vfmadd132sd,//X86_INS_VFMADD132SD,
    translate_vfmadd231sd,//X86_INS_VFMADD231SD,
    NULL,                //X86_INS_VFMADDSS,
    translate_vfmadd213ss,//X86_INS_VFMADD213SS,
    translate_vfmadd132ss,//X86_INS_VFMADD132SS,
    translate_vfmadd231ss,//X86_INS_VFMADD231SS,
    translate_vfmaddsub132pd,//X86_INS_VFMADDSUB132PD,
    translate_vfmaddsub132ps,//X86_INS_VFMADDSUB132PS,
    NULL,                //X86_INS_VFMADDSUBPD,
    translate_vfmaddsub213pd,//X86_INS_VFMADDSUB213PD,
    translate_vfmaddsub231pd,//X86_INS_VFMADDSUB231PD,
    NULL,                //X86_INS_VFMADDSUBPS,
    translate_vfmaddsub213ps,//X86_INS_VFMADDSUB213PS,
    translate_vfmaddsub231ps,//X86_INS_VFMADDSUB231PS,
    translate_vfmsub132pd,//X86_INS_VFMSUB132PD,
    translate_vfmsub132ps,//X86_INS_VFMSUB132PS,
    translate_vfmsubadd132pd,//X86_INS_VFMSUBADD132PD,
    translate_vfmsubadd132ps,//X86_INS_VFMSUBADD132PS,
    NULL,                //X86_INS_VFMSUBADDPD,
    translate_vfmsubadd213pd,//X86_INS_VFMSUBADD213PD,
    translate_vfmsubadd231pd,//X86_INS_VFMSUBADD231PD,
    NULL,                //X86_INS_VFMSUBADDPS,
    translate_vfmsubadd213ps,//X86_INS_VFMSUBADD213PS,
    translate_vfmsubadd231ps,//X86_INS_VFMSUBADD231PS,
    NULL,                //X86_INS_VFMSUBPD,
    translate_vfmsub213pd,//X86_INS_VFMSUB213PD,
    translate_vfmsub231pd,//X86_INS_VFMSUB231PD,
    NULL,                //X86_INS_VFMSUBPS,
    translate_vfmsub213ps,//X86_INS_VFMSUB213PS,
    translate_vfmsub231ps,//X86_INS_VFMSUB231PS,
    NULL,                //X86_INS_VFMSUBSD,
    translate_vfmsub213sd,//X86_INS_VFMSUB213SD,
    translate_vfmsub132sd,//X86_INS_VFMSUB132SD,
    translate_vfmsub231sd,//X86_INS_VFMSUB231SD,
    NULL,                //X86_INS_VFMSUBSS,
    translate_vfmsub213ss,//X86_INS_VFMSUB213SS,
    translate_vfmsub132ss,//X86_INS_VFMSUB132SS,
    translate_vfmsub231ss,//X86_INS_VFMSUB231SS,
    translate_vfnmadd132pd,//X86_INS_VFNMADD132PD,
    translate_vfnmadd132ps,//X86_INS_VFNMADD132PS,
    NULL,                //X86_INS_VFNMADDPD,
    translate_vfnmadd213pd,//X86_INS_VFNMADD213PD,
    translate_vfnmadd231pd,//X86_INS_VFNMADD231PD,
    NULL,                //X86_INS_VFNMADDPS,
    translate_vfnmadd213ps,//X86_INS_VFNMADD213PS,
    translate_vfnmadd231ps,//X86_INS_VFNMADD231PS,
    NULL,                //X86_INS_VFNMADDSD,
    translate_vfnmadd213sd,//X86_INS_VFNMADD213SD,
    translate_vfnmadd132sd,//X86_INS_VFNMADD132SD,
    translate_vfnmadd231sd,//X86_INS_VFNMADD231SD,
    NULL,                //X86_INS_VFNMADDSS,
    translate_vfnmadd213ss,//X86_INS_VFNMADD213SS,
    translate_vfnmadd132ss,//X86_INS_VFNMADD132SS,
    translate_vfnmadd231ss,//X86_INS_VFNMADD231SS,
    translate_vfnmsub132pd,//X86_INS_VFNMSUB132PD,
    translate_vfnmsub132ps,//X86_INS_VFNMSUB132PS,
    NULL,                //X86_INS_VFNMSUBPD,
    translate_vfnmsub213pd,//X86_INS_VFNMSUB213PD,
    translate_vfnmsub231pd,//X86_INS_VFNMSUB231PD,
    NULL,                //X86_INS_VFNMSUBPS,
    translate_vfnmsub213ps,//X86_INS_VFNMSUB213PS,
    translate_vfnmsub231ps,//X86_INS_VFNMSUB231PS,
    NULL,                //X86_INS_VFNMSUBSD,
    translate_vfnmsub213sd,//X86_INS_VFNMSUB213SD,
    translate_vfnmsub132sd,//X86_INS_VFNMSUB132SD,
    translate_vfnmsub231sd,//X86_INS_VFNMSUB231SD,
    NULL,                //X86_INS_VFNMSUBSS,
    translate_vfnmsub213ss,//X86_INS_VFNMSUB213SS,
    translate_vfnmsub132ss,//X86_INS_VFNMSUB132SS,
    translate_vfnmsub231ss,//X86_INS_VFNMSUB231SS,
    NULL,                //X86_INS_VFRCZPD,
    NULL,                //X86_INS_VFRCZPS,
    NULL,                //X86_INS_VFRCZSD,
    NULL,                //X86_INS_VFRCZSS,
    translate_vorpd,     //X86_INS_VORPD,
    translate_vorps,     //X86_INS_VORPS,
    translate_vxorpd,    //X86_INS_VXORPD,
    translate_vxorps,    //X86_INS_VXORPS,
    NULL,                //X86_INS_VGATHERDPD,
    NULL,                //X86_INS_VGATHERDPS,
    NULL,                //X86_INS_VGATHERPF0DPD,
    NULL,                //X86_INS_VGATHERPF0DPS,
    NULL,                //X86_INS_VGATHERPF0QPD,
    NULL,                //X86_INS_VGATHERPF0QPS,
    NULL,                //X86_INS_VGATHERPF1DPD,
    NULL,                //X86_INS_VGATHERPF1DPS,
    NULL,                //X86_INS_VGATHERPF1QPD,
    NULL,                //X86_INS_VGATHERPF1QPS,
    NULL,                //X86_INS_VGATHERQPD,
    NULL,                //X86_INS_VGATHERQPS,
    translate_vhaddpd,   //X86_INS_VHADDPD,
    translate_vhaddps,   //X86_INS_VHADDPS,
    translate_vhsubpd,   //X86_INS_VHSUBPD,
    translate_vhsubps,   //X86_INS_VHSUBPS,
    translate_vinsertf128,//X86_INS_VINSERTF128,
    NULL,                //X86_INS_VINSERTF32X4,
    NULL,                //X86_INS_VINSERTF32X8,
    NULL,                //X86_INS_VINSERTF64X2,
    NULL,                //X86_INS_VINSERTF64X4,
    NULL,                //X86_INS_VINSERTI128,
    NULL,                //X86_INS_VINSERTI32X4,
    NULL,                //X86_INS_VINSERTI32X8,
    NULL,                //X86_INS_VINSERTI64X2,
    NULL,                //X86_INS_VINSERTI64X4,
    translate_vinsertps, //X86_INS_VINSERTPS,
    translate_vlddqu,    //X86_INS_VLDDQU,
    translate_vldmxcsr,  //X86_INS_VLDMXCSR,
    translate_vmaskmovdqu,//X86_INS_VMASKMOVDQU,
    translate_vmaskmovpd,//X86_INS_VMASKMOVPD,
    translate_vmaskmovps,//X86_INS_VMASKMOVPS,
    translate_vmaxpd,    //X86_INS_VMAXPD,
    translate_vmaxps,    //X86_INS_VMAXPS,
    translate_vmaxsd,    //X86_INS_VMAXSD,
    translate_vmaxss,    //X86_INS_VMAXSS,
    translate_vmcall,    //X86_INS_VMCALL,
    translate_vmclear,   //X86_INS_VMCLEAR,
    NULL,                //X86_INS_VMFUNC,
    translate_vminpd,    //X86_INS_VMINPD,
    translate_vminps,    //X86_INS_VMINPS,
    translate_vminsd,    //X86_INS_VMINSD,
    translate_vminss,    //X86_INS_VMINSS,
    translate_vmlaunch,  //X86_INS_VMLAUNCH,
    translate_vmload,    //X86_INS_VMLOAD,
    translate_vmmcall,   //X86_INS_VMMCALL,
    translate_vmovq,     //X86_INS_VMOVQ,
    translate_vmovddup,  //X86_INS_VMOVDDUP,
    translate_vmovd,     //X86_INS_VMOVD,
    NULL,                //X86_INS_VMOVDQA32,
    NULL,                //X86_INS_VMOVDQA64,
    translate_vmovdqa,   //X86_INS_VMOVDQA,
    NULL,                //X86_INS_VMOVDQU16,
    NULL,                //X86_INS_VMOVDQU32,
    NULL,                //X86_INS_VMOVDQU64,
    NULL,                //X86_INS_VMOVDQU8,
    translate_vmovdqu,   //X86_INS_VMOVDQU,
    translate_vmovhlps,  //X86_INS_VMOVHLPS,
    translate_vmovhpd,   //X86_INS_VMOVHPD,
    translate_vmovhps,   //X86_INS_VMOVHPS,
    translate_vmovlhps,  //X86_INS_VMOVLHPS,
    translate_vmovlpd,   //X86_INS_VMOVLPD,
    translate_vmovlps,   //X86_INS_VMOVLPS,
    translate_vmovmskpd, //X86_INS_VMOVMSKPD,
    translate_vmovmskps, //X86_INS_VMOVMSKPS,
    translate_vmovntdqa, //X86_INS_VMOVNTDQA,
    translate_vmovntdq,  //X86_INS_VMOVNTDQ,
    translate_vmovntpd,  //X86_INS_VMOVNTPD,
    translate_vmovntps,  //X86_INS_VMOVNTPS,
    translate_vmovsd,    //X86_INS_VMOVSD,
    translate_vmovshdup, //X86_INS_VMOVSHDUP,
    translate_vmovsldup, //X86_INS_VMOVSLDUP,
    translate_vmovss,    //X86_INS_VMOVSS,
    translate_vmovupd,   //X86_INS_VMOVUPD,
    translate_vmovups,   //X86_INS_VMOVUPS,
    translate_vmpsadbw,  //X86_INS_VMPSADBW,
    translate_vmptrld,   //X86_INS_VMPTRLD,
    translate_vmptrst,   //X86_INS_VMPTRST,
    translate_vmread,    //X86_INS_VMREAD,
    translate_vmresume,  //X86_INS_VMRESUME,
    translate_vmrun,     //X86_INS_VMRUN,
    translate_vmsave,    //X86_INS_VMSAVE,
    translate_vmulpd,    //X86_INS_VMULPD,
    translate_vmulps,    //X86_INS_VMULPS,
    translate_vmulsd,    //X86_INS_VMULSD,
    translate_vmulss,    //X86_INS_VMULSS,
    translate_vmwrite,   //X86_INS_VMWRITE,
    translate_vmxoff,    //X86_INS_VMXOFF,
    translate_vmxon,     //X86_INS_VMXON,
    translate_vpabsb,    //X86_INS_VPABSB,
    translate_vpabsd,    //X86_INS_VPABSD,
    NULL,                //X86_INS_VPABSQ,
    translate_vpabsw,    //X86_INS_VPABSW,
    translate_vpackssdw, //X86_INS_VPACKSSDW,
    translate_vpacksswb, //X86_INS_VPACKSSWB,
    translate_vpackusdw, //X86_INS_VPACKUSDW,
    translate_vpackuswb, //X86_INS_VPACKUSWB,
    translate_vpaddb,    //X86_INS_VPADDB,
    translate_vpaddd,    //X86_INS_VPADDD,
    translate_vpaddq,    //X86_INS_VPADDQ,
    translate_vpaddsb,   //X86_INS_VPADDSB,
    translate_vpaddsw,   //X86_INS_VPADDSW,
    NULL,                //X86_INS_VPADDUSB,
    translate_vpaddusw,  //X86_INS_VPADDUSW,
    translate_vpaddw,    //X86_INS_VPADDW,
    translate_vpalignr,  //X86_INS_VPALIGNR,
    NULL,                //X86_INS_VPANDD,
    NULL,                //X86_INS_VPANDND,
    NULL,                //X86_INS_VPANDNQ,
    translate_vpandn,    //X86_INS_VPANDN,
    NULL,                //X86_INS_VPANDQ,
    translate_vpand,     //X86_INS_VPAND,
    translate_vpavgb,    //X86_INS_VPAVGB,
    translate_vpavgw,    //X86_INS_VPAVGW,
    NULL,                //X86_INS_VPBLENDD,
    NULL,                //X86_INS_VPBLENDMB,
    NULL,                //X86_INS_VPBLENDMD,
    NULL,                //X86_INS_VPBLENDMQ,
    NULL,                //X86_INS_VPBLENDMW,
    translate_vpblendvb, //X86_INS_VPBLENDVB,
    NULL,                //X86_INS_VPBLENDW,
    NULL,                //X86_INS_VPBROADCASTB,
    NULL,                //X86_INS_VPBROADCASTD,
    NULL,                //X86_INS_VPBROADCASTMB2Q,
    NULL,                //X86_INS_VPBROADCASTMW2D,
    NULL,                //X86_INS_VPBROADCASTQ,
    NULL,                //X86_INS_VPBROADCASTW,
    translate_vpclmulqdq,//X86_INS_VPCLMULQDQ,
    NULL,                //X86_INS_VPCMOV,
    NULL,                //X86_INS_VPCMPB,
    NULL,                //X86_INS_VPCMPD,
    translate_vpcmpeqb,  //X86_INS_VPCMPEQB,
    translate_vpcmpeqd,  //X86_INS_VPCMPEQD,
    translate_vpcmpeqq,  //X86_INS_VPCMPEQQ,
    translate_vpcmpeqw,  //X86_INS_VPCMPEQW,
    NULL,                //X86_INS_VPCMPESTRI,
    translate_vpcmpestrm,//X86_INS_VPCMPESTRM,
    translate_vpcmpgtb,  //X86_INS_VPCMPGTB,
    translate_vpcmpgtd,  //X86_INS_VPCMPGTD,
    translate_vpcmpgtq,  //X86_INS_VPCMPGTQ,
    translate_vpcmpgtw,  //X86_INS_VPCMPGTW,
    translate_vpcmpistri,//X86_INS_VPCMPISTRI,
    translate_vpcmpistrm,//X86_INS_VPCMPISTRM,
    NULL,                //X86_INS_VPCMPQ,
    NULL,                //X86_INS_VPCMPUB,
    NULL,                //X86_INS_VPCMPUD,
    NULL,                //X86_INS_VPCMPUQ,
    NULL,                //X86_INS_VPCMPUW,
    NULL,                //X86_INS_VPCMPW,
    NULL,                //X86_INS_VPCOMB,
    NULL,                //X86_INS_VPCOMD,
    NULL,                //X86_INS_VPCOMPRESSD,
    NULL,                //X86_INS_VPCOMPRESSQ,
    NULL,                //X86_INS_VPCOMQ,
    NULL,                //X86_INS_VPCOMUB,
    NULL,                //X86_INS_VPCOMUD,
    NULL,                //X86_INS_VPCOMUQ,
    NULL,                //X86_INS_VPCOMUW,
    NULL,                //X86_INS_VPCOMW,
    NULL,                //X86_INS_VPCONFLICTD,
    NULL,                //X86_INS_VPCONFLICTQ,
    translate_vperm2f128,//X86_INS_VPERM2F128,
    NULL,                //X86_INS_VPERM2I128,
    NULL,                //X86_INS_VPERMD,
    NULL,                //X86_INS_VPERMI2D,
    NULL,                //X86_INS_VPERMI2PD,
    NULL,                //X86_INS_VPERMI2PS,
    NULL,                //X86_INS_VPERMI2Q,
    NULL,                //X86_INS_VPERMIL2PD,
    NULL,                //X86_INS_VPERMIL2PS,
    translate_vpermilpd, //X86_INS_VPERMILPD,
    translate_vpermilps, //X86_INS_VPERMILPS,
    NULL,                //X86_INS_VPERMPD,
    NULL,                //X86_INS_VPERMPS,
    NULL,                //X86_INS_VPERMQ,
    NULL,                //X86_INS_VPERMT2D,
    NULL,                //X86_INS_VPERMT2PD,
    NULL,                //X86_INS_VPERMT2PS,
    NULL,                //X86_INS_VPERMT2Q,
    NULL,                //X86_INS_VPEXPANDD,
    NULL,                //X86_INS_VPEXPANDQ,
    translate_vpextrb,   //X86_INS_VPEXTRB,
    translate_vpextrd,   //X86_INS_VPEXTRD,
    NULL,                //X86_INS_VPEXTRQ,
    translate_vpextrw,   //X86_INS_VPEXTRW,
    NULL,                //X86_INS_VPGATHERDD,
    NULL,                //X86_INS_VPGATHERDQ,
    NULL,                //X86_INS_VPGATHERQD,
    NULL,                //X86_INS_VPGATHERQQ,
    NULL,                //X86_INS_VPHADDBD,
    NULL,                //X86_INS_VPHADDBQ,
    NULL,                //X86_INS_VPHADDBW,
    NULL,                //X86_INS_VPHADDDQ,
    translate_vphaddd,   //X86_INS_VPHADDD,
    translate_vphaddsw,  //X86_INS_VPHADDSW,
    NULL,                //X86_INS_VPHADDUBD,
    NULL,                //X86_INS_VPHADDUBQ,
    NULL,                //X86_INS_VPHADDUBW,
    NULL,                //X86_INS_VPHADDUDQ,
    NULL,                //X86_INS_VPHADDUWD,
    NULL,                //X86_INS_VPHADDUWQ,
    NULL,                //X86_INS_VPHADDWD,
    NULL,                //X86_INS_VPHADDWQ,
    translate_vphaddw,   //X86_INS_VPHADDW,
    translate_vphminposuw,//X86_INS_VPHMINPOSUW,
    NULL,                //X86_INS_VPHSUBBW,
    NULL,                //X86_INS_VPHSUBDQ,
    translate_vphsubd,   //X86_INS_VPHSUBD,
    translate_vphsubsw,  //X86_INS_VPHSUBSW,
    NULL,                //X86_INS_VPHSUBWD,
    translate_vphsubw,   //X86_INS_VPHSUBW,
    translate_vpinsrb,   //X86_INS_VPINSRB,
    translate_vpinsrd,   //X86_INS_VPINSRD,
    translate_vpinsrq,   //X86_INS_VPINSRQ,
    translate_vpinsrw,   //X86_INS_VPINSRW,
    NULL,                //X86_INS_VPLZCNTD,
    NULL,                //X86_INS_VPLZCNTQ,
    NULL,                //X86_INS_VPMACSDD,
    NULL,                //X86_INS_VPMACSDQH,
    NULL,                //X86_INS_VPMACSDQL,
    NULL,                //X86_INS_VPMACSSDD,
    NULL,                //X86_INS_VPMACSSDQH,
    NULL,                //X86_INS_VPMACSSDQL,
    NULL,                //X86_INS_VPMACSSWD,
    NULL,                //X86_INS_VPMACSSWW,
    NULL,                //X86_INS_VPMACSWD,
    NULL,                //X86_INS_VPMACSWW,
    NULL,                //X86_INS_VPMADCSSWD,
    NULL,                //X86_INS_VPMADCSWD,
    translate_vpmaddubsw,//X86_INS_VPMADDUBSW,
    translate_vpmaddwd,  //X86_INS_VPMADDWD,
    NULL,                //X86_INS_VPMASKMOVD,
    NULL,                //X86_INS_VPMASKMOVQ,
    translate_vpmaxsb,   //X86_INS_VPMAXSB,
    translate_vpmaxsd,   //X86_INS_VPMAXSD,
    NULL,                //X86_INS_VPMAXSQ,
    translate_vpmaxsw,   //X86_INS_VPMAXSW,
    translate_vpmaxub,   //X86_INS_VPMAXUB,
    translate_vpmaxud,   //X86_INS_VPMAXUD,
    NULL,                //X86_INS_VPMAXUQ,
    translate_vpmaxuw,   //X86_INS_VPMAXUW,
    translate_vpminsb,   //X86_INS_VPMINSB,
    translate_vpminsd,   //X86_INS_VPMINSD,
    NULL,                //X86_INS_VPMINSQ,
    translate_vpminsw,   //X86_INS_VPMINSW,
    translate_vpminub,   //X86_INS_VPMINUB,
    translate_vpminud,   //X86_INS_VPMINUD,
    NULL,                //X86_INS_VPMINUQ,
    translate_vpminuw,   //X86_INS_VPMINUW,
    NULL,                //X86_INS_VPMOVDB,
    NULL,                //X86_INS_VPMOVDW,
    NULL,                //X86_INS_VPMOVM2B,
    NULL,                //X86_INS_VPMOVM2D,
    NULL,                //X86_INS_VPMOVM2Q,
    NULL,                //X86_INS_VPMOVM2W,
    translate_vpmovmskb, //X86_INS_VPMOVMSKB,
    NULL,                //X86_INS_VPMOVQB,
    NULL,                //X86_INS_VPMOVQD,
    NULL,                //X86_INS_VPMOVQW,
    NULL,                //X86_INS_VPMOVSDB,
    NULL,                //X86_INS_VPMOVSDW,
    NULL,                //X86_INS_VPMOVSQB,
    NULL,                //X86_INS_VPMOVSQD,
    NULL,                //X86_INS_VPMOVSQW,
    translate_vpmovsxbd, //X86_INS_VPMOVSXBD,
    translate_vpmovsxbq, //X86_INS_VPMOVSXBQ,
    translate_vpmovsxbw, //X86_INS_VPMOVSXBW,
    translate_vpmovsxdq, //X86_INS_VPMOVSXDQ,
    translate_vpmovsxwd, //X86_INS_VPMOVSXWD,
    translate_vpmovsxwq, //X86_INS_VPMOVSXWQ,
    NULL,                //X86_INS_VPMOVUSDB,
    NULL,                //X86_INS_VPMOVUSDW,
    NULL,                //X86_INS_VPMOVUSQB,
    NULL,                //X86_INS_VPMOVUSQD,
    NULL,                //X86_INS_VPMOVUSQW,
    translate_vpmovzxbd, //X86_INS_VPMOVZXBD,
    translate_vpmovzxbq, //X86_INS_VPMOVZXBQ,
    translate_vpmovzxbw, //X86_INS_VPMOVZXBW,
    translate_vpmovzxdq, //X86_INS_VPMOVZXDQ,
    translate_vpmovzxwd, //X86_INS_VPMOVZXWD,
    translate_vpmovzxwq, //X86_INS_VPMOVZXWQ,
    translate_vpmuldq,   //X86_INS_VPMULDQ,
    translate_vpmulhrsw, //X86_INS_VPMULHRSW,
    translate_vpmulhuw,  //X86_INS_VPMULHUW,
    translate_vpmulhw,   //X86_INS_VPMULHW,
    translate_vpmulld,   //X86_INS_VPMULLD,
    NULL,                //X86_INS_VPMULLQ,
    translate_vpmullw,   //X86_INS_VPMULLW,
    translate_vpmuludq,  //X86_INS_VPMULUDQ,
    NULL,                //X86_INS_VPORD,
    NULL,                //X86_INS_VPORQ,
    translate_vpor,      //X86_INS_VPOR,
    NULL,                //X86_INS_VPPERM,
    NULL,                //X86_INS_VPROTB,
    NULL,                //X86_INS_VPROTD,
    NULL,                //X86_INS_VPROTQ,
    NULL,                //X86_INS_VPROTW,
    translate_vpsadbw,   //X86_INS_VPSADBW,
    NULL,                //X86_INS_VPSCATTERDD,
    NULL,                //X86_INS_VPSCATTERDQ,
    NULL,                //X86_INS_VPSCATTERQD,
    NULL,                //X86_INS_VPSCATTERQQ,
    NULL,                //X86_INS_VPSHAB,
    NULL,                //X86_INS_VPSHAD,
    NULL,                //X86_INS_VPSHAQ,
    NULL,                //X86_INS_VPSHAW,
    NULL,                //X86_INS_VPSHLB,
    NULL,                //X86_INS_VPSHLD,
    NULL,                //X86_INS_VPSHLQ,
    NULL,                //X86_INS_VPSHLW,
    translate_vpshufb,   //X86_INS_VPSHUFB,
    translate_vpshufd,   //X86_INS_VPSHUFD,
    translate_vpshufhw,  //X86_INS_VPSHUFHW,
    translate_vpshuflw,  //X86_INS_VPSHUFLW,
    translate_vpsignb,   //X86_INS_VPSIGNB,
    translate_vpsignd,   //X86_INS_VPSIGND,
    translate_vpsignw,   //X86_INS_VPSIGNW,
    translate_vpslldq,   //X86_INS_VPSLLDQ,
    translate_vpslld,    //X86_INS_VPSLLD,
    translate_vpsllq,    //X86_INS_VPSLLQ,
    NULL,                //X86_INS_VPSLLVD,
    NULL,                //X86_INS_VPSLLVQ,
    translate_vpsllw,    //X86_INS_VPSLLW,
    translate_vpsrad,    //X86_INS_VPSRAD,
    NULL,                //X86_INS_VPSRAQ,
    NULL,                //X86_INS_VPSRAVD,
    NULL,                //X86_INS_VPSRAVQ,
    translate_vpsraw,    //X86_INS_VPSRAW,
    translate_vpsrldq,   //X86_INS_VPSRLDQ,
    translate_vpsrld,    //X86_INS_VPSRLD,
    translate_vpsrlq,    //X86_INS_VPSRLQ,
    NULL,                //X86_INS_VPSRLVD,
    NULL,                //X86_INS_VPSRLVQ,
    translate_vpsrlw,    //X86_INS_VPSRLW,
    translate_vpsubb,    //X86_INS_VPSUBB,
    translate_vpsubd,    //X86_INS_VPSUBD,
    translate_vpsubq,    //X86_INS_VPSUBQ,
    translate_vpsubsb,   //X86_INS_VPSUBSB,
    translate_vpsubsw,   //X86_INS_VPSUBSW,
    translate_vpsubusb,  //X86_INS_VPSUBUSB,
    translate_vpsubusw,  //X86_INS_VPSUBUSW,
    translate_vpsubw,    //X86_INS_VPSUBW,
    NULL,                //X86_INS_VPTESTMD,
    NULL,                //X86_INS_VPTESTMQ,
    NULL,                //X86_INS_VPTESTNMD,
    NULL,                //X86_INS_VPTESTNMQ,
    translate_vptest,    //X86_INS_VPTEST,
    translate_vpunpckhbw,//X86_INS_VPUNPCKHBW,
    translate_vpunpckhdq,//X86_INS_VPUNPCKHDQ,
    translate_vpunpckhqdq,//X86_INS_VPUNPCKHQDQ,
    translate_vpunpckhwd,//X86_INS_VPUNPCKHWD,
    translate_vpunpcklbw,//X86_INS_VPUNPCKLBW,
    translate_vpunpckldq,//X86_INS_VPUNPCKLDQ,
    translate_vpunpcklqdq,//X86_INS_VPUNPCKLQDQ,
    translate_vpunpcklwd,//X86_INS_VPUNPCKLWD,
    NULL,                //X86_INS_VPXORD,
    NULL,                //X86_INS_VPXORQ,
    translate_vpxor,     //X86_INS_VPXOR,
    NULL,                //X86_INS_VRCP14PD,
    NULL,                //X86_INS_VRCP14PS,
    NULL,                //X86_INS_VRCP14SD,
    NULL,                //X86_INS_VRCP14SS,
    NULL,                //X86_INS_VRCP28PD,
    NULL,                //X86_INS_VRCP28PS,
    NULL,                //X86_INS_VRCP28SD,
    NULL,                //X86_INS_VRCP28SS,
    translate_vrcpps,    //X86_INS_VRCPPS,
    translate_vrcpss,    //X86_INS_VRCPSS,
    NULL,                //X86_INS_VRNDSCALEPD,
    NULL,                //X86_INS_VRNDSCALEPS,
    NULL,                //X86_INS_VRNDSCALESD,
    NULL,                //X86_INS_VRNDSCALESS,
    translate_vroundpd,  //X86_INS_VROUNDPD,
    translate_vroundps,  //X86_INS_VROUNDPS,
    translate_vroundsd,  //X86_INS_VROUNDSD,
    translate_vroundss,  //X86_INS_VROUNDSS,
    NULL,                //X86_INS_VRSQRT14PD,
    NULL,                //X86_INS_VRSQRT14PS,
    NULL,                //X86_INS_VRSQRT14SD,
    NULL,                //X86_INS_VRSQRT14SS,
    NULL,                //X86_INS_VRSQRT28PD,
    NULL,                //X86_INS_VRSQRT28PS,
    NULL,                //X86_INS_VRSQRT28SD,
    NULL,                //X86_INS_VRSQRT28SS,
    translate_vrsqrtps,  //X86_INS_VRSQRTPS,
    translate_vrsqrtss,  //X86_INS_VRSQRTSS,
    NULL,                //X86_INS_VSCATTERDPD,
    NULL,                //X86_INS_VSCATTERDPS,
    NULL,                //X86_INS_VSCATTERPF0DPD,
    NULL,                //X86_INS_VSCATTERPF0DPS,
    NULL,                //X86_INS_VSCATTERPF0QPD,
    NULL,                //X86_INS_VSCATTERPF0QPS,
    NULL,                //X86_INS_VSCATTERPF1DPD,
    NULL,                //X86_INS_VSCATTERPF1DPS,
    NULL,                //X86_INS_VSCATTERPF1QPD,
    NULL,                //X86_INS_VSCATTERPF1QPS,
    NULL,                //X86_INS_VSCATTERQPD,
    NULL,                //X86_INS_VSCATTERQPS,
    translate_vshufpd,   //X86_INS_VSHUFPD,
    translate_vshufps,   //X86_INS_VSHUFPS,
    translate_vsqrtpd,   //X86_INS_VSQRTPD,
    translate_vsqrtps,   //X86_INS_VSQRTPS,
    translate_vsqrtsd,   //X86_INS_VSQRTSD,
    translate_vsqrtss,   //X86_INS_VSQRTSS,
    translate_vstmxcsr,  //X86_INS_VSTMXCSR,
    translate_vsubpd,    //X86_INS_VSUBPD,
    translate_vsubps,    //X86_INS_VSUBPS,
    translate_vsubsd,    //X86_INS_VSUBSD,
    translate_vsubss,    //X86_INS_VSUBSS,
    NULL,                //X86_INS_VTESTPD,
    NULL,                //X86_INS_VTESTPS,
    translate_vunpckhpd, //X86_INS_VUNPCKHPD,
    translate_vunpckhps, //X86_INS_VUNPCKHPS,
    translate_vunpcklpd, //X86_INS_VUNPCKLPD,
    translate_vunpcklps, //X86_INS_VUNPCKLPS,
    translate_vzeroall,  //X86_INS_VZEROALL,
    translate_vzeroupper,//X86_INS_VZEROUPPER,
    translate_wait,      //X86_INS_WAIT,
    translate_wbinvd,    //X86_INS_WBINVD,
    NULL,                //X86_INS_WRFSBASE,
    NULL,                //X86_INS_WRGSBASE,
    translate_wrmsr,     //X86_INS_WRMSR,
    NULL,                //X86_INS_XABORT,
    NULL,                //X86_INS_XACQUIRE,
    NULL,                //X86_INS_XBEGIN,
    translate_xchg,      //X86_INS_XCHG,
    NULL,                //X86_INS_XCRYPTCBC,
    NULL,                //X86_INS_XCRYPTCFB,
    NULL,                //X86_INS_XCRYPTCTR,
    NULL,                //X86_INS_XCRYPTECB,
    NULL,                //X86_INS_XCRYPTOFB,
    NULL,                //X86_INS_XEND,
    translate_xgetbv,    //X86_INS_XGETBV,
    translate_xlat,      /* X86_INS_XLATB */
    NULL,                //X86_INS_XRELEASE,
    translate_xrstor,    //X86_INS_XRSTOR,
    NULL,                //X86_INS_XRSTOR64,
    NULL,                //X86_INS_XRSTORS,
    NULL,                //X86_INS_XRSTORS64,
    translate_xsave,      /* X86_INS_XSAVE, */
    NULL,                //X86_INS_XSAVE64,
    NULL,                //X86_INS_XSAVEC,
    NULL,                //X86_INS_XSAVEC64,
    translate_xsaveopt,  /* X86_INS_XSAVEOPT, */
    NULL,                //X86_INS_XSAVEOPT64,
    NULL,                //X86_INS_XSAVES,
    NULL,                //X86_INS_XSAVES64,
    translate_xsetbv,    //X86_INS_XSETBV,
    NULL,                //X86_INS_XSHA1,
    NULL,                //X86_INS_XSHA256,
    NULL,                //X86_INS_XSTORE,
    NULL,                //X86_INS_XTEST,
    NULL,                //X86_INS_FDISI8087_NOP,
    NULL,                //X86_INS_FENI8087_NOP,
    NULL,                //X86_INS_CMPSS,
    translate_cmpeqss,   //X86_INS_CMPEQSS,
    translate_cmpltss,   //X86_INS_CMPLTSS,
    translate_cmpless,   //X86_INS_CMPLESS,
    translate_cmpunordss,//X86_INS_CMPUNORDSS,
    translate_cmpneqss,  //X86_INS_CMPNEQSS,
    translate_cmpnltss,  //X86_INS_CMPNLTSS,
    translate_cmpnless,  //X86_INS_CMPNLESS,
    translate_cmpordss,  //X86_INS_CMPORDSS,
    translate_cmps,      /* X86_INS_CMPSD, */
    translate_cmpeqsd,   //X86_INS_CMPEQSD,
    translate_cmpltsd,   //X86_INS_CMPLTSD,
    translate_cmplesd,   //X86_INS_CMPLESD,
    translate_cmpunordsd,//X86_INS_CMPUNORDSD,
    translate_cmpneqsd,  //X86_INS_CMPNEQSD,
    translate_cmpnltsd,  //X86_INS_CMPNLTSD,
    translate_cmpnlesd,  //X86_INS_CMPNLESD,
    translate_cmpordsd,  //X86_INS_CMPORDSD,
    NULL,                //X86_INS_CMPPS,
    translate_cmpeqps,   //X86_INS_CMPEQPS,
    translate_cmpltps,   //X86_INS_CMPLTPS,
    translate_cmpleps,   //X86_INS_CMPLEPS,
    translate_cmpunordps,//X86_INS_CMPUNORDPS,
    translate_cmpneqps,  //X86_INS_CMPNEQPS,
    translate_cmpnltps,  //X86_INS_CMPNLTPS,
    translate_cmpnleps,  //X86_INS_CMPNLEPS,
    translate_cmpordps,  //X86_INS_CMPORDPS,
    NULL,                //X86_INS_CMPPD,
    translate_cmpeqpd,   //X86_INS_CMPEQPD,
    translate_cmpltpd,   //X86_INS_CMPLTPD,
    translate_cmplepd,   //X86_INS_CMPLEPD,
    translate_cmpunordpd,//X86_INS_CMPUNORDPD,
    translate_cmpneqpd,  //X86_INS_CMPNEQPD,
    translate_cmpnltpd,  //X86_INS_CMPNLTPD,
    translate_cmpnlepd,  //X86_INS_CMPNLEPD,
    translate_cmpordpd,  //X86_INS_CMPORDPD,
    NULL,                //X86_INS_VCMPSS,
    translate_vcmpeqss,  //X86_INS_VCMPEQSS,
    translate_vcmpltss,  //X86_INS_VCMPLTSS,
    translate_vcmpless,  //X86_INS_VCMPLESS,
    translate_vcmpunordss,//X86_INS_VCMPUNORDSS,
    translate_vcmpneqss, //X86_INS_VCMPNEQSS,
    translate_vcmpnltss, //X86_INS_VCMPNLTSS,
    translate_vcmpnless, //X86_INS_VCMPNLESS,
    translate_vcmpordss, //X86_INS_VCMPORDSS,
    NULL,                //X86_INS_VCMPEQ_UQSS,
    NULL,                //X86_INS_VCMPNGESS,
    NULL,                //X86_INS_VCMPNGTSS,
    NULL,                //X86_INS_VCMPFALSESS,
    NULL,                //X86_INS_VCMPNEQ_OQSS,
    NULL,                //X86_INS_VCMPGESS,
    NULL,                //X86_INS_VCMPGTSS,
    NULL,                //X86_INS_VCMPTRUESS,
    NULL,                //X86_INS_VCMPEQ_OSSS,
    NULL,                //X86_INS_VCMPLT_OQSS,
    NULL,                //X86_INS_VCMPLE_OQSS,
    NULL,                //X86_INS_VCMPUNORD_SSS,
    NULL,                //X86_INS_VCMPNEQ_USSS,
    NULL,                //X86_INS_VCMPNLT_UQSS,
    NULL,                //X86_INS_VCMPNLE_UQSS,
    NULL,                //X86_INS_VCMPORD_SSS,
    NULL,                //X86_INS_VCMPEQ_USSS,
    NULL,                //X86_INS_VCMPNGE_UQSS,
    NULL,                //X86_INS_VCMPNGT_UQSS,
    NULL,                //X86_INS_VCMPFALSE_OSSS,
    NULL,                //X86_INS_VCMPNEQ_OSSS,
    NULL,                //X86_INS_VCMPGE_OQSS,
    NULL,                //X86_INS_VCMPGT_OQSS,
    NULL,                //X86_INS_VCMPTRUE_USSS,
    NULL,                //X86_INS_VCMPSD,
    translate_vcmpeqsd,  //X86_INS_VCMPEQSD,
    translate_vcmpltsd,  //X86_INS_VCMPLTSD,
    translate_vcmplesd,  //X86_INS_VCMPLESD,
    translate_vcmpunordsd,//X86_INS_VCMPUNORDSD,
    translate_vcmpneqsd, //X86_INS_VCMPNEQSD,
    translate_vcmpnltsd, //X86_INS_VCMPNLTSD,
    translate_vcmpnlesd, //X86_INS_VCMPNLESD,
    translate_vcmpordsd, //X86_INS_VCMPORDSD,
    NULL,                //X86_INS_VCMPEQ_UQSD,
    NULL,                //X86_INS_VCMPNGESD,
    NULL,                //X86_INS_VCMPNGTSD,
    NULL,                //X86_INS_VCMPFALSESD,
    NULL,                //X86_INS_VCMPNEQ_OQSD,
    NULL,                //X86_INS_VCMPGESD,
    NULL,                //X86_INS_VCMPGTSD,
    NULL,                //X86_INS_VCMPTRUESD,
    NULL,                //X86_INS_VCMPEQ_OSSD,
    NULL,                //X86_INS_VCMPLT_OQSD,
    NULL,                //X86_INS_VCMPLE_OQSD,
    NULL,                //X86_INS_VCMPUNORD_SSD,
    NULL,                //X86_INS_VCMPNEQ_USSD,
    NULL,                //X86_INS_VCMPNLT_UQSD,
    NULL,                //X86_INS_VCMPNLE_UQSD,
    NULL,                //X86_INS_VCMPORD_SSD,
    NULL,                //X86_INS_VCMPEQ_USSD,
    NULL,                //X86_INS_VCMPNGE_UQSD,
    NULL,                //X86_INS_VCMPNGT_UQSD,
    NULL,                //X86_INS_VCMPFALSE_OSSD,
    NULL,                //X86_INS_VCMPNEQ_OSSD,
    NULL,                //X86_INS_VCMPGE_OQSD,
    NULL,                //X86_INS_VCMPGT_OQSD,
    NULL,                //X86_INS_VCMPTRUE_USSD,
    NULL,                //X86_INS_VCMPPS,
    translate_vcmpeqps,  //X86_INS_VCMPEQPS,
    translate_vcmpltps,  //X86_INS_VCMPLTPS,
    translate_vcmpleps,  //X86_INS_VCMPLEPS,
    translate_vcmpunordps,//X86_INS_VCMPUNORDPS,
    translate_vcmpneqps, //X86_INS_VCMPNEQPS,
    translate_vcmpnltps, //X86_INS_VCMPNLTPS,
    translate_vcmpnleps, //X86_INS_VCMPNLEPS,
    translate_vcmpordps, //X86_INS_VCMPORDPS,
    NULL,                //X86_INS_VCMPEQ_UQPS,
    NULL,                //X86_INS_VCMPNGEPS,
    NULL,                //X86_INS_VCMPNGTPS,
    NULL,                //X86_INS_VCMPFALSEPS,
    NULL,                //X86_INS_VCMPNEQ_OQPS,
    NULL,                //X86_INS_VCMPGEPS,
    NULL,                //X86_INS_VCMPGTPS,
    NULL,                //X86_INS_VCMPTRUEPS,
    NULL,                //X86_INS_VCMPEQ_OSPS,
    NULL,                //X86_INS_VCMPLT_OQPS,
    NULL,                //X86_INS_VCMPLE_OQPS,
    NULL,                //X86_INS_VCMPUNORD_SPS,
    NULL,                //X86_INS_VCMPNEQ_USPS,
    NULL,                //X86_INS_VCMPNLT_UQPS,
    NULL,                //X86_INS_VCMPNLE_UQPS,
    NULL,                //X86_INS_VCMPORD_SPS,
    NULL,                //X86_INS_VCMPEQ_USPS,
    NULL,                //X86_INS_VCMPNGE_UQPS,
    NULL,                //X86_INS_VCMPNGT_UQPS,
    NULL,                //X86_INS_VCMPFALSE_OSPS,
    NULL,                //X86_INS_VCMPNEQ_OSPS,
    NULL,                //X86_INS_VCMPGE_OQPS,
    NULL,                //X86_INS_VCMPGT_OQPS,
    NULL,                //X86_INS_VCMPTRUE_USPS,
    NULL,                //X86_INS_VCMPPD,
    translate_vcmpeqpd,  //X86_INS_VCMPEQPD,
    translate_vcmpltpd,  //X86_INS_VCMPLTPD,
    translate_vcmplepd,  //X86_INS_VCMPLEPD,
    translate_vcmpunordpd,//X86_INS_VCMPUNORDPD,
    translate_vcmpneqpd, //X86_INS_VCMPNEQPD,
    translate_vcmpnltpd, //X86_INS_VCMPNLTPD,
    translate_vcmpnlepd, //X86_INS_VCMPNLEPD,
    translate_vcmpordpd, //X86_INS_VCMPORDPD,
    NULL,                //X86_INS_VCMPEQ_UQPD,
    NULL,                //X86_INS_VCMPNGEPD,
    NULL,                //X86_INS_VCMPNGTPD,
    NULL,                //X86_INS_VCMPFALSEPD,
    NULL,                //X86_INS_VCMPNEQ_OQPD,
    NULL,                //X86_INS_VCMPGEPD,
    NULL,                //X86_INS_VCMPGTPD,
    NULL,                //X86_INS_VCMPTRUEPD,
    NULL,                //X86_INS_VCMPEQ_OSPD,
    NULL,                //X86_INS_VCMPLT_OQPD,
    NULL,                //X86_INS_VCMPLE_OQPD,
    NULL,                //X86_INS_VCMPUNORD_SPD,
    NULL,                //X86_INS_VCMPNEQ_USPD,
    NULL,                //X86_INS_VCMPNLT_UQPD,
    NULL,                //X86_INS_VCMPNLE_UQPD,
    NULL,                //X86_INS_VCMPORD_SPD,
    NULL,                //X86_INS_VCMPEQ_USPD,
    NULL,                //X86_INS_VCMPNGE_UQPD,
    NULL,                //X86_INS_VCMPNGT_UQPD,
    NULL,                //X86_INS_VCMPFALSE_OSPD,
    NULL,                //X86_INS_VCMPNEQ_OSPD,
    NULL,                //X86_INS_VCMPGE_OQPD,
    NULL,                //X86_INS_VCMPGT_OQPD,
    NULL,                //X86_INS_VCMPTRUE_USPD,
    translate_ud0,       /* X86_INS_UD0, */
    translate_endbr32,   /* X86_INS_ENDBR32, */
    translate_endbr64,   /* X86_INS_ENDBR64 */
    NULL,                //X86_INS_ENDING, // mark the end of the list of insn
};

bool ir1_translate(IR1_INST *ir1)
{

#ifdef CONFIG_SOFTMMU
    lsassertm(latxs_is_ir1_ok(ir1),
            "IR1 is not ready in sys %s\n",
            ir1_name(ir1_opcode(ir1)));
#endif

    /* 2. call translate_xx function */
    int tr_func_idx = ir1_opcode(ir1) - X86_INS_INVALID;

#ifndef CONFIG_SOFTMMU
    if (option_by_hand) {
      translate_functions[X86_INS_ADD - X86_INS_INVALID] = translate_add_byhand;
      translate_functions[X86_INS_ADC - X86_INS_INVALID] = translate_adc_byhand;
      translate_functions[X86_INS_AND - X86_INS_INVALID] = translate_and_byhand;
      translate_functions[X86_INS_SUB - X86_INS_INVALID] = translate_sub_byhand;
      translate_functions[X86_INS_XOR - X86_INS_INVALID] = translate_xor_byhand;
      translate_functions[X86_INS_CMP - X86_INS_INVALID] = translate_cmp_byhand;
      translate_functions[X86_INS_INC - X86_INS_INVALID] = translate_inc_byhand;
      translate_functions[X86_INS_DEC - X86_INS_INVALID] = translate_dec_byhand;
      translate_functions[X86_INS_IMUL - X86_INS_INVALID] = translate_imul_byhand;
      translate_functions[X86_INS_TEST - X86_INS_INVALID] = translate_test_byhand;
      translate_functions[X86_INS_MOV - X86_INS_INVALID] = translate_mov_byhand;
      translate_functions[X86_INS_CMPXCHG - X86_INS_INVALID] = translate_cmpxchg_byhand;
      translate_functions[X86_INS_MOVZX - X86_INS_INVALID] =
          translate_movzx_byhand;
      translate_functions[X86_INS_MOVSX - X86_INS_INVALID] =
          translate_movsx_byhand;
      translate_functions[X86_INS_ROL - X86_INS_INVALID] = translate_rol_byhand;
      translate_functions[X86_INS_ROR - X86_INS_INVALID] = translate_ror_byhand;
      translate_functions[X86_INS_SHL - X86_INS_INVALID] = translate_shl_byhand;
      translate_functions[X86_INS_SHR - X86_INS_INVALID] = translate_shr_byhand;
      translate_functions[X86_INS_SAL - X86_INS_INVALID] = translate_sal_byhand;
      translate_functions[X86_INS_SAR - X86_INS_INVALID] = translate_sar_byhand;
      translate_functions[X86_INS_NOT - X86_INS_INVALID] = translate_not_byhand;
      translate_functions[X86_INS_NEG - X86_INS_INVALID] = translate_neg_byhand;
      translate_functions[X86_INS_MUL - X86_INS_INVALID] = translate_mul_byhand;
      translate_functions[X86_INS_DIV - X86_INS_INVALID] = translate_div_byhand;
    }
#endif

    bool translation_success = false;

#ifndef CONFIG_SOFTMMU
    if (ir1_opcode(ir1) == X86_INS_CALL && !ir1_is_indirect_call(ir1) &&
        ir1_addr_next(ir1) == ir1_target_addr(ir1)) {
        return translate_callnext(ir1);
    }
#endif

    // MOVSD means movsd(movs) or movsd(sse2) , diff opcode
    if (ir1_opcode(ir1) == X86_INS_MOVSD) {
        if (ir1->info->detail->x86.opcode[0] == 0xa5) {
            translation_success = translate_movs(ir1);
        } else if (ir1->info->detail->x86.opcode[0] == 0x0f) {
            translation_success = translate_movsd(ir1);
        } else {
            fprintf(stderr, "%s %s %d error : this ins not implemented\n",
                    __FILE__, __func__, __LINE__);
            ir1_opcode_dump(ir1);
            exit(-1);
        }
    } else {
        if (translate_functions[tr_func_idx] == NULL) {
            fprintf(stderr, "%s %s %d error : this ins %d not implemented\n",
                    __FILE__, __func__, __LINE__, tr_func_idx);
            ir1_opcode_dump(ir1);
            exit(-1);
        }
        translation_success = translate_functions[tr_func_idx](ir1); /* TODO */
    }

    /* 3. postprocess */
    ra_free_all_internal_temp();
    /* For now the line number of first member of struct translate_functions is 476, remember to
     * change it if the line number is changed.
     */
    lsassertm(translation_success, "tr_func_idx %d translate failed, the failed ins is at line %d",
                tr_func_idx, (tr_func_idx + idx_start));
    return translation_success;
}

void tr_dump_current_ir2(void)
{
    TRANSLATION_DATA *t = lsenv->tr_data;
    IR2_INST *pir2 = t->first_ir2;

    fprintf(stderr, "IR2 num = %d\n", lsenv->tr_data->ir2_inst_num_current);
    while (pir2) {
        ir2_dump(pir2);
        pir2 = ir2_next(pir2);
    }
}

static inline void tr_init_for_each_ir1_in_tb(IR1_INST *pir1, int nr, int index)
 {
    lsenv->tr_data->curr_ir1_inst = pir1;
#ifdef CONFIG_LATX_DEBUG
    la_append_ir2_opnda(LISA_X86_INST, ir1_addr(pir1));
#endif
#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_init_skipped_flags(pir1);
#endif
    if (index == nr - 1)
        tr_adjust_em();
}

bool tr_ir2_generate(struct TranslationBlock *tb)
{
    int i;
    TRANSLATION_DATA *t = lsenv->tr_data;
    int ir1_nr = tb->icount;

    if (option_dump) {
        fprintf(stderr, "[LATX] translation : generate IR2 from IR1.\n");
        fprintf(stderr, "IR1 num = %d\n", ir1_nr);
    }

    IR1_INST *pir1 = tb->_ir1_instructions;
    for (i = 0; i < ir1_nr; ++i) {
        tr_init_for_each_ir1_in_tb(pir1, ir1_nr, i);

        /*
         * handle segv scenario, store host pc to gen_insn_data and encode to a BYTE
         * at the end of TB translate cache.
         */
        tcg_ctx->gen_insn_data[i][0] = pir1->info->address;
        tcg_ctx->gen_insn_data[i][1] = 0;

        bool translation_success = ir1_translate(pir1);
        assert(translation_success && "ir1_translate fail");

        /*
         * gen_insn_end_off is used for store ir2 insn number.
         */
        tcg_ctx->gen_insn_end_off[i] = (lsenv->tr_data->real_ir2_inst_num)<<2;

        pir1++;
    }

    if (option_dump_ir1) {
        pir1 = tb->_ir1_instructions;
        for (i = 0; i < ir1_nr; ++i) {
             fprintf(stderr, "IR1[%d] ", i);
             ir1_dump(pir1);
            pir1++;
         }
    }

    if (option_dump_ir2) {
        fprintf(stderr, "IR2 num = %d\n", lsenv->tr_data->ir2_inst_num_current);
        for (i = 0; i < t->ir2_inst_num_current; ++i) {
            IR2_INST *pir2 = &t->ir2_inst_array[i];
            ir2_dump(pir2);
        }
    }

    if (!option_lsfpu) {
        etb_check_top_out(tb, lsenv->tr_data->curr_top);
    }

    return true;
}

static inline const void *qm_tb_get_code_cache(void *tb)
{
    struct TranslationBlock *ptb = (struct TranslationBlock *)tb;
    return ptb->tc.ptr;
}

int tr_translate_tb(struct TranslationBlock *tb)
{
    if (option_dump)
        fprintf(stderr, "[LATX] start translation.\n");

    /* some initialization */
    tr_init(tb);
    if (option_dump)
        fprintf(stderr, "tr_init OK. ready to translation.\n");

    /* generate ir2 from ir1 */
    bool translation_done = tr_ir2_generate(tb);

    int code_nr = 0;

    if (translation_done) {
        /* optimize ir2 */
        /*
         * FIXME: LA segv if below code enabled.
         */
        //tr_ir2_optimize();

        /* assemble ir2 to native code */
        code_nr = tr_ir2_assemble(qm_tb_get_code_cache(tb));
    }

    int code_size = code_nr * 4;
    counter_mips_tr += code_nr;

    /* finalize */
    tr_fini(translation_done);
    if (option_dump)
        fprintf(stderr, "tr_fini OK. translation done.\n");

    return code_size;
}

void tr_generate_exit_tb(IR1_INST *branch, int succ_id)
{
    ADDR target_addr;
    IR2_OPND succ_x86_addr_opnd;
    ADDR succ_x86_addr;
    unsigned long curr_ins_pos;
    IR1_OPCODE opcode = ir1_opcode(branch);
    /* Mapping to LA 2 ---> 17 */
    IR2_OPND mips_ret_opnd = ir2_opnd_new(IR2_OPND_IREG, 17); /* v0 */

    TranslationBlock *tb = lsenv->tr_data->curr_tb;

    /* if fpu stack mode is used, we don't rely on tb after linking */
    if (option_lsfpu && option_tb_link && !ir1_is_indirect_jmp(branch) && !ir1_is_indirect_call(branch) &&
        opcode != X86_INS_RET && opcode != X86_INS_RET && opcode != X86_INS_IRETD) {
        /* 1. set a label for native code linkage */
        IR2_OPND goto_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);

        /* remember where the j instruction position */
        /* reuse jmp_reset_offset[], they will be rewritten in label_dispose */
        /* at that time we can calculate exact offset for this position */
        la_append_ir2_opnd1(LISA_LABEL, goto_label_opnd);
        tb->jmp_reset_offset[succ_id] = goto_label_opnd._label_id;

        /* point to current j insn addr plus 8 by default, will resolve in */
        /* label_dispose */
        /*
         * FIXME: We have to add a useless nop here to make offset calculation
         * is correct.
         */
        la_append_ir2_opnda(LISA_B, 0);
        la_append_ir2_opnd2i(LISA_ANDI, zero_ir2_opnd, zero_ir2_opnd, 0);
    }

    IR2_OPND tb_ptr_opnd = ra_alloc_dbt_arg1();
    ADDR tb_addr = (ADDR)tb;
#ifndef N64
#warning need to implement la_append_ir2_opnd1i_em APIs.
    la_append_ir2_opnd1i(LISA_LU12I_W, tb_ptr_opnd, tb_addr >> 12);
    la_append_ir2_opnd2i(LISA_ORI, tb_ptr_opnd, tb_ptr_opnd, tb_addr >> 20);
    ir2_opnd_set_em(&tb_ptr_opnd, EM_X86_ADDRESS, 32);
#else
    if (cpu_get_guest_base() != 0) {
        /* should used from_addr here. but we need to ori with tb address
           later, to avoid assertional in translate_or, we keep it zero
           extend.
         */
        load_ireg_from_imm64(tb_ptr_opnd, tb_addr);
    } else {
        /* when guest == 0, tb_addr with have high 32bit not zero, to minimize
           generated code here, we load only low 32bit for current tb, and 
           rely on the fact that all tb share the same high32 bit
           Be sure to extend this tb address into full address before use it
         */
        load_ireg_from_imm32(tb_ptr_opnd, (uint32_t)tb_addr, ZERO_EXTENSION);
    }
#endif

    if (!option_lsfpu && option_tb_link && !ir1_is_indirect_jmp(branch) && !ir1_is_indirect_call(branch) &&
        opcode != X86_INS_RET && opcode != X86_INS_RET && opcode != X86_INS_IRETD) {
        /* 1. set a label for native code linkage */
        IR2_OPND goto_label_opnd = ir2_opnd_new_type(IR2_OPND_LABEL);

        /* remember where the j instruction position */
        /* reuse jmp_reset_offset[], they will be rewritten in label_dispose */
        /* at that time we can calculate exact offset for this position */
        la_append_ir2_opnd1(LISA_LABEL, goto_label_opnd);
        tb->jmp_reset_offset[succ_id] = goto_label_opnd._label_id;

        /* point to current j insn addr plus 8 by default, will resolve in */
        /* label_dispose */
        la_append_ir2_opnda(LISA_B, 0);
        /*
         * FIXME: We have to add a useless nop here to make offset calculation
         * is correct.
         */
        la_append_ir2_opnd2i(LISA_ANDI, zero_ir2_opnd, zero_ir2_opnd, 0);
    }

    switch (opcode) {
    case X86_INS_CALL:
        if(ir1_is_indirect_call(branch)){
            goto indirect_call;
        }
        target_addr = ir1_target_addr(branch);
        if (option_dump)
            fprintf(stderr, "[LATX] generate exit tb for CALL %lx\n",
                    target_addr);
        /* save next TB's eip in $25 */
        /* Mapping to LA 25->16*/
        load_ireg_from_imm32(ir2_opnd_new(IR2_OPND_IREG, 16), target_addr,
                             ZERO_EXTENSION);
        /* LA didn't has delay slot */
        if (option_tb_link) {
            la_append_ir2_opnd2i_em(LISA_ORI, mips_ret_opnd, tb_ptr_opnd, succ_id);
        } else {
            la_append_ir2_opnd2i_em(LISA_ORI, mips_ret_opnd, zero_ir2_opnd, succ_id);
        }
        /* jump to context_switch_native_to_bt */
        curr_ins_pos = (unsigned long)tb->tc.ptr + (lsenv->tr_data->real_ir2_inst_num << 2);
        la_append_ir2_opnda(LISA_B, (context_switch_native_to_bt - curr_ins_pos)>>2);
        break;
    case X86_INS_JMP:
        if(ir1_is_indirect_jmp(branch)){
            goto indirect_jmp;
        }
        succ_x86_addr_opnd = ra_alloc_dbt_arg2();
        succ_x86_addr = ir1_target_addr(branch);
        la_append_ir2_opnd1i(LISA_LU12I_W, succ_x86_addr_opnd, succ_x86_addr >> 12);
        la_append_ir2_opnd2i(LISA_ORI, succ_x86_addr_opnd, succ_x86_addr_opnd,
                          succ_x86_addr & 0xfff);
        ir2_opnd_set_em(&succ_x86_addr_opnd, EM_X86_ADDRESS, 32);
        /* LA didn't has delay slot */
        if (option_tb_link) {
            la_append_ir2_opnd2i_em(LISA_ORI, mips_ret_opnd, tb_ptr_opnd, succ_id);
        } else {
            la_append_ir2_opnd2i_em(LISA_ORI, mips_ret_opnd, zero_ir2_opnd, succ_id);
        }
        curr_ins_pos = (unsigned long)tb->tc.ptr + (lsenv->tr_data->real_ir2_inst_num << 2);
        la_append_ir2_opnda(LISA_B, (context_switch_native_to_bt - curr_ins_pos)>>2);
        break;
    // case X86_INS_JMPIN:  JUMPIN and CALLIN (part of jmp and call) are not standard i386 instruction 
    // case X86_INS_CALLIN:
    case X86_INS_RET:
    case X86_INS_IRET:
    case X86_INS_IRETD:
        //printf("cpu%d tb_addr = 0x%lx\n", current_cpu->cpu_index, tb_addr);
indirect_call :
indirect_jmp :
        if (option_tb_link) {
            /* store eip (in $25) into env */
            IR2_OPND succ_x86_addr_opnd = ra_alloc_dbt_arg2();

            la_append_ir2_opnd2i_em(LISA_STORE_ADDRX, succ_x86_addr_opnd,
                              env_ir2_opnd, lsenv_offset_of_eip(lsenv));
            /* tb->extra_tb.next_tb should be 0 so glue code know we are */
            /* indirect jmp append_ir2_opnd2i(mips_store_addr, */
            /* zero_ir2_opnd, env_ir2_opnd, */
            /*                   offsetof(CPUX86State, extra_tb) + */
            /*                   offsetof(struct ExtraBlock, next_tb)); */
            curr_ins_pos = (unsigned long)tb->tc.ptr + (lsenv->tr_data->real_ir2_inst_num << 2);
            la_append_ir2_opnda(LISA_B, (native_jmp_glue_2 - curr_ins_pos) >> 2);

        } else {
            /* LA didn't has delay slot */
            la_append_ir2_opnd2i_em(LISA_ORI, mips_ret_opnd, zero_ir2_opnd, succ_id);
            /* jump to context switch (native to bt) */
            //la_append_ir2_opnda(LISA_B, context_switch_native_to_bt);
            curr_ins_pos = (unsigned long)tb->tc.ptr + (lsenv->tr_data->real_ir2_inst_num << 2);
            la_append_ir2_opnda(LISA_B, (context_switch_native_to_bt - curr_ins_pos)>>2);
        }
        break;
    case X86_INS_JE:
    case X86_INS_JNE:
    case X86_INS_JS:
    case X86_INS_JNS:
    case X86_INS_JB:
    case X86_INS_JAE:
    case X86_INS_JO:
    case X86_INS_JNO:
    case X86_INS_JBE:
    case X86_INS_JA:
    case X86_INS_JP:
    case X86_INS_JNP:
    case X86_INS_JL:
    case X86_INS_JGE:
    case X86_INS_JLE:
    case X86_INS_JG:
    case X86_INS_JCXZ:
    case X86_INS_JECXZ:
    case X86_INS_JRCXZ:
    case X86_INS_LOOP:
    case X86_INS_LOOPE:
    case X86_INS_LOOPNE:
        succ_x86_addr_opnd = ra_alloc_dbt_arg2();
        succ_x86_addr =
            succ_id ? ir1_target_addr(branch) : ir1_addr_next(branch);
        la_append_ir2_opnd1i(LISA_LU12I_W, succ_x86_addr_opnd, succ_x86_addr >> 12);
        la_append_ir2_opnd2i(LISA_ORI, succ_x86_addr_opnd, succ_x86_addr_opnd,
                          succ_x86_addr & 0xfff);
        ir2_opnd_set_em(&succ_x86_addr_opnd, EM_X86_ADDRESS, 32);
        /* LA didn't has delay slot */
        if (option_tb_link) {
            la_append_ir2_opnd2i_em(LISA_ORI, mips_ret_opnd, tb_ptr_opnd, succ_id);
        } else {
            la_append_ir2_opnd2i_em(LISA_ORI, mips_ret_opnd, zero_ir2_opnd, succ_id);
        }
        curr_ins_pos = (unsigned long)tb->tc.ptr + (lsenv->tr_data->real_ir2_inst_num << 2);
        la_append_ir2_opnda(LISA_B, (context_switch_native_to_bt - curr_ins_pos)>>2);
        break;
    default:
        lsassertm(0, "not implement.\n");
    }
}

void tr_generate_goto_tb(void) /* TODO */
{
    la_append_ir2_opnd2i(LISA_ANDI, zero_ir2_opnd, zero_ir2_opnd, 0);
}

void generate_context_switch_bt_to_native(void *code_buf)
{
    /* 1. allocate space on the stack */
    la_append_ir2_opnd2i(LISA_ADDI_D, sp_ir2_opnd, sp_ir2_opnd, -256);
    /* 2. save callee-saved registers. s0-s7 ($16-$23), gp($28), and s8($30) */

    /* NOTE: leave some space in the dbt stack, in case that dbt functions */
    /* called by native code need to */
    /* store function arguments onto stack. */
    const int extra_space = 40;
    for (int i = 0; i <= 8; i++) {
        la_append_ir2_opnd2i(LISA_ST_D, create_ir2_opnd(IR2_OPND_GPR, i + 23),
                          sp_ir2_opnd, extra_space + (i << 3));
    }
    la_append_ir2_opnd2i(LISA_ST_D, fp_ir2_opnd, sp_ir2_opnd, extra_space + 72);
    /* 3. save ra($31) */
    la_append_ir2_opnd2i(LISA_ST_D, create_ir2_opnd(IR2_OPND_GPR, 1), sp_ir2_opnd,
                      extra_space + 80);
    /* native address of target block is passed in $a0, while it might be used
       as temp register later. So we just save it to $t9
     */
    IR2_OPND native_addr_opnd = create_ir2_opnd(IR2_OPND_GPR, 16);
    IR2_OPND a0_opnd = create_ir2_opnd(IR2_OPND_GPR, 4);
    la_append_ir2_opnd3(LISA_OR, native_addr_opnd, a0_opnd, zero_ir2_opnd);\
    ir2_opnd_set_em_mov(&native_addr_opnd, &a0_opnd);
    /* set env_ir2_opnd */
    IR2_OPND a1_opnd = create_ir2_opnd(IR2_OPND_GPR, 5);
    la_append_ir2_opnd3(LISA_OR, env_ir2_opnd, a1_opnd, zero_ir2_opnd);
    ir2_opnd_set_em_mov(&env_ir2_opnd, &a1_opnd);
    ir2_opnd_set_em(&env_ir2_opnd, EM_MIPS_ADDRESS, 32);

    /* 4. save dbt FCSR (#31) */
    IR2_OPND fcsr_value_opnd = ra_alloc_itemp();
    la_append_ir2_opnd2(LISA_MOVFCSR2GR, fcsr_value_opnd, fcsr_ir2_opnd);
    la_append_ir2_opnd2i(LISA_ST_D, fcsr_value_opnd, sp_ir2_opnd, extra_space + 88);

    la_append_ir2_opnd2i(LISA_LD_W, fcsr_value_opnd, env_ir2_opnd,
                          lsenv_offset_of_fcsr(lsenv));
    la_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr_ir2_opnd, fcsr_value_opnd);
    ra_free_temp(fcsr_value_opnd);

    ///* 5. set native code FCSR (#31) */
    //IR2_OPND temp_opnd = ra_alloc_itemp();
    ///* TODO:r */
    ///* if(!OPTIONS::check()){ */
    //if (1) {
    //    /* If the bit is set, the processor will directly set the result to
    //     * zero */
    //    /* if the result is underflow, so as not to raise exception. But this
    //     */
    //    /* will cause false positive error report when using rollback checker.
    //     */
    //    /* So, we don't use this optimization in rollback checker. */
    //    /* set FS flag */
    //    la_append_ir2_opnd2i(LISA_ADDU16I_D, temp_opnd, zero_ir2_opnd, 0);
    //    la_append_ir2_opnd3(LISA_OR, fcsr_value_opnd, fcsr_value_opnd, temp_opnd);
    //    la_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr_ir2_opnd, fcsr_value_opnd);
    //}

    /* 6. set s0 to 0xffffffff and set f3 = 32 */
    //load_ireg_from_imm32(temp_opnd, 32, SIGN_EXTENSION);
    //la_append_ir2_opnd2(LISA_MOVGR2FR_D, f32_ir2_opnd, temp_opnd);

    load_ireg_from_imm32(n1_ir2_opnd, -1, ZERO_EXTENSION);
    /* load env into s2 */

    /* load guest base into t2 */
    ADDR guest_base = cpu_get_guest_base();
    if (guest_base != 0) {
        IR2_OPND gbase_opnd = ra_alloc_guest_base();
        load_ireg_from_addr(gbase_opnd, guest_base);
    } else {
        /* store high 32bit of the code buffer address, we will use it to
           extend 32 bit tb into 64 bits
         */
        ADDR tb_high32 = ((ADDR)code_buf) >> 32;
        IR2_OPND gbase_opnd = ra_alloc_guest_base();
        load_ireg_from_imm32(gbase_opnd, tb_high32, UNKNOWN_EXTENSION);
        la_append_ir2_opnd2i(LISA_SLLI_D, gbase_opnd, gbase_opnd, 32);
    }
    if (option_dump)
        fprintf(stderr, "[LATX] guest base = %p\n", (void *)guest_base);

    /* 7. load x86 registers from env. top, eflags, and ss */
    tr_load_registers_from_env(0xff, 0xff, 0xff, 0xff, 0x10); 
    /*append_ir2_opnd2i(mips_sw, zero_ir2_opnd, env_ir2_opnd,
     * lsenv->offset_of_is_reg_latest());*/


    IR2_OPND eflags_opnd = ra_alloc_eflags();
    la_append_ir2_opnd2i(LISA_LD_W, eflags_opnd, env_ir2_opnd,
                      lsenv_offset_of_eflags(lsenv));
    ir2_opnd_set_em(&eflags_opnd, SIGN_EXTENSION, 32);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, eflags_opnd, 0x3f);
    la_append_ir2_opnd2i(LISA_ANDI, eflags_opnd, eflags_opnd, 0x400);
    //ir2_opnd_set_em_and(&dest, &src, ZERO_EXTENSION, uint16_eb(imm));

    /*IR2_OPND ss_opnd = ra_alloc_ss();*/
    /*append_ir2_opnd2i(mips_load_addr, ss_opnd, env_ir2_opnd,
     * lsenv->offset_of_ss());*/
    /* 8. jump to native code address (saved in t9) */
    la_append_ir2_opnd2i(LISA_JIRL, zero_ir2_opnd, native_addr_opnd, 0);
}

void generate_context_switch_native_to_bt(void)
{
    IR2_OPND mips_ret_opnd = ir2_opnd_new(IR2_OPND_IREG, 17); /* v0 */
    la_append_ir2_opnd3(LISA_OR, mips_ret_opnd, zero_ir2_opnd, zero_ir2_opnd);
    ir2_opnd_set_em_mov(&mips_ret_opnd, &zero_ir2_opnd);

    /* 1. store the last executed TB (int $24) into env */
    IR2_OPND tb_ptr_opnd = ra_alloc_dbt_arg1();
    lsassert(lsenv_offset_of_last_executed_tb(lsenv) >= -2048 &&
            lsenv_offset_of_last_executed_tb(lsenv) <= 2047);
    la_append_ir2_opnd2i(LISA_ST_D, tb_ptr_opnd, env_ir2_opnd,
                        lsenv_offset_of_last_executed_tb(lsenv));
    /* 2. store eip (in $25) into env */
    IR2_OPND eip_opnd = ra_alloc_dbt_arg2();
    lsassert(lsenv_offset_of_eip(lsenv) >= -2048 &&
            lsenv_offset_of_eip(lsenv) <= 2047);
    la_append_ir2_opnd2i(LISA_ST_W, eip_opnd, env_ir2_opnd,
                            lsenv_offset_of_eip(lsenv));

    /* 3. store x86 MMX and XMM registers to env */
    tr_save_registers_to_env(0xff, 0xff, 0xff, 0xff, 0x11);
    /*append_ir2_opnd2i(mips_sw, n1_ir2_opnd, env_ir2_opnd,
     * lsenv->offset_of_is_reg_latest());*/
    IR2_OPND eflags_opnd = ra_alloc_eflags();
    IR2_OPND eflags_temp = ra_alloc_itemp();
    la_append_ir2_opnd1i(LISA_X86MFFLAG, eflags_temp, 0x3f);
    la_append_ir2_opnd3(LISA_OR, eflags_opnd, eflags_opnd, eflags_temp);
    ra_free_temp(eflags_temp);
    lsassert(lsenv_offset_of_eflags(lsenv) >= -2048 &&
            lsenv_offset_of_eflags(lsenv) <= 2047);
    la_append_ir2_opnd2i(LISA_ST_W, eflags_opnd, env_ir2_opnd,
                          lsenv_offset_of_eflags(lsenv));
    const int extra_space = 40; 

    /* 4. restore dbt FCSR (#31) */
    IR2_OPND fcsr_value_opnd = ra_alloc_itemp();
	//save fcsr for native
    la_append_ir2_opnd2(LISA_MOVFCSR2GR, fcsr_value_opnd, fcsr_ir2_opnd);
    la_append_ir2_opnd2i(LISA_ST_W, fcsr_value_opnd, env_ir2_opnd,
                          lsenv_offset_of_fcsr(lsenv));

    /* See the note in generate_context_switch_bt_to_native */
    la_append_ir2_opnd2i(LISA_LD_D, fcsr_value_opnd, sp_ir2_opnd, extra_space + 88);
    la_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr_ir2_opnd, fcsr_value_opnd);
    ra_free_temp(fcsr_value_opnd);

    /* 5. restore ra */
    la_append_ir2_opnd2i(LISA_LD_D, ir2_opnd_new(IR2_OPND_IREG, 1), sp_ir2_opnd,
                        extra_space + 80);

    /* 6. restore callee-saved registers. s0-s7 ($16-$23), gp($28), and s8($30) */
    for (int i = 0; i <= 8; i++)
        la_append_ir2_opnd2i(LISA_LD_D, ir2_opnd_new(IR2_OPND_IREG, i + 23),
                            sp_ir2_opnd, extra_space  + (i << 3));

    la_append_ir2_opnd2i(LISA_LD_D, fp_ir2_opnd, sp_ir2_opnd, extra_space + 72);
    /* 7. restore sp */
    la_append_ir2_opnd2i(LISA_ADDI_D, sp_ir2_opnd, sp_ir2_opnd, 256);
    /* 8. return */
    la_append_ir2_opnd3(LISA_OR, a0_ir2_opnd, t5_ir2_opnd, zero_ir2_opnd);

    la_append_ir2_opnd2i(LISA_JIRL, zero_ir2_opnd,
                         ir2_opnd_new(IR2_OPND_IREG, 1), 0);
}

/* code_buf: start code address
 * n = 0: direct fall through jmp
 * n = 1: direct taken jmp
 * n = 2: indirect jmps
 */
static int generate_native_jmp_glue(void *code_buf, int n)
{
    int mips_num;
    /*
     * start/offset used for offset calculation.
     */
    int start = (lsenv->tr_data->real_ir2_inst_num << 2);
    int offset = 0;
    tr_init(NULL);

    if (option_tb_link) {
        /*
         * NOTE: if tb link option enabled, direct/indirect jump optimizations
         * will be enabled at the same time. In this scenario, there is no chance
         * to update eflags to ENV in prologue/epilogue. That will lead to unknown
         * behavior in wine program. So add eflags update operation in all jump
         * scenarios to keep eflags in ENV is the latest value.
         */
        IR2_OPND eflags_opnd = ra_alloc_eflags();
        IR2_OPND eflags_temp = ra_alloc_itemp();
        la_append_ir2_opnd1i(LISA_X86MFFLAG, eflags_temp, 0x3f);
        la_append_ir2_opnd3(LISA_OR, eflags_opnd, eflags_opnd, eflags_temp);
        ra_free_temp(eflags_temp);
        lsassert(lsenv_offset_of_eflags(lsenv) >= -2048 &&
                lsenv_offset_of_eflags(lsenv) <= 2047);
        la_append_ir2_opnd2i(LISA_ST_W, eflags_opnd, env_ir2_opnd,
                              lsenv_offset_of_eflags(lsenv));
        la_append_ir2_opnd2i(LISA_ANDI, eflags_opnd, eflags_opnd, 0x400);
    }

    IR2_OPND tb = ra_alloc_dbt_arg1();
    if (cpu_get_guest_base() == 0) {
        IR2_OPND tb_high32 = ra_alloc_guest_base();
        la_append_ir2_opnd3_em(LISA_OR, tb, tb, tb_high32);
        ir2_opnd_set_em(&tb, EM_MIPS_ADDRESS, 32);
    }
    IR2_OPND step_opnd = ra_alloc_dbt_arg1();
    IR2_OPND tmp_opnd = ra_alloc_dbt_arg2();
    /*
     * Due to LA A0 is parameter and return reg.
     * We have to set param0_opnd/ret_opnd as $r4.
     * For original design, param0_opnd store the _top_in data, we cannot
     * use $r4 only. _top_in will be stored in R17 to avoid r4 conflict.
     */
    IR2_OPND param0_opnd = ir2_opnd_new(IR2_OPND_IREG, 4);
    IR2_OPND ret_opnd = ir2_opnd_new(IR2_OPND_IREG, 4);
    IR2_OPND v0_opnd = ir2_opnd_new(IR2_OPND_IREG, 17);

    if (n == 0 || n == 1) {
        /* load tb->extra_tb.next_tb[n] into v0. */
        la_append_ir2_opnd2i_em(LISA_LOAD_ADDR, ret_opnd, tb,
                    offsetof(TranslationBlock, next_tb) + n * sizeof(void *));
    } else {

        IR2_OPND label_next_tb_exist = ir2_opnd_new_type(IR2_OPND_LABEL);
        IR2_OPND label_miss = ir2_opnd_new_type(IR2_OPND_LABEL);
        IR2_OPND label_hit  = ir2_opnd_new_type(IR2_OPND_LABEL);
        /*
         * If ra_alloc_itemp leveraged, A0 will be used as a temp reg whose
         * value will be destoried.
         * Request r7,r8,r9 directly to avoid r4 conflict.
         */

        IR2_OPND ibt_idx = ir2_opnd_new(IR2_OPND_IREG, 7);
        IR2_OPND addr_opnd = ir2_opnd_new(IR2_OPND_IREG, 8);
        if(option_ibtc) {
            ir2_opnd_set_em(&addr_opnd, EM_MIPS_ADDRESS, 32);
            la_append_ir2_opnd2i_em(LISA_ANDI, ibt_idx, tmp_opnd, IBTC_MASK);
            la_append_ir2_opnd2i_em(LISA_SLLI_D, ibt_idx, ibt_idx, 4);
            //load_ireg_from_addr(addr_opnd, (ADDR)ibtc_table);
            la_append_ir2_opnd2i(LISA_LD_D, addr_opnd, env_ir2_opnd,
                              lsenv_offset_of_ibtc_table(lsenv));
            la_append_ir2_opnd3_em(LISA_ADD_ADDR, addr_opnd, addr_opnd, ibt_idx);
            IR2_OPND eip_opnd = ir2_opnd_new(IR2_OPND_IREG, 9);
            la_append_ir2_opnd2i(LISA_LD_WU, eip_opnd, addr_opnd, 0);
            la_append_ir2_opnd3(LISA_BNE, eip_opnd, tmp_opnd, label_miss);
        } else {
            //tmp_opnd is set in indirect jmp translation
            la_append_ir2_opnd2i_em(LISA_SRLI_D, ret_opnd, tmp_opnd, TB_JMP_CACHE_BITS);
            la_append_ir2_opnd3_em(LISA_XOR, ret_opnd, tmp_opnd, ret_opnd);
            la_append_ir2_opnd2ii(LISA_BSTRPICK_D, ret_opnd, ret_opnd, TB_JMP_CACHE_BITS - 1, 0);
            la_append_ir2_opnd2i_em(LISA_SLLI_D, ret_opnd, ret_opnd, 3);
            la_append_ir2_opnd2i_em(LISA_LD_D, addr_opnd, env_ir2_opnd, lsenv_offset_of_tb_jmp_cache_ptr(lsenv));
            ir2_opnd_set_em(&ret_opnd, EM_MIPS_ADDRESS, 32); //ret_opnd is used as tb addressing
            la_append_ir2_opnd3(LISA_LDX_D, ret_opnd, ret_opnd, addr_opnd);
            la_append_ir2_opnd3(LISA_BEQ, ret_opnd, zero_ir2_opnd, label_miss);
            la_append_ir2_opnd2i(LISA_LD_D, addr_opnd, ret_opnd, 0); //PC
            la_append_ir2_opnd3(LISA_BEQ, addr_opnd, tmp_opnd, label_hit);
        }

        if(option_ibtc) {
            la_append_ir2_opnd2i_em(LISA_LOAD_ADDR, ret_opnd, addr_opnd, 8);
            la_append_ir2_opnd1(LISA_B, label_hit);
        }

        la_append_ir2_opnd1(LISA_LABEL, label_miss);

        /* lookup next_tb */
        /* we know that no fp will be touched in this helper, so we save */
        /* only gpr and vpr(guest base/$24) */
        if (option_shadow_stack) {
            tr_save_registers_to_env(GPR_USEDEF_TO_SAVE, 0, 0, 0, 0x13);
        } else {
            tr_save_registers_to_env(GPR_USEDEF_TO_SAVE, 0, 0, 0, 0x3);
        }

        /* calling helper_lookup_tb */
        /* this call migth trash a0, so put a0 load in delay slot */
        load_ireg_from_addr(tmp_opnd, (ADDR)helper_lookup_tb);

        /* arg0: env * */
        la_append_ir2_opnd2_em(LISA_MOV64, param0_opnd, env_ir2_opnd);
        la_append_ir2_opnd2i(LISA_JIRL, ir2_opnd_new(IR2_OPND_IREG, 1), tmp_opnd, 0);


        /* returned next_tb is in v0, which will not be destroyed by this load
         */
        if (option_shadow_stack) {
            tr_load_registers_from_env(GPR_USEDEF_TO_SAVE, 0, 0, 0, 0x13);
        } else {
            tr_load_registers_from_env(GPR_USEDEF_TO_SAVE, 0, 0, 0, 0x3);
        }

        la_append_ir2_opnd3(LISA_BNE, ret_opnd, zero_ir2_opnd,
                         label_next_tb_exist);

        /* if next_tb == NULL, jump to epilogue */

        /* clear v0 */
        la_append_ir2_opnd2i(LISA_ORI, ret_opnd, zero_ir2_opnd, 0);
        /* load back eip to t9 */
        la_append_ir2_opnd2i_em(LISA_LOAD_ADDR, tmp_opnd, env_ir2_opnd,
                          lsenv_offset_of_eip(lsenv));

        offset = (lsenv->tr_data->real_ir2_inst_num << 2) - start;
        la_append_ir2_opnda(LISA_B, (context_switch_native_to_bt_ret_0 - (ADDR)code_buf - offset) >> 2);

        /* else compare tb->top_out and next_tb->top_in */

        /* next_tb_exist: */
        la_append_ir2_opnd1(LISA_LABEL, label_next_tb_exist);
        if(option_ibtc) {
            la_append_ir2_opnd2i_em(LISA_LOAD_ADDR, tmp_opnd, env_ir2_opnd,
                              lsenv_offset_of_eip(lsenv));
            la_append_ir2_opnd2i_em(LISA_ANDI, ibt_idx, tmp_opnd, IBTC_MASK);
            la_append_ir2_opnd2i_em(LISA_SLLI_D, ibt_idx, ibt_idx, 4);
            //load_ireg_from_addr(addr_opnd, (ADDR)ibtc_table);
            la_append_ir2_opnd2i(LISA_LD_D, addr_opnd, env_ir2_opnd,
                              lsenv_offset_of_ibtc_table(lsenv));
            la_append_ir2_opnd3_em(LISA_ADD_ADDR, addr_opnd, addr_opnd, ibt_idx);
            ir2_opnd_set_em(&addr_opnd, EM_MIPS_ADDRESS, 32);
            la_append_ir2_opnd2i(LISA_ST_D, tmp_opnd, addr_opnd, 0);
            la_append_ir2_opnd2i(LISA_ST_D, ret_opnd, addr_opnd, 8);
        }
        la_append_ir2_opnd1(LISA_LABEL, label_hit);
        /* ensure is address. tr_save/load will destroy t8's status while keep its content */
        ir2_opnd_set_em(&tb, EM_MIPS_ADDRESS, 32);
        ir2_opnd_set_em(&ret_opnd, EM_MIPS_ADDRESS, 32);
    }

    if (!option_lsfpu) {
        /* tb->extra_tb._top_out */
        la_append_ir2_opnd2i_em(LISA_LD_BU, tmp_opnd, tb,
                                offsetof(TranslationBlock, _top_out));
        /* next_tb->extra_tb._top_in */
        la_append_ir2_opnd2i_em(LISA_LD_BU, v0_opnd, ret_opnd,
                                offsetof(TranslationBlock, _top_in));

        /* calculate top_bias, store rotate step in arg1 */
        la_append_ir2_opnd3_em(LISA_SUB_W, step_opnd, tmp_opnd, v0_opnd);

        /* for direct jmps, if no need to rotate, we will make direct link without
         * this glue */
        if (n == 2) {
            IR2_OPND label_rotate = ir2_opnd_new_type(IR2_OPND_LABEL);

            la_append_ir2_opnd3(LISA_BNE, step_opnd, zero_ir2_opnd, label_rotate);

            /* bias ==0, no need to ratate */
            /* fetch native address of next_tb to arg2 */
            la_append_ir2_opnd2i_em(
                    LISA_LOAD_ADDR, tmp_opnd, ret_opnd,
                    offsetof(TranslationBlock, tc) + offsetof(struct tb_tc, ptr));
            la_append_ir2_opnd2i(LISA_JIRL, zero_ir2_opnd, tmp_opnd, 0);

            la_append_ir2_opnd1(LISA_LABEL, label_rotate);
        }

        /* top_bias != 0, need to rotate, step is in arg1 */
        /* fetch native address of next_tb to arg2 */
        la_append_ir2_opnd2i_em(
                LISA_LOAD_ADDR, tmp_opnd, ret_opnd,
                offsetof(TranslationBlock, tc) + offsetof(struct tb_tc, ptr));
        offset = (lsenv->tr_data->real_ir2_inst_num << 2) - start;
        la_append_ir2_opnda(LISA_B, (native_rotate_fpu_by - (ADDR)code_buf - offset) >> 2);
    } else {
        /* fetch native address of next_tb to arg2 */
        la_append_ir2_opnd2i_em(
                LISA_LOAD_ADDR, tmp_opnd, ret_opnd,
                offsetof(TranslationBlock, tc) + offsetof(struct tb_tc, ptr));
        la_append_ir2_opnd2i(LISA_JIRL, zero_ir2_opnd, tmp_opnd, 0);
    }

    mips_num = tr_ir2_assemble(code_buf) + 1;
    tr_fini(false);

    return mips_num;
}

static int ss_generate_match_fail_native_code(void* code_buf){
    //we don't use shadow stack.
    return 0;
#if 0
    tr_init(NULL);
    int total_mips_num = 0;
    // ss_x86_addr is not equal to x86_addr, compare esp
    IR2_OPND ss_opnd = ra_alloc_ss();
    IR2_OPND ss_esp = ra_alloc_itemp();
    append_ir2_opnd2i(mips_load_addrx, ss_esp, ss_opnd, -(int)sizeof(SS_ITEM) + (int)offsetof(SS_ITEM, x86_esp));
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    IR2_OPND temp_result = ra_alloc_itemp();

    // if esp < ss_esp, that indicates ss has less item
    IR2_OPND label_exit_with_fail_match = ir2_opnd_new_type(IR2_OPND_LABEL);
    append_ir2_opnd3_not_nop(mips_bne, temp_result, zero_ir2_opnd, label_exit_with_fail_match);
    append_ir2_opnd3(mips_sltu, temp_result, esp_opnd, ss_esp);
    // x86_addr is not equal, but esp match, it indicates that the x86_addr has been changed
    append_ir2_opnd3_not_nop(mips_beq, esp_opnd, ss_esp, label_exit_with_fail_match);
    append_ir2_opnd2i(mips_addi_addr, ss_opnd, ss_opnd, -(int)sizeof(SS_ITEM));

    // pop till find, compare esp with ss_esp each time
    IR2_OPND label_pop_till_find = ir2_opnd_new_type(IR2_OPND_LABEL);
    append_ir2_opnd1(mips_label, label_pop_till_find);
    append_ir2_opnd2i(mips_load_addrx, ss_esp, ss_opnd, -(int)sizeof(SS_ITEM) + (int)offsetof(SS_ITEM, x86_esp));
    IR2_OPND label_esp_equal = ir2_opnd_new_type(IR2_OPND_LABEL);
    append_ir2_opnd3(mips_beq, esp_opnd, ss_esp, label_esp_equal);
    append_ir2_opnd3_not_nop(mips_bne, temp_result, zero_ir2_opnd, label_exit_with_fail_match);
    append_ir2_opnd3(mips_slt, temp_result, esp_opnd, ss_esp);
    append_ir2_opnd1_not_nop(mips_b, label_pop_till_find);
    append_ir2_opnd2i(mips_addi_addr, ss_opnd, ss_opnd, -(int)sizeof(SS_ITEM));
    ra_free_temp(temp_result);
    ra_free_temp(ss_esp);

    // esp equal, adjust esp with 24#reg value
    append_ir2_opnd1(mips_label, label_esp_equal);
    append_ir2_opnd2i(mips_addi_addr, ss_opnd, ss_opnd, -(int)sizeof(SS_ITEM));
    IR2_OPND etb_addr = ra_alloc_itemp();
    append_ir2_opnd2i(mips_load_addr, etb_addr, ss_opnd, (int)offsetof(SS_ITEM, return_tb));
    IR2_OPND ret_tb_addr = ra_alloc_itemp();
    append_ir2_opnd2i(mips_load_addr, ret_tb_addr, etb_addr, offsetof(ETB, tb));
    ra_free_temp(etb_addr);
    /* check if etb->tb is set */
    IR2_OPND label_have_no_native_code = ir2_opnd_new_type(IR2_OPND_LABEL);
    append_ir2_opnd3(mips_beq, ret_tb_addr, zero_ir2_opnd, label_have_no_native_code);
    IR2_OPND ss_x86_addr = ra_alloc_itemp();
    append_ir2_opnd2i(mips_load_addrx, ss_x86_addr, ret_tb_addr, (int)offsetof(TranslationBlock, pc));
    IR2_OPND x86_addr = ra_alloc_dbt_arg2();
    append_ir2_opnd3(mips_bne, ss_x86_addr, x86_addr, label_exit_with_fail_match);
    ra_free_temp(ss_x86_addr);

    // after several ss_pop, finally match successfully
    IR2_OPND esp_change_bytes = ra_alloc_mda();
    append_ir2_opnd3(mips_add_addrx, esp_opnd, esp_opnd, esp_change_bytes);
    IR2_OPND ret_mips_addr = ra_alloc_itemp();
    append_ir2_opnd2i(mips_load_addr, ret_mips_addr, ret_tb_addr,
        offsetof(TranslationBlock, tc) + offsetof(struct tb_tc, ptr));
    //before jump to the target tb, check whether top_out and top_in are equal
    //NOTE: last_executed_tb is already set before jumping to ss_match_fail_native
    IR2_OPND rotate_step = ra_alloc_dbt_arg1();
    IR2_OPND rotate_ret_addr = ra_alloc_dbt_arg2();
    IR2_OPND label_no_rotate = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND last_executed_tb = ra_alloc_dbt_arg1();
    IR2_OPND top_out = ra_alloc_itemp();
    IR2_OPND top_in = ra_alloc_itemp();
    append_ir2_opnd2i(mips_lbu, top_out, last_executed_tb,
        offsetof(TranslationBlock, extra_tb) + offsetof(ETB,_top_out));
    append_ir2_opnd2i(mips_lbu, top_in, ret_tb_addr,
        offsetof(TranslationBlock, extra_tb) + offsetof(ETB,_top_in));
    append_ir2_opnd3(mips_beq, top_in, top_out, label_no_rotate);
    //top_in != top_out, rotate fpu
    append_ir2_opnd3(mips_subu, rotate_step, top_out, top_in);
    append_ir2_opnda_not_nop(mips_j, native_rotate_fpu_by);
    append_ir2_opnd2(mips_mov64, rotate_ret_addr, ret_mips_addr);
    ra_free_temp(top_in);
    ra_free_temp(top_out);

    //top_in == top_out, directly go to next tb
    append_ir2_opnd1(mips_label, label_no_rotate);
    append_ir2_opnd1(mips_jr, ret_mips_addr);
    ra_free_temp(ret_tb_addr);
    ra_free_temp(ret_mips_addr);

    // finally match failed: adjust esp, load last_execut_tb
    append_ir2_opnd1(mips_label, label_exit_with_fail_match);
    append_ir2_opnd3(mips_add_addrx, esp_opnd, esp_opnd, esp_change_bytes);
    append_ir2_opnd1(mips_label, label_have_no_native_code);
    append_ir2_opnda(mips_j, context_switch_native_to_bt_ret_0);
    //IR2_OPND indirect_lookup_code_addr = ra_alloc_itemp();
    //load_ireg_from_addr(indirect_lookup_code_addr, tb_look_up_native);
    //append_ir2(mips_jr, indirect_lookup_code_addr);
    //ra_free_temp(indirect_lookup_code_addr);

    tr_fini(false);
    total_mips_num = tr_ir2_assemble(code_buf) + 1;

    return total_mips_num;
#endif
}

/* note: native_rotate_fpu_by rotate data between mapped fp registers instead
 * of the in memory env->fpregs
 */
int generate_native_rotate_fpu_by(void *code_buf_addr)
{
    void *code_buf = code_buf_addr;
    int mips_num = 0;
    int total_mips_num = 0;

    static ADDR rotate_by_step_addr[15]; /* rotate -7 ~ 7 */
    ADDR *rotate_by_step_0_addr = rotate_by_step_addr + 7;

    /* 1. generate the rotation code for step 0 */
    rotate_by_step_0_addr[0] = 0;

    /* 2. generate the rotation code for step 1-7 */
    for (int step = 1; step <= 7; ++step) {
        tr_init(NULL);
        /* 2.1 load top_bias early. It will be modified later */
        IR2_OPND top_bias = ra_alloc_itemp();
        lsassert(lsenv_offset_of_top_bias(lsenv) >= -2048 &&
                 lsenv_offset_of_top_bias(lsenv) <= 2047);
        la_append_ir2_opnd2i(LISA_LD_W, top_bias, env_ir2_opnd,
                          lsenv_offset_of_top_bias(lsenv));
        /* 2.2 prepare for the rotation */
        IR2_OPND fpr[8];
        for (int i = 0; i < 8; ++i)
            fpr[i] = ra_alloc_st(i);

        /* 2.3 rotate! */
        IR2_OPND spilled_data = ra_alloc_ftemp();
        int spilled_index = 0;
        int number_of_moved_fpr = 0;
        while (number_of_moved_fpr < 8) {
            /* 2.3.1 spill out a register */
            la_append_ir2_opnd2(LISA_FMOV_D, spilled_data, fpr[spilled_index]);

            /* 2.3.2 rotate, until moving from the spilled register */
            int target_index = spilled_index;
            int source_index = (target_index + step) & 7;
            while (source_index != spilled_index) {
                la_append_ir2_opnd2(LISA_FMOV_D, fpr[target_index],
                                 fpr[source_index]);
                number_of_moved_fpr++;
                target_index = source_index;
                source_index = (target_index + step) & 7;
            };
            /* 2.3.3 move from the spilled data */
            la_append_ir2_opnd2(LISA_FMOV_D, fpr[target_index], spilled_data);
            number_of_moved_fpr++;
            /* 2.3.4 when step is 2, 4, or 6, rotate from the next index; */
            spilled_index++;
        }
        /* 2.4 adjust the top_bias */
        la_append_ir2_opnd2i(LISA_ADDI_W, top_bias, top_bias, step);
        la_append_ir2_opnd2ii(LISA_BSTRPICK_D, top_bias, top_bias, 2, 0);
        IR2_OPND target_native_code_addr = ra_alloc_dbt_arg2();
        lsassert(lsenv_offset_of_top_bias(lsenv) >= -2048 &&
                 lsenv_offset_of_top_bias(lsenv) <= 2047);
        la_append_ir2_opnd2i(LISA_ST_W, top_bias, env_ir2_opnd,
                          lsenv_offset_of_top_bias(lsenv));
        ra_free_temp(top_bias);
        la_append_ir2_opnd2i(LISA_JIRL, zero_ir2_opnd, target_native_code_addr, 0);
        mips_num = 0;
        rotate_by_step_0_addr[step - 8] = rotate_by_step_0_addr[step] =
            (ADDR)code_buf;
        mips_num = tr_ir2_assemble((void *)rotate_by_step_0_addr[step - 8]) + 1;
        tr_fini(false);

        if (option_dump)
            fprintf(stderr,
                    "[fpu rotate] rotate step(%d,%d) at %p, size = %d\n", step,
                    step - 8, code_buf, mips_num);

        code_buf += mips_num * 4;
        total_mips_num += mips_num;
    }

    /* 3. generate dispatch code. two arguments: rotation step and target
     * native */
    /* address */
    tr_init(NULL);

    IR2_OPND rotation_code_addr = ra_alloc_itemp();
    load_ireg_from_addr(rotation_code_addr, (ADDR)(rotate_by_step_0_addr));
    IR2_OPND rotation_step = ra_alloc_dbt_arg1();
    la_append_ir2_opnd2i(LISA_SLLI_W, rotation_step, rotation_step, 3);

    la_append_ir2_opnd3_em(LISA_ADD_ADDR, rotation_code_addr, rotation_code_addr,
                     rotation_step);
    la_append_ir2_opnd2i_em(LISA_LOAD_ADDR, rotation_code_addr, rotation_code_addr,
                      0);
    la_append_ir2_opnd2i(LISA_JIRL, zero_ir2_opnd, rotation_code_addr, 0);
    ra_free_temp(rotation_code_addr);

    mips_num = 0;
    native_rotate_fpu_by = (ADDR)code_buf;
    mips_num = tr_ir2_assemble((void *)native_rotate_fpu_by) + 1;
    tr_fini(false);

    if (option_dump)
        fprintf(stderr, "[fpu rotate] rotate dispatch at %p. size = %d\n",
                code_buf, mips_num);

    total_mips_num += mips_num;
    code_buf += mips_num * 4;

    /* generate native_jmp_glue_0/1/2
     * args pass in: $24: tb, $25: eip(which is also stored in env->eip),
     */
    native_jmp_glue_0 = (ADDR)code_buf;
    mips_num = generate_native_jmp_glue(code_buf, 0);
    total_mips_num += mips_num;
    code_buf += mips_num * 4;

    native_jmp_glue_1 = (ADDR)code_buf;
    mips_num = generate_native_jmp_glue(code_buf, 1);
    total_mips_num += mips_num;
    code_buf += mips_num * 4;

    native_jmp_glue_2 = (ADDR)code_buf;
    mips_num = generate_native_jmp_glue(code_buf, 2);
    total_mips_num += mips_num;
    code_buf += mips_num * 4;

    IR2_OPND a0_opnd = ir2_opnd_new(IR2_OPND_IREG, 4);
    IR2_OPND v0_opnd = ir2_opnd_new(IR2_OPND_IREG, 17);
    IR2_OPND ra_opnd = ir2_opnd_new(IR2_OPND_IREG, 1);

    /* fpu_get_top() */
    fpu_get_top = (ADDR)code_buf;
    tr_init(NULL);

    la_append_ir2_opnd1(LISA_X86MFTOP, v0_opnd);
    la_append_ir2_opnd2i(LISA_JIRL, zero_ir2_opnd, ra_opnd, 0);

    mips_num = tr_ir2_assemble(code_buf) + 1;
    total_mips_num += mips_num;
    code_buf += mips_num * 4;
    tr_fini(false);

    /* fpu_inc_top() */
    fpu_inc_top = (ADDR)code_buf;
    tr_init(NULL);

    la_append_ir2_opnd0(LISA_X86INCTOP);
    la_append_ir2_opnd1(LISA_X86MFTOP, v0_opnd);
    la_append_ir2_opnd2i(LISA_JIRL, zero_ir2_opnd, ra_opnd, 0);


    mips_num = tr_ir2_assemble(code_buf) + 1;
    total_mips_num += mips_num;
    code_buf += mips_num * 4;
    tr_fini(false);

    /* fpu_dec_top() */
    fpu_dec_top = (ADDR)code_buf;
    tr_init(NULL);

    la_append_ir2_opnd0(LISA_X86DECTOP);
    la_append_ir2_opnd1(LISA_X86MFTOP, v0_opnd);
    la_append_ir2_opnd2i(LISA_JIRL, zero_ir2_opnd, ra_opnd, 0);

    mips_num = tr_ir2_assemble(code_buf) + 1;
    total_mips_num += mips_num;
    code_buf += mips_num * 4;
    tr_fini(false);

    /* fpu_enable_top() */
    fpu_enable_top = (ADDR)code_buf;
    tr_init(NULL);

    la_append_ir2_opnd0(LISA_X86SETTM);
    ///////Maybe, there are some errors.
    la_append_ir2_opnd2(LISA_MOVFCSR2GR, v0_opnd, fcsr_ir2_opnd);
    la_append_ir2_opnd2i(LISA_JIRL, zero_ir2_opnd, ra_opnd, 0);

    mips_num = tr_ir2_assemble(code_buf) + 1;
    total_mips_num += mips_num;
    code_buf += mips_num * 4;
    tr_fini(false);

    /* fpu_disable_top() */
    fpu_disable_top = (ADDR)code_buf;
    tr_init(NULL);

    la_append_ir2_opnd0(LISA_X86CLRTM);
    la_append_ir2_opnd1(LISA_X86MFTOP, v0_opnd);
    la_append_ir2_opnd2i(LISA_JIRL, zero_ir2_opnd, ra_opnd, 0);

    mips_num = tr_ir2_assemble(code_buf) + 1;
    total_mips_num += mips_num;
    code_buf += mips_num * 4;
    tr_fini(false);

    /* fpu_set_top(int top) */
    fpu_set_top = (ADDR)code_buf;
    tr_init(NULL);

    IR2_OPND label_loop = ir2_opnd_new_type(IR2_OPND_LABEL);

    la_append_ir2_opndi(LISA_X86MTTOP, 0);
    la_append_ir2_opnd1(LISA_LABEL, label_loop);
    la_append_ir2_opnd0(LISA_X86INCTOP);
    la_append_ir2_opnd2i(LISA_ADDI_W, a0_opnd, a0_opnd, -1);
    la_append_ir2_opnd3(LISA_BNE, a0_opnd, zero_ir2_opnd, label_loop);

    /* one step back */
    la_append_ir2_opnd0(LISA_X86DECTOP);
    la_append_ir2_opnd1(LISA_X86MFTOP, v0_opnd);
    la_append_ir2_opnd2i(LISA_JIRL, zero_ir2_opnd, ra_opnd, 0);

    mips_num = tr_ir2_assemble(code_buf) + 1;
    total_mips_num += mips_num;
    code_buf += mips_num * 4;
    tr_fini(false);

    if (option_dump)
        fprintf(stderr, "[fpu rotate] native jump glue at %p. size = %d\n",
                code_buf, mips_num);
    ss_match_fail_native = (ADDR)code_buf;

    mips_num = 0;
    mips_num = ss_generate_match_fail_native_code(code_buf);
    total_mips_num += mips_num;
    code_buf += mips_num * 4;

    return total_mips_num;
}

/* we have no inst to mov from gpr to top, so we have to be silly */
void tr_load_top_from_env(void)
{
    if (!option_lsfpu) return;

    IR2_OPND top_opnd = ra_alloc_itemp();
    IR2_OPND label_exit;
    int i;
    label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);

    int offset = lsenv_offset_of_top(lsenv);
    lsassert(offset <= 0x7ff);
    la_append_ir2_opnd2i_em(LISA_LD_H, top_opnd, env_ir2_opnd, offset);

    for (i = 0; i < 8; i++) {
        la_append_ir2_opndi(LISA_X86MTTOP, i);
        la_append_ir2_opnd3(LISA_BEQ, top_opnd, zero_ir2_opnd, label_exit);
        la_append_ir2_opnd2i_em(LISA_ADDI_W, top_opnd, top_opnd, -1);
    }
    ra_free_temp(top_opnd);
    la_append_ir2_opnd1(LISA_LABEL, label_exit);
}

void tr_gen_top_mode_init(void)
{
    if (option_lsfpu) {
        la_append_ir2_opndi(LISA_X86MTTOP, 0);
        la_append_ir2_opnd0(LISA_X86SETTM);
    }
}


/* check fpu rotate and patch the native jump address */
void latx_tb_set_jmp_target(TranslationBlock *tb, int n,
                                   TranslationBlock *next_tb)
{
#ifdef CONFIG_SOFTMMU
    if (option_lsfpu || option_soft_fpu || tb->_top_out == next_tb->_top_in) {
#else
    if (option_lsfpu || tb->_top_out == next_tb->_top_in) {
#endif
        tb->next_tb[n] = next_tb;
        tb_set_jmp_target(tb, n, (uintptr_t)next_tb->tc.ptr);
    } else {
        lsassert(next_tb != NULL);
        tb->next_tb[n] = next_tb;
        if (n == 0)
            tb_set_jmp_target(tb, 0, native_jmp_glue_0);
        else
            tb_set_jmp_target(tb, 1, native_jmp_glue_1);
    }
}

void rotate_fpu_by(int step)
{
    assert(step >= -7 && step <= 7);
    assert(step != 0);
    lsenv_set_top(lsenv, (lsenv_get_top(lsenv) - step) & 7);
    switch (step) {
    case 1:
    case -7: {
        FPReg ftemp0;
        ftemp0 = lsenv_get_fpregs(lsenv, 0);
        lsenv_set_fpregs(lsenv, 0, lsenv_get_fpregs(lsenv, 1));
        lsenv_set_fpregs(lsenv, 1, lsenv_get_fpregs(lsenv, 2));
        lsenv_set_fpregs(lsenv, 2, lsenv_get_fpregs(lsenv, 3));
        lsenv_set_fpregs(lsenv, 3, lsenv_get_fpregs(lsenv, 4));
        lsenv_set_fpregs(lsenv, 4, lsenv_get_fpregs(lsenv, 5));
        lsenv_set_fpregs(lsenv, 5, lsenv_get_fpregs(lsenv, 6));
        lsenv_set_fpregs(lsenv, 6, lsenv_get_fpregs(lsenv, 7));
        lsenv_set_fpregs(lsenv, 7, ftemp0);
        lsenv_set_top_bias(lsenv, (lsenv_get_top_bias(lsenv) + 1) & 7);
    } break;
    case 2:
    case -6: {
        FPReg ftemp0, ftemp1;
        ftemp0 = lsenv_get_fpregs(lsenv, 0);
        ftemp1 = lsenv_get_fpregs(lsenv, 1);
        lsenv_set_fpregs(lsenv, 0, lsenv_get_fpregs(lsenv, 2));
        lsenv_set_fpregs(lsenv, 1, lsenv_get_fpregs(lsenv, 3));
        lsenv_set_fpregs(lsenv, 2, lsenv_get_fpregs(lsenv, 4));
        lsenv_set_fpregs(lsenv, 3, lsenv_get_fpregs(lsenv, 5));
        lsenv_set_fpregs(lsenv, 4, lsenv_get_fpregs(lsenv, 6));
        lsenv_set_fpregs(lsenv, 5, lsenv_get_fpregs(lsenv, 7));
        lsenv_set_fpregs(lsenv, 6, ftemp0);
        lsenv_set_fpregs(lsenv, 7, ftemp1);
        lsenv_set_top_bias(lsenv, (lsenv_get_top_bias(lsenv) + 2) & 7);
    } break;
    case 3:
    case -5: {
        FPReg ftemp0, ftemp1, ftemp2;
        ftemp0 = lsenv_get_fpregs(lsenv, 0);
        ftemp1 = lsenv_get_fpregs(lsenv, 1);
        ftemp2 = lsenv_get_fpregs(lsenv, 2);
        lsenv_set_fpregs(lsenv, 0, lsenv_get_fpregs(lsenv, 3));
        lsenv_set_fpregs(lsenv, 1, lsenv_get_fpregs(lsenv, 4));
        lsenv_set_fpregs(lsenv, 2, lsenv_get_fpregs(lsenv, 5));
        lsenv_set_fpregs(lsenv, 3, lsenv_get_fpregs(lsenv, 6));
        lsenv_set_fpregs(lsenv, 4, lsenv_get_fpregs(lsenv, 7));
        lsenv_set_fpregs(lsenv, 5, ftemp0);
        lsenv_set_fpregs(lsenv, 6, ftemp1);
        lsenv_set_fpregs(lsenv, 7, ftemp2);
        lsenv_set_top_bias(lsenv, (lsenv_get_top_bias(lsenv) + 3) & 7);
    } break;
    case 4:
    case -4: {
        FPReg ftemp0, ftemp1, ftemp2, ftemp3;
        ftemp0 = lsenv_get_fpregs(lsenv, 0);
        ftemp1 = lsenv_get_fpregs(lsenv, 1);
        ftemp2 = lsenv_get_fpregs(lsenv, 2);
        ftemp3 = lsenv_get_fpregs(lsenv, 3);
        lsenv_set_fpregs(lsenv, 0, lsenv_get_fpregs(lsenv, 4));
        lsenv_set_fpregs(lsenv, 1, lsenv_get_fpregs(lsenv, 5));
        lsenv_set_fpregs(lsenv, 2, lsenv_get_fpregs(lsenv, 6));
        lsenv_set_fpregs(lsenv, 3, lsenv_get_fpregs(lsenv, 7));
        lsenv_set_fpregs(lsenv, 4, ftemp0);
        lsenv_set_fpregs(lsenv, 5, ftemp1);
        lsenv_set_fpregs(lsenv, 6, ftemp2);
        lsenv_set_fpregs(lsenv, 7, ftemp3);
        lsenv_set_top_bias(lsenv, (lsenv_get_top_bias(lsenv) + 4) & 7);
    } break;
    case 5:
    case -3: {
        FPReg ftemp0, ftemp1, ftemp2, ftemp3, ftemp4;
        ftemp0 = lsenv_get_fpregs(lsenv, 0);
        ftemp1 = lsenv_get_fpregs(lsenv, 1);
        ftemp2 = lsenv_get_fpregs(lsenv, 2);
        ftemp3 = lsenv_get_fpregs(lsenv, 3);
        ftemp4 = lsenv_get_fpregs(lsenv, 4);
        lsenv_set_fpregs(lsenv, 0, lsenv_get_fpregs(lsenv, 5));
        lsenv_set_fpregs(lsenv, 1, lsenv_get_fpregs(lsenv, 6));
        lsenv_set_fpregs(lsenv, 2, lsenv_get_fpregs(lsenv, 7));
        lsenv_set_fpregs(lsenv, 3, ftemp0);
        lsenv_set_fpregs(lsenv, 4, ftemp1);
        lsenv_set_fpregs(lsenv, 5, ftemp2);
        lsenv_set_fpregs(lsenv, 6, ftemp3);
        lsenv_set_fpregs(lsenv, 7, ftemp4);
        lsenv_set_top_bias(lsenv, (lsenv_get_top_bias(lsenv) + 5) & 7);
    } break;
    case 6:
    case -2: {
        FPReg ftemp0, ftemp1, ftemp2, ftemp3, ftemp4, ftemp5;
        ftemp0 = lsenv_get_fpregs(lsenv, 0);
        ftemp1 = lsenv_get_fpregs(lsenv, 1);
        ftemp2 = lsenv_get_fpregs(lsenv, 2);
        ftemp3 = lsenv_get_fpregs(lsenv, 3);
        ftemp4 = lsenv_get_fpregs(lsenv, 4);
        ftemp5 = lsenv_get_fpregs(lsenv, 5);
        lsenv_set_fpregs(lsenv, 0, lsenv_get_fpregs(lsenv, 6));
        lsenv_set_fpregs(lsenv, 1, lsenv_get_fpregs(lsenv, 7));
        lsenv_set_fpregs(lsenv, 2, ftemp0);
        lsenv_set_fpregs(lsenv, 3, ftemp1);
        lsenv_set_fpregs(lsenv, 4, ftemp2);
        lsenv_set_fpregs(lsenv, 5, ftemp3);
        lsenv_set_fpregs(lsenv, 6, ftemp4);
        lsenv_set_fpregs(lsenv, 7, ftemp5);
        lsenv_set_top_bias(lsenv, (lsenv_get_top_bias(lsenv) + 6) & 7);
    } break;
    case 7:
    case -1: {
        FPReg ftemp0, ftemp1, ftemp2, ftemp3, ftemp4, ftemp5, ftemp6;
        ftemp0 = lsenv_get_fpregs(lsenv, 0);
        ftemp1 = lsenv_get_fpregs(lsenv, 1);
        ftemp2 = lsenv_get_fpregs(lsenv, 2);
        ftemp3 = lsenv_get_fpregs(lsenv, 3);
        ftemp4 = lsenv_get_fpregs(lsenv, 4);
        ftemp5 = lsenv_get_fpregs(lsenv, 5);
        ftemp6 = lsenv_get_fpregs(lsenv, 6);
        lsenv_set_fpregs(lsenv, 0, lsenv_get_fpregs(lsenv, 7));
        lsenv_set_fpregs(lsenv, 1, ftemp0);
        lsenv_set_fpregs(lsenv, 2, ftemp1);
        lsenv_set_fpregs(lsenv, 3, ftemp2);
        lsenv_set_fpregs(lsenv, 4, ftemp3);
        lsenv_set_fpregs(lsenv, 5, ftemp4);
        lsenv_set_fpregs(lsenv, 6, ftemp5);
        lsenv_set_fpregs(lsenv, 7, ftemp6);
        lsenv_set_top_bias(lsenv, (lsenv_get_top_bias(lsenv) + 7) & 7);
    } break;
    }
}

void rotate_fpu_to_bias(int bias)
{
    int step = bias - lsenv_get_top_bias(lsenv);
    rotate_fpu_by(step);
}

void rotate_fpu_to_top(int top)
{
    int step = lsenv_get_top(lsenv) - top;
    rotate_fpu_by(step);
}

void tr_fpu_push(void) { tr_fpu_dec(); }
void tr_fpu_pop(void) { tr_fpu_inc(); }

void tr_fpu_inc(void)
{
    if (option_lsfpu) {
        la_append_ir2_opnd0(LISA_X86INCTOP);
    } else {
        lsenv->tr_data->curr_top++;
        lsenv->tr_data->curr_top &= 7;
    }
}

void tr_fpu_dec(void)
{
    if (option_lsfpu) {
        la_append_ir2_opnd0(LISA_X86DECTOP);
    } else {
        lsenv->tr_data->curr_top--;
        lsenv->tr_data->curr_top &= 7;
    }
}

void tr_fpu_enable_top_mode(void)
{
    if (option_lsfpu) {
        la_append_ir2_opnd0(LISA_X86SETTM);
    }
}

void tr_fpu_disable_top_mode(void)
{
    if (option_lsfpu) {
        la_append_ir2_opnd0(LISA_X86CLRTM);
    }
}

void tr_save_fcsr_to_env(void)
{
    IR2_OPND fcsr_value_opnd = ra_alloc_itemp();
    la_append_ir2_opnd2(LISA_MOVFCSR2GR, fcsr_value_opnd, fcsr_ir2_opnd);
    la_append_ir2_opnd2i(LISA_ST_W, fcsr_value_opnd, env_ir2_opnd,
                          lsenv_offset_of_fcsr(lsenv));
    ra_free_temp(fcsr_value_opnd);
}

void tr_load_fcsr_from_env(void)
{
    IR2_OPND saved_fcsr_value_opnd = ra_alloc_itemp();
    la_append_ir2_opnd2i(LISA_LD_W, saved_fcsr_value_opnd, env_ir2_opnd,
                          lsenv_offset_of_fcsr(lsenv));
    la_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr_ir2_opnd, saved_fcsr_value_opnd);
    ra_free_temp(saved_fcsr_value_opnd);
}

void tr_save_registers_to_env(uint8 gpr_to_save, uint8 fpr_to_save,
                              uint8 xmm_lo_to_save, uint8 xmm_hi_to_save,
                              uint8 vreg_to_save)
{
    int i = 0;
    /* 1. GPR */
    for (i = 0; i < 8; ++i) {
        if (BITS_ARE_SET(gpr_to_save, 1 << i)) {
            IR2_OPND gpr_opnd = ra_alloc_gpr(i);
            la_append_ir2_opnd2i(LISA_ST_W, gpr_opnd, env_ir2_opnd,
                              lsenv_offset_of_gpr(lsenv, i));
        }
    }

    /* 2. FPR (MMX) */
    if (fpr_to_save != 0) {
        IR2_OPND top_opnd = ra_alloc_itemp();
        if (!option_lsfpu) {
            load_ireg_from_imm32(top_opnd, lsenv->tr_data->curr_top,
                    UNKNOWN_EXTENSION);
        } else {
            la_append_ir2_opnd1(LISA_X86MFTOP, top_opnd);
        }
        la_append_ir2_opnd2i(LISA_ST_W, top_opnd, env_ir2_opnd,
                              lsenv_offset_of_top(lsenv));
        ra_free_temp(top_opnd);

        /* disable fpu top mode */
        tr_fpu_disable_top_mode();
    }

    for (int i = 0; i < 8; i++) {
        if (BITS_ARE_SET(fpr_to_save, 1 << i)) {
            IR2_OPND mmx_opnd = ra_alloc_mmx(i);
            la_append_ir2_opnd2i(LISA_FST_D, mmx_opnd, env_ir2_opnd,
                                lsenv_offset_of_mmx(lsenv, i));
        }
    }

    /* 3. XMM */
    for (int i = 0; i < 8; i++) {
        if (BITS_ARE_SET(xmm_lo_to_save, 1 << i)){
            la_append_ir2_opnd2i(LISA_VST, ra_alloc_xmm(i),
                                 env_ir2_opnd, lsenv_offset_of_xmm(lsenv, i));
        }
    }


    /* 4. virtual registers */
    for (i = 0; i < 8; ++i) {
        if (BITS_ARE_SET(vreg_to_save, 1 << i)) {
            IR2_OPND vreg_opnd = ra_alloc_vreg(i);
            la_append_ir2_opnd2i(LISA_ST_D, vreg_opnd, env_ir2_opnd,
                              lsenv_offset_of_vreg(lsenv, i));
        }
    }
}

void tr_load_registers_from_env(uint8 gpr_to_load, uint8 fpr_to_load,
                                uint8 xmm_lo_to_load, uint8 xmm_hi_to_load,
                                uint8 vreg_to_load)
{
    int i = 0;

    /* 4. virtual registers */
    for (i = 0; i < 8; ++i) {
        if (BITS_ARE_SET(vreg_to_load, 1 << i)) {
            IR2_OPND vreg_opnd = ra_alloc_vreg(i);
            la_append_ir2_opnd2i(LISA_LD_D, vreg_opnd, env_ir2_opnd,
                              lsenv_offset_of_vreg(lsenv, i));
        }
    }

    /* 3. XMM */
    for (i = 0; i < 8; i++) {
        if (BITS_ARE_SET(xmm_lo_to_load, 1 << i)){
            la_append_ir2_opnd2i(LISA_VLD, ra_alloc_xmm(i),env_ir2_opnd, lsenv_offset_of_xmm(lsenv, i));
        }
    }

    /* 2. FPR (MMX) */
    for (i = 0; i < 8; i++) {
        if (BITS_ARE_SET(fpr_to_load, 1 << i)) {
            IR2_OPND mmx_opnd = ra_alloc_mmx(i);
            la_append_ir2_opnd2i(LISA_FLD_D, mmx_opnd, env_ir2_opnd,
                              lsenv_offset_of_mmx(lsenv, i));
        }
    }

    if (fpr_to_load && option_lsfpu) {
        tr_load_top_from_env();
        /* this can fetch only top of translation time,
           not runtime
        append_ir2_opndi(mips_mttop, lsenv_get_top(lsenv));
        */
        /* enable fpu top mode */
        tr_fpu_enable_top_mode();
    }


    for (i = 0; i < 8; ++i) {
        if (BITS_ARE_SET(gpr_to_load, 1 << i)) {
            IR2_OPND gpr_opnd = ra_alloc_gpr(i);
#ifdef N64 /* validate address */
            if (ir2_opnd_default_em(&gpr_opnd) != SIGN_EXTENSION) {
                /*
                 * LA support 12bits only.
                 */
                assert (lsenv_offset_of_gpr(lsenv, i) >=  -2048
                            && lsenv_offset_of_gpr(lsenv, i) <=  2047);
                if (ir2_opnd_default_em(&gpr_opnd) == EM_X86_ADDRESS) {
                    /*
                     * FIXME: Why MIPS leverage LDWU here?
                     */
                    la_append_ir2_opnd2i(LISA_LD_WU, gpr_opnd, env_ir2_opnd,
                                      lsenv_offset_of_gpr(lsenv, i));
                    ir2_opnd_set_em(&gpr_opnd, EM_X86_ADDRESS, 32);
                } else {
                    lsassert(ir2_opnd_default_em(&gpr_opnd) == ZERO_EXTENSION);
                    la_append_ir2_opnd2i(LISA_LD_WU, gpr_opnd, env_ir2_opnd,
                                      lsenv_offset_of_gpr(lsenv, i));
                    ir2_opnd_set_em(&gpr_opnd, ZERO_EXTENSION, 32);
                }
                continue;
            }
#endif
            la_append_ir2_opnd2i(LISA_LD_W, gpr_opnd, env_ir2_opnd,
                              lsenv_offset_of_gpr(lsenv, i));
        }
    }
}

void tr_gen_call_to_helper(ADDR func_addr)
{
    IR2_OPND func_addr_opnd = ra_alloc_dbt_arg2();
    load_ireg_from_addr(func_addr_opnd, (ADDR)func_addr);
    la_append_ir2_opnd2i(LISA_JIRL, ra_ir2_opnd, func_addr_opnd, 0);
}

static void convert_fpregs_64_to_x80(void)
{
    int i;

    CPUX86State *env = (CPUX86State*)lsenv->cpu_state;
    float_status s = env->fp_status;

    for (i = 0; i < 8; i++) {
        FPReg *p = &(env->fpregs[i]);
        p->d = float64_to_floatx80((float64)p->d.low, &s);
    }
}

static void convert_fpregs_x80_to_64(void)
{
    int i;

    CPUX86State *env = (CPUX86State*)lsenv->cpu_state;
    float_status s = env->fp_status;

    for (i = 0; i < 8; i++) {
        FPReg *p = &(env->fpregs[i]);
        p->d.low = (uint64_t)floatx80_to_float64(p->d, &s);
        p->d.high = 0;
    }
}

static void tr_gen_call_to_helper_prologue(int use_fp)
{

    tr_save_registers_to_env(GPR_USEDEF_TO_SAVE, FPR_USEDEF_TO_SAVE,
                             XMM_LO_USEDEF_TO_SAVE, XMM_HI_USEDEF_TO_SAVE, 0x1|options_to_save()); 

    if (use_fp) {
        IR2_OPND func_addr_opnd = ra_alloc_dbt_arg2();

        load_ireg_from_addr(func_addr_opnd, (ADDR)convert_fpregs_64_to_x80);
        la_append_ir2_opnd2i(LISA_JIRL, ir2_opnd_new(IR2_OPND_IREG, 1), func_addr_opnd, 0);

        IR2_OPND a0_opnd = ir2_opnd_new(IR2_OPND_IREG, 4);
        load_ireg_from_addr(func_addr_opnd, (ADDR)update_fp_status);

        la_append_ir2_opnd2_em(LISA_MOV64,a0_opnd, env_ir2_opnd);
        la_append_ir2_opnd2i(LISA_JIRL, ir2_opnd_new(IR2_OPND_IREG, 1), func_addr_opnd, 0);
    }
}

static void tr_gen_call_to_helper_epilogue(int use_fp)
{
    if (use_fp) {
        IR2_OPND func_addr_opnd = ra_alloc_dbt_arg2();
        load_ireg_from_addr(func_addr_opnd, (ADDR)convert_fpregs_x80_to_64);
        la_append_ir2_opnd2i(LISA_JIRL, ir2_opnd_new(IR2_OPND_IREG, 1), func_addr_opnd, 0);
    }

    tr_load_registers_from_env(GPR_USEDEF_TO_SAVE, FPR_USEDEF_TO_SAVE,
                               XMM_LO_USEDEF_TO_SAVE, XMM_HI_USEDEF_TO_SAVE,
                               0x1|options_to_save());
}

/* helper with 1 default arg(CPUArchState*) */ 
void tr_gen_call_to_helper1(ADDR func, int use_fp)
{
    IR2_OPND func_addr_opnd = ra_alloc_dbt_arg2();
    IR2_OPND a0_opnd = ir2_opnd_new(IR2_OPND_IREG, 4);

    tr_gen_call_to_helper_prologue(use_fp);

    load_ireg_from_addr(func_addr_opnd, (ADDR)func);

    la_append_ir2_opnd2_em(LISA_MOV64, a0_opnd, env_ir2_opnd);
    la_append_ir2_opnd2i(LISA_JIRL, ir2_opnd_new(IR2_OPND_IREG, 1), func_addr_opnd, 0);

    tr_gen_call_to_helper_epilogue(use_fp);
}

void tr_gen_call_to_helper2(ADDR func, IR2_OPND arg_opnd, int use_fp)
{
    IR2_OPND func_addr_opnd = ra_alloc_dbt_arg2();
    IR2_OPND a0_opnd = ir2_opnd_new(IR2_OPND_IREG, 4);
    IR2_OPND a1_opnd = ir2_opnd_new(IR2_OPND_IREG, 5);

    tr_gen_call_to_helper_prologue(use_fp);

    load_ireg_from_addr(func_addr_opnd, (ADDR)func);

    la_append_ir2_opnd2_em(LISA_MOV64, a0_opnd, env_ir2_opnd);
    la_append_ir2_opnd3(LISA_OR, a1_opnd, zero_ir2_opnd, arg_opnd);
    la_append_ir2_opnd2i(LISA_JIRL, ir2_opnd_new(IR2_OPND_IREG, 1), func_addr_opnd, 0);

    tr_gen_call_to_helper_epilogue(use_fp);
}

IR2_OPND tr_lat_spin_lock(IR2_OPND mem_addr, int imm)
{
    IR2_OPND label_lat_lock = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_locked= ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND lat_lock_addr = ra_alloc_itemp();
    IR2_OPND lat_lock_val= ra_alloc_itemp();
    IR2_OPND cpu_index = ra_alloc_itemp();
    ir2_opnd_set_em(&lat_lock_addr, ZERO_EXTENSION, 64);
	//compute lat_lock offset by add (mem_addr+imm)[9:6]
    la_append_ir2_opnd2i(LISA_ADDI_W, lat_lock_addr, mem_addr, imm);
    la_append_ir2_opnd2ii(LISA_BSTRPICK_D, lat_lock_val, lat_lock_addr, 9, 6);
    la_append_ir2_opnd2i(LISA_SLLI_W, lat_lock_val, lat_lock_val, 6);
    load_ireg_from_addr(lat_lock_addr, (ADDR)(lat_lock));
    la_append_ir2_opnd3(LISA_ADD_D, lat_lock_addr, lat_lock_addr, lat_lock_val);

    la_append_ir2_opnd2i(LISA_LD_W, cpu_index, env_ir2_opnd,
                      lsenv_offset_of_cpu_index(lsenv));
    la_append_ir2_opnd2i(LISA_ADDI_W, cpu_index, cpu_index, 1);
    //spin lock
    la_append_ir2_opnd1(LISA_LABEL, label_lat_lock);
    la_append_ir2_opnd2i_em(LISA_LL_W, lat_lock_val, lat_lock_addr, 0);
    la_append_ir2_opnd3(LISA_BNE, lat_lock_val, zero_ir2_opnd, label_locked);
    la_append_ir2_opnd3(LISA_OR, lat_lock_val, lat_lock_val, cpu_index);
    la_append_ir2_opnd1(LISA_LABEL, label_locked);
    la_append_ir2_opnd3(LISA_BNE, lat_lock_val, cpu_index, label_lat_lock);
    la_append_ir2_opnd2i(LISA_SC_W, lat_lock_val, lat_lock_addr, 0);
    la_append_ir2_opnd3(LISA_BEQ, lat_lock_val, zero_ir2_opnd, label_lat_lock);

    ra_free_temp(cpu_index);
    ra_free_temp(lat_lock_val);

	return lat_lock_addr;
}

void tr_lat_spin_unlock(IR2_OPND lat_lock_addr)
{
    la_append_ir2_opnd0(LISA_DBAR);
    la_append_ir2_opnd2i(LISA_ST_W, zero_ir2_opnd, lat_lock_addr, 0);
    ra_free_temp(lat_lock_addr);
}
