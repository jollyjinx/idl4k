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
#include <asm/stx7105reg.h>
#include <asm/io.h>
#include <asm/pio.h>
#include <asm/stbus.h>
#include <ata.h>
#include <spi.h>


static void stx7105_clocks(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	bd_t *bd = gd->bd;

	/*
	 * FIXME
	 * Gross hack to get the serial port working.
	 * See the defintion of PCLK in drivers/stm-asc.c
	 * for where this is used.
	 */
	bd->bi_emifrq = 100;
}

#ifdef CONFIG_DRIVER_NET_STM_GMAC

#define ETHERNET_INTERFACE_ON	(1ul<<16)
#define EXT_MDIO		(1ul<<17)
#define RMII_MODE		(1ul<<18)
#define PHY_CLK_EXT		(1ul<<19)
#define MAC_SPEED_SEL           (1ul<<20)
#define PHY_INTF_SEL_MASK	(0x3ul<<25)
#define ENMII			(1ul<<27)
/* Remaining bits define pad functions, default appears to work */

extern int stmac_default_pbl(void)
{
	return 8;
}

extern void stmac_set_mac_speed(int speed)
{
	unsigned long sysconf = *STX7105_SYSCONF_SYS_CFG07;

	/* MAC_SPEED_SEL = 0|1 */
	if (speed == 100)
		sysconf |= MAC_SPEED_SEL;
	else if (speed == 10)
		sysconf &= ~MAC_SPEED_SEL;

	*STX7105_SYSCONF_SYS_CFG07 = sysconf;
}

/* ETH MAC pad configuration */
static void stmac_eth_hw_setup( int reverse_mii, int rmii_mode, int mode,
				int ext_mdio, int ext_clk, int phy_bus)
{
	unsigned long sysconf;

	sysconf = *STX7105_SYSCONF_SYS_CFG07;
	/* Ethernet ON */
	sysconf |= (ETHERNET_INTERFACE_ON);
	/* MII M-DIO select: 1: miim_dio from external input, 0: from GMAC */
	if (ext_mdio)
		sysconf |= (EXT_MDIO);
	else
		sysconf &= ~(EXT_MDIO);
	/* RMII pin multiplexing: 0: MII interface active, 1: RMII interface */
	/* cut 1: This register was not connected, so only MII available */
	if (rmii_mode)
		sysconf |= (RMII_MODE);
	else
		sysconf &= ~(RMII_MODE);
	/*
	 * PHY EXT CLOCK: 0: provided by STx7105; 1: external
	 * cut 1: sysconf7[19], however this was not connected, so only
	 * input supported.
	 * cut 2: direction now based on PIO direction, so this code removed.
	 */
	/* Default GMII/MII selection */
	sysconf &= ~(PHY_INTF_SEL_MASK);
	sysconf |= ((mode&3ul)<<25);
	/* MII mode */
	if (reverse_mii)
		sysconf &= ~(ENMII);
	else
		sysconf |= (ENMII);
	*STX7105_SYSCONF_SYS_CFG07 = sysconf;

	/* Pin configuration... */

	/* PIO7[4] CFG37[8+4,4] = Alternate1 = MIIRX_DV/MII_EXCRS */
	/* PIO7[5] CFG37[8+5,5] = Alternate1 = MIIRX_ER/MII_EXCOL */
	/* PIO7[6] CFG37[8+6,6] = Alternate1 = MIITXD[0] */
	/* PIO7[7] CFG37[8+7,7] = Alternate1 = MIITXD[1] */
	sysconf = *STX7105_SYSCONF_SYS_CFG37;
	sysconf &= ~(0xf0f0ul);	/* Mask=3,3,3,3 */
	sysconf |=   0x0000ul;	/* OR  =0,0,0,0 */
	*STX7105_SYSCONF_SYS_CFG37 = sysconf;
	SET_PIO_PIN(PIO_PORT(7), 4, STPIO_IN);
	SET_PIO_PIN(PIO_PORT(7), 5, STPIO_IN);
	SET_PIO_PIN(PIO_PORT(7), 6, STPIO_ALT_OUT);
	SET_PIO_PIN(PIO_PORT(7), 7, STPIO_ALT_OUT);

	/* PIO8[0] CFG46[8+0,0] = Alternate1 = MIITXD[2] */
	/* PIO8[1] CFG46[8+1,1] = Alternate1 = MIITXD[3] */
	/* PIO8[2] CFG46[8+2,2] = Alternate1 = MIITX_EN */
	/* PIO8[3] CFG46[8+3,3] = Alternate1 = MIIMDIO */
	/* PIO8[4] CFG46[8+4,4] = Alternate1 = MIIMDC */
	/* PIO8[5] CFG46[8+5,5] = Alternate1 = MIIRXCLK */
	/* PIO8[6] CFG46[8+6,6] = Alternate1 = MIIRXD[0] */
	/* PIO8[7] CFG46[8+7,7] = Alternate1 = MIIRXD[1] */
	sysconf = *STX7105_SYSCONF_SYS_CFG46;
	sysconf &= ~(0xfffful);	/* Mask=3,3,3,3,3,3,3,3 */
	sysconf |=   0x0000ul;	/* OR  =0,0,0,0,0,0,0,0 */
	*STX7105_SYSCONF_SYS_CFG46 = sysconf;
	SET_PIO_PIN(PIO_PORT(8), 0, STPIO_ALT_OUT);
	SET_PIO_PIN(PIO_PORT(8), 1, STPIO_ALT_OUT);
	SET_PIO_PIN(PIO_PORT(8), 2, STPIO_ALT_OUT);
	SET_PIO_PIN(PIO_PORT(8), 3, STPIO_ALT_BIDIR);
	SET_PIO_PIN(PIO_PORT(8), 4, STPIO_ALT_OUT);
	SET_PIO_PIN(PIO_PORT(8), 5, STPIO_IN);
	SET_PIO_PIN(PIO_PORT(8), 6, STPIO_IN);
	SET_PIO_PIN(PIO_PORT(8), 7, STPIO_IN);

	/* PIO9[0] CFG47[8+0,0] = Alternate1 = MIIRXD[2] */
	/* PIO9[1] CFG47[8+1,1] = Alternate1 = MIIRXD[3] */
	/* PIO9[2] CFG47[8+2,2] = Alternate1 = MIITXCLK */
	/* PIO9[3] CFG47[8+3,3] = Alternate1 = MIICOL */
	/* PIO9[4] CFG47[8+4,4] = Alternate1 = MIICRS */
	/* PIO9[5] CFG47[8+5,5] = Alternate1 = MIIPHYCLK */
	/* PIO9[6] CFG47[8+6,6] = Alternate1 = MIIMDINT */
	sysconf = *STX7105_SYSCONF_SYS_CFG47;
	sysconf &= ~(0x7f7ful);	/* Mask=3,3,3,3,3,3,3 */
	sysconf |=   0x0000ul;	/* OR  =0,0,0,0,0,0,0 */
	*STX7105_SYSCONF_SYS_CFG47 = sysconf;
	SET_PIO_PIN(PIO_PORT(9), 0, STPIO_IN);
	SET_PIO_PIN(PIO_PORT(9), 1, STPIO_IN);
	SET_PIO_PIN(PIO_PORT(9), 2, STPIO_IN);
	SET_PIO_PIN(PIO_PORT(9), 3, STPIO_IN);
	SET_PIO_PIN(PIO_PORT(9), 4, STPIO_IN);
	/* MIIPHYCLK */
	/* Not implemented in cut 1 (DDTS GNBvd69906) - clock never output */
	/* In cut 2 PIO direction used to control input or output. */
	if (ext_clk)
		SET_PIO_PIN(PIO_PORT(9), 5, STPIO_IN);
	else
		SET_PIO_PIN(PIO_PORT(9), 5, STPIO_ALT_OUT);
	SET_PIO_PIN(PIO_PORT(9), 6, STPIO_IN);
}
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

