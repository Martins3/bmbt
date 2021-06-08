#ifndef _ETB_H_
#define _ETB_H_

#include "common.h"
#include "x86tomips-config.h"
#include "ir1/ir1.h"

/* functions to access ETB items */
IR1_INST *etb_ir1_inst_first(ETB *etb);
IR1_INST *etb_ir1_inst_last(ETB *etb);
IR1_INST *etb_ir1_inst(ETB *etb, const int i);
int etb_ir1_num(ETB *etb);

ETB* etb_alloc(void);
void etb_init(ETB *etb);
void etb_free(ETB* etb);

void etb_check_top_in(ETB *etb, int top);
void etb_check_top_out(ETB *etb, int top);
int  etb_get_top_in(ETB *etb);
void etb_set_top_in(ETB *etb, int8 top_in);
int  etb_get_top_out(ETB *etb);
void etb_set_top_out(ETB *etb, int8 top_out);

void tb_flag_usedef(TranslationBlock *tb);

#ifndef CONFIG_SOFTMMU

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

/* etb hash table functions */
void etb_cache_qht_init(void);
bool etb_lookup_custom(const void *ap, const void *bp);
ETB *etb_cache_find(ADDRX pc, bool used_to_attach_tb);

/* flag_pattern.c */
void tb_find_flag_pattern(TranslationBlock *tb);

/* flag_reduction.c */
void tb_flag_reduction(TranslationBlock *tb);
uint8 pending_use_of_succ(ETB* etb, int max_depth);
void etb_add_succ(ETB *etb, int depth);
int8 get_etb_type(IR1_INST *last_ir1);
#endif

#ifdef CONFIG_SOFTMMU
#if defined(CONFIG_XTM_PROFILE) || defined(CONFIG_XTM_FAST_CS)
void etb_pref_cs_mask(ETB *etb, IR1_INST *pir1);
#endif
#endif

extern QHT *etb_cache_qht;
#endif
