# latx

- 调用 helper 的时候就进入到 qemu 中间了，当前
  - Niugenen 说切到 helper 这里实际上取决于是否破坏环境，有的不用处理的
  - src/i386/LATX/translator/tr_sys_helper.c 中关于 save 的几个函数, 总之这些东西在调用 helper 的时候是会保护起来的，但是可以确定，这是没有调用 prologue 的，那将会导致从 tb 执行中退出
      - context_save
      - context_save_state
      - tr_save_temp_register
