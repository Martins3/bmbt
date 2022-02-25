#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>


static CPUTLBEntry *__get_cpu_tlb(CPUX86State *env, target_ulong ptr)
{
    int mmu_idx = cpu_mmu_index(env, false);
    return tlb_entry(env, mmu_idx, ptr);
}

ADDR latxs_tr_cvt_cpu_addr_read(CPUX86State *env,
        target_ulong addr, int *error)
{
    CPUTLBEntry *tlb = __get_cpu_tlb(env, addr);

    if (!tlb || tlb->addr_read != (addr & TARGET_PAGE_MASK)) {
        *error = 1;
        return 0;
    }

    *error = 0;
    return addr + tlb->addend;
}

ADDR latxs_tr_cvt_cpu_addr_write(CPUX86State *env,
        target_ulong addr, int *error)
{
    CPUTLBEntry *tlb = __get_cpu_tlb(env, addr);

    if (!tlb || tlb->addr_write != (addr & TARGET_PAGE_MASK)) {
        *error = 1;
        return 0;
    }

    *error = 0;
    return addr + tlb->addend;
}

ADDR latxs_tr_cvt_cpu_addr_code(CPUX86State *env,
        target_ulong addr, int *error)
{
    CPUTLBEntry *tlb = __get_cpu_tlb(env, addr);

    if (!tlb || tlb->addr_code != (addr & TARGET_PAGE_MASK)) {
        *error = 1;
        return 0;
    }

    *error = 0;
    return addr + tlb->addend;
}
