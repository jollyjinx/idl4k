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
#include <asm/stx7141reg.h>
#include <asm/io.h>
#include <asm/pio.h>


extern void flashWriteEnable(void)
{
	/* Enable Vpp for writing to flash */
	/* Nothing to do! */
}

extern void flashWriteDisable(void)
{
	/* Disable Vpp for writing to flash */
	/* Nothing to do! */
}

#ifdef CONFIG_DRIVER_NET_STM_GMAC
extern void stmac_phy_reset(void)
{
#if CFG_STM_STMAC_BASE == CFG_STM_STMAC1_BASE	/* MAC = STM GMAC#1 */
	/* Reset the on-board IC+ IP1001 PHY (U51) */
	SET_PIO_PIN(PIO_PORT(5), 3, STPIO_OUT);

	STPIO_SET_PIN(PIO_PORT(5), 3, 1);
	udelay(10);

	STPIO_SET_PIN(PIO_PORT(5), 3, 0);
	udelay(50);

	STPIO_SET_PIN(PIO_PORT(5), 3, 1);
	udelay(10);
#endif
}
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

static void configPIO(void)
{
	unsigned long sysconf;

	/* Setup PIO of ASC device */
	SET_PIO_ASC(PIO_PORT(1), 0, 1, STPIO_NO_PIN, STPIO_NO_PIN);  /* ASC2 */

	/* Enable ASC UART */
	sysconf = *STX7141_SYSCONF_SYS_CFG36;
	/* CFG36[31] = 0 = UART2_RXD_SEL = PIO1[1] */
	sysconf &= ~(1ul << 31);
	*STX7141_SYSCONF_SYS_CFG36 = sysconf;

	/* Route ASC2 via PIO[1] for TX & RX */
	sysconf = *STX7141_SYSCONF_SYS_CFG19;
	/* PIO1[0] Selector: CFG19[1:0] = 3 TX */
	/* PIO1[1] Selector: CFG19[3:2] = 3 RX */
	sysconf |= 3ul << 0 | 3ul << 2;
	*STX7141_SYSCONF_SYS_CFG19 = sysconf;
}

extern int board_init(void)
{
	configPIO();

#ifdef CONFIG_DRIVER_NET_STM_GMAC
	stmac_phy_reset();	/* Reset the PHY */
#if CFG_STM_STMAC_BASE == CFG_STM_STMAC0_BASE	/* MAC = STM GMAC#0 */
	/* QQQ TODO: No H/W provided to test this! */
	stx7141_configure_ethernet(0, 0, 0, 0);
#elif CFG_STM_STMAC_BASE == CFG_STM_STMAC1_BASE	/* MAC = STM GMAC#1 */
	stx7141_configure_ethernet(1, 0, 0, 1);
#else
#error Unknown GMAC Base address encountered!
#endif
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

#if defined(CONFIG_SH_STM_SATA)
	stx7141_configure_sata ();
#endif	/* CONFIG_SH_STM_SATA */

	return 0;
}

extern int checkboard (void)
{
	printf ("\n\nBoard: STx7141-EUD"
#ifdef CONFIG_SH_SE_MODE
		"  [32-bit mode]"
#else
		"  [29-bit mode]"
#endif
		"\n");

	return 0;
}
