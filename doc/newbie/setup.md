# Doc for newbie

当前的整个工作都是基于: 895fdf6776076c7cbb6b18c6703a40c4e03a084e 进行的
对应的 qemu 版本: 6cdf8c4efa073eac7d5f9894329e2d07743c2955 (tag: v4.2.1)

## 复现 xqm 的工作
- 获取 image
  - http://old-releases.ubuntu.com/releases/10.04.0/ 中找到 ubuntu-10.04-server-i386.iso 下载，然后可以安装(tcg / kvm 都可以)

- 编译参数
```plain
mkdir build
../configure --target-list=i386-softmmu --enable-latx --disable-werror
```

- 运行参数
```sh
#!/bin/bash
/home/loongson/ld/x86-qemu-mips/build/i386-softmmu/qemu-system-i386 \
-hda ~/xqm_images/ubuntu10s.test.img.full -xtm select,tblink,lbt -kernel \
~/xqm_images/vmlinuz-2.6.32 -append "console=ttyS0 root=/dev/sda1 ro init=/bin/bash tsc=reliable rw" --nographic \
-chardev file,path=/tmp/seabios.log,id=seabios -device isa-debugcon,iobase=0x402,chardev=seabios -bios ~/xqm_images/bios.bin

```
- [ ] 这里的 ubuntu10s.test.img.full 和 vmlinuz-2.6.32 还没有自己编译过

```sh
#!/bin/bash

xqm="qemu-system-i386"
DISKIMG=ubuntu10s.img

taskset -c 1 ./${xqm} -hda ${DISKIMG} \
    -kernel vmlinuz-2.6.32 \
    -m 2048 \
    --nographic \
    -xtm select,tblink,lbt \
    -append "console=ttyS0 root=/dev/sda1 ro tsc=reliable init=/bin/bash rw"

# all optimizations option for xtm
# -xtm select,tblink,lbt,trbh,staticcs,njc,lsfpu
```

## 调试环境搭建
[如何增大 dmesg buffer 的大小](https://unix.stackexchange.com/questions/412182/how-to-increase-dmesg-buffer-size-in-centos-7-2)

## 处理 QEMU 的 macro

生成中间文件
```plain
../configure --target-list=x86_64-softmmu  --disable-werror --extra-cflags='-save-temps'
```
但是这似乎导致无法 `make -j10`, 只能串行编译

## 开发环境
小项目 : sshfs
大项目 : rsync

## debug

### 使用 gdb 调试
```gdb
>>> p /x ((CPUX86State *)current_cpu->env_ptr)->eip
>>> p /x ((CPUX86State *)current_cpu->env_ptr)->segs[R_CS].base
```
然后在 gdb 中 disass

如果是 seabios，启动 gdb 的方法: gdb out/rom.o
或者 gdb out/rom16.o

### 在源码中间添加 log
- tb_find : 打印 pc 可以知道当前跑到哪里去了
- 在 tr_ir2_generate 中打开 ir1_dump 的 log 可以看到将要执行的每条执行的反汇编结果
- x86_to_mips_before_exec_tb 和 x86_to_mips_after_exec_tb 中分析打开 CONFIG_XTM_TEST 的内容

- 错误的 : target_x86_to_mips_host 的参数 max_insns 替换为 1

### 屏蔽信号
因为暂时适应的是 signal 来实现 timer 的
这个会导致 qemu 提前停下来

使用可以屏蔽信号:
handle SIG127 nostop noprint


https://stackoverflow.com/questions/24999208/how-to-handle-all-signals-in-gdb


## 配置 5000 的机器，让其可以科学上网
https://github.com/garywill/linux-router

## 在 Loongarch 上交叉编译内核
```c
#!/bin/sh
CC_PREFIX=~/arch/LARCH_toolchain_root_newabi

export ARCH=loongarch64
export CROSS_COMPILE=loongarch64-linux-gnu-
export PATH=$CC_PREFIX/bin:$PATH
export LD_LIBRARY_PATH=$CC_PREFIX/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$CC_PREFIX/lib64:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$CC_PREFIX/loongarch64-linux-gnu/lib64/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$CC_PREFIX/loongarch64-linux-gnu/sysroot/usr/lib/:$LD_LIBRARY_PATH
```

## 编译 tiny 内核
- 教程: https://weeraman.com/building-a-tiny-linux-kernel-8c07579ae79d
  - make tinyconfig 将会输出很多参数确认信息，那并不是问题
- 内核下载地址 : https://mirrors.edge.kernel.org/pub/linux/kernel/v4.x/
- 版本 : linux-4.4.142.tar.gz

### 在 x86 运行
1. 编译出来对应的 qemu
```c
mkdir 32bit
cd 32bit
../configure --target-list=i386-softmmu
```

2. 在 alpine.sh 中修改配置
```sh
kernel_dir=/home/maritns3/core/ld/guest-src/linux-4.4.142
qemu=/home/maritns3/core/xqm/32bit/i386-softmmu/qemu-system-i386
```

## UEFI
- UEFI 基础准备: https://martins3.github.io/bmbt/uefi-linux.html
- 将 BMBT 作为 edk2 的一部分需要进行的操作: https://martins3.github.io/bmbt/uefi-in-action.html
