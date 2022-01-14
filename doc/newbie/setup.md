# Doc for newbie

当前的整个工作都是基于: 895fdf6776076c7cbb6b18c6703a40c4e03a084e 进行的
对应的 qemu 版本: 6cdf8c4efa073eac7d5f9894329e2d07743c2955 (tag: v4.2.1)
musl version : 1e4204d522670a1d8b8ab85f1cfefa960547e8af
glibc version : 8b0a2fa8312cad636060cf04fbb3d3d29a26322f
glib version  : 979da1da482cfd11e20f0fc6b9c23dcbc2cd0017 2.70.2

## 编译 Loongarch64 Host 的内核
huxueshi@172.17.103.58 linux-4.19-loongson
5cc11160aaf9e6904aa14f8ebc00597d028447cc
但是需要修复一个问题:
- 打开 SCHEDSTATS 的选项
  - kernel hacking : collect schedular statistics
- 打开 CONFIG_PCIE_LOONGSON

### 调试
- kernel hacking -> Compile-time checkes and compiler options 打开 DEBUG_INFO 选项

### 修复
实际上这个内核有两个问题需要修复增加两个 patch
- hwbreak.patch
- non_lazy_lbt.patch

### 找一个可以正确运行 KVM 的 QEMU
clone 这个项目 http://rd.loongson.cn:8081/#/admin/projects/kernel/qemu

branch : centos8-4.2.0
commit-id : bc0f70533ca89cc275cfb5b7ab486e45fb70e041
使用项目中的 ./compile_loongarch_kvm.sh 或者下面的(主要是删掉了几个无聊的选项，实际上可以更简单)
```c
mkdir build && cd build
../configure --target-list="loongarch64-softmmu"  --enable-werror --enable-kvm \
            --enable-tcg-interpreter --enable-spice --prefix=/usr --enable-libusb \
            --enable-debug \
            --enable-libiscsi --enable-vnc --enable-vnc-jpeg --enable-vnc-png \
            --enable-libssh --disable-capstone --disable-seccomp --disable-virglrenderer \
            --enable-virtfs
```


## 复现 xqm 的工作
- 获取 image
  - http://old-releases.ubuntu.com/releases/10.04.0/ 中找到 ubuntu-10.04-server-i386.iso 下载，然后可以安装(tcg / kvm 都可以)

- 编译参数
```sh
mkdir build
../configure --target-list=i386-softmmu --enable-latx --disable-werror
```
- 运行
```sh
./run_xqm.sh
```

## 处理 QEMU 的 macro

生成中间文件
```plain
../configure --target-list=x86_64-softmmu  --disable-werror --extra-cflags='-save-temps'
```
但是这似乎导致无法 `make -j10`, 只能串行编译

## 开发环境
- sshfs
- rsync : 使用下面的脚本可以
```c
#!/bin/bash
if [ $# -eq 0 ];then
  echo "need parameter"
  exit 0
fi
echo "sync $1"
rsync --delete -avzh --exclude='/.git' --filter="dir-merge,- .gitignore" maritns3@10.90.50.149:/home/maritns3/core/ld/"$1" /home/loongson/ld
```

## debug 技巧

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
  - 需要打开的选项
    - TTY
    - prink
    - ELF
    - init RAM disk (initrd)

- 内核下载地址 : https://mirrors.edge.kernel.org/pub/linux/kernel/v4.x/
- 版本 : linux-4.4.142.tar.gz

在 arch/x86/boot/main.c 中将 set_video 注释掉就可以继续向下运行了。

### x86 上运行运行 32bit 内核
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
