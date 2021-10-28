#include <exec/memory.h>
#include <exec/ram_addr.h>
#include <qemu/units.h>
#include <test/test.h>
#include <unitest/greatest.h>

TEST test_basic(void) {
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

TEST test_memory_ldst(void) { PASS(); }

TEST test_readonly_memory(void) { PASS(); }

TEST test_smc(void) { PASS(); }

TEST test_pam(void) { PASS(); }

TEST test_lookup(void) { PASS(); }

SUITE(memory_model_test) { RUN_TEST(test_basic); }
