# memory model

在 https://github.com/kernelrookie/DuckBuBi/issues/35 中，
分析 `address_space_*` 以及如何检查 memory_ldst.inc.c 和
memory_ldst.inc.h 的方法。

在 v6.0 中
| file             | desc                                                            |
|------------------|-----------------------------------------------------------------|
| softmmu/memory.c | memory_region_dispatch_read 之类的各种 memory region 的管理工作 |
| softmmu/physmem  | RAMBlock 之类的管理                                             |


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
关注下面两个函数，可以发现
1. 创建的两个分别为普通模式和 SMM 模式
2. 这两个 AddressSpace 只有在 TCG 模式下才会有
- x86_cpu_realizefn
  - cpu_address_space_init

```c
static MemoryRegion *system_memory;
static MemoryRegion *system_io;

AddressSpace address_space_io;
AddressSpace address_space_memory;
```
这两个 AddressSpace 的初始位置，都是在 memory_map_init, 将其和 system_memory 和 system_io 联系起来
而之后的一些列初始化和内容的填充都是通过这两个 MemoryRegion 完成的。

- [ ] 暂时打住一下

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
      - [ ] memory_region_get_flatview_root : 我看不懂这是在表达什么东西，在从上向下的查找一个 MemoryRegions 来作为 root ?
      - generate_memory_topology
        - render_memory_region
        - flatview_simplify
        - address_space_dispatch_new : 初始化 Flatview::dispatch

> info mtree [-f][-d][-o][-D] -- show memory tree (-f: dump flat view for address spaces;-d: dump dispatch tree, valid with -f only);-o: dump region owners/parents;-D: dump disabled regions

#### dispatch 的过程
- [ ] 如果通过 Flatview 来构建来 dispatch read/write ?
- [ ] 这里存在一个非常尴尬的事情，从 memory_ldst.c 的 address_space_stl 调用的时候都是物理地址啊
  - 从操作系统的角度，进行 IO 也是经过了自己的 TLB 翻译的之后，才得到物理地址的啊，之后这个地址才会发给地址总线
  - [ ] IO 也需要从 softmmu 中翻译，找到对应的代码验证一下

从 memory_ldst 中的 address_space_translate 到 phys_page_find 的:
- 在 memory_ldst.c 的经典调用方法:
  - address_space_translate 获取 mr
    - [ ] 为什么需要从 as 获取 as
    - address_space_translate : translate an address range into an address space into a MemoryRegion and an address range into that section.
      - flatview_translate : 将 MemoryRegionSection::mr 返回
        - flatview_do_translate : 返回 MemoryRegionSection
          - address_space_translate_internal
            - phys_page_find : 当没有命中的时候，需要查询一波
  - memory_region_dispatch_read 进行 IO

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

#### Flatview
- generate_memory_topology : Render a memory topology into a list of disjoint absolute ranges.
  - render_memory_region : 具体分析可以参考一个让人更加难受的 blog https://blog.csdn.net/sinat_38205774/article/details/104312303
    - flatview_insert

 
#### [ ] 神奇的 memory_region_get_flatview_root
- memory_region_get_flatview_root 到底发挥什么作用?

只有一个例子，那就是 PCI device 中:

mr = system
ori = bus master container

```
address-space: e1000
  0000000000000000-ffffffffffffffff (prio 0, i/o): bus master container
    0000000000000000-ffffffffffffffff (prio 0, i/o): alias bus master @system 0000000000000000-ffffffffffffffff
```
- 定义出来的这个玩意儿有啥用啊，如何访问 PCIe 空间啊
  - PCIe 空间是放到 system memory 的
  - 定义这个用于和 system 进行 DMA 的

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

-  do_pci_register_device : PCI 设备是如何构建自己的 address_space 的
   - `address_space_init(&pci_dev->bus_master_as, &pci_dev->bus_master_container_region, pci_dev->name);`

#### 举个例子分析 alias
machine_run_board_init 中初始化 `machine->ram`, 也就是 pc.ram 这个 memory region

而分析 system 这个 memory region, 发现其中的两个 subregion ram-below-4g 和 ram-above-4g 都是
是 pc.ram 的 alias.

