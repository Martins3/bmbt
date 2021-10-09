# 整体的计划分析

- [ ] 将 chip 的 QOM 处理掉，但是 memory 还是保持不变的

## 问题
- [ ] 所以还是使用 e820 来实现探测内存吗? 那么 acpi 是做什么用的?
- [ ] get_boot_devices_list 的返回结果是 /rom@genroms/linuxboot_dma.bin，通过这种方式直接启动 Linux，如果是从 disk 上启动 guest 的时候，那么其返回为 NULL
  - 所以，到底用什么方法来加载 guest 的啊

## 将用户态的依赖彻底移除掉
- setsigjmp 这种东西在内核态可以使用吗?
- 如何保证自己对于 glibc 没有依赖?

## 在 bare-mental 上测试 seabios 的执行
- 添加设备和中断支持
- 了解 UEFI SDK
- https://github.com/tianocore/tianocore.github.io
- apci probe
- cache init
- interrupt init
- TLB refill init
- memory probe
- probe pci devices
