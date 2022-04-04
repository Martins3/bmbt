# 总体思路
## Loonarch PCIe 分配 irq 的规则
这些 interrupt irq 总是正好顺着的，分析 `pch_msi_allocate_hwirq` 实际上，msi 就是从 32 开始，逐个分配，没有什么要求。

从哪里知道是 7A1000 table 5.1 中的数值的

```txt
#0  pch_msi_allocate_hwirq (priv=<optimized out>, num_req=<optimized out>) at drivers/irqchip/irq-loongson-pch-msi.c:68
#1  pci_msi_domain_set_desc (arg=0x900000027cd338f0, desc=0x900000027d2cdd80) at drivers/irqchip/irq-loongson-pch-msi.c:116
#2  0x900000000028e3d0 in msi_domain_alloc_irqs (domain=0x900000027c08c600, dev=0x900000027d41c0a8, nvec=<optimized out>) at kernel/irq/msi.c:415
#3  0x90000000008e1840 in msix_capability_init (affd=<optimized out>, nvec=<optimized out>, entries=<optimized out>, dev=<optimized out>) at drivers/pci/msi.c:759
#4  __pci_enable_msix (affd=<optimized out>, nvec=<optimized out>, entries=<optimized out>, dev=<optimized out>) at drivers/pci/msi.c:967
#5  __pci_enable_msix_range (affd=<optimized out>, maxvec=<optimized out>, minvec=<optimized out>, entries=<optimized out>, dev=<optimized out>) at drivers/pci/msi.c:11
00
#6  __pci_enable_msix_range (dev=0x900000027d41c000, entries=0x0, minvec=2, maxvec=2, affd=0x900000027cd33b98) at drivers/pci/msi.c:1081
#7  0x90000000008e2298 in pci_alloc_irq_vectors_affinity (dev=0x900000027d41c000, min_vecs=2, max_vecs=2, flags=12, affd=0x900000027cd33b98) at drivers/pci/msi.c:1170
#8  0x9000000000963658 in vp_request_msix_vectors (desc=<optimized out>, per_vq_vectors=<optimized out>, nvectors=<optimized out>, vdev=<optimized out>) at drivers/virt
io/virtio_pci_common.c:136
#9  vp_find_vqs_msix (vdev=0x900000027cff6800, nvqs=1, vqs=<optimized out>, callbacks=0x900000027d8384c0, names=0x900000027d838480, per_vq_vectors=true, ctx=0x0, desc=0
x900000027cd33b98) at drivers/virtio/virtio_pci_common.c:307
#10 0x9000000000963a1c in vp_find_vqs (vdev=0x900000027cff6800, nvqs=1, vqs=0x900000027d838500, callbacks=0x900000027d8384c0, names=0x900000027d838480, ctx=0x0, desc=0x
900000027cd33b98) at drivers/virtio/virtio_pci_common.c:403
#11 0x90000000009624f0 in vp_modern_find_vqs (vdev=0x900000027cff6800, nvqs=<optimized out>, vqs=<optimized out>, callbacks=<optimized out>, names=<optimized out>, ctx=
<optimized out>, desc=<optimized out>) at drivers/virtio/virtio_pci_modern.c:413
#12 0x9000000000a51c68 in virtio_find_vqs (desc=<optimized out>, names=<optimized out>, callbacks=<optimized out>, vqs=<optimized out>, nvqs=<optimized out>, vdev=<opti
mized out>) at ./include/linux/virtio_config.h:192
#13 init_vq (vblk=0x900000027d1e5800) at drivers/block/virtio_blk.c:542
#14 0x9000000000a52d1c in virtblk_probe (vdev=0x900000027cd338f0) at drivers/block/virtio_blk.c:774
#15 0x900000000095f794 in virtio_dev_probe (_d=0x900000027cff6810) at drivers/virtio/virtio.c:245
#16 0x9000000000a293a8 in really_probe (dev=0x900000027cff6810, drv=0x90000000014359c0 <virtio_blk>) at drivers/base/dd.c:506
#17 0x9000000000a295e0 in driver_probe_device (drv=0x90000000014359c0 <virtio_blk>, dev=0x900000027cff6810) at drivers/base/dd.c:667
#18 0x9000000000a29788 in __driver_attach (data=<optimized out>, dev=<optimized out>) at drivers/base/dd.c:903
#19 __driver_attach (dev=0x900000027cff6810, data=0x90000000014359c0 <virtio_blk>) at drivers/base/dd.c:872
#20 0x9000000000a270dc in bus_for_each_dev (bus=<optimized out>, start=<optimized out>, data=0x0, fn=0x900000027cd338f0) at drivers/base/bus.c:279
#21 0x9000000000a28bec in driver_attach (drv=<optimized out>) at drivers/base/dd.c:922
#22 0x9000000000a28658 in bus_add_driver (drv=0x90000000014359c0 <virtio_blk>) at drivers/base/bus.c:672
#23 0x9000000000a2a3cc in driver_register (drv=0x90000000014359c0 <virtio_blk>) at drivers/base/driver.c:170
#24 0x900000000095f170 in register_virtio_driver (driver=<optimized out>) at drivers/virtio/virtio.c:296
#25 0x90000000014ee768 in init () at drivers/block/virtio_blk.c:1019
#26 0x9000000000200b8c in do_one_initcall (fn=0x90000000014ee6f0 <init>) at init/main.c:884
#27 0x90000000014a4e8c in do_initcall_level (level=<optimized out>) at ./include/linux/init.h:131
#28 do_initcalls () at init/main.c:960
#29 do_basic_setup () at init/main.c:978
#30 kernel_init_freeable () at init/main.c:1145
#31 0x9000000000f78fe8 in kernel_init (unused=<optimized out>) at init/main.c:1062
#32 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```

