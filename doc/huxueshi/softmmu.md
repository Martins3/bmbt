# qemu softmmu 设计

因为设想首先完成在用户态的测试，然后进行在 QEMU 虚拟机测试，最后在物理机上添加设备, 所以这个阶段还是需要完成 softmmu 的移植的。

但是，但是 cputlb.c 中的代码存在过多的外部依赖，希望可以搞清楚设计之后再去分析。

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

- atomic_mmu_lookup
- load_helper
- store_helper


## 问题
- [ ] 找到 load / store 的 softmmu 命中的过程
- [ ] 我们会模拟 hugetlb 之类的操作吗 ?

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
- [ ] 为什么 CPUTLBEntry 需要三个 addr
  - addr_write
  - addr_code
  - addr_read

## softmmu 的 fast path 和 slow path

#### fast path

#### slot path
- tr_ir2_generate
  - tr_gen_tb_start
  - tr_gen_softmmu_slow_path
  - tr_gen_tb_end
