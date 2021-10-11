# Doc for newbie

当前的整个工作都是基于: 895fdf6776076c7cbb6b18c6703a40c4e03a084e 进行的

## 复现 xqm 的工作
- [ ] 编译内核

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

/home/loongson/ld/x86-qemu-mips/build/i386-softmmu/qemu-system-i386 -hda ~/xqm_images/ubuntu10s.test.img.full -xtm select,tblink,lbt -kernel ~/xqm_images/vmlinuz-2.6.32 -append "console=ttyS0 root=/dev/sda1 ro init=/bin/bash tsc=reliable rw" --nographic \
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

## 调试

```gdb
>>> p /x ((CPUX86State *)current_cpu->env_ptr)->eip
$1 = 0xe92d6
```
然后在 gdb 中:
```txt
disass 0xe92d6
```
如果是 seabios，启动 gdb 的方法: gdb out/rom.o

## 配置 5000 的机器
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
