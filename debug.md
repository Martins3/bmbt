# passthrough pci
调查清楚两个事情:
- loongarch 如何使用 pcie 设备
  - [ ] 从头到尾都是 guest 接手可以吗?
- guest 如何使用 pcie 设备

- [ ] 搞清楚是实现　pch msi 中断的过程
  - [ ] 搞一个 looarch 可以测试网络的 img
  - [ ] 给那个 mini guest 添加上网络，分析在 la 上网络的基本过程
- we already have two pci device
- seabios will detect pci devices

- [ ] 咨询一下，为什么只是支持 virtio 设备
  - 是不是因为存在 option rom 需要执行的呀
  - **咨询一下赵天瑞**

## 路径
- 确定 loongson linux 中初始化 pcie 部分
- 让 seabios 可以探测到这些 virtio 设备吧
- 直通 virtio 驱动
- 直通物理驱动

## 挑战
- [ ] loongson 中不存在连个端口的访问，似乎所以的配置空间都在那个位置
- [ ] 不希望每一个设备都搞一次适配的
- [ ] 中断的分配规则是什么
- [ ] 读取配置空间的操作实际上，发生的很少，似乎设备的空间是动态分配的，所以，到时候，需要动态的时候需要放到什么位置

## network_init
- analzy how pci works in qemu

### [ ] ls7a_pci_conf
what's this?
```c
    0000000020000000-0000000027ffffff (prio 0, i/o): pcie-mmcfg-mmio
```
- pcie_mmcfg_data_write
- pcie_mmcfg_data_read

- ls7a_init
  - dev = qdev_create(NULL, TYPE_LS7A_PCIE_HOST_BRIDGE);
  - pci_bus = pci_ls7a_init(machine, dev, pic);
  - qdev_init_nofail(dev);
  - pcid = pci_create(pci_bus, PCI_DEVFN(0, 0), TYPE_PCIE_LS7A);

## pci_ls7a_config 到底有没有作用
- pci_ls7a_config_ops : 不是通过端口访问的
可以直接删除的

