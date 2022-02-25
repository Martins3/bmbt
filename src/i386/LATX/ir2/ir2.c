#include "common.h"
#include "ir1.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "translate.h"
#include <string.h>

#include "ir2.h"

static const char *ir2_name(int value)
{
    /*
     * Todo: Add below defination:
     */
    #if 0
    static const char *ir2_scr_name[] = {
        "$scr0" , "$scr1" , "$scr2" , "$scr3",
    };

    static const char *ir2_cc_name[] = {
        "$cc0" , "$cc1" , "$cc2" , "$cc3" ,
        "$cc4" , "$cc5" , "$cc6" , "$cc7" ,
    };
    #endif
    const char *g_ir2_names[] = {
        "$zero" , "$ra" , "$tp" , "$sp" , "$a0" , "$a1" , "$a2" , "$a3" ,
        "$a4"   , "$a5" , "$a6" , "$a7" , "$t0" , "$t1" , "$t2" , "$t3" ,
        "$t4"   , "$t5" , "$t6" , "$t7" , "$t8" , "$x"  , "$fp" , "$s0" ,
        "$s1"   , "$s2" , "$s3" , "$s4" , "$s5" , "$s6" , "$s7" , "$s8" ,
        "", "", "", "", "", "", "", "", /*  32-39 */
        "$fa0"  , "$fa1"  , "$fa2"  , "$fa3"  ,
        "$fa4"  , "$fa5"  , "$fa6"  , "$fa7"  ,
        "$ft0"  , "$ft1"  , "$ft2"  , "$ft3"  ,
        "$ft4"  , "$ft5"  , "$ft6"  , "$ft7"  ,
        "$ft8"  , "$ft9"  , "$ft10" , "$ft11" ,
        "$ft12" , "$ft13" , "$ft14" , "$ft15" ,
        "$fs0"  , "$fs1"  , "$fs2"  , "$fs3"  ,
        "$fs4"  , "$fs5"  , "$fs6"  , "$s7"   ,
        "", "", "", "", "", "", "", "", /* 72-79 */
        "NONE", "GPR", "SCR", "FPR", "FCSR", "IMMD", "IMMH", "LABEL",/* 80-87 */
        "", "", "", "", "", "", "", "", /* 88-95 */
        "ax", "sx", "zx", "bx", "AD", "AX", "", "",  /* 96-103 */
        "", "", "", "", "", "", "", "", /* 104-111 */
        "", "", "", "", "", "", "", "", /* 112-119 */
        "", "", "", "", "", "", "", "", /* 120-127 */
        /* clang-format on */
        //"", "label", "", "", "", "", "", "", /* 128-135 */
        //"", "", "", "", "", "", "", "", /* 136-143 */
        //"", "", "", "", "", "", "", "", /* 144-151 */
        "invalid",
        "   -->",
        "-------",
        "dup",
        "mov64",
        "mov32_sx",
        "mov32_zx",
        "mov_addrx",

        "add_addr",
        "add_addrx",
        "sub_addr",
        "sub_addrx",
        "addi_addr",
        "addi_addrx",
        "subi_addr",
        "subi_addrx",
        "load_addr",
        "load_addrx",
        "store_addr",
        "store_addrx",

        "subi",
        "subiu",
        "dsubi",
        "dsubiu",

        "gr2scr",
        "scr2gr",
        "clo.w",
        "clz.w",
        "cto.w",
        "ctz.w",
        "clo.d",
        "clz.d",
        "cto.d",
        "ctz.d",
        "revb.2h",
        "revb.4h",
        "revb.2w",
        "revb.d",
        "revh.2w",
        "revh.d",
        "bitrev.4b",
        "bitrev.8b",
        "bitrev.w",
        "bitrev.d",
        "ext.w.h",
        "ext.w.b",
        "rdtimel.w",
        "rdtimeh.w",
        "rdtime.d",
        "cpucfg",
        "x86mttop",
        "x86mftop",
        "x86loope",
        "x86loopne",
        "x86inc.b",
        "x86inc.h",
        "x86inc.w",
        "x86inc.d",
        "x86dec.b",
        "x86dec.h",
        "x86dec.w",
        "x86dec.d",
        "x86settm",
        "x86clrtm",
        "x86inctop",
        "x86dectop",
        "asrtle.d",
        "asrtgt.d",
        "alsl.w",
        "alsl.wu",
        "bytepick.w",
        "bytepick.d",
        "add.w",
        "add.d",
        "sub.w",
        "sub.d",
        "slt",
        "sltu",
        "maskeqz",
        "masknez",
        "nor",
        "and",
        "or",
        "xor",
        "orn",
        "andn",
        "sll.w",
        "srl.w",
        "sra.w",
        "sll.d",
        "srl.d",
        "sra.d",
        "rotr.b",
        "rotr.h",
        "rotr.w",
        "rotr.d",
        "mul.w",
        "mulh.w",
        "mulh.wu",
        "mul.d",
        "mulh.d",
        "mulh.du",
        "mulw.d.w",
        "mulw.d.wu",
        "div.w",
        "mod.w",
        "div.wu",
        "mod.wu",
        "div.d",
        "mod.d",
        "div.du",
        "mod.du",
        "crc.w.b.w",
        "crc.w.h.w",
        "crc.w.w.w",
        "crc.w.d.w",
        "crcc.w.b.w",
        "crcc.w.h.w",
        "crcc.w.w.w",
        "crcc.w.d.w",
        "addu12i.w",
        "addu12i.d",
        "break",
        "dbgcall",
        "syscall",
        "hypcall",
        "alsl.d",
        "adc.b",
        "adc.h",
        "adc.w",
        "adc.d",
        "sbc.b",
        "sbc.h",
        "sbc.w",
        "sbc.d",
        "rcr.b",
        "rcr.h",
        "rcr.w",
        "rcr.d",
        "armmove",
        "setx86j",
        "setarmj",
        "armadd.w",
        "armsub.w",
        "armadc.w",
        "armsbc.w",
        "armand.w",
        "armor.w",
        "armxor.w",
        "armsll.w",
        "armsrl.w",
        "armsra.w",
        "armrotr.w",
        "armslli.w",
        "armsrli.w",
        "armsrai.w",
        "armrotri.w",
        "x86mul.b",
        "x86mul.h",
        "x86mul.w",
        "x86mul.d",
        "x86mul.bu",
        "x86mul.hu",
        "x86mul.wu",
        "x86mul.du",
        "x86add.wu",
        "x86add.du",
        "x86sub.wu",
        "x86sub.du",
        "x86add.b",
        "x86add.h",
        "x86add.w",
        "x86add.d",
        "x86sub.b",
        "x86sub.h",
        "x86sub.w",
        "x86sub.d",
        "x86adc.b",
        "x86adc.h",
        "x86adc.w",
        "x86adc.d",
        "x86sbc.b",
        "x86sbc.h",
        "x86sbc.w",
        "x86sbc.d",
        "x86sll.b",
        "x86sll.h",
        "x86sll.w",
        "x86sll.d",
        "x86srl.b",
        "x86srl.h",
        "x86srl.w",
        "x86srl.d",
        "x86sra.b",
        "x86sra.h",
        "x86sra.w",
        "x86sra.d",
        "x86rotr.b",
        "x86rotr.h",
        "x86rotr.d",
        "x86rotr.w",
        "x86rotl.b",
        "x86rotl.h",
        "x86rotl.w",
        "x86rotl.d",
        "x86rcr.b",
        "x86rcr.h",
        "x86rcr.w",
        "x86rcr.d",
        "x86rcl.b",
        "x86rcl.h",
        "x86rcl.w",
        "x86rcl.d",
        "x86and.b",
        "x86and.h",
        "x86and.w",
        "x86and.d",
        "x86or.b",
        "x86or.h",
        "x86or.w",
        "x86or.d",
        "x86xor.b",
        "x86xor.h",
        "x86xor.w",
        "x86xor.d",
        "armnot.w",
        "armmov.w",
        "armmov.d",
        "armrrx.w",
        "slli.w",
        "slli.d",
        "srli.w",
        "srli.d",
        "srai.w",
        "srai.d",
        "rotri.b",
        "rotri.h",
        "rotri.w",
        "rotri.d",
        "rcri.b",
        "rcri.h",
        "rcri.w",
        "rcri.d",
        "x86slli.b",
        "x86slli.h",
        "x86slli.w",
        "x86slli.d",
        "x86srli.b",
        "x86srli.h",
        "x86srli.w",
        "x86srli.d",
        "x86srai.b",
        "x86srai.h",
        "x86srai.w",
        "x86srai.d",
        "x86rotri.b",
        "x86rotri.h",
        "x86rotri.w",
        "x86rotri.d",
        "x86rcri.b",
        "x86rcri.h",
        "x86rcri.w",
        "x86rcri.d",
        "x86rotli.b",
        "x86rotli.h",
        "x86rotli.w",
        "x86rotli.d",
        "x86rcli.b",
        "x86rcli.h",
        "x86rcli.w",
        "x86rcli.d",
        "x86settag",
        "x86mfflag",
        "x86mtflag",
        "armmfflag",
        "armmtflag",
        "bstrins.w",
        "bstrpick.w",
        "bstrins.d",
        "bstrpick.d",

    /* 12 bit imm instruction */
        "slti",
        "sltui",
        "addi.w",
        "addi.d",
        "lu52i.d",
        "andi",
        "ori",
        "xori",

    /* long imm instruction */
        "addu16i.d",
        "lu12i.w",
        "lu32i.d",
        "pcaddi",
        "pcalau12i",
        "pcaddu12i",
        "pcaddu18i",

    /* memory access instruction */
        "ll.w",
        "sc.w",
        "ll.d",
        "sc.d",
        "ldptr.w",
        "stptr.w",
        "ldptr.d",
        "stptr.d",
        "ld.b",
        "ld.h",
        "ld.w",
        "ld.d",
        "st.b",
        "st.h",
        "st.w",
        "st.d",
        "ld.bu",
        "ld.hu",
        "ld.wu",
        "preld",
        "fld.s",
        "fst.s",
        "fld.d",
        "fst.d",
        "vld",
        "vst",
        "xvld",
        "xvst",
        "ldl.w",
        "ldr.w",
        "ldl.d",
        "ldr.d",
        "stl.w",
        "str.w",
        "stl.d",
        "str.d",
        "vldrepl.d",
        "vldrepl.w",
        "vldrepl.h",
        "vldrepl.b",
        "vstelm.d",
        "vstelm.w",
        "vstelm.h",
        "vstelm.b",
        "xvldrepl.d",
        "xvldrepl.w",
        "xvldrepl.h",
        "xvldrepl.b",
        "xvstelm.d",
        "xvstelm.w",
        "xvstelm.h",
        "xvstelm.b",
        "ldx.b",
        "ldx.h",
        "ldx.w",
        "ldx.d",
        "stx.b",
        "stx.h",
        "stx.w",
        "stx.d",
        "ldx.bu",
        "ldx.hu",
        "ldx.wu",
        "preldx",
        "fldx.s",
        "fldx.d",
        "fstx.s",
        "fstx.d",
        "vldx",
        "vstx",
        "xvldx",
        "xvstx",
        "amswap.w",
        "amswap.d",
        "amadd.w",
        "amadd.d",
        "amand.w",
        "amand.d",
        "amor.w",
        "amor.d",
        "amxor.w",
        "amxor.d",
        "ammax.w",
        "ammax.d",
        "ammin.w",
        "ammin.d",
        "ammax.wu",
        "ammax.du",
        "ammin.wu",
        "ammin.du",
        "amswap.db.w",
        "amswap.db.d",
        "amadd.db.w",
        "amadd.db.d",
        "amand.db.w",
        "amand.db.d",
        "amor.db.w",
        "amor.db.d",
        "amxor.db.w",
        "amxor.db.d",
        "ammax.db.w",
        "ammax.db.d",
        "ammin.db.w",
        "ammin.db.d",
        "ammax.db.wu",
        "ammax.db.du",
        "ammin.db.wu",
        "ammin.db.du",
        "dbar",
        "ibar",
        "fldgt.s",
        "fldgt.d",
        "fldle.s",
        "fldle.d",
        "fstgt.s",
        "fstgt.d",
        "fstle.s",
        "fstle.d",
        "ldgt.b",
        "ldgt.h",
        "ldgt.w",
        "ldgt.d",
        "ldle.b",
        "ldle.h",
        "ldle.w",
        "ldle.d",
        "stgt.b",
        "stgt.h",
        "stgt.w",
        "stgt.d",
        "stle.b",
        "stle.h",
        "stle.w",
        "stle.d",

    /* transfer instructions */
        "beqz",
        "bnez",
        "bceqz",
        "bcnez",
        "jiscr0",
        "jiscr1",
        "jirl",
        "b",
        "bl",
        "beq",
        "bne",
        "blt",
        "bge",
        "bltu",
        "bgeu",

    /* float instructions */
        "fadd.s",
        "fadd.d",
        "fsub.s",
        "fsub.d",
        "fmul.s",
        "fmul.d",
        "fdiv.s",
        "fdiv.d",
        "fmax.s",
        "fmax.d",
        "fmin.s",
        "fmin.d",
        "fmaxa.s",
        "fmaxa.d",
        "fmina.s",
        "fmina.d",
        "fscaleb.s",
        "fscaleb.d",
        "fcopysign.s",
        "fcopysign.d",
        "fabs.s",
        "fabs.d",
        "fneg.s",
        "fneg.d",
        "flogb.s",
        "flogb.d",
        "fclass.s",
        "fclass.d",
        "fsqrt.s",
        "fsqrt.d",
        "frecip.s",
        "frecip.d",
        "frsqrt.s",
        "frsqrt.d",
        "fmov.s",
        "fmov.d",
        "movgr2fr.w",
        "movgr2fr.d",
        "movgr2frh.w",
        "movfr2gr.s",
        "movfr2gr.d",
        "movfrh2gr.s",
        "movgr2fcsr",
        "movfcsr2gr",
        "movfr2cf",
        "movcf2fr",
        "movgr2cf",
        "movcf2gr",
        "fcvt.ld.d",
        "fcvt.ud.d",
        "fcvt.d.ld",
        "fcvt.s.d",
        "fcvt.d.s",
        "ftintrm.w.s",
        "ftintrm.w.d",
        "ftintrm.l.s",
        "ftintrm.l.d",
        "ftintrp.w.s",
        "ftintrp.w.d",
        "ftintrp.l.s",
        "ftintrp.l.d",
        "ftintrz.w.s",
        "ftintrz.w.d",
        "ftintrz.l.s",
        "ftintrz.l.d",
        "ftintrne.w.s",
        "ftintrne.w.d",
        "ftintrne.l.s",
        "ftintrne.l.d",
        "ftint.w.s",
        "ftint.w.d",
        "ftint.l.s",
        "ftint.l.d",
        "ffint.s.w",
        "ffint.s.l",
        "ffint.d.w",
        "ffint.d.l",
        "frint.s",
        "frint.d",

    /* operands instructions */
        "fmadd.s",
        "fmadd.d",
        "fmsub.s",
        "fmsub.d",
        "fnmadd.s",
        "fnmadd.d",
        "fnmsub.s",
        "fnmsub.d",
        "vfmadd.s",
        "vfmadd.d",
        "vfmsub.s",
        "vfmsub.d",
        "vfnmadd.s",
        "vfnmadd.d",
        "vfnmsub.s",
        "vfnmsub.d",
        "xvfmadd.s",
        "xvfmadd.d",
        "xvfmsub.s",
        "xvfmsub.d",
        "xvfnmadd.s",
        "xvfnmadd.d",
        "xvfnmsub.s",
        "xvfnmsub.d",
        "fcmp.cond.s",
        "fcmp.cond.d",
        "vfcmp.cond.s",
        "vfcmp.cond.d",
        "xvfcmp.cond.s",
        "xvfcmp.cond.d",
        "fsel",
        "vbitsel.v",
        "xvbitsel.v",
        "vshuf.b",
        "xvshuf.b",
        "vextr.v",
        "xvextr.v",
        "vfmaddsub.s",
        "vfmaddsub.d",
        "vfmsubadd.s",
        "vfmsubadd.d",
        "xvfmaddsub.s",
        "xvfmaddsub.d",
        "xvfmsubadd.s",
        "xvfmsubadd.d",
    /* vector instructions */
    "VSEQ_B",
    "VSEQ_H",
    "VSEQ_W",
    "VSEQ_D",
    "VSLE_B",
    "VSLE_H",
    "VSLE_W",
    "VSLE_D",
    "VSLE_BU",
    "VSLE_HU",
    "VSLE_WU",
    "VSLE_DU",
    "VSLT_B",
    "VSLT_H",
    "VSLT_W",
    "VSLT_D",
    "VSLT_BU",
    "VSLT_HU",
    "VSLT_WU",
    "VSLT_DU",
    "VADD_B",
    "VADD_H",
    "VADD_W",
    "VADD_D",
    "VSUB_B",
    "VSUB_H",
    "VSUB_W",
    "VSUB_D",
    "VADDW_H_H_B",
    "VADDW_W_W_H",
    "VADDW_D_D_W",
    "VADDW_H_H_BU",
    "VADDW_W_W_HU",
    "VADDW_D_D_WU",
    "VSUBW_H_H_B",
    "VSUBW_W_W_H",
    "VSUBW_D_D_W",
    "VSUBW_H_H_BU",
    "VSUBW_W_W_HU",
    "VSUBW_D_D_WU",
    "VSADDW_H_H_B",
    "VSADDW_W_W_H",
    "VSADDW_D_D_W",
    "VSADDW_HU_HU_BU",
    "VSADDW_WU_WU_HU",
    "VSADDW_DU_DU_WU",
    "VSSUBW_H_H_B",
    "VSSUBW_W_W_H",
    "VSSUBW_D_D_W",
    "VSSUBW_HU_HU_BU",
    "VSSUBW_WU_WU_HU",
    "VSSUBW_DU_DU_WU",
    "VADDWEV_H_B",
    "VADDWEV_W_H",
    "VADDWEV_D_W",
    "VADDWEV_Q_D",
    "VSUBWEV_H_B",
    "VSUBWEV_W_H",
    "VSUBWEV_D_W",
    "VSUBWEV_Q_D",
    "VADDWOD_H_B",
    "VADDWOD_W_H",
    "VADDWOD_D_W",
    "VADDWOD_Q_D",
    "VSUBWOD_H_B",
    "VSUBWOD_W_H",
    "VSUBWOD_D_W",
    "VSUBWOD_Q_D",
    "VADDWL_H_B",
    "VADDWL_W_H",
    "VADDWL_D_W",
    "VADDWL_Q_D",
    "VSUBWL_H_B",
    "VSUBWL_W_H",
    "VSUBWL_D_W",
    "VSUBWL_Q_D",
    "VADDWH_H_B",
    "VADDWH_W_H",
    "VADDWH_D_W",
    "VADDWH_Q_D",
    "VSUBWH_H_B",
    "VSUBWH_W_H",
    "VSUBWH_D_W",
    "VSUBWH_Q_D",
    "VADDWEV_H_BU",
    "VADDWEV_W_HU",
    "VADDWEV_D_WU",
    "VADDWEV_Q_DU",
    "VSUBWEV_H_BU",
    "VSUBWEV_W_HU",
    "VSUBWEV_D_WU",
    "VSUBWEV_Q_DU",
    "VADDWOD_H_BU",
    "VADDWOD_W_HU",
    "VADDWOD_D_WU",
    "VADDWOD_Q_DU",
    "VSUBWOD_H_BU",
    "VSUBWOD_W_HU",
    "VSUBWOD_D_WU",
    "VSUBWOD_Q_DU",
    "VADDWL_H_BU",
    "VADDWL_W_HU",
    "VADDWL_D_WU",
    "VADDWL_Q_DU",
    "VSUBWL_H_BU",
    "VSUBWL_W_HU",
    "VSUBWL_D_WU",
    "VSUBWL_Q_DU",
    "VADDWH_H_BU",
    "VADDWH_W_HU",
    "VADDWH_D_WU",
    "VADDWH_Q_DU",
    "VSUBWH_H_BU",
    "VSUBWH_W_HU",
    "VSUBWH_D_WU",
    "VSUBWH_Q_DU",
    "VADDWEV_H_BU_B",
    "VADDWEV_W_HU_H",
    "VADDWEV_D_WU_W",
    "VADDWEV_Q_DU_D",
    "VADDWOD_H_BU_B",
    "VADDWOD_W_HU_H",
    "VADDWOD_D_WU_W",
    "VADDWOD_Q_DU_D",
    "VADDWL_H_BU_B",
    "VADDWL_W_HU_H",
    "VADDWL_D_WU_W",
    "VADDWL_Q_DU_D",
    "VADDWH_H_BU_B",
    "VADDWH_W_HU_H",
    "VADDWH_D_WU_W",
    "VADDWH_Q_DU_D",
    "VSADD_B",
    "VSADD_H",
    "VSADD_W",
    "VSADD_D",
    "VSSUB_B",
    "VSSUB_H",
    "VSSUB_W",
    "VSSUB_D",
    "VSADD_BU",
    "VSADD_HU",
    "VSADD_WU",
    "VSADD_DU",
    "VSSUB_BU",
    "VSSUB_HU",
    "VSSUB_WU",
    "VSSUB_DU",
    "VSSUB_BU_BU_B",
    "VSSUB_HU_HU_H",
    "VSSUB_WU_WU_W",
    "VSSUB_DU_DU_D",
    "VSSUB_BU_B_BU",
    "VSSUB_HU_H_HU",
    "VSSUB_WU_W_WU",
    "VSSUB_DU_D_DU",
    "VSSUB_B_BU_BU",
    "VSSUB_H_HU_HU",
    "VSSUB_W_WU_WU",
    "VSSUB_D_DU_DU",
    "VHADDW_H_B",
    "VHADDW_W_H",
    "VHADDW_D_W",
    "VHADDW_Q_D",
    "VHSUBW_H_B",
    "VHSUBW_W_H",
    "VHSUBW_D_W",
    "VHSUBW_Q_D",
    "VHADDW_HU_BU",
    "VHADDW_WU_HU",
    "VHADDW_DU_WU",
    "VHADDW_QU_DU",
    "VHSUBW_HU_BU",
    "VHSUBW_WU_HU",
    "VHSUBW_DU_WU",
    "VHSUBW_QU_DU",
    "VADDA_B",
    "VADDA_H",
    "VADDA_W",
    "VADDA_D",
    "VSADDA_B",
    "VSADDA_H",
    "VSADDA_W",
    "VSADDA_D",
    "VABSD_B",
    "VABSD_H",
    "VABSD_W",
    "VABSD_D",
    "VABSD_BU",
    "VABSD_HU",
    "VABSD_WU",
    "VABSD_DU",
    "VAVG_B",
    "VAVG_H",
    "VAVG_W",
    "VAVG_D",
    "VAVG_BU",
    "VAVG_HU",
    "VAVG_WU",
    "VAVG_DU",
    "VAVGR_B",
    "VAVGR_H",
    "VAVGR_W",
    "VAVGR_D",
    "VAVGR_BU",
    "VAVGR_HU",
    "VAVGR_WU",
    "VAVGR_DU",
    "VHALFD_B",
    "VHALFD_H",
    "VHALFD_W",
    "VHALFD_D",
    "VHALFD_BU",
    "VHALFD_HU",
    "VHALFD_WU",
    "VHALFD_DU",
    "VMAX_B",
    "VMAX_H",
    "VMAX_W",
    "VMAX_D",
    "VMIN_B",
    "VMIN_H",
    "VMIN_W",
    "VMIN_D",
    "VMAX_BU",
    "VMAX_HU",
    "VMAX_WU",
    "VMAX_DU",
    "VMIN_BU",
    "VMIN_HU",
    "VMIN_WU",
    "VMIN_DU",
    "VMAXA_B",
    "VMAXA_H",
    "VMAXA_W",
    "VMAXA_D",
    "VMINA_B",
    "VMINA_H",
    "VMINA_W",
    "VMINA_D",
    "VSADW_H_B",
    "VSADW_W_H",
    "VSADW_D_W",
    "VSADW_H_BU",
    "VSADW_W_HU",
    "VSADW_D_WU",
    "VACCSADW_H_B",
    "VACCSADW_W_H",
    "VACCSADW_D_W",
    "VACCSADW_H_BU",
    "VACCSADW_W_HU",
    "VACCSADW_D_WU",
    "VMUL_B",
    "VMUL_H",
    "VMUL_W",
    "VMUL_D",
    "VMUH_B",
    "VMUH_H",
    "VMUH_W",
    "VMUH_D",
    "VMUH_BU",
    "VMUH_HU",
    "VMUH_WU",
    "VMUH_DU",
    "VMUH_BU_B",
    "VMUH_HU_H",
    "VMUH_WU_W",
    "VMUH_DU_D",
    "VMULXW_H_B",
    "VMULXW_W_H",
    "VMULXW_D_W",
    "VMULXW_H_BU",
    "VMULXW_W_HU",
    "VMULXW_D_WU",
    "VMULWEV_H_B",
    "VMULWEV_W_H",
    "VMULWEV_D_W",
    "VMULWEV_Q_D",
    "VMULWOD_H_B",
    "VMULWOD_W_H",
    "VMULWOD_D_W",
    "VMULWOD_Q_D",
    "VMULWL_H_B",
    "VMULWL_W_H",
    "VMULWL_D_W",
    "VMULWL_Q_D",
    "VMULWH_H_B",
    "VMULWH_W_H",
    "VMULWH_D_W",
    "VMULWH_Q_D",
    "VMULWEV_H_BU",
    "VMULWEV_W_HU",
    "VMULWEV_D_WU",
    "VMULWEV_Q_DU",
    "VMULWOD_H_BU",
    "VMULWOD_W_HU",
    "VMULWOD_D_WU",
    "VMULWOD_Q_DU",
    "VMULWL_H_BU",
    "VMULWL_W_HU",
    "VMULWL_D_WU",
    "VMULWL_Q_DU",
    "VMULWH_H_BU",
    "VMULWH_W_HU",
    "VMULWH_D_WU",
    "VMULWH_Q_DU",
    "VMULWEV_H_BU_B",
    "VMULWEV_W_HU_H",
    "VMULWEV_D_WU_W",
    "VMULWEV_Q_DU_D",
    "VMULWOD_H_BU_B",
    "VMULWOD_W_HU_H",
    "VMULWOD_D_WU_W",
    "VMULWOD_Q_DU_D",
    "VMULWL_H_BU_B",
    "VMULWL_W_HU_H",
    "VMULWL_D_WU_W",
    "VMULWL_Q_DU_D",
    "VMULWH_H_BU_B",
    "VMULWH_W_HU_H",
    "VMULWH_D_WU_W",
    "VMULWH_Q_DU_D",
    "VMADD_B",
    "VMADD_H",
    "VMADD_W",
    "VMADD_D",
    "VMSUB_B",
    "VMSUB_H",
    "VMSUB_W",
    "VMSUB_D",
    "VMADDWEV_H_B",
    "VMADDWEV_W_H",
    "VMADDWEV_D_W",
    "VMADDWEV_Q_D",
    "VMADDWOD_H_B",
    "VMADDWOD_W_H",
    "VMADDWOD_D_W",
    "VMADDWOD_Q_D",
    "VMADDWL_H_B",
    "VMADDWL_W_H",
    "VMADDWL_D_W",
    "VMADDWL_Q_D",
    "VMADDWH_H_B",
    "VMADDWH_W_H",
    "VMADDWH_D_W",
    "VMADDWH_Q_D",
    "VMADDWEV_H_BU",
    "VMADDWEV_W_HU",
    "VMADDWEV_D_WU",
    "VMADDWEV_Q_DU",
    "VMADDWOD_H_BU",
    "VMADDWOD_W_HU",
    "VMADDWOD_D_WU",
    "VMADDWOD_Q_DU",
    "VMADDWL_H_BU",
    "VMADDWL_W_HU",
    "VMADDWL_D_WU",
    "VMADDWL_Q_DU",
    "VMADDWH_H_BU",
    "VMADDWH_W_HU",
    "VMADDWH_D_WU",
    "VMADDWH_Q_DU",
    "VMADDWEV_H_BU_B",
    "VMADDWEV_W_HU_H",
    "VMADDWEV_D_WU_W",
    "VMADDWEV_Q_DU_D",
    "VMADDWOD_H_BU_B",
    "VMADDWOD_W_HU_H",
    "VMADDWOD_D_WU_W",
    "VMADDWOD_Q_DU_D",
    "VMADDWL_H_BU_B",
    "VMADDWL_W_HU_H",
    "VMADDWL_D_WU_W",
    "VMADDWL_Q_DU_D",
    "VMADDWH_H_BU_B",
    "VMADDWH_W_HU_H",
    "VMADDWH_D_WU_W",
    "VMADDWH_Q_DU_D",
    "VDP2_H_B",
    "VDP2_W_H",
    "VDP2_D_W",
    "VDP2_Q_D",
    "VDP2_HU_BU",
    "VDP2_WU_HU",
    "VDP2_DU_WU",
    "VDP2_QU_DU",
    "VDP2_H_BU_B",
    "VDP2_W_HU_H",
    "VDP2_D_WU_W",
    "VDP2_Q_DU_D",
    "VDP2ADD_H_B",
    "VDP2ADD_W_H",
    "VDP2ADD_D_W",
    "VDP2ADD_Q_D",
    "VDP2ADD_H_BU",
    "VDP2ADD_W_HU",
    "VDP2ADD_D_WU",
    "VDP2ADD_Q_DU",
    "VDP2ADD_H_BU_B",
    "VDP2ADD_W_HU_H",
    "VDP2ADD_D_WU_W",
    "VDP2ADD_Q_DU_D",
    "VDP2SUB_H_B",
    "VDP2SUB_W_H",
    "VDP2SUB_D_W",
    "VDP2SUB_Q_D",
    "VDP2SUB_H_BU",
    "VDP2SUB_W_HU",
    "VDP2SUB_D_WU",
    "VDP2SUB_Q_DU",
    "VDP4_W_B",
    "VDP4_D_H",
    "VDP4_Q_W",
    "VDP4_W_BU",
    "VDP4_D_HU",
    "VDP4_Q_WU",
    "VDP4_W_BU_B",
    "VDP4_D_HU_H",
    "VDP4_Q_WU_W",
    "VDP4ADD_W_B",
    "VDP4ADD_D_H",
    "VDP4ADD_Q_W",
    "VDP4ADD_W_BU",
    "VDP4ADD_D_HU",
    "VDP4ADD_Q_WU",
    "VDP4ADD_W_BU_B",
    "VDP4ADD_D_HU_H",
    "VDP4ADD_Q_WU_W",
    "VDIV_B",
    "VDIV_H",
    "VDIV_W",
    "VDIV_D",
    "VMOD_B",
    "VMOD_H",
    "VMOD_W",
    "VMOD_D",
    "VDIV_BU",
    "VDIV_HU",
    "VDIV_WU",
    "VDIV_DU",
    "VMOD_BU",
    "VMOD_HU",
    "VMOD_WU",
    "VMOD_DU",
    "VSLL_B",
    "VSLL_H",
    "VSLL_W",
    "VSLL_D",
    "VSRL_B",
    "VSRL_H",
    "VSRL_W",
    "VSRL_D",
    "VSRA_B",
    "VSRA_H",
    "VSRA_W",
    "VSRA_D",
    "VROTR_B",
    "VROTR_H",
    "VROTR_W",
    "VROTR_D",
    "VSRLR_B",
    "VSRLR_H",
    "VSRLR_W",
    "VSRLR_D",
    "VSRAR_B",
    "VSRAR_H",
    "VSRAR_W",
    "VSRAR_D",
    "VSRLN_B_H",
    "VSRLN_H_W",
    "VSRLN_W_D",
    "VSRAN_B_H",
    "VSRAN_H_W",
    "VSRAN_W_D",
    "VSRLRN_B_H",
    "VSRLRN_H_W",
    "VSRLRN_W_D",
    "VSRARN_B_H",
    "VSRARN_H_W",
    "VSRARN_W_D",
    "VSSRLN_B_H",
    "VSSRLN_H_W",
    "VSSRLN_W_D",
    "VSSRAN_B_H",
    "VSSRAN_H_W",
    "VSSRAN_W_D",
    "VSSRLRN_B_H",
    "VSSRLRN_H_W",
    "VSSRLRN_W_D",
    "VSSRARN_B_H",
    "VSSRARN_H_W",
    "VSSRARN_W_D",
    "VSSRLN_BU_H",
    "VSSRLN_HU_W",
    "VSSRLN_WU_D",
    "VSSRAN_BU_H",
    "VSSRAN_HU_W",
    "VSSRAN_WU_D",
    "VSSRLRN_BU_H",
    "VSSRLRN_HU_W",
    "VSSRLRN_WU_D",
    "VSSRARN_BU_H",
    "VSSRARN_HU_W",
    "VSSRARN_WU_D",
    "VBITCLR_B",
    "VBITCLR_H",
    "VBITCLR_W",
    "VBITCLR_D",
    "VBITSET_B",
    "VBITSET_H",
    "VBITSET_W",
    "VBITSET_D",
    "VBITREV_B",
    "VBITREV_H",
    "VBITREV_W",
    "VBITREV_D",
    "VBSTRC12_B",
    "VBSTRC12_H",
    "VBSTRC12_W",
    "VBSTRC12_D",
    "VBSTRC21_B",
    "VBSTRC21_H",
    "VBSTRC21_W",
    "VBSTRC21_D",
    "VPACKEV_B",
    "VPACKEV_H",
    "VPACKEV_W",
    "VPACKEV_D",
    "VPACKOD_B",
    "VPACKOD_H",
    "VPACKOD_W",
    "VPACKOD_D",
    "VILVL_B",
    "VILVL_H",
    "VILVL_W",
    "VILVL_D",
    "VILVH_B",
    "VILVH_H",
    "VILVH_W",
    "VILVH_D",
    "VPICKEV_B",
    "VPICKEV_H",
    "VPICKEV_W",
    "VPICKEV_D",
    "VPICKOD_B",
    "VPICKOD_H",
    "VPICKOD_W",
    "VPICKOD_D",
    "VREPLVE_B",
    "VREPLVE_H",
    "VREPLVE_W",
    "VREPLVE_D",
    "VEXTRCOL_B",
    "VEXTRCOL_H",
    "VEXTRCOL_W",
    "VEXTRCOL_D",
    "VAND_V",
    "VOR_V",
    "VXOR_V",
    "VNOR_V",
    "VANDN_V",
    "VORN_V",
    "VRANDSIGN_B",
    "VRANDSIGN_H",
    "VRORSIGN_B",
    "VRORSIGN_H",
    "VFRSTP_B",
    "VFRSTP_H",
    "VCLRSTRR_V",
    "VCLRSTRV_V",
    "VADD_Q",
    "VSUB_Q",
    "VSIGNCOV_B",
    "VSIGNCOV_H",
    "VSIGNCOV_W",
    "VSIGNCOV_D",
    "VFADD_S",
    "VFADD_D",
    "VFSUB_S",
    "VFSUB_D",
    "VFADDSUB_S",
    "VFADDSUB_D",
    "VFSUBADD_S",
    "VFSUBADD_D",
    "VFMUL_S",
    "VFMUL_D",
    "VFDIV_S",
    "VFDIV_D",
    "VFMAX_S",
    "VFMAX_D",
    "VFMIN_S",
    "VFMIN_D",
    "VFMAXA_S",
    "VFMAXA_D",
    "VFMINA_S",
    "VFMINA_D",
    "VFSCALEB_S",
    "VFSCALEB_D",
    "VFCVT_H_S",
    "VFCVT_S_D",
    "VFFINT_S_L",
    "VFTINT_W_D",
    "VFTINTRM_W_D",
    "VFTINTRP_W_D",
    "VFTINTRZ_W_D",
    "VFTINTRNE_W_D",
    "VHADD4_H_BU",
    "VSHUF4_W",
    "VSHUF2_D",
    "AES128_ENC",
    "AES128_DEC",
    "AES192_ENC",
    "AES192_DEC",
    "AES256_ENC",
    "AES256_DEC",
    "AES_KG",
    "AES_FR_ENC",
    "AES_FR_DEC",
    "AES_LR_ENC",
    "AES_LR_DEC",
    "AES_MC_ENC",
    "AES_MC_DEC",
    "AES_SB_ENC",
    "AES_SB_DEC",
    "AES_SR_ENC",
    "AES_SR_DEC",
    "MD5_MS",
    "MD5_4R",
    "SHA1_MS_1",
    "SHA1_MS_2",
    "SHA1_HASH_4R",
    "SHA256_MS_1",
    "SHA256_MS_2",
    "SHA256_HASH_2R",
    "SHA512_MS_1",
    "SHA512_MS_2",
    "SHA512_HASH_R_1",
    "SHA512_HASH_R_2",
    "VPMUL_W",
    "VPMUL_D",
    "VPMUH_W",
    "VPMUH_D",
    "VPMULACC_W",
    "VPMULACC_D",
    "VPMUHACC_W",
    "VPMUHACC_D",
    "VPMULWL_H_B",
    "VPMULWL_W_H",
    "VPMULWL_D_W",
    "VPMULWL_Q_D",
    "VPMULWH_H_B",
    "VPMULWH_W_H",
    "VPMULWH_D_W",
    "VPMULWH_Q_D",
    "VPMADDWL_H_B",
    "VPMADDWL_W_H",
    "VPMADDWL_D_W",
    "VPMADDWL_Q_D",
    "VPMADDWH_H_B",
    "VPMADDWH_W_H",
    "VPMADDWH_D_W",
    "VPMADDWH_Q_D",
    "VPDP2_Q_D",
    "VPDP2ADD_Q_D",
    "VCDP4_RE_D_H",
    "VCDP4_IM_D_H",
    "VCDP4ADD_RE_D_H",
    "VCDP4ADD_IM_D_H",
    "VCDP2_RE_Q_W",
    "VCDP2_IM_Q_W",
    "VCDP2ADD_RE_Q_W",
    "VCDP2ADD_IM_Q_W",
    "VSIGNSEL_W",
    "VSIGNSEL_D",
    "VSHUF_H",
    "VSHUF_W",
    "VSHUF_D",
    "VSEQI_B",
    "VSEQI_H",
    "VSEQI_W",
    "VSEQI_D",
    "VSLEI_B",
    "VSLEI_H",
    "VSLEI_W",
    "VSLEI_D",
    "VSLEI_BU",
    "VSLEI_HU",
    "VSLEI_WU",
    "VSLEI_DU",
    "VSLTI_B",
    "VSLTI_H",
    "VSLTI_W",
    "VSLTI_D",
    "VSLTI_BU",
    "VSLTI_HU",
    "VSLTI_WU",
    "VSLTI_DU",
    "VADDI_BU",
    "VADDI_HU",
    "VADDI_WU",
    "VADDI_DU",
    "VSUBI_BU",
    "VSUBI_HU",
    "VSUBI_WU",
    "VSUBI_DU",
    "VBSLL_V",
    "VBSRL_V",
    "VMAXI_B",
    "VMAXI_H",
    "VMAXI_W",
    "VMAXI_D",
    "VMINI_B",
    "VMINI_H",
    "VMINI_W",
    "VMINI_D",
    "VMAXI_BU",
    "VMAXI_HU",
    "VMAXI_WU",
    "VMAXI_DU",
    "VMINI_BU",
    "VMINI_HU",
    "VMINI_WU",
    "VMINI_DU",
    "VRANDSIGNI_B",
    "VRANDSIGNI_H",
    "VRORSIGNI_B",
    "VRORSIGNI_H",
    "VFRSTPI_B",
    "VFRSTPI_H",
    "VCLRSTRI_V",
    "VMEPATMSK_V",
    "VCLO_B",
    "VCLO_H",
    "VCLO_W",
    "VCLO_D",
    "VCLZ_B",
    "VCLZ_H",
    "VCLZ_W",
    "VCLZ_D",
    "VPCNT_B",
    "VPCNT_H",
    "VPCNT_W",
    "VPCNT_D",
    "VNEG_B",
    "VNEG_H",
    "VNEG_W",
    "VNEG_D",
    "VMSKLTZ_B",
    "VMSKLTZ_H",
    "VMSKLTZ_W",
    "VMSKLTZ_D",
    "VMSKGEZ_B",
    "VMSKNZ_B",
    "VMSKCOPY_B",
    "VMSKFILL_B",
    "VFRSTM_B",
    "VFRSTM_H",
    "VSETEQZ_V",
    "VSETNEZ_V",
    "VSETANYEQZ_B",
    "VSETANYEQZ_H",
    "VSETANYEQZ_W",
    "VSETANYEQZ_D",
    "VSETALLNEZ_B",
    "VSETALLNEZ_H",
    "VSETALLNEZ_W",
    "VSETALLNEZ_D",
    "VFLOGB_S",
    "VFLOGB_D",
    "VFCLASS_S",
    "VFCLASS_D",
    "VFSQRT_S",
    "VFSQRT_D",
    "VFRECIP_S",
    "VFRECIP_D",
    "VFRSQRT_S",
    "VFRSQRT_D",
    "VFRINT_S",
    "VFRINT_D",
    "VFRINTRM_S",
    "VFRINTRM_D",
    "VFRINTRP_S",
    "VFRINTRP_D",
    "VFRINTRZ_S",
    "VFRINTRZ_D",
    "VFRINTRNE_S",
    "VFRINTRNE_D",
    "VEXTL_W_B",
    "VEXTL_D_B",
    "VEXTL_D_H",
    "VEXTL_W_BU",
    "VEXTL_D_BU",
    "VEXTL_D_HU",
    "VHADD8_D_BU",
    "VHMINPOS_W_HU",
    "VHMINPOS_D_HU",
    "VHMINPOS_Q_HU",
    "VCLRTAIL_B",
    "VCLRTAIL_H",
    "VFCVTL_S_H",
    "VFCVTH_S_H",
    "VFCVTL_D_S",
    "VFCVTH_D_S",
    "VFFINT_S_W",
    "VFFINT_S_WU",
    "VFFINT_D_L",
    "VFFINT_D_LU",
    "VFFINTL_D_W",
    "VFFINTH_D_W",
    "VFTINT_W_S",
    "VFTINT_L_D",
    "VFTINTRM_W_S",
    "VFTINTRM_L_D",
    "VFTINTRP_W_S",
    "VFTINTRP_L_D",
    "VFTINTRZ_W_S",
    "VFTINTRZ_L_D",
    "VFTINTRNE_W_S",
    "VFTINTRNE_L_D",
    "VFTINT_WU_S",
    "VFTINT_LU_D",
    "VFTINTRZ_WU_S",
    "VFTINTRZ_LU_D",
    "VFTINTL_L_S",
    "VFTINTH_L_S",
    "VFTINTRML_L_S",
    "VFTINTRMH_L_S",
    "VFTINTRPL_L_S",
    "VFTINTRPH_L_S",
    "VFTINTRZL_L_S",
    "VFTINTRZH_L_S",
    "VFTINTRNEL_L_S",
    "VFTINTRNEH_L_S",
    "VEXTH_H_B",
    "VEXTH_W_H",
    "VEXTH_D_W",
    "VEXTH_Q_D",
    "VEXTH_HU_BU",
    "VEXTH_WU_HU",
    "VEXTH_DU_WU",
    "VEXTH_QU_DU",
    "VREPLGR2VR_B",
    "VREPLGR2VR_H",
    "VREPLGR2VR_W",
    "VREPLGR2VR_D",
    "VROTRI_B",
    "VROTRI_H",
    "VROTRI_W",
    "VROTRI_D",
    "VSRLRI_B",
    "VSRLRI_H",
    "VSRLRI_W",
    "VSRLRI_D",
    "VSRARI_B",
    "VSRARI_H",
    "VSRARI_W",
    "VSRARI_D",
    "VINSGR2VR_B",
    "VINSGR2VR_H",
    "VINSGR2VR_W",
    "VINSGR2VR_D",
    "VPICKVE2GR_B",
    "VPICKVE2GR_H",
    "VPICKVE2GR_W",
    "VPICKVE2GR_D",
    "VPICKVE2GR_BU",
    "VPICKVE2GR_HU",
    "VPICKVE2GR_WU",
    "VPICKVE2GR_DU",
    "VREPLVEI_B",
    "VREPLVEI_H",
    "VREPLVEI_W",
    "VREPLVEI_D",
    "VEXTRCOLI_B",
    "VEXTRCOLI_H",
    "VEXTRCOLI_W",
    "VEXTRCOLI_D",
    "VSLLWIL_H_B",
    "VSLLWIL_W_H",
    "VSLLWIL_D_W",
    "VEXTL_Q_D",
    "VSLLWIL_HU_BU",
    "VSLLWIL_WU_HU",
    "VSLLWIL_DU_WU",
    "VEXTL_QU_DU",
    "VBITCLRI_B",
    "VBITCLRI_H",
    "VBITCLRI_W",
    "VBITCLRI_D",
    "VBITSETI_B",
    "VBITSETI_H",
    "VBITSETI_W",
    "VBITSETI_D",
    "VBITREVI_B",
    "VBITREVI_H",
    "VBITREVI_W",
    "VBITREVI_D",
    "VBSTRC12I_B",
    "VBSTRC12I_H",
    "VBSTRC12I_W",
    "VBSTRC12I_D",
    "VBSTRC21I_B",
    "VBSTRC21I_H",
    "VBSTRC21I_W",
    "VBSTRC21I_D",
    "VSAT_B",
    "VSAT_H",
    "VSAT_W",
    "VSAT_D",
    "VSAT_BU",
    "VSAT_HU",
    "VSAT_WU",
    "VSAT_DU",
    "VSLLI_B",
    "VSLLI_H",
    "VSLLI_W",
    "VSLLI_D",
    "VSRLI_B",
    "VSRLI_H",
    "VSRLI_W",
    "VSRLI_D",
    "VSRAI_B",
    "VSRAI_H",
    "VSRAI_W",
    "VSRAI_D",
    "VSRLRNENI_B_H",
    "VSRLRNENI_H_W",
    "VSRLRNENI_W_D",
    "VSRLRNENI_D_Q",
    "VSRARNENI_B_H",
    "VSRARNENI_H_W",
    "VSRARNENI_W_D",
    "VSRARNENI_D_Q",
    "VSRLNI_B_H",
    "VSRLNI_H_W",
    "VSRLNI_W_D",
    "VSRLNI_D_Q",
    "VSRLRNI_B_H",
    "VSRLRNI_H_W",
    "VSRLRNI_W_D",
    "VSRLRNI_D_Q",
    "VSSRLNI_B_H",
    "VSSRLNI_H_W",
    "VSSRLNI_W_D",
    "VSSRLNI_D_Q",
    "VSSRLNI_BU_H",
    "VSSRLNI_HU_W",
    "VSSRLNI_WU_D",
    "VSSRLNI_DU_Q",
    "VSSRLRNI_B_H",
    "VSSRLRNI_H_W",
    "VSSRLRNI_W_D",
    "VSSRLRNI_D_Q",
    "VSSRLRNI_BU_H",
    "VSSRLRNI_HU_W",
    "VSSRLRNI_WU_D",
    "VSSRLRNI_DU_Q",
    "VSRANI_B_H",
    "VSRANI_H_W",
    "VSRANI_W_D",
    "VSRANI_D_Q",
    "VSRARNI_B_H",
    "VSRARNI_H_W",
    "VSRARNI_W_D",
    "VSRARNI_D_Q",
    "VSSRANI_B_H",
    "VSSRANI_H_W",
    "VSSRANI_W_D",
    "VSSRANI_D_Q",
    "VSSRANI_BU_H",
    "VSSRANI_HU_W",
    "VSSRANI_WU_D",
    "VSSRANI_DU_Q",
    "VSSRARNI_B_H",
    "VSSRARNI_H_W",
    "VSSRARNI_W_D",
    "VSSRARNI_D_Q",
    "VSSRARNI_BU_H",
    "VSSRARNI_HU_W",
    "VSSRARNI_WU_D",
    "VSSRARNI_DU_Q",
    "VSSRLRNENI_B_H",
    "VSSRLRNENI_H_W",
    "VSSRLRNENI_W_D",
    "VSSRLRNENI_D_Q",
    "VSSRLRNENI_BU_H",
    "VSSRLRNENI_HU_W",
    "VSSRLRNENI_WU_D",
    "VSSRLRNENI_DU_Q",
    "VSSRARNENI_B_H",
    "VSSRARNENI_H_W",
    "VSSRARNENI_W_D",
    "VSSRARNENI_D_Q",
    "VSSRARNENI_BU_H",
    "VSSRARNENI_HU_W",
    "VSSRARNENI_WU_D",
    "VSSRARNENI_DU_Q",
    "VEXTRINS_D",
    "VEXTRINS_W",
    "VEXTRINS_H",
    "VEXTRINS_B",
    "VSHUF4I_B",
    "VSHUF4I_H",
    "VSHUF4I_W",
    "VSHUF4I_D",
    "VSHUFI1_B",
    "VSHUFI2_B",
    "VSHUFI3_B",
    "VSHUFI4_B",
    "VSHUFI1_H",
    "VSHUFI2_H",
    "VSELI_H",
    "VSELI_W",
    "VSELI_D",
    "VBITSELI_B",
    "VBITMVZI_B",
    "VBITMVNZI_B",
    "VANDI_B",
    "VORI_B",
    "VXORI_B",
    "VNORI_B",
    "VLDI",
    "VPERMI_W",
    "XVSEQ_B",
    "XVSEQ_H",
    "XVSEQ_W",
    "XVSEQ_D",
    "XVSLE_B",
    "XVSLE_H",
    "XVSLE_W",
    "XVSLE_D",
    "XVSLE_BU",
    "XVSLE_HU",
    "XVSLE_WU",
    "XVSLE_DU",
    "XVSLT_B",
    "XVSLT_H",
    "XVSLT_W",
    "XVSLT_D",
    "XVSLT_BU",
    "XVSLT_HU",
    "XVSLT_WU",
    "XVSLT_DU",
    "XVADD_B",
    "XVADD_H",
    "XVADD_W",
    "XVADD_D",
    "XVSUB_B",
    "XVSUB_H",
    "XVSUB_W",
    "XVSUB_D",
    "XVADDW_H_H_B",
    "XVADDW_W_W_H",
    "XVADDW_D_D_W",
    "XVADDW_H_H_BU",
    "XVADDW_W_W_HU",
    "XVADDW_D_D_WU",
    "XVSUBW_H_H_B",
    "XVSUBW_W_W_H",
    "XVSUBW_D_D_W",
    "XVSUBW_H_H_BU",
    "XVSUBW_W_W_HU",
    "XVSUBW_D_D_WU",
    "XVSADDW_H_H_B",
    "XVSADDW_W_W_H",
    "XVSADDW_D_D_W",
    "XVSADDW_HU_HU_BU",
    "XVSADDW_WU_WU_HU",
    "XVSADDW_DU_DU_WU",
    "XVSSUBW_H_H_B",
    "XVSSUBW_W_W_H",
    "XVSSUBW_D_D_W",
    "XVSSUBW_HU_HU_BU",
    "XVSSUBW_WU_WU_HU",
    "XVSSUBW_DU_DU_WU",
    "XVADDWEV_H_B",
    "XVADDWEV_W_H",
    "XVADDWEV_D_W",
    "XVADDWEV_Q_D",
    "XVSUBWEV_H_B",
    "XVSUBWEV_W_H",
    "XVSUBWEV_D_W",
    "XVSUBWEV_Q_D",
    "XVADDWOD_H_B",
    "XVADDWOD_W_H",
    "XVADDWOD_D_W",
    "XVADDWOD_Q_D",
    "XVSUBWOD_H_B",
    "XVSUBWOD_W_H",
    "XVSUBWOD_D_W",
    "XVSUBWOD_Q_D",
    "XVADDWL_H_B",
    "XVADDWL_W_H",
    "XVADDWL_D_W",
    "XVADDWL_Q_D",
    "XVSUBWL_H_B",
    "XVSUBWL_W_H",
    "XVSUBWL_D_W",
    "XVSUBWL_Q_D",
    "XVADDWH_H_B",
    "XVADDWH_W_H",
    "XVADDWH_D_W",
    "XVADDWH_Q_D",
    "XVSUBWH_H_B",
    "XVSUBWH_W_H",
    "XVSUBWH_D_W",
    "XVSUBWH_Q_D",
    "XVADDWEV_H_BU",
    "XVADDWEV_W_HU",
    "XVADDWEV_D_WU",
    "XVADDWEV_Q_DU",
    "XVSUBWEV_H_BU",
    "XVSUBWEV_W_HU",
    "XVSUBWEV_D_WU",
    "XVSUBWEV_Q_DU",
    "XVADDWOD_H_BU",
    "XVADDWOD_W_HU",
    "XVADDWOD_D_WU",
    "XVADDWOD_Q_DU",
    "XVSUBWOD_H_BU",
    "XVSUBWOD_W_HU",
    "XVSUBWOD_D_WU",
    "XVSUBWOD_Q_DU",
    "XVADDWL_H_BU",
    "XVADDWL_W_HU",
    "XVADDWL_D_WU",
    "XVADDWL_Q_DU",
    "XVSUBWL_H_BU",
    "XVSUBWL_W_HU",
    "XVSUBWL_D_WU",
    "XVSUBWL_Q_DU",
    "XVADDWH_H_BU",
    "XVADDWH_W_HU",
    "XVADDWH_D_WU",
    "XVADDWH_Q_DU",
    "XVSUBWH_H_BU",
    "XVSUBWH_W_HU",
    "XVSUBWH_D_WU",
    "XVSUBWH_Q_DU",
    "XVADDWEV_H_BU_B",
    "XVADDWEV_W_HU_H",
    "XVADDWEV_D_WU_W",
    "XVADDWEV_Q_DU_D",
    "XVADDWOD_H_BU_B",
    "XVADDWOD_W_HU_H",
    "XVADDWOD_D_WU_W",
    "XVADDWOD_Q_DU_D",
    "XVADDWL_H_BU_B",
    "XVADDWL_W_HU_H",
    "XVADDWL_D_WU_W",
    "XVADDWL_Q_DU_D",
    "XVADDWH_H_BU_B",
    "XVADDWH_W_HU_H",
    "XVADDWH_D_WU_W",
    "XVADDWH_Q_DU_D",
    "XVSADD_B",
    "XVSADD_H",
    "XVSADD_W",
    "XVSADD_D",
    "XVSSUB_B",
    "XVSSUB_H",
    "XVSSUB_W",
    "XVSSUB_D",
    "XVSADD_BU",
    "XVSADD_HU",
    "XVSADD_WU",
    "XVSADD_DU",
    "XVSSUB_BU",
    "XVSSUB_HU",
    "XVSSUB_WU",
    "XVSSUB_DU",
    "XVSSUB_BU_BU_B",
    "XVSSUB_HU_HU_H",
    "XVSSUB_WU_WU_W",
    "XVSSUB_DU_DU_D",
    "XVSSUB_BU_B_BU",
    "XVSSUB_HU_H_HU",
    "XVSSUB_WU_W_WU",
    "XVSSUB_DU_D_DU",
    "XVSSUB_B_BU_BU",
    "XVSSUB_H_HU_HU",
    "XVSSUB_W_WU_WU",
    "XVSSUB_D_DU_DU",
    "XVHADDW_H_B",
    "XVHADDW_W_H",
    "XVHADDW_D_W",
    "XVHADDW_Q_D",
    "XVHSUBW_H_B",
    "XVHSUBW_W_H",
    "XVHSUBW_D_W",
    "XVHSUBW_Q_D",
    "XVHADDW_HU_BU",
    "XVHADDW_WU_HU",
    "XVHADDW_DU_WU",
    "XVHADDW_QU_DU",
    "XVHSUBW_HU_BU",
    "XVHSUBW_WU_HU",
    "XVHSUBW_DU_WU",
    "XVHSUBW_QU_DU",
    "XVADDA_B",
    "XVADDA_H",
    "XVADDA_W",
    "XVADDA_D",
    "XVSADDA_B",
    "XVSADDA_H",
    "XVSADDA_W",
    "XVSADDA_D",
    "XVABSD_B",
    "XVABSD_H",
    "XVABSD_W",
    "XVABSD_D",
    "XVABSD_BU",
    "XVABSD_HU",
    "XVABSD_WU",
    "XVABSD_DU",
    "XVAVG_B",
    "XVAVG_H",
    "XVAVG_W",
    "XVAVG_D",
    "XVAVG_BU",
    "XVAVG_HU",
    "XVAVG_WU",
    "XVAVG_DU",
    "XVAVGR_B",
    "XVAVGR_H",
    "XVAVGR_W",
    "XVAVGR_D",
    "XVAVGR_BU",
    "XVAVGR_HU",
    "XVAVGR_WU",
    "XVAVGR_DU",
    "XVHALFD_B",
    "XVHALFD_H",
    "XVHALFD_W",
    "XVHALFD_D",
    "XVHALFD_BU",
    "XVHALFD_HU",
    "XVHALFD_WU",
    "XVHALFD_DU",
    "XVMAX_B",
    "XVMAX_H",
    "XVMAX_W",
    "XVMAX_D",
    "XVMIN_B",
    "XVMIN_H",
    "XVMIN_W",
    "XVMIN_D",
    "XVMAX_BU",
    "XVMAX_HU",
    "XVMAX_WU",
    "XVMAX_DU",
    "XVMIN_BU",
    "XVMIN_HU",
    "XVMIN_WU",
    "XVMIN_DU",
    "XVMAXA_B",
    "XVMAXA_H",
    "XVMAXA_W",
    "XVMAXA_D",
    "XVMINA_B",
    "XVMINA_H",
    "XVMINA_W",
    "XVMINA_D",
    "XVSADW_H_B",
    "XVSADW_W_H",
    "XVSADW_D_W",
    "XVSADW_H_BU",
    "XVSADW_W_HU",
    "XVSADW_D_WU",
    "XVACCSADW_H_B",
    "XVACCSADW_W_H",
    "XVACCSADW_D_W",
    "XVACCSADW_H_BU",
    "XVACCSADW_W_HU",
    "XVACCSADW_D_WU",
    "XVMUL_B",
    "XVMUL_H",
    "XVMUL_W",
    "XVMUL_D",
    "XVMUH_B",
    "XVMUH_H",
    "XVMUH_W",
    "XVMUH_D",
    "XVMUH_BU",
    "XVMUH_HU",
    "XVMUH_WU",
    "XVMUH_DU",
    "XVMUH_BU_B",
    "XVMUH_HU_H",
    "XVMUH_WU_W",
    "XVMUH_DU_D",
    "XVMULXW_H_B",
    "XVMULXW_W_H",
    "XVMULXW_D_W",
    "XVMULXW_H_BU",
    "XVMULXW_W_HU",
    "XVMULXW_D_WU",
    "XVMULWEV_H_B",
    "XVMULWEV_W_H",
    "XVMULWEV_D_W",
    "XVMULWEV_Q_D",
    "XVMULWOD_H_B",
    "XVMULWOD_W_H",
    "XVMULWOD_D_W",
    "XVMULWOD_Q_D",
    "XVMULWL_H_B",
    "XVMULWL_W_H",
    "XVMULWL_D_W",
    "XVMULWL_Q_D",
    "XVMULWH_H_B",
    "XVMULWH_W_H",
    "XVMULWH_D_W",
    "XVMULWH_Q_D",
    "XVMULWEV_H_BU",
    "XVMULWEV_W_HU",
    "XVMULWEV_D_WU",
    "XVMULWEV_Q_DU",
    "XVMULWOD_H_BU",
    "XVMULWOD_W_HU",
    "XVMULWOD_D_WU",
    "XVMULWOD_Q_DU",
    "XVMULWL_H_BU",
    "XVMULWL_W_HU",
    "XVMULWL_D_WU",
    "XVMULWL_Q_DU",
    "XVMULWH_H_BU",
    "XVMULWH_W_HU",
    "XVMULWH_D_WU",
    "XVMULWH_Q_DU",
    "XVMULWEV_H_BU_B",
    "XVMULWEV_W_HU_H",
    "XVMULWEV_D_WU_W",
    "XVMULWEV_Q_DU_D",
    "XVMULWOD_H_BU_B",
    "XVMULWOD_W_HU_H",
    "XVMULWOD_D_WU_W",
    "XVMULWOD_Q_DU_D",
    "XVMULWL_H_BU_B",
    "XVMULWL_W_HU_H",
    "XVMULWL_D_WU_W",
    "XVMULWL_Q_DU_D",
    "XVMULWH_H_BU_B",
    "XVMULWH_W_HU_H",
    "XVMULWH_D_WU_W",
    "XVMULWH_Q_DU_D",
    "XVMADD_B",
    "XVMADD_H",
    "XVMADD_W",
    "XVMADD_D",
    "XVMSUB_B",
    "XVMSUB_H",
    "XVMSUB_W",
    "XVMSUB_D",
    "XVMADDWEV_H_B",
    "XVMADDWEV_W_H",
    "XVMADDWEV_D_W",
    "XVMADDWEV_Q_D",
    "XVMADDWOD_H_B",
    "XVMADDWOD_W_H",
    "XVMADDWOD_D_W",
    "XVMADDWOD_Q_D",
    "XVMADDWL_H_B",
    "XVMADDWL_W_H",
    "XVMADDWL_D_W",
    "XVMADDWL_Q_D",
    "XVMADDWH_H_B",
    "XVMADDWH_W_H",
    "XVMADDWH_D_W",
    "XVMADDWH_Q_D",
    "XVMADDWEV_H_BU",
    "XVMADDWEV_W_HU",
    "XVMADDWEV_D_WU",
    "XVMADDWEV_Q_DU",
    "XVMADDWOD_H_BU",
    "XVMADDWOD_W_HU",
    "XVMADDWOD_D_WU",
    "XVMADDWOD_Q_DU",
    "XVMADDWL_H_BU",
    "XVMADDWL_W_HU",
    "XVMADDWL_D_WU",
    "XVMADDWL_Q_DU",
    "XVMADDWH_H_BU",
    "XVMADDWH_W_HU",
    "XVMADDWH_D_WU",
    "XVMADDWH_Q_DU",
    "XVMADDWEV_H_BU_B",
    "XVMADDWEV_W_HU_H",
    "XVMADDWEV_D_WU_W",
    "XVMADDWEV_Q_DU_D",
    "XVMADDWOD_H_BU_B",
    "XVMADDWOD_W_HU_H",
    "XVMADDWOD_D_WU_W",
    "XVMADDWOD_Q_DU_D",
    "XVMADDWL_H_BU_B",
    "XVMADDWL_W_HU_H",
    "XVMADDWL_D_WU_W",
    "XVMADDWL_Q_DU_D",
    "XVMADDWH_H_BU_B",
    "XVMADDWH_W_HU_H",
    "XVMADDWH_D_WU_W",
    "XVMADDWH_Q_DU_D",
    "XVDP2_H_B",
    "XVDP2_W_H",
    "XVDP2_D_W",
    "XVDP2_Q_D",
    "XVDP2_HU_BU",
    "XVDP2_WU_HU",
    "XVDP2_DU_WU",
    "XVDP2_QU_DU",
    "XVDP2_H_BU_B",
    "XVDP2_W_HU_H",
    "XVDP2_D_WU_W",
    "XVDP2_Q_DU_D",
    "XVDP2ADD_H_B",
    "XVDP2ADD_W_H",
    "XVDP2ADD_D_W",
    "XVDP2ADD_Q_D",
    "XVDP2ADD_H_BU",
    "XVDP2ADD_W_HU",
    "XVDP2ADD_D_WU",
    "XVDP2ADD_Q_DU",
    "XVDP2ADD_H_BU_B",
    "XVDP2ADD_W_HU_H",
    "XVDP2ADD_D_WU_W",
    "XVDP2ADD_Q_DU_D",
    "XVDP2SUB_H_B",
    "XVDP2SUB_W_H",
    "XVDP2SUB_D_W",
    "XVDP2SUB_Q_D",
    "XVDP2SUB_H_BU",
    "XVDP2SUB_W_HU",
    "XVDP2SUB_D_WU",
    "XVDP2SUB_Q_DU",
    "XVDP4_W_B",
    "XVDP4_D_H",
    "XVDP4_Q_W",
    "XVDP4_W_BU",
    "XVDP4_D_HU",
    "XVDP4_Q_WU",
    "XVDP4_W_BU_B",
    "XVDP4_D_HU_H",
    "XVDP4_Q_WU_W",
    "XVDP4ADD_W_B",
    "XVDP4ADD_D_H",
    "XVDP4ADD_Q_W",
    "XVDP4ADD_W_BU",
    "XVDP4ADD_D_HU",
    "XVDP4ADD_Q_WU",
    "XVDP4ADD_W_BU_B",
    "XVDP4ADD_D_HU_H",
    "XVDP4ADD_Q_WU_W",
    "XVDIV_B",
    "XVDIV_H",
    "XVDIV_W",
    "XVDIV_D",
    "XVMOD_B",
    "XVMOD_H",
    "XVMOD_W",
    "XVMOD_D",
    "XVDIV_BU",
    "XVDIV_HU",
    "XVDIV_WU",
    "XVDIV_DU",
    "XVMOD_BU",
    "XVMOD_HU",
    "XVMOD_WU",
    "XVMOD_DU",
    "XVSLL_B",
    "XVSLL_H",
    "XVSLL_W",
    "XVSLL_D",
    "XVSRL_B",
    "XVSRL_H",
    "XVSRL_W",
    "XVSRL_D",
    "XVSRA_B",
    "XVSRA_H",
    "XVSRA_W",
    "XVSRA_D",
    "XVROTR_B",
    "XVROTR_H",
    "XVROTR_W",
    "XVROTR_D",
    "XVSRLR_B",
    "XVSRLR_H",
    "XVSRLR_W",
    "XVSRLR_D",
    "XVSRAR_B",
    "XVSRAR_H",
    "XVSRAR_W",
    "XVSRAR_D",
    "XVSRLN_B_H",
    "XVSRLN_H_W",
    "XVSRLN_W_D",
    "XVSRAN_B_H",
    "XVSRAN_H_W",
    "XVSRAN_W_D",
    "XVSRLRN_B_H",
    "XVSRLRN_H_W",
    "XVSRLRN_W_D",
    "XVSRARN_B_H",
    "XVSRARN_H_W",
    "XVSRARN_W_D",
    "XVSSRLN_B_H",
    "XVSSRLN_H_W",
    "XVSSRLN_W_D",
    "XVSSRAN_B_H",
    "XVSSRAN_H_W",
    "XVSSRAN_W_D",
    "XVSSRLRN_B_H",
    "XVSSRLRN_H_W",
    "XVSSRLRN_W_D",
    "XVSSRARN_B_H",
    "XVSSRARN_H_W",
    "XVSSRARN_W_D",
    "XVSSRLN_BU_H",
    "XVSSRLN_HU_W",
    "XVSSRLN_WU_D",
    "XVSSRAN_BU_H",
    "XVSSRAN_HU_W",
    "XVSSRAN_WU_D",
    "XVSSRLRN_BU_H",
    "XVSSRLRN_HU_W",
    "XVSSRLRN_WU_D",
    "XVSSRARN_BU_H",
    "XVSSRARN_HU_W",
    "XVSSRARN_WU_D",
    "XVBITCLR_B",
    "XVBITCLR_H",
    "XVBITCLR_W",
    "XVBITCLR_D",
    "XVBITSET_B",
    "XVBITSET_H",
    "XVBITSET_W",
    "XVBITSET_D",
    "XVBITREV_B",
    "XVBITREV_H",
    "XVBITREV_W",
    "XVBITREV_D",
    "XVBSTRC12_B",
    "XVBSTRC12_H",
    "XVBSTRC12_W",
    "XVBSTRC12_D",
    "XVBSTRC21_B",
    "XVBSTRC21_H",
    "XVBSTRC21_W",
    "XVBSTRC21_D",
    "XVPACKEV_B",
    "XVPACKEV_H",
    "XVPACKEV_W",
    "XVPACKEV_D",
    "XVPACKOD_B",
    "XVPACKOD_H",
    "XVPACKOD_W",
    "XVPACKOD_D",
    "XVILVL_B",
    "XVILVL_H",
    "XVILVL_W",
    "XVILVL_D",
    "XVILVH_B",
    "XVILVH_H",
    "XVILVH_W",
    "XVILVH_D",
    "XVPICKEV_B",
    "XVPICKEV_H",
    "XVPICKEV_W",
    "XVPICKEV_D",
    "XVPICKOD_B",
    "XVPICKOD_H",
    "XVPICKOD_W",
    "XVPICKOD_D",
    "XVREPLVE_B",
    "XVREPLVE_H",
    "XVREPLVE_W",
    "XVREPLVE_D",
    "XVEXTRCOL_B",
    "XVEXTRCOL_H",
    "XVEXTRCOL_W",
    "XVEXTRCOL_D",
    "XVAND_V",
    "XVOR_V",
    "XVXOR_V",
    "XVNOR_V",
    "XVANDN_V",
    "XVORN_V",
    "XVRANDSIGN_B",
    "XVRANDSIGN_H",
    "XVRORSIGN_B",
    "XVRORSIGN_H",
    "XVFRSTP_B",
    "XVFRSTP_H",
    "XVCLRSTRR_V",
    "XVCLRSTRV_V",
    "XVADD_Q",
    "XVSUB_Q",
    "XVSIGNCOV_B",
    "XVSIGNCOV_H",
    "XVSIGNCOV_W",
    "XVSIGNCOV_D",
    "XVFADD_S",
    "XVFADD_D",
    "XVFSUB_S",
    "XVFSUB_D",
    "XVFADDSUB_S",
    "XVFADDSUB_D",
    "XVFSUBADD_S",
    "XVFSUBADD_D",
    "XVFMUL_S",
    "XVFMUL_D",
    "XVFDIV_S",
    "XVFDIV_D",
    "XVFMAX_S",
    "XVFMAX_D",
    "XVFMIN_S",
    "XVFMIN_D",
    "XVFMAXA_S",
    "XVFMAXA_D",
    "XVFMINA_S",
    "XVFMINA_D",
    "XVFSCALEB_S",
    "XVFSCALEB_D",
    "XVFCVT_H_S",
    "XVFCVT_S_D",
    "XVFFINT_S_L",
    "XVFTINT_W_D",
    "XVFTINTRM_W_D",
    "XVFTINTRP_W_D",
    "XVFTINTRZ_W_D",
    "XVFTINTRNE_W_D",
    "XVHADD4_H_BU",
    "XVSHUF4_W",
    "XVSHUF2_D",
    "XVPMUL_W",
    "XVPMUL_D",
    "XVPMUH_W",
    "XVPMUH_D",
    "XVPMULACC_W",
    "XVPMULACC_D",
    "XVPMUHACC_W",
    "XVPMUHACC_D",
    "XVPMULWL_H_B",
    "XVPMULWL_W_H",
    "XVPMULWL_D_W",
    "XVPMULWL_Q_D",
    "XVPMULWH_H_B",
    "XVPMULWH_W_H",
    "XVPMULWH_D_W",
    "XVPMULWH_Q_D",
    "XVPMADDWL_H_B",
    "XVPMADDWL_W_H",
    "XVPMADDWL_D_W",
    "XVPMADDWL_Q_D",
    "XVPMADDWH_H_B",
    "XVPMADDWH_W_H",
    "XVPMADDWH_D_W",
    "XVPMADDWH_Q_D",
    "XVPDP2_Q_D",
    "XVPDP2ADD_Q_D",
    "XVCDP4_RE_D_H",
    "XVCDP4_IM_D_H",
    "XVCDP4ADD_RE_D_H",
    "XVCDP4ADD_IM_D_H",
    "XVCDP2_RE_Q_W",
    "XVCDP2_IM_Q_W",
    "XVCDP2ADD_RE_Q_W",
    "XVCDP2ADD_IM_Q_W",
    "XVSIGNSEL_W",
    "XVSIGNSEL_D",
    "XVSHUF_H",
    "XVSHUF_W",
    "XVSHUF_D",
    "XVPERM_W",
    "XVSEQI_B",
    "XVSEQI_H",
    "XVSEQI_W",
    "XVSEQI_D",
    "XVSLEI_B",
    "XVSLEI_H",
    "XVSLEI_W",
    "XVSLEI_D",
    "XVSLEI_BU",
    "XVSLEI_HU",
    "XVSLEI_WU",
    "XVSLEI_DU",
    "XVSLTI_B",
    "XVSLTI_H",
    "XVSLTI_W",
    "XVSLTI_D",
    "XVSLTI_BU",
    "XVSLTI_HU",
    "XVSLTI_WU",
    "XVSLTI_DU",
    "XVADDI_BU",
    "XVADDI_HU",
    "XVADDI_WU",
    "XVADDI_DU",
    "XVSUBI_BU",
    "XVSUBI_HU",
    "XVSUBI_WU",
    "XVSUBI_DU",
    "XVBSLL_V",
    "XVBSRL_V",
    "XVMAXI_B",
    "XVMAXI_H",
    "XVMAXI_W",
    "XVMAXI_D",
    "XVMINI_B",
    "XVMINI_H",
    "XVMINI_W",
    "XVMINI_D",
    "XVMAXI_BU",
    "XVMAXI_HU",
    "XVMAXI_WU",
    "XVMAXI_DU",
    "XVMINI_BU",
    "XVMINI_HU",
    "XVMINI_WU",
    "XVMINI_DU",
    "XVRANDSIGNI_B",
    "XVRANDSIGNI_H",
    "XVRORSIGNI_B",
    "XVRORSIGNI_H",
    "XVFRSTPI_B",
    "XVFRSTPI_H",
    "XVCLRSTRI_V",
    "XVMEPATMSK_V",
    "XVCLO_B",
    "XVCLO_H",
    "XVCLO_W",
    "XVCLO_D",
    "XVCLZ_B",
    "XVCLZ_H",
    "XVCLZ_W",
    "XVCLZ_D",
    "XVPCNT_B",
    "XVPCNT_H",
    "XVPCNT_W",
    "XVPCNT_D",
    "XVNEG_B",
    "XVNEG_H",
    "XVNEG_W",
    "XVNEG_D",
    "XVMSKLTZ_B",
    "XVMSKLTZ_H",
    "XVMSKLTZ_W",
    "XVMSKLTZ_D",
    "XVMSKGEZ_B",
    "XVMSKNZ_B",
    "XVMSKCOPY_B",
    "XVMSKFILL_B",
    "XVFRSTM_B",
    "XVFRSTM_H",
    "XVSETEQZ_V",
    "XVSETNEZ_V",
    "XVSETANYEQZ_B",
    "XVSETANYEQZ_H",
    "XVSETANYEQZ_W",
    "XVSETANYEQZ_D",
    "XVSETALLNEZ_B",
    "XVSETALLNEZ_H",
    "XVSETALLNEZ_W",
    "XVSETALLNEZ_D",
    "XVFLOGB_S",
    "XVFLOGB_D",
    "XVFCLASS_S",
    "XVFCLASS_D",
    "XVFSQRT_S",
    "XVFSQRT_D",
    "XVFRECIP_S",
    "XVFRECIP_D",
    "XVFRSQRT_S",
    "XVFRSQRT_D",
    "XVFRINT_S",
    "XVFRINT_D",
    "XVFRINTRM_S",
    "XVFRINTRM_D",
    "XVFRINTRP_S",
    "XVFRINTRP_D",
    "XVFRINTRZ_S",
    "XVFRINTRZ_D",
    "XVFRINTRNE_S",
    "XVFRINTRNE_D",
    "XVEXTL_W_B",
    "XVEXTL_D_B",
    "XVEXTL_D_H",
    "XVEXTL_W_BU",
    "XVEXTL_D_BU",
    "XVEXTL_D_HU",
    "XVHADD8_D_BU",
    "XVHMINPOS_W_HU",
    "XVHMINPOS_D_HU",
    "XVHMINPOS_Q_HU",
    "XVCLRTAIL_B",
    "XVCLRTAIL_H",
    "XVFCVTL_S_H",
    "XVFCVTH_S_H",
    "XVFCVTL_D_S",
    "XVFCVTH_D_S",
    "XVFFINT_S_W",
    "XVFFINT_S_WU",
    "XVFFINT_D_L",
    "XVFFINT_D_LU",
    "XVFFINTL_D_W",
    "XVFFINTH_D_W",
    "XVFTINT_W_S",
    "XVFTINT_L_D",
    "XVFTINTRM_W_S",
    "XVFTINTRM_L_D",
    "XVFTINTRP_W_S",
    "XVFTINTRP_L_D",
    "XVFTINTRZ_W_S",
    "XVFTINTRZ_L_D",
    "XVFTINTRNE_W_S",
    "XVFTINTRNE_L_D",
    "XVFTINT_WU_S",
    "XVFTINT_LU_D",
    "XVFTINTRZ_WU_S",
    "XVFTINTRZ_LU_D",
    "XVFTINTL_L_S",
    "XVFTINTH_L_S",
    "XVFTINTRML_L_S",
    "XVFTINTRMH_L_S",
    "XVFTINTRPL_L_S",
    "XVFTINTRPH_L_S",
    "XVFTINTRZL_L_S",
    "XVFTINTRZH_L_S",
    "XVFTINTRNEL_L_S",
    "XVFTINTRNEH_L_S",
    "XVEXTH_H_B",
    "XVEXTH_W_H",
    "XVEXTH_D_W",
    "XVEXTH_Q_D",
    "XVEXTH_HU_BU",
    "XVEXTH_WU_HU",
    "XVEXTH_DU_WU",
    "XVEXTH_QU_DU",
    "XVREPLGR2VR_B",
    "XVREPLGR2VR_H",
    "XVREPLGR2VR_W",
    "XVREPLGR2VR_D",
    "VEXT2XV_H_B",
    "VEXT2XV_W_B",
    "VEXT2XV_D_B",
    "VEXT2XV_W_H",
    "VEXT2XV_D_H",
    "VEXT2XV_D_W",
    "VEXT2XV_HU_BU",
    "VEXT2XV_WU_BU",
    "VEXT2XV_DU_BU",
    "VEXT2XV_WU_HU",
    "VEXT2XV_DU_HU",
    "VEXT2XV_DU_WU",
    "XVHSELI_D",
    "XVROTRI_B",
    "XVROTRI_H",
    "XVROTRI_W",
    "XVROTRI_D",
    "XVSRLRI_B",
    "XVSRLRI_H",
    "XVSRLRI_W",
    "XVSRLRI_D",
    "XVSRARI_B",
    "XVSRARI_H",
    "XVSRARI_W",
    "XVSRARI_D",
    "XVINSGR2VR_W",
    "XVINSGR2VR_D",
    "XVPICKVE2GR_W",
    "XVPICKVE2GR_D",
    "XVPICKVE2GR_WU",
    "XVPICKVE2GR_DU",
    "XVREPL128VEI_B",
    "XVREPL128VEI_H",
    "XVREPL128VEI_W",
    "XVREPL128VEI_D",
    "XVEXTRCOLI_B",
    "XVEXTRCOLI_H",
    "XVEXTRCOLI_W",
    "XVEXTRCOLI_D",
    "XVINSVE0_W",
    "XVINSVE0_D",
    "XVPICKVE_W",
    "XVPICKVE_D",
    "XVREPLVE0_B",
    "XVREPLVE0_H",
    "XVREPLVE0_W",
    "XVREPLVE0_D",
    "XVREPLVE0_Q",
    "XVSLLWIL_H_B",
    "XVSLLWIL_W_H",
    "XVSLLWIL_D_W",
    "XVEXTL_Q_D",
    "XVSLLWIL_HU_BU",
    "XVSLLWIL_WU_HU",
    "XVSLLWIL_DU_WU",
    "XVEXTL_QU_DU",
    "XVBITCLRI_B",
    "XVBITCLRI_H",
    "XVBITCLRI_W",
    "XVBITCLRI_D",
    "XVBITSETI_B",
    "XVBITSETI_H",
    "XVBITSETI_W",
    "XVBITSETI_D",
    "XVBITREVI_B",
    "XVBITREVI_H",
    "XVBITREVI_W",
    "XVBITREVI_D",
    "XVBSTRC12I_B",
    "XVBSTRC12I_H",
    "XVBSTRC12I_W",
    "XVBSTRC12I_D",
    "XVBSTRC21I_B",
    "XVBSTRC21I_H",
    "XVBSTRC21I_W",
    "XVBSTRC21I_D",
    "XVSAT_B",
    "XVSAT_H",
    "XVSAT_W",
    "XVSAT_D",
    "XVSAT_BU",
    "XVSAT_HU",
    "XVSAT_WU",
    "XVSAT_DU",
    "XVSLLI_B",
    "XVSLLI_H",
    "XVSLLI_W",
    "XVSLLI_D",
    "XVSRLI_B",
    "XVSRLI_H",
    "XVSRLI_W",
    "XVSRLI_D",
    "XVSRAI_B",
    "XVSRAI_H",
    "XVSRAI_W",
    "XVSRAI_D",
    "XVSRLRNENI_B_H",
    "XVSRLRNENI_H_W",
    "XVSRLRNENI_W_D",
    "XVSRLRNENI_D_Q",
    "XVSRARNENI_B_H",
    "XVSRARNENI_H_W",
    "XVSRARNENI_W_D",
    "XVSRARNENI_D_Q",
    "XVSRLNI_B_H",
    "XVSRLNI_H_W",
    "XVSRLNI_W_D",
    "XVSRLNI_D_Q",
    "XVSRLRNI_B_H",
    "XVSRLRNI_H_W",
    "XVSRLRNI_W_D",
    "XVSRLRNI_D_Q",
    "XVSSRLNI_B_H",
    "XVSSRLNI_H_W",
    "XVSSRLNI_W_D",
    "XVSSRLNI_D_Q",
    "XVSSRLNI_BU_H",
    "XVSSRLNI_HU_W",
    "XVSSRLNI_WU_D",
    "XVSSRLNI_DU_Q",
    "XVSSRLRNI_B_H",
    "XVSSRLRNI_H_W",
    "XVSSRLRNI_W_D",
    "XVSSRLRNI_D_Q",
    "XVSSRLRNI_BU_H",
    "XVSSRLRNI_HU_W",
    "XVSSRLRNI_WU_D",
    "XVSSRLRNI_DU_Q",
    "XVSRANI_B_H",
    "XVSRANI_H_W",
    "XVSRANI_W_D",
    "XVSRANI_D_Q",
    "XVSRARNI_B_H",
    "XVSRARNI_H_W",
    "XVSRARNI_W_D",
    "XVSRARNI_D_Q",
    "XVSSRANI_B_H",
    "XVSSRANI_H_W",
    "XVSSRANI_W_D",
    "XVSSRANI_D_Q",
    "XVSSRANI_BU_H",
    "XVSSRANI_HU_W",
    "XVSSRANI_WU_D",
    "XVSSRANI_DU_Q",
    "XVSSRARNI_B_H",
    "XVSSRARNI_H_W",
    "XVSSRARNI_W_D",
    "XVSSRARNI_D_Q",
    "XVSSRARNI_BU_H",
    "XVSSRARNI_HU_W",
    "XVSSRARNI_WU_D",
    "XVSSRARNI_DU_Q",
    "XVSSRLRNENI_B_H",
    "XVSSRLRNENI_H_W",
    "XVSSRLRNENI_W_D",
    "XVSSRLRNENI_D_Q",
    "XVSSRLRNENI_BU_H",
    "XVSSRLRNENI_HU_W",
    "XVSSRLRNENI_WU_D",
    "XVSSRLRNENI_DU_Q",
    "XVSSRARNENI_B_H",
    "XVSSRARNENI_H_W",
    "XVSSRARNENI_W_D",
    "XVSSRARNENI_D_Q",
    "XVSSRARNENI_BU_H",
    "XVSSRARNENI_HU_W",
    "XVSSRARNENI_WU_D",
    "XVSSRARNENI_DU_Q",
    "XVEXTRINS_D",
    "XVEXTRINS_W",
    "XVEXTRINS_H",
    "XVEXTRINS_B",
    "XVSHUF4I_B",
    "XVSHUF4I_H",
    "XVSHUF4I_W",
    "XVSHUF4I_D",
    "XVSHUFI1_B",
    "XVSHUFI2_B",
    "XVSHUFI3_B",
    "XVSHUFI4_B",
    "XVSHUFI1_H",
    "XVSHUFI2_H",
    "XVSELI_H",
    "XVSELI_W",
    "XVSELI_D",
    "XVBITSELI_B",
    "XVBITMVZI_B",
    "XVBITMVNZI_B",
    "XVANDI_B",
    "XVORI_B",
    "XVXORI_B",
    "XVNORI_B",
    "XVLDI",
    "XVPERMI_W",
    "XVPERMI_D",
    "XVPERMI_Q",

    /* unimplemented instructions */
    "FADD_H",
    "FSUB_H",
    "FMUL_H",
    "FDIV_H",
    "FMAX_H",
    "FMIN_H",
    "FMAXA_H",
    "FMINA_H",
    "FSCALEB_H",
    "FCOPYSIGN_H",
    "FABS_H",
    "FNEG_H",
    "FLOGB_H",
    "FCLASS_H",
    "FSQRT_H",
    "FRECIP_H",
    "FRSQRT_H",
    "FRECIPE_H",
    "FRECIPE_S",
    "FRECIPE_D",
    "FRSQRTE_H",
    "FRSQRTE_S",
    "FRSQRTE_D",
    "FMOV_H",
    "MOVGR2FR_H",
    "MOVFR2GR_H",
    "FCVTRM_H_S",
    "FCVTRM_H_D",
    "FCVTRM_H_Q",
    "FCVTRM_S_H",
    "FCVTRM_S_D",
    "FCVTRM_S_Q",
    "FCVTRM_D_H",
    "FCVTRM_D_S",
    "FCVTRM_D_Q",
    "FCVTRM_Q_H",
    "FCVTRM_Q_S",
    "FCVTRM_Q_D",
    "FCVTRP_H_S",
    "FCVTRP_H_D",
    "FCVTRP_H_Q",
    "FCVTRP_S_H",
    "FCVTRP_S_D",
    "FCVTRP_S_Q",
    "FCVTRP_D_H",
    "FCVTRP_D_S",
    "FCVTRP_D_Q",
    "FCVTRP_Q_H",
    "FCVTRP_Q_S",
    "FCVTRP_Q_D",
    "FCVTRZ_H_S",
    "FCVTRZ_H_D",
    "FCVTRZ_H_Q",
    "FCVTRZ_S_H",
    "FCVTRZ_S_D",
    "FCVTRZ_S_Q",
    "FCVTRZ_D_H",
    "FCVTRZ_D_S",
    "FCVTRZ_D_Q",
    "FCVTRZ_Q_H",
    "FCVTRZ_Q_S",
    "FCVTRZ_Q_D",
    "FCVTRNE_H_S",
    "FCVTRNE_H_D",
    "FCVTRNE_H_Q",
    "FCVTRNE_S_H",
    "FCVTRNE_S_D",
    "FCVTRNE_S_Q",
    "FCVTRNE_D_H",
    "FCVTRNE_D_S",
    "FCVTRNE_D_Q",
    "FCVTRNE_Q_H",
    "FCVTRNE_Q_S",
    "FCVTRNE_Q_D",
    "FCVT_H_S",
    "FCVT_H_D",
    "FCVT_H_Q",
    "FCVT_S_H",
    "FCVT_S_Q",
    "FCVT_D_H",
    "FCVT_D_Q",
    "FCVT_Q_H",
    "FCVT_Q_S",
    "FCVT_Q_D",
    "FTINTRM_W_H",
    "FTINTRM_W_Q",
    "FTINTRM_WU_H",
    "FTINTRM_WU_S",
    "FTINTRM_WU_D",
    "FTINTRM_WU_Q",
    "FTINTRM_L_H",
    "FTINTRM_L_Q",
    "FTINTRM_LU_H",
    "FTINTRM_LU_S",
    "FTINTRM_LU_D",
    "FTINTRM_LU_Q",
    "FTINTRP_W_H",
    "FTINTRP_W_Q",
    "FTINTRP_WU_H",
    "FTINTRP_WU_S",
    "FTINTRP_WU_D",
    "FTINTRP_WU_Q",
    "FTINTRP_L_H",
    "FTINTRP_L_Q",
    "FTINTRP_LU_H",
    "FTINTRP_LU_S",
    "FTINTRP_LU_D",
    "FTINTRP_LU_Q",
    "FTINTRZ_W_H",
    "FTINTRZ_W_Q",
    "FTINTRZ_WU_H",
    "FTINTRZ_WU_S",
    "FTINTRZ_WU_D",
    "FTINTRZ_WU_Q",
    "FTINTRZ_L_H",
    "FTINTRZ_L_Q",
    "FTINTRZ_LU_H",
    "FTINTRZ_LU_S",
    "FTINTRZ_LU_D",
    "FTINTRZ_LU_Q",
    "FTINTRNE_W_H",
    "FTINTRNE_W_Q",
    "FTINTRNE_WU_H",
    "FTINTRNE_WU_S",
    "FTINTRNE_WU_D",
    "FTINTRNE_WU_Q",
    "FTINTRNE_L_H",
    "FTINTRNE_L_Q",
    "FTINTRNE_LU_H",
    "FTINTRNE_LU_S",
    "FTINTRNE_LU_D",
    "FTINTRNE_LU_Q",
    "FTINT_W_H",
    "FTINT_W_Q",
    "FTINT_WU_H",
    "FTINT_WU_S",
    "FTINT_WU_D",
    "FTINT_WU_Q",
    "FTINT_L_H",
    "FTINT_L_Q",
    "FTINT_LU_H",
    "FTINT_LU_S",
    "FTINT_LU_D",
    "FTINT_LU_Q",
    "FFINTRM_H_W",
    "FFINTRM_H_WU",
    "FFINTRM_H_L",
    "FFINTRM_H_LU",
    "FFINTRM_S_W",
    "FFINTRM_S_WU",
    "FFINTRM_S_L",
    "FFINTRM_S_LU",
    "FFINTRM_D_W",
    "FFINTRM_D_WU",
    "FFINTRM_D_L",
    "FFINTRM_D_LU",
    "FFINTRM_Q_W",
    "FFINTRM_Q_WU",
    "FFINTRM_Q_L",
    "FFINTRM_Q_LU",
    "FFINTRP_H_W",
    "FFINTRP_H_WU",
    "FFINTRP_H_L",
    "FFINTRP_H_LU",
    "FFINTRP_S_W",
    "FFINTRP_S_WU",
    "FFINTRP_S_L",
    "FFINTRP_S_LU",
    "FFINTRP_D_W",
    "FFINTRP_D_WU",
    "FFINTRP_D_L",
    "FFINTRP_D_LU",
    "FFINTRP_Q_W",
    "FFINTRP_Q_WU",
    "FFINTRP_Q_L",
    "FFINTRP_Q_LU",
    "FFINTRZ_H_W",
    "FFINTRZ_H_WU",
    "FFINTRZ_H_L",
    "FFINTRZ_H_LU",
    "FFINTRZ_S_W",
    "FFINTRZ_S_WU",
    "FFINTRZ_S_L",
    "FFINTRZ_S_LU",
    "FFINTRZ_D_W",
    "FFINTRZ_D_WU",
    "FFINTRZ_D_L",
    "FFINTRZ_D_LU",
    "FFINTRZ_Q_W",
    "FFINTRZ_Q_WU",
    "FFINTRZ_Q_L",
    "FFINTRZ_Q_LU",
    "FFINTRNE_H_W",
    "FFINTRNE_H_WU",
    "FFINTRNE_H_L",
    "FFINTRNE_H_LU",
    "FFINTRNE_S_W",
    "FFINTRNE_S_WU",
    "FFINTRNE_S_L",
    "FFINTRNE_S_LU",
    "FFINTRNE_D_W",
    "FFINTRNE_D_WU",
    "FFINTRNE_D_L",
    "FFINTRNE_D_LU",
    "FFINTRNE_Q_W",
    "FFINTRNE_Q_WU",
    "FFINTRNE_Q_L",
    "FFINTRNE_Q_LU",
    "FFINT_H_W",
    "FFINT_H_WU",
    "FFINT_H_L",
    "FFINT_H_LU",
    "FFINT_S_WU",
    "FFINT_S_LU",
    "FFINT_D_WU",
    "FFINT_D_LU",
    "FFINT_Q_W",
    "FFINT_Q_WU",
    "FFINT_Q_L",
    "FFINT_Q_LU",
    "FRINTRM_H",
    "FRINTRM_S",
    "FRINTRM_D",
    "FRINTRM_Q",
    "FRINTRP_H",
    "FRINTRP_S",
    "FRINTRP_D",
    "FRINTRP_Q",
    "FRINTRZ_H",
    "FRINTRZ_S",
    "FRINTRZ_D",
    "FRINTRZ_Q",
    "FRINTRNE_H",
    "FRINTRNE_S",
    "FRINTRNE_D",
    "FRINTRNE_Q",
    "FRINT_H",
    "FRINT_Q",
    "FMADD_H",
    "FMSUB_H",
    "FNMADD_H",
    "FNMSUB_H",
    "VFMADD_H",
    "VFMADD_Q",
    "VFMSUB_H",
    "VFMSUB_Q",
    "VFNMADD_H",
    "VFNMADD_Q",
    "VFNMSUB_H",
    "VFNMSUB_Q",
    "XVFMADD_H",
    "XVFMADD_Q",
    "XVFMSUB_H",
    "XVFMSUB_Q",
    "XVFNMADD_H",
    "XVFNMADD_Q",
    "XVFNMSUB_H",
    "XVFNMSUB_Q",
    "FCMP_cond_H",
    "VFCMP_cond_H",
    "VFCMP_cond_Q",
    "XVFCMP_cond_H",
    "XVFCMP_cond_Q",
    "VFMADDSUB_H",
    "VFMADDSUB_Q",
    "VFMSUBADD_H",
    "VFMSUBADD_Q",
    "XVFMADDSUB_H",
    "XVFMADDSUB_Q",
    "XVFMSUBADD_H",
    "XVFMSUBADD_Q",
    
    /* unimplemented vector instructions */
    "VADDW_Q_Q_D",
    "VADDW_Q_Q_DU",
    "VSUBW_Q_Q_D",
    "VSUBW_Q_Q_DU",
    "VSADDW_Q_Q_D",
    "VSADDW_QU_QU_DU",
    "VSSUBW_Q_Q_D",
    "VSSUBW_QU_QU_DU",
    "VFADD_H",
    "VFADD_Q",
    "VFSUB_H",
    "VFSUB_Q",
    "VFADDSUB_H",
    "VFADDSUB_Q",
    "VFSUBADD_H",
    "VFSUBADD_Q",
    "VFMUL_H",
    "VFMUL_Q",
    "VFDIV_H",
    "VFDIV_Q",
    "VFMAX_H",
    "VFMAX_Q",
    "VFMIN_H",
    "VFMIN_Q",
    "VFMAXA_H",
    "VFMAXA_Q",
    "VFMINA_H",
    "VFMINA_Q",
    "VFSCALEB_H",
    "VFSCALEB_Q",
    "VFCVT_D_Q",
    "VFFINT_H_W",
    "VFFINT_H_WU",
    "VFFINT_S_LU",
    "VFTINT_WU_D",
    "VFTINTRM_WU_D",
    "VFTINTRP_WU_D",
    "VFTINTRZ_WU_D",
    "VFTINTRNE_WU_D",
    "VMSKGEZ_H",
    "VMSKGEZ_W",
    "VMSKGEZ_D",
    "VMSKNZ_H",
    "VMSKNZ_W",
    "VMSKNZ_D",
    "VMSKCOPY_H",
    "VMSKCOPY_W",
    "VMSKCOPY_D",
    "VMSKFILL_H",
    "VMSKFILL_W",
    "VMSKFILL_D",
    "VFLOGB_H",
    "VFLOGB_Q",
    "VFCLASS_H",
    "VFCLASS_Q",
    "VFSQRT_H",
    "VFSQRT_Q",
    "VFRECIP_H",
    "VFRECIP_Q",
    "VFRSQRT_H",
    "VFRSQRT_Q",
    "VFRECIPE_H",
    "VFRECIPE_S",
    "VFRECIPE_D",
    "VFRECIPE_Q",
    "VFRSQRTE_H",
    "VFRSQRTE_S",
    "VFRSQRTE_D",
    "VFRSQRTE_Q",
    "VFRINT_H",
    "VFRINT_Q",
    "VFRINTRM_H",
    "VFRINTRM_Q",
    "VFRINTRP_H",
    "VFRINTRP_Q",
    "VFRINTRZ_H",
    "VFRINTRZ_Q",
    "VFRINTRNE_H",
    "VFRINTRNE_Q",
    "VEXTL_Q_B",
    "VEXTL_Q_H",
    "VEXTL_Q_W",
    "VEXTL_Q_BU",
    "VEXTL_Q_HU",
    "VEXTL_Q_WU",
    "VFCVTL_Q_D",
    "VFCVTH_Q_D",
    "VFFINTL_Q_L",
    "VFFINTH_Q_L",
    "VFFINTL_D_WU",
    "VFFINTH_D_WU",
    "VFFINTL_Q_LU",
    "VFFINTH_Q_LU",
    "VFTINTRM_WU_S",
    "VFTINTRM_LU_D",
    "VFTINTRP_WU_S",
    "VFTINTRP_LU_D",
    "VFTINTRNE_WU_S",
    "VFTINTRNE_LU_D",
    "VFTINTL_LU_S",
    "VFTINTH_LU_S",
    "VFTINTRML_LU_S",
    "VFTINTRMH_LU_S",
    "VFTINTRPL_LU_S",
    "VFTINTRPH_LU_S",
    "VFTINTRZL_LU_S",
    "VFTINTRZH_LU_S",
    "VFTINTRNEL_LU_S",
    "VFTINTRNEH_LU_S",
    "VSLLWIL_Q_D",
    "VSLLWIL_QU_DU",
    "XVADDW_Q_Q_D",
    "XVADDW_Q_Q_DU",
    "XVSUBW_Q_Q_D",
    "XVSUBW_Q_Q_DU",
    "XVSADDW_Q_Q_D",
    "XVSADDW_QU_QU_DU",
    "XVSSUBW_Q_Q_D",
    "XVSSUBW_QU_QU_DU",
    "XVFADD_H",
    "XVFADD_Q",
    "XVFSUB_H",
    "XVFSUB_Q",
    "XVFADDSUB_H",
    "XVFADDSUB_Q",
    "XVFSUBADD_H",
    "XVFSUBADD_Q",
    "XVFMUL_H",
    "XVFMUL_Q",
    "XVFDIV_H",
    "XVFDIV_Q",
    "XVFMAX_H",
    "XVFMAX_Q",
    "XVFMIN_H",
    "XVFMIN_Q",
    "XVFMAXA_H",
    "XVFMAXA_Q",
    "XVFMINA_H",
    "XVFMINA_Q",
    "XVFSCALEB_H",
    "XVFSCALEB_Q",
    "XVFCVT_D_Q",
    "XVFFINT_H_W",
    "XVFFINT_H_WU",
    "XVFFINT_S_LU",
    "XVFTINT_WU_D",
    "XVFTINTRM_WU_D",
    "XVFTINTRP_WU_D",
    "XVFTINTRZ_WU_D",
    "XVFTINTRNE_WU_D",
    "XVMSKGEZ_H",
    "XVMSKGEZ_W",
    "XVMSKGEZ_D",
    "XVMSKNZ_H",
    "XVMSKNZ_W",
    "XVMSKNZ_D",
    "XVMSKCOPY_H",
    "XVMSKCOPY_W",
    "XVMSKCOPY_D",
    "XVMSKFILL_H",
    "XVMSKFILL_W",
    "XVMSKFILL_D",
    "XVFLOGB_H",
    "XVFLOGB_Q",
    "XVFCLASS_H",
    "XVFCLASS_Q",
    "XVFSQRT_H",
    "XVFSQRT_Q",
    "XVFRECIP_H",
    "XVFRECIP_Q",
    "XVFRSQRT_H",
    "XVFRSQRT_Q",
    "XVFRECIPE_H",
    "XVFRECIPE_S",
    "XVFRECIPE_D",
    "XVFRECIPE_Q",
    "XVFRSQRTE_H",
    "XVFRSQRTE_S",
    "XVFRSQRTE_D",
    "XVFRSQRTE_Q",
    "XVFRINT_H",
    "XVFRINT_Q",
    "XVFRINTRM_H",
    "XVFRINTRM_Q",
    "XVFRINTRP_H",
    "XVFRINTRP_Q",
    "XVFRINTRZ_H",
    "XVFRINTRZ_Q",
    "XVFRINTRNE_H",
    "XVFRINTRNE_Q",
    "XVEXTL_Q_B",
    "XVEXTL_Q_H",
    "XVEXTL_Q_W",
    "XVEXTL_Q_BU",
    "XVEXTL_Q_HU",
    "XVEXTL_Q_WU",
    "XVFCVTL_Q_D",
    "XVFCVTH_Q_D",
    "XVFFINTL_Q_L",
    "XVFFINTH_Q_L",
    "XVFFINTL_D_WU",
    "XVFFINTH_D_WU",
    "XVFFINTL_Q_LU",
    "XVFFINTH_Q_LU",
    "XVFTINTRM_WU_S",
    "XVFTINTRM_LU_D",
    "XVFTINTRP_WU_S",
    "XVFTINTRP_LU_D",
    "XVFTINTRNE_WU_S",
    "XVFTINTRNE_LU_D",
    "XVFTINTL_LU_S",
    "XVFTINTH_LU_S",
    "XVFTINTRML_LU_S",
    "XVFTINTRMH_LU_S",
    "XVFTINTRPL_LU_S",
    "XVFTINTRPH_LU_S",
    "XVFTINTRZL_LU_S",
    "XVFTINTRZH_LU_S",
    "XVFTINTRNEL_LU_S",
    "XVFTINTRNEH_LU_S",
    "XVINSGR2VR_B",
    "XVINSGR2VR_H",
    "XVPICKVE2GR_B",
    "XVPICKVE2GR_H",
    "XVPICKVE2GR_BU",
    "XVPICKVE2GR_HU",
    "XVINSVE0_B",
    "XVINSVE0_H",
    "XVPICKVE_B",
    "XVPICKVE_H",
    "XVSLLWIL_Q_D",
    "XVSLLWIL_QU_DU",

    /* privileged state instructions */
    "CSRRD",
    "CSRWR",
    "CSRXCHG",
    "GCSRRD",
    "GCSRWR",
    "GCSRXCHG",
    "CACHE",
    "LDDIR",
    "LDPTE",
    "IOCSRRD_B",
    "IOCSRRD_H",
    "IOCSRRD_W",
    "IOCSRRD_D",
    "IOCSRWR_B",
    "IOCSRWR_H",
    "IOCSRWR_W",
    "IOCSRWR_D",
    "TLBINV",
    "GTLBINV",
    "TLBFLUSH",
    "GTLBFLUSH",
    "TLBP",
    "GTLBP",
    "TLBR",
    "GTLBR",
    "TLBWI",
    "GTLBWI",
    "TLBWR",
    "GTLBWR",
    "ERET",
    "DERET",
    "WAIT",

    /* fake instruction", will be removed later */
    "FCMP_COND_H",
    "VFCMP_COND_H",
    "VFCMP_COND_Q",
    "XVFCMP_COND_H",
    "XVFCMP_COND_Q",

        "lable",

    };
    lsassert(value <= LISA_XVFCMP_COND_Q);
    return g_ir2_names[value];
}

