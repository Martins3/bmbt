好吧，实际上，对于 pch_pic 的理解有很大的问题的
```c
#0  pch_pic_ack_irq (d=0x900000027d8cda28) at drivers/irqchip/irq-loongson-pch-pic.c:185
#1  0x90000000002865fc in mask_ack_irq (desc=<optimized out>) at kernel/irq/chip.c:414
#2  handle_level_irq (desc=0x900000027d8cda00) at kernel/irq/chip.c:633
#3  0x9000000000280f00 in generic_handle_irq_desc (desc=<optimized out>) at ./include/linux/irqdesc.h:155
#4  generic_handle_irq (irq=<optimized out>) at kernel/irq/irqdesc.c:639
#5  0x90000000008a2b58 in extioi_irq_dispatch (desc=<optimized out>) at drivers/irqchip/irq-loongarch-extioi.c:256
#6  0x9000000000280f00 in generic_handle_irq_desc (desc=<optimized out>) at ./include/linux/irqdesc.h:155
#7  generic_handle_irq (irq=<optimized out>) at kernel/irq/irqdesc.c:639
#8  0x9000000000f857f8 in do_IRQ (irq=<optimized out>) at arch/loongarch/kernel/irq.c:103
#9  0x90000000002033d4 in except_vec_vi_handler () at arch/loongarch/kernel/genex.S:92
Backtrace stopped: frame did not save the PC
```
- 实际上，pch_pic 这个中断控制器还是存在的，但是为什么没有 dispatch 的啊

为什么他的 base 是这个东西啊:
```c
                                            0x9000000000f857f8
[    0.000000] pch-pic: huxueshi:pch_pic_init ffff800002010000
```
- [ ] 应该只是虚拟地址的映射成为这个样子吧，这个还是虚拟地址吧！

我已经搞不清楚 ioremap 的样子是怎样的了!
```c
  priv->base = ioremap(addr, size);

$2 = 0x10000000
>>> p/x size
$3 = 0x400

```

```c
pr_info("huxueshi:%s %llx %lx\n", __FUNCTION__, (u64)priv->base, VMALLOC_START);

[    0.000000] pch-pic: huxueshi:pch_pic_init ffff800002010000 ffff800002008000
```
ioremap is based on vmalloc, return val of ioremap is virtual address.

- [x] 可以找到这 pch_pic 的文档吗?
  - 7a1000 manual chapter 5

-> hardcode is enough
```c
    register_pch_pic(0, LS7A_PCH_REG_BASE, LOONGSON_PCH_IRQ_BASE);
    irqchip_init_default();
```

- register_pch_pic
```c
#0  register_pch_pic (id=0, address=268435456, irq_base=64) at arch/loongarch/la64/irq.c:96
#1  0x90000000014ac048 in acpi_parse_pch_pic (header=0x0, end=<optimized out>) at arch/loongarch/kernel/acpi.c:118
#2  0x90000000014e5b30 in acpi_parse_entries_array (max_entries=<optimized out>, proc_num=<optimized out>, proc=<optimized out>, table_header=<optimized out>, table_siz
e=<optimized out>, id=<optimized out>) at drivers/acpi/tables.c:298
#3  acpi_table_parse_entries_array (id=0x0, table_size=<optimized out>, proc=0x900000000134bd58, proc_num=1, max_entries=16) at drivers/acpi/tables.c:353
#4  0x90000000014e5c18 in acpi_table_parse_entries (id=<optimized out>, table_size=<optimized out>, entry_id=<optimized out>, handler=<optimized out>, max_entries=<opti
mized out>) at drivers/acpi/tables.c:372
#5  0x90000000014e5c40 in acpi_table_parse_madt (id=<optimized out>, handler=<optimized out>, max_entries=<optimized out>) at drivers/acpi/tables.c:380
#6  0x90000000014ac348 in acpi_parse_madt_pch_pic_entries () at arch/loongarch/kernel/acpi.c:141
#7  acpi_process_madt () at arch/loongarch/kernel/acpi.c:308
#8  acpi_boot_init () at arch/loongarch/kernel/acpi.c:343
#9  0x90000000014a8620 in platform_init () at arch/loongarch/la64/init.c:474
#10 0x90000000014ab260 in setup_arch (cmdline_p=0x900000000134be58) at arch/loongarch/kernel/setup.c:796
#11 0x90000000014a47c4 in start_kernel () at init/main.c:551
#12 0x9000000000f78ea4 in kernel_entry () at arch/loongarch/kernel/head.S:129
```

