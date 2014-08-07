/*
 * (C) Copyright 2008-2010 STMicroelectronics.
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
#include <asm/stx5206reg.h>
#include <asm/io.h>
#include <asm/pio.h>


void flashWriteEnable(void)
{
	/* Enable Vpp for writing to flash */
	/* Nothing to do! */
}

void flashWriteDisable(void)
{
	/* Disable Vpp for writing to flash */
	/* Nothing to do! */
}


#ifdef CONFIG_DRIVER_NET_STM_GMAC
extern void stmac_phy_reset(void)
{
	/*
	 * Reset the Ethernet PHY.
	 *
	 *	PIO2[2] = ETH_RESET
	 *
	 */
	STPIO_SET_PIN(PIO_PORT(2), 2, 0);
	udelay(15000);				/* 15 ms */
	STPIO_SET_PIN(PIO_PORT(2), 2, 1);
}
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

static void configPIO(void)
{
	unsigned long sysconf;

	/* Route the GPIO block to the PIOx pads, as by
	 * default the SATFE owns them... (why, oh why???) */
	sysconf = *STX5206_SYSCONF_SYS_CFG10;
	sysconf |= 1ul << 13;
	*STX5206_SYSCONF_SYS_CFG10 = sysconf;

#if CFG_STM_ASC_BASE == ST40_ASC2_REGS_BASE	/* UART2 = AS0 */
	/* Setup PIO of ASC device */
	SET_PIO_ASC(PIO_PORT(1), 2, 1, STPIO_NO_PIN, STPIO_NO_PIN);

	/* Route UART2 via PIO1 for TX, RX */
	sysconf = *STX5206_SYSCONF_SYS_CFG16;
	/* PIO1[1] CFG16[9,1]   AltFunction = 1 */
	/* PIO1[2] CFG16[10,2]  AltFunction = 1 */
	/* PIO1[3] CFG16[11,3]  AltFunction = 0 */
	/* PIO1[4] CFG16[12,4]  AltFunction = 0 */
	sysconf &= ~(0x0303ul<<1);	/* 0,0,3,3 */
	sysconf |=  (0x0000ul<<1);	/* 0,0,0,0 */
	*STX5206_SYSCONF_SYS_CFG16 = sysconf;
#else
#error Unknown ASC port selected!
#endif	/* CFG_STM_ASC_BASE */

#ifdef CONFIG_DRIVER_NET_STM_GMAC
	/*
	 * Configure the Ethernet PHY Reset signal
	 *	PIO2[2] = ETH_RESET
	 */
	SET_PIO_PIN(PIO_PORT(2), 2, STPIO_OUT);
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */
}

extern int board_init(void)
{
	configPIO();

#ifdef CONFIG_DRIVER_NET_STM_GMAC
	/* Reset the PHY */
	stmac_phy_reset();

	stx5206_configure_ethernet(stx5206_ethernet_mii, 0, 0);
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

	return 0;
}

int checkboard (void)
{
	printf ("\n\nBoard: STx5289-HDK"
#ifdef CONFIG_SH_SE_MODE
		"  [32-bit mode]"
#else
		"  [29-bit mode]"
#endif
		"\n");

	return 0;
}
