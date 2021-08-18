# memory model

在 https://github.com/kernelrookie/DuckBuBi/issues/35 中，
分析 `address_space_*` 以及如何检查 memory_ldst.inc.c 和
memory_ldst.inc.h 的方法。

在 v6.0 中
| file             | desc                                                            |
|------------------|-----------------------------------------------------------------|
| softmmu/memory.c | memory_region_dispatch_read 之类的各种 memory region 的管理工作 |
| softmmu/physmem  | RAMBlock 之类的管理                                             |

- flatview_for_each_range 从来不会被调用
- memory_region_read_with_attrs_accessor 从来不会被调用

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

## 为什么需要给创建多个 AddressSpace 
- [ ] TCG 模式下，实际上，会创建 SMM 的 address space 出来，什么时候搞出来的，为什么需要单独高处这个东西来

memory listener 都是挂载到具体的 AddressSpace 上的

- [ ] info mtree : 关于 memory region, 会发现几个问题:
  - address-space: memory 和 address-space: I/O
  - 每一个 cpu 为什么还创建了自己的 address space address-space: cpu-memory-0
  - 所有的 pci 设备都创建了自己的 addresss space

- [ ] 从其中的元素分析吧
  - memory listener
  - ioeventfd

```c
static MemoryRegion *system_memory;
static MemoryRegion *system_io;

AddressSpace address_space_io;
AddressSpace address_space_memory;
```
这两个 AddressSpace 的初始位置，都是在 memory_map_init, 将其和 system_memory 和 system_io 联系起来
而之后的一些列初始化和内容的填充都是通过这两个 MemoryRegion 完成的。

- [ ] 暂时打住一下
  - [ ] 在分析 fw_cfg 的时候，我忽然意识到 address_space_memory, 这个东西几乎只是和 iommu 相关，这个是 `info mtree` 中间的那两个 AddressSpace 吧

AddressSpace 关联一个 MemoryRegion, 通过 MemoryRegion 可以找到 Flatview Root, 从而找到该 as 关联的真正 flatview
而是 flatview 决定了 io 真正的地址 (address_space_set_flatview)

- 通过  `static GHashTable *flat_views;` 可以找到通过 mr 找到 flatview

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

RAMBlock 结构体分析:
1. RAMBlock::host : host 的虚拟地址空间，存储 mmap 的返回值


- address_space_rw 和 address_space_stl 之类的关系是什么 ?
  - 含义很清晰(指定 address_space 来访问)，但是，到目前为止，没有指向 address_space_rw 调用路径
  - cpu_physical_memory_rw 是关键的调用者

## QEMU Memory Model 结构分析
https://kernelgo.org/images/qemu-address-space.svg

一致存在一个很强的误导，因为 memory model 一层层的很复杂，实际上，info mtree 出来的内容相当的简单啊

关键结构体内容分析:
| struct               | desc                                                                                                                |
|----------------------|---------------------------------------------------------------------------------------------------------------------|
| AddressSpace         | root : 仅仅关联一个 MemoryRegion, current_map : 关联 Flatview，其余是 ioeventfd 之类的                              |
| MemoryRegion         | 主要成员 ram_block, ops, *container*, *alias*, **subregions**, 看来是 MemoryRegion 通过 subregions 负责构建树形结构 |
| Flatview             | ranges : 通过 render_memory_region 生成, 成员 nr nr_allocated 来管理其数量, root : 关联的 MemoryRegions , dispatch  |
| AddressSpaceDispatch | 保存 GPA 到 HVA 的映射关系                                                                                          |


- cpu_address_space_init : 初始化 `CPUAddressSpace *CPUState::cpu_ases`, CPUAddressSpace 的主要成员 AddressSpace + CPUState
  - address_space_init : 使用 MemoryRegion 来初始化 AddressSpace，除了调用
    - address_space_update_topology
      - memory_region_get_flatview_root
      - generate_memory_topology
        - render_memory_region
        - flatview_simplify
        - address_space_dispatch_new : 初始化 Flatview::dispatch

> info mtree [-f][-d][-o][-D] -- show memory tree (-f: dump flat view for address spaces;-d: dump dispatch tree, valid with -f only);-o: dump region owners/parents;-D: dump disabled regions

## render_memory_region
- memory_region_transaction_commit
  - flatviews_reset
    - generate_memory_topology : Render a memory topology into a list of disjoint absolute ranges.
      - render_memory_region : 虽然是一个很长的函数, 
        1. 如果是 alias, 那么 render alias
        2. 如果存在 child，那么按照优先级 render child, memory_region_add_subregion_common 优先级是满足的
        3. 最后，Render the region itself into any gaps left by the current view.
        4. 终极目的，创建 FlatRange 出来，并且使用 flatview_insert 将 FlatRange 放到 FlatView::ranges 数组上
      - flatview_simplify
      - address_space_dispatch_new : 初始化 FlatView::dispatch
      - flatview_add_to_dispatch
      - address_space_dispatch_compact

## AddressSpaceDispatch 的制作过程
这个玩意设计成为多级页面的目的和页表查询的作用应该差不多吧!

```c
struct AddressSpaceDispatch {
    MemoryRegionSection *mru_section;
    /* This is a multi-level map on the physical address space.
     * The bottom level has pointers to MemoryRegionSections.
     */
    PhysPageEntry phys_map;
    PhysPageMap map;
};
```
- phys_map : 相当于 cr3
- map : 相当于管理所有的 page tabel 的页面
- mru_section : 缓存