也即是一个 memory region 的 subregion 可以是其他的

```c
/*
address-space: memory
  0000000000000000-ffffffffffffffff (prio 0, i/o): system
    0000000000000000-00000000bfffffff (prio 0, ram): alias ram-below-4g @pc.ram 0000000000000000-00000000bfffffff
    0000000000000000-ffffffffffffffff (prio -1, i/o): pci
      00000000000a0000-00000000000bffff (prio 1, i/o): vga-lowmem
      00000000000c0000-00000000000dffff (prio 1, rom): pc.rom
      00000000000e0000-00000000000fffff (prio 1, rom): alias isa-bios @pc.bios 0000000000020000-000000000003ffff
      00000000fe000000-00000000fe7fffff (prio 1, ram): vga.vram
      00000000fe800000-00000000fe803fff (prio 1, i/o): virtio-pci
        00000000fe800400-00000000fe80041f (prio 0, i/o): vga ioports remapped
        00000000fe800500-00000000fe800515 (prio 0, i/o): bochs dispi interface
        00000000fe800600-00000000fe800607 (prio 0, i/o): qemu extended regs
        00000000fe801000-00000000fe8017ff (prio 0, i/o): virtio-pci-common-virtio-gpu
        00000000fe801800-00000000fe801fff (prio 0, i/o): virtio-pci-isr-virtio-gpu
        00000000fe802000-00000000fe802fff (prio 0, i/o): virtio-pci-device-virtio-gpu
        00000000fe803000-00000000fe803fff (prio 0, i/o): virtio-pci-notify-virtio-gpu
      00000000fe804000-00000000fe807fff (prio 1, i/o): virtio-pci
        00000000fe804000-00000000fe804fff (prio 0, i/o): virtio-pci-common-virtio-9p
        00000000fe805000-00000000fe805fff (prio 0, i/o): virtio-pci-isr-virtio-9p
        00000000fe806000-00000000fe806fff (prio 0, i/o): virtio-pci-device-virtio-9p
        00000000fe807000-00000000fe807fff (prio 0, i/o): virtio-pci-notify-virtio-9p
      00000000febc0000-00000000febdffff (prio 1, i/o): e1000-mmio
      00000000febf0000-00000000febf3fff (prio 1, i/o): nvme-bar0
        00000000febf0000-00000000febf1fff (prio 0, i/o): nvme
        00000000febf2000-00000000febf240f (prio 0, i/o): msix-table
        00000000febf3000-00000000febf300f (prio 0, i/o): msix-pba
      00000000febf4000-00000000febf4fff (prio 1, i/o): virtio-vga-msix
        00000000febf4000-00000000febf402f (prio 0, i/o): msix-table
        00000000febf4800-00000000febf4807 (prio 0, i/o): msix-pba
      00000000febf5000-00000000febf5fff (prio 1, i/o): virtio-9p-pci-msix
        00000000febf5000-00000000febf501f (prio 0, i/o): msix-table
        00000000febf5800-00000000febf5807 (prio 0, i/o): msix-pba
      00000000fffc0000-00000000ffffffff (prio 0, rom): pc.bios
    00000000000a0000-00000000000bffff (prio 1, i/o): alias smram-region @pci 00000000000a0000-00000000000bffff
    00000000000c0000-00000000000c3fff (prio 1, ram): alias pam-rom @pc.ram 00000000000c0000-00000000000c3fff
    00000000000c4000-00000000000c7fff (prio 1, ram): alias pam-rom @pc.ram 00000000000c4000-00000000000c7fff
    00000000000c8000-00000000000cbfff (prio 1, ram): alias pam-rom @pc.ram 00000000000c8000-00000000000cbfff
    00000000000cb000-00000000000cdfff (prio 1000, ram): alias kvmvapic-rom @pc.ram 00000000000cb000-00000000000cdfff
    00000000000cc000-00000000000cffff (prio 1, ram): alias pam-rom @pc.ram 00000000000cc000-00000000000cffff
    00000000000d0000-00000000000d3fff (prio 1, ram): alias pam-rom @pc.ram 00000000000d0000-00000000000d3fff
    00000000000d4000-00000000000d7fff (prio 1, ram): alias pam-rom @pc.ram 00000000000d4000-00000000000d7fff
    00000000000d8000-00000000000dbfff (prio 1, ram): alias pam-rom @pc.ram 00000000000d8000-00000000000dbfff
    00000000000dc000-00000000000dffff (prio 1, ram): alias pam-rom @pc.ram 00000000000dc000-00000000000dffff
    00000000000e0000-00000000000e3fff (prio 1, ram): alias pam-rom @pc.ram 00000000000e0000-00000000000e3fff
    00000000000e4000-00000000000e7fff (prio 1, ram): alias pam-ram @pc.ram 00000000000e4000-00000000000e7fff
    00000000000e8000-00000000000ebfff (prio 1, ram): alias pam-ram @pc.ram 00000000000e8000-00000000000ebfff
    00000000000ec000-00000000000effff (prio 1, ram): alias pam-ram @pc.ram 00000000000ec000-00000000000effff
    00000000000f0000-00000000000fffff (prio 1, ram): alias pam-rom @pc.ram 00000000000f0000-00000000000fffff
    00000000fec00000-00000000fec00fff (prio 0, i/o): kvm-ioapic
    00000000fed00000-00000000fed003ff (prio 0, i/o): hpet
    00000000fee00000-00000000feefffff (prio 4096, i/o): kvm-apic-msi
    0000000100000000-00000001bfffffff (prio 0, ram): alias ram-above-4g @pc.ram 00000000c0000000-000000017fffffff

memory-region: pc.ram
  0000000000000000-000000017fffffff (prio 0, ram): pc.ram
```

