#include "common.h"
#include "reg-alloc.h"
#include "flag-lbt.h"
#include "translate.h"

bool translate_fcmovb(IR1_INST *pir1){
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fcmovb(pir1);
#else
    IR2_OPND cf_opnd = ra_alloc_itemp();
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
	
    get_eflag_condition(&cf_opnd, pir1);

    la_append_ir2_opnd3(LISA_BEQ, cf_opnd, zero_ir2_opnd, label_exit);

    IR2_OPND dst_opnd = ra_alloc_st(0);
    IR2_OPND src_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
    la_append_ir2_opnd2(LISA_FMOV_D, dst_opnd, src_opnd);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(cf_opnd);

    return true;
#endif
}

bool translate_fcmove(IR1_INST *pir1){
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fcmove(pir1);
#else
    IR2_OPND zf_opnd = ra_alloc_itemp();
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
	
    get_eflag_condition(&zf_opnd, pir1);

    la_append_ir2_opnd3(LISA_BEQ, zf_opnd, zero_ir2_opnd, label_exit);

    IR2_OPND dst_opnd = ra_alloc_st(0);
    IR2_OPND src_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
    la_append_ir2_opnd2(LISA_FMOV_D, dst_opnd, src_opnd);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(zf_opnd);

    return true;
#endif
}

bool translate_fcmovbe(IR1_INST *pir1){
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fcmovbe(pir1);
#else
    IR2_OPND cfzf_opnd = ra_alloc_itemp();
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
	
    get_eflag_condition(&cfzf_opnd, pir1);

    la_append_ir2_opnd3(LISA_BEQ, cfzf_opnd, zero_ir2_opnd, label_exit);

    IR2_OPND dst_opnd = ra_alloc_st(0);
    IR2_OPND src_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
    la_append_ir2_opnd2(LISA_FMOV_D, dst_opnd, src_opnd);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(cfzf_opnd);

    return true;
#endif
}

bool translate_fcmovu(IR1_INST *pir1){
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fcmovu(pir1);
#else
    IR2_OPND pf_opnd = ra_alloc_itemp();
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
	
    get_eflag_condition(&pf_opnd, pir1);

    la_append_ir2_opnd3(LISA_BEQ, pf_opnd, zero_ir2_opnd, label_exit);

    IR2_OPND dst_opnd = ra_alloc_st(0);
    IR2_OPND src_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
    la_append_ir2_opnd2(LISA_FMOV_D, dst_opnd, src_opnd);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(pf_opnd);

    return true;
#endif
}

bool translate_fcmovnb(IR1_INST *pir1){
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fcmovnb(pir1);
#else
    IR2_OPND cf_opnd = ra_alloc_itemp();
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
	
    get_eflag_condition(&cf_opnd, pir1);

    la_append_ir2_opnd3(LISA_BNE, cf_opnd, zero_ir2_opnd, label_exit);

    IR2_OPND dst_opnd = ra_alloc_st(0);
    IR2_OPND src_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
    la_append_ir2_opnd2(LISA_FMOV_D, dst_opnd, src_opnd);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(cf_opnd);

    return true;
#endif
}

bool translate_fcmovne(IR1_INST *pir1){
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fcmovne(pir1);
#else
    IR2_OPND zf_opnd = ra_alloc_itemp();
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
	
    get_eflag_condition(&zf_opnd, pir1);

    la_append_ir2_opnd3(LISA_BNE, zf_opnd, zero_ir2_opnd, label_exit);

    IR2_OPND dst_opnd = ra_alloc_st(0);
    IR2_OPND src_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
    la_append_ir2_opnd2(LISA_FMOV_D, dst_opnd, src_opnd);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(zf_opnd);

    return true;
#endif
}

bool translate_fcmovnbe(IR1_INST *pir1){
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fcmovnbe(pir1);
#else
    IR2_OPND cfzf_opnd = ra_alloc_itemp();
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
	
    get_eflag_condition(&cfzf_opnd, pir1);

    la_append_ir2_opnd3(LISA_BNE, cfzf_opnd, zero_ir2_opnd, label_exit);

    IR2_OPND dst_opnd = ra_alloc_st(0);
    IR2_OPND src_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
    la_append_ir2_opnd2(LISA_FMOV_D, dst_opnd, src_opnd);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(cfzf_opnd);

    return true;
#endif
}

bool translate_fcmovnu(IR1_INST *pir1){
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fcmovnu(pir1);
#else
    IR2_OPND pf_opnd = ra_alloc_itemp();
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
	
    get_eflag_condition(&pf_opnd, pir1);

    la_append_ir2_opnd3(LISA_BNE, pf_opnd, zero_ir2_opnd, label_exit);

    IR2_OPND dst_opnd = ra_alloc_st(0);
    IR2_OPND src_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
    la_append_ir2_opnd2(LISA_FMOV_D, dst_opnd, src_opnd);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(pf_opnd);

    return true;
#endif
}
