#include "../i386/cpu.h"
#include "../i386/helper.h"
#include <exec/memory.h>
#include <exec/ram_addr.h>
#include <qemu/units.h>
#include <sysemu/cpus.h>
#include <test/test.h>
#include <unitest/greatest.h>

TEST test_env_setup(void) {
  memory_map_init(128 * MiB);
  RAMBlock *block;
  uint64_t end = 0;

  RAMBLOCK_FOREACH(block) {
    if (block->mr->offset != end) {
      ASSERT_GT(block->mr->offset, 128 * MiB);
    }
    end = block->mr->size + block->mr->offset;
  }

  MemTxAttrs attrs;

  for (int i = 0; i < RAM_BLOCK_NUM; ++i) {
    MemoryRegion *mr = &ram_list.blocks[i].mr;
    hwaddr addr = mr->offset + rand() % PAM_EXBIOS_SIZE;
    hwaddr xlat;
    uint64_t len = 0xfffffff;
    MemoryRegion *look_up_mr = address_space_translate(
        &address_space_memory, addr, &xlat, &len, false, attrs);
    ASSERT_EQ(mr, look_up_mr);
    ASSERT_EQ(xlat, addr - mr->offset);
    ASSERT_EQ(len, mr->size - xlat);
  }

  PASS();
}

static uint64_t ops_read(void *opaque, hwaddr addr, unsigned size) {
  MemoryRegion *mr = (MemoryRegion *)opaque;
  return 0x12345678abcdef12;
}

static void ops_write(void *opaque, hwaddr addr, uint64_t value,
                      unsigned size) {
  MemoryRegion *mr = (MemoryRegion *)opaque;
  duck_check(value == 0x12345678);
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
  struct MemTxAttrs attrs;
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

TEST test_dirty_memory(void) {
  struct MemTxAttrs attrs;
  CPUX86State env;
  MemTxResult mem;
  env.hflags = 0;
  hwaddr addr = 0x100000;
  uint32_t msg_size = 0x100;
  char *msg = (char *)g_malloc0(msg_size + 1);
  char *msg2 = (char *)g_malloc0(msg_size + 1);
  for (int i = 0; i < msg_size; ++i) {
    msg[i] = 'x';
  }

  mem = address_space_rw(&address_space_memory, addr, attrs, (void *)msg,
                         msg_size, true);
  mem = address_space_rw(&address_space_io, addr, attrs, (void *)msg2, 8, true);
  for (int i = 0; i < msg_size; ++i) {
    ASSERT_EQ(msg[i], msg2[i]);
  }
  PASS();
}

TEST test_smc(void) { PASS(); }

TEST test_pam(void) { PASS(); }

SUITE(memory_model_test) {
  RUN_TEST(test_env_setup);
  RUN_TEST(test_rw);
  // RUN_TEST(test_dirty_memory);
}
