# QEMU QOM
在 QOM 的定位中间，一个 TypeImpl 其实表示一个类，其静态部分在 ObjectClass 定义，非静态部分通过 Object 定义

关于 QOM 的进一步参考
- [ ] http://juniorprincewang.github.io/categories/QEMU/
- [ ] https://qemu.readthedocs.io/en/latest/devel/qom.html
- [ ] https://www.linux-kvm.org/images/9/90/Kvmforum14-qom.pdf

- [x] alias
- [x] struct ObjectProperty 和 struct Property 的关系是什么?
	- 一个是动态的，一个是静态的定义

- [x] visiter : 我放弃了
	- [ ] visit core 只有 400 行
	-	[x] 无论如何，玩一下其中的 qapi
		- https://github.com/arcnmx/qapi-rs : 实际上我并不会写 rust
		- https://gist.github.com/rgl/dc38c6875a53469fdebb2e9c0a220c6c : 直接使用 nc 就可以了
	- [ ] parse_numa


- 和标准的面向对象的差别
	- 没有办法实现 override 和 overload 的操作
	- 但是搞了一个很复杂的 property
		- 构建 QOM tree
		- 构建 qtree
		- alias
		- link

- [x] init QEMU 中的 feature 最后是如何用起来的

## init

select_machine 需要获取所有的 TYPE_MACHINE 的 class, 这最后会调用到 type_initialize

所有 type 早就注册到此处了:

从 type_table_get 中可以获取所有的 type 的内容

type_initialize 中会分配 class 的空间的
```c
/*
#0  apic_common_class_init (klass=0x5555567cb060, data=0x0) at ../hw/intc/apic_common.c:468
#1  0x0000555555d23dcf in type_initialize (ti=0x5555566f7460) at ../qom/object.c:1077
#2  object_class_foreach_tramp (key=<optimized out>, value=0x5555566f7460, opaque=0x7fffffffd030) at ../qom/object.c:1077
#3  0x00007ffff79881b8 in g_hash_table_foreach () at /lib/x86_64-linux-gnu/libglib-2.0.so.0
#4  0x0000555555d2440c in object_class_foreach (fn=fn@entry=0x555555d22aa0 <object_class_get_list_tramp>, implements_type=implements_type@entry=0x555556258463 "machine", include_abstract=include_abstract@entry=false, opaque=opaque@entry=0x7fffffffd070) at ../qom/object.c:86
#5  0x0000555555d244b6 in object_class_get_list (implements_type=implements_type@entry=0x555556258463 "machine", include_abstract=include_abstract@entry=false) at ../qom/object.c:1156
#6  0x0000555555c659b7 in select_machine (errp=<optimized out>, qdict=0x555556705560) at ../softmmu/vl.c:1620
#7  qemu_create_machine (qdict=0x555556705560) at ../softmmu/vl.c:2105
#8  qemu_init (argc=<optimized out>, argv=0x7fffffffd2f8, envp=<optimized out>) at ../softmmu/vl.c:3639
#9  0x0000555555940c8d in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
```

