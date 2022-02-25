#ifndef _REG_ALLOC_H_
#define _REG_ALLOC_H_

#include "ir2.h"

void ra_init(void);

/* allocate x86 register */
IR2_OPND ra_alloc_top(void);
IR2_OPND ra_alloc_eflags(void);
IR2_OPND ra_alloc_ss(void);
IR2_OPND ra_alloc_env(void);
IR2_OPND ra_alloc_f32(void);
IR2_OPND ra_alloc_gpr(int);
IR2_OPND ra_alloc_vreg(int);
IR2_OPND ra_alloc_mmx(int);
IR2_OPND ra_alloc_st(int);
IR2_OPND ra_alloc_xmm_lo(int xmm_num);
IR2_OPND ra_alloc_xmm_hi(int xmm_num);
IR2_OPND ra_alloc_xmm(int num);
IR2_OPND ra_alloc_mda(void);

/* allocate dbt register */
IR2_OPND ra_alloc_dbt_arg1(void);
IR2_OPND ra_alloc_dbt_arg2(void);

/* allocate flag pattern register */
IR2_OPND ra_alloc_flag_pattern_saved_opnd0(void);
IR2_OPND ra_alloc_flag_pattern_saved_opnd1(void);

/* allocate self-defined register */
IR2_OPND ra_alloc_guest_base(void);

/* allocate temp register */
IR2_OPND ra_alloc_itemp(void);
IR2_OPND ra_alloc_ftemp(void);
IR2_OPND ra_alloc_itemp_internal(void);
IR2_OPND ra_alloc_ftemp_internal(void);

/* free temp register */
void ra_free_temp(IR2_OPND);
void ra_free_all_internal_temp(void);
void ra_free_itemp(int);
void ra_free_ftemp(int);

/* assign temp register to physical register */
bool ra_temp_register_allocation(void);

#ifdef CONFIG_SOFTMMU

void latxs_ra_init(void);

/* allocate x86 register */
IR2_OPND latxs_ra_alloc_gpr(int);
IR2_OPND latxs_ra_alloc_vreg(int);
IR2_OPND latxs_ra_alloc_mmx(int);
IR2_OPND latxs_ra_alloc_st(int);
IR2_OPND latxs_ra_alloc_xmm(int num);

/* allocate dbt register */
IR2_OPND latxs_ra_alloc_dbt_arg1(void);

/* allocate temp register */
IR2_OPND latxs_ra_alloc_itemp(void);
IR2_OPND latxs_ra_alloc_ftemp(void);

/* free temp register */
void latxs_ra_free_temp(IR2_OPND *);

#endif

#endif
