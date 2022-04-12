#include <asm/addrspace.h>
#include <stdbool.h>

#define UART_BASE 0x1fe001e0
#define PORT(offset) (TO_UNCAC(UART_BASE) + (offset))

#ifndef IOTYPE
#define IOTYPE char
#endif

#ifndef PORT
#error please define the serial port address for your own machine
#endif

#define UART_LSR 5         /* In:  Line Status Register */
#define UART_LSR_THRE 0x20 /* Transmit-hold-register empty */
#define UART_TX 0          /* Out: Transmit buffer */

// TMP_TODO : 之后仅仅允许一个位置持有 0x1fe001e0 的这个东西吧
bool can_write();

static inline unsigned int serial_in(int offset) {
  return *((volatile IOTYPE *)PORT(offset)) & 0xFF;
}

static inline void serial_out(int offset, int value) {
  if (can_write())
    *((volatile IOTYPE *)PORT(offset)) = value & 0xFF;
}

void uart_putc(char c) {
  int timeout = 1000000;

  while (((serial_in(UART_LSR) & UART_LSR_THRE) == 0) && (timeout-- > 0))
    ;

  serial_out(UART_TX, c);
}