## it seems msi doesn't trigger
so, how the corresponding linux irq 22 and 23 are allocatd?
```c
#0  extioi_domain_alloc (domain=0x900000027c024800, virq=22, nr_irqs=1, arg=0x900000027cd338f0) at drivers/irqchip/irq-loongarch-extioi.c:303
#1  0x900000000028dba0 in msi_domain_alloc (domain=0x900000027c08c600, virq=22, nr_irqs=<optimized out>, arg=0x900000027cd338f0) at kernel/irq/msi.c:150
#2  0x900000000028bf1c in irq_domain_alloc_irqs_hierarchy (arg=<optimized out>, nr_irqs=<optimized out>, irq_base=<optimized out>, domain=<optimized out>) at kernel/irq/irqdomain.c:1270
#3  __irq_domain_alloc_irqs (domain=0x900000027c08c600, irq_base=22, nr_irqs=1, node=<optimized out>, arg=<optimized out>, realloc=<optimized out>, affinity=<optimizedout>) at kernel/irq/irqdomain.c:1326
#4  0x900000000028e3f0 in msi_domain_alloc_irqs (domain=0x900000027c08c600, dev=0x900000027d4cc0a8, nvec=<optimized out>) at ./include/linux/device.h:1075
#5  0x90000000008e1820 in msix_capability_init (affd=<optimized out>, nvec=<optimized out>, entries=<optimized out>, dev=<optimized out>) at drivers/pci/msi.c:759
#6  __pci_enable_msix (affd=<optimized out>, nvec=<optimized out>, entries=<optimized out>, dev=<optimized out>) at drivers/pci/msi.c:967
#7  __pci_enable_msix_range (affd=<optimized out>, maxvec=<optimized out>, minvec=<optimized out>, entries=<optimized out>, dev=<optimized out>) at drivers/pci/msi.c:1100
#8  __pci_enable_msix_range (dev=0x900000027d4cc000, entries=0x0, minvec=2, maxvec=2, affd=0x900000027cd33b98) at drivers/pci/msi.c:1081
#9  0x90000000008e2278 in pci_alloc_irq_vectors_affinity (dev=0x900000027d4cc000, min_vecs=2, max_vecs=2, flags=12, affd=0x900000027cd33b98) at drivers/pci/msi.c:1170
#10 0x900000000096361c in vp_request_msix_vectors (desc=<optimized out>, per_vq_vectors=<optimized out>, nvectors=<optimized out>, vdev=<optimized out>) at drivers/virtio/virtio_pci_common.c:136
#11 vp_find_vqs_msix (vdev=0x900000027d38a800, nvqs=1, vqs=<optimized out>, callbacks=0x900000027ceb8780, names=0x900000027ceb8740, per_vq_vectors=true, ctx=0x0, desc=0x900000027cd33b98) at drivers/virtio/virtio_pci_common.c:307
#12 0x90000000009639e0 in vp_find_vqs (vdev=0x900000027d38a800, nvqs=1, vqs=0x900000027ceb87c0, callbacks=0x900000027ceb8780, names=0x900000027ceb8740, ctx=0x0, desc=0x900000027cd33b98) at drivers/virtio/virtio_pci_common.c:403
#13 0x90000000009624b4 in vp_modern_find_vqs (vdev=0x900000027d38a800, nvqs=<optimized out>, vqs=<optimized out>, callbacks=<optimized out>, names=<optimized out>, ctx=<optimized out>, desc=<optimized out>) at drivers/virtio/virtio_pci_modern.c:413
#14 0x9000000000a51c2c in virtio_find_vqs (desc=<optimized out>, names=<optimized out>, callbacks=<optimized out>, vqs=<optimized out>, nvqs=<optimized out>, vdev=<optimized out>) at ./include/linux/virtio_config.h:192
#15 init_vq (vblk=0x900000027d391800) at drivers/block/virtio_blk.c:542
#16 0x9000000000a52ce0 in virtblk_probe (vdev=0x900000027c024800) at drivers/block/virtio_blk.c:774
#17 0x900000000095f758 in virtio_dev_probe (_d=0x900000027d38a810) at drivers/virtio/virtio.c:245
#18 0x9000000000a2936c in really_probe (dev=0x900000027d38a810, drv=0x90000000014359c0 <virtio_blk>) at drivers/base/dd.c:506
#19 0x9000000000a295a4 in driver_probe_device (drv=0x90000000014359c0 <virtio_blk>, dev=0x900000027d38a810) at drivers/base/dd.c:667
#20 0x9000000000a2974c in __driver_attach (data=<optimized out>, dev=<optimized out>) at drivers/base/dd.c:903
#21 __driver_attach (dev=0x900000027d38a810, data=0x90000000014359c0 <virtio_blk>) at drivers/base/dd.c:872
#22 0x9000000000a270a0 in bus_for_each_dev (bus=<optimized out>, start=<optimized out>, data=0x1, fn=0x900000027cd338f0) at drivers/base/bus.c:279
#23 0x9000000000a28bb0 in driver_attach (drv=<optimized out>) at drivers/base/dd.c:922
#24 0x9000000000a2861c in bus_add_driver (drv=0x90000000014359c0 <virtio_blk>) at drivers/base/bus.c:672
#25 0x9000000000a2a390 in driver_register (drv=0x90000000014359c0 <virtio_blk>) at drivers/base/driver.c:170
#26 0x900000000095f134 in register_virtio_driver (driver=<optimized out>) at drivers/virtio/virtio.c:296
#27 0x90000000014ee768 in init () at drivers/block/virtio_blk.c:1019
#28 0x9000000000200b8c in do_one_initcall (fn=0x90000000014ee6f0 <init>) at init/main.c:884
#29 0x90000000014a4e8c in do_initcall_level (level=<optimized out>) at ./include/linux/init.h:131
#30 do_initcalls () at init/main.c:960
#31 do_basic_setup () at init/main.c:978
#32 kernel_init_freeable () at init/main.c:1145
#33 0x9000000000f78fa8 in kernel_init (unused=<optimized out>) at init/main.c:1062
#34 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```

