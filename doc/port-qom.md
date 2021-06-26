# QEMU QOM
我不想 port QOM，我只是想要移除掉 QOM

- [ ] 将 vn 中间的资料移动过来

- QOM 到底解决了什么问题
  - [ ] 各种初始化 ?
  - [ ] 为了模拟出来设备的层次关系 ?

- QOM 功能总结 : 找到官方文档，理解那些词汇的含义，深入理解一下面向对象的含义
  - refer counter : object_unref


## TODO
- [ ] 各种 realizefn 是如何使用的

- [ ] QOM 不是存在一个标准的教学吗?

- [ ] 到底存在那几个关键概念
  - Type
  - Class
  - ?

- [ ] 一个 ObjectProperty 和普通的函数有什么区别啊 ?
  - 为什么还有普通的指针啊 ?

- [ ] object_class_property_init_all 函数是不是说明，其实还存在 class property

- [ ] TypeInfo 
- [ ] Class : 为什么 Class 也是可以继承的， class 和 typeinfo 是什么关系?


关于 QOM 的进一步参考
- [ ] http://juniorprincewang.github.io/categories/QEMU/
- [ ] https://qemu.readthedocs.io/en/latest/devel/qom.html

## TypeInfo

#### type_init
`type_init(pc_machine_register_types)`

- [x] type_init 注册的函数什么时候会被调用? (在 constructor 中注册，在 register_module_init 中全体调用)
- [ ] 将 TypeInfo 注册了，其中的 class_init 和 instance_init 什么时候调用


- type_init : 携带参数 MODULE_INIT_QOM 调用 module_init
  - module_init
    - register_module_init
      - 创建一个 ModuleEntry, 其携带了需要执行的函数
      - find_type : 将这个 ModuleEntry 放到 ModuleTypeList, 这个 list 每一个类型一个，例如 MODULE_INIT_QOM 使用一个

- main
  - qemu_init
    - qemu_init_subsystems
      - module_call_init : 携带参数 MODULE_INIT_QOM, 那么将会导致曾经靠 type_init 注册上的所有函数全部都调用
        - pc_machine_register_types : 一个例子，类似的函数很多
          - type_register_static : 一般都是调用一个函数，其参数是一个静态定义的 TypeInfo ，例如 x86_cpu_type_info
            - type_register
              - type_register_internal
                - type_new : 使用 TypeInfo 初始化 TypeImpl，基本是拷贝
                - g_hash_table_insert(type_table_get(), (void *)ti->name, ti) : 将创建的 TypeImpl 添加到 type_table 中

```c
/*
>>> bt
#0  nvme_class_init (oc=0x55555674a270, data=0x0) at ../hw/block/nvme.c:6312
#1  0x0000555555c9c29f in type_initialize (ti=0x555556588e30) at ../qom/object.c:1079
#2  object_class_foreach_tramp (key=<optimized out>, value=0x555556588e30, opaque=0x7fffffffd600) at ../qom/object.c:1079
#3  0x00007ffff6ed01b8 in g_hash_table_foreach () at /lib/x86_64-linux-gnu/libglib-2.0.so.0
#4  0x0000555555c9c8dc in object_class_foreach (fn=fn@entry=0x555555c9af70 <object_class_get_list_tramp>, implements_type=implements_type@entry=0x55555612a2d0 "machine" , include_abstract=include_abstract@entry=false, opaque=opaque@entry=0x7fffffffd640) at ../qom/object.c:85
#5  0x0000555555c9c986 in object_class_get_list (implements_type=implements_type@entry=0x55555612a2d0 "machine", include_abstract=include_abstract@entry=false) at ../qo m/object.c:1158
#6  0x0000555555baec35 in select_machine () at ../softmmu/vl.c:3545
#7  qemu_init (argc=<optimized out>, argv=0x7fffffffd8d8, envp=<optimized out>) at ../softmmu/vl.c:3545
#8  0x000055555582b4bd in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49

#0  nvme_instance_init (obj=0x555557c93920) at ../hw/block/nvme.c:6328
#1  0x0000555555c9ce2c in object_initialize_with_type (obj=obj@entry=0x555557c93920, size=size@entry=18496, type=type@entry=0x555556588e30) at ../qom/object.c:527
#2  0x0000555555c9cf79 in object_new_with_type (type=0x555556588e30) at ../qom/object.c:742
#3  0x0000555555cac8fa in qdev_new (name=name@entry=0x5555565d01e0 "nvme") at ../hw/core/qdev.c:153
#4  0x0000555555858844 in qdev_device_add (opts=0x5555565d02d0, errp=errp@entry=0x5555564e2e30 <error_fatal>) at ../softmmu/qdev-monitor.c:650
#5  0x0000555555babb53 in device_init_func (opaque=<optimized out>, opts=<optimized out>, errp=0x5555564e2e30 <error_fatal>) at ../softmmu/vl.c:1211
#6  0x0000555555d20fb2 in qemu_opts_foreach (list=<optimized out>, func=func@entry=0x555555babb40 <device_init_func>, opaque=opaque@entry=0x0, errp=errp@entry=0x5555564 e2e30 <error_fatal>) at ../util/qemu-option.c:1167
#7  0x0000555555bae255 in qemu_create_cli_devices () at ../softmmu/vl.c:2541
#8  qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2589
#9  0x0000555555bb1e02 in qemu_init (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/vl.c:3611
#10 0x000055555582b4bd in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
*/
```

