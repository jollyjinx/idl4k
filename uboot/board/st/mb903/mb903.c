/*
 * (C) Copyright 2008-2011 STMicroelectronics.
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
#include <asm/stx7108reg.h>
#include <asm/io.h>
#include <asm/pio.h>


void flashWriteEnable(void)
{
	/* Enable Vpp for writing to flash */
	/* Nothing to do. */
}

void flashWriteDisable(void)
{
	/* Disable Vpp for writing to flash */
	/* Nothing to do. */
}


#define PIOALT(port, pin, alt, dir)			\
do							\
{							\
	stx7108_pioalt_select((port), (pin), (alt));	\
	stx7108_pioalt_pad((port), (pin), (dir));	\
} while(0)

static void configPIO(void)
{
	/* Setup PIOs for ASC device */

#if CFG_STM_ASC_BASE == ST40_ASC3_REGS_BASE

	/*
	 * Route UART3 via PIO24/25 for TX, RX, RTS & CTS (Alternative #1).
	 *
	 * Note: on the MB903A, RS232_RTS is connected (incorrectly)
	 *       to PIO24[6], this is corrected in MB903B.
	 */
	PIOALT(24, 4, 1, &stx7108_pioalt_pad_out);	/* UART3-TX */
	PIOALT(24, 5, 1, &stx7108_pioalt_pad_in);	/* UART3-RX */
	PIOALT(24, 7, 1, &stx7108_pioalt_pad_out);	/* UART3-RTS */
	PIOALT(25, 0, 1, &stx7108_pioalt_pad_in);	/* UART3-CTS */

#else
#error Unknown ASC port selected!
#endif	/* CFG_STM_ASC_BASE == ST40_ASCx_REGS_BASE */

#ifdef CONFIG_DRIVER_NET_STM_GMAC
	/*
	 * Configure the Ethernet PHY Reset signal
	 *	PIO20[0] == GMII0_notRESET
	 *	PIO15[4] == GMII1_notRESET
	 */
	SET_PIO_PIN(ST40_PIO_BASE(15), 4, STPIO_OUT);
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */
}

#ifdef CONFIG_DRIVER_NET_STM_GMAC
extern void stmac_phy_reset(void)
{
	/*
	 * Reset the Ethernet PHY.
	 *	PIO20[0] == GMII0_notRESET
	 *	PIO15[4] == GMII1_notRESET
	 */
	STPIO_SET_PIN(ST40_PIO_BASE(15), 4, 0);
	udelay(10000);				/* 10 ms */
	STPIO_SET_PIN(ST40_PIO_BASE(15), 4, 1);
}
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */


extern int board_init(void)
{
	configPIO();

		/*
		 * Note each EMI bank can only be a maximum of 64MiB.
		 * set CSA (Bank #0) to be 64MiB (0x00000000 to 0x03ffffff)
		 * set CSB (Bank #1) to be 64MiB (0x04000000 to 0x07ffffff)
		 */
	*ST40_EMI_BASEADDRESS(0) =       (0) >> 22;	/* start of EMI Bank #0 */
	*ST40_EMI_BASEADDRESS(1) = ( 64<<20) >> 22;	/* start of EMI Bank #1 */
	*ST40_EMI_BASEADDRESS(2) = (128<<20) >> 22;	/* start of EMI Bank #2 */

#ifdef QQQ	/* QQQ - DELETE */
#if defined(CONFIG_SH_STM_SATA)
	stx7105_configure_sata ();
#endif	/* CONFIG_SH_STM_SATA */
#endif		/* QQQ - DELETE */

#ifdef CONFIG_DRIVER_NET_STM_GMAC
	/* Reset the PHY */
	stmac_phy_reset();
#if CFG_STM_STMAC_BASE == CFG_STM_STMAC0_BASE		/* MII0, via MoCA Module */
#error U-Boot does not support MoCA!
	stx7108_configure_ethernet(0, &(struct stx7108_ethernet_config) {
			.mode = stx7108_ethernet_mode_mii,
			.ext_clk = 1,
			.phy_bus = 0, });
#elif CFG_STM_STMAC_BASE == CFG_STM_STMAC1_BASE		/* MII1, IC+ IP1001 (U17) */
	stx7108_configure_ethernet(1, &(struct stx7108_ethernet_config) {
			.mode = stx7108_ethernet_mode_mii,
			.ext_clk = 0,
			.phy_bus = 1, });
#else
#error Unknown base address for the STM GMAC
#endif
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

#if defined(CONFIG_CMD_I2C)
	stx7108_configure_i2c();
#endif	/* CONFIG_CMD_I2C */

	return 0;
}

int checkboard (void)
{
	printf ("\n\nBoard: MB903 (STx7108M)"
#ifdef CONFIG_SH_SE_MODE
		"  [32-bit mode]"
#else
		"  [29-bit mode]"
#endif
		"\n");

#if defined(CONFIG_SOFT_SPI)
	/*
	 * Configure for the SPI Serial Flash.
	 * Note: for CFG_BOOT_FROM_SPI + CFG_ENV_IS_IN_EEPROM, this
	 * needs to be done after env_init(), hence it is done
	 * here, and not in board_init().
	 */
	stx7108_configure_spi();
#endif	/* CONFIG_SPI */

	return 0;
}
