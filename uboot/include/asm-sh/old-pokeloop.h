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


#ifndef __OLD_POKE_LOOP_H__
#define __OLD_POKE_LOOP_H__

#ifdef __ASSEMBLER__

/*
 * This is derived from STMicroelectronics gnu toolchain example:
 *	sh-superh-elf/examples/os21/romdynamic/bootstrap.S
 * but it is not identical, because concurrently U-Boot added the
 * IF_DEVID, IF_NOT_DEVID, ELSE and ENDIF commands, while the toolset
 * added IF. This merged version supports both.
 */


/*
 * Old poke table commands
 */
#define POKE_UPDATE_LONG(A1, A2, AND, SHIFT, OR)	.long 8, A1, A2, AND, SHIFT, OR
#define WHILE_NE(A, AND, VAL)				.long 7, A, AND, VAL
#define UPDATE_LONG(A, AND, OR)				.long 6, A, AND, OR
#define OR_LONG(A, V)					.long 5, A, V
#define POKE_LONG(A, V)					.long 4, A, V
#define POKE_SHORT(A, V)				.long 2, A, V
#define POKE_CHAR(A, V)					.long 1, A, V
#define IF_DEVID(V)					.long 9,  (1f - .) -8, V
#define IF_NOT_DEVID(V)					.long 10, (1f - .) -8, V
#define ELSE						.long 11, (2f - .) - 4 ; 1:
#define ENDIF						2: ; 1:
#define IF(A, AND, VAL, COMMAND)			.long 12, A, AND, VAL; COMMAND
#define END_MARKER					.long 0, 0, 0


/*
 * For compatibility with old poke table code, we define some of the
 * new names, to map onto the old names.  Note, it is ill-advised
 * to rely on these mappings, as they are a strict sub-set of
 * the commands, and the latest romgen+targetpacks may start
 * to utilize some of the un-mapped ones!
 */
#define POKE8(A, VAL)				POKE_CHAR(A, VAL)
#define POKE16(A, VAL)				POKE_SHORT(A, VAL)
#define POKE32(A, VAL)				POKE_LONG(A, VAL)
#define OR32(A, VAL)				OR_LONG(A, VAL)
#define UPDATE32(A, AND, OR)			UPDATE_LONG(A, AND, OR)
#define POKE_UPDATE32(A1, A2, AND, SHIFT, OR)	POKE_UPDATE_LONG(A1, A2, AND, SHIFT, OR)
#define WHILE_NE32(A, AND, VAL)			WHILE_NE(A, AND, VAL)
#define DELAY(VAL)				/* do nothing */


/*
 * QQQ: should we move these to somewhere more sensible ?
 */
#define STB7100_CUT1 (STB7100_DEVID_7100_VAL << STB7100_DEVID_ID_SHIFT)
#define STB7100_CUT2 (STB7100_DEVID_7100_VAL << STB7100_DEVID_ID_SHIFT) | (1 << STB7100_DEVID_CUT_SHIFT)
#define STB7100_CUT3 (STB7100_DEVID_7100_VAL << STB7100_DEVID_ID_SHIFT) | (2 << STB7100_DEVID_CUT_SHIFT)
#define STB7109_CUT1 (STB7100_DEVID_7109_VAL << STB7100_DEVID_ID_SHIFT)
#define STB7109_CUT2 (STB7100_DEVID_7109_VAL << STB7100_DEVID_ID_SHIFT) | (1 << STB7100_DEVID_CUT_SHIFT)
#define STB7109_CUT3 (STB7100_DEVID_7109_VAL << STB7100_DEVID_ID_SHIFT) | (2 << STB7100_DEVID_CUT_SHIFT)
#define STX7200_CUT1 (STX7200_DEVID_7200c1_VAL << STX7200_DEVID_ID_SHIFT)
#define STX7200_CUT2 (STX7200_DEVID_7200c2_VAL << STX7200_DEVID_ID_SHIFT) | (1 << STX7200_DEVID_CUT_SHIFT)


#endif	/* __ASSEMBLER__ */

#endif	/* __OLD_POKE_LOOP_H__ */


