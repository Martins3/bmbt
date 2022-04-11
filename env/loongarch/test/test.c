#include "cpu.h"
#include <asm/debug.h>
#include <asm/device.h>
#include <asm/io.h>
#include <asm/loongarchregs.h>
#include <asm/ptrace.h>
#include <asm/time.h>
#include <assert.h>
#include <env/timer.h>
#include <exec/helper-proto.h>
#include <limits.h>
#include <linux/irqflags.h>
#include <linux/pci.h>
#include <math.h>
#include <qemu/queue.h>
#include <sys/mman.h>
#include <unitest/greatest.h>

typedef struct AllocatedMem {
  QLIST_ENTRY(AllocatedMem) mem_next;
  void *p;
  int pages;
} AllocatedMem;

QLIST_HEAD(, AllocatedMem) free_nodes;
QLIST_HEAD(, AllocatedMem) allocated_pool;

#define NODE_NUM 1000
#define MAX_SIZE 10
static AllocatedMem nodes[NODE_NUM];

static void *mmap_pages(int x) {
  void *p = mmap(0, x << PAGE_SHIFT, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANON, -1, 0);
  assert(p != NULL);
  return p;
}

static AllocatedMem *alloc_pages(int num) {
  AllocatedMem *node = QLIST_FIRST(&free_nodes);
  if (node == NULL)
    return NULL;

  char *p = (char *)mmap_pages(num);
  for (int i = 0; i < num; ++i) {
    p[PAGE_SIZE * i] = '0';
  }

  QLIST_INSERT_HEAD(&allocated_pool, node, mem_next);
  return node;
}

static void free_pages() {
  if (QLIST_EMPTY(&allocated_pool)) {
  }

  AllocatedMem *node;
  QLIST_FOREACH(node, &allocated_pool, mem_next) {}
}

static void init_pools() {
  QLIST_INIT(&free_nodes);
  QLIST_INIT(&allocated_pool);
  for (int i = 0; i < NODE_NUM; ++i) {
    QLIST_INSERT_HEAD(&free_nodes, &nodes[i], mem_next);
  }
}

// @todo it's unfinished
TEST test_mmap(void) {
  init_pools();

  for (int i = 0; i < NODE_NUM; ++i) {
    alloc_pages(rand() % MAX_SIZE);
    if (i % 7 == 1) {
      free_pages();
    }
  }
  PASS();
}

TEST test_float(void) {
  float x = 2.0;
  float y = 3.0;
  printf("%s %f\n", __FUNCTION__, x + y);
  PASS();
}

TEST test_interrupt(void) {
  printf("CSR ecfg: %08lx \n", csr_readq(LOONGARCH_CSR_ECFG));
  ASSERT_EQ(csr_readq(LOONGARCH_CSR_ECFG), 0x71fff);
  PASS();
}

TEST test_syscall(void) {
  asm volatile("syscall 0");
  PASS();
}

TEST test_segmentfault(void) {
  int *a = (void *)NULL;
  a[0] = 12;
  PASS();
}

int x;
void hello() {
  x++;
  long tval = csr_readq(LOONGARCH_CSR_TVAL);
  long estat = csr_readq(LOONGARCH_CSR_ESTAT);
  printf("huxueshi:%s tval=%lx\n", __FUNCTION__, tval);
  printf("huxueshi:%s estate=%lx\n", __FUNCTION__, estat);
  printf("huxueshi:%s %d\n\n", __FUNCTION__, x);
  constant_timer_next_event(100000000);
}

TEST test_timer(void) {
  setup_timer(hello);
  local_irq_disable();
  constant_timer_next_event(0x100000);
  local_irq_enable();
  printf("huxueshi:%s \n", __FUNCTION__);
  while (1) {
  }
  PASS();
}

TEST test_sqrt(void) {
  ASSERT_EQ(sqrt(4.0), 2.0);
  PASS();
}

TEST test_bad_instruction(void) {
  asm(".long 0x0");
  PASS();
}

TEST test_rdtime(void) {
  printf("huxueshi:%s %lx\n", __FUNCTION__, drdtime());
  PASS();
}

TEST test_debugcon(void) {
  u8 x = readb(LS_ISA_DEBUGCON_IO_BASE);
  debugcon_puts("this is from debugcon\n");
  ASSERT_EQ(x, 0x77);
  PASS();
}

TEST test_pci(void) {
  u32 val = 0x12345678;
  pci_bus_write_config_dword(0, 0x100, val);
  PASS();
}

TEST test_pio(void) {
  CPUX86State env;
  env.hflags = 0;
  uint32_t port = 0xd000;
  target_ulong x = helper_inl(&env, port);
  printf("[huxueshi:%s:%d] %x\n", __FUNCTION__, __LINE__, x);
  abort();
  PASS();
}

