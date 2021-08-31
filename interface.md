# Interface


| sequence | func / struct                              | modification
|----------|-----------------------------------|------------------------------------------------------------------------------------------------------------------|
| 1        |                                   | 调用 address_space_stl_notdirty 的时候是在处理 page table, 因为 BMBT 不处理 migration， 只处理 SMC，所以哪里没用 |
| 2        | address_space_translate_for_iotlb | 之后直接返回 MemoryRegion 的                                                                                     |
| 3        | memory_access_is_direct           |
