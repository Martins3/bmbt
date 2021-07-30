# BMBT
```txt
 ______________________________________
< BMBT is Bare Metal Binary Translator >
 --------------------------------------
   \
    \
        .--.
       |o_o |
       |:_/ |
      //   \ \
     (|     | )
    /'\_   _/`\
    \___)=(___/
```

## 目标
这个项目的想法是，一个 3A5000 电脑开机之后，然后开机之后就是 Windows 。
基本思路是让 BIOS 启动之后，然后执行 BMBT，进而让 guest OS 来引导启动。

## 思路的来源
1. 在二进制翻译上充分利用硬件资源加速
2. 避免 QEMU 和 Linux 的软件栈的复杂性

系统态的模拟使用需要更多的访问系统态的资源，当然可以通过在内核中间定义内核模块的方法实现访问，但是
1. 系统调用是存在很大的开销的
2. 如何构建一个内核模块来将系统态资源共享给二进制翻译器是很有挑战的

将系统态二进制翻译放到内核中运行的方法去的一系列进展[^1], 但是这些方法都是借助了虚拟化，
host 的软件栈无法移除掉。还有一个就是 QEMU 实际上为了处理各种虚拟化技术，操作系统和指令集，也变的过于复杂了。
## LoongArch Manual
Please contact huxueshi@loongson.cn

## QA
1. 为什么需要将 TCG engine 移植过来
    - 因为 helper 函数需要

2. 为什么可以设备直通？
    - 因为很多设备和架构是无关的，这就是为什么设备驱动在 Linux 内核 arch 文件夹中几乎没有设备驱动

3. 为什么不适用 unikernel + QEMU 的方法?
    - 现在 unikernel 的解决方案都是试图将 unikernel 放到虚拟机的 guest 态中运行，无法避免 host 的软件栈 
      - 那为什么不采用的 A Linux in unikernel clothing[^2] 的方式 ?
          - 这种方法无法实现设备直通, 而且这种方案依赖于 KLM[^3] 的支持

## Contributor
As for collaboration, please follow [these instructions](./CONTRIBUTING.md)

This project is fairly tricky for beginner, we also write some [documents for newbie](./doc/newbie.md)

[^1]: [Efficient Cross-architecture Hardware Virtualisation](https://era.ed.ac.uk/handle/1842/25377)
[^2]: [A Linux in unikernel clothing](https://dl.acm.org/doi/10.1145/3342195.3387526)
[^3]: [Kernel Mode Linux](http://web.yl.is.s.u-tokyo.ac.jp/~tosh/kml/)
