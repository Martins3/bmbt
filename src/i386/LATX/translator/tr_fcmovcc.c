#include "common.h"
#include "reg_alloc.h"
#include "env.h"
#include "flag_lbt.h"

bool translate_fcmovb(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
    if (tr_gen_excp_check(pir1)) return true;
#endif
    lsassertm(0, "fcmovb to be implemented in LoongArch.\n");
//    IR2_OPND cf_opnd = ra_alloc_itemp();
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//	
//    get_eflag_condition(&cf_opnd, pir1);
//
//    append_ir2_opnd3(mips_beq, &cf_opnd, &zero_ir2_opnd, &label_exit);
//
//    IR2_OPND dst_opnd = ra_alloc_st(0);
//    IR2_OPND src_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    append_ir2_opnd2(mips_mov_d, &dst_opnd, &src_opnd);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&cf_opnd);

    return true;
}

bool translate_fcmove(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
    if (tr_gen_excp_check(pir1)) return true;
#endif
    lsassertm(0, "fcmove to be implemented in LoongArch.\n");
//    IR2_OPND zf_opnd = ra_alloc_itemp();
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//	
//    get_eflag_condition(&zf_opnd, pir1);
//
//    append_ir2_opnd3(mips_beq, &zf_opnd, &zero_ir2_opnd, &label_exit);
//
//    IR2_OPND dst_opnd = ra_alloc_st(0);
//    IR2_OPND src_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    append_ir2_opnd2(mips_mov_d, &dst_opnd, &src_opnd);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&zf_opnd);

    return true;
}

bool translate_fcmovbe(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
    if (tr_gen_excp_check(pir1)) return true;
#endif
    lsassertm(0, "fcmovbe to be implemented in LoongArch.\n");
//    IR2_OPND cfzf_opnd = ra_alloc_itemp();
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//	
//    get_eflag_condition(&cfzf_opnd, pir1);
//
//    append_ir2_opnd3(mips_beq, &cfzf_opnd, &zero_ir2_opnd, &label_exit);
//
//    IR2_OPND dst_opnd = ra_alloc_st(0);
//    IR2_OPND src_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    append_ir2_opnd2(mips_mov_d, &dst_opnd, &src_opnd);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&cfzf_opnd);

    return true;
}

bool translate_fcmovu(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
    if (tr_gen_excp_check(pir1)) return true;
#endif
    lsassertm(0, "fcmovu to be implemented in LoongArch.\n");
//    IR2_OPND pf_opnd = ra_alloc_itemp();
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//	
//    get_eflag_condition(&pf_opnd, pir1);
//
//    append_ir2_opnd3(mips_beq, &pf_opnd, &zero_ir2_opnd, &label_exit);
//
//    IR2_OPND dst_opnd = ra_alloc_st(0);
//    IR2_OPND src_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    append_ir2_opnd2(mips_mov_d, &dst_opnd, &src_opnd);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&pf_opnd);

    return true;
}

bool translate_fcmovnb(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
    if (tr_gen_excp_check(pir1)) return true;
#endif
    lsassertm(0, "fcmovnb to be implemented in LoongArch.\n");
//    IR2_OPND cf_opnd = ra_alloc_itemp();
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//	
//    get_eflag_condition(&cf_opnd, pir1);
//
//    append_ir2_opnd3(mips_bne, &cf_opnd, &zero_ir2_opnd, &label_exit);
//
//    IR2_OPND dst_opnd = ra_alloc_st(0);
//    IR2_OPND src_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    append_ir2_opnd2(mips_mov_d, &dst_opnd, &src_opnd);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&cf_opnd);

    return true;
}

bool translate_fcmovne(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
    if (tr_gen_excp_check(pir1)) return true;
#endif
    lsassertm(0, "fcmovne to be implemented in LoongArch.\n");
//    IR2_OPND zf_opnd = ra_alloc_itemp();
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//	
//    get_eflag_condition(&zf_opnd, pir1);
//
//    append_ir2_opnd3(mips_bne, &zf_opnd, &zero_ir2_opnd, &label_exit);
//
//    IR2_OPND dst_opnd = ra_alloc_st(0);
//    IR2_OPND src_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    append_ir2_opnd2(mips_mov_d, &dst_opnd, &src_opnd);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&zf_opnd);

    return true;
}

bool translate_fcmovnbe(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
    if (tr_gen_excp_check(pir1)) return true;
#endif
    lsassertm(0, "fcmovnbe to be implemented in LoongArch.\n");
//    IR2_OPND cfzf_opnd = ra_alloc_itemp();
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//	
//    get_eflag_condition(&cfzf_opnd, pir1);
//
//    append_ir2_opnd3(mips_bne, &cfzf_opnd, &zero_ir2_opnd, &label_exit);
//
//    IR2_OPND dst_opnd = ra_alloc_st(0);
//    IR2_OPND src_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    append_ir2_opnd2(mips_mov_d, &dst_opnd, &src_opnd);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&cfzf_opnd);

    return true;
}

bool translate_fcmovnu(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
    if (tr_gen_excp_check(pir1)) return true;
#endif
    lsassertm(0, "fcmovnu to be implemented in LoongArch.\n");
//    IR2_OPND pf_opnd = ra_alloc_itemp();
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//	
//    get_eflag_condition(&pf_opnd, pir1);
//
//    append_ir2_opnd3(mips_bne, &pf_opnd, &zero_ir2_opnd, &label_exit);
//
//    IR2_OPND dst_opnd = ra_alloc_st(0);
//    IR2_OPND src_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    append_ir2_opnd2(mips_mov_d, &dst_opnd, &src_opnd);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&pf_opnd);

    return true;
}
