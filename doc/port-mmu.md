# softmmu 和 memory model 的移植的设计 

- [ ] memory.h
  - 需要 MemoryRegion，但是不需要 MemoryRegionSection，因为 MemoryRegion 中间不会被划分，MemoryRegion 就是最小的对象
- [ ] io_readx 和 io_writex
- [ ] 重新构建 iotlb


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


一些设计的想法:
- RAMList 的
- 在 1M 的范围内的空间的变化过于鬼畜啊
  - 使用一个 segment RB tree 来管理吧! (使用内核的方法)
  - 根据空间首先命中 MemoryRegion，然后利用 MemoryRegion 来找到 RAMBlock，然后看 RAMBlock 的实际位置
  - 还不如直接划分为 BIOS 空间 / PCI 空间 / RAM 空间，反正 PCI 空间都是确定的
- 无论如何，RAM 在 host 上的具体地址都是需要进行装换的，所以，RAMBlock::host 是需要的

## 需要保留的接口
实际上，为了防止和原来的设计出现巨大的差异，需要保留的接口:
- address_space_stb -> 所以我们需要 AddressSpace 的


## 每一个函数移植方案
| function                          | 作用                                                                                                          | 方案 |
|-----------------------------------|---------------------------------------------------------------------------------------------------------------|------|
| address_space_translate_for_iotlb | 根据 addr 得到 memory region 的                                                                               |      |
| memory_region_section_get_iotlb   | 计算出来当前的 section 是 AddressSpaceDispatch 中的第几个 section, 之后就可以通过 addr 获取 section 了        |      |
| qemu_map_ram_ptr                  | 这是一个神仙设计的接口，如果参数 ram_block 的接口为 NULL, 那么 addr 是 ram addr， 如果不是，那么是 ram 内偏移 |      |
| cpu_addressspace |

flush 的函数的异步运行其实可以好好简化一下。

实际上整个 ram_addr.h 都是处理 dirty page 的问题，而至于 RAMBlock 的概念具体如何设计，
需要等到之后在处理。

## ram_addr.h
- cpu_physical_memory_test_and_clear_dirty : clear dirty，但是这个一个宽接口，在 BMBT 修改为 cpu_physical_memory_clear_dirty，无需向上汇报是否存在 dirty 的问题，那是给 RAMList 使用的
- 实际上，cpu_physical_memory_is_clean 因为现在只有一个 client，所以应该可以被很容易的修改了

## memory.h
- address_space_translate


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
