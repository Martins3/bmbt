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

## 调试 kvm
有时候，kvm 本身可能的出现 bug，这导致 guest 无法测试:

如果 kvm 不是 modulus 的话:
- 在 make menuconfig 中将 kvm 修改为 modulus
- sudo make install && sudo make modules_install
- 重启

之后:
- lsmod 查看 kvm 模块
- sudo rmmod kvm # 删除已经存在的 kvm 模块
- sudo insmod ./arch/loongarch/kvm/kvm.ko # 重新安装

```sh
make -j && sudo rmmod kvm && sudo insmod ./arch/loongarch/kvm/kvm.ko
```

## dmesg may be truncated
```sh
sudo dmesg -w|tee b.txt
```

## 处理 QEMU 的 macro

生成中间文件
```plain
../configure --target-list=x86_64-softmmu  --disable-werror --extra-cflags='-save-temps'
```
但是这似乎导致无法 `make -j10`, 只能串行编译

## 系统态

使用脚本 script/backtrace.sh 可以显示 crash 之后的 backtrace ，在 .bashrc 添加 alias 更加方便:
```sh
alias bt=/home/loongson/core/bmbt/script/backtrace.sh
```

## 在代码中获取 guest id
```c
#include "cpu.h"

CPUX86State *env = ((CPUX86State *)current_cpu->env_ptr);
printf("[huxueshi:%s:%d] %x\n", __FUNCTION__, __LINE__, env->segs[R_CS].base + env->eip);
```

## 使用 gdb 从 ip 获取到 line
- gdb
- disass 0x1234456
