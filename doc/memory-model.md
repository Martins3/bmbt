# memory model

在 https://github.com/kernelrookie/DuckBuBi/issues/35 中，
分析 `address_space_*` 以及如何检查 memory_ldst.inc.c 和
memory_ldst.inc.h 的方法。

## [ ] x86_stl_phys_notdirty
- [ ] notdirty 意味着什么 ?



## memory_ldst 的分析
`#include "exec/memory_ldst.inc.h"` defined four times

```c
// cpu-all.h
#define SUFFIX
#define ARG1         as
#define ARG1_DECL    AddressSpace *as
#define TARGET_ENDIANNESS
#include "exec/memory_ldst.inc.h"

#define SUFFIX       _cached_slow
#define ARG1         cache
#define ARG1_DECL    MemoryRegionCache *cache
#define TARGET_ENDIANNESS
#include "exec/memory_ldst.inc.h"
```

```c
// memory.h
#define SUFFIX
#define ARG1         as
#define ARG1_DECL    AddressSpace *as
#include "exec/memory_ldst.inc.h"

#define SUFFIX       _cached_slow
#define ARG1         cache
#define ARG1_DECL    MemoryRegionCache *cache
#include "exec/memory_ldst.inc.h"
```
but `memory_ldst.inc.c` only two times, both of them defined in exec.c
```c
#define ARG1_DECL                AddressSpace *as
#define ARG1                     as
#define SUFFIX
#define TRANSLATE(...)           address_space_translate(as, __VA_ARGS__)
#define RCU_READ_LOCK(...)       rcu_read_lock()
#define RCU_READ_UNLOCK(...)     rcu_read_unlock() #include "memory_ldst.inc.c"
```
```c
#define ARG1_DECL                MemoryRegionCache *cache
#define ARG1                     cache
#define SUFFIX                   _cached_slow
#define TRANSLATE(...)           address_space_translate_cached(cache, __VA_ARGS__)
#define RCU_READ_LOCK()          ((void)0)
#define RCU_READ_UNLOCK()        ((void)0)
#include "memory_ldst.inc.c"
```
memory_ldst.inc.h 已经被简化到 cpu-all.h 中间了，memory_ldst.inc.c 已经被简化为 memory_ldst.c 了
因为 cache_slow 版本(只有 virtio 在使用)，也不需要 endianness 版本(主要是设备在使用)

## code flow: from helper to memory model

- in helper.c, a lot of functions similar functions are defined.
```c
uint8_t x86_ldub_phys(CPUState *cs, hwaddr addr);
uint32_t x86_lduw_phys(CPUState *cs, hwaddr addr);
uint32_t x86_ldl_phys(CPUState *cs, hwaddr addr);
uint64_t x86_ldq_phys(CPUState *cs, hwaddr addr);
void x86_stb_phys(CPUState *cs, hwaddr addr, uint8_t val);
void x86_stl_phys_notdirty(CPUState *cs, hwaddr addr, uint32_t val);
void x86_stw_phys(CPUState *cs, hwaddr addr, uint32_t val);
void x86_stl_phys(CPUState *cs, hwaddr addr, uint32_t val);
void x86_stq_phys(CPUState *cs, hwaddr addr, uint64_t val);
```
```c
uint8_t x86_ldub_phys(CPUState *cs, hwaddr addr)
{
    X86CPU *cpu = X86_CPU(cs);
    CPUX86State *env = &cpu->env;
    MemTxAttrs attrs = cpu_get_mem_attrs(env);
    AddressSpace *as = cpu_addressspace(cs, attrs);

    return address_space_ldub(as, addr, attrs, NULL);
}
```
- in the beginning of `misc_helper.c`, io related function 
```c
void helper_outb(CPUX86State *env, uint32_t port, uint32_t data)
{
#ifdef CONFIG_USER_ONLY
    fprintf(stderr, "outb: port=0x%04x, data=%02x\n", port, data);
#else
    address_space_stb(&address_space_io, port, data,
                      cpu_get_mem_attrs(env), NULL);
#endif
```

- 在 helper.c 中，那些 `x86_*_phys` 其实都是简单的找到的 cpu 的 AddressSpace 即可
- helper_outb 中，更加简单，因为 address_space_io 总是固定的

- [x] kvm 模式下会调用这些函数吗 ?
   - 应该不会，调用者都是 helper 而已

## 为什么需要给创建多个 AddressSpace ?
关注下面两个函数，可以发现，创建的两个分别为普通模式和 SMM 模式
- x86_cpu_realizefn
  - cpu_address_space_init

## memory_ldst.c 分析

