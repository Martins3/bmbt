# Divergence between qemu

## 注意
1. 存在一种编程方法，将一个头文件 include 两次从而实现 template 的，但是这种方法会影响 ccls 的定位。
  - cpu_ldst_template.h 这个文件在 6.0 版本中被替换掉了，这一部分的代码是按照 6.0 的

2. 在文件夹的组织上
  1. 将原来的 tcg 和 accel/tcg 都合并到一个位置了

3. 暂时将原来在 qemu 中放到顶层目录中的文件放到 qeum 下面了，为了防止破坏原来的一点点 firmware 的代码
