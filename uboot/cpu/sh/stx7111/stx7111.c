/*
 * (C) Copyright 2008-2009 STMicroelectronics.
 *
 * Stuart Menefy <stuart.menefy@st.com>
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
#include <asm/stbus.h>
#include <ata.h>
#include <spi.h>

#define PIO_BASE  ST40_PIO0_REGS_BASE

static void stx7111_clocks(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	bd_t *bd = gd->bd;

	/*
	 * FIXME
	 * Gross hack to get the serial port working.
	 * See the definition of PCLK in drivers/stm-asc.c
	 * for where this is used.
	 */
	bd->bi_emifrq = 100;
}

#ifdef CONFIG_DRIVER_NET_STM_GMAC

#define ETHERNET_INTERFACE_ON	(1ul<<16)
#define PHY_CLK_EXT		(1ul<<19)
#define MAC_SPEED_SEL           (1ul<<20)
#define PHY_INTF_SEL_MASK	(0x7ul<<24)
#define ENMII			(1ul<<27)

/* Remaining bits define pad functions, default appears to work */

extern int stmac_default_pbl(void)
{
	return 32;
}

extern void stmac_set_mac_speed(int speed)
{
	unsigned long sysconf = *STX7111_SYSCONF_SYS_CFG07;

	/* MAC_SPEED_SEL = 0|1 */
	if (speed == 100)
		sysconf |= MAC_SPEED_SEL;
	else if (speed == 10)
		sysconf &= ~MAC_SPEED_SEL;

	*STX7111_SYSCONF_SYS_CFG07 = sysconf;
}

/* ETH MAC pad configuration */
static void stmac_eth_hw_setup(void)
{
	const unsigned long en_mii  = 1;
	const unsigned long sel     = 0;
	const unsigned long ext_clk = 0;

	unsigned long sysconf = *STX7111_SYSCONF_SYS_CFG07;

	/* Ethernet ON */
	sysconf |= ETHERNET_INTERFACE_ON;

	/* PHY EXT CLOCK: 0: provided by STX7111; 1: external */
	if (ext_clk)
		sysconf |= PHY_CLK_EXT;
	else
		sysconf &= ~PHY_CLK_EXT;

	/* Default GMII/MII selection */
	sysconf &= ~PHY_INTF_SEL_MASK;
	sysconf |= ((sel<<24) & PHY_INTF_SEL_MASK);

	/* MII mode */
	if (en_mii)
		sysconf |= ENMII;
	else
		sysconf &= ~ENMII;

	*STX7111_SYSCONF_SYS_CFG07 = sysconf;
}
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

int soc_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	bd_t *bd = gd->bd;

	stx7111_clocks();

#ifdef CONFIG_DRIVER_NET_STM_GMAC
	stmac_eth_hw_setup();
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

	bd->bi_devid = *STX7111_SYSCONF_DEVICEID_0;

	/*  Make sure reset period is shorter than WDT timeout */
	*STX7111_SYSCONF_SYS_CFG09 = (*STX7111_SYSCONF_SYS_CFG09 & 0xFF000000) | 0x000A8C;

	return 0;
}


