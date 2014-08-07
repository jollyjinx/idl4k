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
#include <asm/stx7141reg.h>
#include <asm/io.h>
#include <asm/pio.h>
#include <asm/stbus.h>
#include <ata.h>


static void stx7141_clocks(void)
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

#if CFG_STM_STMAC_BASE == CFG_STM_STMAC0_BASE	/* MAC = STM GMAC#0 */
#	define GMII_CLOCK_OUT		(1ul<<13)
#	define ETHERNET_INTERFACE_ON	(1ul<<16)
#	define MAC_SPEED_SEL		(1ul<<20)
#	define ENMII			(1ul<<27)
#	define PHY_INTF_SEL		24		/* bits [26:24] */
#elif CFG_STM_STMAC_BASE == CFG_STM_STMAC1_BASE	/* MAC = STM GMAC#1 */
#	define GMII_CLOCK_OUT		(1ul<<15)
#	define ETHERNET_INTERFACE_ON	(1ul<<17)
#	define MAC_SPEED_SEL		(1ul<<21)
#	define ENMII			(1ul<<31)
#	define PHY_INTF_SEL		28		/* bits [30:28] */
#else
#error Unknown GMAC Base address encountered!
#endif

#define PHY_INTF_SEL_MASK		(0x7ul<<PHY_INTF_SEL)

#define ARRAY_SIZE(x)			(sizeof(x) / sizeof((x)[0]))

#define AD_CONFIG_OFFSET		0x7000
#define READ_AHEAD_MASK			0xFFCFFFFF