通过调用 object_init_with_type 实现的，并且在启动会 malloc 空间存储这个新的 object 来的。
```c
/*
#0  apic_common_initfn (obj=0x5555569e5db0) at ../hw/intc/apic_common.c:458
#1  0x0000555555d23316 in object_init_with_type (obj=0x5555569e5db0, ti=0x5555566d4e20) at ../qom/object.c:372
#2  0x0000555555d2495c in object_initialize_with_type (obj=obj@entry=0x5555569e5db0, size=size@entry=656, type=type@entry=0x5555566d4e20) at ../qom/object.c:525
#3  0x0000555555d24aa9 in object_new_with_type (type=0x5555566d4e20) at ../qom/object.c:740
#4  0x0000555555b77fd5 in x86_cpu_apic_create (cpu=cpu@entry=0x555556b08d50, errp=errp@entry=0x7fffffffccc0) at ../target/i386/cpu-sysemu.c:274
#5  0x0000555555be255f in x86_cpu_realizefn (dev=0x555556b08d50, errp=0x7fffffffcd20) at ../target/i386/cpu.c:6267
#6  0x0000555555d3e027 in device_set_realized (obj=<optimized out>, value=true, errp=0x7fffffffcda0) at ../hw/core/qdev.c:761
#7  0x0000555555d22caa in property_set_bool (obj=0x555556b08d50, v=<optimized out>, name=<optimized out>, opaque=0x55555670c430, errp=0x7fffffffcda0) at ../qom/object.c:2285
#8  0x0000555555d251dc in object_property_set (obj=obj@entry=0x555556b08d50, name=name@entry=0x555555fe20d6 "realized", v=v@entry=0x555556aeabf0, errp=errp@entry=0x555556618678 <error_fatal>) at ../qom/object.c:1410
#9  0x0000555555d21824 in object_property_set_qobject (obj=obj@entry=0x555556b08d50, name=name@entry=0x555555fe20d6 "realized", value=value@entry=0x5555569f30a0, errp=errp@entry=0x555556618678 <error_fatal>) at ../qom/qom-qobject.c:28
#10 0x0000555555d25449 in object_property_set_bool (obj=0x555556b08d50, name=name@entry=0x555555fe20d6 "realized", value=value@entry=true, errp=errp@entry=0x555556618678 <error_fatal>) at ../qom/object.c:1480
#11 0x0000555555d3ce52 in qdev_realize (dev=<optimized out>, bus=bus@entry=0x0, errp=errp@entry=0x555556618678 <error_fatal>) at ../hw/core/qdev.c:389
#12 0x0000555555badf75 in x86_cpu_new (x86ms=x86ms@entry=0x55555677cde0, apic_id=0, errp=errp@entry=0x555556618678 <error_fatal>) at /home/maritns3/core/kvmqemu/include/hw/qdev-core.h:17
#13 0x0000555555bae05e in x86_cpus_init (x86ms=x86ms@entry=0x55555677cde0, default_cpu_version=<optimized out>) at ../hw/i386/x86.c:138
#14 0x0000555555b8aaf3 in pc_init1 (machine=0x55555677cde0, pci_type=0x555555f5d125 "i440FX", host_type=0x555555ec0aed "i440FX-pcihost") at ../hw/i386/pc_piix.c:156
#15 0x0000555555a6c094 in machine_run_board_init (machine=0x55555677cde0) at ../hw/core/machine.c:1273
#16 0x0000555555c64ec4 in qemu_init_board () at ../softmmu/vl.c:2615
#17 qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2689
#18 qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2682
#19 0x0000555555c68668 in qemu_init (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/vl.c:3706
#20 0x0000555555940c8d in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
```

device_class_init 中注册了
```c
object_class_property_add_bool(class, "realized", device_get_realized, device_set_realized);
```
靠 qdev_realize 就可以了

```c
/*
#0  apic_common_realize (dev=0x5555569e5db0, errp=0x7fffffffcb70) at ../hw/intc/apic_common.c:286
#1  0x0000555555d3e027 in device_set_realized (obj=<optimized out>, value=true, errp=0x7fffffffcbf0) at ../hw/core/qdev.c:761
#2  0x0000555555d22caa in property_set_bool (obj=0x5555569e5db0, v=<optimized out>, name=<optimized out>, opaque=0x55555670c430, errp=0x7fffffffcbf0) at ../qom/object.c
:2285
#3  0x0000555555d251dc in object_property_set (obj=obj@entry=0x5555569e5db0, name=name@entry=0x555555fe20d6 "realized", v=v@entry=0x555556a2c940, errp=errp@entry=0x7fff
ffffccc0) at ../qom/object.c:1410
#4  0x0000555555d21824 in object_property_set_qobject (obj=obj@entry=0x5555569e5db0, name=name@entry=0x555555fe20d6 "realized", value=value@entry=0x555556c0d9b0, errp=e
rrp@entry=0x7fffffffccc0) at ../qom/qom-qobject.c:28
#5  0x0000555555d25449 in object_property_set_bool (obj=0x5555569e5db0, name=name@entry=0x555555fe20d6 "realized", value=value@entry=true, errp=errp@entry=0x7fffffffccc
0) at ../qom/object.c:1480
#6  0x0000555555d3ce52 in qdev_realize (dev=<optimized out>, bus=bus@entry=0x0, errp=errp@entry=0x7fffffffccc0) at ../hw/core/qdev.c:389
#7  0x0000555555b780b9 in x86_cpu_apic_realize (cpu=cpu@entry=0x555556b08d50, errp=errp@entry=0x7fffffffccc0) at /home/maritns3/core/kvmqemu/include/hw/qdev-core.h:17
#8  0x0000555555be2653 in x86_cpu_realizefn (dev=0x555556b08d50, errp=0x7fffffffcd20) at ../target/i386/cpu.c:6299
#9  0x0000555555d3e027 in device_set_realized (obj=<optimized out>, value=true, errp=0x7fffffffcda0) at ../hw/core/qdev.c:761
#10 0x0000555555d22caa in property_set_bool (obj=0x555556b08d50, v=<optimized out>, name=<optimized out>, opaque=0x55555670c430, errp=0x7fffffffcda0) at ../qom/object.c
:2285
#11 0x0000555555d251dc in object_property_set (obj=obj@entry=0x555556b08d50, name=name@entry=0x555555fe20d6 "realized", v=v@entry=0x555556aeabf0, errp=errp@entry=0x5555
56618678 <error_fatal>) at ../qom/object.c:1410
#12 0x0000555555d21824 in object_property_set_qobject (obj=obj@entry=0x555556b08d50, name=name@entry=0x555555fe20d6 "realized", value=value@entry=0x5555569f30a0, errp=e
rrp@entry=0x555556618678 <error_fatal>) at ../qom/qom-qobject.c:28
#13 0x0000555555d25449 in object_property_set_bool (obj=0x555556b08d50, name=name@entry=0x555555fe20d6 "realized", value=value@entry=true, errp=errp@entry=0x55555661867
8 <error_fatal>) at ../qom/object.c:1480
#14 0x0000555555d3ce52 in qdev_realize (dev=<optimized out>, bus=bus@entry=0x0, errp=errp@entry=0x555556618678 <error_fatal>) at ../hw/core/qdev.c:389
#15 0x0000555555badf75 in x86_cpu_new (x86ms=x86ms@entry=0x55555677cde0, apic_id=0, errp=errp@entry=0x555556618678 <error_fatal>) at /home/maritns3/core/kvmqemu/include
/hw/qdev-core.h:17
#16 0x0000555555bae05e in x86_cpus_init (x86ms=x86ms@entry=0x55555677cde0, default_cpu_version=<optimized out>) at ../hw/i386/x86.c:138
#17 0x0000555555b8aaf3 in pc_init1 (machine=0x55555677cde0, pci_type=0x555555f5d125 "i440FX", host_type=0x555555ec0aed "i440FX-pcihost") at ../hw/i386/pc_piix.c:156
#18 0x0000555555a6c094 in machine_run_board_init (machine=0x55555677cde0) at ../hw/core/machine.c:1273
#19 0x0000555555c64ec4 in qemu_init_board () at ../softmmu/vl.c:2615
#20 qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2689
#21 qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2682
#22 0x0000555555c68668 in qemu_init (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/vl.c:3706
#23 0x0000555555940c8d in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
```
注意 : class_init 和 instance_init 都是可以自动调用 parent 的 hook 的，但是 realize 不可以的

