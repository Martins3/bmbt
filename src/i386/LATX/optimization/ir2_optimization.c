#include "common.h"
#include "env.h"
#include "etb.h"

//static void ir2_schedule(TranslationBlock *tb);
//static void tri_separate_branch_from_ldst(TranslationBlock *tb);

//static bool has_dependency(IR2_INST *p1, IR2_INST *p2)
//{
//    IR2_OPND empty_opnd = ir2_opnd_new_none();
//
//    for (int i = 0; i < 3; ++i) {
//        IR2_OPND *opnd1 = p1->_opnd + i;
//        if (ir2_opnd_cmp(opnd1, &empty_opnd)) {
//            break;
//        }
//
//        for (int j = 0; j < 3; ++j) {
//            IR2_OPND *opnd2 = p2->_opnd + j;
//            if (ir2_opnd_cmp(opnd2, &empty_opnd)) {
//                break;
//            }
//
//            /*if (opnd1->_reg_num == opnd2->_reg_num) {*/
//            if (ir2_opnd_base_reg_num(opnd1) ==
//                ir2_opnd_base_reg_num(opnd2) ) {
//                IR2_OPND_TYPE t1 = ir2_opnd_type(opnd1);
//                IR2_OPND_TYPE t2 = ir2_opnd_type(opnd2);
//                if (t1 == IR2_OPND_IREG &&
//                    (t2 == IR2_OPND_IREG || t2 == IR2_OPND_MEM))
//                    return true;
//                else if (t2 == IR2_OPND_IREG &&
//                         (t1 == IR2_OPND_IREG || t1 == IR2_OPND_MEM)) {
//                    return true;
//                } else if (t1 == IR2_OPND_FREG && t2 == IR2_OPND_FREG) {
//                    return true;
//                }
//            }
//        }
//    }
//
//    return false;
//}

//static void tri_schedule_one_ir2(IR2_INST *p, int max_distance)
//{ /* max_distance default is 10 */
//    int distance = 0;
//    /* 1. scan backward to find a proper position */
//    IR2_INST *ir2_first = lsenv->tr_data->first_ir2;
//    IR2_INST *ir2_block = ir2_prev(p);
//    while (ir2_block != ir2_first) {
//        IR2_OPCODE opc = ir2_opcode(ir2_block);
//        /* 1.1 if we meet a load/store/label, stop here. */
//        if (ir2_opcode_is_load(opc) || ir2_opcode_is_store(opc) ||
//            opc == mips_label) {
//            break;
//        }
//        /* 1.2. if we meet a branch/jump, stop after the delay slot */
//        else if (ir2_opcode_is_branch(opc) || ir2_opcode_is_f_branch(opc) ||
//                 opc == mips_j || opc == mips_jal || opc == mips_jr ||
//                 opc == mips_jalr) {
//            ir2_block = ir2_next(ir2_block);
//            break;
//        }
//        /* 1.3 if the operands has any dependency, stop here */
//        else if (has_dependency(p, ir2_block)) {
//            break;
//        }
//        /* 1.4 in other case, check the previous ir2 */
//        else {
//            ir2_block = ir2_prev(ir2_block);
//            distance++;
//        }
//        if (distance >= max_distance) {
//            break;
//        }
//    }
//
//    while (ir2_opcode(ir2_next(ir2_block)) == mips_x86_inst) {
//        ir2_block = ir2_next(ir2_block);
//    }
//
//    /* 2. now we have a proper ir2_block, so insert p after ir2_block */
//    if (p != ir2_block && p != ir2_next(ir2_block)) {
//        ir2_remove(p);
//        ir2_insert_after(p, ir2_block);
//    }
//}

//static void ir2_schedule(TranslationBlock *tb)
//{
//    /* 1. scan forward, from the second ir2 */
//    IR2_INST *ir2_current = ir2_next(lsenv->tr_data->first_ir2);
//    while (ir2_current != NULL) {
//        /* 1.1 schedule every load/store */
//        if (ir2_opcode_is_load(ir2_opcode(ir2_current)) ||
//            ir2_opcode_is_store(ir2_opcode(ir2_current))) {
//            tri_schedule_one_ir2(ir2_current, 10);
//        }
//        /* 1.2 we should stop before the linkage code */
//        else if (ir2_opcode(ir2_current) == mips_label) {
//            int label_id = ir2_opnd_addr(ir2_current->_opnd);
//            if (tb->jmp_target_arg[0] == label_id ||
//                tb->jmp_target_arg[1] == label_id)
//                break;
//        }
//
//        ir2_current = ir2_next(ir2_current);
//    }
//}