const static struct {
	unsigned char syscfg;
	unsigned char lsb, msb;
} pio_sysconf[17][8] = {
	{
		/* PIO0 doesn't exist */
	}, {
		{ 19,  0,  1 },	/* PIO1[0] */
		{ 19,  2,  3 },	/* PIO1[1] */
		{ 19,  4,  5 },	/* PIO1[2] */
		{ 19,  5,  7 },	/* PIO1[3] */
		{ 19,  8,  8 },	/* PIO1[4] */
		{ 19,  9,  9 },	/* PIO1[5] */
		{ 19, 10, 10 },	/* PIO1[6] */
		{ 19, 11, 11 },	/* PIO1[7] */
	}, {
		{ 19, 12, 12 },	/* PIO2[0] */
		{ 19, 13, 13 },	/* PIO2[1] */
		{ 19, 14, 14 },	/* PIO2[2] */
		{ 19, 15, 15 },	/* PIO2[3] */
		{ 19, 16, 16 },	/* PIO2[4] */
		{ 19, 17, 17 },	/* PIO2[5] */
		{ 19, 18, 18 },	/* PIO2[6] */
		{ 19, 19, 19 },	/* PIO2[7] */
	}, {
		{ 19, 20, 20 },	/* PIO3[0] */
		{ 19, 21, 21 },	/* PIO3[1] */
		{ 19, 22, 23 },	/* PIO3[2] */
		{ 19, 24, 25 },	/* PIO3[3] */
		{ 19, 26, 27 },	/* PIO3[4] */
		{ 19, 28, 29 },	/* PIO3[5] */
		{ 19, 30, 31 },	/* PIO3[6] */
		{ 20,  0,  0 },	/* PIO3[7] */
	}, {
		{ 20,  1,  2 },	/* PIO4[0] */
		{ 20,  3,  4 },	/* PIO4[1] */
		{ 20,  5,  6 },	/* PIO4[2] */
		{ 20,  7,  8 },	/* PIO4[3] */
		{ 20,  9, 10 },	/* PIO4[4] */
		{ 20, 11, 12 },	/* PIO4[5] */
		{ 20, 13, 13 },	/* PIO4[6] */
		{ 20, 14, 14 },	/* PIO4[7] */
	}, {
		{ 20, 15, 16 },	/* PIO5[0] */
		{ 20, 17, 18 },	/* PIO5[1] */
		{ 20, 19, 19 },	/* PIO5[2] */
		{ 20, 20, 20 },	/* PIO5[3] */
		{ 20, 21, 21 },	/* PIO5[4] */
		{ 20, 22, 23 },	/* PIO5[5] */
		{ 20, 24, 24 },	/* PIO5[6] */
		{ 20, 25, 26 },	/* PIO5[7] */
	}, {
		{ 20, 27, 28 },	/* PIO6[0] */
		{ 20, 29, 30 },	/* PIO6[1] */
		{ 25,  0,  1 },	/* PIO6[2] */
		{ 25,  2,  3 },	/* PIO6[3] */
		{ 25,  4,  5 },	/* PIO6[4] */
		{ 25,  6,  7 },	/* PIO6[5] */
		{ 25,  8,  9 },	/* PIO6[6] */
		{ 25, 10, 11 },	/* PIO6[7] */
	}, {
		{ 25, 12, 13 },	/* PIO7[0] */
		{ 25, 14, 15 },	/* PIO7[1] */
		{ 25, 16, 17 },	/* PIO7[2] */
		{ 25, 18, 19 },	/* PIO7[3] */
		{ 25, 20, 21 },	/* PIO7[4] */
		{ 25, 22, 23 },	/* PIO7[5] */
		{ 25, 24, 25 },	/* PIO7[6] */
		{ 25, 26, 27 },	/* PIO7[7] */
	}, {
		{ 25, 28, 30 },	/* PIO8[0] */
		{ 35,  0,  2 },	/* PIO8[1] */
		{ 35,  3,  5 },	/* PIO8[2] */
		{ 35,  6,  8 },	/* PIO8[3] */
		{ 35,  9, 11 },	/* PIO8[4] */
		{ 35, 12, 14 },	/* PIO8[5] */
		{ 35, 15, 17 },	/* PIO8[6] */
		{ 35, 18, 20 },	/* PIO8[7] */
	}, {
		{ 35, 21, 22 },	/* PIO9[0] */
		{ 35, 23, 24 },	/* PIO9[1] */
		{ 35, 25, 26 },	/* PIO9[2] */
		{ 35, 27, 28 },	/* PIO9[3] */
		{ 35, 29, 30 },	/* PIO9[4] */
		{ 46,  0,  1 },	/* PIO9[5] */
		{ 46,  2,  3 },	/* PIO9[6] */
		{ 46,  4,  5 },	/* PIO9[7] */
	}, {
		{ 46,  6,  7 },	/* PIO10[0] */
		{ 46,  8,  9 },	/* PIO10[1] */
		{ 46, 10, 11 },	/* PIO10[2] */
		{ 46, 12, 13 },	/* PIO10[3] */
		{ 46, 14, 15 },	/* PIO10[4] */
		{ 46, 16, 17 },	/* PIO10[5] */
		{ 46, 18, 19 },	/* PIO10[6] */
		{ 46, 20, 21 },	/* PIO10[7] */
	}, {
		{ 46, 22, 23 },	/* PIO11[0] */
		{ 46, 24, 26 },	/* PIO11[1] */
		{ 46, 27, 29 },	/* PIO11[2] */
		{ 47,  0,  2 },	/* PIO11[3] */
		{ 47,  3,  5 },	/* PIO11[4] */
		{ 47,  6,  8 },	/* PIO11[5] */
		{ 47,  9, 11 },	/* PIO11[6] */
		{ 47, 12, 14 },	/* PIO11[7] */
	}, {
		{ 47, 15, 17 },	/* PIO12[0] */
		{ 47, 18, 20 },	/* PIO12[1] */
		{ 47, 21, 23 },	/* PIO12[2] */
		{ 47, 24, 25 },	/* PIO12[3] */
		{ 47, 26, 27 },	/* PIO12[4] */
		{ 47, 28, 29 },	/* PIO12[5] */
		{ 48,  0,  2 },	/* PIO12[6] */
		{ 48,  3,  5 },	/* PIO12[7] */
	}, {
		{ 48,  6,  8 },	/* PIO13[0] */
		{ 48,  9, 11 },	/* PIO13[1] */
		{ 48, 12, 14 },	/* PIO13[2] */
		{ 48, 15, 17 },	/* PIO13[3] */
		{ 48, 18, 20 },	/* PIO13[4] */
		{ 48, 21, 23 },	/* PIO13[5] */
		{ 48, 24, 25 },	/* PIO13[6] */
		{ 48, 26, 27 },	/* PIO13[7] */
	}, {
		{ 48, 28, 30 },	/* PIO14[0] */
		{ 49,  0,  2 },	/* PIO14[1] */
		{ 49,  3,  5 },	/* PIO14[2] */
		{ 49,  6,  8 },	/* PIO14[3] */
		{ 49,  9, 11 },	/* PIO14[4] */
		{ 49, 12, 14 },	/* PIO14[5] */
		{ 49, 15, 17 },	/* PIO14[6] */
		{ 49, 18, 19 },	/* PIO14[7] */
	}, {
		{ 49, 20, 21 },	/* PIO15[0] */
		{ 49, 22, 23 },	/* PIO15[1] */
		{ 49, 24, 25 },	/* PIO15[2] */
		{ 49, 26, 27 },	/* PIO15[3] */
		{ 49, 28, 28 },	/* PIO15[4] */
		{ 49, 29, 29 },	/* PIO15[5] */
		{ 49, 30, 30 },	/* PIO15[6] */
		{ 50,  0,  1 },	/* PIO15[7] */
	}, {
		{ 50,  2,  3 },	/* PIO16[0] */
		{ 50,  4,  5 },	/* PIO16[1] */
		{ 50,  6,  7 },	/* PIO16[2] */
		{ 50,  8,  8 },	/* PIO16[3] */
		{ 50,  9,  9 },	/* PIO16[4] */
		{ 50, 10, 10 },	/* PIO16[5] */
		{ 50, 11, 11 },	/* PIO16[6] */
		{ 50, 12, 12 },	/* PIO16[7] */
	}
};

