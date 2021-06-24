# memory model

在 https://github.com/kernelrookie/DuckBuBi/issues/35 中，
分析 `address_space_*` 以及如何检查 memory_ldst.inc.c 和
memory_ldst.inc.h 的方法。

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
这几个函数几乎都是对称的，但是 address_space_stl_notdirty 稍有不同

- [ ] address_space_stl_notdirty 还没有分析

分析 memory.h 吧。

| function                                                   | desc                                                                                                          |
|------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------|
| address_space_translate                                    | 通过 hwaddr 参数找到 MemoryRegion 这里和 Flatview 有关的 |
| memory_region_dispatch_read / memory_region_dispatch_write | 最关键的，访问 device, 逐步向下分发的过程                                                                     |
| memory_region_get_dirty_log_mask                           | 获取 MemoryRegion::dirty_log_mask                                                                             |
| memory_region_get_ram_addr                                 |                                                                                                               |
| devend_memop                                               | 使用一个非常繁杂的宏判断，进行 IO 的时候，设备是否需要进行 endiana 调整，从现在的调用链看，应该永远返回都是 0 |
| qemu_map_ram_ptr                                           | 仔细看看注释，为了定义从 memory region 中的偏移获取 HVA, 定义了一堆函数                                       |
| invalidate_and_set_dirty                                   | 将一个范围的 TLB invalidate 利用 DirtyMemoryBlocks 标记这个区域为 dirty                                       |
| prepare_mmio_access                                        | 如果没有上 QBL 的话，那么把锁加上去                                                                           |
| memory_access_is_direct                                    | 判断内存到底是可以直接写，还是设备空间，需要重新处理一下                                                      |

按道理，memory_ldst 提供的是标准访存接口，那么:

- store_helper 和 io_readx 是如何实现的 ?
  - io_readx 是 address_space_stw_internal 的简化版，相当于直接调用 memory_region_dispatch_read
  - store_helper 是 address_space_stw_internal 的强化版本
    - 主要是需要处理 TLB 命中的问题
    - 以及非对其访问，因为 address_space_stw_internal 的调用者都是从 helper 哪里来的，所以要容易的多

## RAMBlock
- qemu_ram_alloc : MemoryRegion::ram_block 总是使用这个初始化
  - ram_block_add
    - phys_mem_alloc (qemu_anon_ram_alloc)
      - qemu_ram_mmap
        - mmap : 可见，RAMBlock 就是分配一块空间

- address_space_rw 和 address_space_stl 之类的关系是什么 ?
  - 含义很清晰(指定 address_space 来访问)，但是，到目前为止，没有指向 address_space_rw 调用路径
  - cpu_physical_memory_rw 是关键的调用者


```c
/* Return a host pointer to ram allocated with qemu_ram_alloc.
 * This should not be used for general purpose DMA.  Use address_space_map
 * or address_space_rw instead. For local memory (e.g. video ram) that the
 * device owns, use memory_region_get_ram_ptr.
 *
 * Called within RCU critical section.
 */
void *qemu_map_ram_ptr(RAMBlock *ram_block, ram_addr_t addr)
```

- [ ] 关注一下这两个全局变量的作用
```c
AddressSpace address_space_io;
AddressSpace address_space_memory;
```

## [ ] 需要考虑 IOMMU 的问题吗 ?
