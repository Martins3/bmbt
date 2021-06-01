
1. can't compile qemu in loongarch
```
make[1]: 进入目录“/home/loongson/ld/qemu_bak/slirp”
make[1]: 对“all”无需做任何事。
make[1]: 离开目录“/home/loongson/ld/qemu_bak/slirp”
  LINK    tests/qemu-iotests/socket_scm_helper
  CC      qga/commands.o
  CC      qga/guest-agent-command-state.o
  CC      qga/main.o
  CC      qga/commands-posix.o
  CC      qga/channel-posix.o
  CC      qga/qapi-generated/qga-qapi-types.o
  CC      qga/qapi-generated/qga-qapi-visit.o
make: *** 没有规则可制作目标“qapi/qapi-visit-core.o”，由“libqemuutil.a” 需求。 停止。
make: *** 正在等待未完成的任务....
  CC      qga/qapi-generated/qga-qapi-commands.o
```
doesn't sync the repo correctly.