static void stx7141_pio_sysconf(const int bank, const int pin, const int alt)
{
	const unsigned char syscfg = pio_sysconf[bank][pin].syscfg;
	const unsigned char msb = pio_sysconf[bank][pin].msb;
	const unsigned char lsb = pio_sysconf[bank][pin].lsb;
	const unsigned long mask = (1ul<<((msb)-(lsb)+1))-1ul;
	const unsigned long addr = (unsigned long)STX7141_SYSCONF_SYS_CFG00 + (syscfg * 4ul);
	unsigned long sysconf;

	/* set PIO to alternate function 'alt' */
	sysconf = readl(addr);
	sysconf &= ~(mask<<lsb);
	sysconf |= alt<<lsb;
	writel(sysconf, addr);

#if 0
	printf("%s(): PIO%02u[%u] --> CFG%02u[%u:%u] = %u\t*0x%08x=0x%08x\n",
		__FUNCTION__,
		bank, pin,
		syscfg, msb, lsb,
		alt,
		addr, sysconf);
#endif
}


extern int stmac_default_pbl(void)
{
	return 32;
}

extern void stmac_set_mac_speed(int speed)
{
	unsigned long sysconf = *STX7141_SYSCONF_SYS_CFG07;

	/* MAC_SPEED_SEL = 0|1 */
	if (speed == 100)
		sysconf |= MAC_SPEED_SEL;
	else
		sysconf &= ~MAC_SPEED_SEL;

	*STX7141_SYSCONF_SYS_CFG07 = sysconf;
}