#if defined(CONFIG_USB_OHCI_NEW)
extern void stx7111_usb_init(void)
{
	unsigned long reg, req_reg;

	/* Power on the USB */
	reg = readl(STX7111_SYSCONF_SYS_CFG32);
	reg &= ~(1ul<<4); /* USB_POWER_DOWN_REQ = 0 */
	writel(reg, STX7111_SYSCONF_SYS_CFG32);

	/* Work around for USB over-current detection chip being
	 * active low, and the 7111 being active high.
	 * Note this is an undocumented bit, which apparently enables
	 * an inverter on the over-current signal.
	 */
	reg = readl(STX7111_SYSCONF_SYS_CFG06);
	reg |= 1ul<<29;
	writel(reg, STX7111_SYSCONF_SYS_CFG06);

	/* USB oc */
	SET_PIO_PIN(PIO_PORT(5), 6, STPIO_IN);
	/* USB power */
	SET_PIO_PIN(PIO_PORT(5), 7, STPIO_ALT_OUT);
	STPIO_SET_PIN(PIO_PORT(5), 7, 1);

	/* Set strap mode */
#define STRAP_MODE	AHB2STBUS_STRAP_16_BIT
	reg = readl(AHB2STBUS_STRAP);
#if STRAP_MODE == 0
	reg &= ~AHB2STBUS_STRAP_16_BIT;
#else
	reg |= STRAP_MODE;
#endif
	writel(reg, AHB2STBUS_STRAP);

	/* Start PLL */
	reg = readl(AHB2STBUS_STRAP);
	writel(reg | AHB2STBUS_STRAP_PLL, AHB2STBUS_STRAP);
	udelay(100000);	/* QQQ: can this delay be shorter ? */
	writel(reg & (~AHB2STBUS_STRAP_PLL), AHB2STBUS_STRAP);
	udelay(100000);	/* QQQ: can this delay be shorter ? */

	req_reg =
		(1<<21) |  /* Turn on read-ahead */
		(5<<16) |  /* Opcode is store/load 32 */
		(0<<15) |  /* Turn off write posting */
		(1<<14) |  /* Enable threshold */
		(3<<9)  |  /* 2**3 Packets in a chunk */
		(0<<4)  |  /* No messages */
		(8<<0);    /* Threshold is 256 */

	do {
		writel(req_reg, AHB2STBUS_STBUS_CONFIG);
		reg = readl(AHB2STBUS_STBUS_CONFIG);
	} while ((reg & 0x7FFFFFFF) != req_reg);
}

#endif /* defined(CONFIG_USB_OHCI_NEW) */


/**********************************************************************/


#if defined(CONFIG_SPI)

#if defined(CONFIG_SOFT_SPI)			/* Use "bit-banging" for SPI */
extern void stx7111_spi_scl(const int val)
{
	const int pin = 0;	/* PIO2[0] = SPI_CLK */
	STPIO_SET_PIN(PIO_PORT(2), pin, val ? 1 : 0);
}

extern void stx7111_spi_sda(const int val)
{
	const int pin = 1;	/* PIO2[1] = SPI_DOUT */
	STPIO_SET_PIN(PIO_PORT(2), pin, val ? 1 : 0);
}

extern unsigned char stx7111_spi_read(void)
{
	const int pin = 2;	/* PIO2[2] = SPI_DIN */
	return STPIO_GET_PIN(PIO_PORT(2), pin);
}
#elif defined(CONFIG_STM_SSC_SPI)		/* Use the H/W SSC for SPI */
#error Still to impliment SPI via SSC for the STx7111.
#endif	/* CONFIG_SOFT_SPI */

/*
 * assert or de-assert the SPI Chip Select line.
 *
 *	input: cs == true, assert CS, else deassert CS
 *
 * this is used for both S/W bit-banging, and H/W SSC.
 */
static void spi_chip_select(const int cs)
{
	const int pin = 7;	/* PIO6[7] = SPI_NOTCS */

	if (cs)
	{	/* assert SPI CSn */
		STPIO_SET_PIN(PIO_PORT(6), pin, 0);
	}
	else
	{	/* DE-assert SPI CSn */
		STPIO_SET_PIN(PIO_PORT(6), pin, 1);
	}

	if (cs)
	{	/* wait 250ns for CSn assert to propagate  */
		udelay(1);	/* QQQ: can we make this shorter ? */
	}
}

/*
 * The SPI command uses this table of functions for controlling the SPI
 * chip selects: it calls the appropriate function to control the SPI
 * chip selects.
 */
spi_chipsel_type spi_chipsel[] =
{
	spi_chip_select
};
int spi_chipsel_cnt = sizeof(spi_chipsel) / sizeof(spi_chipsel[0]);

#endif	/* CONFIG_SPI */


