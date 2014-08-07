/*
 * (C) Copyright 2004 STMicroelectronics
 *
 * Andy Sturges <andy.sturges@st.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "common.h"
#include <asm/cache.h>
#include <asm/io.h>
#include <asm/sh4reg.h>

struct __large_struct { unsigned long buf[100]; };
#define __m(x) (*(struct __large_struct *)(x))

/*
 * Write back the dirty D-caches, but not invalidate them.
 *
 * START: Virtual Address (U0, P1, or P3)
 * SIZE: Size of the region.
 */
#if 0
static void __flush_wback_region(void *start, int size)
{
	unsigned long v;
	unsigned long begin, end;

	begin = (unsigned long)start & ~(L1_CACHE_BYTES-1);
	end = ((unsigned long)start + size + L1_CACHE_BYTES-1)
		& ~(L1_CACHE_BYTES-1);
	for (v = begin; v < end; v+=L1_CACHE_BYTES) {
		asm volatile("ocbwb	%0"
			     : /* no output */
			     : "m" __m(v));
	}
}
#endif

/*
 * Write back the dirty D-caches and invalidate them.
 *
 * START: Virtual Address (U0, P1, or P3)
 * SIZE: Size of the region.
 */
static void __flush_purge_region(unsigned long start, int size)
{
	unsigned long v;
	unsigned long begin, end;

	begin = (unsigned long)start & ~(L1_CACHE_BYTES-1);
	end = ((unsigned long)start + size + L1_CACHE_BYTES-1)
		& ~(L1_CACHE_BYTES-1);
	for (v = begin; v < end; v+=L1_CACHE_BYTES) {
		asm volatile("ocbp	%0"
			     : /* no output */
			     : "m" __m(v));
	}
}

#if 0
/*
 * No write back please
 */
static void __flush_invalidate_region(void *start, int size)
{
	unsigned long v;
	unsigned long begin, end;

	begin = (unsigned long)start & ~(L1_CACHE_BYTES-1);
	end = ((unsigned long)start + size + L1_CACHE_BYTES-1)
		& ~(L1_CACHE_BYTES-1);
	for (v = begin; v < end; v+=L1_CACHE_BYTES) {
		asm volatile("ocbi	%0"
			     : /* no output */
			     : "m" __m(v));
	}
}
#endif

void flush_cache (ulong start_addr, ulong size)
{
   __flush_purge_region(start_addr, size);
}

/*
 * Flush all the data cache lines
 */
extern void sh_flush_cache_all(void)
{
	unsigned long addr;
	const unsigned long end_addr = CACHE_OC_ADDRESS_ARRAY +
		(DCACHE_SETS << DCACHE_ENTRY_SHIFT) * DCACHE_WAYS;
	const unsigned long entry_offset = 1ul << DCACHE_ENTRY_SHIFT;

	for (addr = CACHE_OC_ADDRESS_ARRAY;
	     addr < end_addr;
	     addr += entry_offset) {
		ctrl_outl(0, addr);
	}
}

/*
 * sh_disable_data_caches() and sh_enable_data_caches()
 * compliment each other. They are designed to be used in
 * pairs to surround code that is designed to work with code
 * that is not cache-coherency aware. e.g. immature drivers.
 *
 * The default assumption is that the chaches are normally ON,
 * and the OFF state is a temporary exception.
 *
 * As an optimization, consequtive calls to change the data
 * cacheability will do nothing, if the caches are correct.
 * A global variable 'sh_data_caches_on' is used to track
 * the current state of the data caches.
 */
int sh_data_caches_on = 1;	/* assume caches are ON */

extern void sh_disable_data_caches(void)
{
	if (!sh_data_caches_on)	/* already OFF ? */
		return;		/* do nothing */

	/*
	 * Flush the operand caches, to ensure that there is
	 * no unwritten data residing only in the data cache.
	 */
	sh_flush_cache_all();

	/* Disable the Operand Caches */
	sh_cache_clear_op(SH4_CCR_OCE);

	sh_data_caches_on = 0;	/* caches are OFF */
}

extern void sh_enable_data_caches(void)
{
	if (sh_data_caches_on)	/* already ON ? */
		return;		/* do nothing */

	/* Invalidate, and enable the data caches */
	sh_cache_set_op(SH4_CCR_OCI|SH4_CCR_OCE);

	sh_data_caches_on = 1;	/* caches are ON */
}

int cleanup_before_linux (void)
{
	return (0);
}

