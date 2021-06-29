# How acpi works in Linux Kernel and Simulated in QEMU

## ACPI 基础教程
1. [Linux kernel doc](https://www.kernel.org/doc/html/latest/firmware-guide/acpi/index.html#)
2. [acpica : acpi introduction](https://acpica.org/sites/acpica/files/ACPI-Introduction.pdf) :star:
3. [ACPI Source Language (ASL) Tutorial](https://acpica.org/sites/acpica/files/asl_tutorial_v20190625.pdf)
4. [ACPI: Design Principles and Concerns](https://www.ssi.gouv.fr/uploads/IMG/pdf/article_acpi.pdf)
5. [Getting started with ACPI](https://dortania.github.io/Getting-Started-With-ACPI/#a-quick-explainer-on-acpi)

## 问题
- [ ] 需要体用动态构建 ACPI 表吗?



## acpi setup

#### FDC0 / MOU / KBD 的 ACPI 描述是如何添加进去的
```c
/*
#0  i8042_build_aml (isadev=0x5555566eb400, scope=0x55555699a990) at ../hw/input/pckbd.c:564
#1  0x000055555590c474 in isa_build_aml (bus=<optimized out>, scope=scope@entry=0x55555699a990) at ../hw/isa/isa-bus.c:214
#2  0x0000555555a6e08d in build_isa_devices_aml (table=table@entry=0x555556909540) at /home/maritns3/core/kvmqemu/include/hw/isa/isa.h:17
#3  0x0000555555a71281 in build_dsdt (machine=0x5555566c0400, pci_hole64=<synthetic pointer>, pci_hole=<synthetic pointer>, misc=<synthetic pointer>, pm=0x7fffffffd450,
 linker=0x5555568d6bc0, table_data=0x555556aae0a0) at ../hw/i386/acpi-build.c:1403
#4  acpi_build (tables=tables@entry=0x7fffffffd530, machine=0x5555566c0400) at ../hw/i386/acpi-build.c:2374
#5  0x0000555555a73e8e in acpi_setup () at /home/maritns3/core/kvmqemu/include/hw/boards.h:24
#6  0x0000555555a5fd1f in pc_machine_done (notifier=0x5555566c0598, data=<optimized out>) at ../hw/i386/pc.c:789
#7  0x0000555555d27e67 in notifier_list_notify (list=list@entry=0x5555564c0a58 <machine_init_done_notifiers>, data=data@entry=0x0) at ../util/notify.c:39
#8  0x00005555558ff94b in qdev_machine_creation_done () at ../hw/core/machine.c:1280
#9  0x0000555555bae301 in qemu_machine_creation_done () at ../softmmu/vl.c:2567
#10 qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2590
#11 0x0000555555bb1e62 in qemu_init (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/vl.c:3611
#12 0x000055555582b4bd in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
```
原来是通过 isa 总线将描述信息添加进去的。


