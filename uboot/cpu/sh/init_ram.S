/*
 * (C) Copyright 2004-2010 STMicroelectronics.
 *
 * Andy Sturges <andy.sturges@st.com>
 * Start Menefy <stuart.menefy@st.com>
 * Sean McGoogan <Sean.McGoogan@st.com>
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

#include <config.h>
#include "asm/regdef.h"
#include "asm/asmdefs.h"
#include "asm/linkage.h"
#include "asm/cache.h"

/*
 *	NOTE:	 ALL THIS CODE MUST BE PIC !!!!
 *
 *	This code expects to be run with the caches enabled.
 */

	.section .text.init, "ax"

ENTRY(init_ram)
	/* Stash the pr somewhere safe */
	sts	pr, r14

	/* read the DEVICE_ID, and stash in a register */
#if defined(CONFIG_SH_OLD_POKE_LOOP_PARADIGM)
	GETDEVID r7	/* use R7 for the OLD interpreter */
#else
	GETDEVID r5	/* use R5 for the NEW interpreter */
#endif	/* CONFIG_SH_OLD_POKE_LOOP_PARADIGM */

	/*
	 * We need to get the poke loop & associated data
	 * into caches. The poke loop is structured so that it is
	 * all pulled into cache on it 1st iteration.
	 * To get the poke table into D-cache, we simply read it all.
	 */
	mova	poke_loop_address, r0	/* R6: &poke_loop() */
	mov.l	@r0, r6
	add	r0, r6
	mov.l	p2_to_p1_mask, r3	/* R3: P2 -> P1 mapping */
	and	r3, r6			/* convert to P1 addresses */
	mov.l	data_start_address, r1	/* R1 = start address */
	add	r0, r1
	mov.l	data_end_address, r2	/* R2 = end address */
	add	r0, r2
	and	r3, r1			/* convert to a P1 addresses */
	and	r3, r2

	/*
	 * Load the entire poke table into the operand cache.
	 * We just need to load one word per cache line.
	 */
	mov	r1, r0			/* R0 = scratch data ptr */
1:	mov.l	@r0, r4			/* read one word per cache line */
	add	#DCACHE_LINESZ, r0	/* next cache line */
	cmp/hi	r2, r0			/* done ? */
	bf	1b

	/*
	 * Its now safe to call the poke loop with real data, since it
	 * and its associated data table are in on-chip caches. Setting
	 * up the memory interfaces may cause the EMI (where this FLASH
	 * image resides) to briefly hang - but the CPU will be safely
	 * executing from cache should this happen.
	 */
do_pokes:
#if !defined(CONFIG_SH_OLD_POKE_LOOP_PARADIGM)
	mov	r1, r4		/* __memory_setup_table */
#endif	/* CONFIG_SH_OLD_POKE_LOOP_PARADIGM */
	jsr	@r6		/* R6 still points poke_loop() */
	  nop

	/* Restore the PR */
	lds	r14, pr
	rts
	  nop


	.balign 4
poke_loop_address:	.long poke_loop - poke_loop_address
data_start_address:	.long __memory_setup_table - poke_loop_address
data_end_address:	.long __memory_setup_table_end - poke_loop_address
p2_to_p1_mask:		.long ~0x20000000



/*
 * Only define the following if you want to use the "old"
 * poke loop table interpreter. This should *not* be
 * defined if you are now using the "romgen" tool to
 * generate contemporary poke tables. The latest generation
 * targetpacks may be incompatible with the old interpreter.
 *
 *	#define CONFIG_SH_OLD_POKE_LOOP_PARADIGM
 */

#if defined(CONFIG_SH_OLD_POKE_LOOP_PARADIGM)
#	include "asm/old-pokeloop.h"
#	include "old-pokeloop.S"
#else
#	include "asm/pokeloop.h"
#	include "pokeloop.S"
#endif	/* CONFIG_SH_OLD_POKE_LOOP_PARADIGM */