### dump mem tree in centos-qemu
```c
(qemu)
(qemu) info mtree
address-space: memory
  0000000000000000-ffffffffffffffff (prio 0, i/o): system
    0000000000000000-000000000fffffff (prio 0, i/o): alias loongarch_ls3a.node0.lowram @loongarch_ls3a.ram 0000000000000000-000000000fffffff
    00000000000a0000-00000000000bffff (prio 1, i/o): cirrus-lowmem-container
      00000000000a0000-00000000000a7fff (prio 1, i/o): alias vga.bank0 @vga.vram 0000000000000000-0000000000007fff
      00000000000a0000-00000000000bffff (prio 0, i/o): cirrus-low-memory
      00000000000a8000-00000000000affff (prio 1, i/o): alias vga.bank1 @vga.vram 0000000000008000-000000000000ffff
    0000000010000000-0000000010000fff (prio 0, i/o): ioapic
    0000000010002000-0000000010002013 (prio 1, i/o): 0x10002000
    000000001001041c-000000001001041f (prio 1, i/o): 0x1001041c
    0000000010013ffc-0000000010013fff (prio 1, i/o): 0x10013ffc
    0000000010080000-00000000100800ff (prio 0, i/o): ls3a_pm
    00000000100d0000-00000000100d00ff (prio 0, i/o): ls7a_pm
      00000000100d000c-00000000100d0013 (prio 0, i/o): acpi-evt
      00000000100d0014-00000000100d0017 (prio 0, i/o): acpi-cnt
      00000000100d0018-00000000100d001b (prio 0, i/o): acpi-tmr
      00000000100d0028-00000000100d002f (prio 0, i/o): acpi-gpe0
      00000000100d0030-00000000100d0033 (prio 0, i/o): acpi-reset
    00000000100d0100-00000000100d01ff (prio 0, i/o): ls7a_rtc
    0000000018000000-0000000019ffffff (prio 0, i/o): alias isa-io @io 0000000000000000-0000000001ffffff
    000000001a000000-000000001bffffff (prio 0, i/o): ls7a_pci_conf
    000000001c000000-000000001c3fffff (prio 0, rom): loongarch.bios
    000000001e000000-000000001e00000b (prio 0, i/o): acpi-mem-hotplug
    000000001e020000-000000001e020001 (prio 0, i/o): fwcfg.ctl
    000000001e020008-000000001e02000f (prio 0, i/o): fwcfg.data
    000000001f000000-000000001f0000ff (prio 0, i/o): gipi0
    000000001f010000-000000001f02ffff (prio 0, i/o): apic0
    000000001fe00008-000000001fe0000f (prio 1, i/o): ((hwaddr)0x1fe00008 | off)
    000000001fe00010-000000001fe00017 (prio 1, i/o): ((hwaddr)0x1fe00010 | off)
    000000001fe00020-000000001fe00027 (prio 1, i/o): ((hwaddr)0x1fe00020 | off)
    000000001fe00180-000000001fe00187 (prio 1, i/o): ((hwaddr)0x1fe00180 | off)
    000000001fe0019c-000000001fe001a3 (prio 1, i/o): ((hwaddr)0x1fe0019c | off)
    000000001fe001d0-000000001fe001d7 (prio 1, i/o): ((hwaddr)0x1fe001d0 | off)
    000000001fe001e0-000000001fe001e7 (prio 0, i/o): serial
    000000001fe002e0-000000001fe002e0 (prio 0, i/o): debugcon
    000000001fe00420-000000001fe00427 (prio 1, i/o): ((hwaddr)0x1fe00420 | off)
    0000000020000000-0000000027ffffff (prio 0, i/o): pcie-mmcfg-mmio
    000000002ff00000-000000002ff00007 (prio 0, i/o): ls3a_msi
    0000000040000000-0000000041ffffff (prio 1, i/o): cirrus-pci-bar0
      0000000040000000-00000000403fffff (prio 1, ram): vga.vram
      0000000040000000-00000000403fffff (prio 0, i/o): cirrus-linear-io
      0000000041000000-00000000413fffff (prio 0, i/o): cirrus-bitblt-mmio
    0000000040000000-000000007fffffff (prio 0, i/o): isa-mem
    0000000042000000-0000000042003fff (prio 1, i/o): virtio-pci
      0000000042000000-0000000042000fff (prio 0, i/o): virtio-pci-common
      0000000042001000-0000000042001fff (prio 0, i/o): virtio-pci-isr
      0000000042002000-0000000042002fff (prio 0, i/o): virtio-pci-device
      0000000042003000-0000000042003fff (prio 0, i/o): virtio-pci-notify
    0000000042004000-0000000042007fff (prio 1, i/o): virtio-pci
      0000000042004000-0000000042004fff (prio 0, i/o): virtio-pci-common
      0000000042005000-0000000042005fff (prio 0, i/o): virtio-pci-isr
      0000000042006000-0000000042006fff (prio 0, i/o): virtio-pci-device
      0000000042007000-0000000042007fff (prio 0, i/o): virtio-pci-notify
    0000000042008000-0000000042008fff (prio 1, i/o): virtio-net-pci-msix
      0000000042008000-000000004200802f (prio 0, i/o): msix-table
      0000000042008800-0000000042008807 (prio 0, i/o): msix-pba
    0000000042009000-0000000042009fff (prio 1, i/o): cirrus-mmio
    000000004200a000-000000004200afff (prio 1, i/o): virtio-blk-pci-msix
      000000004200a000-000000004200a01f (prio 0, i/o): msix-table
      000000004200a800-000000004200a807 (prio 0, i/o): msix-pba
    0000000090000000-000000027fffffff (prio 0, i/o): alias loongarch_ls3a.node0.highram @loongarch_ls3a.ram 0000000010000000-00000001ffffffff

address-space: I/O
  0000000000000000-000000000000ffff (prio 0, i/o): io
    00000000000003b0-00000000000003df (prio 0, i/o): cirrus-io
    0000000000004000-000000000000407f (prio 1, i/o): virtio-pci
    0000000000004080-000000000000409f (prio 1, i/o): virtio-pci

address-space: cpu-memory-0
  0000000000000000-ffffffffffffffff (prio 0, i/o): system
    0000000000000000-000000000fffffff (prio 0, i/o): alias loongarch_ls3a.node0.lowram @loongarch_ls3a.ram 0000000000000000-000000000fffffff
    00000000000a0000-00000000000bffff (prio 1, i/o): cirrus-lowmem-container
      00000000000a0000-00000000000a7fff (prio 1, i/o): alias vga.bank0 @vga.vram 0000000000000000-0000000000007fff
      00000000000a0000-00000000000bffff (prio 0, i/o): cirrus-low-memory
      00000000000a8000-00000000000affff (prio 1, i/o): alias vga.bank1 @vga.vram 0000000000008000-000000000000ffff
    0000000010000000-0000000010000fff (prio 0, i/o): ioapic
    0000000010002000-0000000010002013 (prio 1, i/o): 0x10002000
    000000001001041c-000000001001041f (prio 1, i/o): 0x1001041c
    0000000010013ffc-0000000010013fff (prio 1, i/o): 0x10013ffc
    0000000010080000-00000000100800ff (prio 0, i/o): ls3a_pm
    00000000100d0000-00000000100d00ff (prio 0, i/o): ls7a_pm
      00000000100d000c-00000000100d0013 (prio 0, i/o): acpi-evt
      00000000100d0014-00000000100d0017 (prio 0, i/o): acpi-cnt
      00000000100d0018-00000000100d001b (prio 0, i/o): acpi-tmr
      00000000100d0028-00000000100d002f (prio 0, i/o): acpi-gpe0
      00000000100d0030-00000000100d0033 (prio 0, i/o): acpi-reset
    00000000100d0100-00000000100d01ff (prio 0, i/o): ls7a_rtc
    0000000018000000-0000000019ffffff (prio 0, i/o): alias isa-io @io 0000000000000000-0000000001ffffff
    000000001a000000-000000001bffffff (prio 0, i/o): ls7a_pci_conf
    000000001c000000-000000001c3fffff (prio 0, rom): loongarch.bios
    000000001e000000-000000001e00000b (prio 0, i/o): acpi-mem-hotplug
    000000001e020000-000000001e020001 (prio 0, i/o): fwcfg.ctl
    000000001e020008-000000001e02000f (prio 0, i/o): fwcfg.data
    000000001f000000-000000001f0000ff (prio 0, i/o): gipi0
    000000001f010000-000000001f02ffff (prio 0, i/o): apic0
    000000001fe00008-000000001fe0000f (prio 1, i/o): ((hwaddr)0x1fe00008 | off)
    000000001fe00010-000000001fe00017 (prio 1, i/o): ((hwaddr)0x1fe00010 | off)
    000000001fe00020-000000001fe00027 (prio 1, i/o): ((hwaddr)0x1fe00020 | off)
    000000001fe00180-000000001fe00187 (prio 1, i/o): ((hwaddr)0x1fe00180 | off)
    000000001fe0019c-000000001fe001a3 (prio 1, i/o): ((hwaddr)0x1fe0019c | off)
    000000001fe001d0-000000001fe001d7 (prio 1, i/o): ((hwaddr)0x1fe001d0 | off)
    000000001fe001e0-000000001fe001e7 (prio 0, i/o): serial
    000000001fe002e0-000000001fe002e0 (prio 0, i/o): debugcon
    000000001fe00420-000000001fe00427 (prio 1, i/o): ((hwaddr)0x1fe00420 | off)
    0000000020000000-0000000027ffffff (prio 0, i/o): pcie-mmcfg-mmio
    000000002ff00000-000000002ff00007 (prio 0, i/o): ls3a_msi
    0000000040000000-0000000041ffffff (prio 1, i/o): cirrus-pci-bar0
      0000000040000000-00000000403fffff (prio 1, ram): vga.vram
      0000000040000000-00000000403fffff (prio 0, i/o): cirrus-linear-io
      0000000041000000-00000000413fffff (prio 0, i/o): cirrus-bitblt-mmio
    0000000040000000-000000007fffffff (prio 0, i/o): isa-mem
    0000000042000000-0000000042003fff (prio 1, i/o): virtio-pci
      0000000042000000-0000000042000fff (prio 0, i/o): virtio-pci-common
      0000000042001000-0000000042001fff (prio 0, i/o): virtio-pci-isr
      0000000042002000-0000000042002fff (prio 0, i/o): virtio-pci-device
      0000000042003000-0000000042003fff (prio 0, i/o): virtio-pci-notify
    0000000042004000-0000000042007fff (prio 1, i/o): virtio-pci
      0000000042004000-0000000042004fff (prio 0, i/o): virtio-pci-common
      0000000042005000-0000000042005fff (prio 0, i/o): virtio-pci-isr
      0000000042006000-0000000042006fff (prio 0, i/o): virtio-pci-device
      0000000042007000-0000000042007fff (prio 0, i/o): virtio-pci-notify
    0000000042008000-0000000042008fff (prio 1, i/o): virtio-net-pci-msix
      0000000042008000-000000004200802f (prio 0, i/o): msix-table
      0000000042008800-0000000042008807 (prio 0, i/o): msix-pba
    0000000042009000-0000000042009fff (prio 1, i/o): cirrus-mmio
    000000004200a000-000000004200afff (prio 1, i/o): virtio-blk-pci-msix
      000000004200a000-000000004200a01f (prio 0, i/o): msix-table
      000000004200a800-000000004200a807 (prio 0, i/o): msix-pba
    0000000090000000-000000027fffffff (prio 0, i/o): alias loongarch_ls3a.node0.highram @loongarch_ls3a.ram 0000000010000000-00000001ffffffff

address-space: ls7a1000_pcie
  0000000000000000-ffffffffffffffff (prio 0, i/o): bus master container
    0000000000000000-ffffffffffffffff (prio 0, i/o): alias bus master @system 0000000000000000-ffffffffffffffff [disabled]

address-space: virtio-net-pci
  0000000000000000-ffffffffffffffff (prio 0, i/o): bus master container
    0000000000000000-ffffffffffffffff (prio 0, i/o): alias bus master @system 0000000000000000-ffffffffffffffff

address-space: cirrus-vga
  0000000000000000-ffffffffffffffff (prio 0, i/o): bus master container
    0000000000000000-ffffffffffffffff (prio 0, i/o): alias bus master @system 0000000000000000-ffffffffffffffff

address-space: virtio-blk-pci
  0000000000000000-ffffffffffffffff (prio 0, i/o): bus master container
    0000000000000000-ffffffffffffffff (prio 0, i/o): alias bus master @system 0000000000000000-ffffffffffffffff

memory-region: loongarch_ls3a.ram
  0000000000000000-00000001ffffffff (prio 0, ram): loongarch_ls3a.ram

memory-region: vga.vram
  0000000000000000-00000000003fffff (prio 1, ram): vga.vram

memory-region: io
  0000000000000000-000000000000ffff (prio 0, i/o): io
    00000000000003b0-00000000000003df (prio 0, i/o): cirrus-io
    0000000000004000-000000000000407f (prio 1, i/o): virtio-pci
    0000000000004080-000000000000409f (prio 1, i/o): virtio-pci

memory-region: system
  0000000000000000-ffffffffffffffff (prio 0, i/o): system
    0000000000000000-000000000fffffff (prio 0, i/o): alias loongarch_ls3a.node0.lowram @loongarch_ls3a.ram 0000000000000000-000000000fffffff
    00000000000a0000-00000000000bffff (prio 1, i/o): cirrus-lowmem-container
      00000000000a0000-00000000000a7fff (prio 1, i/o): alias vga.bank0 @vga.vram 0000000000000000-0000000000007fff
      00000000000a0000-00000000000bffff (prio 0, i/o): cirrus-low-memory
      00000000000a8000-00000000000affff (prio 1, i/o): alias vga.bank1 @vga.vram 0000000000008000-000000000000ffff
    0000000010000000-0000000010000fff (prio 0, i/o): ioapic
    0000000010002000-0000000010002013 (prio 1, i/o): 0x10002000
    000000001001041c-000000001001041f (prio 1, i/o): 0x1001041c
    0000000010013ffc-0000000010013fff (prio 1, i/o): 0x10013ffc
    0000000010080000-00000000100800ff (prio 0, i/o): ls3a_pm
    00000000100d0000-00000000100d00ff (prio 0, i/o): ls7a_pm
      00000000100d000c-00000000100d0013 (prio 0, i/o): acpi-evt
      00000000100d0014-00000000100d0017 (prio 0, i/o): acpi-cnt
      00000000100d0018-00000000100d001b (prio 0, i/o): acpi-tmr
      00000000100d0028-00000000100d002f (prio 0, i/o): acpi-gpe0
      00000000100d0030-00000000100d0033 (prio 0, i/o): acpi-reset
    00000000100d0100-00000000100d01ff (prio 0, i/o): ls7a_rtc
    0000000018000000-0000000019ffffff (prio 0, i/o): alias isa-io @io 0000000000000000-0000000001ffffff
    000000001a000000-000000001bffffff (prio 0, i/o): ls7a_pci_conf
    000000001c000000-000000001c3fffff (prio 0, rom): loongarch.bios
    000000001e000000-000000001e00000b (prio 0, i/o): acpi-mem-hotplug
    000000001e020000-000000001e020001 (prio 0, i/o): fwcfg.ctl
    000000001e020008-000000001e02000f (prio 0, i/o): fwcfg.data
    000000001f000000-000000001f0000ff (prio 0, i/o): gipi0
    000000001f010000-000000001f02ffff (prio 0, i/o): apic0
    000000001fe00008-000000001fe0000f (prio 1, i/o): ((hwaddr)0x1fe00008 | off)
    000000001fe00010-000000001fe00017 (prio 1, i/o): ((hwaddr)0x1fe00010 | off)
    000000001fe00020-000000001fe00027 (prio 1, i/o): ((hwaddr)0x1fe00020 | off)
    000000001fe00180-000000001fe00187 (prio 1, i/o): ((hwaddr)0x1fe00180 | off)
    000000001fe0019c-000000001fe001a3 (prio 1, i/o): ((hwaddr)0x1fe0019c | off)
    000000001fe001d0-000000001fe001d7 (prio 1, i/o): ((hwaddr)0x1fe001d0 | off)
    000000001fe001e0-000000001fe001e7 (prio 0, i/o): serial
    000000001fe002e0-000000001fe002e0 (prio 0, i/o): debugcon
    000000001fe00420-000000001fe00427 (prio 1, i/o): ((hwaddr)0x1fe00420 | off)
    0000000020000000-0000000027ffffff (prio 0, i/o): pcie-mmcfg-mmio
    000000002ff00000-000000002ff00007 (prio 0, i/o): ls3a_msi
    0000000040000000-0000000041ffffff (prio 1, i/o): cirrus-pci-bar0
      0000000040000000-00000000403fffff (prio 1, ram): vga.vram
      0000000040000000-00000000403fffff (prio 0, i/o): cirrus-linear-io
      0000000041000000-00000000413fffff (prio 0, i/o): cirrus-bitblt-mmio
    0000000040000000-000000007fffffff (prio 0, i/o): isa-mem
    0000000042000000-0000000042003fff (prio 1, i/o): virtio-pci
      0000000042000000-0000000042000fff (prio 0, i/o): virtio-pci-common
      0000000042001000-0000000042001fff (prio 0, i/o): virtio-pci-isr
      0000000042002000-0000000042002fff (prio 0, i/o): virtio-pci-device
      0000000042003000-0000000042003fff (prio 0, i/o): virtio-pci-notify
    0000000042004000-0000000042007fff (prio 1, i/o): virtio-pci
      0000000042004000-0000000042004fff (prio 0, i/o): virtio-pci-common
      0000000042005000-0000000042005fff (prio 0, i/o): virtio-pci-isr
      0000000042006000-0000000042006fff (prio 0, i/o): virtio-pci-device
      0000000042007000-0000000042007fff (prio 0, i/o): virtio-pci-notify
    0000000042008000-0000000042008fff (prio 1, i/o): virtio-net-pci-msix
      0000000042008000-000000004200802f (prio 0, i/o): msix-table
      0000000042008800-0000000042008807 (prio 0, i/o): msix-pba
    0000000042009000-0000000042009fff (prio 1, i/o): cirrus-mmio
    000000004200a000-000000004200afff (prio 1, i/o): virtio-blk-pci-msix
      000000004200a000-000000004200a01f (prio 0, i/o): msix-table
      000000004200a800-000000004200a807 (prio 0, i/o): msix-pba
    0000000090000000-000000027fffffff (prio 0, i/o): alias loongarch_ls3a.node0.highram @loongarch_ls3a.ram 0000000010000000-00000001ffffffff
```

