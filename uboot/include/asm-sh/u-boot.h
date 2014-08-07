/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 * (C) Copyright 2004-2010 STMicroelectronics.
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

#ifndef _U_BOOT_H_
#define _U_BOOT_H_	1

typedef struct bd_info
{
	int bi_baudrate;	/* serial console baudrate */
	unsigned long bi_ip_addr;	/* IP Address */
	unsigned char bi_enetaddr[6];	/* Ethernet adress */
	unsigned long bi_arch_number;	/* unique id for this board */
	unsigned long bi_boot_params;	/* where this board expects params */
	unsigned long bi_memstart;	/* start of DRAM memory */
	unsigned long bi_memsize;	/* size  of DRAM memory in bytes */
#ifndef CFG_NO_FLASH
	unsigned long bi_flashstart;	/* start of FLASH memory */
	unsigned long bi_flashsize;	/* size  of FLASH memory */
	unsigned long bi_flashoffset;	/* reserved area for startup monitor */
#endif /* CFG_NO_FLASH */
#ifdef CONFIG_SH_STB7100
	unsigned long bi_devid;
	unsigned long bi_pll0frq;
	unsigned long bi_pll1frq;
	unsigned long bi_st40cpufrq;
	unsigned long bi_st40busfrq;
	unsigned long bi_st40perfrq;
	unsigned long bi_st231frq;
	unsigned long bi_stbusfrq;
	unsigned long bi_emifrq;
	unsigned long bi_lmifrq;
#endif
#if	defined(CONFIG_SH_STX5197) ||	\
	defined(CONFIG_SH_STX5206) ||	\
	defined(CONFIG_SH_STX7105) ||	\
	defined(CONFIG_SH_STX7108) ||	\
	defined(CONFIG_SH_STX7111) ||	\
	defined(CONFIG_SH_STX7141) ||	\
	defined(CONFIG_SH_STX7200) ||	\
	defined(CONFIG_SH_FLI7510) ||	\
	defined(CONFIG_SH_FLI7540)
	unsigned long bi_devid;
	unsigned long bi_emifrq;
#endif
} bd_t;
#define bi_env_data bi_env->data
#define bi_env_crc  bi_env->crc

#endif /* _U_BOOT_H_ */