//static void tri_separate_branch_from_ldst(TranslationBlock *tb)
//{
//    IR2_INST *ir2_current = lsenv->tr_data->first_ir2;
//    IR2_INST *ir2_prev = NULL;
//    int ir2_prev_id = 0;
//    int ir2_curr_id = ir2_get_id(ir2_current);
//    bool is_prev_ldst_not_rl = false;
//    bool is_prev_branch = false;
//    while (ir2_current != NULL) {
//        ir2_current = ir2_get(ir2_curr_id);
//        if (ir2_opcode(ir2_current) == mips_label) {
//            int label_id = ir2_opnd_addr(ir2_current->_opnd);
//            if (tb->jmp_target_arg[0] == label_id ||
//                tb->jmp_target_arg[1] == label_id)
//                break;
//            ir2_current = ir2_next(ir2_current);
//            ir2_curr_id = ir2_get_id(ir2_current);
//            continue;
//        }
//
//        if (ir2_opcode(ir2_current) == mips_x86_inst) {
//            ir2_current = ir2_next(ir2_current);
//            ir2_curr_id = ir2_get_id(ir2_current);
//            continue;
//        }
//
//        if (ir2_opcode_is_branch(ir2_opcode(ir2_current))) {
//            if (is_prev_ldst_not_rl) {
//                IR2_INST *extra = append_ir2_opnd0(mips_nop);
//                ir2_remove(extra);
//                ir2_prev = ir2_get(ir2_prev_id);
//                ir2_insert_after(extra, ir2_prev);
//            }
//            ir2_current = ir2_get(ir2_curr_id);
//            ir2_prev_id = ir2_get_id(ir2_current);
//            is_prev_branch = true;
//        } else {
//            ir2_current = ir2_get(ir2_curr_id);
//            ir2_prev_id = ir2_get_id(ir2_current);
//            is_prev_branch = false;
//        }
//
//        if (ir2_opcode_is_load_not_rl(ir2_opcode(ir2_current)) ||
//            ir2_opcode_is_store_not_rl(ir2_opcode(ir2_current))) {
//            if (is_prev_branch) {
//                /* env->tr_data->dump(); */
//                lsassertm(0,
//                          "load/store instruction cannot be in delay slot\n");
//            }
//            ir2_current = ir2_get(ir2_curr_id);
//            ir2_prev_id = ir2_get_id(ir2_current);
//            is_prev_ldst_not_rl = true;
//        } else {
//            ir2_current = ir2_get(ir2_curr_id);
//            ir2_prev_id = ir2_get_id(ir2_current);
//            is_prev_ldst_not_rl = false;
//        }
//
//        ir2_current = ir2_get(ir2_curr_id);
//        ir2_current = ir2_next(ir2_current);
//        if (ir2_current) {
//            ir2_curr_id = ir2_get_id(ir2_current);
//        }
//    }
//}

//static void tri_avoid_last_ir2_is_ldst_not_rl(void)
//{
//    IR2_INST *ir2_curr = lsenv->tr_data->last_ir2;
//    while (ir2_opcode(ir2_curr) == mips_label ||
//           ir2_opcode(ir2_curr) == mips_x86_inst) {
//        if (ir2_curr == lsenv->tr_data->first_ir2) {
//            return;
//        }
//        ir2_curr = ir2_prev(ir2_curr);
//    }
//
//    if (ir2_opcode_is_load_not_rl(ir2_opcode(ir2_curr)) ||
//        ir2_opcode_is_store_not_rl(ir2_opcode(ir2_curr))) {
//        append_ir2_opnd0(mips_nop);
//        /* fprintf(stderr, "last meaningful ir2 is ldst not rl!\n"); */
//    }
//}

//static void tri_avoid_leading_label(void)
//{
//    IR2_INST *ir2_current = lsenv->tr_data->first_ir2;
//    while (ir2_current != NULL) {
//        if (ir2_opcode(ir2_current) == mips_x86_inst) {
//            ir2_current = ir2_next(ir2_current);
//            continue;
//        } else if (ir2_opcode(ir2_current) == mips_label) { /* leading label */
//            IR2_INST *extra = append_ir2_opnd0(mips_nop);
//            ir2_remove(extra);
//            ir2_insert_before(extra, ir2_current);
//            return;
//        } else { /* other instruction, so leading label is impossible */
//            return;
//        }
//    }
//}

void tr_ir2_optimize(void)
{
    lsassertm(0, "IR2 optimization to be implemented in LoongArch.\n");
//    /* temporarily disabled to prevent reschedule ldc1 before dectop/inctop */
//    if (0) { /* if (option_to_execute_ir2_optimize) */
//        ir2_schedule(lsenv->tr_data->curr_tb);
//    }
//    tri_avoid_last_ir2_is_ldst_not_rl();
//    tri_separate_branch_from_ldst(lsenv->tr_data->curr_tb);
//    tri_avoid_leading_label();
}
