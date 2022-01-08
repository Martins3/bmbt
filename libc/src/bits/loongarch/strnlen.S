/* Copyright 2016 Loongson Technology Corporation Limited  */

/* Author: Songyuekun songyuekun@loongson.cn */

/*
 * ISA: MIPS64R2
 * ABI: N64
 */
/*
algorithm:

	#.	use ld/ldr to access word/partial word in the string

	#.	use (x - 0x0101010101010101) & (~(x | 0x7f7f7f7f7f7f7f7f) != 0 to
	judge if x has zero byte

	#.	use dctz((x - 0x0101010101010101) & (~(x | 0x7f7f7f7f7f7f7f7f) >> 3
		to get the index of first rightmost zero byte in dword x;

	#.	use dctz(x) = 64 - dclz(~x & (x-1));

	#.	use pointer to the last non zero byte  minus pointer to the start
	of the string to get the length of string

*/

#include "../../include/features.h"
#include "asm.h"

#define L_ADDIU  addi.d
#define L_ADDU   add.d
#define L_SUBU   sub.d

#define STRNLEN	strnlen
#define L(x)	x
/* rd <- if rc then ra else rb
    will destroy t6
*/

#define CONDITIONSEL(rd,ra,rb,rc)\
        masknez a5, rb, rc;\
        maskeqz rd, ra, rc;\
        or      rd, rd, a5


/* Parameters and Results */
#define srcin	a0
#define	limit   a1
#define len 	v0


/* Internal variable */
#define data1		t0
#define	data2		t1
#define	has_nul1	t2
#define	has_nul2	t3
#define src	        t4
#define zeroones	t5
#define	sevenf		t6
#define data2a	    t7
#define tmp6	    t7
#define pos	        t8
#define tmp1		a2
#define	tmp2		a3
#define	tmp3		a4
#define tmp4        a5
#define tmp5        a6
#define limit_wd    a7



/* size_t strnlen (const char *s1,size_t maxlen); */

LEAF(STRNLEN)

	.align		4
    beqz        limit, L(_hit_limit)
    lu12i.w     zeroones, 0x01010
    lu12i.w     sevenf, 0x7f7f7
    ori         zeroones, zeroones, 0x101
    ori         sevenf, sevenf, 0xf7f
    bstrins.d   zeroones, zeroones, 63, 32
    bstrins.d   sevenf, sevenf, 63, 32
    andi        tmp1, srcin, 15
    sub.d       src, srcin, tmp1
    bnez        tmp1, L(misaligned)
    addi.d      limit_wd, limit, -1
    srli.d      limit_wd, limit_wd, 4
L(_loop):
    ld.d        data1, src, 0
    ld.d        data2, src, 8
    addi.d      src, src, 16
L(_realigned):
    sub.d       tmp1, data1, zeroones
    or          tmp2, data1, sevenf
    sub.d       tmp3, data2, zeroones
    or          tmp4, data2, sevenf
    andn        has_nul1, tmp1, tmp2
    andn        has_nul2, tmp3, tmp4
    addi.d      limit_wd, limit_wd, -1
    srli.d      tmp1, limit_wd, 63
    or          tmp2, has_nul1, has_nul2
    or          tmp3, tmp1, tmp2
    beqz        tmp3, L(_loop)
    beqz        tmp2, L(_hit_limit)
    sub.d       len, src, srcin
    beqz        has_nul1, L(_nul_in_data2)
    move        has_nul2, has_nul1
    addi.d      len, len, -8
L(_nul_in_data2):
    ctz.d       pos, has_nul2
    srli.d      pos, pos, 3
    addi.d      len, len, -8
    add.d       len, len, pos
    sltu        tmp1, len, limit
    CONDITIONSEL(len,len,limit,tmp1)
    jr ra


L(misaligned):
    addi.d      limit_wd, limit, -1
    sub.d       tmp4, zero, tmp1
    andi        tmp3, limit_wd, 15
    srli.d      limit_wd, limit_wd, 4
    li.d        tmp5, -1
    ld.d        data1, src, 0
    ld.d        data2, src, 8
    addi.d      src, src, 16
    slli.d      tmp4, tmp4, 3
    add.d       tmp3, tmp3, tmp1
    srl.d       tmp2, tmp5, tmp4
    srli.d      tmp3, tmp3, 4
    add.d       limit_wd, limit_wd, tmp3
    or          data1, data1, tmp2
    or          data2a, data2, tmp2
    li.w          tmp3, 9
    sltu        tmp1, tmp1, tmp3
    CONDITIONSEL(data1,data1,tmp5,tmp1)
    CONDITIONSEL(data2,data2,data2a,tmp1)
    b           L(_realigned)


L(_hit_limit):
    move len, limit
    jr  ra
END(STRNLEN)
