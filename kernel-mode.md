# kernel mode

记录一下用于便于切换到 kernel mode 的一些事情。


- [ ] capstone 的 kernel 化似乎比想想的容易，可以继续跟踪一下线索 `_KERNEL_MODE`
  - `_KERNEL_MODE` 表示为 Windows driver 的
- [ ] icache_flush
- [ ] 为了 `__popcountdi2` 现在需要 lgcc, 这个应该不会导致 kernel 中不能运行吧

- `__builtin___clear_cache` 在系统态还可以用吗 ?