IR2_OPND sp_ir2_opnd = {._type = IR2_OPND_GPR,   .val = 3,  ._reg_num = 3,  ._addr = 0};
IR2_OPND fp_ir2_opnd = {._type = IR2_OPND_GPR,   .val = 22, ._reg_num = 22, ._addr = 0};
IR2_OPND zero_ir2_opnd = {._type = IR2_OPND_GPR, .val = 0,  ._reg_num = 0,  ._addr = 0};
IR2_OPND env_ir2_opnd = {._type = IR2_OPND_GPR,  .val = 25, ._reg_num = 25, ._addr = 0};
IR2_OPND n1_ir2_opnd = {._type = IR2_OPND_IREG, .val = 23, ._reg_num = 23, ._addr = 0};
IR2_OPND a0_ir2_opnd = {._type = IR2_OPND_GPR,   .val = 4,  ._reg_num = 4,  ._addr = 0};
IR2_OPND t5_ir2_opnd = {._type = IR2_OPND_GPR,   .val = 17,  ._reg_num = 17,  ._addr = 0};
IR2_OPND ra_ir2_opnd = {._type = IR2_OPND_GPR,   .val = 1,  ._reg_num = 1,  ._addr = 0};

/*
 * FIXME: For LA, IR2_OPND didn't need reg_num and addr, val is enough for LA
 */
