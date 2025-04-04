#include <environment/cpu.h>
#include <hw/boards.h>
#include <qemu/units.h>
#include <sysemu/numa.h>

static void smp_parse(MachineState *ms) { g_assert_not_reached(); }

void numa_complete_configuration(MachineState *ms);
void numa_default_auto_assign_ram(MachineClass *mc, NodeInfo *nodes,
                                  int nb_nodes, ram_addr_t size);

void machine_class_init(MachineClass *mc) {
  /* Default 128 MB as guest ram size */
  mc->default_ram_size = 128 * MiB;
  mc->rom_file_has_mr = true;
  mc->smp_parse = smp_parse;

  /* numa node memory size aligned on 8MB by default.
   * On Linux, each node's border has to be 8MB aligned
   */
  mc->numa_mem_align_shift = 23;
  mc->numa_auto_assign_ram = numa_default_auto_assign_ram;

  // [interface 24]
#ifdef BMBT
  object_class_property_add_str(oc, "accel", machine_get_accel,
                                machine_set_accel, &error_abort);
  object_class_property_set_description(oc, "accel", "Accelerator list",
                                        &error_abort);

  object_class_property_add(oc, "kernel-irqchip", "on|off|split", NULL,
                            machine_set_kernel_irqchip, NULL, NULL,
                            &error_abort);
  object_class_property_set_description(
      oc, "kernel-irqchip", "Configure KVM in-kernel irqchip", &error_abort);

  object_class_property_add(
      oc, "kvm-shadow-mem", "int", machine_get_kvm_shadow_mem,
      machine_set_kvm_shadow_mem, NULL, NULL, &error_abort);
  object_class_property_set_description(oc, "kvm-shadow-mem",
                                        "KVM shadow MMU size", &error_abort);

  object_class_property_add_str(oc, "kernel", machine_get_kernel,
                                machine_set_kernel, &error_abort);
  object_class_property_set_description(oc, "kernel", "Linux kernel image file",
                                        &error_abort);

  object_class_property_add_str(oc, "initrd", machine_get_initrd,
                                machine_set_initrd, &error_abort);
  object_class_property_set_description(
      oc, "initrd", "Linux initial ramdisk file", &error_abort);

  object_class_property_add_str(oc, "append", machine_get_append,
                                machine_set_append, &error_abort);
  object_class_property_set_description(
      oc, "append", "Linux kernel command line", &error_abort);

  object_class_property_add_str(oc, "dtb", machine_get_dtb, machine_set_dtb,
                                &error_abort);
  object_class_property_set_description(
      oc, "dtb", "Linux kernel device tree file", &error_abort);

  object_class_property_add_str(oc, "dumpdtb", machine_get_dumpdtb,
                                machine_set_dumpdtb, &error_abort);
  object_class_property_set_description(
      oc, "dumpdtb", "Dump current dtb to a file and quit", &error_abort);

  object_class_property_add(
      oc, "phandle-start", "int", machine_get_phandle_start,
      machine_set_phandle_start, NULL, NULL, &error_abort);
  object_class_property_set_description(
      oc, "phandle-start", "The first phandle ID we may generate dynamically",
      &error_abort);

  object_class_property_add_str(oc, "dt-compatible", machine_get_dt_compatible,
                                machine_set_dt_compatible, &error_abort);
  object_class_property_set_description(
      oc, "dt-compatible",
      "Overrides the \"compatible\" property of the dt root node",
      &error_abort);

  object_class_property_add_bool(oc, "dump-guest-core",
                                 machine_get_dump_guest_core,
                                 machine_set_dump_guest_core, &error_abort);
  object_class_property_set_description(oc, "dump-guest-core",
                                        "Include guest memory in a core dump",
                                        &error_abort);

  object_class_property_add_bool(oc, "mem-merge", machine_get_mem_merge,
                                 machine_set_mem_merge, &error_abort);
  object_class_property_set_description(
      oc, "mem-merge", "Enable/disable memory merge support", &error_abort);

  object_class_property_add_bool(oc, "usb", machine_get_usb, machine_set_usb,
                                 &error_abort);
  object_class_property_set_description(
      oc, "usb", "Set on/off to enable/disable usb", &error_abort);

  object_class_property_add_bool(oc, "graphics", machine_get_graphics,
                                 machine_set_graphics, &error_abort);
  object_class_property_set_description(
      oc, "graphics", "Set on/off to enable/disable graphics emulation",
      &error_abort);

  object_class_property_add_bool(oc, "igd-passthru",
                                 machine_get_igd_gfx_passthru,
                                 machine_set_igd_gfx_passthru, &error_abort);
  object_class_property_set_description(
      oc, "igd-passthru", "Set on/off to enable/disable igd passthrou",
      &error_abort);

  object_class_property_add_str(oc, "firmware", machine_get_firmware,
                                machine_set_firmware, &error_abort);
  object_class_property_set_description(oc, "firmware", "Firmware image",
                                        &error_abort);

  object_class_property_add_bool(oc, "suppress-vmdesc",
                                 machine_get_suppress_vmdesc,
                                 machine_set_suppress_vmdesc, &error_abort);
  object_class_property_set_description(
      oc, "suppress-vmdesc", "Set on to disable self-describing migration",
      &error_abort);

  object_class_property_add_bool(
      oc, "enforce-config-section", machine_get_enforce_config_section,
      machine_set_enforce_config_section, &error_abort);
  object_class_property_set_description(
      oc, "enforce-config-section",
      "Set on to enforce configuration section migration", &error_abort);

  object_class_property_add_str(oc, "memory-encryption",
                                machine_get_memory_encryption,
                                machine_set_memory_encryption, &error_abort);
  object_class_property_set_description(oc, "memory-encryption",
                                        "Set memory encryption object to use",
                                        &error_abort);
#endif
}