```c
struct PhysPageEntry {
    /* How many bits skip to next level (in units of L2_SIZE). 0 for a leaf. */
    uint32_t skip : 6;
     /* index into phys_sections (!skip) or phys_map_nodes (skip) */
    uint32_t ptr : 26;
};
```
- skip : 表示还有多少级就可以到 leaf 节点
- prt : 下标
  - 如果 leaf 节点，那么就是索引 PhysPageMap::sections 的下标
  - 如果 non-leaf 节点，那么就是索引  PhysPageMap::nodes 的下标

```c
typedef PhysPageEntry Node[P_L2_SIZE];

typedef struct PhysPageMap {
    struct rcu_head rcu;

    unsigned sections_nb;
    unsigned sections_nb_alloc;
    unsigned nodes_nb;
    unsigned nodes_nb_alloc;
    Node *nodes;
    MemoryRegionSection *sections;
} PhysPageMap;
```
- Node : 定义这个结构体，相当于定义了一个 page table
- nodes : 存储所有的 Node，如果分配完了，使用 phys_map_node_reserve 来补充
- sections : 最终想要获取的

如何构建 PhysPageMap 这颗树:
- flatview_add_to_dispatch
  - register_subpage : 如果 MemoryRegionSection 无法完整地覆盖一整个页
  - register_multipage
    - phys_section_add : 将 MemoryRegionSection 添加到 PhysPageMap::sections
    - phys_page_set : 设置对应的 PhysPageMap::nodes
      - phys_map_node_reserve : 预留空间
      - phys_page_set_level :

## 神奇的 memory_region_get_flatview_root
这个函数，其实有点硬编码, 参考其中的注释，感觉这个东西就是为了实现处理 PCIDevice 的

之所以创建这个函数，是为了更好的共享让不同的 memory_region 共享 flatview

返回值 mr = system
参数 ori = bus master container
```c
/*
address-space: e1000
  0000000000000000-ffffffffffffffff (prio 0, i/o): bus master container
    0000000000000000-ffffffffffffffff (prio 0, i/o): alias bus master @system 0000000000000000-ffffffffffffffff
```

```c
static MemoryRegion *memory_region_get_flatview_root(MemoryRegion *mr)
{
    while (mr->enabled) {
        if (mr->alias) {
            if (!mr->alias_offset && int128_ge(mr->size, mr->alias->size)) {
                /* The alias is included in its entirety.  Use it as
                 * the "real" root, so that we can share more FlatViews.
                 */
                mr = mr->alias;
                continue;
            }
        } else if (!mr->terminates) {
            unsigned int found = 0;
            MemoryRegion *child, *next = NULL;
            QTAILQ_FOREACH(child, &mr->subregions, subregions_link) {
                if (child->enabled) {
                    // 如果发现了多个 child, 一定会返回 return mr
                    if (++found > 1) {
                        next = NULL;
                        break;
                    }
                    if (!child->addr && int128_ge(mr->size, child->size)) {
                        /* A child is included in its entirety.  If it's the only
                         * enabled one, use it in the hope of finding an alias down the
                         * way. This will also let us share FlatViews.
                         */
                        next = child;
                    }
                }
            }
            if (found == 0) {
                return NULL;
            }
            if (next) {
                // 这种情况就是上面的注释说明的，只有一个 child, 那么就像是 flatview 的工作方式了
                mr = next;
                continue;
            }
        }

        return mr;
    }

    return NULL;
}
```

## alias
machine_run_board_init 中初始化 `machine->ram`, 也就是 pc.ram 这个 memory region

而分析 system 这个 memory region, 发现其中的两个 subregion ram-below-4g 和 ram-above-4g 都是
是 pc.ram 的 alias.  也即是一个 memory region 的 subregion 可以是其他的 alias

```c
/*
address-space: memory
  0000000000000000-ffffffffffffffff (prio 0, i/o): system
    0000000000000000-00000000bfffffff (prio 0, ram): alias ram-below-4g @pc.ram 0000000000000000-00000000bfffffff
    0000000100000000-00000001bfffffff (prio 0, ram): alias ram-above-4g @pc.ram 00000000c0000000-000000017fffffff

memory-region: pc.ram
  0000000000000000-000000017fffffff (prio 0, ram): pc.ram
```
仔细想想，这么设计是很有道理的, 这样，一块物理内存是作为一个 MemoryRegion，拥有相同的属性，
而 system memory 是实际上物理内存中存在空洞的。

此外，alias 的一个例子在 isa 地址空间:
```c
/*
      00000000000e0000-00000000000fffff (prio 1, rom): alias isa-bios @pc.bios 0000000000020000-000000000003ffff
      00000000fffc0000-00000000ffffffff (prio 0, rom): pc.bios
*/
```

- 现在思考一个问题，如何保证访问 alias 的时候，最后获取的是正确的地址:
  - memory_region_get_ram_ptr 中存在一个转换

#### QEMU 内存虚拟化源码分析[^1]
首先，qemu 中用 AddressSpace 用来表示 CPU/设备看到的内存，一个 AddressSpace 下面包含多个 MemoryRegion，这些 MemoryRegion 结构通过树连接起来，树的根是 AddressSpace 的 root 域。

