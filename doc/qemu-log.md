# qemu log

- [ ] QEMU 的 log 可以实现的功能是什么?
  - 可以导入到一个文件中间
  - dfilter : 只是输出某一个方位的 log 出来
    - QEMU_OPTION_DFILTER / qemu_log_in_addr_range / qemu_set_dfilter_ranges 使用
  - qemu_log_items : 用于选择到底 log 那些内容，不是划分等级的，而是通过 mask 来确定 log 哪一个部分
- [ ] 为什么会和 trace 机制关联起来?

## 问题
1. log debug 和 trace : 其实暂时可以补全的, 都是一些 printf 而已
    1. tlb_debug
    2. do_tb_flush 中间又是直接使用 printf 的
    7. qemu_log_mask_and_addr : 和 qemu_log_mask 的区别在于只有在地址范围内才会打印日志
    - [ ] 现在整个 log 体系变的非常的鬼畜，因为开始的懒得移植，后来有准备移植了，现在就是既搞不清楚整个 log 的设计，也没有想清楚怎么移植到裸机环境中
2. log printf 和 qemu printf
    1. include/qemu/log-for-trace.h 和 include/qemu/log-for-trace.h : 一起搞出来 mask 基于日志的角度
    2. qemu_printf 和 printf 的区别在于当前是否存在 monitor

## 主要关联的文件
- /home/maritns3/core/xqm/util/log.c
