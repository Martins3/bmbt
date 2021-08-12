# pci
1. 中断的设计: 实现中断在 PCI 中路由到 ioapic 中的操作

## 资料

- lspci
```txt
00:00.0 Host bridge: Intel Corporation 440FX - 82441FX PMC [Natoma] (rev 02)
00:01.0 ISA bridge: Intel Corporation 82371SB PIIX3 ISA [Natoma/Triton II]
00:01.1 IDE interface: Intel Corporation 82371SB PIIX3 IDE [Natoma/Triton II]
00:01.3 Bridge: Intel Corporation 82371AB/EB/MB PIIX4 ACPI (rev 03)
00:02.0 VGA compatible controller: Red Hat, Inc. Virtio GPU (rev 01)
00:03.0 Ethernet controller: Intel Corporation 82540EM Gigabit Ethernet Controller (rev 03)
00:04.0 Non-Volatile memory controller: Red Hat, Inc. Device 0010 (rev 02)
00:05.0 Non-Volatile memory controller: Red Hat, Inc. Device 0010 (rev 02)
00:06.0 Unclassified device [0002]: Red Hat, Inc. Virtio filesystem
```


## 分析
- [ ] 能不能不要让其探测出来 piix3 ，如此一了百了
  - 使用其他的方法来模拟 piix3 的功能

- [ ] piix3 和 isa 总线是什么关系 ?
  - piix3 是一个 PCI 设备

在我们的设想当中，pci 设备都是可以进行穿透的，还是存在很多需要考虑的问题, 比如
- [ ] 如果提供 piix3 / piix4 等虚拟设备，如何处理
- [ ] 因为需要模拟 piix3 / piix4 所以，pci_default_write_config 这个函数实现，这似乎是一个无底洞啊


- [x] 检查一下，除了 piix3 / piix4 之外，还有那些 x86 独占的设备
  - 从 lspci 看，应该只有这些了
  - [ ] 可以检查一下 seabios 的 pci_device_tbl

- [ ] kvmtool 中 pci_config_mmio_access 让我意识到，pci 的配置空间也是可以进行 MMIO 的, 实际上，LoongArch 还可以配置这个功能

- [ ] 真正的恐惧 : /home/maritns3/core/kvmqemu/hw/isa/piix3.c 中需要模拟，但是到底为什么需要模拟，到底会牵涉多少东西
  - [ ] piix4 也需要处理吗

- [ ] 只是因为为了处理 isa 才需要搞 piix4 的吗 ?
  - piix4 和 acpi 中还有一些酷炫的关系