```txt
#0  pch_msi_allocate_hwirq (priv=<optimized out>, num_req=<optimized out>) at drivers/irqchip/irq-loongson-pch-msi.c:68
#1  pci_msi_domain_set_desc (arg=0x900000027d77fbb0, desc=0x900000027ce03a00) at drivers/irqchip/irq-loongson-pch-msi.c:116
#2  0x900000000028e3d0 in msi_domain_alloc_irqs (domain=0x900000027c08c600, dev=0x900000027d41e0a8, nvec=<optimized out>) at kernel/irq/msi.c:415
#3  0x90000000008e1840 in msix_capability_init (affd=<optimized out>, nvec=<optimized out>, entries=<optimized out>, dev=<optimized out>) at drivers/pci/msi.c:759
#4  __pci_enable_msix (affd=<optimized out>, nvec=<optimized out>, entries=<optimized out>, dev=<optimized out>) at drivers/pci/msi.c:967
#5  __pci_enable_msix_range (affd=<optimized out>, maxvec=<optimized out>, minvec=<optimized out>, entries=<optimized out>, dev=<optimized out>) at drivers/pci/msi.c:11
00
#6  __pci_enable_msix_range (dev=0x900000027d41e000, entries=0x900000027d83acc0, minvec=3, maxvec=3, affd=0x0) at drivers/pci/msi.c:1081
#7  0x9000000000b70bac in e1000e_set_interrupt_capability (adapter=0x900000027d77fbb0) at drivers/net/ethernet/intel/e1000e/netdev.c:2132
#8  0x9000000000b751cc in e1000_sw_init (adapter=<optimized out>) at drivers/net/ethernet/intel/e1000e/netdev.c:4482
#9  e1000_probe (pdev=0x900000027d41e000, ent=<optimized out>) at drivers/net/ethernet/intel/e1000e/netdev.c:7222
#10 0x90000000008c28c0 in local_pci_probe (_ddi=0x900000027cd33c58) at drivers/pci/pci-driver.c:306
#11 0x9000000000235030 in work_for_cpu_fn (work=0x900000027cd33c08) at kernel/workqueue.c:4908
#12 0x9000000000238ce0 in process_one_work (worker=0x900000027d348480, work=0x900000027cd33c08) at kernel/workqueue.c:2152
#13 0x9000000000239114 in worker_thread (__worker=0x900000027d348480) at kernel/workqueue.c:2295
#14 0x900000000023fc20 in kthread (_create=0x900000027d345200) at kernel/kthread.c:259
#15 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```

## reference
drivers/net/ethernet/intel/e1000e/netdev.c 中注册函数。

arch/x86/kernel/apic/msi.c:`irq_msi_compose_msg` 中分析 x86 的 msi 组装。

其中只有低两位是 vector number:
```c
        msg->data =
                MSI_DATA_TRIGGER_EDGE |
                MSI_DATA_LEVEL_ASSERT |
                ((apic->irq_delivery_mode != dest_LowestPrio) ?
                        MSI_DATA_DELIVERY_FIXED :
                        MSI_DATA_DELIVERY_LOWPRI) |
                MSI_DATA_VECTOR(cfg->vector);

```

这是是 latx 中直接运行代码:
```txt
/ # cat /proc/interrupts
           CPU0
  0:       6396   IO-APIC   2-edge      timer
  1:         10   IO-APIC   1-edge      i8042
  4:        186   IO-APIC   4-edge      serial
  8:          1   IO-APIC   8-edge      rtc0
  9:          0   IO-APIC   9-fasteoi   acpi
 12:        126   IO-APIC  12-edge      i8042
 14:          0   IO-APIC  14-edge      ata_piix
 15:         11   IO-APIC  15-edge      ata_piix
 24:         13   PCI-MSI 49152-edge      eth0-rx-0
 25:         10   PCI-MSI 49153-edge      eth0-tx-0
 26:          2   PCI-MSI 49154-edge      eth0
NMI:          0   Non-maskable interrupts
LOC:       3995   Local timer interrupts
SPU:          0   Spurious interrupts
PMI:          0   Performance monitoring interrupts
IWI:          0   IRQ work interrupts
RTR:          0   APIC ICR read retries
RES:          0   Rescheduling interrupts
CAL:          0   Function call interrupts
TLB:          0   TLB shootdowns
TRM:          0   Thermal event interrupts
THR:          0   Threshold APIC interrupts
DFR:          0   Deferred Error APIC interrupts
MCE:          0   Machine check exceptions
MCP:          0   Machine check polls
ERR:          0
MIS:          0
PIN:          0   Posted-interrupt notification event
PIW:          0   Posted-interrupt wakeup event
[   30.686202] cat (953) used greatest stack depth: 6188 bytes left
```
centos-qemu 中运行 loongarch kernel

```txt
loongson@loongson-pc:~$ cat /proc/interrupts
            CPU0
  17:          0  PCH-PIC-EXT    4  acpi
  18:          0  PCH-PIC-EXT    3  ls2x-rtc alarm
  19:       1139  PCH-PIC-EXT    2  ttyS0
  21:          0  PCH-MSI-EXT   32  virtio0-config
  22:       1684  PCH-MSI-EXT   33  virtio0-req.0
  24:          8  PCH-MSI-EXT   34  enp0s2-rx-0
  25:         11  PCH-MSI-EXT   35  enp0s2-tx-0
  26:          1  PCH-MSI-EXT   36  enp0s2
  61:       1215  COREINTC   11  timer
IPI0:          0       Rescheduling interrupts
IPI1:          0       Call Function interrupts
 ERR:          0
```

latx 运行 32bit x86
```txt
   00000000feb80000-00000000feb9ffff (prio 1, i/o): e1000e-mmio
      00000000feba0000-00000000febbffff (prio 1, i/o): e1000e-flash
      00000000febd0000-00000000febd3fff (prio 1, i/o): e1000e-msix
        00000000febd0000-00000000febd004f (prio 0, i/o): msix-table
        00000000febd2000-00000000febd2007 (prio 0, i/o): msix-pba
```

centos-qemu 运行 loongarch 的
```txt
    0000000042040000-000000004205ffff (prio 1, i/o): e1000e-mmio
    0000000042060000-000000004207ffff (prio 1, i/o): e1000e-flash
    0000000042080000-0000000042083fff (prio 1, i/o): e1000e-msix
      0000000042080000-000000004208004f (prio 0, i/o): msix-table
      0000000042082000-0000000042082007 (prio 0, i/o): msix-pba
```
两个完全对应的，所以。

## `msg_address`

- `pch_msi_compose_msi_msg` 构建的地址 X
- 告诉 PCIe 设备当想要发送 PCIe 中断的时候，会写地址空间中的 X 写对应的信息