int soc_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	bd_t *bd = gd->bd;

	stx7105_clocks();

#ifdef CONFIG_DRIVER_NET_STM_GMAC
#ifdef CONFIG_SYS_STM_GMAC_NOT_EXT_CLK
	stmac_eth_hw_setup (0, 0, 0, 0, 0, 0);
#else
	stmac_eth_hw_setup (0, 0, 0, 0, 1, 0);
#endif
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

	bd->bi_devid = *STX7105_SYSCONF_DEVICEID_0;

	/*
	 * Make sure the reset period is shorter than WDT time-out,
	 * and that the reset loop-back chain is *not* bypassed.
	 *	SYS_CFG09[29]    = long_reset_mode
	 *	SYS_CFG09[28:27] = cpu_rst_out_bypass(1:0)
	 *	SYS_CFG09[25:0]  = ResetOut_period
	 */
//QQQ	*STX7105_SYSCONF_SYS_CFG09 = (*STX7105_SYSCONF_SYS_CFG09 & 0xF7000000) | 0x000A8C;
	*STX7105_SYSCONF_SYS_CFG09 = (*STX7105_SYSCONF_SYS_CFG09 & 0xF4000000ul) | 0x000A8Cul;

	return 0;
}


#if defined(CONFIG_USB_OHCI_NEW)
extern int stx7105_usb_init(int port, int over_current, int power_ctrl)
{
	unsigned long reg;
	const unsigned char oc_pins[2]    = {4, 6};	/* PIO4 */
	const unsigned char power_pins[2] = {5, 7};	/* PIO4 */

	if (port >= sizeof(oc_pins))	/* invalid port number ? */
		return -1;		/* failed to initialize! */

	/* Power on the USB */
	reg = readl(STX7105_SYSCONF_SYS_CFG32);
	/* Power up USB host controller */
	/* USBn_HC_POWER_DOWN_REQ = 0 = Powered Up */
	reg &= ~(1ul<<(4+port));
	/* Power up USB PHY */
	/* USBn_PHY_POWER_DOWN_REQ = 0 = Powered Up */
	reg &= ~(1ul<<(6+port));
	writel(reg, STX7105_SYSCONF_SYS_CFG32);

	if (over_current) {
		/* USB overcurrent enable */
		reg = readl(STX7105_SYSCONF_SYS_CFG04);
		/* USB0_PRT_OVCURR_POL = 0 = Active Low */
		reg &= ~(1ul<<(3+port));
		/* USBn_PRT_OVCURR_IN = 0 = PIO4[oc_pins[port]] */
		reg &= ~(1ul<<(5+port));
		/* CFG_USBn_OVRCURR_ENABLE = 1 = OC Enabled */
		reg |= 1ul<<(11+port);
		writel(reg, STX7105_SYSCONF_SYS_CFG04);

		/* Route USBn OC Routing via PIO4[oc_pins[port]] */
		reg = readl(STX7105_SYSCONF_SYS_CFG34);
		/* PIO4[oc_pins[port]] CFG34[8+oc_pins[port],oc_pins[port]] = Alternate4 */
		reg &= ~(0x0101ul<<(oc_pins[port]));	/* Mask=3 */
		reg |=   0x0101ul<<(oc_pins[port]);	/* OR=3 */
		writel(reg, STX7105_SYSCONF_SYS_CFG34);
		/* set PIO directionality, for OC as IN */
		SET_PIO_PIN(PIO_PORT(4), oc_pins[port], STPIO_IN);
	}

	if (power_ctrl) {
		/* Route USBn POWER Routing via PIO4[power_pins[port]] */
		reg = readl(STX7105_SYSCONF_SYS_CFG34);
		/* PIO4[power_pins[port]] CFG34[8+power_pins[port],power_pins[port]] = Alternate4 */
		reg &= ~(0x0101ul<<(power_pins[port]));	/* Mask=3 */
		reg |=   0x0101ul<<(power_pins[port]);	/* OR=3 */
		writel(reg, STX7105_SYSCONF_SYS_CFG34);
		/* set PIO directionality, for POWER as ALT_OUT */
		SET_PIO_PIN(PIO_PORT(4), power_pins[port], STPIO_ALT_OUT);
	}

	/* start the USB Wrapper Host Controller */
	ST40_start_host_control(
		USB_FLAGS_STRAP_8BIT |
		USB_FLAGS_STBUS_CONFIG_THRESHOLD128);

	return 0;
}

