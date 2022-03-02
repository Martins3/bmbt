#ifndef _ETB_H_
#define _ETB_H_

#include "ir1.h"
#include "qemu-def.h"

#define MAX_DEPTH 5 
typedef enum {
    TB_TYPE_NONE = 80,
    TB_TYPE_BRANCH,
    TB_TYPE_CALL,
    TB_TYPE_RETURN,
    TB_TYPE_JUMP,
    TB_TYPE_CALLIN,
    TB_TYPE_JUMPIN,
} TB_TYPE;

/* func to access EXTRA TB */
static inline ETB *qm_tb_get_extra_tb(void *tb)
{
    struct TranslationBlock *ptb = (struct TranslationBlock *)tb;
    return &ptb->extra_tb;
}

/* functions to access ETB items */
static inline IR1_INST *tb_ir1_inst_first(struct TranslationBlock *tb)
{
    return tb->_ir1_instructions;
}

static inline IR1_INST *tb_ir1_inst_last(TranslationBlock *tb)
{
    return tb->_ir1_instructions + tb->icount - 1;
}

static inline IR1_INST *tb_ir1_inst(TranslationBlock *tb, const int i)
{
    return tb->_ir1_instructions + i;
}

static inline int tb_ir1_num(TranslationBlock *tb)
{
    return tb->icount;
}

static inline int8 etb_get_top_in(struct TranslationBlock *tb)
{
    return tb->_top_in;
}

static inline void etb_check_top_in(struct TranslationBlock *tb, int top_in)
{
    if (tb->_top_in == -1) {
        lsassert(tb->_top_out == -1);
        lsassert(top_in >= 0 && top_in <= 7);
        tb->_top_in = top_in;
    /* } else {
     *     assertm(top_in() == top, "\n%s: TB<0x%x>: top_in<%d> does not equal
     *     top<%d>\n\
     *         NOTE: last_tb_executed: first time: 0x%x, current time: 0x%x\n",\
     *         BIN_INFO::get_exe_short_name(), this->addr(), top_in(), top,\
     *         _last_tb_x86_addr, env->last_executed_tb()->addr());
     */
    }
}

static inline void etb_check_top_out(struct TranslationBlock *tb, int top_out)
{
    lsassert(tb->_top_in != -1);

    if (tb->_top_out == -1) {
        lsassert(top_out >= 0 && top_out <= 7);
        tb->_top_out = top_out;
    /* } else {
     *     assertm(top_out() == top, "\n%s: TB<0x%x>: top_out<%d> does not equal
     *     top<%d>\n\
     *         NOTE: last_tb_executed: first time: 0x%x, current time: 0x%x\n",\
     *         BIN_INFO::get_exe_short_name(), this->addr(), top_out(), top,\
     *         _last_tb_x86_addr, env->last_executed_tb()->addr());
     */
    }
}

void etb_qht_init(void);

#endif