MemoryRegion 有多种类型，可以表示一段 ram，rom，MMIO，alias，alias 表示一个 MemoryRegion 的一部分区域，MemoryRegion 也可以表示一个 container，这就表示它只是其他若干个 MemoryRegion 的容器。在 MemoryRegion 中，'ram_block'表示的是分配的实际内存。

AddressSpace 下面 root 及其子树形成了一个虚拟机的物理地址，但是在往 kvm 进行设置的时候，需要将其转换为一个平坦的地址模型，也就是从 0 开始的。这个就用 FlatView 表示，**一个 AddressSpace 对应一个 FlatView**。

在 FlatView 中，FlatRange 表示按照需要被切分为了几个范围。
在内存虚拟化中，还有一个重要的结构是 MemoryRegionSection，这个结构通过函数 section_from_flat_range 可由 FlatRange 转换过来。

mr 很多时候是创建一个 alias，指向已经存在的 mr 的一部分，这也是 alias 的作用

*继续 pc_memory_init，函数在创建好了 ram 并且分配好了空间之后，创建了两个 mr alias，ram_below_4g 以及 ram_above_4g，这两个 mr 分别指向 ram 的低 4g 以及高 4g 空间，这两个 alias 是挂在根 system_memory mr 下面的。*

> - [ ] 这个结构很难理解啊，即是一个 memory region 的 subregion，又是另一个 region 的 alias

为了在虚拟机退出时，能够顺利根据物理地址找到对应的 HVA 地址，qemu 会有一个 AddressSpaceDispatch 结构，用来在 AddressSpace 中进行位置的找寻，继而完成对 IO/MMIO 地址的访问。
> 其实不是获取 HVA，而是通过 GPA 获取到对应的 dispatch 函数

为了监控虚拟机的物理地址访问，对于每一个 AddressSpace，会有一个 MemoryListener 与之对应。每当物理映射（`GPA->HVA`)发生改变时，会回调这些函数。

在上面看到 MemoryListener 之后，我们看看什么时候需要更新内存。 进行内存更新有很多个点，比如我们新创建了一个 AddressSpace address_space_init，再比如我们将一个 mr 添加到另一个 mr 的 subregions 中 memory_region_add_subregion,再比如我们更改了一端内存的属性 memory_region_set_readonly，将一个 mr 设置使能或者非使能 memory_region_set_enabled, 总之一句话，我们修改了虚拟机的内存布局/属性时，就需要通知到各个 Listener，这包括各个 AddressSpace 对应的，以及 kvm 注册的，这个过程叫做 commit，通过函数 memory_region_transaction_commit 实现。

进行内存更新有很多个点，比如我们新创建了一个 AddressSpace address_space_init，再比如我们将一个 mr 添加到另一个 mr 的 subregions 中 memory_region_add_subregion,再比如我们更改了一端内存的属性 memory_region_set_readonly，将一个 mr 设置使能或者非使能 memory_region_set_enabled, 总之一句话，我们修改了虚拟机的内存布局/属性时，就需要通知到各个 Listener，这包括各个 AddressSpace 对应的，以及 kvm 注册的，这个过程叫做 commit，通过函数 memory_region_transaction_commit 实现。

## memory listener
TCG
- cpu_address_space_init 中注册 memory listener

- 忽然意识到，CPUAddressSpace 只是 tcg 特有的
```c
/**
 * CPUAddressSpace: all the information a CPU needs about an AddressSpace
 * @cpu: the CPU whose AddressSpace this is
 * @as: the AddressSpace itself
 * @memory_dispatch: its dispatch pointer (cached, RCU protected)
 * @tcg_as_listener: listener for tracking changes to the AddressSpace
 */
struct CPUAddressSpace {
    CPUState *cpu;
    AddressSpace *as;
    struct AddressSpaceDispatch *memory_dispatch;
    MemoryListener tcg_as_listener;
};
```

kvm 的注册方式:
- kvm_init
  - kvm_memory_listener_register
    - memory_listener_register

- kvm 根本没有注册 MemoryListener::commit

- tcg_commit
  - 处理一些 RCU，iothread 的问题
  - tlb_flush

listener 的 hook 分析：
| hook                  | desc                                                                                                                                                                                                                                    |
|-----------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| begin                 |                                                                                                                                                                                                                                         |
| region_add            | address_space_set_flatview 调用这个 hook, kvm_region_add 最后会调用到 kvm_vm_ioctl(s, KVM_SET_USER_MEMORY_REGION, &mem)  上                                                                                                             |
| region_del            |                                                                                                                                                                                                                                         |
| region_nop            |                                                                                                                                                                                                                                         |
| log_start             | listener_add_address_space 和 address_space_update_topology_pass 两个，kvm_log_start 最终调用到 kvm_vm_ioctl(s, KVM_SET_USER_MEMORY_REGION, &mem)                                                                                       |
| log_stop              |                                                                                                                                                                                                                                         |
| commit                |                                                                                                                                                                                                                                         |
| log_sync              | Called by memory_region_snapshot_and_clear_dirty() and memory_global_dirty_log_sync(), before accessing QEMU's "official" copy of the dirty memory bitmap for a #MemoryRegionSection. 最终调用到 kvm_vm_ioctl(s, KVM_GET_DIRTY_LOG, &d) |
| log_sync_global       | This is the global version of @log_sync when the listener does not have a way to synchronize the log with finer granularity. When the listener registers with @log_sync_global defined, then its @log_sync must be NULL.  Vice versa.   |
| log_clear             |                                                                                                                                                                                                                                         |
| log_global_start      |                                                                                                                                                                                                                                         |
| log_global_stop       |                                                                                                                                                                                                                                         |
| log_global_after_sync |                                                                                                                                                                                                                                         |
| eventfd_add           |                                                                                                                                                                                                                                         |
| eventfd_del           |                                                                                                                                                                                                                                         |
| coalesced_io_add      |                                                                                                                                                                                                                                         |
| coalesced_io_del      |                                                                                                                                                                                                                                         |

