#include "e820_memory_layout.h"
#include <qemu/bswap.h>
#include <errno.h>
#include <uglib.h>

static size_t e820_entries;
struct e820_table e820_reserve;
struct e820_entry *e820_table;

int e820_add_entry(uint64_t address, uint64_t length, uint32_t type) {
  int index = le32_to_cpu(e820_reserve.count);
  struct e820_entry *entry;

  if (type != E820_RAM) {
    /* old FW_CFG_E820_TABLE entry -- reservations only */
    if (index >= E820_NR_ENTRIES) {
      return -EBUSY;
    }
    entry = &e820_reserve.entry[index++];

    entry->address = cpu_to_le64(address);
    entry->length = cpu_to_le64(length);
    entry->type = cpu_to_le32(type);

    e820_reserve.count = cpu_to_le32(index);
  }

  /* new "etc/e820" file -- include ram too */
  e820_table = g_renew(struct e820_entry, e820_table, e820_entries + 1);
  e820_table[e820_entries].address = cpu_to_le64(address);
  e820_table[e820_entries].length = cpu_to_le64(length);
  e820_table[e820_entries].type = cpu_to_le32(type);
  e820_entries++;

  return e820_entries;
}

int e820_get_num_entries(void) { return e820_entries; }

bool e820_get_entry(int idx, uint32_t type, uint64_t *address,
                    uint64_t *length) {
  if (idx < e820_entries && e820_table[idx].type == cpu_to_le32(type)) {
    *address = le64_to_cpu(e820_table[idx].address);
    *length = le64_to_cpu(e820_table[idx].length);
    return true;
  }
  return false;
}
