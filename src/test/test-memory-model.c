#include <exec/memory.h>
#include <unitest/greatest.h>

// memory.c exec.c ram_addr.h 需要 100% 覆盖的
TEST test_mmio(void) {
  printf("huxueshi:%s \n", __FUNCTION__);
  PASS();
}

TEST test_ramblock(void) { PASS(); }

TEST test_memory_ldst(void) { PASS(); }

TEST test_readonly_memory(void) { PASS(); }

TEST test_smc(void) { PASS(); }

TEST test_pam(void) { PASS(); }

TEST test_lookup(void) { PASS(); }

SUITE(memory_model_test) { test_mmio(); }