```c
/*
#0  x86_cpu_common_class_init (oc=0x555555e7e7b8 <device_class_base_init+32>, data=0x7fffffffd330) at ../target/i386/cpu.c:6737
#1  0x0000555555e64a46 in type_initialize (ti=0x55555686beb0) at ../qom/object.c:364
#2  0x0000555555e647b1 in type_initialize (ti=0x555556872820) at ../qom/object.c:312
#3  0x0000555555e66254 in object_class_foreach_tramp (key=0x5555568729a0, value=0x555556872820, opaque=0x7fffffffd4c0) at ../qom/object.c:1069
#4  0x00007ffff70381b8 in g_hash_table_foreach () at /lib/x86_64-linux-gnu/libglib-2.0.so.0
#5  0x0000555555e66337 in object_class_foreach (fn=0x555555e664a0 <object_class_get_list_tramp>, implements_type=0x5555560cb618 "machine", include_abstract=false, opaqu
e=0x7fffffffd510) at ../qom/object.c:1091
#6  0x0000555555e66523 in object_class_get_list (implements_type=0x5555560cb618 "machine", include_abstract=false) at ../qom/object.c:1148
#7  0x0000555555cd8bb0 in select_machine () at ../softmmu/vl.c:1629
#8  0x0000555555cdd514 in qemu_init (argc=28, argv=0x7fffffffd7c8, envp=0x7fffffffd8b0) at ../softmmu/vl.c:3570
#9  0x000055555582e575 in main (argc=28, argv=0x7fffffffd7c8, envp=0x7fffffffd8b0) at ../softmmu/main.c:49

#0  x86_cpu_initfn (obj=0x55555699acb0) at ../target/i386/cpu.c:6426
#1  0x0000555555e64ab0 in object_init_with_type (obj=0x555556c8bf90, ti=0x55555686beb0) at ../qom/object.c:375
#2  0x0000555555e64a92 in object_init_with_type (obj=0x555556c8bf90, ti=0x55555687da00) at ../qom/object.c:371
#3  0x0000555555e64a92 in object_init_with_type (obj=0x555556c8bf90, ti=0x55555687df20) at ../qom/object.c:371
#4  0x0000555555e6500b in object_initialize_with_type (obj=0x555556c8bf90, size=42944, type=0x55555687df20) at ../qom/object.c:517
#5  0x0000555555e65740 in object_new_with_type (type=0x55555687df20) at ../qom/object.c:732
#6  0x0000555555e6579f in object_new (typename=0x55555687e0a0 "host-x86_64-cpu") at ../qom/object.c:747
#7  0x0000555555b67369 in x86_cpu_new (x86ms=0x555556a94800, apic_id=0, errp=0x5555567a94b0 <error_fatal>) at ../hw/i386/x86.c:106
#8  0x0000555555b67485 in x86_cpus_init (x86ms=0x555556a94800, default_cpu_version=1) at ../hw/i386/x86.c:138
#9  0x0000555555b7b69b in pc_init1 (machine=0x555556a94800, host_type=0x55555609e70a "i440FX-pcihost", pci_type=0x55555609e703 "i440FX") at ../hw/i386/pc_piix.c:157
#10 0x0000555555b7c24e in pc_init_v6_1 (machine=0x555556a94800) at ../hw/i386/pc_piix.c:425
#11 0x0000555555aec313 in machine_run_board_init (machine=0x555556a94800) at ../hw/core/machine.c:1239
#12 0x0000555555cdada6 in qemu_init_board () at ../softmmu/vl.c:2526
#13 0x0000555555cdaf85 in qmp_x_exit_preconfig (errp=0x5555567a94b0 <error_fatal>) at ../softmmu/vl.c:2600
#14 0x0000555555cdd65d in qemu_init (argc=28, argv=0x7fffffffd7c8, envp=0x7fffffffd8b0) at ../softmmu/vl.c:3635
#15 0x000055555582e575 in main (argc=28, argv=0x7fffffffd7c8, envp=0x7fffffffd8b0) at ../softmmu/main.c:49
```
#### TypeInfo::class_init
无论是 nvme 还是 x86_cpu_type_info 的 class_init 的调用位置都是相同的:

让我们看看 select_machine 中的  object_class_get_list 得到的结果: 

```
- object_class_get_list
  - object_class_foreach --> object_class_get_list_tramp (将元素添加到后面) <------------
    - g_hash_table_foreach (对于 type_table 循环) ---> object_class_foreach_tramp       |
                                                          - type_initialize             |
                                                          - object_class_dynamic_cast   |
                                                            - 执行 callback 函数 --------
```
- 这次循环，应该会导致所有的 type 都被 type_initialize 掉。
- type_initialize 
  - 分配 TypeImpl::class, 
  - 会将自己 以及各级 parent 注册的 class_init 被调用

