#include "asm.h"

#ifdef LOONGSON_TEST
#define MEMSET	_memset
#else
#define MEMSET	memset
#endif

#define ST_128(n) 	\
	st.d	a1, a0, n;		 \
	st.d    a1, a0, n+8  ; 	 \
	st.d    a1, a0, n+16 ;   \
	st.d    a1, a0, n+24 ;   \
	st.d    a1, a0, n+32 ;   \
	st.d    a1, a0, n+40 ;   \
	st.d    a1, a0, n+48 ;   \
	st.d    a1, a0, n+56 ;   \
	st.d    a1, a0, n+64 ;   \
	st.d    a1, a0, n+72 ;   \
	st.d    a1, a0, n+80 ;   \
	st.d    a1, a0, n+88 ;   \
	st.d    a1, a0, n+96 ;   \
	st.d    a1, a0, n+104;   \
	st.d    a1, a0, n+112;   \
	st.d    a1, a0, n+120;	 \

//1st var: void *str  $4 a0
//2nd var: int val  $5   a1
//3rd var: size_t num  $6  a2

LEAF(MEMSET)

memset:
	.align	6

	bstrins.d a1, a1, 15, 8
	add.d	  t7, a0, a2
	bstrins.d a1, a1, 31, 16
	move	  t0, a0
	bstrins.d a1, a1, 63, 32
	srai.d	  t8, a2, 4         	#num/16
	beqz	  t8, less_16bytes	#num<16
	srai.d	  t8, a2, 6		#num/64
	bnez	  t8, more_64bytes	#num>64
	srai.d	  t8, a2, 5		#num/32
	beqz	  t8, less_32bytes	#num<32
	st.d	  a1, a0, 0 		#32<num<64
	st.d	  a1, a0, 8
	st.d	  a1, a0, 16
	st.d	  a1, a0, 24
	st.d	  a1, t7, -32
	st.d	  a1, t7, -24
	st.d	  a1, t7, -16
	st.d	  a1, t7, -8

	jr	  ra

less_32bytes:
	st.d	  a1, a0, 0
	st.d	  a1, a0, 8
	st.d	  a1, t7, -16
	st.d	  a1, t7, -8

	jr	  ra

less_16bytes:
	srai.d	  t8, a2, 3		#num/8
	beqz	  t8, less_8bytes
	st.d	  a1, a0, 0
	st.d	  a1, t7, -8

	jr	  ra

less_8bytes:
	srai.d	  t8, a2, 2
	beqz	  t8, less_4bytes
	st.w	  a1, a0, 0
	st.w	  a1, t7, -4

	jr	  ra

less_4bytes:
	srai.d	  t8, a2, 1
	beqz	  t8, less_2bytes
	st.h	  a1, a0, 0
	st.h	  a1, t7, -2

	jr	  ra

less_2bytes:
	beqz	  a2, less_1bytes
	st.b	  a1, a0, 0

	jr	  ra

less_1bytes:
	jr	  ra

more_64bytes:
	srli.d	  a0, a0, 3
	slli.d	  a0, a0, 3
	addi.d	  a0, a0, 0x8
	st.d      a1, t0, 0
	sub.d	  t2, t0, a0
	add.d	  a2, t2, a2

	addi.d	  a2, a2, -0x80
	blt       a2, zero, end_unalign_proc

loop_less:
	ST_128(0)
	addi.d	a0, a0,  0x80
	addi.d	a2, a2, -0x80
	bge     a2, zero, loop_less

end_unalign_proc:
	addi.d  a2, a2, 0x80

	pcaddi  t1, 20
	andi    t5, a2, 0x78
	srli.d  t5, t5, 1
	sub.d   t1, t1, t5
	jirl    zero, t1, 0

end_120_128_unalign:
	st.d    a1, a0, 112
end_112_120_unalign:
	st.d    a1, a0, 104
end_104_112_unalign:
	st.d    a1, a0, 96
end_96_104_unalign:
	st.d    a1, a0, 88
end_88_96_unalign:
	st.d    a1, a0, 80
end_80_88_unalign:
	st.d    a1, a0, 72
end_72_80_unalign:
	st.d    a1, a0, 64
end_64_72_unalign:
	st.d    a1, a0, 56
end_56_64_unalign:
	st.d    a1, a0, 48
end_48_56_unalign:
	st.d    a1, a0, 40
end_40_48_unalign:
	st.d    a1, a0, 32
end_32_40_unalign:
	st.d    a1, a0, 24
end_24_32_unalign:
    st.d    a1, a0, 16
end_16_24_unalign:
    st.d    a1, a0, 8
end_8_16_unalign:
    st.d    a1, a0, 0
end_0_8_unalign:

	st.d    a1, t7, -8

	move	  v0, t0
	jr	  ra

END(MEMSET)
