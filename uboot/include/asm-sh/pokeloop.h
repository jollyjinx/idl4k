/*
 * File     : pokeloop.h
 * Synopsis : Header containing macro definitions needed for poke tables and the
 *            poke loop code which uses them.
 *
 * Copyright (c) 2004-2010 STMicroelectronics Limited.  All rights reserved.
 */

#ifndef __POKE_LOOP_H_
#define __POKE_LOOP_H_

/* Opcode values */
#define OP_END_POKES					0
#define OP_POKE8					1
#define OP_POKE16					2
#define OP_POKE32					3
#define OP_OR8						4
#define OP_OR16						5
#define OP_OR32						6
#define OP_UPDATE8					7
#define OP_UPDATE16					8
#define OP_UPDATE32					9
#define OP_POKE_UPDATE32				10
#define OP_WHILE_NE8					11
#define OP_WHILE_NE16					12
#define OP_WHILE_NE32					13
#define OP_IF_EQ32					14
#define OP_IF_GT32					15
#define OP_ELSE						16
#define OP_DELAY					17
#define OP_IF_DEVID_GE					18
#define OP_IF_DEVID_LE					19

#ifdef __ASSEMBLER__

#ifdef __st200__
/* The ST200 Toolset has a version of GNU as which does not support the .warning
   or .error directives, so we switch in the .print directive instead to do the
   best we can.
 */
#define ASM_WARNING(STR)	.print "Warning:"; .print STR
#define ASM_ERROR(STR)		.print STR; .err
#else
#define ASM_WARNING(STR)	.warning STR
#define ASM_ERROR(STR)		.error STR
#endif /* __st200__ */

/* Poke table commands */
#define POKE8(A, VAL)					.long OP_POKE8, A, VAL
#define POKE16(A, VAL)					.long OP_POKE16, A, VAL
#define POKE32(A, VAL)					.long OP_POKE32, A, VAL
#define OR8(A, VAL)					.long OP_OR8, A, VAL
#define OR16(A, VAL)					.long OP_OR16, A, VAL
#define OR32(A, VAL)					.long OP_OR32, A, VAL
#define UPDATE8(A, AND, OR)				.long OP_UPDATE8, A, AND, OR
#define UPDATE16(A, AND, OR)				.long OP_UPDATE16, A, AND, OR
#define UPDATE32(A, AND, OR)				.long OP_UPDATE32, A, AND, OR
#define POKE_UPDATE32(A1, A2, AND, SHIFT, OR)		.long OP_POKE_UPDATE32, A1, A2, AND, SHIFT, OR
#define WHILE_NE8(A, AND, VAL)				.long OP_WHILE_NE8, A, AND, VAL; .if (VAL > 0xFF); ASM_ERROR("Value VAL in WHILE_NE8 should fit in 8 bits"); .endif
#define WHILE_NE16(A, AND, VAL)				.long OP_WHILE_NE16, A, AND, VAL; .if (VAL > 0xFFFF); ASM_ERROR("Value VAL in WHILE_NE16 should fit in 16 bits"); .endif
#define WHILE_NE32(A, AND, VAL)				.long OP_WHILE_NE32, A, AND, VAL
#define IF_EQ32(NESTLEVEL, A, AND, VAL)			.long OP_IF_EQ32, A, AND, VAL, (NESTLEVEL ## f - .)
#define IF_GT32(NESTLEVEL, A, AND, VAL)			.long OP_IF_GT32, A, AND, VAL, (NESTLEVEL ## f - .)
/* An explicit ELSE will skip the OP_ELSE embedded in the ENDIF to make things faster */
#define ELSE(NESTLEVEL)					.long OP_ELSE; NESTLEVEL: ; .long (NESTLEVEL ## f - .)
 /* ENDIF includes an OP_ELSE so that we end up at the correct position regardless of whether there is an explicit ELSE in the IF construct */
#define ENDIF(NESTLEVEL)				.long OP_ELSE; NESTLEVEL: ; .long 0
#define DELAY(ITERATIONS)				.long OP_DELAY, (ITERATIONS + 1)
/* The 2nd argument to the poke loop code (in R5 for ST40, or $r0.17 for ST200)
 * must be the device ID to compare against for these operations to work - the
 * poke loop code does not try to retrieve the device ID itself.
 */
#define IF_DEVID_GE(NESTLEVEL, VAL)			.long OP_IF_DEVID_GE, VAL, (NESTLEVEL ## f - .)
#define IF_DEVID_LE(NESTLEVEL, VAL)			.long OP_IF_DEVID_LE, VAL, (NESTLEVEL ## f - .)
/* The end marker needs two extra entries which get read by the code, but are 
   never used.
 */
#define END_MARKER					.long OP_END_POKES, 0, 0

/* 
 * For compatibility with old poke tables we define some of the old names.
 * Normally we warn about using the old names, but the warnings can be turned
 * off by defining the macro POKETABLE_NO_WARNINGS.
 */
#ifdef POKETABLE_NO_WARNINGS
	#define POKE_CHAR(A, VAL)	POKE8(A, VAL)
	#define POKE_SHORT(A, VAL)	POKE16(A, VAL)
	#define POKE_LONG(A, VAL)	POKE32(A, VAL)
	#define OR_LONG(A, VAL)		OR32(A, VAL)
	#define UPDATE_LONG(A, AND, OR)	UPDATE32(A, AND, OR)
	#define POKE_UPDATE_LONG(A1, A2, AND, SHIFT, OR) POKE_UPDATE32(A1, A2, AND, SHIFT, OR)
	#define WHILE_NE(A, AND, VAL)	WHILE_NE32(A, AND, VAL)
#else
	#define POKE_CHAR(A, VAL)	POKE8(A, VAL); ASM_WARNING("POKE_CHAR() is deprecated; use POKE8()")
	#define POKE_SHORT(A, VAL)	POKE16(A, VAL); ASM_WARNING("POKE_SHORT() is deprecated; use POKE16()")
	#define POKE_LONG(A, VAL)	POKE32(A, VAL); ASM_WARNING("POKE_LONG() is deprecated; use POKE32()")
	#define OR_LONG(A, VAL)		OR32(A, VAL); ASM_WARNING("OR_LONG() is deprecated; use OR32()")
	#define UPDATE_LONG(A, AND, OR)	UPDATE32(A, AND, OR); ASM_WARNING("UPDATE_LONG() is deprecated; use UPDATE32()")
	#define POKE_UPDATE_LONG(A1, A2, AND, SHIFT, OR) POKE_UPDATE32(A1, A2, AND, SHIFT, OR); ASM_WARNING("POKE_UPDATE_LONG() is deprecated; use POKE_UPDATE32()")
	#define WHILE_NE(A, AND, VAL)	WHILE_NE32(A, AND, VAL); ASM_WARNING("WHILE_NE() is deprecated; use WHILE_NE32()")
#endif /* !POKETABLE_NO_WARNINGS */

#endif /* __ASSEMBLER__ */

#endif /* !__POKE_LOOP_H_ */
