/* Copyright 2016 Loongson Technology Corporation Limited  */

/* Author: songyuekun songyuekun@loongson.cn */

/*
 * ISA: MIPS64R2
 * ABI: N64
 */

/* basic algorithm :

	+.	let t0, t1 point to a0, a1, if a0 has smaller low 3 bit of a0 and a1,
		set a4 to 1 and let t0 point to the larger of lower 3bit of a0 and a1

	+.	if low 3 bit of a0 equal low 3 bit of a0, use a ldr one time and more ld other times;

	+.	if not,  load partial t2 and t3, check if t2 has \0;

	+.	then use use ld for t0, ldr for t1,

	+.	if partial 8 byte  from t1 has \0, compare partial 8 byte from t1 with 8
		byte from t0 with a mask in a7

	+.	if not, ldl other part of t1, compare  8 byte from t1 with 8 byte from t0

	+.	if (v0 - 0x0101010101010101) & (~v0) & 0x8080808080808080 != 0, v0 has
		one byte is \0, else has no \0

	+.	for partial 8 byte from ldr t3, 0(a0), preload t3 with 0xffffffffffffffff


*/
#include "asm.h"


#define STRCMP	strcmp

#define REP8_01 0x0101010101010101
#define REP8_7f 0x7f7f7f7f7f7f7f7f
#define REP8_80 0x8080808080808080

/* Parameters and Results */
#define src1	a0
#define	src2	a1
#define result	v0
// Note: v0 = a0 in N64 ABI


/* Internal variable */
#define data1		t0
#define	data2		t1
#define	has_nul		t2
#define	diff		t3
#define syndrome	t4
#define zeroones	t5
#define	sevenf		t6
#define pos		t7
#define exchange	t8
#define tmp1		a4
#define	tmp2		a5
#define	tmp3		a6
#define src1_off    a2
#define src2_off    a3
#define tmp4        a7

/* rd <- if rc then ra else rb
    will destroy tmp3
*/
#define CONDITIONSEL(rd,rc,ra,rb)\
        masknez tmp3, rb, rc;\
        maskeqz rd,   ra, rc;\
        or      rd,   rd, tmp3



/* int strcmp (const char *s1, const char *s2); */

LEAF(STRCMP)
	.align		4

	xor		tmp1, src1, src2
    lu12i.w     zeroones, 0x01010
    lu12i.w     sevenf, 0x7f7f7
    andi        src1_off, src1, 0x7
    ori         zeroones, zeroones, 0x101
    ori         sevenf, sevenf, 0xf7f
	andi		tmp1, tmp1, 0x7
    bstrins.d   zeroones, zeroones, 63, 32
    bstrins.d   sevenf, sevenf, 63, 32
	bnez		tmp1, strcmp_misaligned8
	bnez		src1_off, strcmp_mutual_align
strcmp_loop_aligned:
	ld.d		data1, src1, 0
    addi.d      src1, src1, 8
	ld.d		data2, src2, 0
    addi.d      src2, src2, 8
strcmp_start_realigned:
	sub.d		tmp1, data1, zeroones
	or		tmp2, data1, sevenf
	xor		diff, data1, data2
	andn		has_nul, tmp1, tmp2
	or		syndrome, diff, has_nul
	beqz		syndrome, strcmp_loop_aligned

strcmp_end:
	ctz.d		pos, syndrome
    bstrins.d   pos, zero, 2, 0
	srl.d		data1, data1, pos
	srl.d		data2, data2, pos
	andi		data1, data1, 0xff
	andi		data2, data2, 0xff
	sub.d		result, data1, data2
	jr ra
strcmp_mutual_align:
    bstrins.d   src1, zero, 2, 0
    bstrins.d   src2, zero, 2, 0
	slli.d		tmp1, src1_off,  0x3
	ld.d		data1, src1, 0
	sub.d		tmp1, zero, tmp1
	ld.d		data2, src2, 0
    addi.d      src1, src1, 8
    addi.d      src2, src2, 8
	nor		tmp2, zero, zero
	srl.d		tmp2, tmp2, tmp1
	or		data1, data1, tmp2
	or		data2, data2, tmp2
	b		strcmp_start_realigned

strcmp_misaligned8:

/* check
    if ((src1 != 0) && ((src2 == 0 ) || (src1 < src2)))
    then exchange(src1,src2)

*/
    andi        src2_off, src2, 0x7
    slt         tmp2, src1_off, src2_off
    CONDITIONSEL(tmp2,src2_off,tmp2,tmp1)
    maskeqz     exchange, tmp2, src1_off
    xor         tmp3, src1, src2
    maskeqz     tmp3, tmp3, exchange
    xor         src1, src1, tmp3
    xor         src2, src2, tmp3

	andi		src1_off, src1, 0x7
	beqz		src1_off, strcmp_loop_misaligned
strcmp_do_misaligned:
	ld.bu		data1, src1, 0
	ld.bu		data2, src2, 0
    xor         tmp3, data1, data2
	addi.d		src1, src1, 1
    masknez     tmp3, data1, tmp3
	addi.d		src2, src2, 1
    beqz        tmp3, strcmp_done
	andi		src1_off, src1, 0x7
	bnez		src1_off, strcmp_do_misaligned

strcmp_loop_misaligned:
	andi		tmp1, src2, 0xff8
	xori		tmp1, tmp1, 0xff8
	beqz		tmp1, strcmp_do_misaligned
	ld.d		data1, src1, 0
	ld.d		data2, src2, 0
	addi.d		src1, src1, 8
	addi.d		src2, src2, 8

	sub.d		tmp1, data1, zeroones
	or		tmp2, data1, sevenf
	xor		diff, data1, data2
	andn		has_nul, tmp1, tmp2
	or		syndrome, diff, has_nul
	beqz		syndrome, strcmp_loop_misaligned
//	b		strcmp_end
strcmp_misalign_end:
	ctz.d		pos, syndrome
    bstrins.d   pos, zero, 2, 0
	srl.d		data1, data1, pos
	srl.d		data2, data2, pos
	andi		data1, data1, 0xff
	andi		data2, data2, 0xff
	sub.d		tmp1, data1, data2
	sub.d		tmp2, data2, data1
    CONDITIONSEL(result,exchange,tmp2,tmp1)
	jr ra

strcmp_done:
	sub.d	    tmp1, data1, data2
	sub.d		tmp2, data2, data1
    CONDITIONSEL(result,exchange,tmp2,tmp1)
	jr	ra
END(STRCMP)
