/*
 * File     : pokeloop.S
 * Synopsis : ST40 poke loop code.
 *
 * Copyright (c) 2004-2010 STMicroelectronics Limited.  All rights reserved.
 *
 */


/*
 * The poke table is a series of long words, in the format:
 *
 *	opcode, operand, operand (, operand)*
 *
 * An opcode of 0 marks the table end.
 *
 * MUST_RUN_FROM_ICACHE must be defined to a non-zero value if this code is to
 * be run from a 32 bytes per line instruction cache (it will jump through
 * itself once before starting to ensure all lines are cached).  If it is not
 * defined at all, it will be defined to 1.
 */
#ifndef MUST_RUN_FROM_ICACHE
#define MUST_RUN_FROM_ICACHE 1
#endif /* !MUST_RUN_FROM_ICACHE */

/*
 * Routine to setup peripheral registers.
 * It is executed from within the I-cache (if MUST_RUN_FROM_ICACHE is defined),
 * with all its data in the D-cache.  It is the caller's responsibility to
 * ensure the data is all in the D-cache.
 *
 * This code and the poke table should be put in the same unique section to
 * ensure they are contiguous in memory.  This will ensure that, as long as the
 * data cache is large enough, the SwitchJumpTable and the poke table will not
 * be on the same cache line (guaranteeing functionality on a direct-mapped
 * cache.
 *
 * The poke loop code honours the architecture ABI for parameter passing, with
 * the address of the poke table to process as the first argument, and a device
 * ID to use in the IF_DEVID operations as the second argument.
 * That is:
 *		extern void poke_loop(const long table[], int device_id);
 *
 *	with R4 being the address of the poke table,
 *	and R5 containing the DEVICE_ID.
 */


	.balign	32
poke_loop:
	/* Ensure the jump table is in the data cache */
	mova	SwitchJumpTable, r0	/* Keep this in r0 for use in DoPoking */
	mov	r0, r6
	pref	@r6			/* 2 prefetches as could cover 2 cache lines */
	add	#32, r6
	pref	@r6

#if (MUST_RUN_FROM_ICACHE != 0)
	/* Do a single code jump-through to pull the code into the instruction
	 * cache - we have put a jump point in every cache line.  One way to do
         * this is to use "sh4objdump -d" on an executable containing this code,
         * and visually inspect that each jump-through point is in a separate
         * 0x20 range.
	 * We set the T bit so we can use 1 instruction jumps for the
	 * jump-through.
	 */
	sett
	bt	8f
#endif /* MUST_RUN_FROM_ICACHE != 0 */

DoPoking:
	mov.l	@r4+, r7	/* opcode */
	mov.l	@r4+, r1	/* operand 1 */
	mov.l	@r4+, r2	/* operand 2 */

	mov.b	@(r0, r7), r7
	extu.b	r7, r7
	braf	r7
	  nop
SwitchJumpFrom:

/* The SwitchJumpTable must be in increasing numeric order of opcode (with
 * padding for any missing entries).  Distance between SwitchJumpTable and any
 * of the operations must be less than 255 bytes (the assembler should point it
 * out if we ever break that condition and have to switch to 16 bit values).
 */
	.balign 4
SwitchJumpTable:
	.byte	DoRet - SwitchJumpFrom
	.byte	DoPoke8 - SwitchJumpFrom
	.byte	DoPoke16 - SwitchJumpFrom
	.byte	DoPoke32 - SwitchJumpFrom
	.byte	DoOr8 - SwitchJumpFrom
	.byte	DoOr16 - SwitchJumpFrom
	.byte	DoOr32 - SwitchJumpFrom
	.byte	DoUpdate8 - SwitchJumpFrom
	.byte	DoUpdate16 - SwitchJumpFrom
	.byte	DoUpdate32 - SwitchJumpFrom
	.byte	DoPokeUpdate32 - SwitchJumpFrom
	.byte	DoWhileNe8 - SwitchJumpFrom
	.byte	DoWhileNe16 - SwitchJumpFrom
	.byte	DoWhileNe32 - SwitchJumpFrom
	.byte	DoIfEq32 - SwitchJumpFrom
	.byte	DoIfGT32 - SwitchJumpFrom
	.byte	DoElse - SwitchJumpFrom
	.byte	DoDelay - SwitchJumpFrom
	.byte	DoIfDevIDGE - SwitchJumpFrom
	.byte	DoIfDevIDLE - SwitchJumpFrom

	.balign	2
