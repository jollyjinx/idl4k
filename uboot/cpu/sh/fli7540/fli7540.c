/*
 * (C) Copyright 2008-2010 STMicroelectronics.
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
#include <asm/fli7540reg.h>
#include <asm/io.h>
#include <asm/pio.h>
#include <asm/stbus.h>
#include <asm/sysconf.h>
#include <ata.h>
#include <spi.h>


#define ARRAY_SIZE(x)		(sizeof(x) / sizeof((x)[0]))


#define USB_XTAL_VALID		1	/* CFG_SPARE_1[1]            = USB_xtal_valid */

#define USB_POWERDOWN_REQ	8	/* CFG_COMMS_CONFIG_1[8 ]    = usb_powerdown_req */
#define CONF_PIO24_ALTERNATE	17	/* CFG_COMMS_CONFIG_1[18:17] = conf_pio24_alternate */

#define GMAC_MII_ENABLE		8	/* CFG_COMMS_CONFIG_2[8]     = gmac_mii_enable */
#define GMAC_PHY_CLOCK_SEL	9	/* CFG_COMMS_CONFIG_2[9]     = gmac_phy_clock_sel */
#define GMAC_ENABLE		24	/* CFG_COMMS_CONFIG_2[24]    = gmac_enable */
#define GMAC_MAC_SPEED		25	/* CFG_COMMS_CONFIG_2[25]    = gmac_mac_speed */
#define PHY_INTF_SEL		26	/* CFG_COMMS_CONFIG_2[28:26] = phy_intf_sel */


static void fli7540_clocks(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	bd_t *bd = gd->bd;

	/*
	 * FIXME
	 * Gross hack to get the serial port working.
	 * See the defintion of PCLK in drivers/stm-asc.c
	 * for where this is used.
	 */
	bd->bi_emifrq = 100;	/* comms_clk = 100 MHz */
}


#ifdef CONFIG_DRIVER_NET_STM_GMAC

struct fli7540_gmac_pin {
	unsigned char port, pin, dir;
};

static struct fli7540_gmac_pin fli7540_gmac_mii_pins[] = {
	{ 18, 5, },			/* PHYCLK */
	{ 18, 0, STPIO_ALT_OUT },	/* MDC */
	{ 18, 1, STPIO_IN },		/* COL */
	{ 18, 2, STPIO_IN },		/* CRS */
	{ 18, 3, STPIO_IN },		/* MDINT */
	{ 18, 4, STPIO_ALT_BIDIR },	/* MDIO */
	{ 20, 0, STPIO_ALT_OUT },	/* TXD[0] */
	{ 20, 1, STPIO_ALT_OUT },	/* TXD[1] */
	{ 20, 2, STPIO_ALT_OUT },	/* TXD[2] */
	{ 20, 3, STPIO_ALT_OUT },	/* TXD[3] */
	{ 20, 4, STPIO_ALT_OUT },	/* TXEN */
	{ 20, 5, STPIO_IN },		/* TXCLK */
	{ 21, 0, STPIO_IN },		/* RXD[0] */
	{ 21, 1, STPIO_IN },		/* RXD[1] */
	{ 21, 2, STPIO_IN },		/* RXD[2] */
	{ 21, 3, STPIO_IN },		/* RXD[3] */
	{ 21, 4, STPIO_IN },		/* RXDV */
	{ 21, 5, STPIO_IN },		/* RX_ER */
	{ 21, 6, STPIO_IN },		/* RXCLK */
};

static struct fli7540_gmac_pin fli7540_gmac_gmii_pins[] = {
	{ 18, 5, },			/* PHYCLK */
	{ 18, 0, STPIO_ALT_OUT },	/* MDC */
	{ 18, 1, STPIO_IN },		/* COL */
	{ 18, 2, STPIO_IN },		/* CRS */
	{ 18, 3, STPIO_IN },		/* MDINT */
	{ 18, 4, STPIO_ALT_BIDIR },	/* MDIO */
	{ 20, 0, STPIO_ALT_OUT },	/* TXD[0] */
	{ 20, 1, STPIO_ALT_OUT },	/* TXD[1] */
	{ 20, 2, STPIO_ALT_OUT },	/* TXD[2] */
	{ 20, 3, STPIO_ALT_OUT },	/* TXD[3] */
	{ 24, 4, STPIO_ALT_OUT },	/* TXD[4] */
	{ 24, 5, STPIO_ALT_OUT },	/* TXD[5] */
	{ 24, 6, STPIO_ALT_OUT },	/* TXD[6] */
	{ 24, 7, STPIO_ALT_OUT },	/* TXD[7] */
	{ 20, 4, STPIO_ALT_OUT },	/* TXEN */
	{ 20, 5, STPIO_IN },		/* TXCLK */
	{ 21, 0, STPIO_IN },		/* RXD[0] */
	{ 21, 1, STPIO_IN },		/* RXD[1] */
	{ 21, 2, STPIO_IN },		/* RXD[2] */
	{ 21, 3, STPIO_IN },		/* RXD[3] */
	{ 24, 0, STPIO_IN },		/* RXD[4] */
	{ 24, 1, STPIO_IN },		/* RXD[5] */
	{ 24, 2, STPIO_IN },		/* RXD[6] */
	{ 24, 3, STPIO_IN },		/* RXD[7] */
	{ 21, 4, STPIO_IN },		/* RXDV */
	{ 21, 5, STPIO_IN },		/* RX_ER */
	{ 21, 6, STPIO_IN },		/* RXCLK */
};

