/*
 * (C) Copyright 2008-2011 STMicroelectronics.
 *
 * Stuart Menefy <stuart.menefy@st.com>
 * Sean McGoogan <Sean.McGoogan@st.com>
 * Pawel Moll <pawel.moll@st.com>
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
#include <asm/stbus.h>
#include <asm/sysconf.h>
#include <ata.h>
#include <spi.h>

#undef  BUG_ON
#define BUG_ON(condition) do { if ((condition)!=0) BUG(); } while(0)

#define ARRAY_SIZE(x)		(sizeof(x) / sizeof((x)[0]))

#define TRUE			1
#define FALSE			0


	/*
	 * The STx7108 is only supported in 32-bit (SE) mode,
	 * and is not supported in 29-bit (legacy) mode.
	 * We refuse to build, if this assertion is invalid.
	 */
#if !defined(CONFIG_SH_SE_MODE)
#error This SoC is not supported in 29-bit mode, please enable SE-mode!
#endif	/* !CONFIG_SH_SE_MODE */


static void stx7108_clocks(void)
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


/*
 * PIO alternative Function selector
 */
extern void stx7108_pioalt_select(const int port, const int pin, const int alt)
{
	int num;
	unsigned long sysconf, *sysconfReg;

#if 0
	printf("%s(port=%d, pin=%d, alt=%d)\n", __func__, port, pin, alt);
	BUG_ON(pin < 0 || pin > 7);
	BUG_ON(alt < 0 || alt > 5);
#endif

	switch (port)
	{
	case 0 ... 14:
		num = port;		/* in Bank #2 */
		sysconfReg = (unsigned long*)STX7108_BANK2_SYSCFG(num);
		break;
	case 15 ... 26:
		num = port - 15;	/* in Bank #4 */
		sysconfReg = (unsigned long*)STX7108_BANK4_SYSCFG(num);
		break;
	default:
		BUG();
		return;
	}

	sysconf = readl(sysconfReg);
	SET_SYSCONF_BITS(sysconf, TRUE, pin*4,(pin*4)+3, alt,alt);
	writel(sysconf, sysconfReg);
}

/* Pad configuration */

const struct stx7108_pioalt_pad_cfg stx7108_pioalt_pad_in = {
	.oe = 0,
	.pu = 0,
	.od = 0,
};
#define IN (&stx7108_pioalt_pad_in)

const struct stx7108_pioalt_pad_cfg stx7108_pioalt_pad_out = {
	.oe = 1,
	.pu = 0,
	.od = 0,
};
#define OUT (&stx7108_pioalt_pad_out)

const struct stx7108_pioalt_pad_cfg stx7108_pioalt_pad_od = {
	.oe = 1,
	.pu = 0,
	.od = 1,
};
#define OD (&stx7108_pioalt_pad_od)

const struct stx7108_pioalt_pad_cfg stx7108_pioalt_pad_bidir = {
	.oe = -1,
	.pu = 0,
	.od = 0,
};
#define BIDIR (&stx7108_pioalt_pad_bidir)

void stx7108_pioalt_pad(int port, const int pin,
		const struct stx7108_pioalt_pad_cfg * const cfg)
{
	int num, bit;
	unsigned long sysconf, *sysconfReg;

#if 0
	printf("%s(port=%d, pin=%d, oe=%d, pu=%d, od=%d)\n", __func__, port, pin, cfg->oe, cfg->pu, cfg->od);
	BUG_ON(pin < 0 || pin > 7);
	BUG_ON(!cfg);
#endif

	switch (port)
	{
	case 0 ... 14:
		num = 15 + (port / 4);	/* in Bank #2 */
		sysconfReg = (unsigned long*)STX7108_BANK2_SYSCFG(num);
		break;
	case 15 ... 26:
		port -= 15;
		num = 12 + (port / 4);	/* in Bank #4 */
		sysconfReg = (unsigned long*)STX7108_BANK4_SYSCFG(num);
		break;
	default:
		BUG();
		return;
	}

	bit = ((port * 8) + pin) % 32;

		/* set the "Output Enable" pad control */
	if (cfg->oe >= 0)
	{
		sysconf = readl(sysconfReg);
		SET_SYSCONF_BIT(sysconf, cfg->oe, bit);
		writel(sysconf, sysconfReg);
	}

	sysconfReg += 4;	/* skip 4 syscfg registers */

		/* set the "Pull Up" pad control */
	if (cfg->pu >= 0)
	{
		sysconf = readl(sysconfReg);
		SET_SYSCONF_BIT(sysconf, cfg->pu, bit);
		writel(sysconf, sysconfReg);
	}

	sysconfReg += 4;	/* skip another 4 syscfg registers */

		/* set the "Open Drain Enable" pad control */
	if (cfg->od >= 0)
	{
		sysconf = readl(sysconfReg);
		SET_SYSCONF_BIT(sysconf, cfg->od, bit);
		writel(sysconf, sysconfReg);
	}
}

/* PIO retiming setup */

/* Structure aligned to the "STi7108 Generic Retime Padlogic
 * Application Note" SPEC */
struct stx7108_pioalt_retime_cfg {
	int retime:2;
	int clk1notclk0:2;
	int clknotdata:2;
	int double_edge:2;
	int invertclk:2;
	int delay_input:2;
};

