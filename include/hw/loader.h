#include "nvram/fw_cfg.h"

void rom_set_fw(FWCfgState *f);
int rom_add_option(const char *file, int32_t bootindex);
