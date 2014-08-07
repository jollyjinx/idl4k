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
#include <asm/stx5206reg.h>
#include <asm/io.h>
#include <asm/pio.h>
#include <asm/stbus.h>
#include <asm/sysconf.h>
#include <ata.h>
#include <spi.h>


static void stx5206_clocks(void)
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

#define ETHERNET_INTERFACE_ON	16
#define PHY_CLK_EXT		19
#define MAC_SPEED_SEL		20
#define ENMII			27

extern int stmac_default_pbl(void)
{
	return 32;
}

extern void stmac_set_mac_speed(int speed)
{
	unsigned long sysconf;

	/* gmac_mac_speed = speed==100 ? 1 : 0 */
	sysconf = readl(STX5206_SYSCONF_SYS_CFG07);
	SET_SYSCONF_BIT(sysconf, speed==100, MAC_SPEED_SEL);
	writel(sysconf, STX5206_SYSCONF_SYS_CFG07);
}

	/*
	 * ETH GMAC PIO configuration
	 */
extern void stx5206_configure_ethernet(
	const enum stx5206_ethernet_mode mode,
	const int ext_clk,
	const int phy_bus)
{
	unsigned int phy_sel, enmii;
	unsigned long phy_clk_rate;
	unsigned long sysconf;

	switch (mode) {
	case stx5206_ethernet_mii:
		phy_sel = 0x0;
		enmii = 1;
		phy_clk_rate = 25000000;	/* 25 MHz */
		break;
	case stx5206_ethernet_rmii:
		phy_sel = 0x4;
		enmii = 1;
		phy_clk_rate = 50000000;	/* 50 MHz */
		break;
	case stx5206_ethernet_reverse_mii:
		phy_sel = 0x0;
		enmii = 0;
		phy_clk_rate = 25000000;	/* 25 MHz */
		break;
	default:
		BUG();
		return;
	}

	/* ethernet_interface_on */
	sysconf = readl(STX5206_SYSCONF_SYS_CFG07);
	SET_SYSCONF_BIT(sysconf, 1, ETHERNET_INTERFACE_ON);
	writel(sysconf, STX5206_SYSCONF_SYS_CFG07);

	/* phy_clk_ext: MII_PHYCLK pad function: 1 = phy clock is external,
	 * 0 = phy clock is provided by STx5289 */
	sysconf = readl(STX5206_SYSCONF_SYS_CFG07);
	SET_SYSCONF_BIT(sysconf, ext_clk, PHY_CLK_EXT);
	writel(sysconf, STX5206_SYSCONF_SYS_CFG07);

	/* phy_intf_sel */
	sysconf = readl(STX5206_SYSCONF_SYS_CFG07);
	SET_SYSCONF_BITS(sysconf, 1, 24, 26, phy_sel, phy_sel);
	writel(sysconf, STX5206_SYSCONF_SYS_CFG07);

	/* enMii: 1 = MII mode, 0 = Reverse MII mode */
	sysconf = readl(STX5206_SYSCONF_SYS_CFG07);
	SET_SYSCONF_BIT(sysconf, enmii, ENMII);
	writel(sysconf, STX5206_SYSCONF_SYS_CFG07);

	/* Set PHY clock frequency (if used) */
	if (!ext_clk)
	{
		if (phy_clk_rate == 25000000)	/* 25 MHz */
		{
			/* CLKGENA.CLK_DIV_LS[13] = CLK_ETHERNET_PHY = 25 MHz */
			writel(17, STX5206_CLOCKGENA_PLL0LS_DIV13_CFG);
		}
#if 0		/* QQQ: Need to check this! */
		else				/* 50 MHz */
		{
			/* CLKGENA.CLK_DIV_LS[13] = CLK_ETHERNET_PHY = 50 MHz */
			writel(8, STX5206_CLOCKGENA_PLL0LS_DIV13_CFG);
		}
#endif
	}
}
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */


int soc_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	bd_t *bd = gd->bd;

	stx5206_clocks();

	bd->bi_devid = *STX5206_SYSCONF_DEVICEID_0;

	/*
	 * Make sure the reset period is shorter than WDT time-out,
	 * and that the reset loop-back chain is *not* bypassed.
	 *	SYS_CFG09[29]    = long_reset_mode
	 *	SYS_CFG09[28:27] = cpu_rst_out_bypass(1:0)
	 *	SYS_CFG09[25:0]  = ResetOut_period
	 */
#ifdef QQQ	/* QQQ - DELETE */
//QQQ	*STX7105_SYSCONF_SYS_CFG09 = (*STX7105_SYSCONF_SYS_CFG09 & 0xF7000000) | 0x000A8C;
	*STX7105_SYSCONF_SYS_CFG09 = (*STX7105_SYSCONF_SYS_CFG09 & 0xF4000000ul) | 0x000A8Cul;

#endif		/* QQQ - DELETE */
	return 0;
}


#if defined(CONFIG_USB_OHCI_NEW)
extern void stx5206_usb_init(void)
{
	unsigned long sysconf;

	/* USB_HOST_SOFT_RESET: active low usb host sof reset */
//	sc = sysconf_claim(SYS_CFG, 4, 1, 1, "USB");
//	sysconf_write(sc, 1);
	sysconf = readl(STX5206_SYSCONF_SYS_CFG04);
	SET_SYSCONF_BIT(sysconf, 1, 1);
	writel(sysconf, STX5206_SYSCONF_SYS_CFG04);

	/* suspend_from_config: Signal to suspend USB PHY */
//	sc = sysconf_claim(SYS_CFG, 10, 5, 5, "USB");
//	sysconf_write(sc, 0);
	sysconf = readl(STX5206_SYSCONF_SYS_CFG10);
	SET_SYSCONF_BIT(sysconf, 0, 5);
	writel(sysconf, STX5206_SYSCONF_SYS_CFG10);

	/* usb_power_down_req: power down request for USB Host module */
//	sc = sysconf_claim(SYS_CFG, 32, 4, 4, "USB");
//	sysconf_write(sc, 0);
	sysconf = readl(STX5206_SYSCONF_SYS_CFG32);
	SET_SYSCONF_BIT(sysconf, 0, 4);
	writel(sysconf, STX5206_SYSCONF_SYS_CFG32);

	/* start the USB Wrapper Host Controller */
	ST40_start_host_control(
		USB_FLAGS_STRAP_8BIT |
		USB_FLAGS_STBUS_CONFIG_THRESHOLD128);
}
#endif /* defined(CONFIG_USB_OHCI_NEW) */


