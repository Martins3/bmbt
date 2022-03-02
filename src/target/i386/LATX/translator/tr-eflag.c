#include "common.h"
#include "reg-alloc.h"
#include "lsenv.h"
#include "latx-options.h"
#include "translate.h"

bool translate_popf(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_popf(pir1);
#else
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    IR2_OPND eflags_opnd = ra_alloc_eflags();
    IR2_OPND eflags_temp_opnd = ra_alloc_itemp();

    if (cpu_get_guest_base() != 0) {
        IR2_OPND tmp = ra_alloc_itemp();
        IR2_OPND gbase = ra_alloc_guest_base();
        if (!ir2_opnd_is_address(&esp_opnd)) {
            la_append_ir2_opnd2_em(LISA_MOV_ADDRX, esp_opnd, esp_opnd);
        }
        la_append_ir2_opnd3_em(LISA_ADD_ADDR, tmp, esp_opnd, gbase);
        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
        la_append_ir2_opnd2i(LISA_LD_W, eflags_opnd, tmp, 0);
        ra_free_temp(tmp);
        ra_free_temp(gbase);
    } else {
        la_append_ir2_opnd2i(LISA_LD_W, eflags_opnd, esp_opnd, 0);
    }
    la_append_ir2_opnd1i(LISA_X86MTFLAG, eflags_opnd, 0x3f);
    /*
     * Some apps test eflags [22:12] bits for CPU feature detection.
     * Wine detect bit 21 to detemine whether CPU support SSE.
     * To make this kind of apps happy, we not only store the bit 0 to 12
     * but also store bit 12 to 22.
     */
    load_ireg_from_imm32(eflags_temp_opnd, 0xfffff400, ZERO_EXTENSION);
    la_append_ir2_opnd3(LISA_AND, eflags_opnd, eflags_opnd, eflags_temp_opnd);
    la_append_ir2_opnd2i(LISA_ORI, eflags_opnd, eflags_opnd, 0x202);
    ra_free_temp(eflags_temp_opnd);

    la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd, 4);
    return true;
#endif
}

bool translate_pushf(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pushf(pir1);
#else
    IR2_OPND eflags_opnd = ra_alloc_eflags();
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    IR2_OPND temp   = ra_alloc_itemp();
    la_append_ir2_opnd1i_em(LISA_X86MFFLAG, temp, 0x3f);
    la_append_ir2_opnd3(LISA_OR, eflags_opnd, eflags_opnd, temp);
    ra_free_temp(temp);

    if (cpu_get_guest_base() != 0) {
        IR2_OPND tmp = ra_alloc_itemp();
        IR2_OPND gbase = ra_alloc_guest_base();
        if (!ir2_opnd_is_address(&esp_opnd)) {
            la_append_ir2_opnd2_em(LISA_MOV_ADDRX, esp_opnd, esp_opnd);
        }
        la_append_ir2_opnd3_em(LISA_ADD_ADDR, tmp, esp_opnd, gbase);
        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
        la_append_ir2_opnd2i(LISA_ST_W, eflags_opnd, tmp, -4);
        ra_free_temp(tmp);
        ra_free_temp(gbase);
    } else {
        la_append_ir2_opnd2i(LISA_ST_W, eflags_opnd, esp_opnd, -4);
    }

   la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd, -4);

    return true;
#endif
}

bool translate_clc(IR1_INST *pir1) {
#ifdef CONFIG_SOFTMMU
    return latxs_translate_clc(pir1);
#else
    la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x1);
    return true;
#endif
}

bool translate_cld(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cld(pir1);
#else
    IR2_OPND eflags = ra_alloc_eflags();

    la_append_ir2_opnd2ii(LISA_BSTRINS_W, eflags, zero_ir2_opnd, 10, 10);

    return true;
#endif
}

bool translate_stc(IR1_INST *pir1) {
#ifdef CONFIG_SOFTMMU
    return latxs_translate_stc(pir1);
#else
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x1);
    return true;
#endif
}

bool translate_std(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_std(pir1);
#else
    IR2_OPND eflags = ra_alloc_eflags();

    la_append_ir2_opnd2i_em(LISA_ORI, eflags, eflags, 0x400);

    return true;
#endif
}
