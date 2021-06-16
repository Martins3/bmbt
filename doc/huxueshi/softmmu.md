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
- [ ] mmu_idx 是搞什么的
  - 关键是这个东西老是和 async_run_on_cpu 有关
- [ ] victim_tlb_hit : victim tlb 是做什么的 ?