static struct fli7540_gmac_pin fli7540_gmac_rmii_pins[] = {
	{ 18, 5, },			/* PHYCLK */
	{ 18, 0, STPIO_ALT_OUT },	/* MDC */
	{ 18, 3, STPIO_IN },		/* MDINT */
	{ 18, 4, STPIO_ALT_BIDIR },	/* MDIO */
	{ 20, 0, STPIO_ALT_OUT },	/* TXD[0] */
	{ 20, 1, STPIO_ALT_OUT },	/* TXD[1] */
	{ 20, 4, STPIO_ALT_OUT },	/* TXEN */
	{ 21, 0, STPIO_IN },		/* RXD[0] */
	{ 21, 1, STPIO_IN },		/* RXD[1] */
	{ 21, 4, STPIO_IN },		/* RXDV */
	{ 21, 5, STPIO_IN },		/* RX_ER */
};

static struct fli7540_gmac_pin fli7540_gmac_reverse_mii_pins[] = {
	{ 18, 5, },			/* PHYCLK */
	{ 18, 0, STPIO_IN },		/* MDC */
	{ 18, 1, STPIO_ALT_OUT },	/* COL */
	{ 18, 2, STPIO_ALT_OUT },	/* CRS */
	{ 18, 3, STPIO_IN },		/* MDINT */
	{ 18, 4, STPIO_ALT_BIDIR },	/* MDIO */
	{ 20, 0, STPIO_ALT_OUT },	/* TXD[0] */
	{ 20, 1, STPIO_ALT_OUT },	/* TXD[1] */
	{ 20, 2, STPIO_ALT_OUT },	/* TXD[2] */
	{ 20, 3, STPIO_ALT_OUT },	/* TXD[3] */
	{ 20, 4, STPIO_ALT_OUT },	/* TXEN */
	{ 20, 5, STPIO_IN },		/* TXCLK */
	{ 21, 0, STPIO_IN },		/* RXD[0] */
	{ 21, 1, STPIO_IN },		/* RXD[1] */
	{ 21, 2, STPIO_IN },		/* RXD[2] */
	{ 21, 3, STPIO_IN },		/* RXD[3] */
	{ 21, 4, STPIO_IN },		/* RXDV */
	{ 21, 5, STPIO_IN },		/* RX_ER */
	{ 21, 6, STPIO_IN },		/* RXCLK */
};

extern int stmac_default_pbl(void)
{
	return 32;
}

extern void stmac_set_mac_speed(const int speed)
{
	unsigned long sysconf;

	/* CFG_COMMS_CONFIG_2[25] = gmac_mac_speed */
	/* gmac_mac_speed = speed==100 ? 1 : 0 */
	sysconf = readl(CFG_COMMS_CONFIG_2);
	SET_SYSCONF_BIT(sysconf, speed==100, GMAC_MAC_SPEED);
	writel(sysconf, CFG_COMMS_CONFIG_2);
}


	/*
	 * ETH GMAC PIO configuration
	 */
