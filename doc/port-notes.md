## 移植过程中的一些记录

1. 存在一种编程方法，将一个头文件 include 两次从而实现 template 的，但是这种方法会影响 ccls 的定位。
  - cpu_ldst_template.h 这个文件在 6.0 版本中被替换掉了，这一部分的代码是按照 6.0 的

2. 在文件夹的组织上
  1. 将原来的 tcg 和 accel/tcg 都合并到一个位置了

3. 暂时将原来在 qemu 中放到顶层目录中的文件放到 qeum 下面了，为了防止破坏原来的一点点 firmware 的代码

4. 关于 CONFIG_MACHINE_BSWAP_H 这个 macro
    - 是否定义是 configure 的时候传递参数定义的
    - 检查 musl 的源码，发现，当 CONFIG_MACHINE_BSWAP_H 之后，可以使用 libc 的内容，否则使用 QEMU 提供的
