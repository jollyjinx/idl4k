/*
 * (C) Copyright 2004-2009 STMicroelectronics.
 *
 * Andy Sturges <andy.sturges@st.com>
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
#include "asm/addrspace.h"
#include "asm/asmdefs.h"
#include "asm/linkage.h"
#include "asm/sh4reg.h"
#include "asm/pmb.h"

	.section .text.init, "ax"

	/*
	 * void sh_cache_set_op (ulong op)
	 * set bits in ccn.cr
	 */
ENTRY(sh_cache_set_op)
	/* get ccn.cr address into r0 (0xff00001c) */
	MOV_CONST32_R0	SH4_CCN_CCR
	mov.l	@r0, r1
	or	a0, r1
	mov.l	r1, @r0
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	rts
	  nop


	/*
	 * void sh_cache_clear_op (ulong op)
	 * clears bits in ccn.cr
	 */
ENTRY(sh_cache_clear_op)
	/* get ccn.cr address into r0 (0xff00001c) */
	MOV_CONST32_R0	SH4_CCN_CCR
	mov.l	@r0, r1
	not	a0, a0	/* invert op */
	and	a0, r1	/* clear bits */
	mov.l	r1, @r0
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	rts
	  nop


#ifdef CONFIG_SH_SE_MODE
/* Move the PC from one PMB region to another, by masking.
Note: This macro will clobber r0.
Usage: .SWITCH_PMB <opcode> <mask_reg>
where	<opcode> should be the masking opcode: "and" or "or".
	<mask_reg> register name, containing the mask to use. */
.macro	SWITCH_PMB opcode:req, mask_reg:req
	mova	1f, r0
	\opcode	\mask_reg, r0
	ldc	r0, spc		/* SPC = PC op mask */
	stc	sr, r0
	ldc	r0, ssr		/* SSR = SR */
	rte			/* do it */
	  nop
.balign 4
1:
.endm

	/*
	 * void sh_toggle_pmb_cacheability (void)
	 *
	 * Toggles the cacheability for PMB index #0, and
	 * possibly PMB index #1 as well.
	 *
	 * We do this by changing the PC to run out of an
	 * aliased PMB entry (not #0, or #1), which is UN-cached.
	 * From here we 'toggle' the cacheability of the
	 * appropriate PMB array entries, then invalidate
	 * the ITLB.  Finally we switch back the PC to the
	 * region we were originally running from.
	 *
	 * We only toggle the UB and the C flags of PMB[0],
	 * and possibly PMB[1].
	 *
	 * i.e. this function does:
	 * {
	 *	PC        = PC | 0x10000000;
	 *	PMB[0].C  = ~PMB[0].C;
	 *	PMB[0].UB = ~PMB[0].UB;
	 * #if CFG_SH_LMI_NEEDS_2_PMB_ENTRIES
	 *	PMB[1].C  = ~PMB[1].C;
	 *	PMB[1].UB = ~PMB[1].UB;
	 * #endif
	 *      MMUCR.TI  = 1;
	 *	PC        = PC & ~0x10000000;
	 * }
	 *
	 * Assumptions:
	 * 0) We use 'Z' to mean 0 or 1.
	 * 1) We assume that the PC (on entry) in running in
	 *    the virtual address space controlled by
	 *    array index #Z of the PMB.
	 *    i.e. PC is in VA defined by PMB[Z].
	 * 2) PMB[Z] may be cached or UN-cached on entry.
	 *    The cacheability on exit will be inverted,
	 *    (i.e. toggled) w.r.t the cacheability on entry.
	 * 3) We assume that an UN-cached PMB entry already
	 *    exists which is an alias to the same physical
	 *    address as the PMB entry we are currently running in.
	 *    i.e. PMB[x] is an UN-cached alias for PMB[Z].
	 *    so, PMB[Z].PPN == PMB[x].PPN
	 * 4) The un-cached PMB entry 'x' must not be:
	 *    a) Index #Z as this is the one we are changing
	 *    b) Index #15 as the linux kernel uses this on entry.
	 *    i.e. x is in the range 2 .. 14
	 *    Typically, x will be 2 (or 3), but this function
	 *    does not require this assignment.
	 * 5) We assume that the transition into the
	 *    UN-cached area PMB[x] can be achieved by setting
	 *    bit #28 of the PC.  i.e. OR-ing it with 0x10000000.
	 *    so, PMB[Z].VPN | 0x10000000 == PMB[x].VPN
	 *    i.e. PMB[Z] has VA 0x80000000 ... 0x8fffffff
	 *         PMB[x] has VA 0x90000000 ... 0x9fffffff
	 * 6) We assume both PMB arrays are enabled, valid, and
	 *    will not result in an expection when dereferecned.
	 *    i.e. PMB[Z].V == PMB[x].V == 1
	 */
ENTRY(sh_toggle_pmb_cacheability)
	/* use PMB entry arrays #0, and #1 */
	.set index0, 0		/* PMB array #0 */
	.set index1, 1		/* PMB array #1 */

	/* set PC mask as bit #28 */
#if defined(CONFIG_SH_SE_MODE)
	.set mask, (CFG_SE_UNACHED_BASE-CFG_SDRAM_BASE)
#else
	.set mask, 0x10000000	/* 0x80000000 ^ 0x90000000 */
#endif

	/* set bits to toggle: PMB[n].C and PMB[n].UB */
	.set toggle, (SH4_PMB_C|SH4_PMB_UB)

	/* switch PC, to run from an UN-cached PMB region */
	MOV_CONST32_R0 mask	/* Mask */
	mov	r0, r2
	SWITCH_PMB or, r2	/* PC = PC | 0x10000000 */

	/* now toggle the cacheability of PMB[0] */
	MOV_CONST32_R0	(toggle)
	mov	r0, r3
	MOV_CONST32_R0	(P4SEG_PMB_DATA|(index0<<8))
	mov.l	@r0, r1		/* read it */
	xor	r3, r1		/* xor it */
	mov.l	r1, @r0		/* write it */

#if CFG_SH_LMI_NEEDS_2_PMB_ENTRIES
	/* now toggle the cacheability of PMB[1] */
	MOV_CONST32_R0	(P4SEG_PMB_DATA|(index1<<8))
	mov.l	@r0, r1		/* read it */
	xor	r3, r1		/* xor it */
	mov.l	r1, @r0		/* write it */
#endif

	/* Invalidate the ITLB, by setting MMUCR.TI */
	MOV_CONST32_R0 SH4_CCN_MMUCR
	mov.l	@r0, r1
	mov	#(SH4_MMUCR_TI), r3
	or	r3, r1
	mov.l	r1, @r0

	/* switch PC, to run from original PMB region */
	not	r2, r2		/* invert the mask */
	SWITCH_PMB and, r2	/* PC = PC & ~0x10000000 */

	/* return to the caller */
	rts
	  nop
#endif	/* CONFIG_SH_SE_MODE */

