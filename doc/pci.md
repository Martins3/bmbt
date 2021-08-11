# pci

在我们的设想当中，pci 设备都是可以进行穿透的，还是存在很多需要考虑的问题

- [ ] kvmtool 中 pci_config_mmio_access 让我意识到，pci 的配置空间也是可以进行 MMIO 的

- [ ] 真正的恐惧 : /home/maritns3/core/kvmqemu/hw/isa/piix3.c 中需要模拟，但是到底为什么需要模拟，到底会牵涉多少东西
  - [ ] piix4 也需要处理吗

[^1]: https://stackoverflow.com/questions/52136259/how-to-access-pci-express-configuration-space-via-mmio