#### [official doc](https://qemu.readthedocs.io/en/latest/devel/memory.html)
In addition to MemoryRegion objects, the memory API provides AddressSpace objects for every root and possibly for intermediate MemoryRegions too. These represent memory as seen from the CPU or a device’s viewpoint.
- [ ] 一个 bus 为什么需要自己的视角啊

> For example, a PCI BAR may be composed of a RAM region and an MMIO region.
- [ ] 什么意思 ?

#### TODO
- [ ] 既然 flatview 计算好了，那么按照道理来说，就可以直接注册，结果每次 mmio，路径那么深
  - [ ] 一种可能，那就是，这空间是动态分配的
    - [ ] 似乎不是这个原因

- [ ] 为什么需要设计出来 container ?
  - [ ] alias: a subsection of another region. 
```c
static hwaddr memory_region_to_absolute_addr(MemoryRegion *mr, hwaddr offset)
{
    MemoryRegion *root;
    hwaddr abs_addr = offset;

    abs_addr += mr->addr;
    for (root = mr; root->container; ) {
        root = root->container;
        abs_addr += root->addr;
    }

    return abs_addr;
}
```

- [ ] memory_region_add_subregion

举个例子:
```
0000000000000600-000000000000063f (prio 0, i/o): piix4-pm
```

```
>>> p mr->ops->write
$4 = (void (*)(void *, hwaddr, uint64_t, unsigned int)) 0x555555880a20 <acpi_pm_cnt_write>
>>> p mr->container->name
$5 = 0x5555569b4040 "piix4-pm"
>>> p mr->name
$6 = 0x555556dd37d0 "acpi-cnt"
>>> p/x mr->container->addr
$8 = 0x600
```

- [ ] 之所以设计出来 Flatview 和 AddressSpace 树状的结构，难道不是主要因为地址空间的相互重合问题吗，找到一个相互重叠的例子

#### QEMU内存虚拟化源码分析[^1]
首先，qemu中用AddressSpace用来表示CPU/设备看到的内存，一个AddressSpace下面包含多个MemoryRegion，这些MemoryRegion结构通过树连接起来，树的根是AddressSpace的root域。

MemoryRegion有多种类型，可以表示一段ram，rom，MMIO，alias，alias表示一个MemoryRegion的一部分区域，MemoryRegion也可以表示一个container，这就表示它只是其他若干个MemoryRegion的容器。在MemoryRegion中，'ram_block'表示的是分配的实际内存。