IR2_OPND fcsr_ir2_opnd = {._type = IR2_OPND_CREG, ._reg_num = 0, ._addr = 0};
IR2_OPND fcc0_ir2_opnd = {._type = IR2_OPND_CC, ._reg_num = 0, ._addr = 0};
IR2_OPND fcc1_ir2_opnd = {._type = IR2_OPND_CC, ._reg_num = 1, ._addr = 0};
IR2_OPND fcc2_ir2_opnd = {._type = IR2_OPND_CC, ._reg_num = 2, ._addr = 0};
IR2_OPND fcc3_ir2_opnd = {._type = IR2_OPND_CC, ._reg_num = 3, ._addr = 0};
IR2_OPND fcc4_ir2_opnd = {._type = IR2_OPND_CC, ._reg_num = 4, ._addr = 0};
IR2_OPND fcc5_ir2_opnd = {._type = IR2_OPND_CC, ._reg_num = 5, ._addr = 0};
IR2_OPND fcc6_ir2_opnd = {._type = IR2_OPND_CC, ._reg_num = 6, ._addr = 0};
IR2_OPND fcc7_ir2_opnd = {._type = IR2_OPND_CC, ._reg_num = 7, ._addr = 0};

void ir2_opnd_build_none(IR2_OPND *opnd)
{
    opnd->_type = IR2_OPND_NONE;
    opnd->_reg_num = 0;
    opnd->_addr = 0;
}

