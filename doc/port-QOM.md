# QEMU QOM
在 QOM 的定位中间，一个 TypeImpl 其实表示一个类，其静态部分在 ObjectClass 定义，非静态部分通过 Object 定义

关于 QOM 的进一步参考
- [ ] http://juniorprincewang.github.io/categories/QEMU/
- [ ] https://qemu.readthedocs.io/en/latest/devel/qom.html
- [ ] https://www.linux-kvm.org/images/9/90/Kvmforum14-qom.pdf

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

```plain
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

- 一个 TypeImpl::class 和自己 parent TypeImpl 关联的 ObjectClass 是同一个
  - 不是的，
  - 一个 TypeImpl 和其 parent TypeImpl 初始化的对象持有的 ObjectClass 现在是部分
  - 换言之，不同的类持有静态部分必然存储在两个位置

#### TypeInfo::instance_size 和 TypeInfo::class_size 作用
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

#### instance 如何初始化 
通过 object_new 初始化的

- x86_cpu_new
  - object_new
    - object_new_with_type
      - type_get_by_name : 参数为 host-x86_64-cpu ，因为提供了参数 -cpu host，具体的初始化在 qemu_init 中
      - [ ] object_new_with_type
        - 分配空间 TypeImpl::instance_size
        - object_initialize_with_type
          - object_init_with_type : 这个函数会首先初始化 parent 类型，然后进行本地初始化
  - qdev_realize : 最后调用 x86_cpu_realizefn


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


#### [ ] InterfaceClass
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
        .clas s_init = object_class_init,
        .abstract = true,
    };
```

```c
static const TypeInfo conventional_pci_interface_info = {
    .name          = INTERFACE_CONVENTIONAL_PCI_DEVICE,
    .parent        = TYPE_INTERFACE,
};
```

## DeviceRealize
关于 instance_init 和 DeviceRealize 的关系就是根本没有什么关系,
因为 DeviceRealize 就是为了给设备各种初始化进行抽象的，
如果非要说有关系: 所有的注册的 realize 都是在 device_set_realized 调用的，而 device_set_realized 是靠 property 机制进行的


## object_property
- [ ] 无法理解 property parent 强于 child, 是 class 强于 object 的
  - [ ] object_property_add_child (出现的位置: fw_cfg_init_io_dma)
- [ ] 似乎 property 比想想的更加有趣，实际上，这个 property 关联上  parent_obj.dma_enabled 了

```c
static Property fw_cfg_io_properties[] = {
    DEFINE_PROP_BOOL("dma_enabled", FWCfgIoState, parent_obj.dma_enabled,
                     true),
    DEFINE_PROP_UINT16("x-file-slots", FWCfgIoState, parent_obj.file_slots,
                       FW_CFG_FILE_SLOTS_DFLT),
    DEFINE_PROP_END_OF_LIST(),
};

qdev_prop_set_bit(dev, "dma_enabled", false);
```

- [x] 为什么需要定义出来 property 出来
  - 为了动态的添加属性, 比如 create_default_memdev 中，根据参数 mem_path 可以动态的选择到底是创建 TYPE_MEMORY_BACKEND_FILE 还是 TYPE_MEMORY_BACKEND_RAM 对象，以及是否添加 "mem-path" 属性
  - [ ] 也许还存在其他的原因


- property 数组, 比如 i440fx_props, 并没有什么神奇的，添加的时候循环添加进去即可

- machine_class_init
  - object_class_property_add_bool
    - object_class_property_add : 初始化一个 ObjectProperty
      - g_hash_table_insert : 将构造的插入到 ObjectClass::properties 中

- x86_cpu_new
  - object_new : 最后调用 instance_init
  - qdev_realize
    - object_property_set_bool
      - object_property_set_qobject
        - object_property_set
          - property_set_bool
            - device_set_realized : 这个函数在被 device_class_init 的位置被注册上，通过 object_class_property_add_bool

- x86_cpu_initfn
  - object_property_add : 调用位置应该只是在 instance_init 中
    - object_property_try_add : 进行参数格式检查以及重复的，然后
      - `g_hash_table_insert(obj->properties, prop->name, prop);`

实际上，有点操蛋的事情是 visiter , 其作用主要是为了解析各种 property 而已

- qdev_realize
  - qdev_set_parent_bus : bus 和 dev 的关系确定
  - object_property_set_bool(OBJECT(dev), "realized", true, errp);
    - object_property_set_qobject
      - qobject_input_visitor_new : 将 Qbool 作为参数，创建 Visitor
      - object_property_set
         - object_property_find_err
            - object_property_find 
                - object_get_class
                - object_class_property_find : 从上向下 property
                - g_hash_table_lookup(obj->properties, name) : 首先查找完成 parent 然后查找 child 的部分


