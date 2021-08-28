# softmmu 和 memory model 的移植的设计 
到底是在补齐接口还是在设计新的 memory model 的, 我认为应该是首先思考清楚整个东西是如何设计的，
想清楚了之后，再去验证这些接口的设计，最后具体的留出来的那些接口。

## [ ] 到底那些地方可以简化
- 因为描述的空间是固定的，所以我猜测可以简化设计，没有必要创建出来 MemoryRegion，但是可以保留出来 FlatRange
- 因为 IO 空间和 mmio 空间的数量有限，暂时可以直接一个数组循环来遍历这些 FlatRange 的
- [ ] 我认为没有必要构建出来 RAMBlock 的 FlatRange 出来，实际上，从 AddressSpace 访问出来，最后访问到 RAM 的情况，应该挑出来, 构建一个新的访问路径
    - 这个事情很容易的，使用 source trail 反向分析就可以了
    - 容易你妈，看看 subpage_read 的实现
- [ ] SMM 还是感觉没有分析清楚，如果可以保证安全，那么我有一些大胆的想法，这就是 vga-low 的地方内容发生了替换而已啊

- 为了处理各种 device 的情况，制作出来了 stl_le_phys 之类的函数，这是没有必要的

- 你觉得 SMC 真的需要 ramlist.dirty_memory 来维持生活吗?
  - [ ] 算了，改动太大了, 不过，现在这么多的 RAM，我只是理解其中的一个
- memory_ldst.h 无需考虑 `#define SUFFIX                   _cached_slow`, 那是给 virtio 使用的


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