/* ETH MAC pad configuration */
extern void stx7141_configure_ethernet(
	const int port,
	const int reverse_mii,
	const int mode,
	const int phy_bus)
{
	DECLARE_GLOBAL_DATA_PTR;
	bd_t * const bd = gd->bd;
	size_t i;

	const struct {
		struct {
			unsigned char port, pin, alt;
		} pio[2];
		unsigned char dir;
	} pins[] = {
		{ { {  8, 0, 1 }, { 11, 4, 1 } }, STPIO_IN  },	/* TXCLK */
		{ { {  8, 1, 1 }, { 11, 5, 1 } }, STPIO_OUT },	/* TXEN */
		{ { {  8, 2, 1 }, { 11, 6, 1 } }, STPIO_OUT },	/* TXER */
		{ { {  8, 3, 1 }, { 11, 7, 1 } }, STPIO_OUT },	/* TXD[0] */
		{ { {  8, 4, 1 }, { 12, 0, 1 } }, STPIO_OUT },	/* TXD[1] */
		{ { {  8, 5, 1 }, { 12, 1, 1 } }, STPIO_OUT },	/* TXD[2] */
		{ { {  8, 6, 1 }, { 12, 2, 1 } }, STPIO_OUT },	/* TXD[3] */
		{ { {  8, 7, 1 }, { 12, 3, 1 } }, STPIO_OUT },	/* TXD[4] */
		{ { {  9, 0, 1 }, { 12, 4, 1 } }, STPIO_OUT },	/* TXD[5] */
		{ { {  9, 1, 1 }, { 12, 5, 1 } }, STPIO_OUT },	/* TXD[6] */
		{ { {  9, 2, 1 }, { 12, 6, 1 } }, STPIO_OUT },	/* TXD[7] */
		{ { {  9, 3, 1 }, { 12, 7, 1 } }, STPIO_IN  },	/* RXCLK */
		{ { {  9, 4, 1 }, { 13, 0, 1 } }, STPIO_IN  },	/* RXDV */
		{ { {  9, 5, 1 }, { 13, 1, 1 } }, STPIO_IN  },	/* RX_ER */
		{ { {  9, 6, 1 }, { 13, 2, 1 } }, STPIO_IN  },	/* RXD[0] */
		{ { {  9, 7, 1 }, { 13, 3, 1 } }, STPIO_IN  },	/* RXD[1] */
		{ { { 10, 0, 1 }, { 13, 4, 1 } }, STPIO_IN  },	/* RXD[2] */
		{ { { 10, 1, 1 }, { 13, 5, 1 } }, STPIO_IN  },	/* RXD[3] */
		{ { { 10, 2, 1 }, { 13, 6, 1 } }, STPIO_IN  },	/* RXD[4] */
		{ { { 10, 3, 1 }, { 13, 7, 1 } }, STPIO_IN  },	/* RXD[5] */
		{ { { 10, 4, 1 }, { 14, 0, 1 } }, STPIO_IN  },	/* RXD[6] */
		{ { { 10, 5, 1 }, { 14, 1, 1 } }, STPIO_IN  },	/* RXD[7] */
		{ { { 10, 6, 1 }, { 14, 2, 1 } }, STPIO_IN  },	/* CRS */
		{ { { 10, 7, 1 }, { 14, 3, 1 } }, STPIO_IN  },	/* COL */
		{ { { 11, 0, 1 }, { 14, 4, 1 } }, STPIO_OUT },	/* MDC */
		{ { { 11, 1, 1 }, { 14, 5, 1 } }, STPIO_BIDIR },/* MDIO */
		{ { { 11, 2, 1 }, { 14, 6, 1 } }, STPIO_IN  },	/* MDINT */
		{ { { 11, 3, 1 }, { 14, 7, 1 } }, STPIO_OUT },	/* PHYCLK */
	};

	unsigned long sysconf = *STX7141_SYSCONF_SYS_CFG07;

	/* Cut 2 of 7141 has AHB wrapper bug for ethernet gmac */
	/* Need to disable read-ahead - performance impact     */
	if (STX7141_DEVICEID_CUT(bd->bi_devid) == 2)
	{
		const unsigned long addr = CFG_STM_STMAC_BASE + AD_CONFIG_OFFSET;
		writel(readl(addr) & READ_AHEAD_MASK, addr);
	}

	/* gmac_en: GMAC Enable */
	sysconf |= ETHERNET_INTERFACE_ON;

	/* GMII clock configuration */
	sysconf |= GMII_CLOCK_OUT;

	/* enmii: Interface type (rev MII/MII) */
	if (reverse_mii)
		sysconf &= ~ENMII;
	else
		sysconf |= ENMII;

	/* phy_intf_sel[2;0] : PHY Interface Selection */
	/* Note the that MSB implicitly also set mii_mode */
	sysconf &= ~PHY_INTF_SEL_MASK;
	sysconf |= ((mode<<PHY_INTF_SEL) & PHY_INTF_SEL_MASK);

	*STX7141_SYSCONF_SYS_CFG07 = sysconf;

	/* now configure & route all the MII PIOs */
	for (i = 0; i < ARRAY_SIZE(pins); i++)
	{
		stx7141_pio_sysconf(pins[i].pio[port].port,
				    pins[i].pio[port].pin,
				    pins[i].pio[port].alt);
		SET_PIO_PIN(
			ST40_PIO_BASE(pins[i].pio[port].port),
			pins[i].pio[port].pin,
			pins[i].dir);
	}
}
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