IR2_OPND ir2_opnd_new_none(void)
{
    IR2_OPND opnd;
    ir2_opnd_build_none(&opnd);
    return opnd;
}

void ir2_opnd_build_type(IR2_OPND *opnd, IR2_OPND_TYPE t)
{
    lsassert(t == IR2_OPND_LABEL);
    opnd->_type = t;
    opnd->_label_id = 0;
    opnd->_addr = ++(lsenv->tr_data->label_num);
}

IR2_OPND ir2_opnd_new_type(IR2_OPND_TYPE t)
{
    IR2_OPND opnd;
    ir2_opnd_build_type(&opnd, t);
    return opnd;
}

void ir2_opnd_build(IR2_OPND *opnd, IR2_OPND_TYPE t, int value)
{
    if (t == IR2_OPND_IREG || t == IR2_OPND_FREG || t == IR2_OPND_CREG) {
        opnd->_type = t;
        opnd->_reg_num = value;
        opnd->_addr = 0;
    } else if (t == IR2_OPND_IMM) {
        opnd->_type = t;
        opnd->_reg_num = 0;
        opnd->_addr = 0;
        opnd->_imm16 = value;
    } else if (t == IR2_OPND_ADDR) {
        opnd->_type = t;
        opnd->_reg_num = 0;
        opnd->_addr = value;
    } else {
        printf("[LATX] [error] not implemented in %s : %d", __func__,
               __LINE__);
        exit(-1);
    }
    /*
     * To make LA backend happy.
     */
    opnd->val = value;
}

