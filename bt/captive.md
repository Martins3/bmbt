## [captive](https://github.com/tspink/captive)
- main
  - KernelLoader::create_from_file
  - KVM::create_guest

platform :
1. symbench
2. user
3. virt

- [ ] I think code in arch are auto generated, but how are called from 

- [ ] captive only works on armv8, but no I don't know how to run it

binary translation 

- 之所以出现了很多 unikernel 的代码，是因为这个 hypervisor 自己在 kvm 中间运行是需要二进制翻译的



## source code
- [ ] how engine loaded and executed ?

- Engine::load && Engine::install

`alloc_guest_memory(VM_PHYS_CODE_BASE, VM_CODE_SIZE, 0, (void *)0x670000000000);`

- [ ] how semihosting works ?


#### how unikernel works
src/hypervisor/kvm/cpu.cpp : 的位置启动内核

X86DAGEmitter::cmp_lt is called by arm64-jit-chunk-7.cpp, it's used for emit x86 code.

dbt : x86-emitter.c  x86-value.c and encoder.c

- [ ] VirtualRegisterAllocator and ReverseAllocator

- [ ] linux kernel setup : seems in kvm sregs setup.
  - [ ] what's kernel entry

after some setup, we came here :
/home/maritns3/core/captive-project/captive/arch/common/cpu-block-jit.cpp




- GuestConfiguration
  - GuestCPUConfiguration
  - GuestMemoryRegionConfiguration
  - GuestDeviceConfiguration


#### 还是找不到 unikernel 的范围啊!

#### 分析 keyboard 的流程
从 KVM_EXIT_MMIO 的位置开始。

## problem
- [x] 100% CPU
  - 这应该不是 bug，而是使用 poll 的原因

- [ ] why so many code about devices ?

实在是想不通，对于各种设备访问，captive 是如何模拟的 ?
  - xqm 是如何实现的，我也是很好奇的 

## 文章阅读记录
- [ ] 2.3.2 Translation : 建立 DAG 实验部分。
- [ ] 2.7 Virual Memory Management 的划分为上下两个部分
