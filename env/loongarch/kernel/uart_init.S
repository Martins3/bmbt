#include <linux/linkage.h>
#include <asm/unwind_hints.h>
#include <asm/stackframe.h>

SYM_FUNC_START(asm_init_uart)
li.d t1, 0x800000001fe001e0

# WRITE(li.dne,OFS_FIFO,FIFO_ENABLE|FIFO_RCV_RST|FIFO_XMT_RST|FIFO_TRIGGER_4);
li.d t2, 0x47
st.b t2, t1, 2
dbar 0

# WRITE(li.dne,OFS_LINE_CONTROL, 0x80);
li.d t2, 0x80
st.b t2, t1, 3
dbar 0

# WRITE(li.dne,OFS_DIVISOR_LSB, divisor & 0xff);
li.d t2, 0x1
st.b t2, t1, 0
dbar 0

# WRITE(li.dne,OFS_DIVISOR_MSB, (divisor & 0xff00) >> 8);
li.d t2, 0x0
st.b t2, t1, 1
dbar 0

# WRITE(li.dne,OFS_DATA_FORMAT, data | parity | stop);
li.d t2, 0x3
st.b t2, t1, 3
dbar 0

# WRITE(li.dne,OFS_MODEM_CONTROL,0);
li.d t2, 0x0
st.b t2, t1, 4
dbar 0

jirl    zero, ra, 0
SYM_FUNC_END(asm_init_uart)
