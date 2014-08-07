/*
 * Copyright (C) STMicroelectronics Ltd. 2002, 2003.
 *
 * andy.sturges@st.com
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

/*----------------------------------------------------------------------------*/

#ifndef __SH4REG_H
#define __SH4REG_H

#include "asm/sh4regtype.h"


/*----------------------------------------------------------------------------*/

/*
 * Core SH4 control registers
 */

/* Core control registers (common to all SH4 variants) */
#define SH4_CCN_PTEH	SH4_DWORD_REG(0xff000000)
#define SH4_CCN_PTEL	SH4_DWORD_REG(0xff000004)
#define SH4_CCN_TTB	SH4_DWORD_REG(0xff000008)
#define SH4_CCN_TEA	SH4_DWORD_REG(0xff00000c)
#define SH4_CCN_MMUCR	SH4_DWORD_REG(0xff000010)
#define SH4_CCN_BASRA	SH4_BYTE_REG(0xff000014)
#define SH4_CCN_BASRB	SH4_BYTE_REG(0xff000018)
#define SH4_CCN_CCR	SH4_DWORD_REG(0xff00001c)
#define SH4_CCN_TRA	SH4_DWORD_REG(0xff000020)
#define SH4_CCN_EXPEVT	SH4_DWORD_REG(0xff000024)
#define SH4_CCN_INTEVT	SH4_DWORD_REG(0xff000028)
#define SH4_CCN_PVR	SH4_DWORD_REG(0xff000030)
#define SH4_CCN_PTEA	SH4_DWORD_REG(0xff000034)
#define SH4_CCN_QACR0	SH4_DWORD_REG(0xff000038)
#define SH4_CCN_QACR1	SH4_DWORD_REG(0xff00003c)
#define SH4_CCN_CVR	SH4_DWORD_REG(0xff000040)
#define SH4_CCN_PRR	SH4_DWORD_REG(0xff000044)
#define SH4_CCN_PASCR	SH4_DWORD_REG(0xff000070)

/* Some bits in the CCN.CCR register */
#define SH4_CCR_OCE	(1<<0)
#define SH4_CCR_WT	(1<<1)
#define SH4_CCR_CB	(1<<2)
#define SH4_CCR_OCI	(1<<3)
#define SH4_CCR_ORA	(1<<5)
#define SH4_CCR_ICE	(1<<8)
#define SH4_CCR_ICI	(1<<11)
#define SH4_CCR_EMODE	(1<<31)

/* Some bits in the CCN.MMUCR register */
#define	SH4_MMUCR_TI	(1<<2)		/* MMUCR.TI */
#define	SH4_MMUCR_SE	(1<<4)		/* MMUCR.SE */

/* Some bits in the CCN.PASCR register */
#define	SH4_PASCR_SE	(1<<31)		/* PASCR.SE */


/* User Break Controller control registers (common to all SH4 variants) */
#define SH4_UBC_BARA	SH4_DWORD_REG(0xff200000)
#define SH4_UBC_BAMRA	SH4_BYTE_REG(0xff200004)
#define SH4_UBC_BBRA	SH4_WORD_REG(0xff200008)
#define SH4_UBC_BASRA	SH4_BYTE_REG(0xff000014)
#define SH4_UBC_BARB	SH4_DWORD_REG(0xff20000c)
#define SH4_UBC_BAMRB	SH4_BYTE_REG(0xff200010)
#define SH4_UBC_BBRB	SH4_WORD_REG(0xff200014)
#define SH4_UBC_BASRB	SH4_BYTE_REG(0xff000018)
#define SH4_UBC_BDRB	SH4_DWORD_REG(0xff200018)
#define SH4_UBC_BDMRB	SH4_DWORD_REG(0xff20001c)
#define SH4_UBC_BRCR	SH4_WORD_REG(0xff200020)

/* User Debug Interface control registers (common to all SH4 variants) */
#define SH4_UDI_SDIR	SH4_WORD_REG(0xfff00000)
#define SH4_UDI_SDDR	SH4_DWORD_REG(0xfff00008)
#define SH4_UDI_SDDRH	SH4_WORD_REG(0xfff00008)
#define SH4_UDI_SDDRL	SH4_WORD_REG(0xfff0000a)
#define SH4_UDI_SDINT	SH4_WORD_REG(0xfff00014)

