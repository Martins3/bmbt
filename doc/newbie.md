# Doc for newbie

## 复现 xqm 的工作
- [ ] 编译内核

- 获取 image
  - http://old-releases.ubuntu.com/releases/10.04.0/ 中找到 ubuntu-10.04-server-i386.iso 下载，然后可以安装(tcg / kvm 都可以)

- 编译参数
```
mkdir build
../configure --target-list=i386-softmmu --enable-latx --disable-werror
```

- 运行参数
```
/home/loongson/ld/x86-qemu-mips/build/i386-softmmu -hda ~/ubuntu10s.test.img.full -xtm select,lbt -kernel ~/vmlinuz-2.6.32 -append "console=ttyS0 root=/dev/sda1 ro" --nographic \
-chardev file,path=/tmp/seabios.log,id=seabios -device isa-debugcon,iobase=0x402,chardev=seabios -bios /home/maritns3/core/seabios/out/bios.bin
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


## 学习 QEMU
| 资料                                                               | 主要内容            |
|--------------------------------------------------------------------|---------------------|
| QEMU/KVM 源码解析与应用                                            | 比较全面            |
| [陈伟任的笔记](https://github.com/azru0512/slide/tree/master/QEMU) | 主要描述 tcg 的工作 |

## 学习内核
@todo

## 调试环境搭建
[如何增大 dmesg buffer 的大小](https://unix.stackexchange.com/questions/412182/how-to-increase-dmesg-buffer-size-in-centos-7-2)

## 处理 QEMU 的 macro

生成中间文件 
```
../configure --target-list=x86_64-softmmu  --disable-werror --extra-cflags='-save-temps'
```
但是这似乎导致无法 `make -j10`, 只能串行编译
