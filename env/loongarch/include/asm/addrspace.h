#ifndef ADDRSPACE_H_JZKUY25P
#define ADDRSPACE_H_JZKUY25P

#include <asm/loongarchregs.h>

#define DMW_PABITS 48

#ifndef UNCAC_BASE
#define UNCAC_BASE CSR_DMW0_BASE
#endif

#ifndef CAC_BASE
#define CAC_BASE CSR_DMW1_BASE
#endif

#define TO_PHYS_MASK ((1ULL << DMW_PABITS) - 1)

#define TO_PHYS(x) (((x)&TO_PHYS_MASK))
#define TO_CAC(x) (CAC_BASE | ((x)&TO_PHYS_MASK))
#define TO_UNCAC(x) (UNCAC_BASE | ((x)&TO_PHYS_MASK))

/*
 *  Configure language
 */
#ifdef __ASSEMBLY__
#define _ATYPE_
#define _ATYPE32_
#define _ATYPE64_
#define _CONST64_(x) x
#else
#define _ATYPE_ __PTRDIFF_TYPE__
#define _ATYPE32_ int
#define _ATYPE64_ __s64
#ifdef CONFIG_64BIT
#define _CONST64_(x) x##L
#else
#define _CONST64_(x) x##LL
#endif
#endif

#endif /* end of include guard: ADDRSPACE_H_JZKUY25P */