#ifdef CONFIG_DRIVER_NET_STM_GMAC

static void stx7108_pioalt_retime(const int port, const int pin,
		const struct stx7108_pioalt_retime_cfg * const cfg)
{
	int num;
	unsigned long sysconf, *sysconfReg;

#if 0
	printf("%s(port=%d, pin=%d, retime=%d, clk1notclk0=%d, "
			"clknotdata=%d, double_edge=%d, invertclk=%d, "
			"delay_input=%d)\n", __func__, port, pin,
			cfg->retime, cfg->clk1notclk0, cfg->clknotdata,
			cfg->double_edge, cfg->invertclk, cfg->delay_input
			);
	BUG_ON(pin < 0 || pin > 7);
#endif

	switch (port)
	{
	case 0 ... 14:
		switch (port)
		{
		case 1:
			num = 32;
			break;
		case 6 ... 14:
			num = 34 + ((port - 6) * 2);
			break;
		default:
			BUG();
			return;
		}
		sysconfReg = (unsigned long*)STX7108_BANK2_SYSCFG(num);
		break;
	case 15 ... 23:
		num = 48 + ((port - 15) * 2);
		sysconfReg = (unsigned long*)STX7108_BANK4_SYSCFG(num);
		break;
	default:
		BUG();
		return;
	}

	sysconfReg += 0;	/* use retime configuration register #0 */

	if (cfg->clk1notclk0 >= 0)
	{
		sysconf = readl(sysconfReg);
		SET_SYSCONF_BIT(sysconf, cfg->clk1notclk0, 0 + pin);
		writel(sysconf, sysconfReg);
	}

	if (cfg->clknotdata >= 0)
	{
		sysconf = readl(sysconfReg);
		SET_SYSCONF_BIT(sysconf, cfg->clknotdata, 8 + pin);
		writel(sysconf, sysconfReg);
	}

	if (cfg->delay_input >= 0)
	{
		sysconf = readl(sysconfReg);
		SET_SYSCONF_BIT(sysconf, cfg->delay_input, 16 + pin);
		writel(sysconf, sysconfReg);
	}

	if (cfg->double_edge >= 0)
	{
		sysconf = readl(sysconfReg);
		SET_SYSCONF_BIT(sysconf, cfg->double_edge, 24+ pin);
		writel(sysconf, sysconfReg);
	}

	sysconfReg += 1;	/* use retime configuration register #1 */

	if (cfg->invertclk >= 0)
	{
		sysconf = readl(sysconfReg);
		SET_SYSCONF_BIT(sysconf, cfg->invertclk, 0 + pin);
		writel(sysconf, sysconfReg);
	}

	if (cfg->retime >= 0)
	{
		sysconf = readl(sysconfReg);
		SET_SYSCONF_BIT(sysconf, cfg->retime, 8 + pin);
		writel(sysconf, sysconfReg);
	}
}

struct stx7108_gmac_pin {
	struct {
		unsigned char port, pin, alt;
	} pio[2];
	enum { BYPASS = 1, CLOCK, PHY_CLOCK, DATA, DGTX, RMII_TXD,
	       RMII_MDINT, RMII_MDIO, RMII_MDC, RMII_RXD, RMII_PHY_CLOCK
	} type;
	const struct stx7108_pioalt_pad_cfg *dir;
};

static struct stx7108_gmac_pin stx7108_gmac_mii_pins[] = {
	{ { { 9, 3, 1 }, { 15, 5, 2 } }, PHY_CLOCK, },		/* PHYCLK */
	{ { { 6, 0, 1 }, { 16, 0, 2 } }, DATA, OUT},		/* TXD[0] */
	{ { { 6, 1, 1 }, { 16, 1, 2 } }, DATA, OUT },		/* TXD[1] */
	{ { { 6, 2, 1 }, { 16, 2, 2 } }, DATA, OUT },		/* TXD[2] */
	{ { { 6, 3, 1 }, { 16, 3, 2 } }, DATA, OUT },		/* TXD[3] */
	{ { { 7, 0, 1 }, { 17, 1, 2 } }, DATA, OUT },		/* TXER */
	{ { { 7, 1, 1 }, { 15, 7, 2 } }, DATA, OUT },		/* TXEN */
	{ { { 7, 2, 1 }, { 17, 0, 2 } }, CLOCK, IN },		/* TXCLK */
	{ { { 7, 3, 1 }, { 17, 3, 2 } }, BYPASS, IN },		/* COL */
	{ { { 7, 4, 1 }, { 17, 4, 2 } }, BYPASS, BIDIR },	/* MDIO */
	{ { { 7, 5, 1 }, { 17, 5, 2 } }, CLOCK, OUT },		/* MDC */
	{ { { 7, 6, 1 }, { 17, 2, 2 } }, BYPASS, IN },		/* CRS */
	{ { { 7, 7, 1 }, { 15, 6, 2 } }, BYPASS, IN },		/* MDINT */
	{ { { 8, 0, 1 }, { 18, 0, 2 } }, DATA, IN },		/* RXD[0] */
	{ { { 8, 1, 1 }, { 18, 1, 2 } }, DATA, IN },		/* RXD[1] */
	{ { { 8, 2, 1 }, { 18, 2, 2 } }, DATA, IN },		/* RXD[2] */
	{ { { 8, 3, 1 }, { 18, 3, 2 } }, DATA, IN },		/* RXD[3] */
	{ { { 9, 0, 1 }, { 17, 6, 2 } }, DATA, IN },		/* RXDV */
	{ { { 9, 1, 1 }, { 17, 7, 2 } }, DATA, IN },		/* RX_ER */
	{ { { 9, 2, 1 }, { 19, 0, 2 } }, CLOCK, IN },		/* RXCLK */
};

