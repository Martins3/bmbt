# fw_cfg
- [ ] 到底提供了什么 romfile, 其中的出现了大量的，检测 romfile_loadint 如果失败就离开的代码
  - 从 QEMU 和 seabios 这边分别列举出来

## 基本原理
fw_cfg 出现在两个文件中， hw/nvram/fw_cfg.c 和 hw/i386/fw_cfg.c，
前者是主要实现，后者主要是为架构中添加一些细节。
- fw_cfg_add_acpi_dsdt : 在 acpi 中添加描述
- fw_cfg_build_feature_control : 构建 etc/msr_feature_control
- [ ] fw_cfg_build_smbios : 暂时看不懂


- fw_cfg_arch_create : come from [init-QEMU](./init-QEMU.md)
  - fw_cfg_init_io_dma(FW_CFG_IO_BASE, FW_CFG_IO_BASE + 4, &address_space_memory) : 第一参数是 IO, 第二个是 DMA
    - qdev_new(TYPE_FW_CFG_IO)
    - sysbus_realize_and_unref --> fw_cfg_io_realize
      - fw_cfg_file_slots_allocate : 初始化两个 FWCfgState::entries, 用于保存数据，其 key 就是事先定义好的宏，
      - 创建 fwcfg 和 fwcfg.dma 两个 MemoryRegion
      - fw_cfg_common_realize
        - 一堆 fw_cfg_add_i16 之类的，添加默认配置
    - sysbus_add_io
      - memory_region_add_subregion
    - 一堆 fw_cfg_add_i16 添加 x86 特有的配置 
    - 处理 NUMA 相关的内容

## add / modify
- fw_cfg_add_bytes : 很容易
- fw_cfg_add_file
  - fw_cfg_add_file_callback
    - 创建 FW_CFG_FILE_DIR

文件的处理方法:
- 常规内容都存贮在 FWCfgState::entries
- FWCfgState::entries 持有 FWCfgState::files 地址, FWCfgFiles 是为了记录文件的特有信息
- FW_CFG_FILE_FIRST 开始, FWCfgState::entries 持有 FWCfgState::files 的 index


## modify
- [ ] modify 总是和 reset 机制放到一起的

一共出现在两个位置：
  - pc_machine_done
    - `fw_cfg_modify_i16(x86ms->fw_cfg, FW_CFG_NB_CPUS, x86ms->boot_cpus);`
  - fw_cfg_modify_file
```
#0  fw_cfg_modify_file (s=0x1f, filename=0x0, data=0x5555569a3850, len=93825003170080) at ../hw/nvram/fw_cfg.c:1012
#1  0x000055555591a473 in fw_cfg_machine_reset (opaque=0x555556b92980) at ../hw/nvram/fw_cfg.c:1097
#2  0x0000555555e7ee35 in qemu_devices_reset () at ../hw/core/reset.c:69
#3  0x0000555555b5e68b in pc_machine_reset (machine=0x555556a94800) at ../hw/i386/pc.c:1644
#4  0x0000555555d36f90 in qemu_system_reset (reason=SHUTDOWN_CAUSE_NONE) at ../softmmu/runstate.c:442
#5  0x0000555555aec45c in qdev_machine_creation_done () at ../hw/core/machine.c:1299
#6  0x0000555555cdaea0 in qemu_machine_creation_done () at ../softmmu/vl.c:2579
#7  0x0000555555cdaf73 in qmp_x_exit_preconfig (errp=0x5555567a94b0 <error_fatal>) at ../softmmu/vl.c:2602
#8  0x0000555555cdd641 in qemu_init (argc=28, argv=0x7fffffffd7c8, envp=0x7fffffffd8b0) at ../softmmu/vl.c:3635
#9  0x000055555582e575 in main (argc=28, argv=0x7fffffffd7c8, envp=0x7fffffffd8b0) at ../softmmu/main.c:49
```

## FWCfgEntry::select_cb 和 FWCfgEntry::write_cb
仅仅是在 fw_cfg_select 中调用，出现三次，每次 select_cb 注册都是 acpi_build_update
在 acpi_setup 中正好添加了三次, 而 write_cb 从未使用过。

