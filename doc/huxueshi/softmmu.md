# qemu softmmu 设计

## 基本调用关系
- tlb_fill
  - x86_cpu_tlb_fill
    - handle_mmu_fault : 利用 x86 的页面进行 page walk
      - tlb_set_page_with_attrs : 设置页面

- [ ] 所以现在的 hamt 设计，在 tlb_fill 在 tlb refill 的位置进行修改，tlb_set_page_with_attrs 修改为真正的 tlbwr 之类的东西

下面是 tlb_fill 的几个调用者
- get_page_addr_code : 从 guest 虚拟地址的 pc 获取 guest 物理地址的 pc
  - get_page_addr_code_hostp
    - qemu_ram_addr_from_host_nofail
      - qemu_ram_addr_from_host
        - qemu_ram_block_from_host

## CPUIOTLBEntry
使用者:
- probe_access
  - tlb_hit / victim_tlb_hit / tlb_fill : TLB 访问经典三件套
  - cpu_check_watchpoint
  - notdirty_write : 和 cpu_check_watchpoint 相同，需要 iotlbentry 作为参数
- io_writex / io_readx
  - iotlb_to_section
  - `mr_offset = (iotlbentry->addr & TARGET_PAGE_MASK) + addr;`

在 exec 中间，维护了 MemoryRegionSection (具体参考 iotlb_to_section)，这让 iotlbentry 可以找到 IO 命中的 memory_region
从而知道这个 memory_region 的对应的处理函数是什么。

维护:
  tlb_set_page_with_attrs

```c
    /* refill the tlb */
    /*
     * At this point iotlb contains a physical section number in the lower
     * TARGET_PAGE_BITS, and either
     *  + the ram_addr_t of the page base of the target RAM (RAM)
     *  + the offset within section->mr of the page base (I/O, ROMD)
     * We subtract the vaddr_page (which is page aligned and thus won't
     * disturb the low bits) to give an offset which can be added to the
     * (non-page-aligned) vaddr of the eventual memory access to get
     * the MemoryRegion offset for the access. Note that the vaddr we
     * subtract here is that of the page base, and not the same as the
     * vaddr we add back in io_readx()/io_writex()/get_page_addr_code().
     */
    desc->iotlb[index].addr = iotlb - vaddr_page;
    desc->iotlb[index].attrs = attrs;
```

## CPUTLBEntry 需要三个 addr
在 struct CPUTLBEntry 中，我们发现:
- addr_write
- addr_code
- addr_read
```c
typedef struct CPUTLBEntry {
    /* bit TARGET_LONG_BITS to TARGET_PAGE_BITS : virtual address
       bit TARGET_PAGE_BITS-1..4  : Nonzero for accesses that should not
                                    go directly to ram.
       bit 3                      : indicates that the entry is invalid
       bit 2..0                   : zero
    */
    union {
        struct {
            target_ulong addr_read;
            target_ulong addr_write;
            target_ulong addr_code;
            /* Addend to virtual address to get host address.  IO accesses
               use the corresponding iotlb value.  */
            uintptr_t addend;
        };
        /* padding to get a power of two size */
        uint8_t dummy[1 << CPU_TLB_ENTRY_BITS];
    };
} CPUTLBEntry;
```
这是为了在生成 TLB 对比的指令中消除掉其中的关于权限对比的部分。

分析一手 addr_read / addr_write / addr_code 的调用位置:
1. 三者都是仅仅出现在 cputlb.c 中间
2. tlb_reset_dirty_range_locked : 产生一个很有意思的问题，那就是通过设置 addr_write 表示的确可写，通过设置 TLB_NOTDIRTY 实现写保护。这样做的好处是，可以区分一个页面到底是真的不可写还是因为模拟的原因不可写。
  - 如果使用上 hardware TLB，其实可以这么处理: 如果想要给 TLB 设置上 addr_write，那么就写权限去掉，当因为 write 失败，可以捕获下这个异常，然后查询 x86 page table 来看，到底是因为 SMC 的原因还是因为 guest 本身的不可写
3. tlb_set_dirty1_locked

## CPUIOTLBEntry
```c
/* The IOTLB is not accessed directly inline by generated TCG code,
 * so the CPUIOTLBEntry layout is not as critical as that of the
 * CPUTLBEntry. (This is also why we don't want to combine the two
 * structs into one.)
 */
typedef struct CPUIOTLBEntry {
    /*
     * @addr contains:
     *  - in the lower TARGET_PAGE_BITS, a physical section number
     *  - with the lower TARGET_PAGE_BITS masked off, an offset which
     *    must be added to the virtual address to obtain:
     *     + the ram_addr_t of the target RAM (if the physical section
     *       number is PHYS_SECTION_NOTDIRTY or PHYS_SECTION_ROM)
     *     + the offset within the target MemoryRegion (otherwise)
     */
    hwaddr addr;
    MemTxAttrs attrs;
} CPUIOTLBEntry;
```
- [ ] 什么叫做 physical section number ?
- [ ] PHYS_SECTION_NOTDIRTY
- [ ] PHYS_SECTION_ROM

- [ ] tlb_set_page_with_attrs : 在这里似乎没有看到正常的 tlb refill 啊

## MemTxAttrs
在 `x86_*_phys` 和 helper_outb 都是通过 cpu_get_mem_attrs 来构建参数 MemTxAttrs

从目前看，MemTxAttrs 的主要作用是为了 SMM 模式，完全可以简化。

- [ ] 简化的事情，以后在分析，等待可以编译的之后

## TLB 结构
include/exec/cpu-defs.h

- [ ] 为什么 neg 一定需要放到 CPUArchState 前面
```c
/*
 * This structure must be placed in ArchCPU immediately
 * before CPUArchState, as a field named "neg".
 */
typedef struct CPUNegativeOffsetState {
    CPUTLB tlb;
    IcountDecr icount_decr;
} CPUNegativeOffsetState;

struct X86CPU {
    /*< private >*/
    CPUState parent_obj;
    /*< public >*/

    CPUNegativeOffsetState neg;
    CPUX86State env;
    // ...
}
```

- CPUTLB
  - CPUTLBCommon : 统计数据
  - CPUTLBDesc : 
    - victim tlb
    - large page
    - iotlb
  - CPUTLBDescFast : 通过 `tlb_entry` 实现访问，这就是常规的 TLB
    - mask
    - table : 需要的 page table

> IOTLB的表项和上面的 CPUTLBEntry 分开也是一个需要理解的点。定义里有一些注释，要结合代码才会完全理解。一个IO访问，它的地址匹配依然是通过CPUTLBEntry的地址来完成，但是由于IO访问时CPUTLBEntry相关地址的低位不为0（如果它已经被填充了的话），所以地址不会匹配成功，访存代码会走slow path。

## WatchPoint
- [ ] 如何实现?
  - 主，不在乎

## softmmu 的 fast path 和 slow path
```c
/*
 * Since the addressing of x86 architecture is complex, we
 * remain the original processing that exacts the final
 * x86vaddr from IR1_OPND(x86 opnd).
 * It is usually done by functions load_ireg_from_ir1_mem()
 * and store_ireg_from_ir1_mem().
```
- gen_ldst_softmmu_helper
  - `__gen_ldst_softmmu_helper_native` : 其中的注释非常清晰，首先查询 TLB，如果不成功，进入慢路径

#### fast path

#### slot path
- tr_ir2_generate
  - tr_gen_tb_start
  - tr_gen_softmmu_slow_path : slow path 的代码在每一个 tb 哪里只会生成一次
  - tr_gen_tb_end 

## 问题
- [ ] 我们会模拟 hugetlb 之类的操作吗 ?