IR2_OPND ir2_opnd_new(IR2_OPND_TYPE type, int value)
{
    IR2_OPND opnd;

    ir2_opnd_build(&opnd, type, value);

    return opnd;
}

void ir2_opnd_build2(IR2_OPND *opnd, IR2_OPND_TYPE type, int base, int16 offset)
{
    lsassert(type == IR2_OPND_MEM);
    opnd->_type = type;
    opnd->val = base;
    opnd->_reg_num = base;
    opnd->_imm16 = offset;
}

IR2_OPND ir2_opnd_new2(IR2_OPND_TYPE type, int base, int16 offset)
{
    IR2_OPND opnd;

    ir2_opnd_build2(&opnd, type, base, offset);

    return opnd;
}

int16 ir2_opnd_imm(IR2_OPND *opnd) { return opnd->_imm16; }

int ir2_opnd_is_ireg(IR2_OPND *opnd) { return opnd->_type == IR2_OPND_IREG; }

int ir2_opnd_is_freg(IR2_OPND *opnd) { return opnd->_type == IR2_OPND_FREG; }

int ir2_opnd_is_creg(IR2_OPND *opnd) { return opnd->_type == IR2_OPND_CREG; }

int32 ir2_opnd_label_id(IR2_OPND *opnd) { return opnd->_label_id; }

int ir2_opnd_is_itemp(IR2_OPND *opnd)
{
    return ir2_opnd_is_ireg(opnd) &&
           (ir2_opnd_base_reg_num(opnd) >= ITEMP0_NUM) &&
           (ir2_opnd_base_reg_num(opnd) <= ITEMP9_NUM);
}

int ir2_opnd_is_ftemp(IR2_OPND *opnd)
{
    return ir2_opnd_is_freg(opnd) &&
           (ir2_opnd_base_reg_num(opnd) >= FTEMP0_NUM) &&
           (ir2_opnd_base_reg_num(opnd) <= FTEMP6_NUM);
}

int ir2_opnd_is_mem_base_itemp(IR2_OPND *opnd)
{
    return ir2_opnd_is_mem(opnd) &&
           (ir2_opnd_base_reg_num(opnd) > 3) &&
           (ir2_opnd_base_reg_num(opnd) < 14);
}

int ir2_opnd_is_mem(IR2_OPND *opnd) { return opnd->_type == IR2_OPND_MEM; }

int ir2_opnd_is_imm(IR2_OPND *opnd) { return opnd->_type == IR2_OPND_IMM; }

int ir2_opnd_is_label(IR2_OPND *opnd) { return opnd->_type == IR2_OPND_LABEL; }

int ir2_opnd_base_reg_num(IR2_OPND *opnd) { return opnd->val; }

IR2_OPND_TYPE ir2_opnd_type(IR2_OPND *opnd)
{
    return (IR2_OPND_TYPE)opnd->_type;
}

int ir2_opnd_cmp(IR2_OPND *opnd1, IR2_OPND *opnd2)
{
    return opnd1->_type == opnd2->_type && opnd1->val == opnd2->val;
}

void ir2_opnd_convert_label_to_imm(IR2_OPND *opnd, int imm)
{
    lsassert(ir2_opnd_is_label(opnd));
    opnd->_type = IR2_OPND_IMM;
    opnd->_imm16 = imm;
    opnd->val = imm;
}

int ir2_opnd_to_string(IR2_OPND *opnd, char *str, bool hex)
{
    int base_reg_num = ir2_opnd_base_reg_num(opnd);

    switch (ir2_opnd_type(opnd)) {
    case IR2_OPND_INV:
        return 0;
    case IR2_OPND_GPR: {
        if (ir2_opnd_is_itemp(opnd)) {
            return sprintf(str, "\033[3%dmitmp%d\033[m", base_reg_num % 6 + 1,
                           base_reg_num);
        } else {
            strcpy(str, ir2_name(base_reg_num));
            return strlen(str);
        }
    }
    case IR2_OPND_FPR: {
        if (ir2_opnd_is_ftemp(opnd)) {
            return sprintf(str, "\033[3%dmftmp%d\033[m", base_reg_num % 6 + 1,
                           base_reg_num);
        } else {
            strcpy(str, ir2_name(40 + base_reg_num));
            return strlen(str);
        }
    }
    case IR2_OPND_FCSR: {
        return sprintf(str, "$c%d", base_reg_num);
    }
    case IR2_OPND_CC: {
        return sprintf(str, "$c%d", base_reg_num);
    }
    //case IR2_OPND_MEM: {
    //    if (ir2_opnd_imm(opnd) > -1024 && ir2_opnd_imm(opnd) < 1024) {
    //        if (ir2_opnd_is_mem_base_itemp(opnd)) {
    //            return sprintf(str, "%d(\033[3%dmitmp%d\033[m)",
    //                           ir2_opnd_imm(opnd), base_reg_num % 6 + 1,
    //                           base_reg_num);
    //        } else {
    //            return sprintf(str, "%d(%s)", ir2_opnd_imm(opnd),
    //                           ir2_name(base_reg_num));
    //        }
    //    } else {
    //        if (ir2_opnd_is_mem_base_itemp(opnd)) {
    //            return sprintf(str, "0x%x(\033[3%dmitmp%d\033[m)",
    //                           ir2_opnd_imm(opnd), base_reg_num % 6 + 1,
    //                           base_reg_num);
    //        } else {
    //            return sprintf(str, "0x%x(%s)", ir2_opnd_imm(opnd),
    //                           ir2_name(base_reg_num));
    //        }
    //    }
    //}
    case IR2_OPND_IMMD: {
            return sprintf(str, "%d", ir2_opnd_imm(opnd));
    }
    case IR2_OPND_IMMH: {
        if(hex)
            return sprintf(str, "0x%x", (uint16)ir2_opnd_imm(opnd));
        else
            return sprintf(str, "%d", ir2_opnd_imm(opnd));
    }
    //case IR2_OPND_ADDR:
    //    return sprintf(str, "0x%x", ir2_opnd_addr(opnd));
    case IR2_OPND_LABEL:
        return sprintf(str, "LABEL %d", ir2_opnd_imm(opnd));
    default:
        lsassertm(0, "type = %d\n", ir2_opnd_type(opnd));
        return 0;
    }
}

bool ir2_opcode_is_branch(IR2_OPCODE opcode)
{
    return (opcode >= LISA_BEQZ && opcode <= LISA_BCNEZ) ||
           (opcode >= LISA_B && opcode <= LISA_BGEU);
}

bool ir2_opcode_is_branch_with_3opnds(IR2_OPCODE opcode)
{
    if (opcode >= LISA_BEQ && opcode <= LISA_BGEU) {
        return true;
    }
    return false;
}

bool ir2_opcode_is_f_branch(IR2_OPCODE opcode)
{
    return (opcode == LISA_BCEQZ || opcode == LISA_BCNEZ);
}

bool ir2_opcode_is_convert(IR2_OPCODE opcode)
{
    switch (opcode) {
    case LISA_FCVT_D_S:
    case LISA_FCVT_S_D:
    case LISA_FFINT_D_W:
    case LISA_FFINT_D_L:
    case LISA_FFINT_S_W:
    case LISA_FFINT_S_L:
    case LISA_FTINT_L_D:
    case LISA_FTINT_L_S:
    case LISA_FTINT_W_D:
    case LISA_FTINT_W_S:
        return true;
    default:
        return false;
    }
}

bool ir2_opcode_is_fcmp(IR2_OPCODE opcode)
{
    return (opcode == LISA_FCMP_COND_S || opcode == LISA_FCMP_COND_D);
}

static IR2_OPCODE ir2_opcode_rrr_to_rri(IR2_OPCODE opcode)
{
    switch (opcode) {
    case LISA_ADDI_ADDR:
    case LISA_ADD_ADDR:
        return LISA_ADDI_ADDR;
    case LISA_ADDI_ADDRX:
    case LISA_ADD_ADDRX:
        return LISA_ADDI_ADDRX;
    case LISA_SUBI_ADDR:
    case LISA_SUB_ADDR:
        return LISA_SUBI_ADDR;
    case LISA_SUBI_ADDRX:
    case LISA_SUB_ADDRX:
        return LISA_SUBI_ADDRX;
    case LISA_ADDI_W:
    case LISA_ADD_W:
        return LISA_ADDI_W;
    case LISA_ADDI_D:
    case LISA_ADD_D:
        return LISA_ADDI_D;
    case LISA_SUBIU:
    case LISA_SUB_W:
        return LISA_SUBIU;
    case LISA_DSUBIU:
    case LISA_SUB_D:
        return LISA_DSUBIU;
    case LISA_SLTI:
    case LISA_SLT:
        return LISA_SLTI;
    case LISA_SLTUI:
    case LISA_SLTU:
        return LISA_SLTUI;
    case LISA_ANDI:
    case LISA_AND:
        return LISA_ANDI;
    case LISA_ORI:
    case LISA_OR:
        return LISA_ORI;
    case LISA_XORI:
    case LISA_XOR:
        return LISA_XORI;
    case LISA_SLL_W:
    case LISA_SLLI_W:
        return LISA_SLLI_W;
    case LISA_SRL_W:
    case LISA_SRLI_W:
        return LISA_SRLI_W;
    case LISA_SRA_W:
    case LISA_SRAI_W:
        return LISA_SRAI_W;
    default:
        lsassertm(0, "illegal immediate operand used for %s\n",
                  ir2_name(opcode));
        return LISA_INVALID;
    }
}

void ir2_set_id(IR2_INST *ir2, int id) { ir2->_id = id; }

int ir2_get_id(IR2_INST *ir2) { return ir2->_id; }

IR2_OPCODE ir2_opcode(IR2_INST *ir2) { return (IR2_OPCODE)(ir2->_opcode); }

void ir2_set_opcode(IR2_INST *ir2, IR2_OPCODE type) {
    ir2->_opcode = type;
}

ADDR ir2_addr(IR2_INST *ir2) { return ir2->_addr; }

void ir2_set_addr(IR2_INST *ir2, ADDR a) { ir2->_addr = a; }

uint32 ir2_opnd_addr(IR2_OPND *ir2) { return ir2->_addr; }

IR2_INST *ir2_prev(IR2_INST *ir2)
{
    if (ir2->_prev == -1) {
        return NULL;
    } else {
        return lsenv->tr_data->ir2_inst_array + ir2->_prev;
    }
}

IR2_INST *ir2_next(IR2_INST *ir2)
{
    if (ir2->_next == -1) {
        return NULL;
    } else {
        return lsenv->tr_data->ir2_inst_array + ir2->_next;
    }
}

int ir2_to_string(IR2_INST *ir2, char *str)
{
    int length = 0;
    int i = 0;
    bool print_hex_imm = false;

    length = sprintf(str, "%-8s  ", ir2_name(ir2_opcode(ir2)));

    if (ir2_opcode(ir2) == LISA_ANDI || ir2_opcode(ir2) == LISA_ORI ||
        ir2_opcode(ir2) == LISA_XORI || ir2_opcode(ir2) == LISA_LU12I_W) {
        print_hex_imm = true;
    }

    for (i = 0; i < ir2->op_count; ++i) {
        if (ir2_opnd_type(&ir2->_opnd[i]) == IR2_OPND_NONE) {
            return length;
        } else {
            if (i > 0) {
                strcat(str, ",");
                length += 1;
            }
            /*length += _opnd[i].to_string(str+length, print_hex_imm);*/
            length +=
                ir2_opnd_to_string(&ir2->_opnd[i], str + length, print_hex_imm);
        }
    }

    return length;
}

static int ir1_id;

int ir2_dump(IR2_INST *ir2)
{
    char str[64];
    int size = 0;

    if (ir2_opcode(ir2) ==
        0) { /* an empty IR2_INST was inserted into the ir2 */
             /* list, but not assigned yet. */
        return 0;
    }

    size = ir2_to_string(ir2, str);
    if (ir2->_id == 0) {
        ir1_id = 0;
        fprintf(stderr, "[%d, %d] %s\n", ir2->_id, ir1_id, str);
    } else if (str[0] == '-') {
        ir1_id ++;
        fprintf(stderr, "[%d, %d] %s\n", ir2->_id, ir1_id, str);
    } else {
        fprintf(stderr, "%s\n", str);
    }


    return size;
}

void ir2_build(IR2_INST *ir2, IR2_OPCODE opcode, IR2_OPND opnd0, IR2_OPND opnd1,
               IR2_OPND opnd2)
{
    ir2->_opcode = opcode;
    ir2->_opnd[0] = opnd0;
    ir2->_opnd[1] = opnd1;
    ir2->_opnd[2] = opnd2;
    ir2->_opnd[3] = ir2_opnd_new_none();
}

void ir2_append(IR2_INST *ir2)
{
    TRANSLATION_DATA *t = lsenv->tr_data;
    IR2_INST *former_last = t->last_ir2;

    if (former_last != NULL) {
        lsassert(t->first_ir2 != NULL);
        ir2->_prev = ir2_get_id(former_last);
        ir2->_next = -1;
        t->last_ir2 = ir2;
        former_last->_next = ir2_get_id(ir2);
    } else {
        lsassert(t->first_ir2 == NULL);
        ir2->_prev = -1;
        ir2->_next = -1;
        t->last_ir2 = ir2;
        t->first_ir2 = ir2;
    }

    if(ir2->_opcode >= LISA_GR2SCR)
        t->real_ir2_inst_num++;
}

