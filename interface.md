# Interface

| sequence | func / struct                                                                                                          | modification                                                                                                         |
|----------|------------------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------|
| 1        |                                                                                                                        | 调用 address_space_stl_notdirty 的时候是在处理 page table, 因为 BMBT 不处理 migration， 只处理 SMC，所以哪里没用     |
| 2        | address_space_translate_for_iotlb                                                                                      | 之后直接返回 MemoryRegion 的                                                                                         |
| 3        | memory_access_is_direct                                                                                                |                                                                                                                      |
| 4        | memory_region_get_ram_ptr                                                                                              | 没有令人窒息的 alias 之类的，直接返回 qemu_map_ram_ptr 就可以了                                                      |
| 5        | qemu_map_ram_ptr                                                                                                       | 只是一个非常阴间的函数，addr 可以是 RAM 内偏移，也同时可以是 ram_addr, 但是 BMBT 中其调用者可以保证一定是 ram 内偏移 |
| 6        | iotlb_to_section                                                                                                       |                                                                                                                      |
| 7        | CPUAddressSpace                                                                                                        | 被简化，CPUState 还是持有一个 AddressSpace 的，但是只是靠两个 CPUAddressSpace。                                      |
| 8        | AddressSpace 直接持有的 MemoryRegion，MemoryRegion 是不可以互相重叠，互相不可以重叠，因为主要重叠的地方都是 ROM 空间的 |                                                                                                                      |
| 9        | memory_region_clear_dirty_bitmap                                                                                       | 这是给 migration 使用的，没有必要                                                                                    |
| 10       | RAMList::mru_block                                                                                                     | 暂时保留 RAMList 的概念，但是因为 RAMList 现在只有一个 RAM, 所以 mru_block 就是那个一，不存在对应的 list 了          |
| 11       | BQl                                                                                                                    | 因为暂时只有一个 thread，所以 BQL 无需上锁了，现在只有逻辑功能的模拟                                                 |
| 12       | QemuSpin 和 QemuMutex                                                                                                  | 采取和 BQL 类似的做法                                                                                                |
| 13       | start_tcg_kick_timer                                                                                                   | 暂时只是支持一个 guest vCPU 的，所以无需 timer 来 kick 实现 vCPU 的轮转                                              |
| 14       | cpu_can_run, CPUState::stop, CPUState::stopped                                                                         | stop 适用于 migration 的时候将 CPU stop 的                                                                           |
| 15       | cpu_exec_start / cpu_exec_start / start_exclusive / end_exclusive                                                      | 因为现在只是支持单核，所以暂时不支持这些个操作                                                                       |
| 16       | cpus.c 中的几个 icount 函数                                                                                            |                                                                                                                      |
| 17       | i8259_init_chip                                                                                                        | 没有必要再去构建 ISABus ISADevice 之类的抽象概念了，没有总线的概念了，所有的设备也就是这几个                         |
| 18       | pic_common_realize                                                                                                     | 统计之类的事情以后再说吧                                                                                             |
| 19       | i8259.h                                                                                                                | 将 pc.h 中和 i8259 相关的代码移动到 i8259.h 中                                                                       |
| 20       | CPUArchId::cpu                                                                                                         | 为了保持 X86_CPU 的语义一致性，当然也没有实现 Object 类型，将类型修改为 CPUState 了                                  |
| 21       | pc_memory_init                                                                                                         | 在这里设置了众多的 memory region 的映射问题，但是实际上，我们是进行了硬编码处理这个事情的                            |
| 22       | pc_system_firmware_init                                                                                                | 因为不支持 pflash 所以这个函数被简化为直接调用 x86_bios_rom_init 了                                                  |

# 几个 macro 的说明
我发现，不要将原来的代码递归的拷贝过来，而是整个代码都拷贝过来，然后使用 `#if` 逐个 disable 掉。

- NEED_LATER : 将来应该需要
- BMBT : 一定不需要，应该在每一个 BMBT 跟上解释为什么不需要这个东西
