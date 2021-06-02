#ifndef ADDRSPACE_H_JZKUY25P
#define ADDRSPACE_H_JZKUY25P

#include "./loongarchregs.h"

#ifndef UNCAC_BASE
#define UNCAC_BASE		CSR_DMW0_BASE
#endif

#ifndef CAC_BASE
#define CAC_BASE		CSR_DMW1_BASE
#endif

#define TO_PHYS_MASK	((1ULL << DMW_PABITS) - 1)

#define TO_PHYS(x)		(((x) & TO_PHYS_MASK))
#define TO_CAC(x)		(CAC_BASE   | ((x) & TO_PHYS_MASK))
#define TO_UNCAC(x)		(UNCAC_BASE | ((x) & TO_PHYS_MASK))
#endif /* end of include guard: ADDRSPACE_H_JZKUY25P */
