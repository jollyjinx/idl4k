/*
 * (C) Copyright 2007 STMicroelectronics.
 *
 * Stuart Menefy <stuart.menefy@st.com>
 * Martin Lesniak <martin.lesniak@st.com> - added cb101 support
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
#include <asm/stx7200reg.h>
#include <asm/io.h>
#include <asm/pio.h>

void flashWriteEnable(void)
{
	/*  Enable vpp for writing to flash */
}

void flashWriteDisable(void)
{
	/*  Disable vpp for writing to flash */
}


#define PIO_BASE  0xfd020000


static void configPIO(void)
{
	/*  Setup PIO of ASC device */
	SET_PIO_ASC(PIO_PORT(4), 3, 2, 4, 5);  /* UART2 - AS0 */
	SET_PIO_ASC(PIO_PORT(5), 4, 3, 5, 6);  /* UART3 - AS1 */

#ifdef CONFIG_DRIVER_NETSTMAC
	/* Reset the on-board STe101P PHY */
	SET_PIO_PIN(PIO_PORT(4), 7, STPIO_OUT);
	STPIO_SET_PIN(PIO_PORT(4), 7, 1);
	udelay(1);
	STPIO_SET_PIN(PIO_PORT(4), 7, 0);
	udelay(1);
	STPIO_SET_PIN(PIO_PORT(4), 7, 1);
#endif	/* CONFIG_DRIVER_NETSTMAC */

#if defined(CONFIG_CMD_NAND)
	/*  Setup PIO for NAND FLASH devices: Ready/Not_Busy */
	SET_PIO_PIN(PIO_PORT(2), 7, STPIO_IN);
#endif	/* CONFIG_CMD_NAND */
}


#if defined(CONFIG_CMD_NAND)
static void nand_emi_init(void)
{
	/* setup the EMI configuration for the 2 banks
	 * containing NAND flash devices. */

	/* NAND FLASH in EMI Bank #1 (128MB) */
	*ST40_EMI_BANK1_EMICONFIGDATA0 = 0x04100e99;
	*ST40_EMI_BANK1_EMICONFIGDATA1 = 0x04000200;
	*ST40_EMI_BANK1_EMICONFIGDATA2 = 0x04000200;
	*ST40_EMI_BANK1_EMICONFIGDATA3 = 0x00000000;

	/* NAND FLASH in EMI Bank #2 (1GB) */
	*ST40_EMI_BANK2_EMICONFIGDATA0 = 0x04100e99;
	*ST40_EMI_BANK2_EMICONFIGDATA1 = 0x04000200;
	*ST40_EMI_BANK2_EMICONFIGDATA2 = 0x04000200;
	*ST40_EMI_BANK2_EMICONFIGDATA3 = 0x00000000;
}
#endif	/* CONFIG_CMD_NAND */


int board_init(void)
{
	unsigned long sysconf;

	/* Serial port set up */
	/* Route UART2&3 or SCI inputs instead of DVP to pins: conf_pad_dvp = 0 */
	sysconf = *STX7200_SYSCONF_SYS_CFG40;
	sysconf &= ~(1<<16);
	*STX7200_SYSCONF_SYS_CFG40 = sysconf;

	/* Route UART2&3/SCI outputs instead of DVP to pins: conf_pad_pio[1]=0 */
	sysconf = *STX7200_SYSCONF_SYS_CFG07;
	sysconf &= ~(1<<25);
	*STX7200_SYSCONF_SYS_CFG07 = sysconf;

	/* No idea, more routing: conf_pad_pio[0] = 0 */
	sysconf = *STX7200_SYSCONF_SYS_CFG07;
	sysconf &= ~(1<<24);
	*STX7200_SYSCONF_SYS_CFG07 = sysconf;

	/* Route UART2 (inputs and outputs) instead of SCI to pins: ssc2_mux_sel = 0 */
	sysconf = *STX7200_SYSCONF_SYS_CFG07;
	sysconf &= ~(1<<2);
	*STX7200_SYSCONF_SYS_CFG07 = sysconf;

	/* conf_pad_pio[4] = 0 */
	sysconf = *STX7200_SYSCONF_SYS_CFG07;
	sysconf &= ~(1<<28);
	*STX7200_SYSCONF_SYS_CFG07 = sysconf;

	/* Route UART3 (inputs and outputs) instead of SCI to pins: ssc3_mux_sel = 0 */
	sysconf = *STX7200_SYSCONF_SYS_CFG07;
	sysconf &= ~(1<<3);
	*STX7200_SYSCONF_SYS_CFG07 = sysconf;

	/* conf_pad_clkobs = 1 */
	sysconf = *STX7200_SYSCONF_SYS_CFG07;
	sysconf |= (1<<14);
	*STX7200_SYSCONF_SYS_CFG07 = sysconf;

	/* I2C and USB related routing */
	/* bit4: ssc4_mux_sel = 0 (treat SSC4 as I2C) */
	/* bit26: conf_pad_pio[2] = 0 route USB etc instead of DVO */
	/* bit27: conf_pad_pio[3] = 0 DVO output selection (probably ignored) */
	sysconf = *STX7200_SYSCONF_SYS_CFG07;
	sysconf &= ~((1<<27)|(1<<26)|(1<<4));
	*STX7200_SYSCONF_SYS_CFG07 = sysconf;

	/* Enable SOFT_JTAG mode.
	 * Taken from OS21, but is this correct?
	 */
	sysconf = *STX7200_SYSCONF_SYS_CFG33;
	sysconf |= (1<<6);
	sysconf &= ~((1<<0)|(1<<1)|(1<<2)|(1<<3));
	*STX7200_SYSCONF_SYS_CFG33 = sysconf;

#if defined(CONFIG_CMD_NAND)
	nand_emi_init();
#endif	/* CONFIG_CMD_NAND */

	configPIO();

	return 0;
}

int checkboard (void)
{
	printf ("\n\nBoard: cb101"
#ifdef CONFIG_SH_SE_MODE
		"  [32-bit mode]"
#else
		"  [29-bit mode]"
#endif
		"\n");

#ifdef CONFIG_DRIVER_NETSTMAC
#if defined(CONFIG_STMAC_MAC0)
	/* On-board PHY (MII0) in MII mode, using MII_CLK */
	stx7200_configure_ethernet(0, 0, 0, 0);
#endif	/* CONFIG_STMAC_MAC0 */
#endif	/* CONFIG_DRIVER_NETSTMAC */
	return 0;
}