实际上，这些 hook 都是 KVM 注册的:
- region_add / region_del 是因为需要通过 iotcl 告知 kvm
- eventfd 和 coalesced_io 都是需要和内核打交道的机制
- 关于 dirty log 可以参考李强的 blog[^1]

## dirty log
- DIRTY_MEMORY_CODE : 和 ram_list 配合使用的时候，为什么划分为三种类型的内存
```c
#define DIRTY_MEMORY_VGA       0
#define DIRTY_MEMORY_CODE      1
#define DIRTY_MEMORY_MIGRATION 2
#define DIRTY_MEMORY_NUM       3        /* num of dirty bits */
```

- colo_incoming_start_dirty_log
  - ramblock_sync_dirty_bitmap
    - cpu_physical_memory_sync_dirty_bitmap

RAMList 和 DirtyMemoryBlocks 在搞什么, 这个东西为什么会成为 memory region 啊

qemu_ram_alloc
  - qemu_ram_alloc_internal
    - ram_block_add
      - dirty_memory_extend : 应该是唯一初始化 ram_list.dirty_memory 的位置吧, 另外使用的位置在 cpu_physical_memory_test_and_clear_dirty 和  cpu_physical_memory_snapshot_and_clear_dirty


- 在正常的 kvm 其中的操作过程中，cpu_physical_memory_test_and_clear_dirty 和  cpu_physical_memory_snapshot_and_clear_dirty 都不会被调用
- 在 cpu_physical_memory_test_and_clear_dirty 作为 SMC 的基础设施, 这是为数不多的需要支持的接口

- tlb_protect_code
  - [ ] cpu_physical_memory_test_and_clear_dirty : 从 tcg 的角度，这个函数中间有一堆似乎没用用的东西，之后再去分析吧
    - memory_region_clear_dirty_bitmap : 因为 tcg 的 memory listener 没有注册 `listener->log_clear`, 所以这个函数什么都是不需要做的
    - tlb_reset_dirty_range_all
      - tlb_reset_dirty : 将这个范围内的 TLB 全部添加上 TLB_NOTDIRTY

- store_helper
  - notdirty_write : 当写向一个 dirty 的位置的处理
    - cpu_physical_memory_get_dirty_flag
    - tb_invalidate_phys_page_fast : 
    - cpu_physical_memory_set_dirty_range : Set both VGA and migration bits for simplicity and to remove the notdirty callback faster.
    - tlb_set_dirty

```c
/*
#0  cpu_physical_memory_set_dirty_range (start=952888, length=4, mask=5 '\005') at /home/maritns3/core/kvmqemu/include/exec/ram_addr.h:290
#1  0x0000555555c6d24f in notdirty_write (cpu=0x555556d5fc00, mem_vaddr=952888, size=4, iotlbentry=0x7ffdcc01db90, retaddr=140734602805726) at ../accel/tcg/cputlb.c:156
1
#2  0x0000555555c6f601 in store_helper (env=0x555556d68480, addr=952888, val=3221225472, oi=34, retaddr=140734602805726, op=MO_32) at ../accel/tcg/cputlb.c:2451
#3  0x0000555555c6f815 in helper_le_stl_mmu (env=0x555556d68480, addr=952888, val=3221225472, oi=34, retaddr=140734602805726) at ../accel/tcg/cputlb.c:2505
#4  0x00007fff540201de in code_gen_buffer ()
#5  0x0000555555cb0254 in cpu_tb_exec (cpu=0x555556d5fc00, itb=0x7fff940200c0, tb_exit=0x7fffe888e1b0) at ../accel/tcg/cpu-exec.c:190
#6  0x0000555555cb11d8 in cpu_loop_exec_tb (cpu=0x555556d5fc00, tb=0x7fff940200c0, last_tb=0x7fffe888e1b8, tb_exit=0x7fffe888e1b0) at ../accel/tcg/cpu-exec.c:673
#7  0x0000555555cb14c9 in cpu_exec (cpu=0x555556d5fc00) at ../accel/tcg/cpu-exec.c:798
#8  0x0000555555d60555 in tcg_cpus_exec (cpu=0x555556d5fc00) at ../accel/tcg/tcg-accel-ops.c:67
#9  0x0000555555c496c3 in mttcg_cpu_thread_fn (arg=0x555556d5fc00) at ../accel/tcg/tcg-accel-ops-mttcg.c:70
#10 0x0000555555f53fcd in qemu_thread_start (args=0x555556c6dd10) at ../util/qemu-thread-posix.c:521
#11 0x00007ffff6298609 in start_thread (arg=<optimized out>) at pthread_create.c:477
#12 0x00007ffff61bd293 in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:95
```
- migration_bitmap_sync
  - memory_global_dirty_log_sync
    - memory_region_sync_dirty_bitmap
      - MemoryListener::log_sync
         - kvm_log_sync
            - kvm_physical_sync_dirty_bitmap
              - kvm_slot_get_dirty_log  : 使用 KVM_GET_DIRTY_LOG ioctl
              - kvm_slot_sync_dirty_pages
                - cpu_physical_memory_set_dirty_lebitmap : 这个将从 kvm 中得到的 dirty map 的信息放到 ram_list.dirty_memory
      - MemoryListener::log_sync_global
  - ramblock_sync_dirty_bitmap
    - cpu_physical_memory_sync_dirty_bitmap