AddressSpace下面root及其子树形成了一个虚拟机的物理地址，但是在往kvm进行设置的时候，需要将其转换为一个平坦的地址模型，也就是从0开始的。这个就用FlatView表示，**一个AddressSpace对应一个FlatView**。

在 FlatView 中，FlatRange表示按照需要被切分为了几个范围。
在内存虚拟化中，还有一个重要的结构是MemoryRegionSection，这个结构通过函数 section_from_flat_range 可由 FlatRange 转换过来。

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
> woc, 这简直就是离谱，就是一个简单的拼装啊!

mr 很多时候是创建一个 alias，指向已经存在的 mr 的一部分，这也是 alias 的作用

*继续 pc_memory_init，函数在创建好了 ram 并且分配好了空间之后，创建了两个mr alias，ram_below_4g以及ram_above_4g，这两个mr分别指向ram的低4g以及高4g空间，这两个alias是挂在根system_memory mr下面的。*

> - [ ] 这个结构很难理解啊，即是一个 memory region 的 subregion，又是另一个 region 的 alias

为了在虚拟机退出时，能够顺利根据物理地址找到对应的 HVA 地址，qemu 会有一个 AddressSpaceDispatch 结构，用来在 AddressSpace 中进行位置的找寻，继而完成对IO/MMIO地址的访问。
> 其实不是获取 HVA，而是通过 GPA 获取到对应的 dispatch 函数

为了监控虚拟机的物理地址访问，对于每一个AddressSpace，会有一个MemoryListener与之对应。每当物理映射（`GPA->HVA`)发生改变时，会回调这些函数。

在上面看到MemoryListener之后，我们看看什么时候需要更新内存。 进行内存更新有很多个点，比如我们新创建了一个AddressSpace address_space_init，再比如我们将一个mr添加到另一个mr的subregions中memory_region_add_subregion,再比如我们更改了一端内存的属性memory_region_set_readonly，将一个mr设置使能或者非使能memory_region_set_enabled, 总之一句话，我们修改了虚拟机的内存布局/属性时，就需要通知到各个Listener，这包括各个AddressSpace对应的，以及kvm注册的，这个过程叫做commit，通过函数memory_region_transaction_commit实现。

进行内存更新有很多个点，比如我们新创建了一个AddressSpace address_space_init，再比如我们将一个mr添加到另一个mr的subregions中memory_region_add_subregion,再比如我们更改了一端内存的属性memory_region_set_readonly，将一个mr设置使能或者非使能memory_region_set_enabled, 总之一句话，我们修改了虚拟机的内存布局/属性时，就需要通知到各个Listener，这包括各个AddressSpace对应的，以及kvm注册的，这个过程叫做commit，通过函数memory_region_transaction_commit实现。

## [^2]
- [ ] TODO

## memory listener
- [x] 这个玩意儿到底做啥的
  -  用来监听 `GPA->HVA` 的改变

- [ ] 如果一边正在修改映射关系，一边在进行 IO，怎么办?

- [ ] 最经典的位置应该在于 PCI 设备的初始化