extern void fli7540_configure_ethernet(
	const enum fli7540_ethernet_mode mode,
	const int ext_clk,
	const int phy_bus)
{
	struct fli7540_gmac_pin *pins;
	int pins_num;
	unsigned char phy_sel, enmii;
	int i;
	unsigned long sysconf;

	/* Ethernet interface on */
	/* CFG_COMMS_CONFIG_2[24] = gmac_enable */
	sysconf = readl(CFG_COMMS_CONFIG_2);
	SET_SYSCONF_BIT(sysconf, 1, GMAC_ENABLE);
	writel(sysconf, CFG_COMMS_CONFIG_2);

	switch (mode) {
	case fli7540_ethernet_mii:
		phy_sel = 0;
		enmii = 1;
		pins = fli7540_gmac_mii_pins;
		pins_num = ARRAY_SIZE(fli7540_gmac_mii_pins);
		break;
	case fli7540_ethernet_rmii:
		phy_sel = 4;
		enmii = 1;
		pins = fli7540_gmac_rmii_pins;
		pins_num = ARRAY_SIZE(fli7540_gmac_rmii_pins);
		break;
	case fli7540_ethernet_gmii:
		phy_sel = 0;
		enmii = 1;
		pins = fli7540_gmac_gmii_pins;
		pins_num = ARRAY_SIZE(fli7540_gmac_gmii_pins);
		/* CFG_COMMS_CONFIG_1[18:17] = conf_pio24_alternate */
		sysconf = readl(CFG_COMMS_CONFIG_1);
		sysconf &= ~(0x3ul<<CONF_PIO24_ALTERNATE);
		sysconf |= (0x2ul<<CONF_PIO24_ALTERNATE);
		writel(sysconf, CFG_COMMS_CONFIG_1);
		break;
	case fli7540_ethernet_reverse_mii:
		phy_sel = 0;
		enmii = 0;
		pins = fli7540_gmac_reverse_mii_pins;
		pins_num = ARRAY_SIZE(fli7540_gmac_reverse_mii_pins);
		break;
	default:
		BUG();
		return;
	}

	/* CFG_COMMS_CONFIG_2[28:26] = phy_intf_sel */
	sysconf = readl(CFG_COMMS_CONFIG_2);
	sysconf &= ~(0x7ul<<PHY_INTF_SEL);
	sysconf |= (phy_sel<<PHY_INTF_SEL);
//	writel(sysconf, CFG_COMMS_CONFIG_2);

	/* CFG_COMMS_CONFIG_2[8]     = gmac_mii_enable */
//	sysconf = readl(CFG_COMMS_CONFIG_2);
	SET_SYSCONF_BIT(sysconf, enmii, GMAC_MII_ENABLE);
//	writel(sysconf, CFG_COMMS_CONFIG_2);

	/* CFG_COMMS_CONFIG_2[9]     = gmac_phy_clock_sel */
//	sysconf = readl(CFG_COMMS_CONFIG_2);
	SET_SYSCONF_BIT(sysconf, ext_clk, GMAC_PHY_CLOCK_SEL);
	writel(sysconf, CFG_COMMS_CONFIG_2);

	/* choose the correct direction for PHYCLK */
	pins[0].dir = (ext_clk) ? STPIO_IN : STPIO_ALT_OUT;

	/* set all the PIOs correctly */
	for (i = 0; i < pins_num; i++)
	{
		SET_PIO_PIN(ST40_PIO_BASE(pins[i].port), pins[i].pin, pins[i].dir);
	}
}
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */


