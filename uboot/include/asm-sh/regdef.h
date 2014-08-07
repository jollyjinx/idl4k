/*
 * include/asm-sh/regdefs.h
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004, STMicroelectronics
 */

#ifndef __ASM_SH_REGDEF_H
#define __ASM_SH_REGDEF_H

/*
 * Symbolic register names for 32 bit ABI
 */

#define v0      r0		/* return value */
#define v1      r1
#define v2      r2
#define v3      r3
#define a0      r4		/* argument registers */
#define a1      r5
#define a2      r6
#define a3      r7
#define s0      r8		/* callee saved */
#define s1      r9
#define s2      r10
#define s3      r11
#define gp      r12		/* global pointer */
#define s4      r13
#define fp      r14		/* frame pointer */
#define sp      r15		/* stack pointer */

#endif /* __ASM_SH_REGDEF_H */