11. 分析这个路径产生的原因是什么:
```c
/*
#0  huxueshi (irq=14) at ../hw/i386/kvm/ioapic.c:112
#1  insert_counter (irq=14) at ../hw/i386/kvm/ioapic.c:125
#2  kvm_ioapic_set_irq (opaque=0x555556bec800, irq=14, level=0) at ../hw/i386/kvm/ioapic.c:135
#3  0x0000555555b92644 in gsi_handler (opaque=0x555556a0e200, n=14, level=0) at ../hw/i386/x86.c:600
#4  0x0000555555a23e2e in piix3_set_irq_pic (piix3=0x555556a60310, piix3=0x555556a60310, pic_irq=<optimized out>) at ../hw/isa/piix3.c:123
#5  piix3_write_config (address=<optimized out>, val=<optimized out>, len=<optimized out>, dev=<optimized out>) at ../hw/isa/piix3.c:123
#6  piix3_write_config (dev=<optimized out>, address=<optimized out>, val=<optimized out>, len=<optimized out>) at ../hw/isa/piix3.c:112
#7  0x0000555555a34bbb in pci_host_config_write_common (pci_dev=0x555556a60310, addr=96, limit=<optimized out>, val=10, len=1) at ../hw/pci/pci_host.c:83
#8  0x0000555555cd2661 in memory_region_write_accessor (mr=mr@entry=0x555556a22de0, addr=0, value=value@entry=0x7fffe890d0a8, size=size@entry=1, shift=<optimized out>,
mask=mask@entry=255, attrs=...) at ../softmmu/memory.c:492
#9  0x0000555555cceaee in access_with_adjusted_size (addr=addr@entry=0, value=value@entry=0x7fffe890d0a8, size=size@entry=1, access_size_min=<optimized out>, access_siz
e_max=<optimized out>, access_fn=access_fn@entry=0x555555cd25d0 <memory_region_write_accessor>, mr=0x555556a22de0, attrs=...) at ../softmmu/memory.c:554
#10 0x0000555555cd1b97 in memory_region_dispatch_write (mr=mr@entry=0x555556a22de0, addr=0, data=<optimized out>, op=<optimized out>, attrs=attrs@entry=...) at ../softm
mu/memory.c:1504
#11 0x0000555555c9c060 in flatview_write_continue (fv=fv@entry=0x7ffe4c043840, addr=addr@entry=3324, attrs=..., ptr=ptr@entry=0x7fffeb180000, len=len@entry=1, addr1=<op
timized out>, l=<optimized out>, mr=0x555556a22de0) at /home/maritns3/core/kvmqemu/include/qemu/host-utils.h:165
#12 0x0000555555c9c276 in flatview_write (fv=0x7ffe4c043840, addr=addr@entry=3324, attrs=attrs@entry=..., buf=buf@entry=0x7fffeb180000, len=len@entry=1) at ../softmmu/p
hysmem.c:2818
#13 0x0000555555c9ef46 in address_space_write (as=0x555556606a40 <address_space_io>, addr=addr@entry=3324, attrs=..., buf=0x7fffeb180000, len=len@entry=1) at ../softmmu
/physmem.c:2910
#14 0x0000555555c9efde in address_space_rw (as=<optimized out>, addr=addr@entry=3324, attrs=..., attrs@entry=..., buf=<optimized out>, len=len@entry=1, is_write=is_writ
e@entry=true) at ../softmmu/physmem.c:2920
#15 0x0000555555c8ecb9 in kvm_handle_io (count=1, size=1, direction=<optimized out>, data=<optimized out>, attrs=..., port=3324) at ../accel/kvm/kvm-all.c:2632
#16 kvm_cpu_exec (cpu=cpu@entry=0x555556b030d0) at ../accel/kvm/kvm-all.c:2883
#17 0x0000555555cf17f5 in kvm_vcpu_thread_fn (arg=arg@entry=0x555556b030d0) at ../accel/kvm/kvm-accel-ops.c:49
#18 0x0000555555e55953 in qemu_thread_start (args=<optimized out>) at ../util/qemu-thread-posix.c:541
#19 0x00007ffff628d609 in start_thread (arg=<optimized out>) at pthread_create.c:477
#20 0x00007ffff61b4293 in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:95
```

- 分析一下 piix3_write_config 的源代码位置
  - 因为 piix3 是 isa 的 bridge, 所以应该可以实现一些中断路由的操作?
  - [ ] 无法理解为什么写的时候就要触发一下中断啊
  - [ ] 在 seabios 中如果没有注册 irq handler 的话，这些是如何处理的

```c
static void pci_bios_init_devices(void)
{
    struct pci_device *pci;
    foreachpci(pci) {
        pci_bios_init_device(pci);
    }
}

/* PIIX3/PIIX4 PCI to ISA bridge */
static void piix_isa_bridge_setup(struct pci_device *pci, void *arg)
{
    int i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;
    for (i = 0; i < 4; i++) {
        irq = pci_irqs[i];
        /* set to trigger level */
        elcr[irq >> 3] |= (1 << (irq & 7));
        /* activate irq remapping in PIIX */
        pci_config_writeb(pci->bdf, 0x60 + i, irq);
    }
    outb(elcr[0], PIIX_PORT_ELCR1);
    outb(elcr[1], PIIX_PORT_ELCR2);
    dprintf(1, "PIIX3/PIIX4 init: elcr=%02x %02x\n", elcr[0], elcr[1]);
}
```
- [ ] 为什么需要将 bridge 接入的四个中断全部设置为 level 的

- [ ] wiki[^2] 描述了 elcr 说处理 isa 总线之类的
  - [ ] 找到 QEMU 对应的 handler 的处理效果

- [ ] 一个好端端的 PCI bridge 为什么需要和 pic 控制器产生联系
    - [ ] 而且 pic 不是很快就 disable 掉了吗?

- [ ] 内核中也会调用 piix3_write_config 的, 找到对应的位置

