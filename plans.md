# 整体的计划分析
> github 上编辑实在是太蛋疼了, 将 https://github.com/Martins3/BMBT/issues/34 移动到此处吧!

思考一下，在 kernel 态到底需要什么？
- 内存管理
- **中断**
- 一些设备的模拟

- [ ] 所以还是使用 e820 来实现探测内存吗? 那么 acpi 是做什么用的?

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