in `pci_msi_setup_msi_irqs`, msi domain is accessed by dev
```c
	domain = dev_get_msi_domain(&dev->dev);
```

how dev's msi domain setup?
```c
#0  dev_set_msi_domain (d=<optimized out>, dev=<optimized out>) at ./include/linux/device.h:1103
#1  pci_set_bus_msi_domain (bus=0x0) at drivers/pci/probe.c:825
#2  0x90000000008b7db4 in pci_register_host_bridge (bridge=0x900000027d08d000) at drivers/pci/probe.c:879
#3  0x90000000008b8044 in pci_create_root_bus (parent=0x0, bus=0, ops=0x900000000141e670 <pci_generic_ecam_ops+8>, sysdata=0x900000027d439380, resources=0x900000027d3ab058) at drivers/pci/probe.c:3004
#4  0x90000000009231d0 in acpi_pci_root_create (root=0x900000027d440c00, ops=0x900000027d3ab080, info=0x900000027d3ab040, sysdata=0x900000027d439380) at drivers/acpi/pci_root.c:906
#5  0x9000000000da8328 in pci_acpi_scan_root (root=0x900000027d440c00) at arch/loongarch/pci/acpi.c:400
#6  0x9000000000922bfc in acpi_pci_root_add (device=0x900000027d349800, not_used=<optimized out>) at drivers/acpi/pci_root.c:607
#7  0x900000000091b44c in acpi_scan_attach_handler (device=<optimized out>) at drivers/acpi/scan.c:1989
#8  acpi_bus_attach (device=0x900000027d349800) at drivers/acpi/scan.c:2033
#9  0x900000000091b3dc in acpi_bus_attach (device=0x900000027d349000) at drivers/acpi/scan.c:2054
#10 0x900000000091b3dc in acpi_bus_attach (device=0x900000027d348800) at drivers/acpi/scan.c:2054
#11 0x900000000091d3f4 in acpi_bus_scan (handle=0xffffffffffffffff) at drivers/acpi/scan.c:2106
#12 0x90000000014e77b8 in acpi_scan_init () at drivers/acpi/scan.c:2259
#13 0x90000000014e7438 in acpi_init () at drivers/acpi/bus.c:1261
#14 0x9000000000200b8c in do_one_initcall (fn=0x90000000014e7154 <acpi_init>) at init/main.c:884
#15 0x90000000014a4e8c in do_initcall_level (level=<optimized out>) at ./include/linux/init.h:131
#16 do_initcalls () at init/main.c:960
#17 do_basic_setup () at init/main.c:978
#18 kernel_init_freeable () at init/main.c:1145
#19 0x9000000000f78fa8 in kernel_init (unused=<optimized out>) at init/main.c:1062
#20 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```
dev get msi domain from it's host bridge `pci_host_bridge_msi_domain`
- pci_host_bridge_acpi_msi_domain
```c
/**
 * pci_host_bridge_acpi_msi_domain - Retrieve MSI domain of a PCI host bridge
 * @bus:      The PCI host bridge bus.
 *
 * This function uses the callback function registered by
 * pci_msi_register_fwnode_provider() to retrieve the irq_domain with
 * type DOMAIN_BUS_PCI_MSI of the specified host bridge bus.
 * This returns NULL on error or when the domain is not found.
 */
struct irq_domain *pci_host_bridge_acpi_msi_domain(struct pci_bus *bus)
{
	struct fwnode_handle *fwnode;

	if (!pci_msi_get_fwnode_cb)
		return NULL;

	fwnode = pci_msi_get_fwnode_cb(&bus->dev);
	if (!fwnode)
		return NULL;

	return irq_find_matching_fwnode(fwnode, DOMAIN_BUS_PCI_MSI);
}
```
it seems get the firmware by the fwnode.
