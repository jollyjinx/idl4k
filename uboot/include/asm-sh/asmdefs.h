/*
 * (C) Copyright 2004, 2007, 2009, 2011 STMicroelectronics.
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

/* Macros to support assembly programing */


	/* build up a 32-bit constant into R0 */
.macro MOV_CONST32_R0 p1:req
	mov	#((\p1)>>24), r0
	shll8	r0
	.ifne    (((\p1) >> 16)&0xFF)
	or	#(((\p1) >> 16)&0xFF), r0
	.endif
	shll8	r0
	.ifne    (((\p1) >> 8) &0xFF)
	or	#(((\p1) >> 8) &0xFF), r0
	.endif
	shll8	r0
	.ifne    ((\p1) &0xFF)
	or	#((\p1) &0xFF), r0
	.endif
.endm

	/* build up a 16-bit constant into R0 */
.macro MOV_CONST16_R0 p1:req
	mov	#(((\p1) >> 8) &0xFF), r0
	shll8	r0
	.ifne    ((\p1) &0xFF)
	or	#((\p1) &0xFF), r0
	.endif
.endm

	/* unset top 3 bits of PC */
.macro ENTER_P0
	mova	1f, r0
	mov	#0xE0, r1
	shll16	r1
	shll8	r1
	not	r1, r1		/* MASK is 0x1fffffff */
	and	r1, r0		/* unset top 3-bits */
	jmp	@r0
	  nop
.balign 4
1:
.endm

	/* OR 0x80 into top byte of PC */
.macro ENTER_P1
	mova	1f, r0
	mov	#0xE0, r1
	shll16	r1
	shll8	r1
	not	r1, r1		/* MASK is 0x1fffffff */
	and	r1, r0		/* unset top 3-bits */
	mov	#0x80, r1
	shll16	r1
	shll8	r1		/* MASK is 0x80000000 */
	or	r1, r0		/* put PC in P1 */
	jmp	@r0
	  nop
.balign 4
1:
.endm

	/* OR 0xA0 into top byte of PC */
.macro ENTER_P2
	mova	1f, r0
	mov	#0xE0, r1
	shll16	r1
	shll8	r1
	not	r1, r1		/* MASK is 0x1fffffff */
	and	r1, r0		/* unset top 3-bits */
	mov	#0xA0, r1
	shll16	r1
	shll8	r1		/* MASK is 0xA0000000 */
	or	r1, r0		/* put PC in P2 */
	jmp	@r0
	  nop
.balign 4
1:
.endm

	/* call a routine in another file */
.macro CALL p1:req
	mova	\p1, r0
	mov.l	@r0, r1
	add	r1, r0
	jsr	@r0
	  nop
.endm

	/* put device id in p1 */
.macro GETDEVID p1:req
#ifdef CONFIG_SH_STB7100
	mov.l	1f, r0
	mov.l	@r0, \p1
	mov.l	2f, r0
	bra	3f
	  and	r0, \p1
	.balign 4
1:	.long STB7100_SYSCONF_DEVICEID_0
2:	.long (STB7100_DEVID_ID_MASK << STB7100_DEVID_ID_SHIFT) | (STB7100_DEVID_CUT_MASK << STB7100_DEVID_CUT_SHIFT)
3:
#endif
.endm

/* Enable a single PMB entry
Note: This macro will clobber both r0 and r1.
Usage: SH4_SET_PMB <index> <virtual> <physical> <size> [<cache>=1 [<wt>=0 [<ub>=0]]]]
where <index> is the PMB entry
	<virtual> is the virtual page number	(required)
	<physical> is the physical page number	(required)
	<size> is the page size in MBytes	(required)
	<cache> is optional and is the page cacheability (default: 1 [on])
	<wt> is optional and is the page cache mode (default: 0 [copy-back])
	<ub> is optional and is the page buffer mode (default: 0 [buffered]) */
.macro SH4_SET_PMB i:req, vpn:req, ppn:req, size:req, cache=1, wt=0, ub=0
	.set pmbdata, 0

	.if (\size==16)		/* PMB[n].SZ */
		.set pmbdata, pmbdata|0x00000000
	.elseif (\size==64)
		.set pmbdata, pmbdata|0x00000010
	.elseif (\size==128)
		.set pmbdata, pmbdata|0x00000080
	.elseif (\size==512)
		.set pmbdata, pmbdata|0x00000090
	.else
		.error "Unsupported page SIZE for a PMB entry"
	.endif

	.if (\cache)		/* PMB[n].C */
		.set pmbdata, pmbdata|(SH4_PMB_C)
	.endif

	.if (\wt)		/* PMB[n].WT */
		.set pmbdata, pmbdata|(SH4_PMB_WT)
	.endif

	.if (\ub)		/* PMB[n].UB */
		.set pmbdata, pmbdata|(SH4_PMB_UB)
	.endif

	.if ( (\vpn<0x80) || (\vpn>=0xc0) )
		.error "Invalid Virtual Page Number for PMB entry"
	.endif

	.if (\i>15)
		.error "Invalid Index for PMB entry"
	.endif

		/* poke ADDR_ARRAY entry */
	MOV_CONST32_R0	((\vpn)<<24)
	mov	r0,r1
	MOV_CONST32_R0	(P4SEG_PMB_ADDR | ((\i)<<8))
	mov.l	r1,@r0

		/* poke DATA_ARRAY entry */
	MOV_CONST32_R0	(((\ppn)<<24) | SH4_PMB_V | pmbdata)
	mov	r0,r1
	MOV_CONST32_R0	(P4SEG_PMB_DATA | ((\i)<<8))
	mov.l	r1,@r0
.endm

/*
 * Write out a series of bytes, monotonically increasing
 * in value from "first" to "last" (inclusive).
 *
 * Usage:	BYTES <first> <last>
 * where <first> is the first byte to generate
 * where <last>  is the last byte to generate
 *
 * Note: this macro uses recursion (one level per byte)
 */
.macro BYTES first=0, last=63
	.byte \first
	.if \last-\first
	BYTES "(\first+1)",\last	/* note: recursion */
	.endif
.endm

