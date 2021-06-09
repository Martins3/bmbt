#include "include/common.h"
#include "include/mem.h"
#include "include/env.h"
#include "include/etb.h"
#include "ir2/ir2.h"

IR1_INST *etb_ir1_inst_first(ETB *etb)
{
    return etb_ir1_inst(etb, 0);
}
IR1_INST *etb_ir1_inst_last(ETB *etb)
{
    return etb_ir1_inst(etb, etb->_ir1_num - 1);
}
IR1_INST *etb_ir1_inst(ETB *etb, const int i)
{
    return etb->_ir1_instructions + i;
}

int etb_ir1_num(ETB *etb) { return etb->_ir1_num; }

int etb_get_top_in(ETB *etb)  { return etb->_top_in; }
int etb_get_top_out(ETB *etb) { return etb->_top_out; }

void etb_set_top_in(ETB *etb, int8 top_in)
{
    lsassert(top_in >= 0 && top_in <= 7);
    etb->_top_in = top_in;
}
void etb_set_top_out(ETB *etb, int8 top_out)
{
    lsassert(top_out >= 0 && top_out <= 7);
    etb->_top_out = top_out;
}

void etb_check_top_in(ETB *etb, int top)
{
    if (etb_get_top_in(etb) == -1) {
        lsassert(etb_get_top_out(etb) == -1);
        etb_set_top_in(etb, top);
    }
}
void etb_check_top_out(ETB *etb, int top)
{
    lsassert(etb_get_top_in(etb) != -1);
    if (etb_get_top_out(etb) == -1) {
        etb_set_top_out(etb, top);
    }
}

void etb_init(ETB *etb)
{
    memset(etb, 0, sizeof(ETB));
    etb->_top_out = -1;
    etb->_top_in = -1;

    etb->end_with_jcc = 0;
    etb->branch_to_target_direct_in_mips_branch = 0;
    etb->mips_branch_inst_offset = -1;
    etb->mips_branch_backup = 0;
}

void etb_free(ETB* etb)
{
    if (etb == NULL) return;

    /* Since we do not keep the disasm result 
     * etb->ir1_instructions will always be NULL
     * outside the translation process */
    if (etb->_ir1_instructions != NULL) {
        int i = 0;
        for (i = 0; i < etb->_ir1_num; ++i) {
            /* Free capstone's disassemable result */
            ir1_free_info(&(etb->_ir1_instructions[i]));
        }
        mm_free(etb->_ir1_instructions);
        etb->_ir1_instructions = NULL;
        etb->_ir1_num = 0;
    }

    mm_free(etb);
}

ETB* etb_alloc(void)
{
    ETB *etb = (ETB*)mm_malloc(sizeof(ETB));
    etb_init(etb);
    return etb;
}

#ifndef CONFIG_SOFTMMU
/* etb_cache_qht functions and etb_array */
static bool etb_qht_cmp_func(const void *ap, const void *bp)
{
    ETB *p = (ETB*)ap;
    ETB *q = (ETB*)bp;
    return p->pc == q->pc;
}

void etb_cache_qht_init(void)
{
    unsigned int mode = QHT_MODE_AUTO_RESIZE;

    qht_init(etb_cache_qht, etb_qht_cmp_func, 1 << 10, mode);
}

bool etb_lookup_custom(const void *ap, const void *bp)
{
    ETB *etb = (ETB*)ap;
    ADDRX pc = *(ADDRX *)bp;
    return etb->pc == pc;
}

ETB *etb_cache_find(ADDRX pc, bool used_to_attach_tb)
{
    uint32_t hash = pc & 0x3ff;

    ETB *etb = (ETB*)qht_lookup_custom(etb_cache_qht, &pc, hash, etb_lookup_custom);
    if (etb != NULL && used_to_attach_tb) {
        qht_remove(etb_cache_qht, etb, hash);
    }
    if (etb == NULL) {
        etb = etb_alloc();
        etb->pc = pc;
        lsassertm(etb,"memory is full\n");
        if (!used_to_attach_tb) {
            qht_insert(etb_cache_qht, etb, hash, NULL);
	}
    }
    return etb;
}

static bool etb_free_qht_iter_func(void *p, uint32_t h, void *up)
{
    ETB *etb = (ETB *)p;
    etb_free(etb);
    return true;
}

void etb_cache_clear(void)
{
    qht_iter_remove(etb_cache_qht, etb_free_qht_iter_func, NULL);
}

void etb_add_succ(ETB *etb,int depth) 
{
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
        
        ETB* succ_etb = etb_cache_find(linear_address(succ[i]), false);
        etb->succ[i] = succ_etb;
      
        IR1_INST *ir1_list = succ_etb->_ir1_instructions;
        int ir1_num = succ_etb->_ir1_num;
        if (ir1_list == NULL) {
            ir1_list = get_ir1_list(succ_etb, linear_address(succ[i]), &ir1_num);
            succ_etb->_ir1_instructions = ir1_list;
            succ_etb->_ir1_num = ir1_num;
            succ_etb->_tb_type = get_etb_type(ir1_list + ir1_num - 1); 
        }
        etb_add_succ(succ_etb, depth-1);
    
    }
    return;
}

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

#ifdef CONFIG_SOFTMMU
#if defined(CONFIG_XTM_PROFILE) || defined(CONFIG_XTM_FAST_CS)
void etb_pref_cs_mask(ETB *etb, IR1_INST *pir1)
{
    int i = 0;

    int grp_nr = ir1_grp_nr(pir1);
    uint8_t *grps = ir1_get_grps(pir1);

    switch (ir1_opcode(pir1)) {
    case X86_INS_FXSAVE:
    case X86_INS_FXRSTOR:
    case X86_INS_XSAVE:
    case X86_INS_XRSTOR:
    case X86_INS_XSAVEOPT:
        etb->fast_cs_mask |= XTM_FAST_CS_MASK_FPU;
        etb->fast_cs_mask |= XTM_FAST_CS_MASK_XMM;
        break;
    default:
        break;
    }

    for (i = 0; i < grp_nr; ++i) {
        switch (grps[i]) {
            case X86_GRP_FPU:
            case X86_GRP_MMX:
            case X86_GRP_3DNOW: /* MMX */
                etb->fast_cs_mask |= XTM_FAST_CS_MASK_FPU;
                break;
            case X86_GRP_AES:
            case X86_GRP_ADX:
            case X86_GRP_AVX:
            case X86_GRP_AVX2:
            case X86_GRP_AVX512:
            case X86_GRP_F16C:
            case X86_GRP_FMA:
            case X86_GRP_FMA4:
            case X86_GRP_SHA:
            case X86_GRP_SSE1:
            case X86_GRP_SSE2:
            case X86_GRP_SSE3:
            case X86_GRP_SSE41:
            case X86_GRP_SSE42:
            case X86_GRP_SSE4A:
            case X86_GRP_PCLMUL:
            case X86_GRP_XOP:
            case X86_GRP_CDI: /* AVX512CD  */
            case X86_GRP_ERI: /* AVX512ER  */
            case X86_GRP_DQI: /* AVX512DQ  */
            case X86_GRP_BWI: /* AVX512BW  */
            case X86_GRP_PFI: /* AVX512PF  */
            case X86_GRP_VLX: /* AVX512VL? */
                etb->fast_cs_mask |= XTM_FAST_CS_MASK_XMM;
                break;
            case X86_GRP_SSSE3:
                etb->fast_cs_mask |= XTM_FAST_CS_MASK_FPU;
                etb->fast_cs_mask |= XTM_FAST_CS_MASK_XMM;
                break;
            default:
                break;
        }
    }
}
#endif
#endif
