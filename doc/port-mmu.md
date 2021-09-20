# softmmu 和 memory model 的移植的设计

## [ ] 到底那些地方可以简化
- 因为描述的空间是固定的，所以我猜测可以简化设计，没有必要创建出来 MemoryRegion，但是可以保留出来 FlatRange
- 因为 IO 空间和 mmio 空间的数量有限，暂时可以直接一个数组循环来遍历这些 FlatRange 的
- 为了处理各种 device 的情况，制作出来了 stl_le_phys 之类的函数，这是没有必要的
- memory_ldst.h 无需考虑 `#define SUFFIX                   _cached_slow`, 那是给 virtio 使用的
- 几乎无需处理 endianness 的问题
- 对于 PAM 和 SMM 我们存在更加深入的观察，这让 render_memory_region 之类的复杂操作毫无意义
  - 没有必要采用 AddressSpace 的概念

- [ ] 猜测一下需要处理的接口
    - address_space_translate : 将这些全部使用 segment RB tree 管理
    - CPUAddressSpace : 在 smm 中间发生替换的时候
      - 进入到 SMM 的时候
    - 处理 RAMBlock 的
        - qemu_map_ram_ptr
        - RAMList 的相关的 dirty memory 的记录
        - 将其 clean 以及清理出来的
    - IO 空间和 memory 的空间需要区分。
        - SMM 空间也是可以区分的
    - 各种 invalidate 的接口
        - 比如 invalidate_and_set_dirty, 实际上，dirty_log_mask 之类的设计可以简化很多的


- 无论如何，RAM 在 host 上的具体地址都是需要进行装换的，所以，RAMBlock::host 是需要的

1. 一个 CPUAddressSpace 持有一个 AddressSpace
2. 在 AddressSpace 中持有一个数组(因为 iotlb_to_section 需要的)
  - iotlb_to_section 是一定需要的，从 store_helper 中直接判断，那才是最重要的入口

3. 暂时直接在这个数组上移动就可以了
  - 对于 1M 直接判断，然后就是 RAM 或者 PCI 了，而 IO 和 memory 早就可以区分
  - 如果 PAM 打开，不需要存在一棵树的

在 1M 的 MemoryRegion 直接创建出来，分配到数组中:
```c
/*
 * SMRAM memory area and PAM memory area in Legacy address range for PC.
 * PAM: Programmable Attribute Map registers
 *
 * 0xa0000 - 0xbffff compatible SMRAM
 *
 * 0xc0000 - 0xc3fff Expansion area memory segments
 * 0xc4000 - 0xc7fff
 * 0xc8000 - 0xcbfff
 * 0xcc000 - 0xcffff
 * 0xd0000 - 0xd3fff
 * 0xd4000 - 0xd7fff
 * 0xd8000 - 0xdbfff
 * 0xdc000 - 0xdffff
 * 0xe0000 - 0xe3fff Extended System BIOS Area Memory Segments
 * 0xe4000 - 0xe7fff
 * 0xe8000 - 0xebfff
 * 0xec000 - 0xeffff
 *
 * 0xf0000 - 0xfffff System BIOS Area Memory Segments
 */
```
1. 如果是 SMM，将这个空间替换掉
2. PAM 每一个寄存器分别对应一个，当进行修改的时候，直接修改对应的属性啊

address_space_translate_internal 中，计算了一个关键的返回值 xlat, 表示在 MemoryRegion 中的偏移。
因为取消掉了 MemoryRegion 的操作，所以，实际上，需要


## 需要保留的接口
实际上，为了防止和原来的设计出现巨大的差异，需要保留的接口:
- address_space_stb -> 所以我们需要 AddressSpace 的


## 每一个函数移植方案
| function                          | 作用                                                                                                          | 方案 |
|-----------------------------------|---------------------------------------------------------------------------------------------------------------|------|
| address_space_translate_for_iotlb | 根据 addr 得到 memory region 的                                                                               |      |
| memory_region_section_get_iotlb   | 计算出来当前的 section 是 AddressSpaceDispatch 中的第几个 section, 之后就可以通过 addr 获取 section 了        |      |
| qemu_map_ram_ptr                  | 这是一个神仙设计的接口，如果参数 ram_block 的接口为 NULL, 那么 addr 是 ram addr， 如果不是，那么是 ram 内偏移 |      |
| cpu_addressspace                  |                                                                                                               |      |

