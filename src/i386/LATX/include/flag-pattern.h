#ifndef _FLAG_PATTERN_H_
#define _FLAG_PATTERN_H_
#include "qemu/osdep.h"
#include "ir1.h"
#include "ir2.h"
#ifdef CONFIG_LATX_FLAG_PATTERN

void fp_init(void);
bool fp_is_save_dest_opnd(IR1_INST *pir1, IR2_OPND dest);
void fp_save_src_opnd(IR1_INST *pir1, IR2_OPND src0, IR2_OPND src1);
void fp_save_dest_opnd(IR1_INST *pir1, IR2_OPND dest);
bool fp_translate_pattern_tail(IR1_INST *pir1, IR2_OPND label_or_condition);
void fp_init_skipped_flags(IR1_INST *pir1);
void tb_find_flag_pattern(void *tb);

#define MAX_PATTERN_TAIL_NUM_PER_HEAD 3
#define MAX_PATTERN_HEAD_NUM_PER_TB (MAX_IR1_NUM_PER_TB / 2)

typedef struct FLAG_PATTERN_ITEM {
    IR1_INST *head;
    IR1_INST *tails[MAX_PATTERN_TAIL_NUM_PER_HEAD];
    int16 dest_reg_num;
    int16 src0_reg_num;
    bool src1_is_imm;
    int src1_imm_value;
    int16 src1_reg_num;
    uint8 skipped_flags;
} FLAG_PATTERN_ITEM;

typedef struct FLAG_PATTERN_DATA {
    int8 is_head_or_tail[MAX_IR1_NUM_PER_TB];
    FLAG_PATTERN_ITEM pattern_items[MAX_PATTERN_HEAD_NUM_PER_TB];
    int pattern_items_num;
} FLAG_PATTERN_DATA;

#endif
#endif