int soc_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	bd_t *bd = gd->bd;
	unsigned long reg;

	stx7141_clocks();

	bd->bi_devid = *STX7141_SYSCONF_DEVICEID_0;

	/*
	 * Reset Generation Configuration
	 * Make sure reset period is shorter than WDT time-out,
	 * and that the reset is not bypassed.
	 *
	 *	[28:27] = CPU_RST_OUT_BYPASS[1:0]
	 *	[25:0]  = RESETOUT_PERIOD
	 */
	reg = *STX7141_SYSCONF_SYS_CFG09;
	/* Propagate the reset signal */
	reg = (reg & (~(3ul<<27))) | ((0ul)<<27);
	/* use the default "short" reset time of 100us */
	reg = (reg & (~0x03FFFFFFul)) | 0x00000A8Cul;
	*STX7141_SYSCONF_SYS_CFG09 = reg;

	/*
	 * SH4 Boot Configuration
	 * Unmask the reset signal from the SH4 core.
	 *
	 *	[4:3] = SH4_MASK_RST_OUT[1:0]
	 *
	 * SH4_MASK_RST_OUT[1]: mask rst_out signal from SH4-eCM core
	 * SH4_MASK_RST_OUT[0]: mask rst_out signal from SH4-eSTB core
	 */
	reg = *STX7141_SYSCONF_SYS_CFG08;
#if 1
	/* Unmask the reset signal from the SH4-eSTB core */
	reg = (reg & (~(1ul<<3))) | ((0ul)<<3);
#else
	/* Unmask the reset signal from the SH4-eCM core */
	reg = (reg & (~(1ul<<4))) | ((0ul)<<4);
#endif
	*STX7141_SYSCONF_SYS_CFG08 = reg;

	return 0;
}


#if defined(CONFIG_USB_OHCI_NEW)
extern void stx7141_usb_init(void)
{
#ifdef QQQ	/* QQQ - TO DO */
	unsigned long reg, req_reg;

	/* Power on the USB */
	reg = readl(STX7141_SYSCONF_SYS_CFG32);
	reg &= ~(1ul<<4); /* USB_POWER_DOWN_REQ = 0 */
	writel(reg, STX7141_SYSCONF_SYS_CFG32);

	/* Work around for USB over-current detection chip being
	 * active low, and the 7141 being active high.
	 * Note this is an undocumented bit, which apparently enables
	 * an inverter on the overcurrent signal.
	 */
	reg = readl(STX7141_SYSCONF_SYS_CFG06);
	reg |= 1ul<<29;
	writel(reg, STX7141_SYSCONF_SYS_CFG06);

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
#endif		/* QQQ - TO DO */
}

#endif /* defined(CONFIG_USB_OHCI_NEW) */


#if defined(CONFIG_SH_STM_SATA)
extern void stx7141_configure_sata(void)
{
	static int initialised_phy = 0;
	unsigned long sysconf;

	if (!initialised_phy)
	{
		/* enable reset  */
		sysconf = *STX7141_SYSCONF_SYS_CFG04;
		sysconf |= 1u<<9;
		*STX7141_SYSCONF_SYS_CFG04 = sysconf;

		sysconf = *STX7141_SYSCONF_SYS_CFG32;
		sysconf |= 1u<<6;	/* [6] SATA_SLUMBER_POWER_MODE */
		*STX7141_SYSCONF_SYS_CFG32 = sysconf;

		/* initialize the SATA PHY */
		stm_sata_miphy_init();

		/* configure the SATA host controller */
		stm_sata_probe();

		initialised_phy = 1;
	}
}
#endif	/* CONFIG_SH_STM_SATA */