/* Advanced User Debugger control registers (common to all SH4 variants) */
#define SH4_AUD_AUCSR	SH4_WORD_REG(0xff2000cc)
#define SH4_AUD_AUWASR	SH4_DWORD_REG(0xff2000d0)
#define SH4_AUD_AUWAER	SH4_DWORD_REG(0xff2000d4)
#define SH4_AUD_AUWBSR	SH4_DWORD_REG(0xff2000d8)
#define SH4_AUD_AUWBER	SH4_DWORD_REG(0xff2000dc)

/*
 * Generic SH4 control registers
 */

/* Timer Unit control registers (common to all SH4 variants) */
#define TOCR	SH4_BYTE_REG(SH4_TMU_REGS_BASE + 0x00)
#define TSTR	SH4_BYTE_REG(SH4_TMU_REGS_BASE + 0x04)
#define TCOR0	SH4_DWORD_REG(SH4_TMU_REGS_BASE + 0x08)
#define TCNT0	SH4_DWORD_REG(SH4_TMU_REGS_BASE + 0x0c)
#define TCR0	SH4_WORD_REG(SH4_TMU_REGS_BASE + 0x10)
#define TCOR1	SH4_DWORD_REG(SH4_TMU_REGS_BASE + 0x14)
#define TCNT1	SH4_DWORD_REG(SH4_TMU_REGS_BASE + 0x18)
#define TCR1	SH4_WORD_REG(SH4_TMU_REGS_BASE + 0x1c)
#define TCOR2	SH4_DWORD_REG(SH4_TMU_REGS_BASE + 0x20)
#define TCNT2	SH4_DWORD_REG(SH4_TMU_REGS_BASE + 0x24)
#define TCR2	SH4_WORD_REG(SH4_TMU_REGS_BASE + 0x28)
#define TCPR2	SH4_DWORD_REG(SH4_TMU_REGS_BASE + 0x2c)

/* Real Time Clock control registers (common to all SH4 variants) */
#define SH4_RTC_R64CNT	SH4_BYTE_REG(SH4_RTC_REGS_BASE + 0x00)
#define SH4_RTC_RSECCNT	SH4_BYTE_REG(SH4_RTC_REGS_BASE + 0x04)
#define SH4_RTC_RMINCNT	SH4_BYTE_REG(SH4_RTC_REGS_BASE + 0x08)
#define SH4_RTC_RHRCNT	SH4_BYTE_REG(SH4_RTC_REGS_BASE + 0x0c)
#define SH4_RTC_RWKCNT	SH4_BYTE_REG(SH4_RTC_REGS_BASE + 0x10)
#define SH4_RTC_RDAYCNT	SH4_BYTE_REG(SH4_RTC_REGS_BASE + 0x14)
#define SH4_RTC_RMONCNT	SH4_BYTE_REG(SH4_RTC_REGS_BASE + 0x18)
#define SH4_RTC_RYRCNT	SH4_WORD_REG(SH4_RTC_REGS_BASE + 0x1c)
#define SH4_RTC_RSECAR	SH4_BYTE_REG(SH4_RTC_REGS_BASE + 0x20)
#define SH4_RTC_RMINAR	SH4_BYTE_REG(SH4_RTC_REGS_BASE + 0x24)
#define SH4_RTC_RHRAR	SH4_BYTE_REG(SH4_RTC_REGS_BASE + 0x28)
#define SH4_RTC_RWKAR	SH4_BYTE_REG(SH4_RTC_REGS_BASE + 0x2c)
#define SH4_RTC_RDAYAR	SH4_BYTE_REG(SH4_RTC_REGS_BASE + 0x30)
#define SH4_RTC_RMONAR	SH4_BYTE_REG(SH4_RTC_REGS_BASE + 0x34)
#define SH4_RTC_RCR1	SH4_BYTE_REG(SH4_RTC_REGS_BASE + 0x38)
#define SH4_RTC_RCR2	SH4_BYTE_REG(SH4_RTC_REGS_BASE + 0x3c)

/*
 * Privileged Mapping Buffer (PMB) bit-field defintions
 */
#define	SH4_PMB_WT		(1<<0)	/* PMB[n].WT */
#define	SH4_PMB_C		(1<<3)	/* PMB[n].C */
#define	SH4_PMB_V		(1<<8)	/* PMB[n].V */
#define	SH4_PMB_UB		(1<<9)	/* PMB[n].UB */

/*----------------------------------------------------------------------------*/

#endif /* __SH4REG_H */
