#ifndef _REG_ALLOC_H_
#define _REG_ALLOC_H_

#include "ir2/ir2.h"

typedef struct {
    uint32 physical_id;
} TEMP_REG_STATUS;

extern TEMP_REG_STATUS itemp_status_default[];
extern TEMP_REG_STATUS ftemp_status_default[];

#define itemp_status_num 8
#define ftemp_status_num 7

/* allocate x86 register */
IR2_OPND ra_alloc_gpr(int);
IR2_OPND ra_alloc_vreg(int);
IR2_OPND ra_alloc_mmx(int);
IR2_OPND ra_alloc_st(int);
IR2_OPND ra_alloc_xmm_lo(int xmm_num);
IR2_OPND ra_alloc_xmm_hi(int xmm_num);
IR2_OPND ra_alloc_xmm(int num);

/* allocate dbt register */
IR2_OPND ra_alloc_dbt_arg1(void);
IR2_OPND ra_alloc_dbt_arg2(void);

/* allocate self-defined register */
IR2_OPND ra_alloc_guest_base(void);

/* allocate temp register */
IR2_OPND ra_alloc_itemp(void);
IR2_OPND ra_alloc_ftemp(void);
IR2_OPND ra_alloc_itemp_internal(void);
IR2_OPND ra_alloc_ftemp_internal(void);

/* free temp register */
void ra_free_temp(IR2_OPND*);
void ra_free_itemp(int);
void ra_free_ftemp(int);

/* temp register management */
int ra_get_temp_nr(void);
int ra_get_temp_reg_by_idx(int idx);

#endif
