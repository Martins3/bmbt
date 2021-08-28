# 整体的计划分析
> github 上编辑实在是太蛋疼了, 将 https://github.com/Martins3/BMBT/issues/34 移动到此处吧!

如果想要一个解决问题，那就吧问题列举成为一个 20 min 可以完成的小项目。

没有必要思考将来要做的事情的细节，接下来一个个的处理就可以了。

当前的计划:

- acpi 暂时可以不着急，着急的是理清楚，到底如何加载 bios 以及内核，然后开始第一行代码，所以将这些东西理清楚
    - acpi 搞了一个专门的 hw/acpi/aml-build.c 用于投建 acpi，所以，并没有那么多的恐惧的东西
- 实际上，我发现，只是需要运行一条指令就可以了, fw_cfg 和 acpi 其实都是暂时可以不用考虑了

现在为什么忽然回到 memory model 的重构了，因为 apic 之类的东西暂时不需要也没有关系，将那些函数全部定义为 qemu_unreachable

- [ ] 只是需要分析在 vcpu 线程的执行过程中，需要的那些内容即可
  - [ ] 也许思考一些，如何保证一个变量只有在初始化之后才可以访问
    - [ ] 其实这很容易，初始化一个对象，然后定义出来其 init 函数，在 init 函数中间必须初始化出来

我忽然意识到一个事情，如果 guest 只有一个串口，那么我们可以在用户态运行很长时间吧!

## Make linker happy
- reference glib : 处理文件 /home/maritns3/core/ld/DuckBuBi/src/tcg/glib_stub.h
- reference capstone #105
- 现在构建的代码实际上在用户态只能测试一部分，需要使用测试代码覆盖工具将没有测试的部分找出来

## 在用户态测试 seabios 的执行，暂时可以保留一些用户态的依赖
- clear structures and load the init function #75 : 通过校对结构体中的数值来将函数粘贴过来
- port fw_cfg #61
- smbios #65
- How does fw_cfg load Linux kernel? #63
- 如何加载的 bios 镜像的，虽然很多镜像都是通过 fw_cfg 加载的，但是 bios 是执行的基础啊
- port QHT #41
- rewrite trace subsystem #47
- pam
    - https://wiki.qemu.org/Documentation/Platforms/PC
    - seabios : make_bios_writable
    - QEMU 如何初始化
    - seabios 如何使用的
- remove cpu_reset in x86_cpu_realizefn #69
- clear documents
- How to deal with x86tomips-options.c? #31

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