```txt
#0  pch_msi_compose_msi_msg (data=0x900000027da57e28, msg=0x900000027d9cbb48) at drivers/irqchip/irq-loongson-pch-msi.c:48
#1  0x9000000000287de8 in irq_chip_compose_msi_msg (data=<optimized out>, msg=<optimized out>) at kernel/irq/chip.c:1425
#2  0x900000000028de48 in msi_domain_activate (domain=<optimized out>, irq_data=0x900000027da57e28, early=<optimized out>) at kernel/irq/msi.c:123
#3  0x900000000028a158 in __irq_domain_activate_irq (irqd=0x900000027da57e28, reserve=false) at kernel/irq/irqdomain.c:1600
#4  0x900000000028c65c in irq_domain_activate_irq (irq_data=0x900000027da57e28, reserve=<optimized out>) at kernel/irq/irqdomain.c:1623
#5  0x900000000028e560 in msi_domain_alloc_irqs (domain=0x900000027c08c600, dev=0x900000027d5320a8, nvec=<optimized out>) at kernel/irq/msi.c:458
#6  0x90000000008e1840 in msix_capability_init (affd=<optimized out>, nvec=<optimized out>, entries=<optimized out>, dev=<optimized out>) at drivers/pci/msi.c:759
#7  __pci_enable_msix (affd=<optimized out>, nvec=<optimized out>, entries=<optimized out>, dev=<optimized out>) at drivers/pci/msi.c:967
#8  __pci_enable_msix_range (affd=<optimized out>, maxvec=<optimized out>, minvec=<optimized out>, entries=<optimized out>, dev=<optimized out>) at drivers/pci/msi.c:11
00
#9  __pci_enable_msix_range (dev=0x900000027d532000, entries=0x900000027d976cc0, minvec=3, maxvec=3, affd=0x0) at drivers/pci/msi.c:1081
#10 0x9000000000b70bac in e1000e_set_interrupt_capability (adapter=0x900000027da57e28) at drivers/net/ethernet/intel/e1000e/netdev.c:2132
#11 0x9000000000b751cc in e1000_sw_init (adapter=<optimized out>) at drivers/net/ethernet/intel/e1000e/netdev.c:4482
#12 e1000_probe (pdev=0x900000027d532000, ent=<optimized out>) at drivers/net/ethernet/intel/e1000e/netdev.c:7222
#13 0x90000000008c28c0 in local_pci_probe (_ddi=0x900000027cd33c58) at drivers/pci/pci-driver.c:306
#14 0x9000000000235030 in work_for_cpu_fn (work=0x900000027cd33c08) at kernel/workqueue.c:4908
#15 0x9000000000238ce0 in process_one_work (worker=0x900000027d440480, work=0x900000027cd33c08) at kernel/workqueue.c:2152
#16 0x9000000000239114 in worker_thread (__worker=0x900000027d440480) at kernel/workqueue.c:2295
#17 0x900000000023fc20 in kthread (_create=0x900000027d43d200) at kernel/kthread.c:259
#18 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```

```c
void __pci_write_msi_msg(struct msi_desc *entry, struct msi_msg *msg)
{
        struct pci_dev *dev = msi_desc_to_pci_dev(entry);

        if (dev->current_state != PCI_D0 || pci_dev_is_disconnected(dev)) {
                /* Don't touch the hardware now */
        } else if (entry->msi_attrib.is_msix) {
                void __iomem *base = pci_msix_desc_addr(entry);

                writel(msg->address_lo, base + PCI_MSIX_ENTRY_LOWER_ADDR);
                writel(msg->address_hi, base + PCI_MSIX_ENTRY_UPPER_ADDR);
                writel(msg->data, base + PCI_MSIX_ENTRY_DATA);
```

```txt
    000000002ff00000-000000002ff00007 (prio 0, i/o): ls3a_msi
```

这些地址是的生成方法： `msix_capability_init` => `msix_map_region`
```txt
[    1.297477] [huxueshi:__pci_write_msi_msg:306] ffff800002ff8000
[    1.298276] [huxueshi:__pci_write_msi_msg:306] ffff800002ff8010
[    1.299073] [huxueshi:__pci_write_msi_msg:306] ffff800002ff8020

[    1.279100] [huxueshi:__pci_write_msi_msg:306] ffff80000b1bd000
[    1.279897] [huxueshi:__pci_write_msi_msg:306] ffff80000b1bd010
```
那么最后，这个地址是什么的?

`pch_msi_compose_msi_msg` 的 address 是 :
```txt
$2 = 0x2ff00000
```

## `ls3a_msi`
将 `ls3a_msi` 的地址放到 msi 中了
```c
static uint64_t ls3a_msi_mem_read(void *opaque, hwaddr addr, unsigned size)
{
    return 0;
}

static void ls3a_msi_mem_write(void *opaque, hwaddr addr,
                               uint64_t val, unsigned size)
{
    struct kvm_msi msi;
    apicState *apic;

    apic = (apicState *)opaque;
    msi.address_lo = 0;
    msi.address_hi = 0;
    msi.data = val & 0xff;
    msi.flags = 0;
    memset(msi.pad, 0, sizeof(msi.pad));

    if (kvm_irqchip_in_kernel()) {
        kvm_vm_ioctl(kvm_state, KVM_SIGNAL_MSI, &msi);
    } else {
        qemu_set_irq(apic->irq[msi.data], 1);
    }
}

static const MemoryRegionOps ls3a_msi_ops = {
    .read  = ls3a_msi_mem_read,
    .write = ls3a_msi_mem_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};
```

