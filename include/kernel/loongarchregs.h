#ifndef LOONGARCHREGS_H_TJ8URLGY
#define LOONGARCHREGS_H_TJ8URLGY
#include "config.h"
#include "types.h"
/* Loongson csr registers */

/* Basic CSR registers */
#define LOONGARCH_CSR_CRMD 0x0 /* Current mode info */
#define CSR_CRMD_DACM_SHIFT 7
#define CSR_CRMD_DACM_WIDTH 2
#define CSR_CRMD_DACM (_ULCAST_(0x3) << CSR_CRMD_DACM_SHIFT)
#define CSR_CRMD_DACF_SHIFT 5
#define CSR_CRMD_DACF_WIDTH 2
#define CSR_CRMD_DACF (_ULCAST_(0x3) << CSR_CRMD_DACF_SHIFT)
#define CSR_CRMD_PG_SHIFT 4
#define CSR_CRMD_PG (_ULCAST_(0x1) << CSR_CRMD_PG_SHIFT)
#define CSR_CRMD_DA_SHIFT 3
#define CSR_CRMD_DA (_ULCAST_(0x1) << CSR_CRMD_DA_SHIFT)
#define CSR_CRMD_IE_SHIFT 2
#define CSR_CRMD_IE (_ULCAST_(0x1) << CSR_CRMD_IE_SHIFT)
#define CSR_CRMD_PLV_SHIFT 0
#define CSR_CRMD_PLV_WIDTH 2
#define CSR_CRMD_PLV (_ULCAST_(0x3) << CSR_CRMD_PLV_SHIFT)

/* Direct map windows registers */
#define LOONGARCH_CSR_DMWIN0 0x180 /* 64 direct map win0: MEM & IF */
#define LOONGARCH_CSR_DMWIN1 0x181 /* 64 direct map win1: MEM & IF */
#define LOONGARCH_CSR_DMWIN2 0x182 /* 64 direct map win2: MEM */
#define LOONGARCH_CSR_DMWIN3 0x183 /* 64 direct map win3: MEM */

/* Direct map window 0/1 */
#define CSR_DMW0_PLV0 _CONST64_(1 << 0)
#define CSR_DMW0_VSEG _CONST64_(0x8000)
#define CSR_DMW0_BASE (CSR_DMW0_VSEG << DMW_PABITS)
#define CSR_DMW0_INIT (CSR_DMW0_BASE | CSR_DMW0_PLV0)

#define CSR_DMW1_PLV0 _CONST64_(1 << 0)
#define CSR_DMW1_MAT _CONST64_(1 << 4)
#define CSR_DMW1_VSEG _CONST64_(0x9000)
#define CSR_DMW1_BASE (CSR_DMW1_VSEG << DMW_PABITS)
#define CSR_DMW1_INIT (CSR_DMW1_BASE | CSR_DMW1_MAT | CSR_DMW1_PLV0)

#endif /* end of include guard: LOONGARCHREGS_H_TJ8URLGY */
