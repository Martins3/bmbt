# fw_cfg

- [ ] 到底提供了什么 romfile, 其中的出现了大量的，检测 romfile_loadint 如果失败就离开的代码
  - 从 QEMU 和 seabios 这边分别列举出来

- fw_cfg 的原理:
  - DMA
  - 添加文件

## FW_CFG_IO_BASE

```c
#define FW_CFG_IO_BASE     0x510
```

fw_cfg_add_acpi_dsdt : 制作 acpi table 的时候会包含这个信息

在这里初始化:
```c
static void fw_cfg_io_realize(DeviceState *dev, Error **errp)
{
    ERRP_GUARD();
    FWCfgIoState *s = FW_CFG_IO(dev);

    fw_cfg_file_slots_allocate(FW_CFG(s), errp);
    if (*errp) {
        return;
    }

    /* when using port i/o, the 8-bit data register ALWAYS overlaps
     * with half of the 16-bit control register. Hence, the total size
     * of the i/o region used is FW_CFG_CTL_SIZE */
    memory_region_init_io(&s->comb_iomem, OBJECT(s), &fw_cfg_comb_mem_ops,
                          FW_CFG(s), "fwcfg", FW_CFG_CTL_SIZE);

    if (FW_CFG(s)->dma_enabled) {
        memory_region_init_io(&FW_CFG(s)->dma_iomem, OBJECT(s),
                              &fw_cfg_dma_mem_ops, FW_CFG(s), "fwcfg.dma",
                              sizeof(dma_addr_t));
    }

    fw_cfg_common_realize(dev, errp);
}
```

fw_cfg_comb_mem_ops 中对应的 read / write 实现，首先选择地址，然后操作:

- dma 的操作: fw_cfg_dma_transfer 中的，根据配置的地址，最后调用 dma_memory_read / dma_memory_write

## 基本原理
出现的文件:
- [ ] hw/nvram/fw_cfg.c
- [ ] hw/i386/fw_cfg.c

- fw_cfg_arch_create : come from [init-QEMU](./init-QEMU.md)
  - fw_cfg_init_io_dma
    - qdev_new(TYPE_FW_CFG_IO)
    - sysbus_realize_and_unref --> fw_cfg_io_realize
      - fw_cfg_file_slots_allocate
      - 创建两个 io 空间
      - fw_cfg_common_realize
        - 添加一堆默认配置
    - [ ] `sysbus_add_io(sbd, iobase, &ios->comb_iomem);`
      - SysBusDevice 是 FWCfgState 的 parent
      - [ ] iobase 是啥 ?
    - 一堆 fw_cfg_add_i16 和 fw_cfg_add_file
    - 处理 NUMA 相关的内容

```c
struct FWCfgState {
    /*< private >*/
    SysBusDevice parent_obj;
    /*< public >*/

    uint16_t file_slots;
    FWCfgEntry *entries[2];
    int *entry_order;
    FWCfgFiles *files;
    uint16_t cur_entry;
    uint32_t cur_offset;
    Notifier machine_ready;

    int fw_cfg_order_override;

    bool dma_enabled;
    dma_addr_t dma_addr;
    AddressSpace *dma_as;
    MemoryRegion dma_iomem;

    /* restore during migration */
    bool acpi_mr_restore;
    uint64_t table_mr_size;
    uint64_t linker_mr_size;
    uint64_t rsdp_mr_size;
};

struct FWCfgIoState {
    /*< private >*/
    FWCfgState parent_obj;
    /*< public >*/

    MemoryRegion comb_iomem;
};

struct FWCfgMemState {
    /*< private >*/
    FWCfgState parent_obj;
    /*< public >*/

    MemoryRegion ctl_iomem, data_iomem;
    uint32_t data_width;
    MemoryRegionOps wide_data_ops;
};
```
