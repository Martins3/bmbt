#include "cpu.h"
#include <exec/helper-proto.h>
#include <exec/memory.h>
#include <exec/ram_addr.h>
#include <qemu/units.h>
#include <sysemu/cpus.h>
#include <test/test.h>
#include <unitest/greatest.h>

TEST test_env_setup(void) {
  MemTxAttrs attrs = {.secure = 1};

  for (int i = 0; i < RAM_BLOCK_NUM; ++i) {
    // 1. pam extend and system bios are mapped to pc.bios
    // 2. smram is mapped into low.vga
    // so, skip them
    if (i >= SMRAM_INDEX && i < PC_RAM_INDEX) {
      continue;
    }
    MemoryRegion *mr = &ram_list.blocks[i].mr;
    hwaddr addr = mr->offset + rand() % PAM_EXBIOS_SIZE;
    hwaddr xlat;
    uint64_t len = 0xffffffffffffffffULL;
    MemoryRegion *look_up_mr = address_space_translate(
        &address_space_memory, addr, &xlat, &len, false, attrs);
    assert(mr == look_up_mr);
    assert(xlat == addr - mr->offset);
    assert(len == mr->size - xlat);
  }

  PASS();
}

static uint64_t ops_read(void *opaque, hwaddr addr, unsigned size) {
  // MemoryRegion *mr = (MemoryRegion *)opaque;
  return 0x12345678abcdef12;
}

static void ops_write(void *opaque, hwaddr addr, uint64_t value,
                      unsigned size) {
  // MemoryRegion *mr = (MemoryRegion *)opaque;
  assert(value == 0x12345678);
}

static bool test_valid(void *opaque, hwaddr addr, unsigned size, bool is_write,
                       MemTxAttrs attrs) {
  return (size == 4) || (is_write && size == 8);
}

static const MemoryRegionOps ops1 = {
    .read = ops_read,
    .write = ops_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid.accepts = test_valid,
};

TEST test_rw(void) {
  // initialize
  qemu_init_cpu_loop();
  MemTxAttrs attrs = {.secure = 1};
  CPUX86State env;
  env.hflags = 0;
  MemoryRegion *mr1 = g_malloc0(sizeof(MemoryRegion));
  uint32_t port = 0x100;
  memory_region_init_io(mr1, &ops1, mr1, "<ioport 1>", 4);
  io_add_memory_region(port, mr1);

  // helper
  target_ulong x = helper_inl(&env, port);
  ASSERT_EQ(x, 0xabcdef12);
  helper_outl(&env, port, 0x12345678);

  // address space io rw
  uint64_t buf = 0x3f3f3f3f12345678;
  MemTxResult mem =
      address_space_rw(&address_space_io, port, attrs, (void *)&buf, 4, false);
  ASSERT_EQ(buf, 0x3f3f3f3fabcdef12);
  ASSERT_EQ(mem, MEMTX_OK);
  buf = 0x3f3f3f3f12345678;
  mem = address_space_rw(&address_space_io, port, attrs, (void *)&buf, 4, true);
  ASSERT_EQ(mem, MEMTX_OK);

  PASS();
}

TEST test_memory_rw(void) {
  MemTxAttrs attrs = {.secure = 1};
  uint32_t msg_size = 0x100;
  char *msg = (char *)g_malloc0(msg_size + 1);
  char *msg2 = (char *)g_malloc0(msg_size + 1);
  for (int i = 0; i < msg_size; ++i) {
    msg[i] = i;
  }

  uint64_t total_size = get_guest_total_ram() + CONFIG_GUEST_BIOS_SIZE;
  for (int addr = 0; addr < total_size; addr += TARGET_PAGE_SIZE) {
    cpu_physical_memory_test_and_clear_dirty(addr, TARGET_PAGE_SIZE,
                                             DIRTY_MEMORY_CODE);
  }

  int guest_ram_num = get_guest_ram_num();
  hwaddr offset = 0;
  for (int i = 0; i < guest_ram_num; ++i) {
    assert(!cpu_physical_memory_get_dirty(offset, 4, DIRTY_MEMORY_CODE));
    assert(!cpu_physical_memory_get_dirty(offset + msg_size, 4,
                                          DIRTY_MEMORY_CODE));
    offset += (guest_ram(i).end - guest_ram(i).start);
  }

  for (int i = 0; i < guest_ram_num; ++i) {
    hwaddr addr = guest_ram(i).start;
    address_space_rw(&address_space_memory, addr, attrs, (void *)msg, msg_size,
                     true);
    address_space_rw(&address_space_memory, addr, attrs, (void *)msg2, msg_size,
                     false);
    for (int i = 0; i < msg_size; ++i) {
      ASSERT_EQ(msg[i], msg2[i]);
    }
  }

  offset = 0;
  for (int i = 0; i < guest_ram_num; ++i) {
    assert(!cpu_physical_memory_get_dirty(offset, 4, DIRTY_MEMORY_CODE));
    assert(!cpu_physical_memory_get_dirty(offset + msg_size, 4,
                                          DIRTY_MEMORY_CODE));
    offset += (guest_ram(i).end - guest_ram(i).start);
  }

  cpu_physical_memory_set_dirty_range(0, total_size, 1 << DIRTY_MEMORY_CODE);

  PASS();
}

SUITE(memory_model_test) {
  RUN_TEST(test_env_setup);
  RUN_TEST(test_rw);
  RUN_TEST(test_memory_rw);
}