好吧，我的回忆全都回来了:
```txt
#0  0x00000001201af030 in ls3a_msi_mem_write (opaque=0xe8014b95da11cb00, addr=1098974949024, val=804257792, size=255) at /home/loongson/core/centos-qemu/hw/loongarch/la
rch_3a.c:1015
#1  0x00000001200d8114 in memory_region_write_accessor (mr=0x120e86860, addr=0, value=0xffe7ffdc48, size=4, shift=0, mask=4294967295, attrs=...) at /home/loongson/core/
centos-qemu/memory.c:483
#2  0x00000001200d83f0 in access_with_adjusted_size (addr=0, value=0xffe7ffdc48, size=4, access_size_min=1, access_size_max=4, access_fn=0x1200d8024 <memory_region_writ
e_accessor>, mr=0x120e86860, attrs=...) at /home/loongson/core/centos-qemu/memory.c:544
#3  0x00000001200db830 in memory_region_dispatch_write (mr=0x120e86860, addr=0, data=36, op=MO_32, attrs=...) at /home/loongson/core/centos-qemu/memory.c:1475
#4  0x0000000120067bc4 in address_space_stl_internal (as=0xfff4004228, addr=804257792, val=36, attrs=..., result=0x0, endian=DEVICE_LITTLE_ENDIAN) at /home/loongson/cor
e/centos-qemu/memory_ldst.inc.c:315
#5  0x0000000120067d5c in address_space_stl_le (as=0xfff4004228, addr=804257792, val=36, attrs=..., result=0x0) at /home/loongson/core/centos-qemu/memory_ldst.inc.c:353
#6  0x00000001203ba598 in msi_send_message (dev=0xfff4004010, msg=...) at /home/loongson/core/centos-qemu/hw/pci/msi.c:340
#7  0x00000001203b8a90 in msix_notify (dev=0xfff4004010, vector=2) at /home/loongson/core/centos-qemu/hw/pci/msix.c:493
#8  0x00000001203921b4 in e1000e_msix_notify_one (core=0xfff4006ce0, cause=16777216, int_cfg=32778) at /home/loongson/core/centos-qemu/hw/net/e1000e_core.c:2008
#9  0x00000001203923f0 in e1000e_msix_notify (core=0xfff4006ce0, causes=16777220) at /home/loongson/core/centos-qemu/hw/net/e1000e_core.c:2058
#10 0x0000000120392728 in e1000e_send_msi (core=0xfff4006ce0, msix=true) at /home/loongson/core/centos-qemu/hw/net/e1000e_core.c:2132
#11 0x00000001203928d4 in e1000e_update_interrupt_state (core=0xfff4006ce0) at /home/loongson/core/centos-qemu/hw/net/e1000e_core.c:2177
#12 0x00000001203929b0 in e1000e_set_interrupt_cause (core=0xfff4006ce0, val=16777220) at /home/loongson/core/centos-qemu/hw/net/e1000e_core.c:2200
#13 0x00000001203936a0 in e1000e_set_ics (core=0xfff4006ce0, index=50, val=16777220) at /home/loongson/core/centos-qemu/hw/net/e1000e_core.c:2459
#14 0x00000001203949a0 in e1000e_core_write (core=0xfff4006ce0, addr=200, val=16777220, size=4) at /home/loongson/core/centos-qemu/hw/net/e1000e_core.c:3256
#15 0x0000000120381b4c in e1000e_mmio_write (opaque=0xfff4004010, addr=200, val=16777220, size=4) at /home/loongson/core/centos-qemu/hw/net/e1000e.c:118
#16 0x00000001200d8114 in memory_region_write_accessor (mr=0xfff4006910, addr=200, value=0xffe7ffdfc8, size=4, shift=0, mask=4294967295, attrs=...) at /home/loongson/co
re/centos-qemu/memory.c:483
#17 0x00000001200d83f0 in access_with_adjusted_size (addr=200, value=0xffe7ffdfc8, size=4, access_size_min=4, access_size_max=4, access_fn=0x1200d8024 <memory_region_wr
ite_accessor>, mr=0xfff4006910, attrs=...) at /home/loongson/core/centos-qemu/memory.c:544
#18 0x00000001200db830 in memory_region_dispatch_write (mr=0xfff4006910, addr=200, data=16777220, op=MO_32, attrs=...) at /home/loongson/core/centos-qemu/memory.c:1475
#19 0x0000000120065a68 in flatview_write_continue (fv=0xffe002eea0, addr=1107558600, attrs=..., buf=0xfff7fcc028 "\004", len=4, addr1=200, l=4, mr=0xfff4006910) at /hom
e/loongson/core/centos-qemu/exec.c:3129
#20 0x0000000120065c00 in flatview_write (fv=0xffe002eea0, addr=1107558600, attrs=..., buf=0xfff7fcc028 "\004", len=4) at /home/loongson/core/centos-qemu/exec.c:3169
#21 0x0000000120066020 in address_space_write (as=0x120b3ce88 <address_space_memory>, addr=1107558600, attrs=..., buf=0xfff7fcc028 "\004", len=4) at /home/loongson/core
/centos-qemu/exec.c:3259
#22 0x00000001200660b4 in address_space_rw (as=0x120b3ce88 <address_space_memory>, addr=1107558600, attrs=..., buf=0xfff7fcc028 "\004", len=4, is_write=true) at /home/l
oongson/core/centos-qemu/exec.c:3269
#23 0x00000001200f9540 in kvm_cpu_exec (cpu=0x120c9eaf0) at /home/loongson/core/centos-qemu/accel/kvm/kvm-all.c:2386
#24 0x00000001200c5d20 in qemu_kvm_cpu_thread_fn (arg=0x120c9eaf0) at /home/loongson/core/centos-qemu/cpus.c:1318
#25 0x00000001206e2378 in qemu_thread_start (args=0x120ccdee0) at /home/loongson/core/centos-qemu/util/qemu-thread-posix.c:519
#26 0x000000fff754489c in start_thread () at /lib/loongarch64-linux-gnu/libpthread.so.0
#27 0x000000fff74aafe4 in  () at /lib/loongarch64-linux-gnu/libc.so.6
```

## x86 的 `irq_msi_compose_msg` 中的地址使用的是 apic