void machine_class_base_init(MachineClass *mc) {
  // MachineClass::name is used for debug
  mc->name = "pc-i440fx-4.2";
  // mc->compat_props = g_ptr_array_new();
}

void machine_initfn(MachineState *ms) {
  MachineClass *mc = MACHINE_GET_CLASS(ms);

  ms->kernel_irqchip_allowed = true;
  ms->kernel_irqchip_split = mc->default_kernel_irqchip_split;
  ms->kvm_shadow_mem = -1;
  ms->dump_guest_core = true;
  // ms->mem_merge = true;
  ms->enable_graphics = true;

  // qemu option is too complex, hard code it here.
  ms->kernel_filename = "image/bzImage.bin";
  switch (get_boot_parameter()) {
  case FROM_INIT_RD:
    ms->initrd_filename = "image/initrd.bin";
    ms->kernel_cmdline = "console=ttyS0";
    break;
  case FROM_DISK:
    ms->initrd_filename = NULL;
    ms->kernel_cmdline = "root=PARTUUID=616e86aa-a573-4d60-bb44-b1b701d5a552 "
                         "rw console=ttyS0 init=/bin/bash";
    break;
  default:
    g_assert_not_reached();
  }

  if (mc->nvdimm_supported) {
    g_assert_not_reached();
#ifdef BMBT
    Object *obj = OBJECT(ms);

    ms->nvdimms_state = g_new0(NVDIMMState, 1);
    object_property_add_bool(obj, "nvdimm", machine_get_nvdimm,
                             machine_set_nvdimm, &error_abort);
    object_property_set_description(obj, "nvdimm",
                                    "Set on/off to enable/disable "
                                    "NVDIMM instantiation",
                                    NULL);

    object_property_add_str(obj, "nvdimm-persistence",
                            machine_get_nvdimm_persistence,
                            machine_set_nvdimm_persistence, &error_abort);
    object_property_set_description(obj, "nvdimm-persistence",
                                    "Set NVDIMM persistence"
                                    "Valid values are cpu, mem-ctrl",
                                    NULL);
#endif
  }

  if (mc->cpu_index_to_instance_props && mc->get_default_cpu_node_id) {
    ms->numa_state = g_new0(NumaState, 1);
  }

#ifdef BMBT
  /* Register notifier when init is done for sysbus sanity checks */
  ms->sysbus_notifier.notify = machine_init_notify;
  qemu_add_machine_init_done_notifier(&ms->sysbus_notifier);
#endif
}

// static void machine_finalize(Object *obj) {}

static void machine_numa_finish_cpu_init(MachineState *machine) {
  g_assert_not_reached();
}

void machine_run_board_init(MachineState *machine) {
  MachineClass *machine_class = MACHINE_GET_CLASS(machine);

  if (machine->numa_state) {
    numa_complete_configuration(machine);
    if (machine->numa_state->num_nodes) {
      machine_numa_finish_cpu_init(machine);
    }
  }

  // PC doesn't check valid_cpu_types
  assert(machine_class->valid_cpu_types == NULL);
  /* If the machine supports the valid_cpu_types check and the user
   * specified a CPU with -cpu check here that the user CPU is supported.
   */
  if (machine_class->valid_cpu_types && machine->cpu_type) {
    g_assert_not_reached();
  }

  machine_class->init(machine);
}

#ifdef BMBT
static const TypeInfo machine_info = {
    .name = TYPE_MACHINE,
    .parent = TYPE_OBJECT,
    .abstract = true,
    .class_size = sizeof(MachineClass),
    .class_init = machine_class_init,
    .class_base_init = machine_class_base_init,
    .instance_size = sizeof(MachineState),
    .instance_init = machine_initfn,
    .instance_finalize = machine_finalize,
};

static void machine_register_types(void) {
  type_register_static(&machine_info);
}

type_init(machine_register_types)
#endif
