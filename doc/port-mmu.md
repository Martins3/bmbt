# softmmu 和 memory model 的移植的设计

## 设计
- CPUAddressSpace 需要创建出来两个 AddressSpace 来
  - 但是 AddressSpace 共同的持有
  - 将两个 AddressSpace 和 smram smram_region 共同合作的，来确定 vag-low 还是 ram

- 需要 MemoryRegion，但是不需要 MemoryRegionSection，因为 MemoryRegion 中间不会被划分，MemoryRegion 就是最小的对象
- 一个 MemoryRegion 关联一个 RAMBlock，MemoryRegion 和 RAMBlock 的大小完全相同

1. segments 直接初始化为固定大小的数组
2. 所有的 MemoryRegion 在按照顺序排放
3. 使用二分查找来找到 MemoryRegion
4. 以后增加一个 mru 的 cache 维持生活这个样子的

- 对于 SMM 的处理，首先判断是否命中 SMM 空间
  - 常规路径
  - 查看是否是 SMM 的那个 MemoryRegion 的，如果是，再看 attrs 的结果

- 对于 0xfffff 下面的那些 memory region 都是直接静态的定义下来的

- 修改那些位置是需要进行 tcg_commit 的
  - 修改 AddressSpace::segments 的时候，也就是 io_add_memory_region 和 mem_add_memory_region
  - smm / pam 修改映射关系的时候，对应原来的代码  memory_region_set_enabled 会调用 memory_region_transaction_commit 的情况啊!

## TODO
- [ ] 处理一下各个 submodule 注册 memory region，比如 fw_cfg

- [ ] 确认一下 : SMM 状态的转换会删除掉所有的 TLB，不然 iotlb_to_section 还在使用第一次 TLB miss 注册的 attrs
- [ ] 现在将很多 MemoryRegion 都切碎了，可能在原来的 QEMU 中是没有越界的，但是现在出现了越界
  - 只要在 MemoryRegion generated topology 不要让其

## 重写的函数接口
| function                          | 作用                                                                                                          |
|-----------------------------------|---------------------------------------------------------------------------------------------------------------|
| address_space_translate_for_iotlb | 根据 addr 得到 memory region 的                                                                               |
| memory_region_section_get_iotlb   | 计算出来当前的 section 是 AddressSpaceDispatch 中的第几个 section, 之后就可以通过 addr 获取 section 了        |
| qemu_map_ram_ptr                  | 这是一个神仙设计的接口，如果参数 ram_block 的接口为 NULL, 那么 addr 是 ram addr， 如果不是，那么是 ram 内偏移 |
| cpu_addressspace                  |                                                                                                               |
| iotlb_to_section                  |                                                                                                               |

#### 真的需要移除掉 iotlb 机制吗
而 IOTLB 的 MMIO 移除掉，让 io_readx 和 io_writex 中直接走 memory region translate 的操作

需要修改的内容，将其尽可能变为空的:
- memory_region_section_get_iotlb
- address_space_translate_for_iotlb : 只是进行 RAM 装换就好了，那么这些就是
- iotlb_to_section

#### address_space_translate_for_iotlb 和 address_space_translate 的关系
- address_space_translate
  - address_space_to_flatview : 获取 Flatview
  - flatview_translate : 返回 MemoryRegionSection
    - flatview_do_translation
      - flatview_to_dispatch : 从 Flatview 获取 dispatch
        - address_space_translate_internal
    - 从 MemoryRegionSection 获取 mr

- address_space_translate_for_iotlb
  - `atomic_rcu_read(&cpu->cpu_ases[asidx].memory_dispatch)` : 直接获取 dispatch
  - address_space_translate_internal

| function                          | para                                            | res                       |
|-----------------------------------|-------------------------------------------------|---------------------------|
| address_space_translate_for_iotlb | [cpu asidx](获取地址) addr [attrs prot](没用的) | MemoryRegionSection, xlat |
| address_space_translate           | as addr [is_write attrs](没用的)                       | MemoryRegion, xlat, len   |
maddress_space_translate_internal  | d, addr, resolve_subpage                        | xlat, plen                |


## 到底那些地方可以简化
- stl_le_phys 族的函数只是需要这一个

- memory_ldst.h 无需考虑 `#define SUFFIX                   _cached_slow`, 那是给 virtio 使用的
- 几乎无需处理 endianness 的问题

1. 一个 CPUAddressSpace 持有一个 AddressSpace
2. 在 AddressSpace 中持有一个数组(因为 iotlb_to_section 需要的)
  - iotlb_to_section 是一定需要的，从 store_helper 中直接判断，那才是最重要的入口

3. 暂时直接在这个数组上移动就可以了
  - 对于 1M 直接判断，然后就是 RAM 或者 PCI 了，而 IO 和 memory 早就可以区分
  - 如果 PAM 打开，不需要存在一棵树的

## 处理 SMM
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