static struct stx7108_gmac_pin stx7108_gmac_gmii_pins[] = {
	{ { { 9, 3, 1 }, { 15, 5, 2 } }, PHY_CLOCK, },		/* PHYCLK */
	{ { { 6, 0, 1 }, { 16, 0, 2 } }, DATA, OUT },		/* TXD[0] */
	{ { { 6, 1, 1 }, { 16, 1, 2 } }, DATA, OUT },		/* TXD[1] */
	{ { { 6, 2, 1 }, { 16, 2, 2 } }, DATA, OUT },		/* TXD[2] */
	{ { { 6, 3, 1 }, { 16, 3, 2 } }, DATA, OUT },		/* TXD[3] */
	{ { { 6, 4, 1 }, { 16, 4, 2 } }, DATA, OUT },		/* TXD[4] */
	{ { { 6, 5, 1 }, { 16, 5, 2 } }, DATA, OUT },		/* TXD[5] */
	{ { { 6, 6, 1 }, { 16, 6, 2 } }, DATA, OUT },		/* TXD[6] */
	{ { { 6, 7, 1 }, { 16, 7, 2 } }, DATA, OUT },		/* TXD[7] */
	{ { { 7, 0, 1 }, { 17, 1, 2 } }, DATA, OUT },		/* TXER */
	{ { { 7, 1, 1 }, { 15, 7, 2 } }, DATA, OUT },		/* TXEN */
	{ { { 7, 2, 1 }, { 17, 0, 2 } }, CLOCK, IN },		/* TXCLK */
	{ { { 7, 3, 1 }, { 17, 3, 2 } }, BYPASS, IN },		/* COL */
	{ { { 7, 4, 1 }, { 17, 4, 2 } }, BYPASS, BIDIR },	/* MDIO */
	{ { { 7, 5, 1 }, { 17, 5, 2 } }, CLOCK, OUT },		/* MDC */
	{ { { 7, 6, 1 }, { 17, 2, 2 } }, BYPASS, IN },		/* CRS */
	{ { { 7, 7, 1 }, { 15, 6, 2 } }, BYPASS, IN },		/* MDINT */
	{ { { 8, 0, 1 }, { 18, 0, 2 } }, DATA, IN }, 		/* RXD[0] */
	{ { { 8, 1, 1 }, { 18, 1, 2 } }, DATA, IN }, 		/* RXD[1] */
	{ { { 8, 2, 1 }, { 18, 2, 2 } }, DATA, IN }, 		/* RXD[2] */
	{ { { 8, 3, 1 }, { 18, 3, 2 } }, DATA, IN }, 		/* RXD[3] */
	{ { { 8, 4, 1 }, { 18, 4, 2 } }, DATA, IN }, 		/* RXD[4] */
	{ { { 8, 5, 1 }, { 18, 5, 2 } }, DATA, IN }, 		/* RXD[5] */
	{ { { 8, 6, 1 }, { 18, 6, 2 } }, DATA, IN }, 		/* RXD[6] */
	{ { { 8, 7, 1 }, { 18, 7, 2 } }, DATA, IN }, 		/* RXD[7] */
	{ { { 9, 0, 1 }, { 17, 6, 2 } }, DATA, IN },		/* RXDV */
	{ { { 9, 1, 1 }, { 17, 7, 2 } }, DATA, IN },		/* RX_ER */
	{ { { 9, 2, 1 }, { 19, 0, 2 } }, CLOCK, IN  },		/* RXCLK */
};