```txt
#0  0x000000aaab1db304 in apic_mem_write (opaque=0xaaabe11c00, addr=4108, val=16705, size=4) at ../hw/intc/apic.c:745
#1  0x000000aaab1bfaa0 in memory_region_write_accessor (mr=0xaaabe11c90, addr=4108, value=<optimized out>, size=<optimized out>, shift=<optimized out>, mask=<optimized9
#2  0x000000aaab1be1d4 in access_with_adjusted_size (addr=addr@entry=4108, value=value@entry=0xfff5461838, size=size@entry=4, access_size_min=<optimized out>, access_s0
#3  0x000000aaab1c2114 in memory_region_dispatch_write (mr=mr@entry=0xaaabe11c90, addr=4108, data=<optimized out>, data@entry=16705, op=op@entry=MO_32, attrs=attrs@ent0
#4  0x000000aaab19d258 in address_space_stl_internal (endian=DEVICE_LITTLE_ENDIAN, result=0x0, attrs=..., val=16705, addr=<optimized out>, as=<optimized out>) at /home6
#5  0x000000aaab19d258 in address_space_stl_le (as=<optimized out>, addr=4276097036, addr@entry=1048576, val=<optimized out>, attrs=..., result=result@entry=0x0) at /h7
#6  0x000000aaaaf796f4 in msi_send_message (dev=<optimized out>, msg=...) at ../hw/pci/msi.c:340
#7  0x000000aaaaf28630 in msix_notify (vector=<optimized out>, dev=<optimized out>) at ../hw/pci/msix.c:503
#8  0x000000aaaaf28630 in msix_notify (dev=<optimized out>, vector=vector@entry=0) at ../hw/pci/msix.c:488
#9  0x000000aaaafa58f8 in e1000e_msix_notify_one (core=core@entry=0xfff431ad00, cause=cause@entry=1048576, int_cfg=<optimized out>) at ../hw/net/e1000e_core.c:2001
#10 0x000000aaaafa62f8 in e1000e_msix_notify (causes=1048576, core=0xfff431ad00) at ../hw/net/e1000e_core.c:2031
#11 0x000000aaaafa62f8 in e1000e_send_msi (msix=true, core=0xfff431ad00) at ../hw/net/e1000e_core.c:2125
#12 0x000000aaaafa62f8 in e1000e_update_interrupt_state (core=core@entry=0xfff431ad00) at ../hw/net/e1000e_core.c:2170
#13 0x000000aaaafaaaf4 in e1000e_set_interrupt_cause (val=1048576, core=<optimized out>) at ../hw/net/e1000e_core.c:2193
#14 0x000000aaaafaaaf4 in e1000e_receive_iov (core=0xfff431ad00, iov=<optimized out>, iovcnt=<optimized out>) at ../hw/net/e1000e_core.c:1731
#15 0x000000aaab0312d0 in qemu_deliver_packet_iov (sender=<optimized out>, opaque=0xaaacc15390, iovcnt=1, iov=0xfff5461c98, flags=0) at ../net/net.c:761
#16 0x000000aaab0312d0 in qemu_deliver_packet_iov (sender=<optimized out>, flags=<optimized out>, iov=0xfff5461c98, iovcnt=<optimized out>, opaque=0xaaacc15390) at ../2
#17 0x000000aaaae88698 in qemu_net_queue_deliver (size=590, data=0xfff5461e20 "\377\377\377\377\377\377RU\n", flags=0, sender=0xaaabedbf20, queue=0xaaacc15550) at ../n4
#18 0x000000aaaae88698 in qemu_net_queue_send (queue=0xaaacc15550, sender=sender@entry=0xaaabedbf20, flags=flags@entry=0, data=data@entry=0xfff5461e20 "\377\377\377\371
#19 0x000000aaab031528 in qemu_send_packet_async_with_flags (sender=0xaaabedbf20, flags=<optimized out>, buf=0xfff5461e20 "\377\377\377\377\377\377RU\n", size=<optimiz0
#20 0x000000aaaae98af8 in net_slirp_send_packet (pkt=0xfff5461e20, pkt_len=590, opaque=0xaaabedbf20) at ../net/slirp.c:129
#21 0x000000aaab34eea4 in slirp_send_packet_all (slirp=<optimized out>, buf=<optimized out>, len=590) at ../slirp/src/slirp.c:1181
#22 0x000000aaab34f52c in if_encap (slirp=slirp@entry=0xaaabedc140, ifm=ifm@entry=0xffeca59a50) at ../slirp/src/slirp.c:985
#23 0x000000aaab35a704 in if_start (slirp=0xaaabedc140) at ../slirp/src/if.c:183
#24 0x000000aaab35d6d8 in ip_output (so=so@entry=0x0, m0=m0@entry=0xffeca59a50) at ../slirp/src/ip_output.c:81
#25 0x000000aaab356d04 in udp_output (so=so@entry=0x0, m=m@entry=0xffeca59a50, saddr=saddr@entry=0xfff5471fb0, daddr=daddr@entry=0xfff5471fc0, iptos=iptos@entry=16) at6
#26 0x000000aaab3592f8 in bootp_reply (bp=<optimized out>, slirp=<optimized out>) at ../slirp/src/bootp.c:359
#27 0x000000aaab3592f8 in bootp_input (m=m@entry=0xffeca593e0) at ../slirp/src/bootp.c:367
#28 0x000000aaab357094 in udp_input (m=0xffeca593e0, iphlen=20) at ../slirp/src/udp.c:143
#29 0x000000aaab34f1cc in slirp_input (slirp=<optimized out>, pkt=0xffec1cdc00 "\377\377\377\377\377\377RT", pkt_len=<optimized out>) at ../slirp/src/slirp.c:845
#30 0x000000aaaae98b98 in net_slirp_receive (nc=<optimized out>, buf=<optimized out>, size=342) at ../net/slirp.c:136
#31 0x000000aaab0313c0 in nc_sendv_compat (flags=<optimized out>, iovcnt=4, iov=0xaaacc25930, nc=0xaaabedbf20) at ../net/net.c:735
#32 0x000000aaab0313c0 in qemu_deliver_packet_iov (sender=<optimized out>, opaque=0xaaabedbf20, iovcnt=4, iov=0xaaacc25930, flags=<optimized out>) at ../net/net.c:763
#33 0x000000aaab0313c0 in qemu_deliver_packet_iov (sender=<optimized out>, flags=<optimized out>, iov=0xaaacc25930, iovcnt=<optimized out>, opaque=0xaaabedbf20) at ../2
#34 0x000000aaaae88798 in qemu_net_queue_deliver_iov (iovcnt=4, iov=0xaaacc25930, flags=0, sender=0xaaacc15390, queue=0xaaabedc100) at ../net/queue.c:179
#35 0x000000aaaae88798 in qemu_net_queue_send_iov (queue=0xaaabedc100, sender=0xaaacc15390, flags=<optimized out>, iov=0xaaacc25930, iovcnt=<optimized out>, sent_cb=0x6
#36 0x000000aaab00dfb8 in net_tx_pkt_sendv (pkt=0xaaacc158c0, iov_cnt=<optimized out>, iov=<optimized out>, nc=0xaaacc15390) at ../hw/net/net_tx_pkt.c:558
#37 0x000000aaab00dfb8 in net_tx_pkt_send (nc=0xaaacc15390, pkt=0xaaacc158c0) at ../hw/net/net_tx_pkt.c:633
#38 0x000000aaab00dfb8 in net_tx_pkt_send (pkt=0xaaacc158c0, nc=nc@entry=0xaaacc15390) at ../hw/net/net_tx_pkt.c:609
#39 0x000000aaaafa8ec8 in e1000e_tx_pkt_send (queue_index=<optimized out>, tx=0xfff433af68, core=0xfff431ad00) at ../hw/net/e1000e_core.c:659
#40 0x000000aaaafa8ec8 in e1000e_process_tx_desc (queue_index=<optimized out>, dp=0xfff54722e8, tx=0xfff433af68, core=0xfff431ad00) at ../hw/net/e1000e_core.c:736
#41 0x000000aaaafa8ec8 in e1000e_start_xmit (core=0xfff431ad00, txr=<optimized out>, txr=<optimized out>) at ../hw/net/e1000e_core.c:927
#42 0x000000aaaafa92e8 in e1000e_set_tdt (core=<optimized out>, index=<optimized out>, val=<optimized out>) at ../hw/net/e1000e_core.c:2444
#43 0x000000aaaafac294 in e1000e_core_write (core=0xfff431ad00, addr=<optimized out>, val=1, size=<optimized out>) at ../hw/net/e1000e_core.c:3256
#44 0x000000aaab1bfaa0 in memory_region_write_accessor (mr=0xfff431a930, addr=14360, value=<optimized out>, size=<optimized out>, shift=<optimized out>, mask=<optimize9
#45 0x000000aaab1be1d4 in access_with_adjusted_size (addr=addr@entry=14360, value=value@entry=0xfff54724f8, size=size@entry=4, access_size_min=<optimized out>, access_0
#46 0x000000aaab1c2114 in memory_region_dispatch_write (mr=mr@entry=0xfff431a930, addr=addr@entry=14360, data=<optimized out>, data@entry=1, op=op@entry=MO_32, attrs=.0
#47 0x000000aaab234548 in io_writex (env=0xfff557c8d0, mmu_idx=<optimized out>, val=1, addr=<optimized out>, retaddr=1097012996328, op=<optimized out>, iotlbentry=<opt3
#48 0x000000ff6b11e9bc in code_gen_buffer ()
#49 0x0000000000000000 in  ()
```