#if (MUST_RUN_FROM_ICACHE != 0)
8:	bt	8f 
#endif /* MUST_RUN_FROM_ICACHE != 0 */

/* END_MARKER */
DoRet:
	rts			/* Return point */
	  nop

/* POKE8(A, VAL) */
DoPoke8:
	bra	DoPoking
	  mov.b	r2, @r1		/* *A = VAL */

/* POKE16(A, VAL) */
DoPoke16:
	bra	DoPoking
	  mov.w	r2, @r1		/* *A = VAL */

/* POKE32(A, VAL) */
DoPoke32:
	bra	DoPoking
	  mov.l	r2, @r1		/* *A = VAL */

#if (MUST_RUN_FROM_ICACHE != 0)
8:	bt	8f
#endif /* MUST_RUN_FROM_ICACHE != 0 */

/* OR8(A, VAL) */
DoOr8:
	mov.b	@r1, r3		/* *A */
	or	r2, r3		/* *A | OR */
	bra	DoPoking
	  mov.b	r3, @r1		/* *A |= OR */

/* OR16(A, VAL) */
DoOr16:
	mov.w	@r1, r3		/* *A */
	or	r2, r3		/* *A | OR */
	bra	DoPoking
	  mov.w	r3, @r1		/* *A |= OR */

/* OR32(A, VAL) */
DoOr32:
	mov.l	@r1, r3		/* *A */
	or	r2, r3		/* *A | OR */
	bra	DoPoking
	  mov.l	r3, @r1		/* *A |= OR */

/* UPDATE8(A, AND, OR) */
DoUpdate8:
	mov.b	@r1, r3		/* *A */
	and	r2, r3		/* *A & AND */
	mov.l	@r4+, r2	/* read OR */
	or	r2, r3		/* (*A & AND) | OR */
	bra	DoPoking
	  mov.b	r3, @r1		/* *A = ((*A & AND) | OR) */

#if (MUST_RUN_FROM_ICACHE != 0)
8:	bt	8f 
#endif /* MUST_RUN_FROM_ICACHE != 0 */

/* UPDATE16(A, AND, OR) */
DoUpdate16:
	mov.w	@r1, r3		/* *A */
	and	r2, r3		/* *A & AND */
	mov.l	@r4+, r2	/* read OR */
	or	r2, r3		/* (*A & AND) | OR */
	bra	DoPoking
	  mov.w	r3, @r1		/* *A = ((*A & AND) | OR) */

/* UPDATE32(A, AND, OR) */
DoUpdate32:
	mov.l	@r1, r3		/* *A */
	and	r2, r3		/* *A & AND */
	mov.l	@r4+, r2	/* read OR */
	or	r2, r3		/* (*A & AND) | OR */
	bra	DoPoking
	  mov.l	r3, @r1		/* *A = ((*A & AND) | OR) */

#if (MUST_RUN_FROM_ICACHE != 0)
8:	bt	8f
#endif /* MUST_RUN_FROM_ICACHE != 0 */

/* POKE_UPDATE32(A1, A2, AND, SHIFT, OR) */
DoPokeUpdate32:
	mov.l	@r2, r3		/* *A2 */
	mov.l	@r4+, r2	/* read AND */
	and	r2, r3		/* *A2 & AND */
	mov.l	@r4+, r2	/* read SHIFT */
	shld	r2, r3		/* (*A2 & AND) << SHIFT */
	mov.l	@r4+, r2	/* read OR */
	or	r2, r3		/* ((*A2 & AND) << SHIFT) | OR */
	bra	DoPoking
	  mov.l	r3, @r1		/* *A1 = (((*A2 & AND) << SHIFT) | OR) */

/* WHILE_NE8(A, AND, VAL) */
DoWhileNe8:
	mov.l	@r4+, r7	/* read VAL */
1:	mov.b	@r1, r3		/* *A */
	extu.b	r3, r3		/* 32bit(*A) */ 
	and	r2, r3		/* *A & AND */    
	cmp/eq	r3, r7		/* if ((*A & AND) == VAL) */
	bf	1b		/* loop if false */
	bt	DoPoking