void ir2_remove(IR2_INST *ir2)
{
    TRANSLATION_DATA *t = lsenv->tr_data;

    IR2_INST *next = ir2_next(ir2);
    IR2_INST *prev = ir2_prev(ir2);

    if (t->first_ir2 == ir2) {
        if (t->last_ir2 == ir2) { /* head and tail */
            t->first_ir2 = NULL;
            t->last_ir2 = NULL;
        } else { /* head but not tail */
            t->first_ir2 = next;
            next->_prev = -1;
        }
    } else if (t->last_ir2 == ir2) { /* tail but not head */
        t->last_ir2 = prev;
        prev->_next = -1;
    } else {
        prev->_next = ir2_get_id(next);
        next->_prev = ir2_get_id(prev);
    }

    ir2->_prev = -1;
    ir2->_next = -1;
}

void ir2_insert_before(IR2_INST *ir2, IR2_INST *next)
{
    TRANSLATION_DATA *t = lsenv->tr_data;

    if (t->first_ir2 == next) {
        t->first_ir2 = ir2;
        ir2->_prev = -1;
        ir2->_next = ir2_get_id(next);
        next->_prev = ir2_get_id(ir2);
    } else {
        IR2_INST *prev = ir2_prev(next);

        ir2->_prev = ir2_get_id(prev);
        prev->_next = ir2_get_id(ir2);

        ir2->_next = ir2_get_id(next);
        next->_prev = ir2_get_id(ir2);
    }
}

void ir2_insert_after(IR2_INST *ir2, IR2_INST *prev)
{
    TRANSLATION_DATA *t = lsenv->tr_data;

    if (t->last_ir2 == prev) {
        t->last_ir2 = ir2;
        ir2->_next = -1;
        ir2->_prev = ir2_get_id(prev);
        prev->_next = ir2_get_id(ir2);
    } else {
        IR2_INST *next = ir2_next(prev);

        ir2->_next = ir2_get_id(next);
        next->_prev = ir2_get_id(ir2);

        ir2->_prev = ir2_get_id(prev);
        prev->_next = ir2_get_id(ir2);
    }
}

static IR2_INST *ir2_allocate(void)
{
    TRANSLATION_DATA *t = lsenv->tr_data;

    /* 1. make sure we have enough space */
    if (t->ir2_inst_num_current == t->ir2_inst_num_max) {
        /* printf("[LATX] [error] not implemented in %s : %d", __func__, */
        /* __LINE__); exit(-1); 1.1. current array size in bytes */
        int bytes = sizeof(IR2_INST) * t->ir2_inst_num_max;

        /* 1.2. double the array */
        t->ir2_inst_num_max *= 2;
        IR2_INST *back_ir2_inst_array = t->ir2_inst_array;
        t->ir2_inst_array =
            (IR2_INST *)mm_realloc(t->ir2_inst_array, bytes << 1);
        t->first_ir2 =
            (IR2_INST *)((ADDR)t->first_ir2 - (ADDR)back_ir2_inst_array +
                         (ADDR)t->ir2_inst_array);
        t->last_ir2 =
            (IR2_INST *)((ADDR)t->last_ir2 - (ADDR)back_ir2_inst_array +
                         (ADDR)t->ir2_inst_array);
    }

    /* 2. allocate one */
    IR2_INST *p = t->ir2_inst_array + t->ir2_inst_num_current;
    ir2_set_id(p, t->ir2_inst_num_current);
    t->ir2_inst_num_current++;

    /* 3. return it */
    return p;
}

static inline EXTENSION_MODE int16_em(int16 imm)
{
    if (imm >= 0) {
        return ZERO_EXTENSION;
    } else {
        return SIGN_EXTENSION;
    }
}

static inline int int16_eb(int16 imm)
{
    int i = 0;
    if (imm >= 0) { /* where is the first "1"? */
        for (i = 14; i >= 0; --i) {
            if (BITS_ARE_SET(imm, 1 << i)) {
                return i + 1;
            }
        }
        return 0; /* no "1", the value is zero */
    } else {
        for (i = 14; i >= 0; --i) { /* where is the first "0"? */
            if (BITS_ARE_CLEAR(imm, 1 << i)) {
                return i + 2;
            }
        }
        return 1; /* no "0", the value is -1 */
    }
}

static inline int uint16_eb(uint16 imm)
{
    int i = 0;
    for (i = 15; i >= 0; --i) {
        if (BITS_ARE_SET(imm, 1 << i)) {
            return i + 1;
        }
    }
    return 0;
}

/********************************************
 *                                          *
 *     LA IR2 implementation.               *
 *                                          *
 ********************************************/
bool la_ir2_opcode_is_load(IR2_OPCODE opcode)
{
    if (opcode >= LISA_LD_B && opcode <=  LISA_LD_D) {
        return true;
    }
    if (opcode >=  LISA_LD_BU && opcode <=  LISA_LD_WU) {
        return true;
    }
    if (opcode == LISA_LL_W || opcode == LISA_LL_D) {
        return true;
    }
    if (opcode == LISA_LDPTR_W || opcode == LISA_LDPTR_D) {
        return true;
    }
    if (opcode == LISA_FLD_S || opcode == LISA_FLD_D) {
        return true;
    }
    if (opcode == LISA_VLD || opcode == LISA_XVLD){
        return true;
    }
    if (opcode >= LISA_LDL_W && opcode <= LISA_LDR_D){
        return true;
    }
    if (opcode >= LISA_VLDREPL_D && opcode <= LISA_VLDREPL_B){
        return true;
    }
    if (opcode >= LISA_XVLDREPL_D && opcode <= LISA_XVLDREPL_B){
        return true;
    }
    if (opcode == LISA_PRELD) {
        return true;
    }
    return false;
}

bool la_ir2_opcode_is_store(IR2_OPCODE opcode)
{
    if (opcode >= LISA_ST_B && opcode <= LISA_ST_D) {
        return true;
    }
    if (opcode >= LISA_STL_W && opcode <= LISA_STR_D) {
        return true;
    }
    if (opcode == LISA_SC_D || opcode == LISA_SC_W) {
        return true;
    }
    if (opcode == LISA_STPTR_W || opcode == LISA_STPTR_D) {
        return true;
    }
    if (opcode == LISA_FST_S || opcode == LISA_FST_D) {
        return true;
    }
    if (opcode == LISA_XVST || opcode == LISA_VST){
        return true;
    }
    return false;
}

IR2_OPND create_ir2_opnd(IR2_OPND_TYPE type, int val) {
    IR2_OPND res;
    switch (type) {
    case IR2_OPND_GPR:
        res._reg_num = val;
        break;
    case IR2_OPND_IMMH:
        res._imm16 = val;
        break;
    default:
        break;
    }
    res._type = type;
    res.val = val;
    return res;
}

IR2_INST *la_append_ir2_opnd0(IR2_OPCODE type) {

    IR2_OPND ir2_opnd_none = ir2_opnd_new_none();
    IR2_INST *pir2 = ir2_allocate();
    pir2->_opcode = type;
    pir2->op_count = 1;
    pir2->_opnd[0] = ir2_opnd_none;
    ir2_append(pir2);
    return pir2;
}

IR2_INST *la_append_ir2_opnd1(IR2_OPCODE type, IR2_OPND op0) {
    /*
     * FIXME: Might be handle mfhi/mflo later.
     */
    //if (type == LISA_X86MFTOP) {
    //    lsassertm(0, "cannot handle %s. Need to set EM",
    //                ir2_name(type));
    //}
    IR2_INST *pir2 = ir2_allocate();
    pir2->_opcode = type;
    pir2->op_count = 1;
    pir2->_opnd[0] = op0;
    ir2_append(pir2);
    if(!ir2_op_check(pir2))
        lsassertm(0, "Maybe you should check the type of operand");
    return pir2;
}

IR2_INST *la_append_ir2_opnd2(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1) {
    IR2_INST *pir2 = ir2_allocate();
    pir2->_opcode = type;
    pir2->op_count = 2;
    pir2->_opnd[0] = op0;
    pir2->_opnd[1] = op1;
    ir2_append(pir2);
    if(!ir2_op_check(pir2))
        lsassertm(0, "Maybe you should check the type of operand");
    return pir2;
}

IR2_INST *la_append_ir2_opnd3(IR2_OPCODE type, IR2_OPND op0,
                      IR2_OPND op1, IR2_OPND op2) {
    IR2_INST *pir2 = ir2_allocate();
    pir2->_opcode = type;
    pir2->op_count = 3;
    pir2->_opnd[0] = op0;
    pir2->_opnd[1] = op1;
    pir2->_opnd[2] = op2;
    ir2_append(pir2);
    if(!ir2_op_check(pir2))
        lsassertm(0, "Maybe you should check the type of operand");
    return pir2;
}

IR2_INST *la_append_ir2_opnd4(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1,
                      IR2_OPND op2, IR2_OPND op3) {
    IR2_INST *pir2 = ir2_allocate();
    pir2->_opcode = type;
    pir2->op_count = 4;
    pir2->_opnd[0] = op0;
    pir2->_opnd[1] = op1;
    pir2->_opnd[2] = op2;
    pir2->_opnd[3] = op3;
    ir2_append(pir2);
    if(!ir2_op_check(pir2))
        lsassertm(0, "Maybe you should check the type of operand");
    return pir2;
}

IR2_INST *la_append_ir2_opndi(IR2_OPCODE type, int imm) {
    IR2_OPND opnd = create_ir2_opnd(IR2_OPND_IMMH, imm);
    return la_append_ir2_opnd1(type, opnd);
}

IR2_INST *la_append_ir2_opnd1i(IR2_OPCODE type, IR2_OPND op0, int imm) {
    IR2_OPND opnd = create_ir2_opnd(IR2_OPND_IMMH, imm);
    return la_append_ir2_opnd2(type, op0, opnd);
}

IR2_INST *la_append_ir2_opnd2i(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1, int imm) {
    if(type == LISA_ANDI || type == LISA_ORI || type == LISA_XORI || type == LISA_LU52I_D)
        lsassert((unsigned int)(imm) <= 0xfff);
    else
        lsassert(imm >= -2048 && imm <= 2047);
    /*
     * This code is used for add AND xx, n1.
     */
#ifdef N64 /* validate address */
    if (la_ir2_opcode_is_load(type) || la_ir2_opcode_is_store(type)) {
        int base_ireg_num = ir2_opnd_base_reg_num(&op1);
        base_ireg_num = em_validate_address(base_ireg_num);
    }
#endif

    if (type == LISA_SC_W || type == LISA_LL_W) {
        lsassertm(((imm % 4) == 0), "ll/sc imm %d error.\n", imm);
        imm = imm >> 2;
    } else if (type == LISA_SC_D || type == LISA_LL_D) {
        lsassertm(((imm % 4) == 0), "ll/sc imm %d error.\n", imm);
        imm = imm >> 2;
    }


    IR2_OPND opnd = create_ir2_opnd(IR2_OPND_IMMH, imm);
    return la_append_ir2_opnd3(type, op0, op1, opnd);
}

IR2_INST *la_append_ir2_opnd3i(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1,
        IR2_OPND op2, int imm0) {
    IR2_OPND imm0_opnd = create_ir2_opnd(IR2_OPND_IMMH, imm0);
    return la_append_ir2_opnd4(type, op0, op1, op2, imm0_opnd);
}

IR2_INST *la_append_ir2_opnd2ii(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1,
        int imm0, int imm1) {
    IR2_OPND imm0_opnd = create_ir2_opnd(IR2_OPND_IMMH, imm0);
    IR2_OPND imm1_opnd = create_ir2_opnd(IR2_OPND_IMMH, imm1);
    return la_append_ir2_opnd4(type, op0, op1, imm0_opnd, imm1_opnd);
}

//ir2 with extension mode handling
IR2_INST *la_append_ir2_opnd3_em(IR2_OPCODE opcode, IR2_OPND op0,
                      IR2_OPND op1, IR2_OPND op2) {
    if (ir2_opnd_is_imm(&op2)) {
        IR2_OPCODE new_opcode = ir2_opcode_rrr_to_rri(opcode);
        return la_append_ir2_opnd2i_em(new_opcode, op0, op1,
                                 ir2_opnd_imm(&op2));
    }

    IR2_INST *pir2 = ir2_allocate();
    pir2->_opcode = opcode;
    pir2->op_count = 3;
    pir2->_opnd[0] = op0;
    pir2->_opnd[1] = op1;
    pir2->_opnd[2] = op2;

    lsassert(ir2_opnd_is_ireg(&op0) && ir2_opnd_is_ireg(&op1) &&
             ir2_opnd_is_ireg(&op2));
    switch (opcode) {
    case LISA_ADD_ADDR:
        if (ir2_opnd_is_mips_address(&op1)) {
            lsassert(ir2_opnd_is_sx(&op2, 32) &&
                     !ir2_opnd_is_mips_address(&op2));
            ir2_set_opcode(pir2, LISA_ADD_D);
            ir2_opnd_set_em(&op0, EM_MIPS_ADDRESS, 32);
        } else if (ir2_opnd_is_mips_address(&op2)) {
            lsassert(ir2_opnd_is_sx(&op1, 32) &&
                     !ir2_opnd_is_mips_address(&op1));
            ir2_set_opcode(pir2, LISA_ADD_D);
            ir2_opnd_set_em(&op0, EM_MIPS_ADDRESS, 32);
        } else if (ir2_opnd_is_sx(&op1, 32) && ir2_opnd_is_sx(&op2, 32)) {
            ir2_set_opcode(pir2, LISA_ADD_W);
            ir2_opnd_set_em_add2(&op0, &op1, &op2);
            if (!ir2_opnd_is_sx(&op0, 32)) {
                ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_set_opcode(pir2, LISA_ADD_D);
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        }
        break;
    case LISA_ADD_ADDRX:
        if (ir2_opnd_is_x86_address(&op1)) {
            lsassert(!ir2_opnd_is_x86_address(&op2));
            ir2_set_opcode(pir2, LISA_ADD_D);
            if (ir2_opnd_is_sx(&op2, 32)) {
                ir2_opnd_set_em(&op0, EM_X86_ADDRESS, 32);
            } else {
                ir2_opnd_set_em_add2(&op0, &op1, &op2);
            }
        } else if (ir2_opnd_is_x86_address(&op2)) {
            lsassert(!ir2_opnd_is_x86_address(&op1));
            ir2_set_opcode(pir2, LISA_ADD_D);
            if (ir2_opnd_is_sx(&op1, 32)) {
                ir2_opnd_set_em(&op0, EM_X86_ADDRESS, 32);
            } else {
                ir2_opnd_set_em_add2(&op0, &op1, &op2);
            }
        } else if (ir2_opnd_is_sx(&op1, 32) && ir2_opnd_is_sx(&op2, 32)) {
            ir2_set_opcode(pir2, LISA_ADD_W);
            ir2_opnd_set_em_add2(&op0, &op1, &op2);
            if (!ir2_opnd_is_sx(&op0, 32)) {
                ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_set_opcode(pir2, LISA_ADD_W);
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        }
        break;
    case LISA_SUB_ADDR:
        if (ir2_opnd_is_mips_address(&op1) &&
            ir2_opnd_is_mips_address(&op2)) {
            ir2_set_opcode(pir2, LISA_SUB_D);
            ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
        } else if (ir2_opnd_is_mips_address(&op1) &&
                   ir2_opnd_is_sx(&op2, 32)) {
            ir2_set_opcode(pir2, LISA_SUB_D);
            ir2_opnd_set_em(&op0, EM_MIPS_ADDRESS, 32);
        } else if (ir2_opnd_is_sx(&op1, 32) && ir2_opnd_is_sx(&op2, 32)) {
            ir2_set_opcode(pir2, LISA_SUB_W);
            ir2_opnd_set_em_sub2(&op0, &op1, &op2);
            if (!ir2_opnd_is_sx(&op0, 32)) {
                ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_set_opcode(pir2, LISA_ADD_D);
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        }
        break;
    case LISA_SUB_ADDRX:
        if (ir2_opnd_is_x86_address(&op1) &&
            ir2_opnd_is_x86_address(&op2)) {
            ir2_set_opcode(pir2, LISA_SUB_D);
            ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
        } else if (ir2_opnd_is_x86_address(&op1) &&
                   ir2_opnd_is_sx(&op2, 32)) {
            ir2_set_opcode(pir2, LISA_SUB_D);
            ir2_opnd_set_em(&op0, EM_X86_ADDRESS, 32);
        } else if (ir2_opnd_is_sx(&op1, 32) && ir2_opnd_is_sx(&op2, 32)) {
            ir2_set_opcode(pir2, LISA_SUB_W);
            ir2_opnd_set_em_sub2(&op0, &op1, &op2);
            if (!ir2_opnd_is_sx(&op0, 32)) {
                ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_set_opcode(pir2, LISA_SUB_D);
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        }
        break;
    case LISA_ADD_D:
        ir2_opnd_set_em_add2(&op0, &op1, &op2);
        break;
    case LISA_SUB_D:
        ir2_opnd_set_em_sub2(&op0, &op1, &op2);
        break;
    case LISA_ADD_W:
        //lsassert(ir2_opnd_is_sx(&op1, 32) && ir2_opnd_is_sx(&op2, 32));
        ir2_opnd_set_em_add2(&op0, &op1, &op2);
        if (!ir2_opnd_is_sx(&op0, 32)) {
            ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
        }
        break;
    case LISA_SUB_W:
        //lsassert(ir2_opnd_is_sx(&op1, 32) && ir2_opnd_is_sx(&op2, 32));
        ir2_opnd_set_em_sub2(&op0, &op1, &op2);
        if (!ir2_opnd_is_sx(&op0, 32)) {
            ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
        }
        break;
    case LISA_AND:
    case LISA_ANDN:
        if (ir2_opnd_is_x86_address(&op1) ||
            ir2_opnd_is_x86_address(&op2)) {
            ir2_opnd_set_em(&op0, EM_X86_ADDRESS, 32);
        } else if (ir2_opnd_is_mips_address(&op1) ||
                   ir2_opnd_is_mips_address(&op2)) {
            ir2_opnd_set_em(&op0, EM_MIPS_ADDRESS, 32);
        } else {
            ir2_opnd_set_em_and2(&op0, &op1, &op2);
        }
        break;
    case LISA_OR:
        if (ir2_opnd_is_ax(&op1, 32) || ir2_opnd_is_ax(&op2, 32)) {
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        }
        if (ir2_opnd_is_mips_address(&op1) ||
            ir2_opnd_is_mips_address(&op2)) {
            /*if (op1.is_mips_address() && op2 == zero_ir2_opnd)*/
            if (ir2_opnd_is_mips_address(&op1) &&
                ir2_opnd_cmp(&op2, &zero_ir2_opnd)) {
                ir2_opnd_set_em_mov(&op0, &op1);
            } else {
                lsassertm(0, "not implemented yet\n");
            }
        } else if (ir2_opnd_is_x86_address(&op1)) {
            lsassert(!ir2_opnd_is_address(&op2));
            if (ir2_opnd_is_zx(&op2, 32)) {
                ir2_opnd_set_em(&op0, EM_X86_ADDRESS, 32);
            } else {
                ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
            }
        } else if (ir2_opnd_is_x86_address(&op2)) {
            lsassert(!ir2_opnd_is_address(&op1));
            if (ir2_opnd_is_zx(&op1, 32)) {
                ir2_opnd_set_em(&op0, EM_X86_ADDRESS, 32);
            } else {
                ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_opnd_set_em_or2(&op0, &op1, &op2);
        }
        break;
    case LISA_NOR:
        if (ir2_opnd_is_ax(&op1, 32) || ir2_opnd_is_ax(&op2, 32)) {
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        } else if (ir2_opnd_is_address(&op1) || ir2_opnd_is_address(&op2)) {
            lsassertm(0, "not implemented yet\n");
        } else {
            ir2_opnd_set_em_nor2(&op0, &op1, &op2);
        }
        break;
    case LISA_XOR:
        if (ir2_opnd_is_ax(&op1, 32) || ir2_opnd_is_ax(&op2, 32)) {
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        } else {
            ir2_opnd_set_em_xor2(&op0, &op1, &op2);
        }
        break;
    case LISA_SLL_W:
        ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
        break;
    case LISA_SLL_D:
        ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        break;
    case LISA_SRL_W:
        //lsassert(ir2_opnd_is_sx(&op1, 32));
        ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
        break;
    case LISA_SRL_D:
        if (ir2_opnd_is_ax(&op1, 32)) {
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        } else if (ir2_opnd_is_address(&op1)) {
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        } else if (ir2_opnd_is_zx(&op1, 63)) {
            ir2_opnd_set_em_mov(&op0, &op1);
        } else {
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        }
        break;
    case LISA_SRA_W:
        lsassert(ir2_opnd_is_sx(&op1, 32));
        ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
        break;
    case LISA_SRA_D:
        if (ir2_opnd_is_ax(&op1, 32)) {
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        } else if (ir2_opnd_is_address(&op1)) {
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        } else if (ir2_opnd_is_sx(&op1, 63)) {
            ir2_opnd_set_em_mov(&op0, &op1);
        } else {
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        }
        break;
    case LISA_SLTU:
    case LISA_SLT:
        ir2_opnd_set_em(&op0, ZERO_EXTENSION, 1);
        break;
    //case mips_dmult_g:
    //    ir2_opnd_set_em_dmult_g(&op0, &op1, &op2);
    //    break;
    case LISA_MUL_D:
        ir2_opnd_set_em_dmultu_g(&op0, &op1, &op2);
        break;
    //case mips_mult_g:
    //    ir2_opnd_set_em_dmult_g(&op0, &op1, &op2);
    //    if (!ir2_opnd_is_sx(&op0, 32)) {
    //        ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
    //    }
    //    break;
    case LISA_MUL_W:
        lsassert(ir2_opnd_is_sx(&op1, 32) && ir2_opnd_is_sx(&op2, 32));
        ir2_opnd_set_em_dmultu_g(&op0, &op1, &op2);
        if (!ir2_opnd_is_sx(&op0, 32)) {
            ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
        }
        break;
    //case mips_movz: //TODO
    //case mips_movn:
    //    // ir2_opnd_set_em_movcc(&op0, &op1, &op2);
    //    // arg error
    //    ir2_opnd_set_em_movcc(&op0, &op1, &op0);
    //    break;
    default:
        lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                  ir2_name(opcode));
        break;
    }
    ir2_append(pir2);
    if(!ir2_op_check(pir2))
        lsassertm(0, "Maybe you should check the type of operand");
    return pir2;
}

IR2_INST *la_append_ir2_opnd2i_em(IR2_OPCODE opcode, IR2_OPND dest, IR2_OPND src, int imm)
{
    if(opcode == LISA_ANDI || opcode == LISA_ORI || 
       opcode == LISA_XORI || opcode == LISA_LU52I_D) {
        lsassert((unsigned int)(imm) <= 0xfff);
    } else if (opcode == LISA_SUBIU || opcode == LISA_DSUBIU) {
        lsassert(imm > -2048 && imm <= 2048);
    } else {
        lsassert(imm >= -2048 && imm < 2048);
    }

    /*
     * This code is used for add AND xx, n1.
     */
#ifdef N64 /* validate address */
    if (la_ir2_opcode_is_load(opcode) || la_ir2_opcode_is_store(opcode)) {
        int base_ireg_num = ir2_opnd_base_reg_num(&src);
        base_ireg_num = em_validate_address(base_ireg_num);
    }
#endif

    IR2_INST *pir2 = ir2_allocate();
    pir2->_opcode = opcode;
    pir2->op_count = 3;
    pir2->_opnd[0] = dest;
    pir2->_opnd[1] = src;

    switch (opcode) {
    case LISA_LOAD_ADDR:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_set_opcode(pir2, LISA_LD_D);
        ir2_opnd_set_em(&dest, EM_X86_ADDRESS, 32);
        break;
    case LISA_LOAD_ADDRX:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_set_opcode(pir2, LISA_LD_WU);
        ir2_opnd_set_em(&dest, EM_X86_ADDRESS, 32);
        break;
    case LISA_STORE_ADDR:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_set_opcode(pir2, LISA_ST_D);
        break;
    case LISA_STORE_ADDRX:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_set_opcode(pir2, LISA_ST_W);
        break;
    case LISA_SUBI_ADDR:
    case LISA_ADDI_ADDR:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        if (opcode == LISA_SUBI_ADDR) {
            imm = -imm;
        }
        opcode = LISA_ADDI_ADDR;
        if (ir2_opnd_is_mips_address(&src)) {
            ir2_set_opcode(pir2, LISA_ADDI_D);
            ir2_opnd_set_em(&dest, EM_MIPS_ADDRESS, 32);
        } else if (ir2_opnd_is_sx(&src, 32)) {
            ir2_set_opcode(pir2, LISA_ADDI_W);
            ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
            if (!ir2_opnd_is_sx(&dest, 32)) {
                ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_set_opcode(pir2, LISA_ADDI_D);
            ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
        }
        break;
    case LISA_SUBI_ADDRX:
    case LISA_ADDI_ADDRX:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        if (opcode == LISA_SUBI_ADDRX) {
            imm = -imm;
        }
        opcode = LISA_ADDI_ADDRX;
        if (ir2_opnd_is_x86_address(&src)) {
            ir2_set_opcode(pir2, LISA_ADDI_D);
            ir2_opnd_set_em(&dest, EM_X86_ADDRESS, 32);
        } else if (ir2_opnd_is_sx(&src, 32)) {
            ir2_set_opcode(pir2, LISA_ADDI_D);
            ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
            if (!ir2_opnd_is_sx(&dest, 32)) {
                ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_set_opcode(pir2, LISA_ADDI_D);
            ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
        }
        break;
    case LISA_LL_D:
        lsassertm(((imm % 4) == 0), "ll_d imm %d error.\n", imm);
        imm = imm >> 2;
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, UNKNOWN_EXTENSION, 32);
        break;
    case LISA_LD_D:
    case LISA_LDR_D:
    case LISA_LDL_D:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, UNKNOWN_EXTENSION, 32);
        break;
    case LISA_LL_W:
        lsassertm(((imm % 4) == 0), "ll_w imm %d error.\n", imm);
        imm = imm >> 2;
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        break;
    case LISA_LD_W:
    case LISA_LDR_W:
    case LISA_LDL_W:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        break;
    case LISA_LD_H:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 16);
        break;
    case LISA_LD_B:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 8);
        break;
    case LISA_LD_WU:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, ZERO_EXTENSION, 32);
        break;
    case LISA_LD_HU:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, ZERO_EXTENSION, 16);
        break;
    case LISA_LD_BU:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, ZERO_EXTENSION, 8);
        break;
    case LISA_SUBIU:
    case LISA_ADDI_W:
        if (opcode == LISA_SUBIU) {
            ir2_set_opcode(pir2, LISA_ADDI_W);
            imm = -imm;
        }
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        //lsassert(ir2_opnd_is_sx(&src, 32));
        lsassertm(!ir2_opnd_is_address(&src),
                  "should use addi_addr or addi_addrx\n");
        ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
        if (!ir2_opnd_is_sx(&dest, 32)) {
            ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        }
        break;
    case LISA_DSUBIU:
    case LISA_ADDI_D:
        if (opcode == LISA_DSUBIU) {
            ir2_set_opcode(pir2, LISA_ADDI_D);
            imm = -imm;
        }
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassertm(!ir2_opnd_is_address(&src),
                  "should use addi_addr or addi_addrx\n");
        ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
        break;
    //case LISA_daddi:
    //    ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
    //    break;
    case LISA_SLTI:
    case LISA_SLTUI: //TODO imm sign extend, but compared as unsigned
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, ZERO_EXTENSION, 1);
        break;
    case LISA_ANDI:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em_and(&dest, &src, ZERO_EXTENSION, uint16_eb(imm));
        break;
    case LISA_ORI:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em_or(&dest, &src, ZERO_EXTENSION, uint16_eb(imm));
        break;
    case LISA_XORI:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em_xor(&dest, &src, ZERO_EXTENSION, uint16_eb(imm));
        break;
    case LISA_SLLI_W:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert((uint32)imm <= 31);
        ir2_opnd_set_em_dsll(&dest, &src, imm);
        if (!ir2_opnd_is_sx(&dest, 32)) {
            ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        }
        break;
    case LISA_SRAI_W:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        //lsassert(ir2_opnd_is_sx(&src, 32));
        lsassert((uint32)imm <= 31);
        if (!ir2_opnd_is_sx(&src, 32)) {
            ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        } else {
            ir2_opnd_set_em_mov(&dest, &src);
        }
        ir2_opnd_set_em_dsra(&dest, &dest, imm); /* may be a problem */
        lsassert(ir2_opnd_is_sx(&dest, 32));
        break;
    case LISA_SRLI_W:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        //lsassert(ir2_opnd_is_zx(&src, 32));
        lsassert((uint32)imm <= 31);
        if (!ir2_opnd_is_zx(&src, 32)) {
            ir2_opnd_set_em(&dest, ZERO_EXTENSION, 32);
        } else {
            ir2_opnd_set_em_mov(&dest, &src);
        }
        ir2_opnd_set_em_dsrl(&dest, &dest, imm);
        lsassert(ir2_opnd_is_sx(&dest, 32));
        break;
    case LISA_SLLI_D:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert((uint32)imm <= 63);
        ir2_opnd_set_em_dsll(&dest, &src, imm);
        break;
    case LISA_SRAI_D:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert((uint32)imm <= 63);
        ir2_opnd_set_em_dsra(&dest, &src, imm);
        break;
    case LISA_SRLI_D:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert((uint32)imm <= 63);
        ir2_opnd_set_em_dsrl(&dest, &src, imm);
        break;
    case LISA_VSHUF4I_B:
    case LISA_VSHUF4I_H:
    case LISA_VSHUF4I_W:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert((imm & 0xffffff00) == 0);
        //ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        //ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case LISA_VLD:
        /*
         * VLD si12
         */
         lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_ireg(&src));
        break;
    default:
        printf("[LATX] [%s] not implemented in %s : %d\n", ir2_name(opcode),
				__func__, __LINE__);
        exit(-1);
        break;
    }

    IR2_OPND op2 = create_ir2_opnd(IR2_OPND_IMMH, imm);
    pir2->_opnd[2] = op2;
    ir2_append(pir2);
    if(!ir2_op_check(pir2))
        lsassertm(0, "Maybe you should check the type of operand");
    return pir2;
}