static struct stx7108_gmac_pin stx7108_gmac_gmii_gtx_pins[] = {
	{ { { 9, 3, 3 }, { 15, 5, 4 } }, PHY_CLOCK, },		/* PHYCLK */
	{ { { 6, 0, 1 }, { 16, 0, 2 } }, DATA, OUT },		/* TXD[0] */
	{ { { 6, 1, 1 }, { 16, 1, 2 } }, DATA, OUT },		/* TXD[1] */
	{ { { 6, 2, 1 }, { 16, 2, 2 } }, DATA, OUT },		/* TXD[2] */
	{ { { 6, 3, 1 }, { 16, 3, 2 } }, DATA, OUT },		/* TXD[3] */
	{ { { 6, 4, 1 }, { 16, 4, 2 } }, DGTX, OUT },		/* TXD[4] */
	{ { { 6, 5, 1 }, { 16, 5, 2 } }, DGTX, OUT },		/* TXD[5] */
	{ { { 6, 6, 1 }, { 16, 6, 2 } }, DGTX, OUT },		/* TXD[6] */
	{ { { 6, 7, 1 }, { 16, 7, 2 } }, DGTX, OUT },		/* TXD[7] */
	{ { { 7, 0, 1 }, { 17, 1, 2 } }, DATA, OUT },		/* TXER */
	{ { { 7, 1, 1 }, { 15, 7, 2 } }, DATA, OUT },		/* TXEN */
	{ { { 7, 2, 1 }, { 17, 0, 2 } }, CLOCK, IN },		/* TXCLK */
	{ { { 7, 3, 1 }, { 17, 3, 2 } }, BYPASS, IN },		/* COL */
	{ { { 7, 4, 1 }, { 17, 4, 2 } }, BYPASS, BIDIR },	/* MDIO */
	{ { { 7, 5, 1 }, { 17, 5, 2 } }, CLOCK, OUT },		/* MDC */
	{ { { 7, 6, 1 }, { 17, 2, 2 } }, BYPASS, IN },		/* CRS */
	{ { { 7, 7, 1 }, { 15, 6, 2 } }, BYPASS, IN },		/* MDINT */
	{ { { 8, 0, 1 }, { 18, 0, 2 } }, DATA, IN }, 		/* RXD[0] */
	{ { { 8, 1, 1 }, { 18, 1, 2 } }, DATA, IN }, 		/* RXD[1] */
	{ { { 8, 2, 1 }, { 18, 2, 2 } }, DATA, IN }, 		/* RXD[2] */
	{ { { 8, 3, 1 }, { 18, 3, 2 } }, DATA, IN }, 		/* RXD[3] */
	{ { { 8, 4, 1 }, { 18, 4, 2 } }, DGTX, IN }, 		/* RXD[4] */
	{ { { 8, 5, 1 }, { 18, 5, 2 } }, DGTX, IN }, 		/* RXD[5] */
	{ { { 8, 6, 1 }, { 18, 6, 2 } }, DGTX, IN }, 		/* RXD[6] */
	{ { { 8, 7, 1 }, { 18, 7, 2 } }, DGTX, IN }, 		/* RXD[7] */
	{ { { 9, 0, 1 }, { 17, 6, 2 } }, DATA, IN },		/* RXDV */
	{ { { 9, 1, 1 }, { 17, 7, 2 } }, DATA, IN },		/* RX_ER */
	{ { { 9, 2, 1 }, { 19, 0, 2 } }, CLOCK, IN  },		/* RXCLK */
};

static struct stx7108_gmac_pin stx7108_gmac_rmii_pins[] = {
	{ { { 9, 3, 2 }, { 15, 5, 3 } }, RMII_PHY_CLOCK, },	/* PHYCLK */
	{ { { 6, 0, 1 }, { 16, 0, 2 } }, RMII_TXD, OUT },	/* TXD[0] */
	{ { { 6, 1, 1 }, { 16, 1, 2 } }, RMII_TXD, OUT },	/* TXD[1] */
	{ { { 7, 0, 1 }, { 17, 1, 2 } }, RMII_TXD, OUT },	/* TXER */
	{ { { 7, 1, 1 }, { 15, 7, 2 } }, RMII_TXD, OUT },	/* TXEN */
	{ { { 7, 4, 1 }, { 17, 4, 2 } }, RMII_MDIO, BIDIR },	/* MDIO */
	{ { { 7, 5, 1 }, { 17, 5, 2 } }, RMII_MDC, OUT },	/* MDC */
	{ { { 7, 7, 1 }, { 15, 6, 2 } }, RMII_MDINT, IN  },	/* MDINT */
	{ { { 8, 0, 1 }, { 18, 0, 2 } }, RMII_RXD, IN  },	/* RXD[0] */
	{ { { 8, 1, 1 }, { 18, 1, 2 } }, RMII_RXD, IN  },	/* RXD[1] */
	{ { { 9, 0, 1 }, { 17, 6, 2 } }, RMII_RXD, IN  },	/* RXDV */
	{ { { 9, 1, 1 }, { 17, 7, 2 } }, RMII_RXD, IN  },	/* RX_ER */
};

static struct stx7108_gmac_pin stx7108_gmac_reverse_mii_pins[] = {
	{ { { 9, 3, 1 }, { 15, 5, 2 } }, PHY_CLOCK, },		/* PHYCLK */
	{ { { 6, 0, 1 }, { 16, 0, 2 } }, DATA, OUT },		/* TXD[-1] */
	{ { { 6, 1, 1 }, { 16, 1, 2 } }, DATA, OUT },		/* TXD[1] */
	{ { { 6, 2, 1 }, { 16, 2, 2 } }, DATA, OUT },		/* TXD[2] */
	{ { { 6, 3, 1 }, { 16, 3, 2 } }, DATA, OUT },		/* TXD[3] */
	{ { { 7, 0, 1 }, { 17, 1, 2 } }, DATA, OUT },		/* TXER */
	{ { { 7, 1, 1 }, { 15, 7, 2 } }, DATA, OUT },		/* TXEN */
	{ { { 7, 2, 1 }, { 17, 0, 2 } }, CLOCK, IN },		/* TXCLK */
	{ { { 7, 3, 2 }, { 17, 3, 3 } }, BYPASS, OUT },		/* COL */
	{ { { 7, 4, 1 }, { 17, 4, 2 } }, BYPASS, BIDIR },	/* MDIO */
	{ { { 7, 5, 2 }, { 17, 5, 3 } }, CLOCK, IN },		/* MDC */
	{ { { 7, 6, 2 }, { 17, 2, 3 } }, BYPASS, OUT },		/* CRS */
	{ { { 7, 7, 1 }, { 15, 6, 2 } }, BYPASS, IN },		/* MDINT */
	{ { { 8, 0, 1 }, { 18, 0, 2 } }, DATA, IN },		/* RXD[0] */
	{ { { 8, 1, 1 }, { 18, 1, 2 } }, DATA, IN },		/* RXD[1] */
	{ { { 8, 2, 1 }, { 18, 2, 2 } }, DATA, IN },		/* RXD[2] */
	{ { { 8, 3, 1 }, { 18, 3, 2 } }, DATA, IN },		/* RXD[3] */
	{ { { 9, 0, 1 }, { 17, 6, 2 } }, DATA, IN },		/* RXDV */
	{ { { 9, 1, 1 }, { 17, 7, 2 } }, DATA, IN },		/* RX_ER */
	{ { { 9, 2, 1 }, { 19, 0, 2 } }, CLOCK, IN },		/* RXCLK */
};

