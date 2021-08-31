# Interface

| sequence | func / struct                     | modification                                                                                                         |
|----------|-----------------------------------|----------------------------------------------------------------------------------------------------------------------|
| 1        |                                   | 调用 address_space_stl_notdirty 的时候是在处理 page table, 因为 BMBT 不处理 migration， 只处理 SMC，所以哪里没用     |
| 2        | address_space_translate_for_iotlb | 之后直接返回 MemoryRegion 的                                                                                         |
| 3        | memory_access_is_direct           |                                                                                                                      |
| 4        | memory_region_get_ram_ptr         | 没有令人窒息的 alias 之类的，直接返回 qemu_map_ram_ptr 就可以了                                                      |
| 5        | qemu_map_ram_ptr                  | 只是一个非常阴间的函数，addr 可以是 RAM 内偏移，也同时可以是 ram_addr, 但是 BMBT 中其调用者可以保证一定是 ram 内偏移 |
| 6| iotlb_to_section| |
