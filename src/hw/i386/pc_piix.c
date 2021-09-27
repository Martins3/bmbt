#include "../../include/hw/i386/pc.h"
#include "../../include/hw/pci-host/i440fx.h"

/* PC hardware initialisation */
static void pc_init1(MachineState *machine, const char *host_type,
                     const char *pci_type) {}

static void pc_i440fx_machine_options(MachineClass *m) {
  PCMachineClass *pcmc = PC_MACHINE_CLASS(m);
  // pcmc->default_nic_model = "e1000";

  m->family = "pc_piix";
  m->desc = "Standard PC (i440FX + PIIX, 1996)";
  // m->default_machine_opts = "firmware=bios-256k.bin";
  // m->default_display = "std";
  // machine_class_allow_dynamic_sysbus_dev(m, TYPE_RAMFB_DEVICE);
}

static void pc_i440fx_4_2_machine_options(MachineClass *m) {
  PCMachineClass *pcmc = PC_MACHINE_CLASS(m);
  pc_i440fx_machine_options(m);
  m->alias = "pc";
  m->is_default = 1;
  pcmc->default_cpu_version = 1;
}

static void pc_init_v4_2(MachineState *machine) {
  void (*compat)(MachineState * m) = (NULL);
  if (compat) {
    compat(machine);
  }
  pc_init1(machine, TYPE_I440FX_PCI_HOST_BRIDGE, TYPE_I440FX_PCI_DEVICE);
}

static void pc_machine_v4_2_class_init(MachineClass *mc, void *data) {
  pc_i440fx_4_2_machine_options(mc);
  mc->init = pc_init_v4_2;
}

#if BMBT
static const TypeInfo pc_machine_type_v4_2 = {
    .name = "pc-i440fx-4.2"
            "-machine",
    .parent = TYPE_PC_MACHINE,
    .class_init = pc_machine_v4_2_class_init,
};
static void pc_machine_init_v4_2(void) { type_register(&pc_machine_type_v4_2); }
type_init(pc_machine_init_v4_2);
#endif