#if (MUST_RUN_FROM_ICACHE != 0)
8:	bt	8f
#endif /* MUST_RUN_FROM_ICACHE != 0 */

/* WHILE_NE16(A, AND, VAL) */
DoWhileNe16:
	mov.l	@r4+, r7	/* read VAL */
1:	mov.w	@r1, r3		/* *A */
	extu.w	r3, r3		/* 32bit(*A) */ 
	and	r2, r3		/* *A & AND */    
	cmp/eq	r3, r7		/* if ((*A & AND) == VAL) */
	bf	1b		/* loop if false */
	bt	DoPoking

/* WHILE_NE32(A, AND, VAL) */
DoWhileNe32:
	mov.l	@r4+, r7	/* read VAL */
1:	mov.l	@r1, r3		/* *A */
	and	r2, r3		/* *A & AND */
	cmp/eq	r3, r7		/* if ((*A & AND) == VAL) */
	bf	1b		/* loop if false */
	bt	DoPoking

#if (MUST_RUN_FROM_ICACHE != 0)
8:	bt	8f
#endif /* MUST_RUN_FROM_ICACHE != 0 */

/* IF_EQ32(NESTLEVEL, A, AND, VAL)
   Note that NESTLEVEL is not in the actual table, but there is a distance
   field following VAL.
 */
DoIfEq32:
	mov.l	@r1, r1		/* *A */
	and	r2, r1		/* *A & AND */
	mov.l	@r4+, r2	/* read VAL */
	mov.l	@r4+, r3	/* read distance to ELSE/ENDIF */
	cmp/eq	r2, r1		/* if ((*A & AND) == VAL) */
	bt	DoPoking	/* go ahead with these pokes */
	add	r3, r4		/* skip forward through pokes to ELSE or ENDIF */
	bf	DoPoking

/* IF_GT32(NESTLEVEL, A, AND, VAL)
   Note that NESTLEVEL is not in the actual table, but there is a distance
   field following VAL.
 */
DoIfGT32:
	mov.l	@r1, r1		/* *A */
	and	r2, r1		/* *A & AND */
	mov.l	@r4+, r2	/* read VAL */
	mov.l	@r4+, r3	/* read distance to ELSE/ENDIF */
	cmp/hi	r2, r1		/* if ((*A & AND) > VAL) */
	bt	DoPoking	/* go ahead with these pokes if true*/
	add	r3, r4		/* skip forward through pokes to ELSE or ENDIF */
	bf	DoPoking

#if (MUST_RUN_FROM_ICACHE != 0)
8:	bt	8f
#endif /* MUST_RUN_FROM_ICACHE != 0 */

/* ELSE(NESTLEVEL)
   Note that NESTLEVEL is not in the actual table, but there is a distance
   field following the opcode.
 */
DoElse:
	add	#-4, r4		/* We took 1 arg too many from r4 for an else */
	bra	DoPoking
	  add	r1, r4		/* skip through to ENDIF */

/* DELAY(ITERATIONS) */
DoDelay:
1:	dt	r1		/* if (--OPERATIONS == 0) */
	bf	1b
	add	#-4, r4		/* We took 1 arg too many from r4 for a delay */
	bt	DoPoking

/* IF_DEVID_GE(NESTLEVEL, VAL)
   Note that NESTLEVEL is not in the actual table, but there is a distance
   field following VAL.
 */
DoIfDevIDGE:
	cmp/hs	r1, r5		/* if (device ID >= VAL) */
	bt	DoPoking	/* go ahead with these pokes if true */
	bra	DoPoking
	  add	r2, r4

/* IF_DEVID_LE(NESTLEVEL, VAL)
   Note that NESTLEVEL is not in the actual table, but there is a distance
   field following VAL.
 */
DoIfDevIDLE:
	cmp/hi	r5, r1		/* if (device ID <= VAL) */
	bt	1f		/* go ahead with these pokes if true */
	add	r2, r4
1:	bra	DoPoking
	  nop

#if (MUST_RUN_FROM_ICACHE != 0)
8:	/* The final jump back to where we start performing the pokes */
	bra	DoPoking
	  nop
#endif /* MUST_RUN_FROM_ICACHE != 0 */

