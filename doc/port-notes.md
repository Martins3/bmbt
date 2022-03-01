## 移植过程中的一些记录

- 存在一种编程方法，将一个头文件 include 两次从而实现 template 的，但是这种方法会影响 ccls 的定位。
  - cpu_ldst_template.h 这个文件在 6.0 版本中被替换掉了，这一部分的代码是按照 6.0 的
- 在文件夹的组织上
  2. 暂时将原来在 qemu 中放到顶层目录中的文件放到 qeum 下面了
- 关于 CONFIG_MACHINE_BSWAP_H 这个 macro
    - 是否定义是 configure 的时候传递参数定义的
    - 检查 musl 的源码，发现，当 CONFIG_MACHINE_BSWAP_H 之后，可以使用 libc 的内容，否则使用 QEMU 提供的
- 在 include/qemu/queue.h 中定义 QLIST, 但是 QEMU 同时有使用了 GSList
  - GSList 的使用方法是创建链表然后管理指针
  - 而 QLIST 的方法内核的方法，需要将要管理的结构体内插入一个 entry
  - 所以，其实共存也问题不大, 在加上 GSList 目前完全没有出现，所以暂时不考虑了
- RAM_BLOCK_NUM:
  - 对于 ram bloc 已经是写死的了，或者说，一定不支持 above_4g_mem
- `__cpu_ases`: 直接静态分配的，所以目前仅仅支持一个 CPU
- 当前可以保证 first_cpu == current_cpu

##  memory 移植差异性的记录
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

## 第二次更新更新的文件
- 所有的 latx
- ops_sse.h
- ops_sse_header.h
- atomic_template.h
- src/fpu
- include/fpu
- src/i386/fpu_helper.c
- include/exec/helper-head.h
- include/exec/helper-gen.h
- include/exec/helper-proto.h

## 发生修改的位置
- 被 SYNC_LATX 包围的部分
- `__thread` 被直接删除掉了
- call_constructor 中需要调用的 constructor 函数的 `static __attribute__((constructor))` 删除

## 修改主线的部分
参考 ./sync-latx.md 中，只需要对于每一个 CONFIG_LATX 逐个分析就可以了。

## 注意
- latx_lsenv_init 的位置被提前了，但是那只是为多核设计的，可以不用管。