#if defined(CONFIG_USB_OHCI_NEW)
extern void fli7540_usb_init(
	const int port,
	const enum fli7540_usb_ovrcur_mode ovrcur_mode)
{
	static int xtal_initialized;
	int clken, rstn;
	int override, ovrcur, polarity;
	unsigned long sysconf;

	if (!xtal_initialized++)
	{
		sysconf = readl(CFG_SPARE_1);
#if defined(CONFIG_SH_FLI7510)
		SET_SYSCONF_BIT(sysconf, 0, USB_XTAL_VALID);
#else
		SET_SYSCONF_BIT(sysconf, 1, USB_XTAL_VALID);
#endif	/* CONFIG_SH_FLI7510 */
		writel(sysconf, CFG_SPARE_1);
	}

	switch (port) {
	case 0:
		clken = 21;
		rstn = 23;
		override = 12;
		ovrcur = 13;
		polarity = 11;
#if defined(CONFIG_SH_FLI7510)
		SET_PIO_PIN(ST40_PIO_BASE(27), 1, STPIO_IN);		/* USB_A_OVRCUR */
		SET_PIO_PIN(ST40_PIO_BASE(27), 2, STPIO_ALT_OUT);	/* USB_A_PWREN */
#else
		SET_PIO_PIN(ST40_PIO_BASE(26), 3, STPIO_IN);		/* USB_A_OVRCUR */
		SET_PIO_PIN(ST40_PIO_BASE(26), 4, STPIO_ALT_OUT);	/* USB_A_PWREN */
#endif	/* CONFIG_SH_FLI7510 */

		break;
	case 1:
		clken = 22;
		rstn = 24;
		override = 15;
		ovrcur = 16;
		polarity = 14;
		SET_PIO_PIN(ST40_PIO_BASE(26), 5, STPIO_IN);		/* USB_C_OVRCUR */
		SET_PIO_PIN(ST40_PIO_BASE(26), 6, STPIO_ALT_OUT);	/* USB_C_PWREN */
		break;
	default:
		BUG();
		return;
	}

#if !defined(CONFIG_SH_FLI7510)
	sysconf = readl(CFG_COMMS_CONFIG_1);
		/* CFG_COMMS_CONFIG_1[22,21] = conf_usb_clk_en */
	SET_SYSCONF_BIT(sysconf, 1, clken);
		/* CFG_COMMS_CONFIG_1[24,23] = conf_usb_rst_n */
	SET_SYSCONF_BIT(sysconf, 1, rstn);
	writel(sysconf, CFG_COMMS_CONFIG_1);
#endif	/* CONFIG_SH_FLI7510 */

switch (ovrcur_mode) {
	case fli7540_usb_ovrcur_disabled:
		/* CFG_COMMS_CONFIG_1[12,15] = usbX_enable_pad_override */
		sysconf = readl(CFG_COMMS_CONFIG_1);
		SET_SYSCONF_BIT(sysconf, 1, override);
		writel(sysconf, CFG_COMMS_CONFIG_1);

		/* CFG_COMMS_CONFIG_1[13,16] = usbX_ovrcur */
		sysconf = readl(CFG_COMMS_CONFIG_1);
		SET_SYSCONF_BIT(sysconf, 1, ovrcur);
		writel(sysconf, CFG_COMMS_CONFIG_1);
		break;
	default:
		/* CFG_COMMS_CONFIG_1[12,15] = usbX_enable_pad_override */
		sysconf = readl(CFG_COMMS_CONFIG_1);
		SET_SYSCONF_BIT(sysconf, 0, override);
		writel(sysconf, CFG_COMMS_CONFIG_1);

		/* CFG_COMMS_CONFIG_1[11,14] = usbX_ovrcur_polarity */
		switch (ovrcur_mode) {
		case fli7540_usb_ovrcur_active_high:
			sysconf = readl(CFG_COMMS_CONFIG_1);
			SET_SYSCONF_BIT(sysconf, 0, polarity);
			writel(sysconf, CFG_COMMS_CONFIG_1);
			break;
		case fli7540_usb_ovrcur_active_low:
			sysconf = readl(CFG_COMMS_CONFIG_1);
			SET_SYSCONF_BIT(sysconf, 1, polarity);
			writel(sysconf, CFG_COMMS_CONFIG_1);
			break;
		default:
			BUG();
			break;
		}
		break;
	}

	/* start the USB Wrapper Host Controller */
	ST40_start_host_control(
		USB_FLAGS_STRAP_8BIT		|
		USB_FLAGS_STRAP_PLL		|
		USB_FLAGS_STBUS_CONFIG_THRESHOLD128);
}
#endif /* defined(CONFIG_USB_OHCI_NEW) */


/**********************************************************************/


#if defined(CONFIG_SPI)

#if defined(CONFIG_SOFT_SPI)			/* Use "bit-banging" for SPI */
extern void fli7540_spi_scl(const int val)
{
	const int pin = 2;	/* PIO21[2] = SPI_CLK */
	STPIO_SET_PIN(PIO_PORT(21), pin, val ? 1 : 0);
}

extern void fli7540_spi_sda(const int val)
{
	const int pin = 3;	/* PIO21[3] = SPI_MOSI */
	STPIO_SET_PIN(PIO_PORT(21), pin, val ? 1 : 0);
}

extern unsigned char fli7540_spi_read(void)
{
	const int pin = 5;	/* PIO20[5] = SPI_MISO */
	return STPIO_GET_PIN(PIO_PORT(20), pin);
}
#endif	/* CONFIG_SOFT_SPI */

/*
 * assert or de-assert the SPI Chip Select line.
 *
 *	input: cs == true, assert CS, else deassert CS
 */
static void spi_chip_select(const int cs)
{
	const int pin = 2;	/* PIO20[2] = SPI_CSN */

	if (cs)
	{	/* assert SPI CSn */
		STPIO_SET_PIN(PIO_PORT(20), pin, 0);
	}
	else
	{	/* DE-assert SPI CSn */
		STPIO_SET_PIN(PIO_PORT(20), pin, 1);
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


/**********************************************************************/


extern int soc_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	bd_t *bd = gd->bd;

	fli7540_clocks();

	/* obtain the chip cut + device id */
	bd->bi_devid = readl(CFG_DEVICE_ID);

	return 0;
}