分析一下 x86_cpu_common_class_init 的实现:
- X86_CPU_CLASS，CPU_CLASS 和 DEVICE_CLASS : 使用通过一个参数就可以获取
- [ ]  `device_class_set_parent_realize(dc, x86_cpu_realizefn, &xcc->parent_realize);`
  - [ ] 什么时候调用的 x86_cpu_realizefn
  - [ ] x86_cpu_realizefn 是一个非常关键的函数
  - [ ] 参数是 dc
- [ ] 这里对于 cc (CPUClass) 进行一堆初始化，为什么不是 xcc (X86CPUClass) 的内容初始化


#### [ ]  TypeInfo::instance_size 和 TypeInfo::class_size 作用
显然是用于提前分配这个对象
更加重要的是，这个东西关联了 X86CPU 和 X86CPUClass, 至于 X86CPUClass 的初始化，
就是在

```c
static const TypeInfo x86_cpu_type_info = {
    .name = TYPE_X86_CPU,
    .parent = TYPE_CPU,
    .instance_size = sizeof(X86CPU),
    .instance_init = x86_cpu_initfn,
    .abstract = true,
    .class_size = sizeof(X86CPUClass),
    .class_init = x86_cpu_common_class_init,
};
```

#### instance 如何初始化 ?
通过 object_new 初始化的

- x86_cpu_new
  - object_new
    - object_new_with_type
      - type_get_by_name : 参数为 host-x86_64-cpu ，因为提供了参数 -cpu host，具体的初始化在 qemu_init 中
      - [ ] object_new_with_type
        - 分配空间 TypeImpl::instance_size
        - object_initialize_with_type
          - object_init_with_type : 这个函数会首先初始化 parent 类型，然后进行本地初始化
  - [ ] qdev_realize : realize 的通用设计模式是什么?


从 backtrace 看，kvm -cpu host 的关于 CPU 存在如下
- [ ] device - x86_64-cpu - max-x86_64-cpu - host-x86_64-cpu

#### OBJECT_DECLARE_TYPE
- 初始化的时候，只是告诉了 instance_size 和 class_size, 而且也告诉了 class 和 instance 的初始化函数


在 x86_cpu_new 的一行代码，相当于获取从 X86MachineState 获取 MachineState, 从而知道 cpu_type, 从而初始化 x86
- [ ] 其中的 x86ms (X86MachineState) 如何靠 MACHINE 的这个 marco 装换为 MachineState
- [ ] 当两个 OBJECT_DECLARE_TYPE 展开到底得到什么东西 ? 根本没有提到 


```c
Object *cpu = object_new(MACHINE(x86ms)->cpu_type);

OBJECT_DECLARE_TYPE(MachineState, MachineClass, MACHINE)
```
使用 gcc -E 展开，可以得到
```c
typedef struct MachineState MachineState;
typedef struct MachineClass MachineClass;
G_DEFINE_AUTOPTR_CLEANUP_FUNC(MachineState, object_unref)
static inline G_GNUC_UNUSED MachineState *MACHINE(const void *obj) {
  return ((MachineState *)object_dynamic_cast_assert(
      ((Object *)(obj)), ("machine"), "a.c", 49, __func__));
}
static inline G_GNUC_UNUSED MachineClass *MACHINE_GET_CLASS(const void *obj) {
  return ((MachineClass *)object_class_dynamic_cast_assert(
      ((ObjectClass *)(object_get_class(((Object *)(obj))))), ("machine"),
      "a.c", 49, __func__));
}
static inline G_GNUC_UNUSED MachineClass *MACHINE_CLASS(const void *klass) {
  return ((MachineClass *)object_class_dynamic_cast_assert(
      ((ObjectClass *)(klass)), ("machine"), "a.c", 49, __func__));
}
```
这里的 a.c 和 数值 是 OBJECT_DECLARE_TYPE 定义的位置。

- ObjectClass : The base for all classes
- Object : 持有一个指针 ObjectClass, 而 Object 持有一个 struct TypeImpl * ，所以可以动态的查找到一个 object 真正类型

- object_dynamic_cast_assert : 将一个 object cast 成为类型，且参数为 "machine"
  - 如果没有进行 CONFIG_QOM_CAST_DEBUG, 那么什么都不需要做，因为这些类型都是嵌套到一起的
  - 首先扫描一下 object_cast_cache 中是否以前从这个 object cast 到过参数的类型，如果之前正确，那么现在肯定正确
  - object_dynamic_cast
    - object_class_dynamic_cast : 简单来说, 通过 type_get_by_name 找到 TypeImpl, 然后通过 type_is_ancestor 就可以判断了
  - 装换成功，设置 object_cast_cache
- object_class_dynamic_cast_assert : 就会 cache 机制和调用一下 object_class_dynamic_cast

object_dynamic_cast_assert 真正恐怖的地方在于，现在所有的对象都是都是可以装换为 Object 类型，
而一个 object 类型的变量，实际上，可以在完全缺乏上下文的环境中 cast 到可以 cast 的任何类型。
而这个关键在于，Object 中通过 ObjectClass 知道自己的真正的类型。


#### InterfaceClass
object_class_dynamic_cast 的 happy path 实现是很容易的，但是其中涉及到了 InterfaceClass

