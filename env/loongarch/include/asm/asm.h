#ifndef ASM_H_LOAQNS21
#define ASM_H_LOAQNS21

#include <autoconf.h>

#if (_LOONGARCH_SZLONG == 64)
#define LONG_ADDU add.d
#define LONG_ADDIU addi.d
#define LONG_SUBU sub.d
#define LONG_L ld.d
#define LONG_S st.d
#define LONG_SP sdp
#define LONG_SLL slli.d
#define LONG_SLLV sll.d
#define LONG_SRL srli.d
#define LONG_SRLV srl.d
#define LONG_SRA sra.w
#define LONG_SRAV sra.d

#define LONG .dword
#define LONGSIZE 8
#define LONGMASK 7
#define LONGLOG 3
#endif

#if (_LOONGARCH_SZPTR == 64)
#define PTR_ADDU add.d
#define PTR_ADDIU addi.d
#define PTR_SUBU sub.d
#define PTR_L ld.d
#define PTR_S st.d
#define PTR_LI li.d
#define PTR_SLL slli.d
#define PTR_SLLV sll.d
#define PTR_SRL srli.d
#define PTR_SRLV srl.d
#define PTR_SRA srai.d
#define PTR_SRAV sra.d

#define PTR_SCALESHIFT 3

#define PTR .dword
#define PTRSIZE 8
#define PTRLOG 3
#endif
#endif /* end of include guard: ASM_H_LOAQNS21 */
