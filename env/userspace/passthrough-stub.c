#include <assert.h>
#include <env/cpu.h>
#include <env/device.h>
#include <env/memory.h>
#include <inttypes.h>
#include <qemu/units.h>
#include <sys/mman.h>
#include <uglib.h>

void setup_serial_pass_through() { g_assert_not_reached(); }
void pci_config_pass_write(uint32_t addr, uint32_t val, int l) {
  g_assert_not_reached();
}
uint32_t pci_config_pass_read(uint32_t addr, int l) { g_assert_not_reached(); }
void pci_pass_through_init() { g_assert_not_reached(); }
void cpu_wait(void) {}

#define GUEST_RAM_COUNT 4
typedef struct {
  uint64_t addr;
  uint64_t size;
} GuestRam;

static GuestRam user_guest_ram[GUEST_RAM_COUNT] = {
    {
        .size = 32 * MiB,
    },
    {
        .size = 32 * MiB,
    },
    {
        .size = 32 * MiB,
    },
    {
        .size = 32 * MiB,
    },
};

static uint64_t alloc_ram(hwaddr size) {
  // surround with inaccessible hole to capture errors
  mmap(0, size, 0, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  // (qemu) qemu_ram_mmap size=0x180200000 flags=0x22 guardfd=-1
  uint64_t host = (uint64_t)mmap(0, size, PROT_EXEC | PROT_READ | PROT_WRITE,
                                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  mmap(0, size, 0, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  assert(host != -1);
  return host;
}

static inline void exchange(GuestRam *x, GuestRam *y) {
  GuestRam tmp = *x;
  *x = *y;
  *y = tmp;
}

static void sort_addr(GuestRam addr[], int len) {
  for (int i = 0; i < len; ++i) {
    for (int j = i + 1; j < len; ++j) {
      if (addr[i].addr > addr[j].addr) {
        exchange(addr + i, addr + j);
      }
    }
  }
}

static uint64_t host_offset;
static void init_guest_ram() {
  if (host_offset)
    return;
  for (int i = 0; i < GUEST_RAM_COUNT; ++i) {
    user_guest_ram[i].addr = alloc_ram(user_guest_ram[i].size);
  }
  sort_addr(user_guest_ram, GUEST_RAM_COUNT);
  for (int i = 0; i < GUEST_RAM_COUNT; ++i) {
    printf("[huxueshi:%s:%d] %lx %lx\n", __FUNCTION__, __LINE__,
           user_guest_ram[i].addr, user_guest_ram[i].size);
  }
  assert(user_guest_ram[GUEST_RAM_COUNT - 1].addr - user_guest_ram[0].addr <
         0xe0000000); // 3.5G
  host_offset = user_guest_ram[0].addr;
}

int get_guest_ram_num() {
  init_guest_ram();
  return GUEST_RAM_COUNT;
}

uint64_t get_guest_total_ram() {
  init_guest_ram();
  uint64_t size = 0;
  for (int i = 0; i < GUEST_RAM_COUNT; ++i) {
    size += user_guest_ram[i].size;
  }
  return size;
}

RamRange guest_ram(int idx) {
  init_guest_ram();
  RamRange ram = {.start = user_guest_ram[idx].addr - host_offset,
                  .end = user_guest_ram[idx].addr - host_offset +
                         user_guest_ram[idx].size};
  return ram;
}

uint64_t get_host_offset() {
  init_guest_ram();
  return host_offset;
}

BootParameter get_boot_parameter() { return FROM_INIT_RD; }
