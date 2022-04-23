#include "internal.h"
#include <asm/addrspace.h>
#include <asm/device.h>
#include <asm/io.h>
#include <stdbool.h>
#include <stdio.h>

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
  else
    kern_not_reach("mixed printf\n");
}

void uart_putc(char c) {
  int timeout = 1000000;

  while (((serial_in(UART_LSR) & UART_LSR_THRE) == 0) && (timeout-- > 0))
    ;

  serial_out(UART_TX, c);
}

#define UART_IER 1         /* Out: Interrupt Enable Register */
#define UART_IER_MSI 0x08  /* Enable Modem status interrupt */
#define UART_IER_RLSI 0x04 /* Enable receiver line status interrupt */
#define UART_IER_THRI 0x02 /* Enable Transmitter holding register int. */
#define UART_IER_RDI 0x01  /* Enable receiver data interrupt */

#define UART_IIR 2           /* In:  Interrupt ID Register */
#define UART_IIR_NO_INT 0x01 /* No interrupts pending */
#define UART_IIR_ID 0x0e     /* Mask for the interrupt ID */
#define UART_IIR_MSI 0x00    /* Modem status interrupt */
#define UART_IIR_THRI 0x02   /* Transmitter holding register empty */
#define UART_IIR_RDI 0x04    /* Receiver data interrupt */
#define UART_IIR_RLSI 0x06   /* Receiver line status interrupt */

#define UART_LSR 5                   /* In:  Line Status Register */
#define UART_LSR_FIFOE 0x80          /* Fifo error */
#define UART_LSR_TEMT 0x40           /* Transmitter empty */
#define UART_LSR_THRE 0x20           /* Transmit-hold-register empty */
#define UART_LSR_BI 0x10             /* Break interrupt indicator */
#define UART_LSR_FE 0x08             /* Frame error indicator */
#define UART_LSR_PE 0x04             /* Parity error indicator */
#define UART_LSR_OE 0x02             /* Overrun error indicator */
#define UART_LSR_DR 0x01             /* Receiver data ready */
#define UART_LSR_BRK_ERROR_BITS 0x1E /* BI, FE, PE, OE bits */

void shutup_uart(){
    writeb(0x0, LS_ISA_SERIAL_IO_BASE + UART_IER);
}

void dump_uart_state() {
  u8 ier = readb(LS_ISA_SERIAL_IO_BASE + UART_IER);
  printf("[huxueshi:%s:%d] 0x%x\n", __FUNCTION__, __LINE__, ier);
  u8 iir = readb(LS_ISA_SERIAL_IO_BASE + UART_IIR);
  u8 lsr = readb(LS_ISA_SERIAL_IO_BASE + UART_LSR);

  if (iir & UART_IIR_NO_INT) {
    printf("[huxueshi:%s:%d] no interrupt at all\n", __FUNCTION__, __LINE__);
  } else {
    printf("great there's interrupt\n");
  }
  printf("[huxueshi:%s:%d] 0x%x\n", __FUNCTION__, __LINE__, lsr);
}
