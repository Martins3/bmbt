# capstone

- [ ] 下面是 capstone 实际上的需要的 CFLAGS 的，实际上，真正添加的很少

```Makefile
CFLAGS="-O2
-U_FORTIFY_SOURCE
-D_FORTIFY_SOURCE=2
-g -I/usr/include/pixman-1
-I/home/loongson/ld/x86-qemu-mips/dtc/libfdt
-pthread -I/usr/include/glib-2.0
-I/usr/lib/loongarch64-linux-gnu/glib-2.0/include
-D_GNU_SOURCE
-D_FILE_OFFSET_BITS=64
-D_LARGEFILE_SOURCE
-fno-strict-aliasing
-fno-common
-fwrapv
-std=gnu99
-fstack-protector-strong
-I/usr/include/libpng16
-I/usr/include/spice-server
-I/usr/include/spice-1
-I/home/loongson/ld/x86-qemu-mips/capstone/include
-I/home/loongson/ld/x86-qemu-mips/tests
-DCAPSTONE_USE_SYS_DYN_MEM
-DCAPSTONE_HAS_ARM
-DCAPSTONE_HAS_ARM64
-DCAPSTONE_HAS_POWERPC
-DCAPSTONE_HAS_X86"
```

- [ ] 头文件重构一下
