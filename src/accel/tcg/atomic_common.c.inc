/*
 * Common Atomic Helper Functions
 *
 * This file should be included before the various instantiations of
 * the atomic_template.h helpers.
 *
 * Copyright (c) 2019 Linaro
 * Written by Alex Bennée <alex.bennee@linaro.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

// [BMBT_MTTCG 5]
static inline
void atomic_trace_rmw_pre(CPUArchState *env, target_ulong addr, uint16_t info)
{
}

static inline void
atomic_trace_rmw_post(CPUArchState *env, target_ulong addr, uint16_t info)
{
}

static inline
void atomic_trace_ld_pre(CPUArchState *env, target_ulong addr, uint16_t info)
{
}

static inline
void atomic_trace_ld_post(CPUArchState *env, target_ulong addr, uint16_t info)
{
}

static inline
void atomic_trace_st_pre(CPUArchState *env, target_ulong addr, uint16_t info)
{
}

static inline
void atomic_trace_st_post(CPUArchState *env, target_ulong addr, uint16_t info)
{
}

static inline uint16_t trace_mem_build_info( int size_shift, bool sign_extend,
  MemOp endianness, bool store, unsigned int mmu_idx)
{
  return 0;
}
