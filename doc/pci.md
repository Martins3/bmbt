- [ ] 看看 pci_init_multifunction 是怎么被调用的吧!
	- 感觉 pci 整个还是很麻烦的，可以好好整理出来一下

- [ ] i440fx pm 这个设备到底是如何管理 power 的


显然是需要移植 PCI 的，如何增量的，逐步的移植这个东西啊?
- /home/maritns3/core/xqm/hw/pci-host/i440fx.c
- /home/maritns3/core/xqm/hw/isa/piix3.c

```c
static void i440fx_pcihost_realize(DeviceState *dev, Error **errp)
{
    PCIHostState *s = PCI_HOST_BRIDGE(dev);
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);

    sysbus_add_io(sbd, 0xcf8, &s->conf_mem);
    sysbus_init_ioports(sbd, 0xcf8, 4);

    sysbus_add_io(sbd, 0xcfc, &s->data_mem);
    sysbus_init_ioports(sbd, 0xcfc, 4);

    /* register i440fx 0xcf8 port as coalesced pio */
    memory_region_set_flush_coalesced(&s->data_mem);
    memory_region_add_coalescing(&s->conf_mem, 0, 4);
}
```
