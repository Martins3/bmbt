/* Copyright 2016 Loongson Technology Corporation Limited  */

/* Author: Huang Pei huangpei@loongson.cn */

/*
 * ISA: MIPS64R2
 * ABI: N64
 */

/* basic algorithm :

    +.  if src aligned. just do the copy loop. if not, do the cross page check and copy one double word.

        Then move src to aligned.

	+.	if (v0 - 0x0101010101010101) & (~v0) & 0x8080808080808080 != 0, v0 has
		one byte is \0, else has no \0


*/


#include "asm.h"


#define STRCPY	strcpy


#define REP8_01 0x0101010101010101
#define REP8_7f 0x7f7f7f7f7f7f7f7f
#define REP8_80 0x8080808080808080

/* Parameters and Results */
#define dest	a0
#define	src	a1
#define result	v0
// Note: v0 = a0 in N64 ABI


/* Internal variable */
#define data		t0
#define	data1		t1
#define	has_nul		t2
#define	diff		t3
#define syndrome	t4
#define zeroones	t5
#define	sevenf		t6
#define pos		t7
#define dest_backup	t8
#define tmp1		a4
#define	tmp2		a5
#define	tmp3		a6
#define dest_off    a2
#define src_off     a3
#define tmp4        a7

/* rd <- if rc then ra else rb
    will destroy tmp3
*/
#define CONDITIONSEL(rd,rc,ra,rb)\
        masknez tmp3, rb, rc;\
        maskeqz rd,   ra, rc;\
        or      rd,   rd, tmp3



/* int strcpy (const char *s1, const char *s2); */

LEAF(STRCPY)
	.align		4

    move        dest_backup, dest
    lu12i.w     zeroones, 0x01010
    lu12i.w     sevenf, 0x7f7f7
    ori         zeroones, zeroones, 0x101
    ori         sevenf, sevenf, 0xf7f
    bstrins.d   zeroones, zeroones, 63, 32
    bstrins.d   sevenf, sevenf, 63, 32
    andi        src_off, src, 0x7
	beqz		src_off, strcpy_loop_aligned_1
    b           strcpy_mutual_align
strcpy_loop_aligned:
    st.d        data, dest, 0
    addi.d      dest, dest, 8
strcpy_loop_aligned_1:
	ld.d		data, src, 0
    addi.d      src, src, 8
strcpy_start_realigned:
	sub.d		tmp1, data, zeroones
	or		    tmp2, data, sevenf
	andn		has_nul, tmp1, tmp2
	beqz		has_nul, strcpy_loop_aligned

strcpy_end:

/*
8 4 2 1
*/
	ctz.d		pos, has_nul
	srli.d		pos, pos, 3
    addi.d      pos, pos, 1
/*
    Do 8/4/2/1 strcpy based on pos value.
    pos value is the number of bytes to be copied
    the bytes include the final \0 so the max length is 8 and the min length is 1
*/

strcpy_end_8:
    andi        tmp1, pos, 0x8
    beqz        tmp1, strcpy_end_4
    st.d        data, dest, 0
    move        dest, dest_backup
    jr  ra
strcpy_end_4:
    andi        tmp1, pos, 0x4
    beqz        tmp1, strcpy_end_2
    st.w        data, dest, 0
    srli.d      data, data, 32
    addi.d      dest, dest, 4
strcpy_end_2:
    andi        tmp1, pos, 0x2
    beqz        tmp1, strcpy_end_1
    st.h        data, dest, 0
    srli.d      data, data, 16
    addi.d      dest, dest, 2
strcpy_end_1:
    andi        tmp1, pos, 0x1
    beqz        tmp1, strcpy_end_ret
    st.b        data, dest, 0
strcpy_end_ret:
    move        result, dest_backup
    jr  ra


strcpy_mutual_align:
/*
    Check if around src page bound.
    if not go to page cross ok.
    if it is, do further check.
    use tmp2 to accelerate.
*/

    li.w          tmp2, 0xff8
    andi        tmp1, src,  0xff8
    beq         tmp1, tmp2, strcpy_page_cross

strcpy_page_cross_ok:
/*
    Load a misaligned double word and check if has \0
    If no, do a misaligned double word paste.
    If yes, calculate the number of avaliable bytes,
    then jump to 4/2/1 end.
*/
    ld.d        data, src, 0
	sub.d		tmp1, data, zeroones
	or		    tmp2, data, sevenf
	andn		has_nul, tmp1, tmp2
    bnez        has_nul, strcpy_end
strcpy_mutual_align_finish:
/*
    Before jump back to align loop, make dest/src aligned.
    This will cause a duplicated paste for several bytes between the first double word and the second double word,
    but should not bring a problem.
*/
    li.w          tmp1, 8
    st.d        data, dest, 0
    sub.d       tmp1, tmp1, src_off
    add.d       src,  src,  tmp1
    add.d       dest, dest, tmp1

	b		strcpy_loop_aligned_1

strcpy_page_cross:
/*
    ld.d from aligned address(src & ~0x7).
    check if high bytes have \0.
    it not, go back to page cross ok,
    since the string is supposed to cross the page bound in such situation.
    if it is, do a srl for data to make it seems like a direct double word from src,
    then go to 4/2/1 strcpy end.

    tmp4 is 0xffff...ffff mask
    tmp2 demonstrate the bytes to be masked
    tmp2 = src_off << 3
    data = data >> (src_off * 8) | -1 << (64 - src_off * 8)
    and
    -1 << (64 - src_off * 8) ->  ~(-1 >> (src_off * 8))

*/
    li.w          tmp1, 0x7
    andn        tmp3, src,  tmp1
    ld.d        data, tmp3, 0
    li.w          tmp4, -1
    slli.d      tmp2, src_off, 3
    srl.d       tmp4, tmp4, tmp2
    srl.d       data, data, tmp2
    nor         tmp4, tmp4, zero
    or          data, data, tmp4
    sub.d		tmp1, data, zeroones
	or		    tmp2, data, sevenf
	andn		has_nul, tmp1, tmp2
	beqz		has_nul, strcpy_page_cross_ok
    b           strcpy_end
END(STRCPY)