### 使用 e1000 而不是 virtio

## 转换 mmcfg 吧
只能说，应该难度不大!

### 在 Loongson QEMU 中
```c
/* the helper function to get a PCIDevice* for a given pci address */
static inline PCIDevice *pci_dev_find_by_addr(PCIBus *bus, uint32_t addr)
{
    uint8_t bus_num = addr >> 16;
    uint8_t devfn = addr >> 8;

    return pci_find_device(bus, bus_num, devfn);
}

static void pci_ls7a_config_write(void *opaque, hwaddr addr,
                                        uint64_t val, unsigned size)
{
    hwaddr tmp_addr;
    tmp_addr = addr & 0xffffff;

    pci_data_write(opaque, tmp_addr, val, size);
}
```

正确的 QEMU 中，
```c
const MemoryRegionOps pci_host_data_le_ops = {
    .read = pci_host_data_read,
    .write = pci_host_data_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
};

static uint64_t pci_host_data_read(void *opaque,
                                   hwaddr addr, unsigned len)
{
    PCIHostState *s = opaque;

    if (!(s->config_reg & (1U << 31))) {
        return 0xffffffff;
    }
    return pci_data_read(s->bus, s->config_reg | (addr & 3), len);
}
```

### 在 Loongson Linux 中
- [ ] 一定存在对应的配置吧
  - 确认是一下，pci_ops 就是注册的
