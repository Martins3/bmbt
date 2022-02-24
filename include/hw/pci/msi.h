#ifndef MSI_H_T5WWZFJM
#define MSI_H_T5WWZFJM
#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint64_t address;
  uint32_t data;
} MSIMessage;

extern bool msi_nonbroken;

#endif /* end of include guard: MSI_H_T5WWZFJM */
