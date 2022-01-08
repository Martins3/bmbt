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


#define STRNCMP	strncmp

#define REP8_01 0x0101010101010101
#define REP8_7f 0x7f7f7f7f7f7f7f7f
#define REP8_80 0x8080808080808080

/* Parameters and Results */
#define src1	a0
#define	src2	a1
#define	limit	a2
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
#define tmp1		a5
#define	tmp2		a6
#define	tmp3		a7
#define src1_off    a3
#define limit_wd    a4


/* int strncmp (const char *s1, const char *s2); */

LEAF(STRNCMP)
	.align		4
    beqz        limit, strncmp_ret0

	xor		tmp1, src1, src2
    lu12i.w     zeroones, 0x01010
    lu12i.w     sevenf, 0x7f7f7
    andi        src1_off, src1, 0x7
    ori         zeroones, zeroones, 0x101
	andi		tmp1, tmp1, 0x7
    ori         sevenf, sevenf, 0xf7f
    bstrins.d   zeroones, zeroones, 63, 32
    bstrins.d   sevenf, sevenf, 63, 32
	bnez		tmp1, strncmp_misaligned8
	bnez		src1_off, strncmp_mutual_align
    /* */
    addi.d      limit_wd, limit, -1
    srli.d      limit_wd, limit_wd, 3

strncmp_loop_aligned:
	ld.d		data1, src1, 0
    addi.d      src1, src1, 8
	ld.d		data2, src2, 0
    addi.d      src2, src2, 8
strncmp_start_realigned:
    addi.d      limit_wd, limit_wd, -1
	sub.d		tmp1, data1, zeroones
	or		    tmp2, data1, sevenf
	xor	    	diff, data1, data2
	andn		has_nul, tmp1, tmp2
    srli.d      tmp1, limit_wd, 63
	or		    syndrome, diff, has_nul
    or          tmp2, syndrome, tmp1
	beqz		tmp2, strncmp_loop_aligned

    /* if not reach limit */
    bge         limit_wd, zero, strncmp_not_limit
    /* if reach limit */
    andi        limit, limit, 0x7
    li.w          tmp1, 0x8
    sub.d       limit, tmp1, limit
    slli.d      limit, limit, 0x3
    li.d        tmp1, -1
    srl.d       tmp1, tmp1, limit
    and         data1, data1, tmp1
    and         data2, data2, tmp1
    orn         syndrome, syndrome, tmp1


strncmp_not_limit:
	ctz.d		pos, syndrome
    bstrins.d   pos, zero, 2, 0
	srl.d		data1, data1, pos
	srl.d		data2, data2, pos
	andi		data1, data1, 0xff
	andi		data2, data2, 0xff
	sub.d		result, data1, data2
	jr ra



strncmp_mutual_align:
    bstrins.d   src1, zero, 2, 0
    bstrins.d   src2, zero, 2, 0
	slli.d		tmp1, src1_off,  0x3
	ld.d		data1, src1, 0
	ld.d		data2, src2, 0
    addi.d      src2, src2, 8
    addi.d      src1, src1, 8

    addi.d      limit_wd, limit, -1
    andi        tmp3, limit_wd, 0x7
    srli.d      limit_wd, limit_wd, 3
    add.d       limit, limit, src1_off
    add.d       tmp3, tmp3, src1_off
    srli.d      tmp3, tmp3, 3
    add.d       limit_wd, limit_wd, tmp3

	sub.d		tmp1, zero, tmp1
	nor		tmp2, zero, zero
	srl.d		tmp2, tmp2, tmp1
	or		data1, data1, tmp2
	or		data2, data2, tmp2
	b		strncmp_start_realigned

strncmp_misaligned8:

    li.w          tmp1, 0x10
    bge         limit, tmp1, strncmp_try_words
strncmp_byte_loop:
    ld.bu       data1, src1, 0
    ld.bu       data2, src2, 0
    addi.d      limit, limit, -1
    xor         tmp1, data1, data2
    masknez     tmp1, data1, tmp1
    maskeqz     tmp1, limit, tmp1
    beqz        tmp1, strncmp_done

    ld.bu       data1, src1, 1
    ld.bu       data2, src2, 1
    addi.d      src1, src1, 2
    addi.d      src2, src2, 2
    addi.d      limit, limit, -1
    xor         tmp1, data1, data2
    masknez     tmp1, data1, tmp1
    maskeqz     tmp1, limit, tmp1
    bnez        tmp1, strncmp_byte_loop


strncmp_done:
    sub.d       result, data1, data2
    jr ra

strncmp_try_words:
    srli.d      limit_wd, limit, 3
    beqz        src1_off, strncmp_do_misaligned

    sub.d       src1_off, zero, src1_off
    andi        src1_off, src1_off, 0x7
    sub.d       limit, limit, src1_off
    srli.d      limit_wd, limit, 0x3


strncmp_page_end_loop:
    ld.bu       data1, src1, 0
    ld.bu       data2, src2, 0
    addi.d      src1, src1, 1
    addi.d      src2, src2, 1
    xor         tmp1, data1, data2
    masknez     tmp1, data1, tmp1
    beqz        tmp1, strncmp_done
    andi        tmp1, src1, 0x7
    bnez        tmp1, strncmp_page_end_loop
strncmp_do_misaligned:
    li.w          src1_off, 0x8
    addi.d      limit_wd, limit_wd, -1
    blt         limit_wd, zero, strncmp_done_loop

strncmp_loop_misaligned:
    andi        tmp2, src2, 0xff8
    xori        tmp2, tmp2, 0xff8
    beqz        tmp2, strncmp_page_end_loop

    ld.d        data1, src1, 0
    ld.d        data2, src2, 0
    addi.d      src1, src1, 8
    addi.d      src2, src2, 8
    sub.d       tmp1, data1, zeroones
    or          tmp2, data1, sevenf
    xor         diff, data1, data2
    andn        has_nul, tmp1, tmp2
    or          syndrome, diff, has_nul
    bnez        syndrome, strncmp_not_limit
    addi.d      limit_wd, limit_wd, -1
    #blt         zero, limit_wd, strncmp_loop_misaligned
    bge         limit_wd, zero, strncmp_loop_misaligned

strncmp_done_loop:
    andi        limit, limit, 0x7
    beqz        limit, strncmp_not_limit
    /* Read the last double word */
    /* check if the final part is about to exceed the page */
    andi        tmp1, src2, 0x7
    andi        tmp2, src2, 0xff8
    add.d       tmp1, tmp1, limit
    xori        tmp2, tmp2, 0xff8
    andi        tmp1, tmp1, 0x8
    masknez     tmp1, tmp1, tmp2
    bnez        tmp1, strncmp_byte_loop
    addi.d      src1, src1, -8
    addi.d      src2, src2, -8
    ldx.d       data1, src1, limit
    ldx.d       data2, src2, limit
    sub.d       tmp1, data1, zeroones
    or          tmp2, data1, sevenf
    xor         diff, data1, data2
    andn        has_nul, tmp1, tmp2
    or          syndrome, diff, has_nul
    bnez        syndrome, strncmp_not_limit

strncmp_ret0:
    move result, zero
    jr ra
/* check
    if ((src1 != 0) && ((src2 == 0 ) || (src1 < src2)))
    then exchange(src1,src2)

*/
END(STRNCMP)