- register_types 中定义了 interface_info 和 object_info 两个根 TypeInfo
```c
    static TypeInfo interface_info = {
        .name = TYPE_INTERFACE,
        .class_size = sizeof(InterfaceClass),
        .abstract = true,
    };

    static TypeInfo object_info = {
        .name = TYPE_OBJECT,
        .instance_size = sizeof(Object),
        .class_init = object_class_init,
        .abstract = true,
    };
```

#### [ ] X86CPUClass 和 X86CPU 的定位区别

- 如果 instance 的 size == 0, 那么说明是抽象的

- [ ]  是不是说 X86CPUClass 只有一个，但是 X86CPU 是可以存在多个

- [ ] Class 和 Instance 的关系是什么 ?
  - 是不是，如果 TypeInfo 定义了多少层次的继承结构，那么 Class 和 Instance 都必须定义对应层次的继承结构

- [ ] 存在一些根本不关联 instance 和 class 的 Type
```c
static const TypeInfo conventional_pci_interface_info = {
    .name          = INTERFACE_CONVENTIONAL_PCI_DEVICE,
    .parent        = TYPE_INTERFACE,
};
```



## object_property : pci_e1000_realize
- [ ] class 可以存在 property 吗？


```c
struct ObjectProperty
{
    char *name;
    char *type;
    char *description;
    ObjectPropertyAccessor *get;
    ObjectPropertyAccessor *set;
    ObjectPropertyResolve *resolve;
    ObjectPropertyRelease *release;
    ObjectPropertyInit *init;
    void *opaque;
    QObject *defval;
};
```
- [ ] 从一个普通的调用变为 pci_e1000_realize
```c
/**
// 检测这个 memory region 是如何被加入的, 这个时候的大小是确定的
#0  huxueshi () at ../softmmu/memory.c:1188
#1  0x0000555555b8eeb5 in memory_region_init (mr=0x5555579fa5c0, owner=0x5555579f7ca0, name=<optimized out>, size=131072) at ../softmmu/memory.c:1198
#2  0x0000555555b8ef5c in memory_region_init_io (mr=mr@entry=0x5555579fa5c0, owner=owner@entry=0x5555579f7ca0, ops=ops@entry=0x5555562d9520 <e1000_mmio_ops>, opaque=opa que@entry=0x5555579f7ca0, name=name@entry=0x555555dc1976 "e1000-mmio", size=size@entry=131072) at ../softmmu/memory.c:1532
#3  0x000055555590a682 in e1000_mmio_setup (d=0x5555579f7ca0) at ../hw/net/e1000.c:1640
#4  pci_e1000_realize (pci_dev=0x5555579f7ca0, errp=<optimized out>) at ../hw/net/e1000.c:1698
#5  0x000055555596eb91 in pci_qdev_realize (qdev=0x5555579f7ca0, errp=<optimized out>) at ../hw/pci/pci.c:2117
#6  0x0000555555cadec7 in device_set_realized (obj=<optimized out>, value=true, errp=0x7fffffffd300) at ../hw/core/qdev.c:761
#7  0x0000555555c9b21a in property_set_bool (obj=0x5555579f7ca0, v=<optimized out>, name=<optimized out>, opaque=0x5555565d1db0, errp=0x7fffffffd300) at ../qom/object.c :2257
#8  0x0000555555c9d72c in object_property_set (obj=obj@entry=0x5555579f7ca0, name=name@entry=0x555555ed7f96 "realized", v=v@entry=0x555556dbfe10, errp=errp@entry=0x5555 564e2e30 <error_fatal>) at ../qom/object.c:1402
#9  0x0000555555c9fa64 in object_property_set_qobject (obj=obj@entry=0x5555579f7ca0, name=name@entry=0x555555ed7f96 "realized", value=value@entry=0x555556d9d790, errp=e rrp@entry=0x5555564e2e30 <error_fatal>) at ../qom/qom-qobject.c:28
#10 0x0000555555c9d979 in object_property_set_bool (obj=0x5555579f7ca0, name=0x555555ed7f96 "realized", value=<optimized out>, errp=0x5555564e2e30 <error_fatal>) at ../ qom/object.c:1472
#11 0x0000555555cacd93 in qdev_realize_and_unref (dev=dev@entry=0x5555579f7ca0, bus=bus@entry=0x555556e0f800, errp=<optimized out>) at ../hw/core/qdev.c:396
#12 0x000055555596d209 in pci_realize_and_unref (dev=dev@entry=0x5555579f7ca0, bus=bus@entry=0x555556e0f800, errp=<optimized out>) at ../hw/pci/pci.c:2182
#13 0x000055555596d437 in pci_nic_init_nofail (nd=nd@entry=0x5555564c4300 <nd_table>, rootbus=rootbus@entry=0x555556e0f800, default_model=default_model@entry=0x555555d7 227c "e1000", default_devaddr=default_devaddr@entry=0x0) at ../hw/pci/pci.c:1957
#14 0x0000555555a62b20 in pc_nic_init (pcmc=pcmc@entry=0x5555567978b0, isa_bus=0x5555568b4980, pci_bus=pci_bus@entry=0x555556e0f800) at ../hw/i386/pc.c:1189
#15 0x0000555555a65bed in pc_init1 (machine=0x5555566c0000, pci_type=0x555555dbe5ad "i440FX", host_type=0x555555d80e54 "i440FX-pcihost") at ../hw/i386/pc_piix.c:244
#16 0x00005555558ff1ae in machine_run_board_init (machine=machine@entry=0x5555566c0000) at ../hw/core/machine.c:1232
#17 0x0000555555bae25e in qemu_init_board () at ../softmmu/vl.c:2514
#18 qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2588
#19 0x0000555555bb1ea2 in qemu_init (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/vl.c:3611
#20 0x000055555582b4bd in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
```

