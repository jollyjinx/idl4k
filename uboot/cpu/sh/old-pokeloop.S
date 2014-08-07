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


/*
 *	NOTE:	 ALL THIS CODE MUST BE PIC !!!!
 *
 *	This code expects to be run with the caches enabled.
 */


/*
 * This is derived from STMicroelectronics gnu toolchain example:
 *	sh-superh-elf/examples/os21/romdynamic/bootstrap.S
 * but it is not identical, because concurrently U-Boot added the
 * IF_DEVID, IF_NOT_DEVID, ELSE and ENDIF commands, while the toolset
 * added IF. This merged version supports both.
 */

/*
 * The poke table is a series of long words, in the format
 *
 *	opcode, address, operand, ...
 *
 * An opcode of 0 marks the table end
 */

/*
 * Routine to setup peripheral registers.
 * It is executed from within the I-cache,
 * with all its data in the D-cache
 */
poke_loop:
	mov.l	@r1+, r0	/* opcode */
	mov.l	@r1+, r2	/* address */
	mov.l	@r1+, r3	/* value */

	cmp/eq	#0, r0		/* End marker ? */
	bf	1f

	rts			/* Return point */
	  nop

1:	cmp/eq	#4, r0		/* 4 byte write... */
	bf	1f
	mov.l	r3, @r2

1:	cmp/eq	#2, r0		/* 2 byte write... */
	bf	1f
	mov.w	r3, @r2

1:	cmp/eq	#1, r0		/* 1 byte write... */
	bf	1f
	mov.b	r3, @r2

1:	cmp/eq	#5, r0		/* 4 byte OR... */
	bf	1f
	mov.l	@r2,r4
	or	r3,r4
	mov.l	r4,@r2

1:	cmp/eq	#6, r0		/* 4 byte UPDATE... */
	bf	1f
	mov.l	@r2,r4
	and	r3,r4
	mov.l	@r1+,r3
	or	r3,r4
	mov.l	r4,@r2

1:	cmp/eq	#8, r0		/* 4 byte write UPDATE... */
	bf	1f
	mov.l	@r3,r4
	mov.l	@r1+,r3
	and	r3,r4
	mov.l	@r1+,r3
	shld	r3,r4
	mov.l	@r1+,r3
	or	r3,r4
	mov.l	r4,@r2

1:	cmp/eq	#7, r0		/* WHILE != ... */
	bf	1f
	mov.l	@r1+,r5
2:	mov.l	@r2,r4
	and	r3,r4
	cmp/eq	r4,r5
	bf	2b

1:	cmp/eq	#9, r0		/* IF_DEVID */
	bf	1f		/* r2 skip offset, r3 condition, r7 holds out cut value */
	cmp/eq	r3, r7
	bt	poke_loop	/* go ahead with these pokes */
	add	r2, r1
	bra	poke_loop
	  nop

1:	cmp/eq	#10, r0		/* IF_NOT_DEVID */
	bf	1f		/* r2 skip offset, r3 condition, r7 holds out cut value */
	cmp/eq	r3, r7
	bf	poke_loop	/* go ahead with these pokes */
	add	r2, r1
	bra	poke_loop
	  nop

1:	cmp/eq	#11, r0		/* ELSE */
	bf	1f		/* r2 skip offset, r3 condition, r7 holds out cut value */
	add	r2, r1
	bra	poke_loop
	  nop

1:	cmp/eq	#12,r0		/* IF == ... next op */
	bf	delay
	mov.l	@r1+,r5
	mov.l	@r2,r4
	and	r3,r4
	cmp/eq	r4,r5
	bt	poke_loop	/* Compare succeeded - perform next op */
2:				/* Skip the next operation (read past it) */
	mov.l	@r1+,r0		/* R0 = opcode */
	mov.l	@r1+,r2		/* skip address */
	mov.l	@r1+,r2		/* skip value */
				/* How many further reads do we need to skip? */
	cmp/eq	#12,r0		/* If it's another IF, skip 1 and go back to start of skip loop */
	bf	3f
	mov.l	@r1+,r2
	bra	2b
	  nop

3:	mov	#5,r2
	cmp/gt	r2,r0
	bf	5f		/* 0 further reads */

	cmp/eq	#8,r0		/* Is it number 8 (3 reads, otherwise 1 read) */
	bf	4f
	mov.l	@r1+,r2		/* Skip 1 read */
	mov.l	@r1+,r2		/* Skip 1 read */
4:	mov.l	@r1+,r2		/* Skip 1 read and continue */
5:	bra	poke_loop
	  nop

delay:	mov	#1,r0		/* small delay after EACH opcode */
	swap.w	r0, r0		/* 0x10000 iterations (~65k) */
2:	add	#-1,r0
	cmp/eq	#0, r0
	bf	2b
	bt	poke_loop