在 lat 的 qemu 中截获的两个数据:
```txt
[huxueshi:apic_mem_write:755] addr=1004 data=134
[huxueshi:apic_mem_write:755] addr=100c data=4141
```
如果 apic 的中断和 MSI 的中断混合编码在这个地方，很难的啊

```txt
[   19.424030] [huxueshi:irq_msi_compose_msg:55] fee0100c 4141
[   19.424055] [huxueshi:irq_msi_compose_msg:55] fee0100c 4141
[   19.425579] [huxueshi:irq_msi_compose_msg:55] fee0100c 4151
[   19.425861] [huxueshi:irq_msi_compose_msg:55] fee0100c 4151
[   19.426499] [huxueshi:irq_msi_compose_msg:55] fee0100c 4161
[   19.426787] [huxueshi:irq_msi_compose_msg:55] fee0100c 4161

[    9.083339] [huxueshi:irq_msi_compose_msg:55] fee0100c 4141
[    9.084110] [huxueshi:irq_msi_compose_msg:55] fee0100c 4151
[    9.084351] [huxueshi:irq_msi_compose_msg:55] fee0100c 4161
```
这个东西和这个的对应关系 ?
```txt
 24:         32   PCI-MSI 49152-edge      eth0-rx-0
 25:         10   PCI-MSI 49153-edge      eth0-tx-0
 26:          2   PCI-MSI 49154-edge      eth0
```

现在似乎两个架构中间，都是从某一个数值开始的哦!

- [x] 这里的 41 51 61 应该是 idt 编号吧！
  - 只要将所有的 msix table 的位置全部记录下来，然后就可以知道到时候，中断到底注入到什么，到时候，应该拉高什么中断了。
- [x] `irq_msi_compose_msg` 中的 `cfg->vector` 是如何获取的, 为什么分配的总是 41 51 和 61 ?
    - 因为 `__assign_irq_vector` 中，应该是出于 cpumask 之类的操作，每次都是 irq += 16 的

## loongarch 和 `compose_msi_msg` 和 x86 的不同，但是无所谓

loongarch 的 msg 和 x86 的 arch/x86/kernel/apic/msi.c:`irq_msi_compose_msg` 的内容就很不一样:
```c
static void pch_msi_compose_msi_msg(struct irq_data *data,
                                        struct msi_msg *msg)
{
        struct pch_msi_data *priv;
        struct msi_domain_info *info = (struct msi_domain_info *)data->domain->host_data;
        priv = (struct pch_msi_data *)info->data;

        msg->address_hi = priv->msg_address >> 32;
        msg->address_lo = priv->msg_address;
        msg->data = data->hwirq;
}
```

### 从 `pci_host.c` 看，只能访问 256K 的配置空间，但是 PCIe 要求访问 2K 的配置空间

从 QEMU 看，两者的编码空间显然不同
```c
/*
 * PCI express ECAM (Enhanced Configuration Address Mapping) format.
 * AKA mmcfg address
 * bit 20 - 28: bus number
 * bit 15 - 19: device number
 * bit 12 - 14: function number
 * bit  0 - 11: offset in configuration space of a given device
 */
#define PCIE_MMCFG_SIZE_MAX             (1ULL << 29)
#define PCIE_MMCFG_SIZE_MIN             (1ULL << 20)
#define PCIE_MMCFG_BUS_BIT              20
#define PCIE_MMCFG_BUS_MASK             0x1ff
#define PCIE_MMCFG_DEVFN_BIT            12
#define PCIE_MMCFG_DEVFN_MASK           0xff
#define PCIE_MMCFG_CONFOFFSET_MASK      0xfff
#define PCIE_MMCFG_BUS(addr)            (((addr) >> PCIE_MMCFG_BUS_BIT) & \
                                         PCIE_MMCFG_BUS_MASK)
#define PCIE_MMCFG_DEVFN(addr)          (((addr) >> PCIE_MMCFG_DEVFN_BIT) & \
                                         PCIE_MMCFG_DEVFN_MASK)
#define PCIE_MMCFG_CONFOFFSET(addr)     ((addr) & PCIE_MMCFG_CONFOFFSET_MASK)
```

