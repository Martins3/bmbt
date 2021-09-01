#include "../include/common.h"
#include "../include/etb.h"
#include "../ir1/ir1.h"
#include "../translator/translate.h"
#include "../include/flag_usedef.h"

// FIXME maybe this is bug of original QEMU
// this is function/file is used by the usermode
#if 0
uint8 pending_use_of_succ(ETB* etb, int max_depth)
{
    if((!etb->succ[0] && !etb->succ[1]) || max_depth==0)
        return -1;
    uint8 pending_use = 0;
    for(int i=0;i<2;i++) {
        if (etb->succ[i] == NULL ) break;
        ETB* succ_etb = etb->succ[i];
        uint8 succ_pending_use = pending_use_of_succ(succ_etb, max_depth - 1);
        for (int j = etb_ir1_num(succ_etb) - 1; j >= 0; --j) {
            IR1_INST *pir1 = etb_ir1_inst(succ_etb, j);

            IR1_EFLAG_USEDEF curr_usedef =
                *ir1_opcode_to_eflag_usedef(pir1);
            // IR1_EFLAG_USEDEF curr_usedef;
            // curr_usedef.def = ir1_get_eflag_inherent_def(pir1);
            // curr_usedef.use = ir1_get_eflag_inherent_use(pir1);
            // curr_usedef.undef = ir1_get_eflag_inherent_undef(pir1);

            curr_usedef.def &= succ_pending_use;

            succ_pending_use &= (~curr_usedef.def);
            if (ir1_prefix(pir1) != 0 &&
                (ir1_opcode(pir1) == X86_INS_CMPSB ||
                 ir1_opcode(pir1) == X86_INS_CMPSW ||
                 ir1_opcode(pir1) == X86_INS_CMPSD ||
                 ir1_opcode(pir1) == X86_INS_SCASB ||
                 ir1_opcode(pir1) == X86_INS_SCASW ||
                 ir1_opcode(pir1) == X86_INS_SCASD ||
                 ir1_opcode(pir1) == X86_INS_SCASQ)
               )
                succ_pending_use |=
                    curr_usedef.def; /* "rep cmps" may not define eflags */

            /* 4.4 this instruction may have new pending use */
            succ_pending_use |= curr_usedef.use;
        }
        pending_use |= succ_pending_use;
    }
    return pending_use;
}

void tb_flag_reduction(TranslationBlock *tb)
{
    ETB *etb = tb->extra_tb;
    /* if(BITS_ARE_SET(_flags, FT_FAKE_TB)){ */
    /*    BITS_SET(_flags, FT_FLAG_REDUCTION); */
    /*    etb->_pending_use=0; */
    /*    return; */
    /*} */

    /* if (BITS_ARE_SET(_flags, FT_FLAG_REDUCTION)) { */
    /*    //lsassert(pending_use()!=-1); */
    /*    return; */
    /*} */

    /* 1. disasm */
    /* tr_disasm(tb); */

    /* 2. determine the pending use (will be used by successors) */
    uint8 pending_use = pending_use_of_succ(etb, MAX_DEPTH);

    /* 3. the eflags that are defined in the TB. */
    uint8 total_def = 0;

    /* 4. process eflags of instructions in reverse order */
    for (int i = etb_ir1_num(etb) - 1; i >= 0; --i) {
        IR1_INST *pir1 = etb_ir1_inst(etb, i);

        IR1_EFLAG_USEDEF curr_usedef =
            *ir1_opcode_to_eflag_usedef(pir1);

        // IR1_EFLAG_USEDEF curr_usedef;
        // curr_usedef.def = ir1_get_eflag_inherent_def(pir1);
        // curr_usedef.use = ir1_get_eflag_inherent_use(pir1);
        // curr_usedef.undef = ir1_get_eflag_inherent_undef(pir1);

        /* 4.1 the eflags that are defined (at least once) in the TB. */
        total_def |= curr_usedef.def;

        /* 4.2 some eflag definitions of this instuction may be useless, so */
        /* clear them */
        curr_usedef.def &= pending_use;

        /* 4.3 pending use is still pending if not satisfied by this instruction
         */
        pending_use &= (~curr_usedef.def);
        if (ir1_prefix(pir1) != 0 &&
            (ir1_opcode(pir1) == X86_INS_CMPSB ||
             ir1_opcode(pir1) == X86_INS_CMPSW ||
             ir1_opcode(pir1) == X86_INS_CMPSD ||
             ir1_opcode(pir1) == X86_INS_SCASB ||
             ir1_opcode(pir1) == X86_INS_SCASW ||
             ir1_opcode(pir1) == X86_INS_SCASD ||
             ir1_opcode(pir1) == X86_INS_SCASQ)
           )
            pending_use |=
                curr_usedef.def; /* "rep cmps" may not define eflags */

        /* 4.4 this instruction may have new pending use */
        pending_use |= curr_usedef.use;

        ir1_set_eflag_use(&(etb->_ir1_instructions[i]), curr_usedef.use);
        ir1_set_eflag_def(&(etb->_ir1_instructions[i]),
                          curr_usedef.def & (~curr_usedef.undef));
    }

    /* 5. the result is precise */
    /* etb->_pending_use = pending_use & __ALL_EFLAGS; */
    /* BITS_SET(_flags, FT_FLAG_REDUCTION); */
    //free_etb(etb->succ[0]);
    //free_etb(etb->succ[1]);
}
#endif
