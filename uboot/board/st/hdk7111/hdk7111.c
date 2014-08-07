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
#include <asm/stx7111reg.h>
#include <asm/io.h>
#include <asm/pio.h>


void flashWriteEnable(void)
{
	/* Enable Vpp for writing to flash */
	/* Vpp is tied HIGH, so nothing to be done */
}

void flashWriteDisable(void)
{
	/* Disable Vpp for writing to flash */
	/* Vpp is tied HIGH, so nothing can be done */
}

static void configEthernet(void)
{
	/* Setup PIO for the PHY's reset */
	SET_PIO_PIN(PIO_PORT(1),6,STPIO_OUT);	/* PHY_RES is on PIO1[6] */

	/* Finally, just toggle the PHY Reset pin */
	STPIO_SET_PIN(PIO_PORT(1), 6, 0);	/* Assert PHY_RES */
	udelay(100);				/* small delay (100us) */
	STPIO_SET_PIN(PIO_PORT(1), 6, 1);	/* de-assert PHY_RES */
}

#if defined(CONFIG_SPI)
static void configSpi(void)
{
#if defined(CONFIG_SOFT_SPI)
	/* Configure SPI Serial Flash for PIO "bit-banging" */

	/* SPI is on PIO2[2:0], with CS on PIO6[7] */
	SET_PIO_PIN(PIO_PORT(2),0,STPIO_OUT);	/* SPI_CLK */
	SET_PIO_PIN(PIO_PORT(2),1,STPIO_OUT);	/* SPI_DOUT */
	SET_PIO_PIN(PIO_PORT(2),2,STPIO_IN);	/* SPI_DIN */
	SET_PIO_PIN(PIO_PORT(6),7,STPIO_OUT);	/* SPI_NOTCS */

	/* drive outputs with sensible initial values */
	STPIO_SET_PIN(PIO_PORT(6), 7, 1);	/* deassert SPI_NOCS */
	STPIO_SET_PIN(PIO_PORT(2), 0, 1);	/* assert SPI_CLK */
	STPIO_SET_PIN(PIO_PORT(2), 1, 0);	/* deassert SPI_DOUT */
#elif defined(CONFIG_STM_SSC_SPI)		/* Use the H/W SSC for SPI */
#error Still to impliment SPI via SSC for the STx7111.
#endif	/* CONFIG_SOFT_SPI */
}
#endif	/* CONFIG_SPI */

static void configPIO(void)
{
	/* Setup PIO of ASC device */
#if CFG_STM_ASC_BASE == ST40_ASC2_REGS_BASE	/* UART #2 */
	SET_PIO_ASC(PIO_PORT(4), 3, 2, 4, 5);	/* UART2 - AS0 */
#else
#error Unsure which UART to configure!
#endif	/* CFG_STM_ASC_BASE == ST40_ASC2_REGS_BASE */

	/* Configure & Reset the Ethernet PHY */
	configEthernet();

#if defined(CONFIG_SPI)
	/* Configure for SPI Serial Flash */
	configSpi();
#endif	/* CONFIG_SPI */
}

extern int board_init(void)
{
	configPIO();

	return 0;
}

int checkboard (void)
{
	printf ("\n\nBoard: STx7111-HDK"
#ifdef CONFIG_SH_SE_MODE
		"  [32-bit mode]"
#else
		"  [29-bit mode]"
#endif
		"\n");

	return 0;
}
