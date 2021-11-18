/*
 * QEMU 8253/8254 interval timer emulation
 *
 * Copyright (c) 2003-2004 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef HW_I8254_H
#define HW_I8254_H

#include "hw/isa/isa.h"
#include "hw/timer/i8254_internal.h"

#define PIT_FREQ 1193182

struct PITChannelInfo {
  int gate;
  int mode;
  int initial_count;
  int out;
};

#define TYPE_PIT_COMMON "pit-common"
#define TYPE_I8254 "isa-pit"

static inline PITCommonClass *PIT_COMMON_GET_CLASS(PITCommonState *pit) {
  assert(pit->pcc);
  return pit->pcc;
}

static inline void PIT_COMMON_SET_CLASS(PITCommonState *pit,
                                        PITCommonClass *pcc) {
  assert(pcc);
  pit->pcc = pcc;
}

void pit_set_gate(PITCommonState *pit, int channel, int val);
void pit_get_channel_info(PITCommonState *pit, int channel,
                          PITChannelInfo *info);

#endif /* HW_I8254_H */