总结一下，为了让 dirty log 机制是放到一起的，memory_global_dirty_log_sync 对于 tcg 是一个空函数，实际上，tcg 通过 cpu_physical_memory_sync_dirty_bitmap 将 dirty log 直接可以放到
ramlist.dirty_memory 上

## TCG 和 SMM
SMM 实际上是给 firmware 使用的

在 tcg_cpu_realizefn 中

- get_system_memory : 获取的 MemoryRegion 的名称为 system, 总会挂到 cpu-memory-0 / cpu-memory-2 上

```plain
address-space: tcg-cpu-smm-0
  0000000000000000-ffffffffffffffff (prio 0, i/o): cpu_as_root-memory
    0000000000000000-00000000ffffffff (prio 1, i/o): alias smram @smram 0000000000000000-00000000ffffffff
    0000000000000000-ffffffffffffffff (prio 0, i/o): alias cpu_as_mem-memory @system 0000000000000000-ffffffffffffffff
```

通过 qemu_add_machine_init_done_notifier 调用
在 tcg_cpu_machine_done 中，从而在 cpu_as_root-memory 下创建一个 smram


而在 i440fx_init 中，创建出来了 smram
```plain
memory-region: smram
  0000000000000000-00000000ffffffff (prio 0, i/o): smram
    00000000000a0000-00000000000bffff (prio 0, ram): alias smram-low @pc.ram 00000000000a0000-00000000000bffff
```

## QEMU 和 IOMMU 的关系
在 [^6] 分析了下为什么 guest 需要 vIOMMU

pci_device_iommu_address_space : 如果一个 device 被用于直通，那么其进行 IO 的 address space 就可能不是
address_space_memory

## PCI Device AddressSpace
想要构建如下的结构，分别发生在: pci_init_bus_master 和 do_pci_register_device
```c
/*
address-space: e1000
  0000000000000000-ffffffffffffffff (prio 0, i/o): bus master container
    0000000000000000-ffffffffffffffff (prio 0, i/o): alias bus master @system 0000000000000000-ffffffffffffffff
```

```c
struct PCIDevice {
    // ...
    PCIIORegion io_regions[PCI_NUM_REGIONS];
    AddressSpace bus_master_as;
    MemoryRegion bus_master_container_region;
    MemoryRegion bus_master_enable_region;
```

```c
static void pci_init_bus_master(PCIDevice *pci_dev)
{
    AddressSpace *dma_as = pci_device_iommu_address_space(pci_dev); // dma 的空间就是 system memory

    memory_region_init_alias(&pci_dev->bus_master_enable_region,
                             OBJECT(pci_dev), "bus master",
                             dma_as->root, 0, memory_region_size(dma_as->root)); // 创建一个 alias 到 system memory
    memory_region_set_enabled(&pci_dev->bus_master_enable_region, false);
    memory_region_add_subregion(&pci_dev->bus_master_container_region, 0, // 创建一个 container
                                &pci_dev->bus_master_enable_region);
}
```

- do_pci_register_device
   - `address_space_init(&pci_dev->bus_master_as, &pci_dev->bus_master_container_region, pci_dev->name);`

## MemoryRegionSection and RCU 
[^4] 中间提到了一个非常有意思的事情，将 MemoryRegion 的 inaccessible 和 destroy 划分为两个阶段
所以使用 rcu, 其中涉及到
- memory_region_destroy / memory_region_del_subregion
- hotplug

## address_space_map 和 address_space_unmap 是如何使用的
当访问的空间不是 memory_access_is_direct 的时候，那么需要考虑, 目前的系统中并不知道如何触发这个东西，
所以暂时放到这里，以后再说吧
```c
typedef struct {
    MemoryRegion *mr;
    void *buffer;
    hwaddr addr;
    hwaddr len;
    bool in_use;
} BounceBuffer;
```

否则，address_space_map 和 qemu_map_ram_ptr 一样，只是用于从 GPA 计算出来 HVA 而已

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

## dma
暂时，认为 dma 并不是很难移植，这些调用过程只是为了组装 address_space_rw 的参数而已