- [ ] piix3_read_config 并没有对应的版本的


## [ ] elcr
- i8259_init_chip : 注册地址
- pic_common_realize : 将 PICCommonState::elcr_io 这个地址空间注册到 isa 上去
- pic_realize : 注册 handler, 似乎超级简单, 作用就是修改 PICCommonState::elcr 的数值，最后的作用体现在 pic_set_irq 上的

- [ ] kvm 为何无需考虑 eclr 这个东西啊

## [ ] 中断在 piix3 中的工作方式
- [ ] 当前的分析都是基于 nomsi 的，并不知道采用 msi 之后有什么区别

- nvme_irq_check
  - nvme_irq_check
    - pci_irq_assert
      - pci_set_irq
        - pci_irq_handler


分析一下 piix3_set_irq 的使用位置: 
1. nvme_init_cq : 注册是 timer 上的，会周期性的触发 nvme_post_cqes
  - nvme_post_cqes
    - nvme_irq_assert
      - nvme_irq_check
        - pci_irq_assert
          - int intx = pci_intx(pci_dev); // 用于获取到底是 pci 中断线
          - pci_irq_handler
            - pci_change_irq_level
              - pci_bus_change_irq_level
                - PCIBus::set_irq : 根据当前的机器配置，注册的是 piix3_set_irq

## piix4
piix4 只是 piix3 的升级版本而已，但是不采用这个东西

- [x] 似乎所有的中断都是经过 piix3 的，piix4 的作用是什么?
  - hw/isa/meson.build : 指出只有 CONFIG_PIIX4 的时候，才会编译 piix4.c
  - 仅仅是在 configs/devices/mips-softmmu/common.mak 中存在的 CONFIG_PIIX4
  - 但是 build/x86_64-softmmu-config-devices.h 中，并没有配置

实际上，对于 piix3 和 piix4 在 QEMU 中的地位不是可以随意配置替换的，从 pc_init1 中直接硬编码调用 piix3_create 就可见一斑

## i440fx 需要被模拟吗
- [ ] 为什么 nvme 的中断最后不是经过 i440fx 的，这才是 pci hub 的啊
- 应该只是需要处理相当少的内容的吧!


## PCI 中断的一般过程
1. 在 hw/isa/piix3.c 中实际上，将 PCIBus 的中断功能就是交给 piix3 实现了
2. 通过 piix3 实际上控制着 pci 中断的 level 的，体现在 piix3_write_config
    - 暂时无法理解 piix3_write_config 在修改 level 之后需要将 pic 的所有中断全部 raise 一遍
    - [ ] 修改这些的时候 level，我相信整个 CPU 都是屏蔽中断的

```c
struct PCIBus {
    BusState qbus;
    enum PCIBusFlags flags;
    PCIIOMMUFunc iommu_fn;
    void *iommu_opaque;
    uint8_t devfn_min;
    uint32_t slot_reserved_mask;
    pci_set_irq_fn set_irq;
    pci_map_irq_fn map_irq;
    pci_route_irq_fn route_intx_to_irq;
    void *irq_opaque;
    PCIDevice *devices[PCI_SLOT_MAX * PCI_FUNC_MAX];
    PCIDevice *parent_dev;
    MemoryRegion *address_space_mem;
    MemoryRegion *address_space_io;

    QLIST_HEAD(, PCIBus) child; /* this will be replaced by qdev later */
    QLIST_ENTRY(PCIBus) sibling;/* this will be replaced by qdev later */

    /* The bus IRQ state is the logical OR of the connected devices.
       Keep a count of the number of devices with raised IRQs.  */
    int nirq;
    int *irq_count;

    Notifier machine_done;
};
```

- [ ] map_irq 是什么作用?
```c
/*
 * Return the global irq number corresponding to a given device irq
 * pin. We could also use the bus number to have a more precise mapping.
 */
static int pci_slot_get_pirq(PCIDevice *pci_dev, int pci_intx)
{
    int slot_addend;
    slot_addend = PCI_SLOT(pci_dev->devfn) - 1;
    return (pci_intx + slot_addend) & 3;
}
```

[^1]: https://stackoverflow.com/questions/52136259/how-to-access-pci-express-configuration-space-via-mmio
[^2]: https://en.wikipedia.org/wiki/Intel_8259