#define MAC_SPEED_SEL		1	/* [1:1] */
#define PHY_SEL			2,4	/* [4:2] */
#define ENMII			5	/* [5:5] */

#define ENABLE_GMAC		0	/* [0:0] */

extern int stmac_default_pbl(void)
{
	return 32;
}

#if CFG_STM_STMAC_BASE == CFG_STM_STMAC0_BASE		/* MII0 */
#	define STX7108_MII_SYSGFG(x)	(STX7108_BANK2_SYSCFG(x))
#elif CFG_STM_STMAC_BASE == CFG_STM_STMAC1_BASE		/* MII1 */
#	define STX7108_MII_SYSGFG(x)	(STX7108_BANK4_SYSCFG(x))
#else
#error Unknown base address for the STM GMAC
#endif

extern void stmac_set_mac_speed(int speed)
{
#if CFG_STM_STMAC_BASE == CFG_STM_STMAC0_BASE		/* MII0 */
	unsigned long * const sysconfReg = (void*)STX7108_MII_SYSGFG(27);
#elif CFG_STM_STMAC_BASE == CFG_STM_STMAC1_BASE		/* MII1 */
	unsigned long * const sysconfReg = (void*)STX7108_MII_SYSGFG(23);
#else
#error Unknown base address for the STM GMAC
#endif
	unsigned long sysconf = *sysconfReg;

	/* MIIx_MAC_SPEED_SEL = 0|1 */
	SET_SYSCONF_BIT(sysconf, (speed==100), MAC_SPEED_SEL);

	*sysconfReg = sysconf;
}