flush 的函数的异步运行其实可以好好简化一下。

实际上整个 ram_addr.h 都是处理 dirty page 的问题，而至于 RAMBlock 的概念具体如何设计，
需要等到之后在处理。

## ram_addr.h
- cpu_physical_memory_test_and_clear_dirty : clear dirty，但是这个一个宽接口，在 BMBT 修改为 cpu_physical_memory_clear_dirty，无需向上汇报是否存在 dirty 的问题，那是给 RAMList 使用的
- 实际上，cpu_physical_memory_is_clean 因为现在只有一个 client，所以应该可以被很容易的修改了

## memory.h
- address_space_translate
- memory_region_dispatch_write / memory_region_dispatch_read

- 需要 MemoryRegion，但是不需要 MemoryRegionSection，因为 MemoryRegion 中间不会被划分，MemoryRegion 就是最小的对象


## 移植差异性的记录
### memory_ldst.h
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

### cpu-ldst.h
类似于 cpu_ldq_data_ra 之类的, 目前就是照抄的
希望可以修改为 v6.0 的形式

### atomic_template.h
atomic_template.h 被 cputlb.c include 了 8 次，四种数据大小 * 两种调用接口

```c
/* First set of helpers allows passing in of OI and RETADDR.  This makes
   them callable from other helpers.  */

/* Second set of helpers are directly callable from TCG as helpers.  */
```


```c
// first set 生成的代码
uint32_t helper_atomic_fetch_addl_le_mmu
uint32_t helper_atomic_fetch_andl_le_mmu
uint32_t helper_atomic_fetch_orl_le_mmu
uint32_t helper_atomic_fetch_xorl_le_mmu
uint32_t helper_atomic_add_fetchl_le_mmu
uint32_t helper_atomic_and_fetchl_le_mmu
uint32_t helper_atomic_or_fetchl_le_mmu
uint32_t helper_atomic_xor_fetchl_le_mmu

// second set 生成的函数
uint32_t helper_atomic_fetch_addl_le
uint32_t helper_atomic_fetch_andl_le
uint32_t helper_atomic_fetch_orl_le
uint32_t helper_atomic_fetch_xorl_le
uint32_t helper_atomic_add_fetchl_le
uint32_t helper_atomic_and_fetchl_le
uint32_t helper_atomic_or_fetchl_le
uint32_t helper_atomic_xor_fetchl_le
```

两个具体的差别如下，这是两种调用的方法:
```diff
-uint32_t helper_atomic_cmpxchgl_le(CPUArchState *env, target_ulong addr,
-                              uint32_t cmpv, uint32_t newv , TCGMemOpIdx oi)
+# 81 "a.c"
+uint32_t helper_atomic_cmpxchgl_le_mmu(CPUArchState *env, target_ulong addr,
+                              uint32_t cmpv, uint32_t newv , TCGMemOpIdx oi, uintptr_t retaddr)
 {
     ;
-    uint32_t *haddr = atomic_mmu_lookup(env, addr, oi, GETPC());
+    uint32_t *haddr = atomic_mmu_lookup(env, addr, oi, retaddr);
     uint32_t ret;
     uint16_t info = trace_mem_build_info_no_se_le(2, false,
                                                            get_mmuidx(oi));
@@ -25,12 +25,12 @@ uint32_t helper_atomic_cmpxchgl_le(CPUArchState *env, target_ulong addr,
     atomic_trace_rmw_post(env, addr, info);
     return ret;
 }
```

但是，构造出来的这么多函数，目前使用者只有 helper_atomic_cmpxchgq_le_mmu

# include/exec/memory_ldst_phys.inc.h
因为目前只有一个用户: stl_le_phys

所以只是在 cpu-all.h 中间增加了下面两个函数，其余利用上 memory_ldst.inc.c 的内容
```c
extern void address_space_stl_le(AddressSpace *as, hwaddr addr, uint32_t val,
                                 MemTxAttrs attrs, MemTxResult *result);

static inline void stl_le_phys(AddressSpace *as, hwaddr addr, uint32_t val) {
  address_space_stl_le(as, addr, val, MEMTXATTRS_UNSPECIFIED, NULL);
}
```
