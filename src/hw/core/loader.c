#include "../../../include/hw/nvram/fw_cfg.h"

static FWCfgState *fw_cfg;

void rom_set_fw(FWCfgState *f) { fw_cfg = f; }
