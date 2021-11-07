# 整体的计划分析

## 问题
- [ ] 所以还是使用 e820 来实现探测内存吗? 那么 acpi 是做什么用的?

## 在 bare-mental 上测试 seabios 的执行
- 添加设备和中断支持
- 了解 UEFI SDK : 按照 UEFI SDK 的方式加载程序，而不是内核的方式
- https://github.com/tianocore/tianocore.github.io
- apci probe
- cache init
- interrupt init
- TLB refill init
- memory probe
- probe pci devices

## 需要处理的大块问题
```plain
src/hw/i386/fw_cfg.c:33:#if SMBIOS_TODO
```

```plain
src/hw/i386/pc_piix.c:133:#if NEED_LATER
src/hw/i386/pc_piix.c:156:#if NEED_LATER
src/hw/i386/pc_piix.c:219:#if NEED_LATER
src/hw/i386/fw_cfg.c:101:#ifdef NEED_LATER
src/hw/i386/fw_cfg.c:111:#ifdef NEED_LATER
src/i386/helper.c:397:#ifdef NEED_LATER
src/hw/intc/ioapic.c:398:#ifdef NEED_LATER
```

- PORT_RTC
- MEM_TODO

应该是这里有事情被简化掉了，所以现在 x86_cpu_realizefn 中没有初始化 features 字段
如果上面的 NEED_LATER 都修复了，但是问题还是没有解决，那么就出现了大问题了