IR2_INST *la_append_ir2_opnd2_em(IR2_OPCODE opcode, IR2_OPND dest, IR2_OPND src) {
    if (opcode == LISA_MOV64) {
        if (ir2_opnd_cmp(&dest, &src)) {
            return NULL;
        } else {
            IR2_INST *p = la_append_ir2_opnd3_em(LISA_OR, dest, src, zero_ir2_opnd);
            ir2_opnd_set_em_mov(&dest, &src);
            return p;
        }
    } else if (opcode == LISA_MOV32_SX) {
        if (ir2_opnd_cmp(&dest, &src) && ir2_opnd_is_sx(&src, 32)) {
            return NULL;
        } else {
            return la_append_ir2_opnd2i_em(LISA_SLLI_W, dest, src, 0);
        }
    } else if (opcode == LISA_MOV32_ZX) {
        if (ir2_opnd_cmp(&dest, &src) && ir2_opnd_is_zx(&src, 32)) {
            return NULL;
        } else {
            return la_append_ir2_opnd3_em(LISA_AND, dest, src, n1_ir2_opnd);
        }
    } else if (opcode == LISA_MOV_ADDRX) {
#ifdef N64
        if (ir2_opnd_cmp(&dest, &src) && ir2_opnd_is_zx(&src, 32)) {
            if (!(ir2_opnd_cmp(&dest, &zero_ir2_opnd))) {
                ir2_opnd_set_em(&dest, EM_X86_ADDRESS, 32);
            }
            return NULL;
        } else {
            IR1_INST *pir1 = lsenv->tr_data->curr_ir1_inst;
            if (pir1 != NULL) {
                BITS_SET(pir1->flags, FI_ZX_ADDR_USED);
            }
            IR2_INST *p = la_append_ir2_opnd3_em(LISA_AND, dest, src, n1_ir2_opnd);
            ir2_opnd_set_em(&dest, EM_X86_ADDRESS, 32);
            return p;
        }
#else
        if (ir2_opnd_cmp(&dest, &src) && ir2_opnd_is_sx(&src)) {
            return NULL;
        } else {
            return la_append_ir2_opnd2i_em(LISA_SLLI_W, dest, src, 0);
        }
#endif
    }

    IR2_INST *p = ir2_allocate();

    IR2_OPND ir2_opnd_none = ir2_opnd_new_none();

    switch (opcode) {
    case LISA_MOVFCSR2GR:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_creg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        break;
    case LISA_MOVGR2FCSR:
        lsassert(ir2_opnd_is_ireg(&src) && ir2_opnd_is_creg(&dest));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case LISA_MOVFR2GR_S:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        break;
    case LISA_MOVFR2GR_D:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        ir2_opnd_set_em(&dest, UNKNOWN_EXTENSION, 32);
        break;
    case LISA_MOVGR2FR_W:
    case LISA_MOVGR2FR_D:
        lsassert(ir2_opnd_is_ireg(&src) && ir2_opnd_is_freg(&dest));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
#if 0
    case mips_mult:
    case mips_multu:
    case mips_div:
    case mips_divu:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src) &&
                 ir2_opnd_is_sx(&dest, 32) && ir2_opnd_is_sx(&src, 32));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        lsenv->tr_data->hi_em = SIGN_EXTENSION;
        lsenv->tr_data->lo_em = SIGN_EXTENSION;
        break;
    case mips_dmult:
    case mips_dmultu:
    case mips_ddiv:
    case mips_ddivu:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        lsenv->tr_data->hi_em = UNKNOWN_EXTENSION;
        lsenv->tr_data->lo_em = UNKNOWN_EXTENSION;
        break;
    case mips_teq:
    case mips_tne:
    case mips_tge:
    case mips_tlt:
#endif
    case LISA_X86ADC_B:
    case LISA_X86ADC_H:
    case LISA_X86ADC_W:
    case LISA_X86ADC_D:

    case LISA_X86ADD_B:
    case LISA_X86ADD_H:
    case LISA_X86ADD_W:
    case LISA_X86ADD_D:

    case LISA_X86SBC_B:
    case LISA_X86SBC_H:
    case LISA_X86SBC_W:
    case LISA_X86SBC_D:

    case LISA_X86SUB_B:
    case LISA_X86SUB_H:
    case LISA_X86SUB_W:
    case LISA_X86SUB_D:

    case LISA_X86XOR_B:
    case LISA_X86XOR_H:
    case LISA_X86XOR_W:
    case LISA_X86XOR_D:

    case LISA_X86AND_B:
    case LISA_X86AND_H:
    case LISA_X86AND_W:
    case LISA_X86AND_D:

    case LISA_X86OR_B:
    case LISA_X86OR_H:
    case LISA_X86OR_W:
    case LISA_X86OR_D:

    case LISA_X86SLL_B:
    case LISA_X86SLL_H:
    case LISA_X86SLL_W:
    case LISA_X86SLL_D:

    case LISA_X86SRL_B:
    case LISA_X86SRL_H:
    case LISA_X86SRL_W:
    case LISA_X86SRL_D:

    case LISA_X86SRA_B:
    case LISA_X86SRA_H:
    case LISA_X86SRA_W:
    case LISA_X86SRA_D:

    case LISA_X86ROTL_B:
    case LISA_X86ROTL_H:
    case LISA_X86ROTL_W:
    case LISA_X86ROTL_D:

    case LISA_X86RCR_B:
    case LISA_X86RCR_H:
    case LISA_X86RCR_W:
    case LISA_X86RCR_D:

    case LISA_X86RCL_B:
    case LISA_X86RCL_H:
    case LISA_X86RCL_W:
    case LISA_X86RCL_D:

    case LISA_X86ROTR_B:
    case LISA_X86ROTR_H:
    case LISA_X86ROTR_W:
    case LISA_X86ROTR_D:

    case LISA_X86MUL_B:
    case LISA_X86MUL_H:
    case LISA_X86MUL_W:
    case LISA_X86MUL_D:

    case LISA_X86MUL_BU:
    case LISA_X86MUL_HU:
    case LISA_X86MUL_WU:
    case LISA_X86MUL_DU:
        /*
         * FIXME: for LISA_X86MUL_*, we need add new assert here.
         */
        //lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
#if 0
    case mips_sqrt_d:
    case mips_sqrt_s:
    case mips_rsqrt_s:
    case mips_rsqrt_d:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_c_un_d:
    case mips_c_un_s:
    case mips_c_lt_d:
    case mips_c_lt_s:
    case mips_cvt_l_d:
    case mips_cvt_w_d:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_biadd:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_pmovmskb:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_floor_l_d:
    case mips_floor_l_s:
    case mips_ceil_l_d:
    case mips_trunc_l_d:
    case mips_round_l_d:
    case mips_round_l_s:
    case mips_floor_w_d:
    case mips_ceil_w_d:
    case mips_trunc_w_d:
    case mips_round_w_d:
    case mips_floor_w_s:
    case mips_ceil_w_s:
    case mips_trunc_w_s:
    case mips_round_w_s:
    case mips_movev:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_vsignfillb:
    case mips_vsignfillh:
    case mips_vsignfillw:
    case mips_vsignfilld:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_vgetsignb:
    case mips_vgetsignh:
    case mips_vgetsignw:
    case mips_vgetsignd:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_frcpw:
    case mips_frcpd:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_fsqrtw:
    case mips_fsqrtd:
    case mips_frsqrtw:
    case mips_frsqrtd:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_vacc8b_ud:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_insveb:
    case mips_insveh:
    case mips_insvew:
    case mips_insved:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
#endif
    default:
        if (opcode == LISA_FMOV_D || ir2_opcode_is_convert(opcode) ||
            ir2_opcode_is_fcmp(opcode)) {
            lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
            ir2_build(p, opcode, dest, src, ir2_opnd_none);
        } else if (ir2_opcode_is_branch(opcode) &&
                   !ir2_opcode_is_branch_with_3opnds(opcode) &&
                   opcode != LISA_B) {
            lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_label(&src));
            ir2_build(p, opcode, dest, src, ir2_opnd_none);
            //p = append_ir2_opnd0(mips_nop);
        } else {
            lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                      ir2_name(opcode));
        }
        break;
    }

    ir2_append(p);
    if(!ir2_op_check(p))
        lsassertm(0, "Maybe you should check the type of operand");
    return p;
}

IR2_INST *la_append_ir2_opnd1i_em(IR2_OPCODE opcode, IR2_OPND op0, int imm) {
    IR2_INST *pir2 = ir2_allocate();
    IR2_OPND op1= create_ir2_opnd(IR2_OPND_IMMH, imm);
    pir2->_opcode = opcode;
    pir2->op_count = 2;
    pir2->_opnd[0] = op0;
    pir2->_opnd[1] = op1;
    ir2_append(pir2);

    switch (opcode) {
    case LISA_X86MFFLAG:
        ir2_opnd_set_em(&op0, ZERO_EXTENSION, 12);
        break;
    case LISA_LU12I_W:
        ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
        break;
    case LISA_LU32I_D:
        ir2_opnd_set_em(&op0, SIGN_EXTENSION, 52);
        break;
    default:
        lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                    ir2_name(opcode));
        break;
    }

    lsassertm(ir2_op_check(pir2), "Maybe you should check the type of operand");
    return pir2;
}

IR2_INST *la_append_ir2_opnda(IR2_OPCODE opcode, ADDR addr)
{
    IR2_INST *p = ir2_allocate();
    IR2_OPND ir2_opnd_addr;
    IR2_OPND ir2_opnd_none = ir2_opnd_new_none();

    ir2_opnd_build(&ir2_opnd_addr, IR2_OPND_ADDR, addr);
    switch (opcode) {
        case LISA_B:
        case LISA_BL:
        case LISA_X86_INST:
        case LISA_DUP:
            ir2_build(p, opcode, ir2_opnd_addr, ir2_opnd_none, ir2_opnd_none);
            break;
    default:
            lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                      ir2_name(opcode));
            break;
    }

    ir2_append(p);
    ir2_op_check(p);
    return p;
}

#ifdef CONFIG_SOFTMMU

#define LATXS_REG_ALLOC_DEFINE_IR2(ir2name, ir2type, ir2val) \
IR2_OPND ir2name = {._type = ir2type, .val = ir2val}

#define LATXS_REG_ALLOC_DEFINE_IR2_GPR(ir2name, ir2val) \
LATXS_REG_ALLOC_DEFINE_IR2(ir2name, IR2_OPND_GPR, ir2val);

/* Integer Register */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_zero_ir2_opnd,    0); /* ZERO */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_sp_ir2_opnd,      3); /* SP */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_fp_ir2_opnd,     22); /* FP */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_env_ir2_opnd,    23); /* S0 */

/* Floating Point  Register */
LATXS_REG_ALLOC_DEFINE_IR2(latxs_f32_ir2_opnd, IR2_OPND_FPR, 8);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcsr_ir2_opnd, IR2_OPND_FCSR, 0);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcsr1_ir2_opnd, IR2_OPND_FCSR, 1);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcsr2_ir2_opnd, IR2_OPND_FCSR, 2);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcsr3_ir2_opnd, IR2_OPND_FCSR, 3);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcc0_ir2_opnd, IR2_OPND_CC, 0);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcc1_ir2_opnd, IR2_OPND_CC, 1);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcc2_ir2_opnd, IR2_OPND_CC, 2);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcc3_ir2_opnd, IR2_OPND_CC, 3);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcc4_ir2_opnd, IR2_OPND_CC, 4);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcc5_ir2_opnd, IR2_OPND_CC, 5);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcc6_ir2_opnd, IR2_OPND_CC, 6);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcc7_ir2_opnd, IR2_OPND_CC, 7);

/* Helper Arguments Register */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_arg0_ir2_opnd,  4); /* a0 */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_arg1_ir2_opnd,  5); /* a1 */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_arg2_ir2_opnd,  6); /* a2 */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_arg3_ir2_opnd,  7); /* a3 */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_arg4_ir2_opnd,  8); /* a4/stmp1      */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_arg5_ir2_opnd,  9); /* a5/stmp2      */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_arg6_ir2_opnd, 10); /* a6/TB address */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_arg7_ir2_opnd, 11); /* a7/eip        */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_ret0_ir2_opnd,  4); /* v0 */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_ret1_ir2_opnd,  5); /* v1 */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_ra_ir2_opnd,    1); /* ra */

/* Static Temp Registers */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_stmp1_ir2_opnd, 8); /* a4 */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_stmp2_ir2_opnd, 9); /* a5 */

/* Consist value */
LATXS_REG_ALLOC_DEFINE_IR2(latxs_invalid_ir2_opnd, IR2_OPND_INV, 0);

LATXS_REG_ALLOC_DEFINE_IR2(latxs_scr0_ir2_opnd, IR2_OPND_SCR, 0);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_scr1_ir2_opnd, IR2_OPND_SCR, 1);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_scr2_ir2_opnd, IR2_OPND_SCR, 2);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_scr3_ir2_opnd, IR2_OPND_SCR, 3);

/* Functions to build IR2_OPND */
IR2_OPND latxs_ir2_opnd_new(IR2_OPND_TYPE type, int value)
{
    IR2_OPND opnd;
    latxs_ir2_opnd_build(&opnd, type, value);
    return opnd;
}

IR2_OPND latxs_ir2_opnd_new_inv(void)
{
    return latxs_invalid_ir2_opnd;
}

IR2_OPND latxs_ir2_opnd_new_label(void)
{
    IR2_OPND opnd;
    opnd._type = IR2_OPND_LABEL;
    opnd.val   = ++(lsenv->tr_data->label_num);
    return opnd;
}

void latxs_ir2_opnd_build(IR2_OPND *opnd, IR2_OPND_TYPE t, int value)
{
    opnd->_type = t;
    opnd->val   = value;
}

void latxs_ir2_opnd_build_mem(IR2_OPND *opnd, int base, int offset)
{
    opnd->_type = IR2_OPND_MEMY;
    opnd->val = base;
    opnd->imm = offset;
}

/* Functions to access IR2_OPND's fields */
IR2_OPND_TYPE latxs_ir2_opnd_type(IR2_OPND *opnd) { return opnd->_type; }
int latxs_ir2_opnd_reg(IR2_OPND *opnd) { return opnd->val; }
int latxs_ir2_opnd_imm(IR2_OPND *opnd) { return opnd->val; }
int latxs_ir2_opnd_label_id(IR2_OPND *opnd) { return opnd->val; }
int latxs_ir2_opnd_addr(IR2_OPND *opnd) { return opnd->val; }
int latxs_ir2_opnd_offset(IR2_OPND *opnd)
{
    lsassert(latxs_ir2_opnd_is_mem(opnd));
    return opnd->imm;
}

/* Functions to modify IR2_OPND_MEMY */
IR2_OPND latxs_ir2_opnd_mem_get_base(IR2_OPND *opnd)
{
    lsassert(latxs_ir2_opnd_is_mem(opnd));
    IR2_OPND ir2_opnd =
        latxs_ir2_opnd_new(IR2_OPND_GPR, latxs_ir2_opnd_reg(opnd));
    return ir2_opnd;
}
void latxs_ir2_opnd_mem_set_base(IR2_OPND *mem, IR2_OPND *base)
{
    lsassert(latxs_ir2_opnd_is_mem(mem));
    mem->val = latxs_ir2_opnd_reg(base);
}
int latxs_ir2_opnd_mem_get_offset(IR2_OPND *opnd)
{
    lsassertm(latxs_ir2_opnd_is_mem(opnd),
            "ir2 opnd offset could be used for IR2 OPND MEM only\n");
    return opnd->imm;
}
void latxs_ir2_opnd_mem_adjust_offset(IR2_OPND *mem, int offset)
{
    lsassert(latxs_ir2_opnd_is_mem(mem));
    mem->imm += offset;
}

/* Compare IR2 OPND */
int latxs_ir2_opnd_cmp(IR2_OPND *opnd1, IR2_OPND *opnd2)
{
    return opnd1->_type == opnd2->_type &&
           opnd1->val   == opnd2->val;
}

/* Functions to identify IR2_OPND's type/attribute */
int latxs_ir2_opnd_is_inv(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_INV;
}
int latxs_ir2_opnd_is_gpr(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_GPR;
}
int latxs_ir2_opnd_is_fpr(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_FPR;
}
int latxs_ir2_opnd_is_scr(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_SCR;
}
int latxs_ir2_opnd_is_fcsr(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_FCSR;
}
int latxs_ir2_opnd_is_cc(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_CC;
}
int latxs_ir2_opnd_is_label(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_LABEL;
}
int latxs_ir2_opnd_is_mem(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_MEMY;
}
int latxs_ir2_opnd_is_immd(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_IMMD;
}
int latxs_ir2_opnd_is_immh(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_IMMH;
}
int latxs_ir2_opnd_is_imm(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_IMMD || opnd->_type == IR2_OPND_IMMH;
}

int latxs_ir2_opnd_is_itemp(IR2_OPND *opnd)
{
    return latxs_ir2_opnd_is_gpr(opnd) && latxs_ir2_opnd_is_reg_temp(opnd);
}
int latxs_ir2_opnd_is_ftemp(IR2_OPND *opnd)
{
    return latxs_ir2_opnd_is_fpr(opnd) && latxs_ir2_opnd_is_reg_temp(opnd);
}
int latxs_ir2_opnd_is_mem_base_itemp(IR2_OPND *opnd)
{
    return latxs_ir2_opnd_is_mem(opnd) && latxs_ir2_opnd_is_reg_temp(opnd);
}
int latxs_ir2_opnd_is_reg_temp(IR2_OPND *opnd)
{
    int reg;
    switch (latxs_ir2_opnd_type(opnd)) {
    case IR2_OPND_MEMY:
    case IR2_OPND_GPR: {
        reg = latxs_ir2_opnd_reg(opnd);
#ifdef LATXS_ITMP_CONTINUS
        return LATXS_ITMP_MIN <= reg && reg <= LATXS_ITMP_MAX;
#else
        int i = 0;
        for (; i < latxs_itemp_status_num; i++) {
            if (reg == latxs_itemp_status_default[i].physical_id) {
                return 1;
            }
        }
        return 0;
#endif
    }
    case IR2_OPND_FPR: {
        reg = latxs_ir2_opnd_reg(opnd);
#ifdef LATXS_FTMP_CONTINUS
        return LATXS_FTMP_MIN <= reg && reg <= LATXS_FTMP_MAX;
#else
        int i = 0;
        for (; i < ftemp_status_num; i++) {
            if (reg == ftemp_status_default[i].physical_id) {
                return 1;
            }
        }
        return 0;
#endif
    }
    default: {
        lsassert(0);
        return -1;
    }
    }
}

