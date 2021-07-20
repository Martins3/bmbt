## 来自 header 的终极挑战: NEED_CPU_H

NEED_CPU_H 中间的，这个东西出现的位置
1. helper_head
2. memop.h 中间对于这个东西存在一个临时定义的
3. memory.h 中间定义

从 /home/maritns3/core/kvmqemu/meson.build 中找到 NEED_CPU_H ，其中的
看来是所有在 target 下的代码都是需要 NEED_CPU_H 的。

对于 NEED_CPU_H 的理解:
1. 如果没有 #ifdef NEED_CPU_H ，那么这个 macro 就没有什么意义了
2. 如果只有 #ifdef NEED_CPU_H 没有对应的 else，多定义出来的类型应该问题不大
3. 所以，唯一的挑战就是在 memop.h 导致结构体 MemOp 的含义不同

## 资料
https://patchwork.kernel.org/project/qemu-devel/patch/1455818725-7647-7-git-send-email-peter.maydell@linaro.org/

> `NEED_CPU_H` is the define we use to distinguish per-target object
compilation from common object compilation. For the former, we must
also include config-target.h so that the .c files see the necessary
`CONFIG_` constants.

看一下几个添加 NEED_CPU_H 的 commit:
1. git show 1c14f162dd92c0448948791531dc82ac277330ae
```diff
Author: Blue Swirl <blauwirbel@gmail.com>
Date:   Mon Mar 29 19:23:47 2010 +0000

    Allow various header files to be included from non-CPU code

    Allow balloon.h, gdbstub.h and kvm.h to be included from
    non-CPU code.

    Signed-off-by: Blue Swirl <blauwirbel@gmail.com>
```
- [ ] 所以, 只要添加了 NEED_CPU_H 就可以叫做是可以被 non-CPU node ?
- [ ] 为什么会引出 non-cpu code 的概念啊

2. git show 13b48fb00e61dc7662da27c020c3263b74374acc

```diff
commit 13b48fb00e61dc7662da27c020c3263b74374acc
Author: Thomas Huth <thuth@redhat.com>
Date:   Wed Apr 14 13:20:01 2021 +0200

    include/sysemu: Poison all accelerator CONFIG switches in common code

    We are already poisoning CONFIG_KVM since this switch is not working
    in common code. Do the same with the other accelerator switches, too
    (except for CONFIG_TCG, which is special, since it is also defined in
    config-host.h).

    Message-Id: <20210414112004.943383-2-thuth@redhat.com>
    Reviewed-by: Philippe Mathieu-Daudé <philmd@redhat.com>
    Signed-off-by: Thomas Huth <thuth@redhat.com>
```