qdev_realize_and_unref
```c
/**
 * DeviceState:
 * @realized: Indicates whether the device has been fully constructed.
 *            When accessed outside big qemu lock, must be accessed with
 *            qatomic_load_acquire()
 * @reset: ResettableState for the device; handled by Resettable interface.
 *
 * This structure should not be accessed directly.  We declare it here
 * so that it can be embedded in individual device state structures.
 */
struct DeviceState {
    /*< private >*/
    Object parent_obj;
    /*< public >*/

    const char *id;
    char *canonical_path;
    bool realized;
    bool pending_deleted_event;
    QemuOpts *opts;
    int hotplugged;
    bool allow_unplug_during_migration;
    BusState *parent_bus;
    QLIST_HEAD(, NamedGPIOList) gpios;
    QLIST_HEAD(, NamedClockList) clocks;
    QLIST_HEAD(, BusState) child_bus;
    int num_child_bus;
    int instance_id_alias;
    int alias_required_for_version;
    ResettableState reset;
};

/**
 * BusState:
 * @hotplug_handler: link to a hotplug handler associated with bus.
 * @reset: ResettableState for the bus; handled by Resettable interface.
 */
struct BusState {
    Object obj;
    DeviceState *parent;
    char *name;
    HotplugHandler *hotplug_handler;
    int max_index;
    bool realized;
    int num_children;

    /*
     * children is a RCU QTAILQ, thus readers must use RCU to access it,
     * and writers must hold the big qemu lock
     */

    QTAILQ_HEAD(, BusChild) children;
    QLIST_ENTRY(BusState) sibling;
    ResettableState reset;
};
```
实际上，qdev_realize_and_unref 都是各种总线模块的调用，比如 pci, 也就是说，代码设计上，就是设备和总线关联起来的。

- qdev_realize_and_unref
  - qdev_realize
    - qdev_set_parent_bus : bus 和 dev 的关系确定
    - object_property_set_bool(OBJECT(dev), "realized", true, errp);
      - object_property_set_qobject
        - qobject_input_visitor_new : *TODO* 真 NM 离谱, 将 Qbool 作为参数，创建 Visitor
        - object_property_set
           - object_property_find_err
              - object_property_find 
                  - object_get_class
                  - object_class_property_find : 递归的查找这个 property
                  - g_hash_table_lookup
  - object_unref

```c
/*
>>> bt
#0  object_property_try_add (obj=0x5555566c0000, name=0x5555567dc8c0 "peripheral", type=0x5555567dc990 "child<container>", get=0x555555c9eac0 <object_get_child_property
>, set=0x0, release=0x555555c9cc70 <object_finalize_child_property>, opaque=0x5555567dc900, errp=0x5555564e2e38 <error_abort>) at ../qom/object.c:1196
#1  0x0000555555c9e401 in object_property_try_add_child (obj=0x5555566c0000, name=0x5555567dc8c0 "peripheral", child=0x5555567dc900, errp=0x5555564e2e38 <error_abort>)
at ../qom/object.c:1744
#2  0x0000555555c99e25 in container_get (root=root@entry=0x5555566c0000, path=path@entry=0x555555d72051 "/peripheral") at ../qom/container.c:41
#3  0x00005555558ff749 in machine_initfn (obj=0x5555566c0000) at ../hw/core/machine.c:923
#4  0x0000555555c9b7e6 in object_init_with_type (obj=0x5555566c0000, ti=0x5555565a1d70) at ../qom/object.c:371
#5  0x0000555555c9b7e6 in object_init_with_type (obj=0x5555566c0000, ti=0x55555659d180) at ../qom/object.c:371
#6  0x0000555555c9b7e6 in object_init_with_type (obj=0x5555566c0000, ti=0x55555659d8e0) at ../qom/object.c:371
#7  0x0000555555c9ce2c in object_initialize_with_type (obj=0x5555566c0000, size=<optimized out>, type=0x55555659d8e0) at ../qom/object.c:517
#8  0x0000555555c9cf79 in object_new_with_type (type=0x55555659d8e0) at ../qom/object.c:732
#9  0x0000555555baf103 in qemu_create_machine (machine_class=0x5555567978b0) at ../softmmu/vl.c:2067
#10 qemu_init (argc=<optimized out>, argv=0x7fffffffd968, envp=<optimized out>) at ../softmmu/vl.c:3545
#11 0x000055555582b4bd in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
*/
```

使用 -kernel 作为例子:
```c
    object_class_property_add_str(oc, "kernel", machine_get_kernel, machine_set_kernel);
```