```c
/*
 * PCI address
 * bit 16 - 24: bus number
 * bit  8 - 15: devfun number
 * bit  0 -  7: offset in configuration space of a given pci device
 */

/* the helper function to get a PCIDevice* for a given pci address */
static inline PCIDevice *pci_dev_find_by_addr(PCIBus *bus, uint32_t addr)
{
    uint8_t bus_num = addr >> 16;
    uint8_t devfn = addr >> 8;

    return pci_find_device(bus, bus_num, devfn);
}
```

从 https://en.wikipedia.org/wiki/PCI_configuration_space 才知道

`CONFIG_ADDRESS` 的编码发生了如下改变:
```txt
0x80000000 | (offset & 0xf00) << 16 | bus << 16 | device << 11 | function <<  8 | (offset & 0xff)
```
- [ ] 这个想法需要在 64bit x64 kernel 中验证一下

## 为什么最开始在 bios 中，msix table 的地址是 42040000

```txt
[huxueshi:insert_msix_table:84] msix table added try 42040000 4
```

这个应该是 loongarch  bios 中的数值了。

## 在 QEMU 中，就是 `pch_pic` 实际上被叫做 ioapic 的
```txt
    0000000010000000-0000000010000fff (prio 0, i/o): ioapic
    0000000010002000-0000000010002013 (prio 1, i/o): 0x10002000
```

## `pch_pic` 和 msi 都需要调用 `ext_set_irq_affinity`

```txt
#0  ext_set_irq_affinity (d=0x900000027cf4c880, affinity=0x900000027cf98e18, force=false) at drivers/irqchip/irq-loongarch-extioi.c:110
#1  0x900000000029ed58 in irq_do_set_affinity (data=0x900000027cf98e28, mask=0x900000027cf98e18, force=<optimized out>) at kernel/irq/manage.c:230
#2  0x90000000002a2234 in irq_startup (desc=0x900000027cf98e00, resend=true, force=<optimized out>) at kernel/irq/chip.c:272
#3  0x900000000029fd14 in __setup_irq (irq=21, desc=0x900000027cf98e00, new=0x900000027d267a80) at kernel/irq/manage.c:1510
#4  0x900000000029ff18 in request_threaded_irq (irq=2096416896, handler=0x900000027cf98e18, thread_fn=0x0, irqflags=<optimized out>, devname=0x40 <error: Cannot access
memory at address 0x40>, dev_id=0x900000027d42e800) at kernel/irq/manage.c:1931
#5  0x900000000097e6a0 in request_irq (dev=<optimized out>, name=<optimized out>, flags=<optimized out>, handler=<optimized out>, irq=<optimized out>) at ./include/linu
x/interrupt.h:147
#6  vp_request_msix_vectors (desc=<optimized out>, per_vq_vectors=<optimized out>, nvectors=<optimized out>, vdev=<optimized out>) at drivers/virtio/virtio_pci_common.c
:146
#7  vp_find_vqs_msix (vdev=0x900000027d42e800, nvqs=1, vqs=<optimized out>, callbacks=0x900000027cf4c640, names=0x900000027cf4c600, per_vq_vectors=true, ctx=0x0, desc=0
x900000027cd33b98) at drivers/virtio/virtio_pci_common.c:307
#8  0x900000000097ea00 in vp_find_vqs (vdev=0x900000027d42e800, nvqs=1, vqs=0x900000027cf4c680, callbacks=0x900000027cf4c640, names=0x900000027cf4c600, ctx=0x0, desc=0x
900000027cd33b98) at drivers/virtio/virtio_pci_common.c:403
#9  0x900000000097d4d4 in vp_modern_find_vqs (vdev=0x900000027d42e800, nvqs=<optimized out>, vqs=<optimized out>, callbacks=<optimized out>, names=<optimized out>, ctx=
<optimized out>, desc=<optimized out>) at drivers/virtio/virtio_pci_modern.c:413
#10 0x9000000000a6cc4c in virtio_find_vqs (desc=<optimized out>, names=<optimized out>, callbacks=<optimized out>, vqs=<optimized out>, nvqs=<optimized out>, vdev=<opti
mized out>) at ./include/linux/virtio_config.h:192
#11 init_vq (vblk=0x900000027d43d800) at drivers/block/virtio_blk.c:542
#12 0x9000000000a6dd00 in virtblk_probe (vdev=0x900000027cf4c880) at drivers/block/virtio_blk.c:774
#13 0x900000000097a778 in virtio_dev_probe (_d=0x900000027d42e810) at drivers/virtio/virtio.c:245
#14 0x9000000000a4438c in really_probe (dev=0x900000027d42e810, drv=0x900000000145c340 <virtio_blk>) at drivers/base/dd.c:506
#15 0x9000000000a445c4 in driver_probe_device (drv=0x900000000145c340 <virtio_blk>, dev=0x900000027d42e810) at drivers/base/dd.c:667
#16 0x9000000000a4476c in __driver_attach (data=<optimized out>, dev=<optimized out>) at drivers/base/dd.c:903
#17 __driver_attach (dev=0x900000027d42e810, data=0x900000000145c340 <virtio_blk>) at drivers/base/dd.c:872
#18 0x9000000000a420c0 in bus_for_each_dev (bus=<optimized out>, start=<optimized out>, data=0x0, fn=0x0) at drivers/base/bus.c:279
#19 0x9000000000a43bd0 in driver_attach (drv=<optimized out>) at drivers/base/dd.c:922
#20 0x9000000000a4363c in bus_add_driver (drv=0x900000000145c340 <virtio_blk>) at drivers/base/bus.c:672
#21 0x9000000000a453b0 in driver_register (drv=0x900000000145c340 <virtio_blk>) at drivers/base/driver.c:170
#22 0x900000000097a154 in register_virtio_driver (driver=<optimized out>) at drivers/virtio/virtio.c:296
#23 0x9000000001517040 in init () at drivers/block/virtio_blk.c:1019
#24 0x9000000000200b8c in do_one_initcall (fn=0x9000000001516fc8 <init>) at init/main.c:884
#25 0x90000000014cce8c in do_initcall_level (level=<optimized out>) at ./include/linux/init.h:131
#26 do_initcalls () at init/main.c:960
#27 do_basic_setup () at init/main.c:978
#28 kernel_init_freeable () at init/main.c:1145
#29 0x9000000000f94010 in kernel_init (unused=<optimized out>) at init/main.c:1062
#30 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```