/* ETH MAC pad configuration */
extern void stx7108_configure_ethernet(
	const int port,
	const struct stx7108_ethernet_config * const config)
{
	unsigned long sysconf;

	int sc_regnum;
	struct stx7108_gmac_pin *pins;
	int pins_num;
	unsigned char phy_sel, enmii;
	int i;

	switch (port) {
	case 0:
		sc_regnum = 27;
		/* ENABLE_GMAC0 */
		sysconf = *STX7108_MII_SYSGFG(53);
		SET_SYSCONF_BIT(sysconf, TRUE, ENABLE_GMAC);
		*STX7108_MII_SYSGFG(53) = sysconf;
		break;
	case 1:
		sc_regnum = 23;
		/* ENABLE_GMAC1 */
		sysconf = *STX7108_MII_SYSGFG(67);
		SET_SYSCONF_BIT(sysconf, TRUE, ENABLE_GMAC);
		*STX7108_MII_SYSGFG(67) = sysconf;
		break;
	default:
		BUG();
		return;
	};

	switch (config->mode) {
	case stx7108_ethernet_mode_mii:
		phy_sel = 0;
		enmii = 1;
		pins = stx7108_gmac_mii_pins;
		pins_num = ARRAY_SIZE(stx7108_gmac_mii_pins);
		break;
	case stx7108_ethernet_mode_rmii:
		phy_sel = 4;
		enmii = 1;
		pins = stx7108_gmac_rmii_pins;
		pins_num = ARRAY_SIZE(stx7108_gmac_rmii_pins);
		break;
	case stx7108_ethernet_mode_gmii:
		phy_sel = 0;
		enmii = 1;
		pins = stx7108_gmac_gmii_pins;
		pins_num = ARRAY_SIZE(stx7108_gmac_gmii_pins);
		break;
	case stx7108_ethernet_mode_gmii_gtx:
		phy_sel = 0;
		enmii = 1;
		pins = stx7108_gmac_gmii_gtx_pins;
		pins_num = ARRAY_SIZE(stx7108_gmac_gmii_gtx_pins);
		break;
	case stx7108_ethernet_mode_reverse_mii:
		phy_sel = 0;
		enmii = 0;
		pins = stx7108_gmac_reverse_mii_pins;
		pins_num = ARRAY_SIZE(stx7108_gmac_reverse_mii_pins);
		break;
	default:
		BUG();
		return;
	}

	/* MIIx_PHY_SEL */
	sysconf = *STX7108_MII_SYSGFG(sc_regnum);
	SET_SYSCONF_BITS(sysconf, TRUE, 2,4, phy_sel,phy_sel);
	*STX7108_MII_SYSGFG(sc_regnum) = sysconf;

	/* ENMIIx */
	sysconf = *STX7108_MII_SYSGFG(sc_regnum);
	SET_SYSCONF_BIT(sysconf, enmii, ENMII);
	*STX7108_MII_SYSGFG(sc_regnum) = sysconf;

	pins[0].dir = config->ext_clk ? IN : OUT;

	for (i = 0; i < pins_num; i++) {
		const struct stx7108_gmac_pin *pin = &pins[i];
		int portno = pin->pio[port].port;
		int pinno = pin->pio[port].pin;
		struct stx7108_pioalt_retime_cfg retime_cfg = {
			-1, -1, -1, -1, -1, -1 /* -1 means "do not set" */
		};

		stx7108_pioalt_select(portno, pinno, pin->pio[port].alt);

		stx7108_pioalt_pad(portno, pinno, pin->dir);

		switch (pin->type) {
		case BYPASS:
			retime_cfg.clknotdata = 0;
			retime_cfg.retime = 0;
			break;
		case CLOCK:
			retime_cfg.clknotdata = 1;
			retime_cfg.clk1notclk0 = port;
			break;
		case PHY_CLOCK:
			retime_cfg.clknotdata = 1;
			if (config->mode == stx7108_ethernet_mode_gmii_gtx) {
				retime_cfg.clk1notclk0 = 1;
				retime_cfg.double_edge = 0;
			} else {
				retime_cfg.clk1notclk0 = 0;
			}
			break;
		case DGTX: /* extra configuration for GMII (GTK CLK) */
			if (port == 1) {
				retime_cfg.retime = 1;
				retime_cfg.clk1notclk0 = 1;
				retime_cfg.double_edge = 0;
				retime_cfg.clknotdata = 0;
			} else {
				retime_cfg.retime = 1;
				retime_cfg.clk1notclk0 = 0;
				retime_cfg.double_edge = 0;
				retime_cfg.clknotdata = 0;
			}
			break;
		case DATA:
			retime_cfg.clknotdata = 0;
			retime_cfg.retime = 1;
			retime_cfg.clk1notclk0 = port;
			break;
		case RMII_TXD:
			retime_cfg.retime = 1;
			retime_cfg.clk1notclk0 = 1;
			retime_cfg.clknotdata = 0;
			retime_cfg.double_edge = 0;
			retime_cfg.invertclk = 0;
			retime_cfg.delay_input = 0;
			break;
		case RMII_RXD:
			retime_cfg.retime = 1;
			retime_cfg.clk1notclk0 = 1;
			retime_cfg.clknotdata = 0;
			retime_cfg.double_edge = 0;
			retime_cfg.invertclk = 0;
			retime_cfg.delay_input = 2;
			break;
		case RMII_MDINT:
			retime_cfg.retime = 0;
			retime_cfg.clknotdata = 0;
			retime_cfg.delay_input = 0;
			break;
		case RMII_MDIO:
			retime_cfg.retime = 0;
			retime_cfg.clknotdata = 0;
			retime_cfg.delay_input = 3;
			break;
		case RMII_MDC:
			/* fallthru */
		case RMII_PHY_CLOCK:
			retime_cfg.retime = 1;
			retime_cfg.clk1notclk0 = 0;
			retime_cfg.clknotdata = 1;
			retime_cfg.invertclk = 0;
			retime_cfg.delay_input = 0;
			break;
		default:
			BUG();
			break;
		}
		stx7108_pioalt_retime(portno, pinno, &retime_cfg);
	}
}
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

extern int soc_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	bd_t *bd = gd->bd;

	stx7108_clocks();

	bd->bi_devid = *STX7108_SYSCONF_DEVICEID_0;

	/* Make sure reset period is shorter than WDT time-out */
	*STX7108_BANK0_SYSCFG(14) = 3000;	/* about 100 us */

	return 0;
}