```c
struct pci_ecam_ops loongson_pci_ecam_ops = {
  .bus_shift  = 16,
  .init   = loongson_pci_ecam_init,
  .pci_ops  = {
    .map_bus  = pci_loongson_map_bus,
    .read   = pci_loongson_config_read,
    .write    = pci_generic_config_write,
  }
};
```

# pci
https://wiki.osdev.org/PCI_Express : 通过 ACPI 的配置，可以让 PCI 配置空间的访问使用 mmio 的方式

- [ ] https://lwn.net/Articles/367630/ : how to write acpi driver

- [ ] 那些 probe 工作是怎么进行的

## Ntoes
- [ ] acpi_scan_add_handler
- [ ] acpi_get_table 可以直接获取 acpi table 出来，所以，这些 table 是什么时候构建的 ?

## really probe

#### acpi_init
1. acpi_init 位于 `subsys_initcall` 中间:

- [ ] 似乎再次之前，内存分配已经搞定了

```c
/*

[    1.846074] Call Trace:
[    1.846076] [<900000000020864c>] show_stack+0x2c/0x100
[    1.846079] [<9000000000ec3948>] dump_stack+0x90/0xc0
[    1.846081] [<900000000029f8e0>] irq_domain_set_mapping+0x90/0xb8
[    1.846084] [<90000000002a0698>] __irq_domain_alloc_irqs+0x200/0x2e0
[    1.846085] [<90000000002a2d08>] msi_domain_alloc_irqs+0x90/0x320
[    1.846088] [<900000000020306c>] arch_setup_msi_irqs+0x8c/0xc8
[    1.846090] [<900000000085cc64>] __pci_enable_msi_range+0x324/0x5e8
[    1.846092] [<900000000085d078>] pci_alloc_irq_vectors_affinity+0x120/0x158
[    1.846094] [<9000000000a926b8>] ahci_init_one+0xb28/0x1020
[    1.846097] [<900000000083c910>] local_pci_probe+0x48/0xe0
[    1.846099] [<900000000024cff4>] work_for_cpu_fn+0x1c/0x30
[    1.846100] [<9000000000250778>] process_one_work+0x210/0x418
[    1.846102] [<9000000000250cb8>] worker_thread+0x338/0x5e0
[    1.846104] [<90000000002578fc>] kthread+0x124/0x128
[    1.846106] [<9000000000203cc8>] ret_from_kernel_thread+0xc/0x10
[    1.846108] irq: irq_domain_set_mapping 64 38


[    0.888500] [<900000000020864c>] show_stack+0x2c/0x100
[    0.888502] [<9000000000ec3968>] dump_stack+0x90/0xc0
[    0.888505] [<90000000009a601c>] really_probe+0x20c/0x2b8
[    0.888507] [<90000000009a6274>] driver_probe_device+0x64/0x100
[    0.888509] [<90000000009a3dd8>] bus_for_each_drv+0x68/0xa8
[    0.888511] [<90000000009a5d8c>] __device_attach+0x124/0x1a0
[    0.888513] [<90000000009a500c>] bus_probe_device+0x9c/0xc0
[    0.888514] [<90000000009a1470>] device_add+0x350/0x608
[    0.888517] [<90000000009a7f80>] platform_device_add+0x128/0x288
[    0.888519] [<90000000009a8d90>] platform_device_register_full+0xb8/0x130
[    0.888521] [<90000000008a255c>] acpi_create_platform_device+0x28c/0x300
[    0.888523] [<9000000000898328>] acpi_default_enumeration+0x28/0x58
[    0.888524] [<9000000000898558>] acpi_bus_attach+0x1d0/0x210
[    0.888526] [<90000000008983e0>] acpi_bus_attach+0x58/0x210
[    0.888528] [<90000000008983e0>] acpi_bus_attach+0x58/0x210
[    0.888529] [<900000000089a52c>] acpi_bus_scan+0x34/0x78
[    0.888532] [<90000000012f8060>] acpi_scan_init+0x170/0x2b8
[    0.888534] [<90000000012f7c68>] acpi_init+0x2e4/0x338
[    0.888535] [<90000000002004fc>] do_one_initcall+0x6c/0x170
[    0.888537] [<90000000012d4ce0>] kernel_init_freeable+0x1f8/0x2b8
[    0.888540] [<9000000000eda774>] kernel_init+0x10/0xf4
```