irq=21 什么时候分配的
```txt
#0  extioi_domain_alloc (domain=0x900000027c024800, virq=21, nr_irqs=1, arg=0x900000027cd338f0) at drivers/irqchip/irq-loongarch-extioi.c:303
#1  0x90000000002a8ba0 in msi_domain_alloc (domain=0x900000027c08c600, virq=21, nr_irqs=<optimized out>, arg=0x900000027cd338f0) at kernel/irq/msi.c:150
#2  0x90000000002a6f1c in irq_domain_alloc_irqs_hierarchy (arg=<optimized out>, nr_irqs=<optimized out>, irq_base=<optimized out>, domain=<optimized out>) at kernel/irq
/irqdomain.c:1270
#3  __irq_domain_alloc_irqs (domain=0x900000027c08c600, irq_base=21, nr_irqs=1, node=<optimized out>, arg=<optimized out>, realloc=<optimized out>, affinity=<optimized
out>) at kernel/irq/irqdomain.c:1326
#4  0x90000000002a93f0 in msi_domain_alloc_irqs (domain=0x900000027c08c600, dev=0x900000027d5140a8, nvec=<optimized out>) at ./include/linux/device.h:1075
#5  0x90000000008fc840 in msix_capability_init (affd=<optimized out>, nvec=<optimized out>, entries=<optimized out>, dev=<optimized out>) at drivers/pci/msi.c:759
#6  __pci_enable_msix (affd=<optimized out>, nvec=<optimized out>, entries=<optimized out>, dev=<optimized out>) at drivers/pci/msi.c:967
#7  __pci_enable_msix_range (affd=<optimized out>, maxvec=<optimized out>, minvec=<optimized out>, entries=<optimized out>, dev=<optimized out>) at drivers/pci/msi.c:11
00
#8  __pci_enable_msix_range (dev=0x900000027d514000, entries=0x0, minvec=2, maxvec=2, affd=0x900000027cd33b98) at drivers/pci/msi.c:1081
#9  0x90000000008fd298 in pci_alloc_irq_vectors_affinity (dev=0x900000027d514000, min_vecs=2, max_vecs=2, flags=12, affd=0x900000027cd33b98) at drivers/pci/msi.c:1170
#10 0x900000000097e63c in vp_request_msix_vectors (desc=<optimized out>, per_vq_vectors=<optimized out>, nvectors=<optimized out>, vdev=<optimized out>) at drivers/virt
io/virtio_pci_common.c:136
#11 vp_find_vqs_msix (vdev=0x900000027d682800, nvqs=1, vqs=<optimized out>, callbacks=0x900000027cee4640, names=0x900000027cee4600, per_vq_vectors=true, ctx=0x0, desc=0
x900000027cd33b98) at drivers/virtio/virtio_pci_common.c:307
#12 0x900000000097ea00 in vp_find_vqs (vdev=0x900000027d682800, nvqs=1, vqs=0x900000027cee4680, callbacks=0x900000027cee4640, names=0x900000027cee4600, ctx=0x0, desc=0x
900000027cd33b98) at drivers/virtio/virtio_pci_common.c:403
#13 0x900000000097d4d4 in vp_modern_find_vqs (vdev=0x900000027d682800, nvqs=<optimized out>, vqs=<optimized out>, callbacks=<optimized out>, names=<optimized out>, ctx=
<optimized out>, desc=<optimized out>) at drivers/virtio/virtio_pci_modern.c:413
#14 0x9000000000a6cc4c in virtio_find_vqs (desc=<optimized out>, names=<optimized out>, callbacks=<optimized out>, vqs=<optimized out>, nvqs=<optimized out>, vdev=<opti
mized out>) at ./include/linux/virtio_config.h:192
#15 init_vq (vblk=0x900000027d221800) at drivers/block/virtio_blk.c:542
#16 0x9000000000a6dd00 in virtblk_probe (vdev=0x900000027c024800) at drivers/block/virtio_blk.c:774
#17 0x900000000097a778 in virtio_dev_probe (_d=0x900000027d682810) at drivers/virtio/virtio.c:245
#18 0x9000000000a4438c in really_probe (dev=0x900000027d682810, drv=0x900000000145c340 <virtio_blk>) at drivers/base/dd.c:506
#19 0x9000000000a445c4 in driver_probe_device (drv=0x900000000145c340 <virtio_blk>, dev=0x900000027d682810) at drivers/base/dd.c:667
#20 0x9000000000a4476c in __driver_attach (data=<optimized out>, dev=<optimized out>) at drivers/base/dd.c:903
#21 __driver_attach (dev=0x900000027d682810, data=0x900000000145c340 <virtio_blk>) at drivers/base/dd.c:872
#22 0x9000000000a420c0 in bus_for_each_dev (bus=<optimized out>, start=<optimized out>, data=0x1, fn=0x900000027cd338f0) at drivers/base/bus.c:279
#23 0x9000000000a43bd0 in driver_attach (drv=<optimized out>) at drivers/base/dd.c:922
#24 0x9000000000a4363c in bus_add_driver (drv=0x900000000145c340 <virtio_blk>) at drivers/base/bus.c:672
#25 0x9000000000a453b0 in driver_register (drv=0x900000000145c340 <virtio_blk>) at drivers/base/driver.c:170
#26 0x900000000097a154 in register_virtio_driver (driver=<optimized out>) at drivers/virtio/virtio.c:296
#27 0x9000000001517040 in init () at drivers/block/virtio_blk.c:1019
#28 0x9000000000200b8c in do_one_initcall (fn=0x9000000001516fc8 <init>) at init/main.c:884
#29 0x90000000014cce8c in do_initcall_level (level=<optimized out>) at ./include/linux/init.h:131
#30 do_initcalls () at init/main.c:960
#31 do_basic_setup () at init/main.c:978
#32 kernel_init_freeable () at init/main.c:1145
#33 0x9000000000f94010 in kernel_init (unused=<optimized out>) at init/main.c:1062
#34 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```

容易找到，是在 `extioi_domain_alloc` 中设置的 `irq_data` 的。
