# BMBT(Bare Mental Binary Translator)

## Introduction
`target-ARCH/*` 定義了如何將 ARCH binary 反匯編成 TCG IR。tcg/ARCH 定義了如何將 TCG IR 翻譯成 ARCH binary。

- [ ] 有件事情没有想明白，调用 helper 的时候就进入到 qemu 中间了，是什么时候调用的 prologue 的离开 tb 执行的环境的。(heler_inw 之类的) (写一个进入 tb 环境 和 离开的小专题，顺便分析一下如何是 setjmp 的使用方法)
   - Niugene 说切到 helper 这里实际上取决于是否破坏环境，有的不用处理的

## LoongArch Manual
Please contact huxueshi@loongson.cn

## QA
1. 为什么需要将 TCG engine 移植过来
    - 因为 helper 函数需要

2. 为什么可以设备直通？
    - 因为很多设备和架构是无关的，这就是为什么设备驱动在 Linux 内核 arch 文件夹中几乎没有设备驱动

## Contributor
As for collaboration, please follow [these instructions](./CONTRIBUTING.md)

This project is fairly tricky for beginner, we also write some [documents for newbie](./doc/newbie.md)

## 任务进度
- [ ] 锁，多核
- [ ] 启动, 初始化
- [ ] memory model 的接口
- [ ] fw_cfg / loader 之类的 : 其实就是 machine 启动相关的部分


任务具体细节参考 [#34](https://github.com/Martins3/BMBT/issues/34)