/* Functions to convert IR2_INST to string */
int latxs_ir2_dump(IR2_INST *ir2)
{
    char str[64];
    int size = 0;

    if (latxs_ir2_opcode(ir2) == 0) {
        /*
         * an empty IR2_INST was inserted into the ir2 list,
         * but not assigned yet.
         */
        return 0;
    }

    size = latxs_ir2_to_string(ir2, str);
    fprintf(stderr, "[%03d] %s\n", ir2->_id, str);
    return size;
}

int latxs_ir2_to_string(IR2_INST *ir2, char *str)
{
    int length = 0;
    int i = 0;
    bool hex = false;

    length = sprintf(str, "%-8s  ", ir2_name(ir2_opcode(ir2)));

    if (latxs_ir2_opcode(ir2) == LISA_ANDI ||
        latxs_ir2_opcode(ir2) == LISA_ORI  ||
        latxs_ir2_opcode(ir2) == LISA_XORI ||
        latxs_ir2_opcode(ir2) == LISA_LU12I_W) {
        hex = true;
    }

    for (i = 0; i < 4; ++i) {
        IR2_OPND *opnd = &ir2->_opnd[i];
        if (latxs_ir2_opnd_type(opnd) == IR2_OPND_INV) {
            return length;
        } else {
            if (i > 0) {
                strcat(str, ",");
                length += 1;
            }
            length += latxs_ir2_opnd_to_string(opnd, str + length, hex);
        }
    }

    return length;
}

int latxs_ir2_opnd_to_string(IR2_OPND *opnd, char *str, bool hex)
{
    int reg_num = latxs_ir2_opnd_reg(opnd);

    switch (latxs_ir2_opnd_type(opnd)) {
    case IR2_OPND_INV: return 0;
    case IR2_OPND_GPR: {
        strcpy(str, latxs_ir2_name(reg_num));
        return strlen(str);
    }
    case IR2_OPND_FPR: {
        strcpy(str, latxs_ir2_name(40 + reg_num));
        return strlen(str);
    }
    case IR2_OPND_SCR: {
        return sprintf(str, "$scr%d", reg_num);
    }
    case IR2_OPND_FCSR: {
        return sprintf(str, "$c%d", reg_num);
    }
    case IR2_OPND_CC: {
        return sprintf(str, "$c%d", reg_num);
    }
    case IR2_OPND_IMMD: {
        return sprintf(str, "%d", latxs_ir2_opnd_imm(opnd));
    }
    case IR2_OPND_IMMH: {
        if (hex) {
            return sprintf(str, "0x%x", (uint16_t)latxs_ir2_opnd_imm(opnd));
        } else {
            return sprintf(str, "%d", latxs_ir2_opnd_imm(opnd));
        }
    }
    case IR2_OPND_LABEL:
        return sprintf(str, "LABEL %d", latxs_ir2_opnd_imm(opnd));
    case IR2_OPND_MEMY: {
        return sprintf(str, "0x%x(%s)",
                       latxs_ir2_opnd_imm(opnd),
                       latxs_ir2_name(reg_num));
    }
    default:
        lsassertm(0, "type = %d\n", latxs_ir2_opnd_type(opnd));
        return 0;
    }
}

const char *latxs_ir2_name(int value)
{
    return ir2_name(value);
}

/* Function used during label disposing */
void latxs_ir2_opnd_convert_label_to_imm(IR2_OPND *opnd, int imm)
{
    lsassert(latxs_ir2_opnd_is_label(opnd));
    opnd->_type = IR2_OPND_IMMH;
    opnd->val = imm;
}

/* Functions to manage IR2 extension mode */
static
int latxs_ir2_opnd_is_mapping(IR2_OPND *opnd2, int *x86_gpr_num)
{
    lsassert(latxs_ir2_opnd_is_gpr(opnd2));
    int reg = latxs_ir2_opnd_reg(opnd2);
    if (24 <= reg && reg <= 31) {
        *x86_gpr_num = reg - 24;
        return true;
    }
    return false;
}

void latxs_ir2_opnd_set_emb(IR2_OPND *opnd2, EXMode em, EXBits eb)
{
    opnd2->em = em; opnd2->eb = eb;
    int gpr_num = 0;
    if (latxs_ir2_opnd_is_mapping(opnd2, &gpr_num)) {
        latxs_td_set_reg_extmb(gpr_num, em, eb);
    }
}

void latxs_ir2_opnd_set_em(IR2_OPND *opnd2, EXMode em)
{
    opnd2->em = em;
    int gpr_num = 0;
    if (latxs_ir2_opnd_is_mapping(opnd2, &gpr_num)) {
        latxs_td_set_reg_extm(gpr_num, em);
    }
}

void latxs_ir2_opnd_set_eb(IR2_OPND *opnd2, EXBits eb)
{
    opnd2->eb = eb;
    int gpr_num = 0;
    if (latxs_ir2_opnd_is_mapping(opnd2, &gpr_num)) {
        latxs_td_set_reg_extb(gpr_num, eb);
    }
}

EXMode latxs_ir2_opnd_get_em(IR2_OPND *opnd2)
{
    int gpr_num = 0;
    if (latxs_ir2_opnd_is_mapping(opnd2, &gpr_num)) {
        return latxs_td_get_reg_extm(gpr_num);
    }
    return opnd2->em;
}

EXBits latxs_ir2_opnd_get_eb(IR2_OPND *opnd2)
{
    int gpr_num = 0;
    if (latxs_ir2_opnd_is_mapping(opnd2, &gpr_num)) {
        return latxs_td_get_reg_extb(gpr_num);
    }
    return opnd2->eb;
}

/* ------------------ IR2_INST ------------------ */

/* Fucntion to build IR2_INST */
void latxs_ir2_build(
        IR2_INST *ir2, IR2_OPCODE opcode,
        IR2_OPND *opnd0, IR2_OPND *opnd1,
        IR2_OPND *opnd2, IR2_OPND *opnd3)
{
    ir2->_opcode  = opcode;
    ir2->_opnd[0] = opnd0 ? *opnd0 : latxs_invalid_ir2_opnd;
    ir2->_opnd[1] = opnd1 ? *opnd1 : latxs_invalid_ir2_opnd;
    ir2->_opnd[2] = opnd2 ? *opnd2 : latxs_invalid_ir2_opnd;
    ir2->_opnd[3] = opnd3 ? *opnd3 : latxs_invalid_ir2_opnd;
}
void latxs_ir2_build0(IR2_INST *pir2, IR2_OPCODE opcode)
{
    latxs_ir2_build(pir2, opcode, NULL, NULL, NULL, NULL);
}
void latxs_ir2_build1(
        IR2_INST *pir2, IR2_OPCODE opcode,
        IR2_OPND *opnd0)
{
    latxs_ir2_build(pir2, opcode, opnd0, NULL, NULL, NULL);
}
void latxs_ir2_build2(
        IR2_INST *pir2, IR2_OPCODE opcode,
        IR2_OPND *opnd0, IR2_OPND *opnd1)
{
    latxs_ir2_build(pir2, opcode, opnd0, opnd1, NULL, NULL);
}
void latxs_ir2_build3(
        IR2_INST *pir2, IR2_OPCODE opcode,
        IR2_OPND *opnd0, IR2_OPND *opnd1,
        IR2_OPND *opnd2)
{
    latxs_ir2_build(pir2, opcode, opnd0, opnd1, opnd2, NULL);
}
void latxs_ir2_build4(
        IR2_INST *pir2, IR2_OPCODE opcode,
        IR2_OPND *opnd0, IR2_OPND *opnd1,
                IR2_OPND *opnd2, IR2_OPND *opnd3)
{
    latxs_ir2_build(pir2, opcode, opnd0, opnd1, opnd2, opnd3);
}

/* Fucntions ot access IR2_INST's fields */
void latxs_ir2_set_id(IR2_INST *ir2, int id)
{
    ir2->_id = id;
}
int latxs_ir2_get_id(IR2_INST *ir2)
{
    return ir2->_id;
}
IR2_OPCODE latxs_ir2_opcode(IR2_INST *ir2)
{
    return (IR2_OPCODE)(ir2->_opcode);
}
ADDR latxs_ir2_addr(IR2_INST *ir2)
{
    return ir2->_addr;
}
void latxs_ir2_set_addr(IR2_INST *ir2, ADDR a)
{
    ir2->_addr = a;
}
IR2_OPND *latxs_ir2_branch_get_label(IR2_INST *pir2)
{
    int index = latxs_ir2_branch_label_index(pir2);

    if (index >= 0) {
        IR2_OPND *label = &pir2->_opnd[index];
        if (latxs_ir2_opnd_is_label(label)) {
            return label;
        }
    } else {
        lsassertm(0, "unsupported branch label.\n");
    }

    return NULL;
}
int latxs_ir2_branch_label_index(IR2_INST *pir2)
{
    IR2_OPCODE opc = latxs_ir2_opcode(pir2);
    lsassert(latxs_ir2_opcode_is_branch(opc));
    switch (opc) {
    case LISA_BEQZ:
    case LISA_BNEZ: return 1; break;
    case LISA_B:
    case LISA_BL:   return 0; break;
    case LISA_BEQ:
    case LISA_BNE:
    case LISA_BLT:
    case LISA_BGE:
    case LISA_BLTU:
    case LISA_BGEU: return 2; break;
    case LISA_BCEQZ:
    case LISA_BCNEZ: return 1; break;
    default: return -1; break;
    }
    return -1;
}

/* Functions to manage the linked list of IR2_INST */
IR2_INST *latxs_ir2_allocate(void)
{
    TRANSLATION_DATA *t = lsenv->tr_data;
    /* 1. make sure we have enough space */
    if (t->ir2_inst_num_current == t->ir2_inst_num_max) {
        int bytes = sizeof(IR2_INST) * t->ir2_inst_num_max;
        /* double the array */
        t->ir2_inst_num_max *= 2;
        IR2_INST *back_ir2_inst_array = t->ir2_inst_array;
        t->ir2_inst_array = mm_realloc(t->ir2_inst_array, bytes << 1);
        t->first_ir2 = (IR2_INST *)((ADDR)t->first_ir2 -
                                    (ADDR)back_ir2_inst_array +
                                    (ADDR)t->ir2_inst_array);
        t->last_ir2  = (IR2_INST *)((ADDR)t->last_ir2  -
                                    (ADDR)back_ir2_inst_array +
                                    (ADDR)t->ir2_inst_array);
    }
    /* 2. allocate one */
    IR2_INST *p = t->ir2_inst_array + t->ir2_inst_num_current;
    latxs_ir2_set_id(p, t->ir2_inst_num_current);
    t->ir2_inst_num_current++;
    return p;
}

void latxs_ir2_append(IR2_INST *ir2)
{
    TRANSLATION_DATA *t = lsenv->tr_data;
    IR2_INST *former_last = t->last_ir2;
    if (former_last != NULL) {
        lsassert(t->first_ir2 != NULL);
        ir2->_prev = latxs_ir2_get_id(former_last);
        ir2->_next = -1;
        t->last_ir2 = ir2;
        former_last->_next = latxs_ir2_get_id(ir2);
    } else {
        lsassert(t->first_ir2 == NULL);
        ir2->_prev = -1;
        ir2->_next = -1;
        t->last_ir2 = ir2;
        t->first_ir2 = ir2;
    }
    if (ir2->_opcode >= LISA_GR2SCR) {
        t->real_ir2_inst_num++;
    }
}
void latxs_ir2_remove(IR2_INST *ir2)
{
    lsassert(0);
    TRANSLATION_DATA *t = lsenv->tr_data;
    IR2_INST *next = latxs_ir2_next(ir2);
    IR2_INST *prev = latxs_ir2_prev(ir2);
    if (t->first_ir2 == ir2) {
        if (t->last_ir2 == ir2) { /* head and tail */
            t->first_ir2 = NULL;
            t->last_ir2 = NULL;
        } else { /* head but not tail */
            t->first_ir2 = next;
            next->_prev = -1;
        }
    } else if (t->last_ir2 == ir2) { /* tail but not head */
        t->last_ir2 = prev;
        prev->_next = -1;
    } else {
        prev->_next = latxs_ir2_get_id(next);
        next->_prev = latxs_ir2_get_id(prev);
    }
    ir2->_prev = -1;
    ir2->_next = -1;
}
void latxs_ir2_insert_before(IR2_INST *ir2, IR2_INST *next)
{
    TRANSLATION_DATA *t = lsenv->tr_data;
    if (t->first_ir2 == next) {
        t->first_ir2 = ir2;
        ir2->_prev = -1;
        ir2->_next = latxs_ir2_get_id(next);
        next->_prev = latxs_ir2_get_id(ir2);
    } else {
        IR2_INST *prev = latxs_ir2_prev(next);
        ir2->_prev = latxs_ir2_get_id(prev);
        prev->_next = latxs_ir2_get_id(ir2);
        ir2->_next = latxs_ir2_get_id(next);
        next->_prev = latxs_ir2_get_id(ir2);
    }
}
void latxs_ir2_insert_after(IR2_INST *ir2, IR2_INST *prev)
{
    TRANSLATION_DATA *t = lsenv->tr_data;
    if (t->last_ir2 == prev) {
        t->last_ir2 = ir2;
        ir2->_next = -1;
        ir2->_prev = latxs_ir2_get_id(prev);
        prev->_next = latxs_ir2_get_id(ir2);
    } else {
        IR2_INST *next = latxs_ir2_next(prev);
        ir2->_next = latxs_ir2_get_id(next);
        next->_prev = latxs_ir2_get_id(ir2);
        ir2->_prev = latxs_ir2_get_id(prev);
        prev->_next = latxs_ir2_get_id(ir2);
    }
}
IR2_INST *latxs_ir2_prev(IR2_INST *ir2)
{
    if (ir2->_prev == -1) {
        return NULL;
    } else {
        return lsenv->tr_data->ir2_inst_array + ir2->_prev;
    }
}
IR2_INST *latxs_ir2_next(IR2_INST *ir2)
{
    if (ir2->_next == -1) {
        return NULL;
    } else {
        return lsenv->tr_data->ir2_inst_array + ir2->_next;
    }
}
IR2_INST *latxs_ir2_get(int id)
{
    lsassert(id >= 0 && id < lsenv->tr_data->ir2_inst_num_current);
    return lsenv->tr_data->ir2_inst_array + id;
}

/* Functions to generate IR2_INST and add into the linked list */
IR2_INST *latxs_append_ir2_opnd3i(IR2_OPCODE opcode,
        IR2_OPND *opnd0, IR2_OPND *opnd1,
        IR2_OPND *opnd2, int imm)
{
    IR2_INST *pir2 = latxs_ir2_allocate();

    IR2_OPND imm_opnd = latxs_ir2_opnd_new(IR2_OPND_IMMH, imm);
    latxs_ir2_build4(pir2, opcode, opnd0, opnd1, opnd2, &imm_opnd);

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    latxs_ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnd2ii(IR2_OPCODE opcode,
        IR2_OPND *opnd0, IR2_OPND *opnd1,
        int imm1, int imm2)
{
    IR2_INST *pir2 = latxs_ir2_allocate();
    IR2_OPND ir2_opnd2, ir2_opnd3;
    latxs_ir2_opnd_build(&ir2_opnd2, IR2_OPND_IMMH, imm1);
    latxs_ir2_opnd_build(&ir2_opnd3, IR2_OPND_IMMH, imm2);

    switch (opcode) {
    case LISA_BSTRINS_W:
    case LISA_BSTRINS_D:
    case LISA_BSTRPICK_W:
    case LISA_BSTRPICK_D:
        latxs_ir2_build4(pir2, opcode,
                opnd0, opnd1, &ir2_opnd2, &ir2_opnd3);
        break;
    default:
        lsassertm(0, "append opnd2ii unsupport LISA instruciton (%d)%s\n",
                opcode, latxs_ir2_name(opcode));
        break;
    }

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    latxs_ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnd3(IR2_OPCODE opcode,
        IR2_OPND  *opnd0, IR2_OPND *opnd1, IR2_OPND *opnd2)
{
    IR2_INST *pir2 = latxs_ir2_allocate();
    latxs_ir2_build3(pir2, opcode, opnd0, opnd1, opnd2);

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    latxs_ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnd4(IR2_OPCODE opcode,
    IR2_OPND  *opnd0, IR2_OPND *opnd1, IR2_OPND *opnd2, IR2_OPND *opnd3)
{
    IR2_INST *pir2 = latxs_ir2_allocate();
    latxs_ir2_build4(pir2, opcode, opnd0, opnd1, opnd2, opnd3);
    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));
    latxs_ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnd2i(IR2_OPCODE opcode,
        IR2_OPND  *opnd0, IR2_OPND *opnd1, int32_t imm)
{
    IR2_INST *pir2 = latxs_ir2_allocate();

    IR2_OPND imm_opnd = latxs_ir2_opnd_new(IR2_OPND_IMMH, imm);
    latxs_ir2_build3(pir2, opcode, opnd0, opnd1, &imm_opnd);

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    latxs_ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnd2(IR2_OPCODE opcode,
        IR2_OPND  *opnd0, IR2_OPND *opnd1)
{
    if (latxs_ir2_opcode_is_load(opcode) ||
        latxs_ir2_opcode_is_store(opcode)) {
        /* opnd1 could be IR2 OPND MEM */
        if (latxs_ir2_opnd_type(opnd1) == IR2_OPND_MEMY) {
            IR2_OPND base = latxs_ir2_opnd_mem_get_base(opnd1);
            int offset = latxs_ir2_opnd_mem_get_offset(opnd1);
            return latxs_append_ir2_opnd2i(opcode, opnd0, &base, offset);
        }
    }

    IR2_INST *pir2 = latxs_ir2_allocate();
    latxs_ir2_build2(pir2, opcode, opnd0, opnd1);

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    latxs_ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnd1i(IR2_OPCODE opcode,
        IR2_OPND  *opnd0, int32 imm)
{
    IR2_INST *pir2 = latxs_ir2_allocate();
    IR2_OPND imm_opnd = latxs_ir2_opnd_new(IR2_OPND_IMMH, imm);
    latxs_ir2_build2(pir2, opcode, opnd0, &imm_opnd);

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    latxs_ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnd1(IR2_OPCODE opcode,
        IR2_OPND  *opnd0)
{
    IR2_INST *pir2 = latxs_ir2_allocate();
    latxs_ir2_build1(pir2, opcode, opnd0);

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    latxs_ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opndi(IR2_OPCODE opcode,
        int32_t imm)
{
    IR2_INST *pir2 = latxs_ir2_allocate();
    IR2_OPND imm_opnd = latxs_ir2_opnd_new(IR2_OPND_IMMH, imm);
    latxs_ir2_build1(pir2, opcode, &imm_opnd);

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    latxs_ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnda(IR2_OPCODE opcode,
        ADDR addr)
{
    IR2_INST *pir2 = latxs_ir2_allocate();
    IR2_OPND imm_opnd = latxs_ir2_opnd_new(IR2_OPND_IMMH, addr);

    switch (opcode) {
    case LISA_B:
    case LISA_BL:
        lsassert(int32_in_int26(addr));
        latxs_ir2_build1(pir2, opcode, &imm_opnd);
        break;
    case LISA_X86_INST:
    case LISA_DUP:
        latxs_ir2_build1(pir2, opcode, &imm_opnd);
        break;
    default:
        lsassertm(0, "append opnda unsupport LISA instruciton (%d)%s\n",
                     opcode, latxs_ir2_name(opcode));
        break;
    }

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    latxs_ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnd0(
        IR2_OPCODE opcode)
{
    IR2_INST *pir2 = latxs_ir2_allocate();
    latxs_ir2_build0(pir2, opcode);

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    latxs_ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnd2_(IR2_OPCODE opcode,
        IR2_OPND  *opnd0, IR2_OPND *opnd1)
{
    IR2_INST *pir2 = NULL;

    switch (opcode) {
    case lisa_mov:
        pir2 = latxs_append_ir2_opnd3(LISA_OR,
                opnd0, opnd1, &latxs_zero_ir2_opnd);
        break;
    case lisa_mov32z:
        pir2 = latxs_append_ir2_opnd2ii(LISA_BSTRPICK_D,
                opnd0, opnd1, 31, 0);
        break;
    case lisa_mov24z:
        pir2 = latxs_append_ir2_opnd2ii(LISA_BSTRPICK_D,
                opnd0, opnd1, 23, 0);
        break;
    case lisa_mov16z:
        pir2 = latxs_append_ir2_opnd2ii(LISA_BSTRPICK_D,
                opnd0, opnd1, 15, 0);
        break;
    case lisa_mov8z:
        pir2 = latxs_append_ir2_opnd2ii(LISA_BSTRPICK_D,
                opnd0, opnd1,  7, 0);
        break;
    case lisa_mov32s:
        pir2 = latxs_append_ir2_opnd2ii(LISA_BSTRPICK_W,
                opnd0, opnd1, 31, 0);
        break;
    case lisa_mov16s:
        pir2 = latxs_append_ir2_opnd2(LISA_EXT_W_H, opnd0, opnd1);
        break;
    case lisa_mov8s:
        pir2 = latxs_append_ir2_opnd2(LISA_EXT_W_B, opnd0, opnd1);
        break;
    case lisa_not:
        pir2 = latxs_append_ir2_opnd3(LISA_NOR,
                opnd0, opnd1, &latxs_zero_ir2_opnd);
        break;
    default:
        lsassertm(0, "append opnd2_ only for fake LISA.\n");
        break;
    }

    return pir2;
}

IR2_INST *latxs_append_ir2_opnd1_(IR2_OPCODE opcode,
        IR2_OPND  *opnd0)
{
    IR2_INST *pir2 = NULL;

    switch (opcode) {
    case lisa_call:
        pir2 = latxs_append_ir2_opnd2i(LISA_JIRL,
                &latxs_ra_ir2_opnd, opnd0, 0);
        break;
    case lisa_jr:
        pir2 = latxs_append_ir2_opnd2i(LISA_JIRL,
                &latxs_zero_ir2_opnd, opnd0, 0);
        break;
    default:
        lsassertm(0, "append opnd1_ only for fake LISA.\n");
        break;
    }

    return pir2;
}

IR2_INST *latxs_append_ir2_opnd0_(IR2_OPCODE opcode)
{
    IR2_INST *pir2 = NULL;

    switch (opcode) {
    case lisa_return:
        pir2 = latxs_append_ir2_opnd2i(LISA_JIRL,
                &latxs_zero_ir2_opnd,
                &latxs_ra_ir2_opnd, 0);
        break;
    case lisa_nop:
        pir2 = latxs_append_ir2_opnd2i(LISA_ANDI,
                &latxs_zero_ir2_opnd,
                &latxs_zero_ir2_opnd, 0);
        break;
    default:
        lsassertm(0, "append opnd0_ only for fake LISA.\n");
        break;
    }

    return pir2;
}

bool latxs_ir2_opcode_is_load(IR2_OPCODE opcode)
{
    return la_ir2_opcode_is_load(opcode);
}
bool latxs_ir2_opcode_is_store(IR2_OPCODE opcode)
{
    return la_ir2_opcode_is_store(opcode);
}
bool latxs_ir2_opcode_is_branch(IR2_OPCODE opcode)
{
    return (opcode >= LISA_BEQZ && opcode <= LISA_BNEZ) ||
           (opcode >= LISA_BCEQZ && opcode <= LISA_BCNEZ) ||
           (opcode >= LISA_B && opcode <= LISA_BGEU);
}
bool latxs_ir2_opcode_is_convert(IR2_OPCODE opcode)
{
    return ir2_opcode_is_convert(opcode);
}
bool latxs_ir2_opcode_is_fcmp(IR2_OPCODE opcode)
{
    return ir2_opcode_is_fcmp(opcode);
}

#endif