相当于创建了 option 和 method 之间的方法:
```c
/**
#0  machine_set_kernel (obj=0x5555566c0000, value=0x5555569562a0 "/home/maritns3/core/ubuntu-linux/arch/x86/boot/bzImage", errp=0x7fffffffd610) at ../hw/core/machine.c: 244
#1  0x0000555555c9b0f7 in property_set_str (obj=0x5555566c0000, v=<optimized out>, name=<optimized out>, opaque=0x5555565ee8a0, errp=0x7fffffffd610) at ../qom/object.c: 2180
#2  0x0000555555c9d70c in object_property_set (obj=0x5555566c0000, name=0x5555565cfff0 "kernel", v=0x5555568e25c0, errp=0x5555564e2e30 <error_fatal>) at ../qom/object.c :1402
#3  0x0000555555c9df84 in object_property_parse (obj=obj@entry=0x5555566c0000, name=name@entry=0x5555565cfff0 "kernel", string=string@entry=0x5555565d00c0 "/home/maritn s3/core/ubuntu-linux/arch/x86/boot/bzImage", errp=errp@entry=0x5555564e2e30 <error_fatal>) at ../qom/object.c:1642
#4  0x0000555555bacf5f in object_parse_property_opt (skip=0x555555e104d6 "type", errp=0x5555564e2e30 <error_fatal>, value=0x5555565d00c0 "/home/maritns3/core/ubuntu-lin ux/arch/x86/boot/bzImage", name=0x5555565cfff0 "kernel", obj=0x5555566c0000) at ../softmmu/vl.c:1651
#5  object_parse_property_opt (errp=0x5555564e2e30 <error_fatal>, skip=0x555555e104d6 "type", value=0x5555565d00c0 "/home/maritns3/core/ubuntu-linux/arch/x86/boot/bzIma ge", name=0x5555565cfff0 "kernel", obj=0x5555566c0000) at ../softmmu/vl.c:1643
#6  machine_set_property (opaque=0x5555566c0000, name=0x5555565cfff0 "kernel", value=0x5555565d00c0 "/home/maritns3/core/ubuntu-linux/arch/x86/boot/bzImage", errp=0x555 5564e2e30 <error_fatal>) at ../softmmu/vl.c:1693
#7  0x0000555555d2041d in qemu_opt_foreach (opts=opts@entry=0x5555565d0010, func=func@entry=0x555555bace80 <machine_set_property>, opaque=0x5555566c0000, errp=errp@entr y=0x5555564e2e30 <error_fatal>) at ../util/qemu-option.c:593
#8  0x0000555555bb11a9 in qemu_apply_machine_options () at ../softmmu/vl.c:1812
#9  qemu_init (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/vl.c:3554
#10 0x000055555582b4bd in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
*/
```

各种 realized 函数, 最后被调用的方式为:

```c
return object_property_set_bool(OBJECT(dev), "realized", true, errp);
```

- [ ] 关于 object_property 的唯一问题在于，总是从 parent 中间搜索的，之后才到 child 中间找


## 类型转换

PIIX3_PCI_DEVICE 是如何将一个 parent 类型 pci_dev 转化为 piix3 的
```c
pci_dev = pci_create_simple_multifunction(pci_bus, -1, true, TYPE_PIIX3_DEVICE);
piix3 = PIIX3_PCI_DEVICE(pci_dev);
```

在创建 pci_create_simple_multifunction 的参数, TYPE_PIIX3_DEVICE 可以保证创建出来的对象就是一个 piix3

而 PIIX3_PCI_DEVICE 应该只是进行在 child 到 parent 之间的转换使用上了检查吧

## qom-tree

