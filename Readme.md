# BMBT(Bare Mental Binary Translator)

## Introduction
`target-ARCH/*` 定義了如何將 ARCH binary 反匯編成 TCG IR。tcg/ARCH 定義了如何將 TCG IR 翻譯成 ARCH binary。

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