[听君一席话，如听一席话](http://people.redhat.com/~thuth/blog/qemu/2018/09/10/instance-init-realize.html)

关于 instance_init 和 DeviceRealize 的关系就是根本没有什么关系,
因为 DeviceRealize 就是为了给设备各种初始化进行抽象的，
如果非要说有关系: 所有的注册的 realize 都是在 device_set_realized 调用的，而 device_set_realized 是靠 property 机制进行的

## qdev

- [ ] 将整个 qdev.c 的每个函数分析一下
```c
struct DeviceClass {
    /*< private >*/
    ObjectClass parent_class;
    /*< public >*/

    DECLARE_BITMAP(categories, DEVICE_CATEGORY_MAX);
    const char *fw_name;
    const char *desc;

    /*
     * The underscore at the end ensures a compile-time error if someone
     * assigns to dc->props instead of using device_class_set_props.
     */
    Property *props_;

    /*
     * Can this device be instantiated with -device / device_add?
     * All devices should support instantiation with device_add, and
     * this flag should not exist.  But we're not there, yet.  Some
     * devices fail to instantiate with cryptic error messages.
     * Others instantiate, but don't work.  Exposing users to such
     * behavior would be cruel; clearing this flag will protect them.
     * It should never be cleared without a comment explaining why it
     * is cleared.
     * TODO remove once we're there
     */
    bool user_creatable;
    bool hotpluggable;

    /* callbacks */
    /*
     * Reset method here is deprecated and replaced by methods in the
     * resettable class interface to implement a multi-phase reset.
     * TODO: remove once every reset callback is unused
     */
    DeviceReset reset;
    DeviceRealize realize;
    DeviceUnrealize unrealize;

    /* device state */
    const VMStateDescription *vmsd;

    /* Private to qdev / bus.  */
    const char *bus_type;
};
```
实际上多出来的主要内容:
- [ ] `props_`
- [ ] reset / realize / unrealize


## memory region
才意识到 memory region 也是一个 qobject 并且是需要挂到

```c
void memory_region_init(MemoryRegion *mr,
                        Object *owner,
                        const char *name,
                        uint64_t size)
{
    object_initialize(mr, sizeof(*mr), TYPE_MEMORY_REGION);
    memory_region_do_init(mr, owner, name, size);
}
```
qom tree 实际上，不是一个继承关系，比如这样的:

```txt
  /i440fx (i440FX-pcihost)
    /ioapic (kvm-ioapic)
      /kvm-ioapic[0] (memory-region)
```

## hmp_info_qom_tree
```c
static void print_qom_composition(Monitor *mon, Object *obj, int indent)
{
    GArray *children = g_array_new(false, false, sizeof(Object *));
    const char *name;
    int i;

    if (obj == object_get_root()) {
        name = "";
    } else {
        name = object_get_canonical_path_component(obj);
    }
    monitor_printf(mon, "%*s/%s (%s)\n", indent, "", name, // 可以明白了，就是一个是名称 qom tree 的路径，一个是 type 名称
                   object_get_typename(obj));

    object_child_foreach(obj, insert_qom_composition_child, children);
    g_array_sort(children, qom_composition_compare);

    for (i = 0; i < children->len; i++) {
        print_qom_composition(mon, g_array_index(children, Object *, i), // 递归下去
                              indent + 2);
    }
    g_array_free(children, TRUE);
}
```

- [x] 这个东西是如何构建出来的? /device[19] 这个名称的构建
```plain
    /device[15] (isa-serial)
      /serial (serial)
      /serial[0] (memory-region)
```

在 device_set_realized 中，如果 Object::parent == NULL, 那么会在
/unattached (container) 中添加增加一个 child 属性为 device[unattached_count]

```c
if (!obj->parent) {
    gchar *name = g_strdup_printf("device gg[%d]", unattached_count++);

    object_property_add_child(container_get(qdev_get_machine(),
                                            "/unattached"),
                              name, obj);
    unattached_parent = true;
    g_free(name);
}
```

isa-serial 在继承上的关系有多个，但是 Object::parent 的赋值是在
object_property_add_child => object_property_try_add_child 中，也即是 parent 还是这个路线上的。

```c
static const TypeInfo serial_isa_info
static const TypeInfo isa_device_type_info
static const TypeInfo device_type_info
```



## hmp_info_qtree
```c
void hmp_info_qtree(Monitor *mon, const QDict *qdict)
{
    if (sysbus_get_default())
        qbus_print(mon, sysbus_get_default(), 0);
}
```

- [ ] 我的龟龟哇 : `static BusState *main_system_bus;`

```c
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
    BusState *parent_bus;                  // 关联了 parent bus 哦
    QLIST_HEAD(, NamedGPIOList) gpios;
    QLIST_HEAD(, NamedClockList) clocks;
    QLIST_HEAD(, BusState) child_bus;
    int num_child_bus;
    int instance_id_alias;
    int alias_required_for_version;
    ResettableState reset;
};
```
- dev 和 bus 是互相交错放置的
  - 在 qbus_init 中间，创建的 bus 的时候，使用 BusState::sibling 将 BusState 挂到 DeviceState::child_bus 上
  - 在 bus_add_child 中，使用 DeviceState::sibling 将 DeviceState 挂到 BusState::children 上

```c
static void qdev_print(Monitor *mon, DeviceState *dev, int indent){
    // ...
    do {
        qdev_print_props(mon, dev, DEVICE_CLASS(class)->props_, indent);
        class = object_class_get_parent(class);
    } while (class != object_class_by_name(TYPE_DEVICE));
    // ...
}
```

```txt
dev: fw_cfg_io, id ""
  dma_enabled = true
  x-file-slots = 32 (0x20)
  acpi-mr-restore = true
```

```c
static Property fw_cfg_mem_properties[] = {
    DEFINE_PROP_UINT32("data_width", FWCfgMemState, data_width, -1),
    DEFINE_PROP_BOOL("dma_enabled", FWCfgMemState, parent_obj.dma_enabled,
                     true),
    DEFINE_PROP_UINT16("x-file-slots", FWCfgMemState, parent_obj.file_slots,
                       FW_CFG_FILE_SLOTS_DFLT),
    DEFINE_PROP_END_OF_LIST(),
};
```
关联到结构体的成员上的

- qdev_print_props : 输出属性
- device_class_set_props : 设置属性


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

现在 qemu_init 会根据参数调整 cpu_type 的:
```c
    /* parse features once if machine provides default cpu_type */
    current_machine->cpu_type = machine_class->default_cpu_type;
    if (cpu_option) {
        current_machine->cpu_type = parse_cpu_option(cpu_option);
    }
```

然后在 x86_cpu_new 中初始化的

```c
void x86_cpu_new(X86MachineState *x86ms, int64_t apic_id, Error **errp)
{
    Object *cpu = object_new(MACHINE(x86ms)->cpu_type);

    if (!object_property_set_uint(cpu, "apic-id", apic_id, errp)) {
        goto out;
    }
    qdev_realize(DEVICE(cpu), NULL, errp);

out:
    object_unref(cpu);
}
```

而 `machine_class->default_cpu_type` 的初始化为 pc_machine_class_init, 将默认数值初始化为 TARGET_DEFAULT_CPU_TYPE
```c
#ifdef TARGET_X86_64
#define TARGET_DEFAULT_CPU_TYPE X86_CPU_TYPE_NAME("qemu64")
#else
#define TARGET_DEFAULT_CPU_TYPE X86_CPU_TYPE_NAME("qemu32")
#endif
```


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
- 在 86_cpu_initfn 中，多次的调用了 object_property_add_alias 只是为了修改名称
  - 这个想法在 x86_cpu_register_feature_bit_props  x86_cpu_register_feature_bit_props 中得到验证

object_property_add_alias 只是在调用 object_property_add 的时候，其 hook 为
- property_get_alias
- property_set_alias
这和 object_property_add_str 是没有什么区别的。


比如在 x86_cpu_initfn 中间的操作:
- [x] 第一种使用方法，多个名称指向同一个属性，从而实现

```c
    object_property_add_alias(obj, "sse3", obj, "pni", &error_abort);
    object_property_add_alias(obj, "pclmuldq", obj, "pclmulqdq", &error_abort);
    object_property_add_alias(obj, "sse4-1", obj, "sse4.1", &error_abort);
    object_property_add_alias(obj, "sse4-2", obj, "sse4.2", &error_abort);
```
object_property_add_alias

- [x] 第二种，pc_machine_initfn，保证两个属性的相同的
```c
object_property_add_alias(OBJECT(pcms), "pcspk-audiodev", OBJECT(pcms->pcspk), "audiodev");
```

#### GlobalProperty
```c
/**
 * GlobalProperty:
 * @used: Set to true if property was used when initializing a device.
 * @optional: If set to true, GlobalProperty will be skipped without errors
 *            if the property doesn't exist.
 *
 * An error is fatal for non-hotplugged devices, when the global is applied.
 */
typedef struct GlobalProperty {
    const char *driver;
    const char *property;
    const char *value;
    bool used;
    bool optional;
} GlobalProperty;
```

```c
QemuOptsList qemu_global_opts = {
    .name = "global",
    .head = QTAILQ_HEAD_INITIALIZER(qemu_global_opts.head),
    .desc = {
        {
            .name = "driver",
            .type = QEMU_OPT_STRING,
        },{
            .name = "property",
            .type = QEMU_OPT_STRING,
        },{
            .name = "value",
            .type = QEMU_OPT_STRING,
        },
        { /* end of list */ }
    },
};
```

通过 Man qemu-system(1) 中找到的:
```txt
-global driver.prop=value
-global driver=driver,property=property,value=value
   Set default value of driver's property prop to value, e.g.:

           qemu-system-x86_64 -global ide-hd.physical_block_size=4096 disk-image.img

   In particular, you can use this to set driver properties for devices which are created automatically by the
   machine model. To create a device which is not created automatically and set properties on it, use -device.

   -global driver.prop=value is shorthand for -global driver=driver,property=prop,value=value.  The longhand
   syntax works even when driver contains a dot.
```
- [ ] 这个东西是和 object 没有关系的 priority 吗?


在 x86_cpu_parse_featurestr 中，实际上:
```c
/* Parse "+feature,-feature,feature=foo" CPU feature string
 */
static void x86_cpu_parse_featurestr(const char *typename, char *features,
                                     Error **errp)
```
构建的 GlobalProperty 通过 qdev_prop_register_global 添加到 global_props 上，其返回一个 staic 数组


```c
bool object_apply_global_props(Object *obj, const GPtrArray *props,
                               Error **errp)
{
    for (i = 0; i < props->len; i++) {
        GlobalProperty *p = g_ptr_array_index(props, i);

        if (object_dynamic_cast(obj, p->driver) == NULL) {
            continue;
        }
        if (p->optional && !object_property_find(obj, p->property)) {
            continue;
        }
    }

    return true;
}
```
主要的两个调用位置:
- device_post_init
- do_configure_accelerator


## object_property_add_uint32_ptr
- object_property_get_uint
	- object_property_get_qobject

```c
/*
#0  property_get_uint32_ptr (obj=0x555557b5f850, v=0x555556cd0e00, name=0x555555e98ccf "pm_io_base", opaque=0x555557b602b0, errp=0x7fffffffcca0) at ../qom/object.c:2480
#1  0x0000555555d250cc in object_property_get (obj=obj@entry=0x555557b5f850, name=name@entry=0x555555e98ccf "pm_io_base", v=v@entry=0x555556cd0e00, errp=errp@entry=0x0)
 at ../qom/object.c:1384
#2  0x0000555555d21891 in object_property_get_qobject (obj=0x555557b5f850, name=0x555555e98ccf "pm_io_base", errp=0x0) at ../qom/qom-qobject.c:40
#3  0x0000555555d258a7 in object_property_get_uint (obj=obj@entry=0x555557b5f850, name=name@entry=0x555555e98ccf "pm_io_base", errp=errp@entry=0x0) at ../qom/object.c:1
584
#4  0x0000555555ba0c68 in init_common_fadt_data (data=0x7fffffffcef8, o=0x555557b5f850, ms=0x555556811800) at ../hw/i386/acpi-build.c:151
#5  acpi_get_pm_info (machine=machine@entry=0x555556811800, pm=pm@entry=0x7fffffffcef0) at ../hw/i386/acpi-build.c:217
#6  0x0000555555ba1b4f in acpi_build (tables=tables@entry=0x7fffffffcfd0, machine=0x555556811800) at ../hw/i386/acpi-build.c:2449
#7  0x0000555555ba4d0e in acpi_setup () at /home/maritns3/core/kvmqemu/include/hw/boards.h:24
#8  0x0000555555b9afff in pc_machine_done (notifier=0x555556811998, data=<optimized out>) at ../hw/i386/pc.c:783
#9  0x0000555555e89bd7 in notifier_list_notify (list=list@entry=0x5555565f55a8 <machine_init_done_notifiers>, data=data@entry=0x0) at ../util/notify.c:39
#10 0x0000555555a6c82b in qdev_machine_creation_done () at ../hw/core/machine.c:1321
#11 0x0000555555c64fa0 in qemu_machine_creation_done () at ../softmmu/vl.c:2668
#12 qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2691
#13 qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2682
#14 0x0000555555c68668 in qemu_init (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/vl.c:3706
#15 0x0000555555940c8d in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
```

#### QObject
```c
/* Not for use outside include/qapi/qmp/ */
struct QObjectBase_ {
    QType type;
    size_t refcnt;
};

/* this struct must have no other members than base */
struct QObject {
    struct QObjectBase_ base;
};

#define QOBJECT(obj) ({                                         \
    typeof(obj) _obj = (obj);                                   \
    _obj ? container_of(&(_obj)->base, QObject, base) : NULL;   \
})
```

QObject 是和这些属性是放到一起的:
```c
typedef struct QListEntry {
    QObject *value;
    QTAILQ_ENTRY(QListEntry) next;
} QListEntry;

struct QList {
    struct QObjectBase_ base;
    QTAILQ_HEAD(,QListEntry) head;
};
```


#### object_property_add_child
- x86_cpu_apic_create
- ioapic_init_gsi


- [ ] 在使用这个例子来分析一下

```c
Object *qdev_get_machine(void)
{
    static Object *dev;

    if (dev == NULL) {
        dev = container_get(object_get_root(), "/machine");
    }

    return dev;
}
```

应该是在 qemu_create_machine 中使用，进行添加的
```c
object_property_add_child(object_get_root(), "machine", OBJECT(current_machine));
```

- [ ] 居然还有 object_resolve_path 的操作
```c
object_property_add_child(object_resolve_path(parent_name, NULL), "ioapic", OBJECT(dev));
```

#### link

第一个例子: vmport 和 i8042 之间的 link : object_property_set_link
```txt
huxueshi:object_resolve_link i8042 /machine/unattached/device[19]
huxueshi:object_resolve_link a20[0] /machine/unattached/non-qdev-gpio[24]
huxueshi:object_resolve_link a20[0] /machine/unattached/non-qdev-gpio[25]
```

```c
/*
#1  0x0000555555d26d35 in object_resolve_link (errp=0x7fffffffcda0, path=0x555556f74730 "/machine/unattached/device[19]", name=0x555555f1ea10 "i8042", obj=0x555556b27b50) at ../qom/object.c:1832
#2  object_set_link_property (obj=0x555556b27b50, v=<optimized out>, name=<optimized out>, opaque=0x5555568bd2d0, errp=0x7fffffffcda0) at ../qom/object.c:1874
#3  0x0000555555d2517c in object_property_set (obj=obj@entry=0x555556b27b50, name=name@entry=0x555555f1ea10 "i8042", v=v@entry=0x555556d0af70, errp=errp@entry=0x555556618680 <error_abort>) at ../qom/object.c:1403
#4  0x0000555555d217c4 in object_property_set_qobject (obj=0x555556b27b50, name=0x555555f1ea10 "i8042", value=<optimized out>, errp=0x555556618680 <error_abort>) at ../qom/qom-qobject.c:28
#5  0x0000555555d25269 in object_property_set_str (obj=0x555556b27b50, name=0x555555f1ea10 "i8042", value=<optimized out>, errp=0x555556618680 <error_abort>) at ../qom/object.c:1412
#6  0x0000555555d265d8 in object_property_set_link (obj=obj@entry=0x555556b27b50, name=name@entry=0x555555f1ea10 "i8042", value=value@entry=0x555556848900, errp=0x555556618680 <error_abort>) at ../qom/object.c:1448
#7  0x0000555555b9dbe0 in pc_superio_init (no_vmport=<optimized out>, create_fdctrl=<optimized out>, isa_bus=0x555556ad61d0) at ../hw/i386/pc.c:1081
#8  pc_basic_device_init (pcms=pcms@entry=0x555556811800, isa_bus=0x555556ad61d0, gsi=<optimized out>, rtc_state=rtc_state@entry=0x7fffffffcf38, create_fdctrl=create_fdctrl@entry=true, hpet_irqs=hpet_irqs@entry=4) at ../hw/i386/pc.c:1168
#9  0x0000555555b8ac0d in pc_init1 (machine=0x555556811800, pci_type=0x555555f5d125 "i440FX", host_type=0x555555ec0aed "i440FX-pcihost") at ../hw/i386/pc_piix.c:238
#10 0x0000555555a6c094 in machine_run_board_init (machine=0x555556811800) at ../hw/core/machine.c:1273
#11 0x0000555555c64e64 in qemu_init_board () at ../softmmu/vl.c:2615
#12 qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2689
#13 qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2682
#14 0x0000555555c68608 in qemu_init (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/vl.c:3706
#15 0x0000555555940c8d in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
```
第二个例子:
在 pc_init1 中，实际上，我看不懂为什么如此设计

```c
        object_property_add_link(OBJECT(machine), PC_MACHINE_ACPI_DEVICE_PROP,
                                 TYPE_HOTPLUG_HANDLER,
                                 (Object **)&x86ms->acpi_dev,
                                 object_property_allow_set_link,
                                 OBJ_PROP_LINK_STRONG);
        object_property_set_link(OBJECT(machine), PC_MACHINE_ACPI_DEVICE_PROP,
                                 OBJECT(piix4_pm), &error_abort);
```

第三个例子: bus_add_child

第四个: qdev_init_gpio_out_named

- pic_realize
  - `qdev_init_gpio_out(dev, s->int_out, ARRAY_SIZE(s->int_out));`
  - `qdev_init_gpio_in(dev, pic_set_irq, 8);`

```c
        object_property_add_link(OBJECT(dev), propname, TYPE_IRQ,
                                 (Object **)&pins[i],
                                 object_property_allow_set_link,
                                 OBJ_PROP_LINK_STRONG);
```
- qdev_init_gpio_out_named
  - object_property_add_link
    - object_add_link_prop
      - object_property_add : 这个调用就是普通的添加，但是其 get set resolve 对应的 hook 函数不同的

```c
object_property_set_link(OBJECT(dev), propname, OBJECT(pin), &error_abort);
```

- qdev_connect_gpio_out_named
  - object_property_set_link : 实际上，这就是一个简答的赋值操作
    - object_get_canonical_path : 不是通过继承构建的，而是通过 priority 构建的
    - object_property_set_str
      - object_property_set_qobject
        - object_property_set : 实际上就是简单的赋值了

#### ObjectProperty::type
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

- [ ] ObjectProperty::type
```c
static inline bool object_property_is_child(ObjectProperty *prop)
{
    printf("huxueshi:%s %s\n", __FUNCTION__, prop->type);
    return strstart(prop->type, "child<", NULL);
}

const char *object_get_canonical_path_component(const Object *obj)
{
    ObjectProperty *prop = NULL;
    GHashTableIter iter;

    if (obj->parent == NULL) {
        return NULL;
    }

    g_hash_table_iter_init(&iter, obj->parent->properties);
    while (g_hash_table_iter_next(&iter, NULL, (gpointer *)&prop)) {
        if (!object_property_is_child(prop)) {
            continue;
        }

        if (prop->opaque == obj) {
            return prop->name;
        }
    }

    /* obj had a parent but was not a child, should never happen */
    g_assert_not_reached();
    return NULL;
}
```
// 我的龟龟，难道这种层级结构体是通过 properties 构建起来的

```c
/*
#0  huxueshi () at ../qom/object.c:609
#1  0x0000555555d24a65 in object_property_try_add (obj=0x555556810800, name=0x555556709150 "peripheral", type=0x55555691fee0 "child<container>", get=0x555555d26250 <obj
ect_get_child_property>, set=0x0, release=0x555555d24390 <object_finalize_child_property>, opaque=0x55555691fe50, errp=0x555556617680 <error_abort>) at ../qom/object.c:1238
#2  0x0000555555d25b91 in object_property_try_add_child (obj=0x555556810800, name=0x555556709150 "peripheral", child=0x55555691fe50, errp=0x555556617680 <error_abort>)at ../qom/object.c:1753
#3  0x0000555555d21365 in container_get (root=root@entry=0x555556810800, path=path@entry=0x555555f249e9 "/peripheral") at ../qom/container.c:41
#4  0x0000555555a6c639 in machine_initfn (obj=0x555556810800) at ../hw/core/machine.c:985
#5  0x0000555555d22ee6 in object_init_with_type (obj=0x555556810800, ti=0x5555566d7a50) at ../qom/object.c:372
#6  0x0000555555d22ee6 in object_init_with_type (obj=0x555556810800, ti=0x5555566d6d70) at ../qom/object.c:372
#7  0x0000555555d22ee6 in object_init_with_type (obj=0x555556810800, ti=0x5555566d05f0) at ../qom/object.c:372
#8  0x0000555555d2454c in object_initialize_with_type (obj=0x555556810800, size=<optimized out>, type=0x5555566d05f0) at ../qom/object.c:518
#9  0x0000555555d24699 in object_new_with_type (type=0x5555566d05f0) at ../qom/object.c:738
#10 0x0000555555c65ac0 in qemu_create_machine (qdict=<optimized out>) at ../softmmu/vl.c:2110
#11 qemu_init (argc=<optimized out>, argv=0x7fffffffd358, envp=<optimized out>) at ../softmmu/vl.c:3640
#12 0x0000555555940c8d in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
```

```c
static void machine_initfn(Object *obj)
{
    MachineState *ms = MACHINE(obj);
    MachineClass *mc = MACHINE_GET_CLASS(obj);

    container_get(obj, "/peripheral");      // 这个玩意儿的作用是什么?
    container_get(obj, "/peripheral-anon");
}
```
- container_get 实际上是在 obj 下创建出来两个


```c
static const TypeInfo machine_info = {
    .name = TYPE_MACHINE,
    .parent = TYPE_OBJECT,
    .abstract = true,
    .class_size = sizeof(MachineClass),
    .class_init    = machine_class_init,
    .class_base_init = machine_class_base_init,
    .instance_size = sizeof(MachineState),
    .instance_init = machine_initfn,
    .instance_finalize = machine_finalize,
};
```

- [ ] 我不能理解，为什么 machine_info 的 parent 是 TYPE_OBJECT 而不是 qdev 啊


- [x] ObjectPropertyRelease : 一个 Property 释放的时候可以调用的 hook

- [x] ObjectPropertyResolve : 用于路径解析的吧!
```c
Object *object_resolve_path_component(Object *parent, const char *part)
{
    ObjectProperty *prop = object_property_find(parent, part);
    if (prop == NULL) {
        return NULL;
    }

    if (prop->resolve) {
        return prop->resolve(parent, prop->opaque, part);
    } else {
        return NULL;
    }
}
```

其中的 link 是什么意思哇，原来即使 soft link / hard link 的含义:
```c
typedef struct {
    union {
        Object **targetp;
        Object *target; /* if OBJ_PROP_LINK_DIRECT, when holding the pointer  */
        ptrdiff_t offset; /* if OBJ_PROP_LINK_CLASS */
    };
    void (*check)(const Object *, const char *, Object *, Error **);
    ObjectPropertyLinkFlags flags;
} LinkProperty;
```

```c
static void register_smram_listener(Notifier *n, void *unused)
{
    MemoryRegion *smram =
        (MemoryRegion *) object_resolve_path("/machine/smram", NULL);
```

```c
/*
#0  object_resolve_link_property (parent=0x555556810800, opaque=0x555556aa5dd0, part=0x555557e86480 "smram") at ../qom/object.c:1900
#1  0x0000555555d26416 in object_resolve_abs_path (parent=<optimized out>, parts=<optimized out>, typename=0x555555fc246d "object") at ../qom/object.c:2082
#2  0x0000555555d2660c in object_resolve_path_type (path=<optimized out>, typename=0x555555fc246d "object", ambiguousp=0x0) at ../qom/object.c:2144
#3  0x0000555555b78ec8 in register_smram_listener (n=<optimized out>, unused=<optimized out>) at ../target/i386/kvm/kvm.c:2204
#4  0x0000555555e89817 in notifier_list_notify (list=list@entry=0x5555565f45a8 <machine_init_done_notifiers>, data=data@entry=0x0) at ../util/notify.c:39
#5  0x0000555555a6c82b in qdev_machine_creation_done () at ../hw/core/machine.c:1321
#6  0x0000555555c64ec0 in qemu_machine_creation_done () at ../softmmu/vl.c:2669
#7  qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2692
#8  qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2683
#9  0x0000555555c685a8 in qemu_init (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/vl.c:3707
#10 0x0000555555940c8d in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
```

就是获取一下存储到 LinkProperty::targetp 中的内容:
```c
static Object *object_resolve_link_property(Object *parent, void *opaque,
                                            const char *part)
{
    LinkProperty *lprop = opaque;
    return *object_link_get_targetp(parent, lprop);
}

static Object **
object_link_get_targetp(Object *obj, LinkProperty *lprop)
{
    if (lprop->flags & OBJ_PROP_LINK_DIRECT) {
        return &lprop->target;
    } else if (lprop->flags & OBJ_PROP_LINK_CLASS) {
        return (void *)obj + lprop->offset;
    } else {
        return lprop->targetp;
    }
}
```
这个东西是在 object_add_link_prop 中初始化的，其调用者为 object_property_add_link 和 object_property_add_const_link

- [x] ObjectPropertyInit : 初始化的 hook

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