TEST test_1M_2M_write(void) {
  unsigned char *x = (unsigned char *)TO_CAC(0);
  for (int i = 0x100000; i < 0x200000; ++i) {
    unsigned char *ptr = (x + i);
    *ptr = 0x33;
  }
  for (int i = 0x100000; i < 0x200000; ++i) {
    unsigned char *ptr = (x + i);
    assert(*ptr == 0x33);
  }
  PASS();
}

#define DLAB 0x80

#define TXR 0 /*  Transmit register (WRITE) */
#define RXR 0 /*  Receive register  (READ)  */
#define IER 1 /*  Interrupt Enable          */
#define IIR 2 /*  Interrupt ID              */
#define FCR 2 /*  FIFO control              */
#define LCR 3 /*  Line control              */
#define MCR 4 /*  Modem control             */
#define LSR 5 /*  Line Status               */
#define MSR 6 /*  Modem Status              */
#define DLL 0 /*  Divisor Latch Low         */
#define DLH 1 /*  Divisor latch High        */

TEST test_baud_rate() {

  void *port = LS_ISA_SERIAL_IO_BASE;
  int baud = 115200;
  unsigned char t;
  unsigned divisor;
  printf("[huxueshi:%s:%d] read \n", __FUNCTION__, __LINE__);
  printf("[huxueshi:%s:%d] read \n", __FUNCTION__, __LINE__);
  printf("[huxueshi:%s:%d] read \n", __FUNCTION__, __LINE__);
  divisor = 6220800 / baud;
  writeb(0x3, port + LCR); /* 8n1 */
  writeb(0, port + IER);   /* no interrupt */
  writeb(0, port + FCR);   /* no fifo */
  writeb(0x3, port + MCR); /* DTR + RTS */

  t = readb(port + LCR);
  writeb(t | DLAB, port + LCR);
  writeb(divisor & 0xff, port + DLL);
  writeb((divisor >> 8) & 0xff, port + DLH);
  writeb(t & ~DLAB, port + LCR);

  printf("[huxueshi:%s:%d] after \n", __FUNCTION__, __LINE__);
  printf("[huxueshi:%s:%d] after \n", __FUNCTION__, __LINE__);
  printf("[huxueshi:%s:%d] after \n", __FUNCTION__, __LINE__);
  PASS();
}

TEST test_read_freq() {
  void *port = LS_ISA_SERIAL_IO_BASE;
  unsigned char a;
  unsigned char b;
  unsigned char t;
  printf("[huxueshi:%s:%d] read \n", __FUNCTION__, __LINE__);
  printf("[huxueshi:%s:%d] read \n", __FUNCTION__, __LINE__);
  printf("[huxueshi:%s:%d] read \n", __FUNCTION__, __LINE__);

  t = readb(port + LCR);
  writeb(t | DLAB, port + LCR);
  a = readb(port + DLL);
  b = readb(port + DLH);
  writeb(t & ~DLAB, port + LCR);

  printf("[huxueshi:%s:%d] %x %x\n", __FUNCTION__, __LINE__, a, b);
  PASS();
}

extern void asm_init_uart();
TEST test_baud_rate_asm() {
  printf("[huxueshi:%s:%d] before \n", __FUNCTION__, __LINE__);
  printf("[huxueshi:%s:%d] before \n", __FUNCTION__, __LINE__);
  printf("[huxueshi:%s:%d] before \n", __FUNCTION__, __LINE__);
  asm_init_uart();
  printf("[huxueshi:%s:%d] after \n", __FUNCTION__, __LINE__);
  printf("[huxueshi:%s:%d] after \n", __FUNCTION__, __LINE__);
  printf("[huxueshi:%s:%d] after \n", __FUNCTION__, __LINE__);
  PASS();
}

SUITE(env_test) {
  RUN_TEST(test_read_freq);
  RUN_TEST(test_baud_rate);
  /* RUN_TEST(test_baud_rate_asm); */
  /* RUN_TEST(test_1M_2M_write); */
  /* RUN_TEST(test_pio); */
  /* RUN_TEST(test_pci); */
  /* RUN_TEST(test_debugcon); */
  /* RUN_TEST(test_timer); */
  /* RUN_TEST(test_rdtime); */
  /* RUN_TEST(test_bad_instruction); */
  /* RUN_TEST(test_sqrt); */
  /* RUN_TEST(test_float); */
  /* RUN_TEST(test_interrupt); */
  /* RUN_TEST(test_syscall); */
  /* RUN_TEST(test_segmentfault); */
  /* RUN_TEST(test_mmap); */
}
