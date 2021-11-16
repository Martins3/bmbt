# 整体的计划分析

## 问题
- [ ] 所以还是使用 e820 来实现探测内存吗? 那么 acpi 是做什么用的?

## 需要处理的大块问题
```plain
src/hw/i386/fw_cfg.c:33:#if SMBIOS_TODO
```

应该是这里有事情被简化掉了，所以现在 x86_cpu_realizefn 中没有初始化 features 字段
如果上面的 NEED_LATER 都修复了，但是问题还是没有解决，那么就出现了大问题了

## 正在调试的东西
- [ ] 分析一下，下面的几个 log 输出的含义:
  - 其实之前一致都是没有搞清楚的问题，在 bios 中，这个时候 qemu_raise_irq 的最后的效果是什么?
```plain
CPUIRQ: pc: lowering GSI 8
CPUIRQ: pic_irqs: lower irq 0
CPUIRQ: pic_irqs: lower irq 0
CPUIRQ: pic_irqs: lower irq 0
```

- [ ] signal 我的 gdb 现在是没有办法 backtrace 了呀，好难啊
  - 有没有办法让 gdb 不被信号打断的

- [ ] 验证一下，active_timers_lock 也是可以通过 verify_BQL 的
```c
    qemu_mutex_unlock(&timer_list->active_timers_lock);
```

- [ ] 重新 review 一下 pc_machine_device_pre_plug_cb 和 pc_machine_device_plug_cb
  - [ ] 确认一下，真的只有是只有 cpu 和 acpi 使用吗?
  - [ ] 为什么我感觉 pci 设备才是 hotplug 大户