#if defined(CONFIG_USB_OHCI_NEW)
extern int stx7108_usb_init(const int port)
{
//QQQ	DECLARE_GLOBAL_DATA_PTR;
//QQQ	bd_t *bd = gd->bd;
	static int initialized = 0;
	unsigned int flags;
	const struct {
		struct {
			unsigned char port, pin, alt;
		} oc, pwr;
	} usb_pins[] = {
		{ .oc = { 23, 6, 1 }, .pwr = { 23, 7, 1 } },
		{ .oc = { 24, 0, 1 }, .pwr = { 24, 1, 1 } },
		{ .oc = { 24, 2, 1 }, .pwr = { 24, 3, 1 } },
	};

	if (port < 0 || port >= 3)	/* invalid port number ? */
		return -1;		/* failed to initialize! */

	if (!initialized)		/* first time ? */
	{	/* set USB2TRIPPHY_OSCIOK = 1 */
		unsigned long * const sysconfReg = (unsigned long*)STX7108_BANK4_SYSCFG(44);
		unsigned long sysconf = readl(sysconfReg);
		SET_SYSCONF_BIT(sysconf, TRUE, 6);
		writel(sysconf, sysconfReg);
		initialized = 1;	/* do this just once */
	}

	/* Power up the USB port, i.e. set USB_x_POWERDOWN_REQ = 0 */
	{
		unsigned long * const sysconfReg = (unsigned long*)STX7108_BANK4_SYSCFG(46);
		unsigned long sysconf = readl(sysconfReg);
		SET_SYSCONF_BIT(sysconf, FALSE, port);
		writel(sysconf, sysconfReg);
	}
	/* wait until USBx_PWR_DWN_GRANT == 0 */
	while (*STX7108_BANK4_SYSSTS(2) & (1u<<port))
		;	/* just wait ... */

	/* route the USB power enable (output) signal */
	stx7108_pioalt_select(usb_pins[port].pwr.port,
			    usb_pins[port].pwr.pin,
			    usb_pins[port].pwr.alt);
	stx7108_pioalt_pad(usb_pins[port].pwr.port,
			  usb_pins[port].pwr.pin, OUT);

	/* route the USB over-current (input) signal */
	stx7108_pioalt_select(usb_pins[port].oc.port,
			    usb_pins[port].oc.pin,
			    usb_pins[port].oc.alt);
	stx7108_pioalt_pad(usb_pins[port].oc.port,
			  usb_pins[port].oc.pin, IN);

	/* start the USB Wrapper Host Controller */
#if 1	/* QQQ - DELETE */
	flags = USB_FLAGS_STRAP_8BIT | USB_FLAGS_STBUS_CONFIG_THRESHOLD128;
#else	/* QQQ - DELETE */
	if (STX7108_DEVICEID_CUT(bd->bi_devid) < 2)
	{		/* for cut 1.x */
		flags = USB_FLAGS_STRAP_8BIT |
			USB_FLAGS_STBUS_CONFIG_OPCODE_LD64_ST64 |
			USB_FLAGS_STBUS_CONFIG_PKTS_PER_CHUNK_1 |
			USB_FLAGS_STBUS_CONFIG_THRESHOLD_64;
	}
	else
	{		/* for cut 2.x */
		flags = USB_FLAGS_STRAP_8BIT |
			USB_FLAGS_STBUS_CONFIG_OPCODE_LD64_ST64 |
			USB_FLAGS_STBUS_CONFIG_PKTS_PER_CHUNK_2 |
			USB_FLAGS_STBUS_CONFIG_THRESHOLD_128;
	}
#endif	/* QQQ - DELETE */
	ST40_start_host_control(flags);

	return 0;
}

#endif /* defined(CONFIG_USB_OHCI_NEW) */


#if defined(CONFIG_SH_STM_SATA)
extern void stx7108_configure_sata(void)
{
#ifdef QQQ	/* QQQ - DELETE */
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
#endif		/* QQQ - DELETE */
}
#endif	/* CONFIG_SH_STM_SATA */


#if defined(CONFIG_CMD_I2C) && defined(CONFIG_SOFT_I2C)
struct ssc_pios
{
	struct
	{
		char port;
		char pin;
	}	pio[3];
};
static const struct ssc_pios ssc_pios[7] =
{
	{ { /* SSC0 */
		{ 1, 6 }, /* SCLK */
		{ 1, 7 }, /* MTSR */
		{ 2, 0 }, /* MRST */
	} }, { { /* SSC1 */
		{ 9, 6 }, /* SCLK */
		{ 9, 7 }, /* MTSR */
		{ 9, 5 }, /* MRST */
	} }, { { /* SSC2 */
#if 1
		{ 1, 3 }, /* SCLK */	/* Variant A */
		{ 1, 4 }, /* MTSR */
		{ 1, 5 }, /* MRST */
#else
		{ 14, 4 }, /* SCLK */	/* Variant B */
		{ 14, 5 }, /* MTSR */
		{ 14, 6 }, /* MRST */
#endif
	} }, { { /* SSC3 */
		{ 5, 2 }, /* SCLK */
		{ 5, 3 }, /* MTSR */
		{ 5, 4 }, /* MRST */
	} }, { { /* SSC4 */
		{ 13, 6 }, /* SCLK */
		{ 13, 7 }, /* MTSR */
		{ 14, 0 }, /* MRST */
	} }, { { /* SSC5 */
		{ 5, 6 }, /* SCLK */
		{ 5, 7 }, /* MTSR */
		{ 5, 5 }, /* MRST */
	} }, { { /* SSC6 */
		{ 15, 2 }, /* SCLK */
		{ 15, 3 }, /* MTSR */
		{ 15, 4 }, /* MRST */
	} },
};
// static const int stx7108_ssc_alt_funcs[] = { 2, 1, 2, 2, 1, 2, 1 };


extern void stx7108_configure_i2c(void)
{
	/*
	 * The I2C busses are routed as follows:
	 *
	 *	Bus	  SCL		  SDA
	 *	---	  ---		  ---
	 *	 0	PIO1[6]		PIO1[7]		SSC #0
	 *	 1	PIO9[6]		PIO9[7]		SSC #1
	 *	 2	PIO1[3]		PIO1[4]		SSC #2, Variant A
	 *	 2	PIO14[4]	PIO14[5]	SSC #2, Variant B
	 *	 3	PIO5[2]		PIO5[3]		SSC #3
	 *	 4	PIO13[6]	PIO13[7]	SSC #4
	 *	 5	PIO5[6]		PIO5[7]		SSC #5
	 *	 6	PIO15[2]	PIO15[3]	SSC #6
	 */
	const int scl_port = ssc_pios[CONFIG_I2C_BUS].pio[0].port;
	const int scl_pin  = ssc_pios[CONFIG_I2C_BUS].pio[0].pin;
	const int sda_port = ssc_pios[CONFIG_I2C_BUS].pio[1].port;
	const int sda_pin  = ssc_pios[CONFIG_I2C_BUS].pio[1].pin;

	if (CONFIG_I2C_BUS >= ARRAY_SIZE(ssc_pios)) BUG();

	SET_PIO_PIN(ST40_PIO_BASE(scl_port), scl_pin, STPIO_BIDIR);	/* I2C_SCL */
	SET_PIO_PIN(ST40_PIO_BASE(sda_port), sda_pin, STPIO_BIDIR);	/* I2C_SDA */
}