下面开启分析 FWCfgEntry::select_cb
```c
/*
#0  fw_cfg_select (s=0x555556c76600, key=42) at ../hw/nvram/fw_cfg.c:298
#1  0x00005555559187a5 in fw_cfg_dma_transfer (s=0x555556c76600) at ../hw/nvram/fw_cfg.c:371
#2  0x0000555555918b73 in fw_cfg_dma_mem_write (opaque=0x555556c76600, addr=4, value=28024, size=4) at ../hw/nvram/fw_cfg.c:469
#3  0x0000555555ca6b2a in memory_region_write_accessor (mr=0x555556c76980, addr=4, value=0x7fffe890efe8, size=4, shift=0, mask=4294967295, attrs=...) at ../softmmu/memory.c:489
#4  0x0000555555ca6d07 in access_with_adjusted_size (addr=4, value=0x7fffe890efe8, size=4, access_size_min=1, access_size_max=8, access_fn=0x555555ca6a3d <memory_region
_write_accessor>, mr=0x555556c76980, attrs=...) at ../softmmu/memory.c:545
#5  0x0000555555ca9e10 in memory_region_dispatch_write (mr=0x555556c76980, addr=4, data=28024, op=MO_32, attrs=...) at ../softmmu/memory.c:1500
#6  0x0000555555d31c60 in flatview_write_continue (fv=0x7ffdcc1cf380, addr=1304, attrs=..., ptr=0x7fffeb180000, len=4, addr1=4, l=4, mr=0x555556c76980) at ../softmmu/ph
ysmem.c:2767
#7  0x0000555555d31da9 in flatview_write (fv=0x7ffdcc1cf380, addr=1304, attrs=..., buf=0x7fffeb180000, len=4) at ../softmmu/physmem.c:2807
#8  0x0000555555d32123 in address_space_write (as=0x5555567a6b00 <address_space_io>, addr=1304, attrs=..., buf=0x7fffeb180000, len=4) at ../softmmu/physmem.c:2899
#9  0x0000555555d32194 in address_space_rw (as=0x5555567a6b00 <address_space_io>, addr=1304, attrs=..., buf=0x7fffeb180000, len=4, is_write=true) at ../softmmu/physmem.
c:2909
#10 0x0000555555c408d3 in kvm_handle_io (port=1304, attrs=..., data=0x7fffeb180000, direction=1, size=4, count=1) at ../accel/kvm/kvm-all.c:2626
#11 0x0000555555c410d1 in kvm_cpu_exec (cpu=0x555556c8be90) at ../accel/kvm/kvm-all.c:2877
#12 0x0000555555c95315 in kvm_vcpu_thread_fn (arg=0x555556c8be90) at ../accel/kvm/kvm-accel-ops.c:49
#13 0x0000555555f54268 in qemu_thread_start (args=0x555556bb1810) at ../util/qemu-thread-posix.c:521
#14 0x00007ffff6298609 in start_thread (arg=<optimized out>) at pthread_create.c:477
#15 0x00007ffff61bd293 in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:95
```
- acpi_build_update
  - acpi_build_tables_init : 初始化 tables 的数值
  - acpi_build : 我屮艸芔茻，这是把整个 acpi table 构建一次
  - acpi_ram_update


应该是，acpi 必须在运行时才可以构建好, 而且是通过 copy of table in RAM 来 patched
```c
typedef struct AcpiBuildState {
    /* Copy of table in RAM (for patching). */
    MemoryRegion *table_mr;
    /* Is table patched? */
    uint8_t patched;
    void *rsdp;
    MemoryRegion *rsdp_mr;
    MemoryRegion *linker_mr;
} AcpiBuildState;
```
又是涉及到 linker 之类的东西了

## io 和 mem
在 section [FWCfgState FWCfgIoState FWCfgMemState](#fwcfgstate-fwcfgiostate-fwcfgmemstate) 中我们看到存在两种 IO 模式，PIO 和 MMIO

1. fw_cfg 的核心部分 arch 的相关的提供四个接口，但是 x86 只是使用一个
  - fw_cfg_init_io_dma
  - fw_cfg_init_io
  - fw_cfg_init_mem_wide
  - fw_cfg_init_mem


## dma
在 fw_cfg_init_io_dma 中，根据参数，会设置 qdev_prop_set_bit(dev, "dma_enabled", false);

- fw_cfg_dma_mem_read : 根本不会被调用

- fw_cfg_dma_mem_write -> fw_cfg_dma_transfer
  - 无论是 dma read 或者 dma write 都是调用的 fw_cfg_dma_mem_write, 只是告诉 FWCfgState::dma_addr
  - 使用 dma_memory_read 利用 FWCfgState::dma_addr 来获取 FWCfgDmaAccess，其中包含了进行传输的必要信息
  - dma_memory_set
  - dma_memory_write
  - dma_memory_read

然后 DMA 进行有一个单独的端口，想要进行 DMA，首先组装 FWCfgDmaAccess，然后进行将地址告知即可。
## memory region
1. 在 fw_cfg_io_realize 中创建
2. 在 fw_cfg_init_io_dma 中和 link 到 get_system_io() 上 

```
address-space: I/O
  0000000000000000-000000000000ffff (prio 0, i/o): io
    0000000000000510-0000000000000511 (prio 0, i/o): fwcfg
    0000000000000514-000000000000051b (prio 0, i/o): fwcfg.dma
```

## Quick Ref

#### FWCfgState FWCfgIoState FWCfgMemState
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

#### MemoryRegionOps
```c
static const MemoryRegionOps fw_cfg_comb_mem_ops = {
    .read = fw_cfg_data_read,
    .write = fw_cfg_comb_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid.accepts = fw_cfg_comb_valid,
};

static const MemoryRegionOps fw_cfg_dma_mem_ops = {
    .read = fw_cfg_dma_mem_read,
    .write = fw_cfg_dma_mem_write,
    .endianness = DEVICE_BIG_ENDIAN,
    .valid.accepts = fw_cfg_dma_mem_valid,
    .valid.max_access_size = 8,
    .impl.max_access_size = 8,
};
```

#### FWCfgEntry
```c
struct FWCfgEntry {
    uint32_t len;
    bool allow_write;
    uint8_t *data;
    void *callback_opaque;
    FWCfgCallback select_cb;
    FWCfgWriteCallback write_cb;
};
```
