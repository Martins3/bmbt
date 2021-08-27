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

## cpu-ldst.h
类似于 cpu_ldq_data_ra 之类的, 目前就是照抄的
希望可以修改为 v6.0 的形式

## atomic_template.h
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