```c
static Property x86_cpu_properties[] = {
    DEFINE_PROP_BOOL("tcg-cpuid", X86CPU, expose_tcg, true),
}

GlobalProperty pc_compat_2_8[] = {
    { TYPE_X86_CPU, "tcg-cpuid", "off" },
    { "kvmclock", "x-mach-use-reliable-get-clock", "off" },
    { "ICH9-LPC", "x-smi-broadcast", "off" },
    { TYPE_X86_CPU, "vmware-cpuid-freq", "off" },
    { "Haswell-" TYPE_X86_CPU, "stepping", "1" },
};
```
```diff
History:        #0
Commit:         1ce36bfe6424243082d3d7c2330e1a0a4ff72a43
Author:         Daniel P. Berrangé <berrange@redhat.com>
Committer:      Eduardo Habkost <ehabkost@redhat.com>
Author Date:    Tue 09 May 2017 09:27:36 PM CST
Committer Date: Tue 18 Jul 2017 02:41:30 AM CST

i386: expose "TCGTCGTCGTCG" in the 0x40000000 CPUID leaf

Currently when running KVM, we expose "KVMKVMKVM\0\0\0" in
the 0x40000000 CPUID leaf. Other hypervisors (VMWare,
HyperV, Xen, BHyve) all do the same thing, which leaves
TCG as the odd one out.

The CPUID signature is used by software to detect which
virtual environment they are running in and (potentially)
change behaviour in certain ways. For example, systemd
supports a ConditionVirtualization= setting in unit files.
The virt-what command can also report the virt type it is
running on

Currently both these apps have to resort to custom hacks
like looking for 'fw-cfg' entry in the /proc/device-tree
file to identify TCG.

This change thus proposes a signature "TCGTCGTCGTCG" to be
reported when running under TCG.

To hide this, the -cpu option tcg-cpuid=off can be used.
```
                
#### object_property_add_alias
- [ ] 干啥用的啊

#### 

## 附录: 宏展开
```c
#define OBJECT(obj) \
    ((Object *)(obj))

#define OBJECT_CLASS(class) \
    ((ObjectClass *)(class))

#define OBJECT_CHECK(type, obj, name) \
    ((type *)object_dynamic_cast_assert(OBJECT(obj), (name), \
                                        __FILE__, __LINE__, __func__))

#define OBJECT_CLASS_CHECK(class_type, class, name) \
    ((class_type *)object_class_dynamic_cast_assert(OBJECT_CLASS(class), (name), \
                                               __FILE__, __LINE__, __func__))

#define OBJECT_GET_CLASS(class, obj, name) \
    OBJECT_CLASS_CHECK(class, object_get_class(OBJECT(obj)), name)

#define DECLARE_CLASS_CHECKERS(ClassType, OBJ_NAME, TYPENAME) \
    static inline G_GNUC_UNUSED ClassType * \
    OBJ_NAME##_GET_CLASS(const void *obj) \
    { return OBJECT_GET_CLASS(ClassType, obj, TYPENAME); } \
    \
    static inline G_GNUC_UNUSED ClassType * \
    OBJ_NAME##_CLASS(const void *klass) \
    { return OBJECT_CLASS_CHECK(ClassType, klass, TYPENAME); }

#define DECLARE_INSTANCE_CHECKER(InstanceType, OBJ_NAME, TYPENAME) \
    static inline G_GNUC_UNUSED InstanceType * \
    OBJ_NAME(const void *obj) \
    { return OBJECT_CHECK(InstanceType, obj, TYPENAME); }


#define DECLARE_OBJ_CHECKERS(InstanceType, ClassType, OBJ_NAME, TYPENAME) \
    DECLARE_INSTANCE_CHECKER(InstanceType, OBJ_NAME, TYPENAME) \
    \
    DECLARE_CLASS_CHECKERS(ClassType, OBJ_NAME, TYPENAME)

#define OBJECT_DECLARE_TYPE(InstanceType, ClassType, MODULE_OBJ_NAME) \
    typedef struct InstanceType InstanceType; \
    typedef struct ClassType ClassType; \
    \
    G_DEFINE_AUTOPTR_CLEANUP_FUNC(InstanceType, object_unref) \
    \
    DECLARE_OBJ_CHECKERS(InstanceType, ClassType, \
                         MODULE_OBJ_NAME, TYPE_##MODULE_OBJ_NAME)

/**
 * OBJECT_DECLARE_SIMPLE_TYPE:
 * @InstanceType: instance struct name
 * @MODULE_OBJ_NAME: the object name in uppercase with underscore separators
 *
 * This does the same as OBJECT_DECLARE_TYPE(), but with no class struct
 * declared.
 *
 * This macro should be used unless the class struct needs to have
 * virtual methods declared.
 */
#define OBJECT_DECLARE_SIMPLE_TYPE(InstanceType, MODULE_OBJ_NAME) \
    typedef struct InstanceType InstanceType; \
    \
    G_DEFINE_AUTOPTR_CLEANUP_FUNC(InstanceType, object_unref) \
    \
    DECLARE_INSTANCE_CHECKER(InstanceType, MODULE_OBJ_NAME, TYPE_##MODULE_OBJ_NAME)
```

#### OBJECT_DECLARE_SIMPLE_TYPE
simple type 只能是一个基本看上去只是将其中的 class 部分去掉

```c
#define TYPE_MC146818_RTC "mc146818rtc"
OBJECT_DECLARE_SIMPLE_TYPE(RTCState, MC146818_RTC)
```

```c
typedef struct RTCState RTCState;

G_DEFINE_AUTOPTR_CLEANUP_FUNC(RTCState, object_unref)

static inline G_GNUC_UNUSED RTCState *MC146818_RTC(const void *obj) {
  return ((RTCState *)object_dynamic_cast_assert(
      ((Object *)(obj)), ("mc146818rtc"), "a.c", 69, __func__));
}
```

