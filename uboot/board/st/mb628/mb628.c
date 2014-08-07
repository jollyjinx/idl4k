/*
 * (C) Copyright 2008-2009 STMicroelectronics.
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <asm/soc.h>
#include <asm/stx7141reg.h>
#include <asm/io.h>
#include <asm/pio.h>


/* EPLD registers */
#ifdef CONFIG_SH_SE_MODE
#define EPLD_BASE		0xb5000000	/* Phys 0x05000000 */
#else
#define EPLD_BASE		0xa5000000
#endif	/* CONFIG_SH_SE_MODE */
#define EPLD_IDENT		0x00010000
#define EPLD_TEST		0x00020000
#define EPLD_FLASH		0x00050000


static inline void epld_write(unsigned long value, unsigned long offset)
{
	/* 8-bit write to EPLD registers */
	writeb(value, EPLD_BASE + offset);
}

static inline unsigned long epld_read(unsigned long offset)
{
	/* 8-bit read from EPLD registers */
	return readb(EPLD_BASE + offset);
}

extern void flashWriteEnable(void)
{
	/* Enable Vpp for writing to flash */
	epld_write(3, EPLD_FLASH);
}

extern void flashWriteDisable(void)
{
	/* Disable Vpp for writing to flash */
	epld_write(2, EPLD_FLASH);
}

static void configPIO(void)
{
	unsigned long sysconf;

	/* Setup PIO of ASC device */
	SET_PIO_ASC(PIO_PORT(10), 0, 1, 2, 3);  /* ASC1 */
	SET_PIO_ASC(PIO_PORT(6),  0, 1, 2, 3);  /* ASC2 */

	/* Enable ASC UARTS */
	sysconf = *STX7141_SYSCONF_SYS_CFG36;
	/* CFG36[29] = 0 = UART1_CTRL_NOT_MII_SEL */
	sysconf &= 1ul << 29;
	/* CFG36[30] = 1 = UART2_CTS_SEL */
	/* CFG36[31] = 1 = UART2_RXD_SEL */
	sysconf |= 1ul << 30 | 1ul << 31;
	*STX7141_SYSCONF_SYS_CFG36 = sysconf;

	/* Route ASC1 via PIO[10] for TX, RX, CTS & RTS */
	sysconf = *STX7141_SYSCONF_SYS_CFG46;
	/* PIO10[0] Selector: CFG46[7:6]   = 3 */
	/* PIO10[1] Selector: CFG46[9:8]   = 3 */
	/* PIO10[2] Selector: CFG46[11:10] = 3 */
	/* PIO10[3] Selector: CFG46[13:12] = 3 */
	sysconf |= 3ul << 6 | 3ul << 8 | 3ul << 10 | 3ul << 12;
	*STX7141_SYSCONF_SYS_CFG46 = sysconf;

	/* Route ASC2 via PIO[6] for TX & RX */
	sysconf = *STX7141_SYSCONF_SYS_CFG20;
	/* PIO6[0] Selector: CFG20[28:27] = 3 */
	/* PIO6[0] Selector: CFG20[30:29] = 3 */
	sysconf |= 3ul << 27 | 3ul << 29;
	*STX7141_SYSCONF_SYS_CFG20 = sysconf;

	/* Route ASC2 via PIO[6] for CTS & RTS */
	sysconf = *STX7141_SYSCONF_SYS_CFG25;
	/* PIO6[0] Selector: CFG25[1:0] = 3 */
	/* PIO6[0] Selector: CFG25[3:2] = 3 */
	sysconf |= 3ul << 0 | 3ul << 2;
	*STX7141_SYSCONF_SYS_CFG25 = sysconf;
}

extern int board_init(void)
{
	configPIO();

#if defined(CONFIG_SH_STM_SATA)
	stx7141_configure_sata ();
#endif	/* CONFIG_SH_STM_SATA */

	return 0;
}

extern int checkboard (void)
{
	unsigned version;

	printf ("\n\nBoard: STx7141-Mboard (MB628)"
#ifdef CONFIG_SH_SE_MODE
		"  [32-bit mode]"
#else
		"  [29-bit mode]"
#endif
		"\n");

	version = epld_read(EPLD_IDENT);
	printf("mb628 EPLD version %02d\n", version);
	return 0;
}
