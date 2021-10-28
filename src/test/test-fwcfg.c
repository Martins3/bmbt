#include "../hw/i386/fw_cfg.h"
#include <hw/i386/pc.h>
#include <unitest/greatest.h>

// TODO I find it's fucking difficult to test the code without
// call the qemu_init
TEST test_init(void) { PASS(); }

TEST test_io(void) { PASS(); }

TEST test_dma(void) { PASS(); }

SUITE(fw_cfg_test) {
  RUN_TEST(test_init);
  RUN_TEST(test_io);
  RUN_TEST(test_dma);
}