- [ ] dma_barrier
```c
/*
#0  flatview_read_continue (fv=0x0, addr=655360, attrs=..., ptr=0x7fffe888d7a0, len=93825001741418, addr1=93825012630272, l=16, mr=0x0) at ../softmmu/physmem.c:2818
#1  0x0000555555d31fce in flatview_read (fv=0x7ffdcc06d2e0, addr=28476, attrs=..., buf=0x7fffe888d9c0, len=16) at ../softmmu/physmem.c:2870
#2  0x0000555555d3205b in address_space_read_full (as=0x5555567a6b60 <address_space_memory>, addr=28476, attrs=..., buf=0x7fffe888d9c0, len=16) at ../softmmu/physmem.c:2883
#3  0x0000555555d32187 in address_space_rw (as=0x5555567a6b60 <address_space_memory>, addr=28476, attrs=..., buf=0x7fffe888d9c0, len=16, is_write=false) at ../softmmu/physmem.c:2911
#4  0x00005555559171ef in dma_memory_rw_relaxed (as=0x5555567a6b60 <address_space_memory>, addr=28476, buf=0x7fffe888d9c0, len=16, dir=DMA_DIRECTION_TO_DEVICE) at /home
/maritns3/core/kvmqemu/include/sysemu/dma.h:88
#5  0x000055555591723c in dma_memory_rw (as=0x5555567a6b60 <address_space_memory>, addr=28476, buf=0x7fffe888d9c0, len=16, dir=DMA_DIRECTION_TO_DEVICE) at /home/maritns
3/core/kvmqemu/include/sysemu/dma.h:127
#6  0x0000555555917274 in dma_memory_read (as=0x5555567a6b60 <address_space_memory>, addr=28476, buf=0x7fffe888d9c0, len=16) at /home/maritns3/core/kvmqemu/include/sysemu/dma.h:145
#7  0x0000555555918732 in fw_cfg_dma_transfer (s=0x555556edda00) at ../hw/nvram/fw_cfg.c:360
#8  0x0000555555918b73 in fw_cfg_dma_mem_write (opaque=0x555556edda00, addr=4, value=28476, size=4) at ../hw/nvram/fw_cfg.c:469
#9  0x0000555555ca6ae5 in memory_region_write_accessor (mr=0x555556eddd80, addr=4, value=0x7fffe888db18, size=4, shift=0, mask=4294967295, attrs=...) at ../softmmu/memo
ry.c:489
#10 0x0000555555ca6cc2 in access_with_adjusted_size (addr=4, value=0x7fffe888db18, size=4, access_size_min=1, access_size_max=8, access_fn=0x555555ca69f8 <memory_region
_write_accessor>, mr=0x555556eddd80, attrs=...) at ../softmmu/memory.c:545
#11 0x0000555555ca9de3 in memory_region_dispatch_write (mr=0x555556eddd80, addr=4, data=28476, op=MO_32, attrs=...) at ../softmmu/memory.c:1507
#12 0x0000555555d3367a in address_space_stl_internal (as=0x5555567a6b00 <address_space_io>, addr=1304, val=1013907456, attrs=..., result=0x0, endian=DEVICE_NATIVE_ENDIA
N) at /home/maritns3/core/kvmqemu/memory_ldst.c.inc:319
#13 0x0000555555d33775 in address_space_stl (as=0x5555567a6b00 <address_space_io>, addr=1304, val=1013907456, attrs=..., result=0x0) at /home/maritns3/core/kvmqemu/memo
ry_ldst.c.inc:350
#14 0x0000555555b458a8 in helper_outl (env=0x555556d66880, port=1304, data=1013907456) at ../target/i386/tcg/sysemu/misc_helper.c:54
```

## dispatch 的过程(1) : 百川归海
进行 pio / mmio 最后总是到达 : memory_region_dispatch_read

- 非常尴尬, 才意识到，从 memory_ldst.c 的 address_space_stl 调用的时候都是物理地址啊
  - 从操作系统的角度，进行 IO 也是经过了自己的 TLB 翻译的之后，才得到物理地址的啊，之后这个地址才会发给地址总线
  - [ ] IO 也需要从 softmmu 中翻译，找到对应的代码验证一下


使用 memory_ldst.c 的 address_space_ldl_internal 中分析

- helper_inw
  - address_space_lduw
    - address_space_ldl_internal
      - address_space_translate : 获取具体是在那个 memory region 是为了判断当前的读写发生在哪一个 memory region 上
        - flatview_translate : 参数 Flatview, 和 hwaddr 返回 MemoryRegion
            - flatview_do_translate : 其实没有什么奇怪的，这就是利用 AddressSpaceDispatch 的基础设施查询
              - address_space_translate_internal
                - phys_page_find : 这存在一个 cache, 当没有命中的时候，需要查询一波
      - memory_region_dispatch_read : 如果进行的是 mmio, 通过持有 MemoryRegions 可以很快的找到对应的空间
        - memory_region_dispatch_read1
          - access_with_adjusted_size
      - qemu_map_ram_ptr : 如果是 RAM 的访问就很容易


- kvm_handle_io
  - address_space_rw
    - address_space_read_full
      - address_space_to_flatview : 从 as 到 flatview
      - flatview_read
        - flatview_translate : 从 flatview 到 mr
        - flatview_read_continue : 会在这里区分到底是 MMIO 还是一般的, 之所以叫做 continue 是为了处理访问在多个连续的 memory region 的情况
          - memory_region_dispatch_read : 这里，现在所有人都相同了