```c
/*
(qemu) info qom-tree
/machine (pc-i440fx-6.0-machine)
  /fw_cfg (fw_cfg_io)
    /\x2from@etc\x2facpi\x2frsdp[0] (memory-region)
    /\x2from@etc\x2facpi\x2ftables[0] (memory-region)
    /\x2from@etc\x2ftable-loader[0] (memory-region)
    /fwcfg.dma[0] (memory-region)
    /fwcfg[0] (memory-region)
  /i440fx (i440FX-pcihost)
    /ioapic (kvm-ioapic)
      /kvm-ioapic[0] (memory-region)
      /unnamed-gpio-in[0] (irq)
      /unnamed-gpio-in[10] (irq)
      /unnamed-gpio-in[11] (irq)
      /unnamed-gpio-in[12] (irq)
      /unnamed-gpio-in[13] (irq)
      /unnamed-gpio-in[14] (irq)
      /unnamed-gpio-in[15] (irq)
      /unnamed-gpio-in[16] (irq)
      /unnamed-gpio-in[17] (irq)
      /unnamed-gpio-in[18] (irq)
      /unnamed-gpio-in[19] (irq)
      /unnamed-gpio-in[1] (irq)
      /unnamed-gpio-in[20] (irq)
      /unnamed-gpio-in[21] (irq)
      /unnamed-gpio-in[22] (irq)
      /unnamed-gpio-in[23] (irq)
      /unnamed-gpio-in[2] (irq)
      /unnamed-gpio-in[3] (irq)
      /unnamed-gpio-in[4] (irq)
      /unnamed-gpio-in[5] (irq)
      /unnamed-gpio-in[6] (irq)
      /unnamed-gpio-in[7] (irq)
      /unnamed-gpio-in[8] (irq)
      /unnamed-gpio-in[9] (irq)
    /pam-pci[0] (memory-region)
    /pam-pci[10] (memory-region)
    /pam-pci[11] (memory-region)
    /pam-pci[12] (memory-region)
    /pam-pci[13] (memory-region)
    /pam-pci[14] (memory-region)
    /pam-pci[15] (memory-region)
    /pam-pci[16] (memory-region)
    /pam-pci[17] (memory-region)
    /pam-pci[18] (memory-region)
    /pam-pci[19] (memory-region)
    /pam-pci[1] (memory-region)
    /pam-pci[20] (memory-region)
    /pam-pci[21] (memory-region)
    /pam-pci[22] (memory-region)
    /pam-pci[23] (memory-region)
    /pam-pci[24] (memory-region)
    /pam-pci[25] (memory-region)
    /pam-pci[2] (memory-region)
    /pam-pci[3] (memory-region)
    /pam-pci[4] (memory-region)
    /pam-pci[5] (memory-region)
    /pam-pci[6] (memory-region)
    /pam-pci[7] (memory-region)
    /pam-pci[8] (memory-region)
    /pam-pci[9] (memory-region)
    /pam-ram[0] (memory-region)
    /pam-ram[10] (memory-region)
    /pam-ram[11] (memory-region)
    /pam-ram[12] (memory-region)
    /pam-ram[1] (memory-region)
    /pam-ram[2] (memory-region)
    /pam-ram[3] (memory-region)
    /pam-ram[4] (memory-region)
    /pam-ram[5] (memory-region)
    /pam-ram[6] (memory-region)
    /pam-ram[7] (memory-region)
    /pam-ram[8] (memory-region)
    /pam-ram[9] (memory-region)
    /pam-rom[0] (memory-region)
    /pam-rom[10] (memory-region)
    /pam-rom[11] (memory-region)
    /pam-rom[12] (memory-region)
    /pam-rom[1] (memory-region)
    /pam-rom[2] (memory-region)
    /pam-rom[3] (memory-region)
    /pam-rom[4] (memory-region)
    /pam-rom[5] (memory-region)
    /pam-rom[6] (memory-region)
    /pam-rom[7] (memory-region)
    /pam-rom[8] (memory-region)
    /pam-rom[9] (memory-region)
    /pci-conf-data[0] (memory-region)
    /pci-conf-idx[0] (memory-region)
    /pci.0 (PCI)
  /peripheral (container)
  /peripheral-anon (container)
    /device[0] (isa-debugcon)
      /isa-debugcon[0] (memory-region)
    /device[1] (nvme)                            // 实际上，nvme 和 e1000 根本不是对称的
      /bus master container[0] (memory-region)
      /bus master[0] (memory-region)
      /msix-pba[0] (memory-region)
      /msix-table[0] (memory-region)
      /nvme-bar0[0] (memory-region)
      /nvme-bus.0 (nvme-bus)
      /nvme[0] (memory-region)
  /unattached (container)                         // 所以，什么是 unattached 的 
    /device[0] (host-x86_64-cpu)
      /lapic (kvm-apic)
        /kvm-apic-msi[0] (memory-region)
    /device[10] (kvm-pit)
      /kvm-pit[0] (memory-region)
      /unnamed-gpio-in[0] (irq)
    /device[11] (isa-pcspk)
      /pcspk[0] (memory-region)
    /device[12] (i8257)
      /dma-chan[0] (memory-region)
      /dma-cont[0] (memory-region)
      /dma-page[0] (memory-region)
      /dma-page[1] (memory-region)
    /device[13] (i8257)
      /dma-chan[0] (memory-region)
      /dma-cont[0] (memory-region)
      /dma-page[0] (memory-region)
      /dma-page[1] (memory-region)
    /device[14] (isa-serial)
      /serial (serial)
      /serial[0] (memory-region)
    /device[15] (isa-parallel)
      /parallel[0] (memory-region)
    /device[16] (isa-fdc)
      /fdc[0] (memory-region)
      /fdc[1] (memory-region)
      /floppy-bus.0 (floppy-bus)
    /device[17] (floppy)
    /device[18] (i8042)
      /i8042-cmd[0] (memory-region)
      /i8042-data[0] (memory-region)
    /device[19] (vmport)
      /vmport[0] (memory-region)
    /device[1] (kvmvapic)
      /kvmvapic-rom[0] (memory-region)
      /kvmvapic[0] (memory-region)
    /device[20] (vmmouse)
    /device[21] (port92)
      /port92[0] (memory-region)
    /device[22] (e1000)
      /bus master container[0] (memory-region)
      /bus master[0] (memory-region)
      /e1000-io[0] (memory-region)
      /e1000-mmio[0] (memory-region)
      /e1000.rom[0] (memory-region)
    /device[23] (piix3-ide)
      /bmdma[0] (memory-region)
      /bmdma[1] (memory-region)
      /bus master container[0] (memory-region)
      /bus master[0] (memory-region)
      /ide.0 (IDE)
      /ide.1 (IDE)
      /piix-bmdma-container[0] (memory-region)
      /piix-bmdma[0] (memory-region)
      /piix-bmdma[1] (memory-region)
    /device[24] (ide-hd)
    /device[25] (ide-cd)
    /device[26] (PIIX4_PM)
      /acpi-cnt[0] (memory-region)
      /acpi-cpu-hotplug[0] (memory-region)
      /acpi-cpu-hotplug[1] (memory-region)
      /acpi-evt[0] (memory-region)
      /acpi-gpe0[0] (memory-region)
      /acpi-pci-hotplug[0] (memory-region)
      /acpi-tmr[0] (memory-region)
      /apm-io[0] (memory-region)
      /bus master container[0] (memory-region)
      /bus master[0] (memory-region)
      /i2c (i2c-bus)
      /piix4-pm[0] (memory-region)
      /pm-smbus[0] (memory-region)
    /device[27] (smbus-eeprom)
    /device[28] (smbus-eeprom)
    /device[29] (smbus-eeprom)
    /device[2] (kvmclock)
    /device[30] (smbus-eeprom)
    /device[31] (smbus-eeprom)
    /device[32] (smbus-eeprom)
    /device[33] (smbus-eeprom)
    /device[34] (smbus-eeprom)
    /device[3] (i440FX)
      /bus master container[0] (memory-region)
      /bus master[0] (memory-region)
      /smram-low[0] (memory-region)
      /smram-region[0] (memory-region)
      /smram[0] (memory-region)
    /device[4] (PIIX3)
      /bus master container[0] (memory-region)
      /bus master[0] (memory-region)
      /isa.0 (ISA)
      /piix3-reset-control[0] (memory-region)
    /device[5] (kvm-i8259)
      /kvm-elcr[0] (memory-region)
      /kvm-pic[0] (memory-region)
    /device[6] (kvm-i8259)
      /kvm-elcr[0] (memory-region)
      /kvm-pic[0] (memory-region)
    /device[7] (virtio-vga)
      /bochs dispi interface[0] (memory-region)
      /bus master container[0] (memory-region)
      /bus master[0] (memory-region)
      /msix-pba[0] (memory-region)
      /msix-table[0] (memory-region)
      /qemu extended regs[0] (memory-region)
      /vbe[0] (memory-region)
      /vga ioports remapped[0] (memory-region)
      /vga-lowmem[0] (memory-region)
      /vga.vram[0] (memory-region)
      /vga[0] (memory-region)
      /vga[1] (memory-region)
      /vga[2] (memory-region)
      /vga[3] (memory-region)
      /vga[4] (memory-region)
      /virtio-backend (virtio-gpu-device)
      /virtio-bus (virtio-pci-bus)
      /virtio-pci-common-virtio-gpu[0] (memory-region)
      /virtio-pci-device-virtio-gpu[0] (memory-region)
      /virtio-pci-isr-virtio-gpu[0] (memory-region)
      /virtio-pci-notify-pio-virtio-gpu[0] (memory-region)
      /virtio-pci-notify-virtio-gpu[0] (memory-region)
      /virtio-pci[0] (memory-region)
      /virtio-vga-msix[0] (memory-region)
      /virtio-vga.rom[0] (memory-region)
    /device[8] (hpet)
      /hpet[0] (memory-region)
      /unnamed-gpio-in[0] (irq)
      /unnamed-gpio-in[1] (irq)
    /device[9] (mc146818rtc)
      /rtc-index[0] (memory-region)
      /rtc[0] (memory-region)
    /ide[0] (memory-region)
    /ide[1] (memory-region)
    /ide[2] (memory-region)
    /ide[3] (memory-region)
    /io[0] (memory-region)
    /ioport80[0] (memory-region)
    /ioportF0[0] (memory-region)
    /isa-bios[0] (memory-region)
    /non-qdev-gpio[0] (irq)
    /non-qdev-gpio[10] (irq)
    /non-qdev-gpio[11] (irq)
    /non-qdev-gpio[12] (irq)
    /non-qdev-gpio[13] (irq)
    /non-qdev-gpio[14] (irq)
    /non-qdev-gpio[15] (irq)
    /non-qdev-gpio[16] (irq)
    /non-qdev-gpio[17] (irq)
    /non-qdev-gpio[18] (irq)
    /non-qdev-gpio[19] (irq)
    /non-qdev-gpio[1] (irq)
    /non-qdev-gpio[20] (irq)
    /non-qdev-gpio[21] (irq)
    /non-qdev-gpio[22] (irq)
    /non-qdev-gpio[23] (irq)
    /non-qdev-gpio[24] (irq)
    /non-qdev-gpio[25] (irq)
    /non-qdev-gpio[2] (irq)
    /non-qdev-gpio[3] (irq)
    /non-qdev-gpio[4] (irq)
    /non-qdev-gpio[5] (irq)
    /non-qdev-gpio[6] (irq)
    /non-qdev-gpio[7] (irq)
    /non-qdev-gpio[8] (irq)
    /non-qdev-gpio[9] (irq)
    /pc.bios[0] (memory-region)
    /pc.rom[0] (memory-region)
    /pci[0] (memory-region)
    /ram-above-4g[0] (memory-region)
    /ram-below-4g[0] (memory-region)
    /sysbus (System)
    /system[0] (memory-region)
(qemu)
```