```c
/*
#0  memory_listener_register (listener=0x55555582e460 <_start>, as=0x7fffffffcfc0) at ../softmmu/memory.c:2777
#1  0x0000555555d2ce67 in cpu_address_space_init (cpu=0x555556c8ac00, asidx=0, prefix=0x555556092140 "cpu-memory", mr=0x555556a85900) at ../softmmu/physmem.c:767
#2  0x0000555555b41d1d in tcg_cpu_realizefn (cs=0x555556c8ac00, errp=0x7fffffffd070) at ../target/i386/tcg/sysemu/tcg-cpu.c:76
#3  0x0000555555c849c0 in accel_cpu_realizefn (cpu=0x555556c8ac00, errp=0x7fffffffd070) at ../accel/accel-common.c:119
#4  0x0000555555c67ed9 in cpu_exec_realizefn (cpu=0x555556c8ac00, errp=0x7fffffffd070) at ../cpu.c:136
#5  0x0000555555ba4f37 in x86_cpu_realizefn (dev=0x555556c8ac00, errp=0x7fffffffd0f0) at ../target/i386/cpu.c:6139
#6  0x0000555555e7e012 in device_set_realized (obj=0x555556c8ac00, value=true, errp=0x7fffffffd1f8) at ../hw/core/qdev.c:761
#7  0x0000555555e68ee6 in property_set_bool (obj=0x555556c8ac00, v=0x555556bae8b0, name=0x555556128bb9 "realized", opaque=0x55555689a7a0, errp=0x7fffffffd1f8) at ../qom
/object.c:2257
#8  0x0000555555e66f07 in object_property_set (obj=0x555556c8ac00, name=0x555556128bb9 "realized", v=0x555556bae8b0, errp=0x5555567a94b0 <error_fatal>) at ../qom/object
.c:1402
#9  0x0000555555e63789 in object_property_set_qobject (obj=0x555556c8ac00, name=0x555556128bb9 "realized", value=0x555556b8cd40, errp=0x5555567a94b0 <error_fatal>) at .
./qom/qom-qobject.c:28
#10 0x0000555555e6727f in object_property_set_bool (obj=0x555556c8ac00, name=0x555556128bb9 "realized", value=true, errp=0x5555567a94b0 <error_fatal>) at ../qom/object.
c:1472
#11 0x0000555555e7d032 in qdev_realize (dev=0x555556c8ac00, bus=0x0, errp=0x5555567a94b0 <error_fatal>) at ../hw/core/qdev.c:389
#12 0x0000555555b673b2 in x86_cpu_new (x86ms=0x555556a37000, apic_id=0, errp=0x5555567a94b0 <error_fatal>) at ../hw/i386/x86.c:111
#13 0x0000555555b67485 in x86_cpus_init (x86ms=0x555556a37000, default_cpu_version=1) at ../hw/i386/x86.c:138
#14 0x0000555555b7b69b in pc_init1 (machine=0x555556a37000, host_type=0x55555609e70a "i440FX-pcihost", pci_type=0x55555609e703 "i440FX") at ../hw/i386/pc_piix.c:157
#15 0x0000555555b7c24e in pc_init_v6_1 (machine=0x555556a37000) at ../hw/i386/pc_piix.c:425
#16 0x0000555555aec313 in machine_run_board_init (machine=0x555556a37000) at ../hw/core/machine.c:1239
#17 0x0000555555cdada6 in qemu_init_board () at ../softmmu/vl.c:2526
#18 0x0000555555cdaf85 in qmp_x_exit_preconfig (errp=0x5555567a94b0 <error_fatal>) at ../softmmu/vl.c:2600
#19 0x0000555555cdd65d in qemu_init (argc=25, argv=0x7fffffffd7a8, envp=0x7fffffffd878) at ../softmmu/vl.c:3635
#20 0x000055555582e575 in main (argc=25, argv=0x7fffffffd7a8, envp=0x7fffffffd878) at ../softmmu/main.c:49
```

## TCG 和 SMM
SMM 实际上是给 firmware 使用的

在 tcg_cpu_realizefn 中

- get_system_memory : 获取的 MemoryRegion 的名称为 system, 总会挂到 cpu-memory-0 / cpu-memory-2 上

```
address-space: tcg-cpu-smm-0
  0000000000000000-ffffffffffffffff (prio 0, i/o): cpu_as_root-memory
    0000000000000000-00000000ffffffff (prio 1, i/o): alias smram @smram 0000000000000000-00000000ffffffff
    0000000000000000-ffffffffffffffff (prio 0, i/o): alias cpu_as_mem-memory @system 0000000000000000-ffffffffffffffff
```

通过 qemu_add_machine_init_done_notifier 调用
在 tcg_cpu_machine_done 中，从而在 cpu_as_root-memory 下创建一个 smram


而在 i440fx_init 中，创建出来了 smram
```
memory-region: smram
  0000000000000000-00000000ffffffff (prio 0, i/o): smram
    00000000000a0000-00000000000bffff (prio 0, ram): alias smram-low @pc.ram 00000000000a0000-00000000000bffff
```


[^1]: https://www.anquanke.com/post/id/86412
[^2]: https://oenhan.com/qemu-memory-struct
[^3]: https://wiki.osdev.org/System_Management_Mode