kvm 的 style:
```c
/*
#0  pci_host_config_read_common (pci_dev=0x5555570d4000, addr=2147483648, limit=1439872976, len=21845) at ../hw/pci/pci_host.c:88
#1  0x0000555555a49a17 in pci_data_read (s=0x5555570d4000, addr=2147483648, len=2) at ../hw/pci/pci_host.c:133
#2  0x0000555555a49b51 in pci_host_data_read (opaque=0x555556c44270, addr=0, len=2) at ../hw/pci/pci_host.c:178
#3  0x0000555555ca681c in memory_region_read_accessor (mr=0x555556c44680, addr=0, value=0x7fffe890f060, size=2, shift=0, mask=65535, attrs=...) at ../softmmu/memory.c:4
40
#4  0x0000555555ca6d1c in access_with_adjusted_size (addr=0, value=0x7fffe890f060, size=2, access_size_min=1, access_size_max=4, access_fn=0x555555ca67d6 <memory_region
_read_accessor>, mr=0x555556c44680, attrs=...) at ../softmmu/memory.c:550
#5  0x0000555555ca9a38 in memory_region_dispatch_read1 (mr=0x555556c44680, addr=0, pval=0x7fffe890f060, size=2, attrs=...) at ../softmmu/memory.c:1427
#6  0x0000555555ca9b0e in memory_region_dispatch_read (mr=0x555556c44680, addr=0, pval=0x7fffe890f060, op=MO_16, attrs=...) at ../softmmu/memory.c:1455
#7  0x0000555555d31e77 in flatview_read_continue (fv=0x555556db8900, addr=3324, attrs=..., ptr=0x7fffeb180000, len=2, addr1=0, l=2, mr=0x555556c44680) at ../softmmu/phy
smem.c:2831
#8  0x0000555555d31fce in flatview_read (fv=0x555556db8900, addr=3324, attrs=..., buf=0x7fffeb180000, len=2) at ../softmmu/physmem.c:2870
#9  0x0000555555d3205b in address_space_read_full (as=0x5555567a6b00 <address_space_io>, addr=3324, attrs=..., buf=0x7fffeb180000, len=2) at ../softmmu/physmem.c:2883
#10 0x0000555555d32187 in address_space_rw (as=0x5555567a6b00 <address_space_io>, addr=3324, attrs=..., buf=0x7fffeb180000, len=2, is_write=false) at ../softmmu/physmem
```

tcg 的 style:
```c
/*
#0  pci_host_config_read_common (pci_dev=0x5555570c6c00, addr=2147483648, limit=1479011232, len=21845) at ../hw/pci/pci_host.c:88
#1  0x0000555555a49a17 in pci_data_read (s=0x5555570c6c00, addr=2147483648, len=2) at ../hw/pci/pci_host.c:133
#2  0x0000555555a49b51 in pci_host_data_read (opaque=0x555556c48e00, addr=0, len=2) at ../hw/pci/pci_host.c:178
#3  0x0000555555ca681c in memory_region_read_accessor (mr=0x555556c49210, addr=0, value=0x7fffe888db80, size=2, shift=0, mask=65535, attrs=...) at ../softmmu/memory.c:4
40
#4  0x0000555555ca6d1c in access_with_adjusted_size (addr=0, value=0x7fffe888db80, size=2, access_size_min=1, access_size_max=4, access_fn=0x555555ca67d6 <memory_region
_read_accessor>, mr=0x555556c49210, attrs=...) at ../softmmu/memory.c:550
#5  0x0000555555ca9a38 in memory_region_dispatch_read1 (mr=0x555556c49210, addr=0, pval=0x7fffe888db80, size=2, attrs=...) at ../softmmu/memory.c:1427
#6  0x0000555555ca9b0e in memory_region_dispatch_read (mr=0x555556c49210, addr=0, pval=0x7fffe888db80, op=MO_16, attrs=...) at ../softmmu/memory.c:1455
#7  0x0000555555d332d7 in address_space_lduw_internal (as=0x5555567a6b00 <address_space_io>, addr=3324, attrs=..., result=0x0, endian=DEVICE_NATIVE_ENDIAN) at /home/mar
itns3/core/kvmqemu/memory_ldst.c.inc:214
#8  0x0000555555d333cc in address_space_lduw (as=0x5555567a6b00 <address_space_io>, addr=3324, attrs=..., result=0x0) at /home/maritns3/core/kvmqemu/memory_ldst.c.inc:2
46
#9  0x0000555555b45861 in helper_inw (env=0x555556c94340, port=3324) at ../target/i386/tcg/sysemu/misc_helper.c:48
#10 0x00007fff540080ff in code_gen_buffer ()
```