extern void stx7108_i2c_scl(const int val)
{
	/* SSC's SCLK == I2C's SCL */
	const int port = ssc_pios[CONFIG_I2C_BUS].pio[0].port;
	const int pin  = ssc_pios[CONFIG_I2C_BUS].pio[0].pin;
	STPIO_SET_PIN(ST40_PIO_BASE(port), pin, (val) ? 1 : 0);
}

extern void stx7108_i2c_sda(const int val)
{
	/* SSC's MTSR == I2C's SDA */
	const int port = ssc_pios[CONFIG_I2C_BUS].pio[1].port;
	const int pin  = ssc_pios[CONFIG_I2C_BUS].pio[1].pin;
	STPIO_SET_PIN(ST40_PIO_BASE(port), pin, (val) ? 1 : 0);
}

extern int stx7108_i2c_read(void)
{
	/* SSC's MTSR == I2C's SDA */
	const int port = ssc_pios[CONFIG_I2C_BUS].pio[1].port;
	const int pin  = ssc_pios[CONFIG_I2C_BUS].pio[1].pin;
	return STPIO_GET_PIN(ST40_PIO_BASE(port), pin);
}
#endif	/* defined(CONFIG_CMD_I2C) && defined(CONFIG_SOFT_I2C) */

#if defined(CONFIG_I2C_CMD_TREE)
extern unsigned int i2c_get_bus_speed(void)
{
	return CFG_I2C_SPEED;
}
extern int i2c_set_bus_speed(unsigned int speed)
{
	return -1;
}
#endif	/* CONFIG_I2C_CMD_TREE */


#if defined(CONFIG_SPI) && defined(CONFIG_SOFT_SPI)
	/*
	 * We want to use "bit-banging" for SPI (not SSC, nor FSM).
	 */
extern void stx7108_configure_spi(void)
{
	/*
	 *	We set up the PIO pins correctly for SPI
	 */

	/* route PIO (alternate #0) */
	stx7108_pioalt_select(2, 1, 0);			/* SPI_MISO */
	stx7108_pioalt_select(2, 0, 0);			/* SPI_MOSI */
	stx7108_pioalt_select(1, 7, 0);			/* SPI_notCS */
	stx7108_pioalt_select(1, 6, 0);			/* SPI_CLK */

	/* set PIO directionality */
	SET_PIO_PIN(ST40_PIO_BASE(2), 1, STPIO_IN);	/* SPI_MISO */
	SET_PIO_PIN(ST40_PIO_BASE(2), 0, STPIO_OUT);	/* SPI_MOSI */
	SET_PIO_PIN(ST40_PIO_BASE(1), 7, STPIO_OUT);	/* SPI_notCS */
	SET_PIO_PIN(ST40_PIO_BASE(1), 6, STPIO_OUT);	/* SPI_CLK */

	/* drive outputs with sensible initial values */
	STPIO_SET_PIN(ST40_PIO_BASE(2), 0, 0);		/* deassert SPI_MOSI */
	STPIO_SET_PIN(ST40_PIO_BASE(1), 7, 1);		/* deassert SPI_notCS */
	STPIO_SET_PIN(ST40_PIO_BASE(1), 6, 1);		/* assert SPI_CLK */
}

extern void stx7108_spi_scl(const int val)
{
	const int pin = 6;	/* PIO1[6] = SPI_CLK */
	STPIO_SET_PIN(ST40_PIO_BASE(1), pin, val ? 1 : 0);
}

extern void stx7108_spi_sda(const int val)
{
	const int pin = 0;	/* PIO2[0] = SPI_MOSI */
	STPIO_SET_PIN(ST40_PIO_BASE(2), pin, val ? 1 : 0);
}

extern unsigned char stx7108_spi_read(void)
{
	const int pin = 1;	/* PIO2[1] = SPI_MISO */
	return STPIO_GET_PIN(ST40_PIO_BASE(2), pin);
}

/*
 * assert or de-assert the SPI Chip Select line.
 *
 *	input: cs == true, assert CS, else deassert CS
 */
static void spi_chip_select(const int cs)
{
	const int pin = 7;	/* PIO1[7] = SPI_notCS */

	if (cs)
	{	/* assert SPI CSn */
		STPIO_SET_PIN(ST40_PIO_BASE(1), pin, 0);
	}
	else
	{	/* DE-assert SPI CSn */
		STPIO_SET_PIN(ST40_PIO_BASE(1), pin, 1);
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

#endif	/* CONFIG_SPI && CONFIG_SOFT_SPI */