#endif /* defined(CONFIG_USB_OHCI_NEW) */


#if defined(CONFIG_SH_STM_SATA)
extern void stx7105_configure_sata(void)
{
	static int initialised_phy = 0;
	unsigned long sysconf;

	if (!initialised_phy)
	{
		/* Power up the SATA PHY */
		sysconf = *STX7105_SYSCONF_SYS_CFG32;
		sysconf &= ~(1u<<9);	/* [11] SATA1_PHY_POWER_DOWN_REQ */
		*STX7105_SYSCONF_SYS_CFG32 = sysconf;

		/* initialize the SATA PHY */
		stm_sata_miphy_init();

		/* Power up the SATA host */
		sysconf = *STX7105_SYSCONF_SYS_CFG32;
		sysconf &= ~(1u<<11);	/* [9] SATA1_HC_POWER_DOWN_REQ */
		*STX7105_SYSCONF_SYS_CFG32 = sysconf;

		/* configure the SATA host controller */
		stm_sata_probe();

		initialised_phy = 1;
	}
}
#endif	/* CONFIG_SH_STM_SATA */


#if defined(CONFIG_SPI)

#if defined(CONFIG_SOFT_SPI)			/* Use "bit-banging" for SPI */
extern void stx7105_spi_scl(const int val)
{
	const int pin = 0;	/* PIO15[0] = SPI_CLK */
	STPIO_SET_PIN(PIO_PORT(15), pin, val ? 1 : 0);
}

extern void stx7105_spi_sda(const int val)
{
	const int pin = 1;	/* PIO15[1] = SPI_DOUT */
	STPIO_SET_PIN(PIO_PORT(15), pin, val ? 1 : 0);
}

extern unsigned char stx7105_spi_read(void)
{
	const int pin = 3;	/* PIO15[3] = SPI_DIN */
	return STPIO_GET_PIN(PIO_PORT(15), pin);
}
#endif	/* CONFIG_SOFT_SPI */

/*
 * assert or de-assert the SPI Chip Select line.
 *
 *	input: cs == true, assert CS, else deassert CS
 */
static void spi_chip_select(const int cs)
{
	const int pin = 2;	/* PIO15[2] = SPI_NOTCS */

	if (cs)
	{	/* assert SPI CSn */
		STPIO_SET_PIN(PIO_PORT(15), pin, 0);
	}
	else
	{	/* DE-assert SPI CSn */
		STPIO_SET_PIN(PIO_PORT(15), pin, 1);
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