tcg 的 style : io_readx
```c
/*
>>> bt
#0  flatview_read_continue (fv=0x0, addr=384, attrs=..., ptr=0x7fffe888d8b0, len=0, addr1=93825027792176, l=1, mr=0x0) at ../softmmu/physmem.c:2818
#1  0x0000555555d31fce in flatview_read (fv=0x7ffdcc4e0850, addr=4273946630, attrs=..., buf=0x7fffe888d8b0, len=1) at ../softmmu/physmem.c:2870
#2  0x0000555555d31306 in subpage_read (opaque=0x7ffdcc52f420, addr=6, data=0x7fffe888d908, len=1, attrs=...) at ../softmmu/physmem.c:2453
#3  0x0000555555ca692e in memory_region_read_with_attrs_accessor (mr=0x7ffdcc52f420, addr=6, value=0x7fffe888da78, size=1, shift=0, mask=255, attrs=...) at ../softmmu/m
emory.c:462
#4  0x0000555555ca6d1c in access_with_adjusted_size (addr=6, value=0x7fffe888da78, size=1, access_size_min=1, access_size_max=8, access_fn=0x555555ca68ca <memory_region
_read_with_attrs_accessor>, mr=0x7ffdcc52f420, attrs=...) at ../softmmu/memory.c:550
#5  0x0000555555ca9a76 in memory_region_dispatch_read1 (mr=0x7ffdcc52f420, addr=6, pval=0x7fffe888da78, size=1, attrs=...) at ../softmmu/memory.c:1433
#6  0x0000555555ca9b0e in memory_region_dispatch_read (mr=0x7ffdcc52f420, addr=6, pval=0x7fffe888da78, op=MO_8, attrs=...) at ../softmmu/memory.c:1455
#7  0x0000555555c6cb1b in io_readx (env=0x555556d66880, iotlbentry=0x7ffdcc01d6b0, mmu_idx=2, addr=4273946630, retaddr=140734603597128, access_type=MMU_DATA_LOAD, op=MO
_8) at ../accel/tcg/cputlb.c:1359
#8  0x0000555555c6dfb9 in load_helper (env=0x555556d66880, addr=4273946630, oi=2, retaddr=140734603597128, op=MO_8, code_read=false, full_load=0x555555c6e19c <full_ldub
_mmu>) at ../accel/tcg/cputlb.c:1914
#9  0x0000555555c6e1e6 in full_ldub_mmu (env=0x555556d66880, addr=4273946630, oi=2, retaddr=140734603597128) at ../accel/tcg/cputlb.c:1972
#10 0x0000555555c6e21e in helper_ret_ldub_mmu (env=0x555556d66880, addr=4273946630, oi=2, retaddr=140734603597128) at ../accel/tcg/cputlb.c:1978
```

在 [dam](#dma) 中，还有一个类似 backtrace, 其实总是到达 memory_region_dispatch_read, 而到达之前总是通过各种方法获取
mr 而已，在 kvm_handle_io 中经过了 as 到 flatview 再到 mr 的过程，在 io_readx 中几乎立刻到达，这是因为 iotlb 存储了一个地址对应的 mr

## dispatch 的过程(2) : memory_region_dispatch_read
各种场景到达 memory_region_dispatch_read 的时候，总是会进行一个 memory_access_is_direct 的检查，否则就会进入到
qemu_map_ram_ptr 的计算中, 也就是说，memory_region_dispatch_read 总是在处理 pio / mmio

#### endianness
memory_region_dispatch_read 在最后会调用 adjust_endianness
而 memory_region_dispatch_write 会在开始的时候调用

目前只有一个 device 是 big endianness 的，那就是 fwcfg.dma
- hw/nvram/fw_cfg.c 中 fw_cfg_dma_mem_ops 和 fw_cfg_comb_mem_ops 的确如此定义
- 从 qemu_cfg_dma_transfer 中也可以找到证据

```c
/*
0000000000000510-0000000000000511 (prio 0, i/o): fwcfg
0000000000000514-000000000000051b (prio 0, i/o): fwcfg.dma
```

```c
static void
qemu_cfg_dma_transfer(void *address, u32 length, u32 control)
{
    QemuCfgDmaAccess access;

    access.address = cpu_to_be64((u64)(u32)address);
    access.length = cpu_to_be32(length);
    access.control = cpu_to_be32(control);

    barrier();

    outl(cpu_to_be32((u32)&access), PORT_QEMU_CFG_DMA_ADDR_LOW);

    while(be32_to_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}
```

#### access_size
access_with_adjusted_size 会计算调用的大小，实际上，最终将大小约束到 1 - 4 之间, 如果需要进行的 io 的大小超过这个 4, 那么就使用循环反复调用 MemoryRegionOps::read

所以，真的会出现 pio/mmio 的 size > 4 的情况吗, 实际测试显示，只有 vga-lowmem 会是如此。

MemoryRegionOps::read 的参数是有 size 的

## FlatRange 和 MemoryRegionSection
- section_from_flat_range : 很简单的封装

```c
static inline MemoryRegionSection section_from_flat_range(FlatRange *fr, FlatView *fv)
{
    return (MemoryRegionSection) {
        .mr = fr->mr,
        .fv = fv,
        .offset_within_region = fr->offset_in_region,
        .size = fr->addr.size,
        .offset_within_address_space = int128_get64(fr->addr.start),
        .readonly = fr->readonly,
        .nonvolatile = fr->nonvolatile,
    };
}
```
实际上，这个函数的调用者几乎就是 memory listener 了

##  kvmtool
无论是 pio 还是 mmio，传输数据都是进行传输都是 byte 级别的，所以
ioport__register 就可以了

而至于内存分配，使用 kvm__init_ram ，考虑一下 pci_hole 就差不多了

[^1]: https://www.anquanke.com/post/id/86412
[^3]: https://wiki.osdev.org/System_Management_Mode
[^4]: https://www.linux-kvm.org/images/1/17/Kvm-forum-2013-Effective-multithreading-in-QEMU.pdf
[^5]: https://terenceli.github.io/%E6%8A%80%E6%9C%AF/2018/08/11/dirty-pages-tracking-in-migration
[^6]: https://wiki.qemu.org/Features/VT-d
[^8]: [official doc](https://qemu.readthedocs.io/en/latest/devel/memory.html)
