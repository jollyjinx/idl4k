/*
 *  Copyright (c) 2006-2011 STMicroelectronics Limited
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * author(s): Andy Sturges (andy.sturges@st.com)
 *            Sean McGoogan <Sean.McGoogan@st.com>
 */

#include <common.h>

#if defined(CONFIG_DRIVER_NETSTMAC) || defined(CONFIG_DRIVER_NET_STM_GMAC)

#include <command.h>
#include <asm/soc.h>
#include <asm/addrspace.h>
#include <asm/io.h>
#include <net.h>
#include <malloc.h>
#include <miiphy.h>
#include "stm-stmac.h"

#ifdef DEBUG
#	define PRINTK(args...) printf(args)
#else
#	define PRINTK(args...)
#endif

void stmac_set_mac_addr (unsigned char *Addr)
{
	unsigned long data;

	data = (Addr[5] << 8) | Addr[4];
	STMAC_WRITE (data, MAC_ADDR_HIGH);
	data = (Addr[3] << 24) | (Addr[2] << 16) | (Addr[1] << 8) | Addr[0];
	STMAC_WRITE (data, MAC_ADDR_LOW);
}

#if defined(CONFIG_CMD_NET)

/* do we want to put the PHY in loop-back mode ? */
/* #define CONFIG_PHY_LOOPBACK */

/* do we want to dump the first 14-bytes of each
 * RX/TX ethernet packet (i.e. the IEEE 802.3 MAC,
 * (or RFC 894/1042) encapsulation header ? */
/* #define DUMP_ENCAPSULATION_HEADER */

/* prefix to use for diagnostics */
#ifdef CONFIG_DRIVER_NETSTMAC
#	define STMAC	"STM-MAC: "
#else
#	define STMAC	"STM-GMAC: "
#endif /* CONFIG_DRIVER_NETSTMAC */

#define CONFIG_DMA_RX_SIZE 8
#define CONFIG_DMA_TX_SIZE 1	/* Only ever use 1 tx buffer */

static volatile stmac_dma_des *dma_tx;
static volatile stmac_dma_des *dma_rx;
static int cur_rx;
static int eth_phy_addr;
static char miidevice[] = "stmacphy";

#define MAX_ETH_FRAME_SIZE      1536
#define MAX_PAUSE_TIME (MAC_FLOW_CONTROL_PT_MASK>>MAC_FLOW_CONTROL_PT_SHIFT)

static void stmac_mii_write (int phy_addr, int reg, int value);
static unsigned int stmac_mii_read (int phy_addr, int reg);
static void stmac_set_mac_mii_cap (int full_duplex, unsigned int speed);

/* DMA structure */
struct dma_t
{
	stmac_dma_des desc_rx[CONFIG_DMA_RX_SIZE];
	stmac_dma_des desc_tx[CONFIG_DMA_TX_SIZE];
	uchar rx_buff[CONFIG_DMA_RX_SIZE * (PKTSIZE_ALIGN)];
	uchar _dummy[L1_CACHE_BYTES];
} __attribute__ ((aligned (L1_CACHE_BYTES))) dma;

static void *rx_packets[CONFIG_DMA_RX_SIZE];

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

/* ----------------------------------------------------------------------------
				 Phy interface
   ---------------------------------------------------------------------------*/

#if defined(CONFIG_STMAC_STE10XP)	/* ST STe10xp */

/* STe100p phy identifier values */
#define STE100P_PHY_ID		0x1c040011u
#define STE100P_PHY_ID_MASK	0xffffffffu

/* STe101p phy identifier values */
#define STE101P_PHY_ID		0x00061c50u
#define STE101P_PHY_ID_MASK	0xfffffff0u

/******************************************************************************
 * IEEE Standard 802.3-2002 vendor specific registers (0x10-0x1e) STe10xP
 *****************************************************************************/
#define MII_XCIIS                0x11	/* Config info & int status register */
#define MII_XIE                  0x12	/* Interrupt enable register */
#define MII_100CTR               0x13	/* 100BaseX control register */
#define MII_XMC                  0x14	/* Mode control register */

/******************************************************************************
 * 100BaseX Auxiliary Status register defines
 *****************************************************************************/
#define XCIIS_FIFO_OVR           0x0800	/* FIFO Overrun */
#define XCIIS_SPEED              0x0200	/* Speed */
#define XCIIS_DUPLEX             0x0100	/* Duplex */
#define XCIIS_PAUSE              0x0080	/* Pause */
#define XCIIS_ANEG_INT           0x0040	/* Auto Negotiation Interrupt */
#define XCIIS_RFAULT             0x0020	/* Remote Fault Interrupt */
#define XCIIS_LDOWN              0x0010	/* Link Down Interrupt */
#define XCIIS_LCWR               0x0008	/* Link Code Word Received Interrupt */
#define XCIIS_PFAULT             0x0004	/* Parallel Detection Fault */
#define XCIIS_ANEG_PAGE          0x0002	/* Auto Negotiation Page Rec Intr */
#define XCIIS_REF_INTR           0x0001	/* Ref Interrupt */

/******************************************************************************
 * XCVR Mode Control register defines
 *****************************************************************************/
#define XMC_LDETECT              0x0800	/* Link Detect */
#define XMC_PHY_ADDR_MSK         0x00f8	/* PHY Address Mask */
#define XMC_PHY_ADDR_SHIFT       3	/* PHY Address Mask */
#define XMC_PRE_SUP              0x0002	/* Preamble Suppression */
#define PHY_ADDR_MSK		XMC_PHY_ADDR_MSK	/* PHY Address Mask */
#define PHY_ADDR_SHIFT		XMC_PHY_ADDR_SHIFT	/* PHY Address Mask */

/* MII mode */
#define MII_TSTAT_SMII  0x1000
#define MII_TSTAT_RMII  0x0800
#define MII_TSTAT_MII   0x0400

#elif defined(CONFIG_STMAC_LAN8700)	/* SMSC LAN8700 */

/* SMSC LAN8700 phy identifier values */
#define LAN8700_PHY_ID		0x0007c0c0u
#define LAN8700_PHY_ID_MASK	0xfffffff0u

#define SPECIAL_MODE_REG	0x12	/* Special Modes Register */
#define PHY_ADDR_MSK		0x001f	/* PHY Address Mask */
#define PHY_ADDR_SHIFT		0	/* PHY Address Mask */

#elif defined(CONFIG_STMAC_LAN8710)	/* SMSC LAN8710/20 */

/* SMSC LAN8710 phy identifier values */
#define LAN8710_PHY_ID		0x0007c0f0u
#define LAN8710_PHY_ID_MASK	0xfffffff0u

#define SPECIAL_MODE_REG	0x12	/* Special Modes Register */
#define PHY_ADDR_MSK		0x001f	/* PHY Address Mask */
#define PHY_ADDR_SHIFT		0	/* PHY Address Mask */

#elif defined(CONFIG_STMAC_DP83865)	/* Nat Semi DP83865 */

/* Nat Semi DP83865 phy identifier values */
#define DP83865_PHY_ID		0x20005c70u
#define DP83865_PHY_ID_MASK	0xfffffff0u

#define PHY_SUP_REG		0x1f	/* PHY Support Register */
#define PHY_ADDR_MSK		0x001f	/* PHY Address Mask */
#define PHY_ADDR_SHIFT		0	/* PHY Address Mask */

#elif defined(CONFIG_STMAC_KSZ8041)	/* Micrel KSZ8041 */

/* Micrel KSZ8041 phy identifier values */
#define KSZ8041_PHY_ID		0x00221512u /* 1512 = A3, 1513 = A4 */
#define KSZ8041_PHY_ID_MASK	0xfffffffeu

#elif defined(CONFIG_STMAC_IP1001)	/* IC+ IP1001 */

/* IC+ IP1001 phy identifier values */
#define IP1001_PHY_ID		0x02430d90u
#define IP1001_PHY_ID_MASK	0xfffffff0u

#elif defined(CONFIG_STMAC_IP101A)	/* IC+ IP101A */

/* IC+ IP101A phy identifier values */
#define IP101A_PHY_ID		0x02430c54u
#define IP101A_PHY_ID_MASK	0xffffffffu

#elif defined(CONFIG_STMAC_78Q2123)	/* TERIDIAN 78Q2123 */

/* TERIDIAN 78Q2123 phy identifier values */
#define TERIDIAN_PHY_ID		0x000e7230u
#define TERIDIAN_PHY_ID_MASK	0xfffffff0u

#elif defined(CONFIG_STMAC_RTL8201E)	/* REALTEK RTL8201E(L) */

/* REALTEK RTL8201E(L) phy identifier values */
#define RTL8201E_PHY_ID		0x001cc815u
#define RTL8201E_PHY_ID_MASK	0xffffffffu

#define PHY_TEST_REG		0x19	/* PHY Support Register */
#define PHY_ADDR_MSK		0x180	/* PHY Address Mask */
#define PHY_ADDR_SHIFT		7	/* PHY Address Mask */

#else
#error Need to define which PHY to use
#endif


/* MII mode */
#define MII_ADVERTISE_PAUSE 0x0400	/* supports the pause command */


#ifndef CONFIG_PHY_LOOPBACK
static int stmac_phy_negotiate (int phy_addr)
{
	uint now, tmp, status;

	status = 0;

	tmp = stmac_mii_read (phy_addr, MII_BMCR);
	tmp |= (BMCR_ANENABLE | BMCR_ANRESTART);
	stmac_mii_write (phy_addr, MII_BMCR, tmp);

	now = get_timer (0);
	while (get_timer (now) < CONFIG_STMAC_AUTONEG_TIMEOUT) {
		status = stmac_mii_read (phy_addr, MII_BMSR);
		if (status & BMSR_ANEGCOMPLETE) {
			break;
		}

		/* Restart auto-negotiation if remote fault */
		if (status & BMSR_RFAULT) {
			printf (STMAC "PHY remote fault detected\n");
			/* Restart auto-negotiation */
			printf (STMAC "PHY restarting auto-negotiation\n");
			stmac_mii_write (phy_addr, MII_BMCR,
					 BMCR_ANENABLE | BMCR_ANRESTART);
		}
	}

	if (!(status & BMSR_ANEGCOMPLETE)) {
		printf (STMAC "PHY auto-negotiate timed out\n");
	}

	if (status & BMSR_RFAULT) {
		printf (STMAC "PHY remote fault detected\n");
	}

	return (1);
}

static unsigned int stmac_phy_check_speed (int phy_addr)
{
	unsigned int status;
	int full_duplex = 0;
	int speed = 0;

	/* Read Status register */
	status = stmac_mii_read (phy_addr, MII_BMSR);

	printf (STMAC);

	/* Check link status.  If 0, default to 100 Mbps. */
	if ((status & BMSR_LSTATUS) == 0) {
		printf ("*Warning* no link detected\n");
		return 1;
	} else {
		int negotiated = stmac_mii_read (phy_addr, MII_LPA);

		if (negotiated & LPA_100FULL) {
			printf ("100Mbs full duplex link detected\n");
			full_duplex = 1;
			speed = 100;
		} else if (negotiated & LPA_100HALF) {
			printf ("100Mbs half duplex link detected\n");
			full_duplex = 0;
			speed = 100;
		} else if (negotiated & LPA_10FULL) {
			printf ("10Mbs full duplex link detected\n");
			full_duplex = 1;
			speed = 10;
		} else {
			printf ("10Mbs half duplex link detected\n");
			full_duplex = 0;
			speed = 10;
		}
	}
	stmac_set_mac_mii_cap (full_duplex, speed);
	return 0;
}
#endif	/* CONFIG_PHY_LOOPBACK */

/* Automatically gets and returns the PHY device */
static unsigned int stmac_phy_get_addr (void)
{
	unsigned int i, id;

	for (i = 0; i < 32; i++) {
		unsigned int phyaddr = (i + 1u) % 32u;
		unsigned int id1 = stmac_mii_read (phyaddr, MII_PHYSID1);
		unsigned int id2 = stmac_mii_read (phyaddr, MII_PHYSID2);
		id  = (id1 << 16) | (id2);
		/* Make sure it is a valid (known) identifier */
#if defined(CONFIG_STMAC_STE10XP)
		if ((id & STE101P_PHY_ID_MASK) == STE101P_PHY_ID) {
			printf (STMAC "STe101P found\n");
			return phyaddr;
		} else if ((id & STE100P_PHY_ID_MASK) == STE100P_PHY_ID) {
			printf (STMAC "STe100P found\n");
			return phyaddr;
		}
#elif defined(CONFIG_STMAC_LAN8700)
		if ((id & LAN8700_PHY_ID_MASK) == LAN8700_PHY_ID) {
			printf (STMAC "SMSC LAN8700 found\n");
			return phyaddr;
		}
#elif defined(CONFIG_STMAC_LAN8710)
		if ((id & LAN8710_PHY_ID_MASK) == LAN8710_PHY_ID) {
			printf (STMAC "SMSC LAN8710/20 found\n");
			return phyaddr;
		}
#elif defined(CONFIG_STMAC_DP83865)
		if ((id & DP83865_PHY_ID_MASK) == DP83865_PHY_ID) {
			printf (STMAC "NS DP83865 found\n");
			return phyaddr;
		}
#elif defined(CONFIG_STMAC_KSZ8041)
		if ((id & KSZ8041_PHY_ID_MASK) == KSZ8041_PHY_ID) {
			printf (STMAC "Micrel KSZ8041 found\n");
			return phyaddr;
		}
#elif defined(CONFIG_STMAC_IP1001)
		if ((id & IP1001_PHY_ID_MASK) == IP1001_PHY_ID) {
			printf (STMAC "IC+ IP1001 found\n");
			return phyaddr;
		}
#elif defined(CONFIG_STMAC_IP101A)
		if ((id & IP101A_PHY_ID_MASK) == IP101A_PHY_ID) {
			printf (STMAC "IC+ IP101A found\n");
			return phyaddr;
		}
#elif defined(CONFIG_STMAC_78Q2123)
		if ((id & TERIDIAN_PHY_ID_MASK) == TERIDIAN_PHY_ID) {
			printf (STMAC "TERIDIAN 78Q2123 found\n");
			return phyaddr;
		}
#elif defined(CONFIG_STMAC_RTL8201E)
		if ((id & RTL8201E_PHY_ID_MASK) == RTL8201E_PHY_ID) {
			printf (STMAC "REALTEK RTL8201E(L) found\n");
			return phyaddr;
		}
#else
#error Need to define which PHY to use
#endif	/* CONFIG_STMAC_STE10XP */
	}

	printf (STMAC "Unable to find a known PHY!\n");

	/* write out the IDs of all PHYs who respond */
	for (i = 0; i < 32; i++) {
		unsigned int phyaddr = i;
		unsigned int id1 = stmac_mii_read (phyaddr, MII_PHYSID1);
		unsigned int id2 = stmac_mii_read (phyaddr, MII_PHYSID2);
		id  = (id1 << 16) | (id2);
		if (id != ~0u) {	/* not all ones */
			printf (STMAC "info: PHY at address=0x%02x has ID=0x%08x\n", phyaddr, id);
		}
	}

	return (-1);
}

static int stmac_phy_init (void)
{
	uint advertised_caps, value;

	/* Obtain the PHY's address/id */
	eth_phy_addr = stmac_phy_get_addr ();
	if (eth_phy_addr < 0)
		return -1;

	/* Now reset the PHY we just found */
	if (miiphy_reset (miidevice, eth_phy_addr)< 0) {
		PRINTK (STMAC "PHY reset failed!\n");
		return -1;
	}

	/* test for H/W address disagreement with the assigned address */
#if defined(CONFIG_STMAC_STE10XP)
	value = stmac_mii_read (eth_phy_addr, MII_XMC);
#elif defined(CONFIG_STMAC_LAN8700) || defined(CONFIG_STMAC_LAN8710)
	value = stmac_mii_read (eth_phy_addr, SPECIAL_MODE_REG);
#elif defined(CONFIG_STMAC_DP83865)
	value = stmac_mii_read (eth_phy_addr, PHY_SUP_REG);
#elif defined(CONFIG_STMAC_RTL8201E)
	value = stmac_mii_read (eth_phy_addr, PHY_TEST_REG);
#elif defined(CONFIG_STMAC_KSZ8041)
	/* The Micrel KSZ8041 does not appear to support
	 * reading the H/W PHY address from any register.  */
#	define CONFIG_STMAC_BYPASS_ADDR_MISMATCH
#elif defined(CONFIG_STMAC_IP1001)
	/* The IC+ IP1001 does not appear to support
	 * reading the H/W PHY address from any register.  */
#	define CONFIG_STMAC_BYPASS_ADDR_MISMATCH
#elif defined(CONFIG_STMAC_IP101A)
	/* The IC+ IP1001 does not appear to support
	 * reading the H/W PHY address from any register.  */
#	define CONFIG_STMAC_BYPASS_ADDR_MISMATCH
#elif defined(CONFIG_STMAC_78Q2123)
	/* The TERIDIAN 78Q2123 does not appear to support
	 * reading the H/W PHY address from any register.  */
#	define CONFIG_STMAC_BYPASS_ADDR_MISMATCH
#else
#error Need to define which PHY to use
#endif
#if !defined(CONFIG_STMAC_BYPASS_ADDR_MISMATCH)
	value = (value & PHY_ADDR_MSK) >> PHY_ADDR_SHIFT;
	if (value != eth_phy_addr) {
		printf (STMAC "PHY address mismatch with hardware (hw %d != %d)\n",
			value,
			eth_phy_addr);
	}
#endif

	/* Read the ANE Advertisement register */
	advertised_caps = stmac_mii_read (eth_phy_addr, MII_ADVERTISE);

	/* Copy our capabilities from MII_BMSR to MII_ADVERTISE */
	value = stmac_mii_read (eth_phy_addr, MII_BMSR);

	/* Set the advertised capabilities */
	if (value & BMSR_100BASE4)
		advertised_caps |= ADVERTISE_100BASE4;
	if (value & BMSR_100FULL)
		advertised_caps |= ADVERTISE_100FULL;
	if (value & BMSR_100HALF)
		advertised_caps |= ADVERTISE_100HALF;
	if (value & BMSR_10FULL)
		advertised_caps |= ADVERTISE_10FULL;
	if (value & BMSR_10HALF)
		advertised_caps |= ADVERTISE_10HALF;

#ifdef CONFIG_STMAC_FLOWCTRL
	advertised_caps |= MII_ADVERTISE_PAUSE;
#else
	advertised_caps &= ~MII_ADVERTISE_PAUSE;
#endif

	/* Update our Auto-Neg Advertisement Register */
	stmac_mii_write (eth_phy_addr, MII_ADVERTISE, advertised_caps);

	/*
	 * For Gigabit capable PHYs, then we will disable the
	 * ability to auto-negotiate at 1000BASE-T (Gigabit).
	 * Once ST's SoCs are capable of Gigabit, then we will review!
	 */
#if defined(CONFIG_STMAC_IP1001)
	value = stmac_mii_read (eth_phy_addr, MII_GBCR);
	value &= ~(GBCR_1000HALF|GBCR_1000FULL);
	stmac_mii_write (eth_phy_addr, MII_GBCR, value);
#endif

#ifdef CONFIG_PHY_LOOPBACK

	/* put the PHY in loop-back mode, if required */
	printf ( STMAC "Forcing PHY loop-back at full-duplex, 100Mbps\n");
	value = stmac_mii_read (eth_phy_addr, MII_BMCR);
	value |= BMCR_LOOPBACK;		/* enable loop-back mode (in the PHY) */
	value &= ~BMCR_ANENABLE;	/* disable auto-negotiation */
	value &= ~BMCR_SPEED_MASK;	/* clear all speed bits */
	value |= BMCR_SPEED100;		/* set speed to 100Mbps */
	value |= BMCR_FULLDPLX;		/* enable full-duplex */
	stmac_mii_write (eth_phy_addr, MII_BMCR, value);
	/* ensure the write completes! */
	(void)stmac_mii_read (eth_phy_addr, MII_BMCR);

	/* set the MAC capabilities appropriately */
	stmac_set_mac_mii_cap (1, 100);	/* 100Mbps, full-duplex */

#else	/* CONFIG_PHY_LOOPBACK */

	/* auto-negotiate with remote link partner */
	stmac_phy_negotiate (eth_phy_addr);
	stmac_phy_check_speed (eth_phy_addr);

#endif	/* CONFIG_PHY_LOOPBACK */

	return 0;
}


/* ----------------------------------------------------------------------------
				 MII Interface
   ---------------------------------------------------------------------------*/

static int stmac_mii_poll_busy (void)
{
	/* arm simple, non interrupt dependent timer */
	ulong now = get_timer (0);
	while (get_timer (now) < CONFIG_STMAC_MII_POLL_BUSY_DELAY) {
		if (!(STMAC_READ (MAC_MII_ADDR) & MAC_MII_ADDR_BUSY)) {
			return 1;
		}
	}
	printf (STMAC "stmac_mii_busy timeout\n");
	return (0);
}

static void stmac_mii_write (int phy_addr, int reg, int value)
{
	int mii_addr;

	/* Select register */
	mii_addr =
		((phy_addr & MAC_MII_ADDR_PHY_MASK) << MAC_MII_ADDR_PHY_SHIFT)
		| ((reg & MAC_MII_ADDR_REG_MASK) << MAC_MII_ADDR_REG_SHIFT)
		| MAC_MII_ADDR_WRITE | MAC_MII_ADDR_BUSY;

	stmac_mii_poll_busy ();

	/* Set the MII address register to write */
	STMAC_WRITE (value, MAC_MII_DATA);
	STMAC_WRITE (mii_addr, MAC_MII_ADDR);

	stmac_mii_poll_busy ();

#if defined(CONFIG_STMAC_STE10XP)	/* ST STE10xP PHY */
	/* QQQ: is the following actually needed ? */
	(void) stmac_mii_read (phy_addr, reg);
#endif	/* CONFIG_STMAC_STE10XP */
}

static unsigned int stmac_mii_read (int phy_addr, int reg)
{
	int mii_addr, val;

	mii_addr =
		((phy_addr & MAC_MII_ADDR_PHY_MASK) << MAC_MII_ADDR_PHY_SHIFT)
		| ((reg & MAC_MII_ADDR_REG_MASK) << MAC_MII_ADDR_REG_SHIFT)
		| MAC_MII_ADDR_BUSY;

	/* Select register */
	stmac_mii_poll_busy ();

	STMAC_WRITE (mii_addr, MAC_MII_ADDR);

	stmac_mii_poll_busy ();

	/* Return read value */
	val = STMAC_READ (MAC_MII_DATA);
	return val;
}

/* define external interface to mii, through miiphy_register() */
static int stmac_miiphy_read (char *devname, unsigned char addr, unsigned char reg, unsigned short *value)
{
	*value = stmac_mii_read (addr, reg);
#if 0
	printf("QQQ: %s(addr=%u, reg=%u) --> value=0x%04x)\n", __FUNCTION__, addr, reg, *value);
#endif
	return 0;
}

static int stmac_miiphy_write (char *devname, unsigned char addr, unsigned char reg, unsigned short value)
{
#if 0
	printf("QQQ: %s(addr=%u, reg=%u, value=0x%04x)\n", __FUNCTION__, addr, reg, value);
#endif
	stmac_mii_write (addr, reg, value);
	return 0;
}

/* ----------------------------------------------------------------------------
				 MAC CORE Interface
   ---------------------------------------------------------------------------*/

#ifdef DEBUG
static void gmac_dump_regs(void)
{
	int i;
	const char fmt[] =
		"\tReg No. %2d (offset 0x%03x): 0x%08x\n";
	const char header[] =
		"\t----------------------------------------------\n"
		"\t  %s registers (base addr = 0x%8x)\n"
		"\t----------------------------------------------\n";

	printf (header, "MAC CORE", (unsigned int)CFG_STM_STMAC_BASE);
	for (i = 0; i < 18; i++) {
		int offset = i * 4;
		printf(fmt, i, offset, STMAC_READ (offset));
	}

	printf (header, "MAC DMA", (unsigned int)CFG_STM_STMAC_BASE);
	for (i = 0; i < 9; i++) {
		int offset = i * 4;
		printf (fmt, i, (DMA_BUS_MODE + offset),
			STMAC_READ (DMA_BUS_MODE + offset));
	}

#ifdef CONFIG_DRIVER_NET_STM_GMAC
	printf ("\tSTBus bridge register (0x%08x) = 0x%08x\n",
		(unsigned int)(CFG_STM_STMAC_BASE + STBUS_BRIDGE_OFFSET),
		STMAC_READ (STBUS_BRIDGE_OFFSET));
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */
}
#endif	/* DEBUG */

static int stmac_get_mac_addr (unsigned char *addr)
{
	unsigned int hi_addr, lo_addr;

	/* Read the MAC address from the hardware */
	hi_addr = (unsigned int) STMAC_READ (MAC_ADDR_HIGH);
	lo_addr = (unsigned int) STMAC_READ (MAC_ADDR_LOW);

	if (((hi_addr&0x0000FFFFUL) == 0x0000FFFFUL) && (lo_addr == 0xFFFFFFFF))
		return 0;

	/* Extract the MAC address from the high and low words */
	addr[0] = lo_addr & 0xffu;
	addr[1] = (lo_addr >> 8) & 0xffu;
	addr[2] = (lo_addr >> 16) & 0xffu;
	addr[3] = (lo_addr >> 24) & 0xffu;
	addr[4] = hi_addr & 0xffu;
	addr[5] = (hi_addr >> 8) & 0xffu;

	return 1;
}

static void stmac_mac_enable (void)
{
	unsigned int value = (unsigned int) STMAC_READ (MAC_CONTROL);

	PRINTK (STMAC "MAC RX/TX enabled\n");

	/* set: TE (transmitter enable), RE (receive enable) */
	value |= (MAC_CONTROL_TE | MAC_CONTROL_RE);

#ifdef CONFIG_DRIVER_NETSTMAC
	/* and RA (receive all mode) */
//	value |= MAC_CONTROL_RA;	/* QQQ: suspect we can delete this */
#endif	/* CONFIG_DRIVER_NETSTMAC */

	STMAC_WRITE (value, MAC_CONTROL);
	return;
}

static void stmac_mac_disable (void)
{
	unsigned int value = (unsigned int) STMAC_READ (MAC_CONTROL);

	PRINTK (STMAC "MAC RX/TX disabled\n");

	value &= ~(MAC_CONTROL_TE | MAC_CONTROL_RE);

#ifdef CONFIG_DRIVER_NETSTMAC
//	value &= ~MAC_CONTROL_RA;	/* QQQ: suspect we can delete this */
#endif	/* CONFIG_DRIVER_NETSTMAC */

	STMAC_WRITE (value, MAC_CONTROL);
	return;
}

static void stmac_set_rx_mode (void)
{
	unsigned int value = (unsigned int) STMAC_READ (MAC_CONTROL);

#ifdef CONFIG_DRIVER_NETSTMAC
	PRINTK (STMAC "MAC address perfect filtering only mode\n");
	value &= ~(MAC_CONTROL_PM | MAC_CONTROL_PR | MAC_CONTROL_IF |
		   MAC_CONTROL_HO | MAC_CONTROL_HP);
#endif	/* CONFIG_DRIVER_NETSTMAC */

	STMAC_WRITE (0x0, MAC_HASH_HIGH);
	STMAC_WRITE (0x0, MAC_HASH_LOW);

	STMAC_WRITE (value, MAC_CONTROL);

	return;
}

static void stmac_set_mac_mii_cap (int full_duplex, unsigned int speed)
{
	unsigned int flow = (unsigned int) STMAC_READ (MAC_FLOW_CONTROL);
	unsigned int ctrl = (unsigned int) STMAC_READ (MAC_CONTROL);

	PRINTK (STMAC "%s(full_duplex=%d, speed=%u)\n", __FUNCTION__, full_duplex, speed);

	if (!(full_duplex)) {	/* Half Duplex */
#ifdef CONFIG_DRIVER_NETSTMAC
		flow &= ~(MAC_FLOW_CONTROL_FCE | MAC_FLOW_CONTROL_PT_MASK |
			  MAC_FLOW_CONTROL_PCF);
		ctrl &= ~MAC_CONTROL_F;
		ctrl |= MAC_CONTROL_DRO;
#endif	/* CONFIG_DRIVER_NETSTMAC */
#ifdef CONFIG_DRIVER_NET_STM_GMAC
		flow &= ~(MAC_FLOW_CONTROL_TFE | MAC_FLOW_CONTROL_PT_MASK |
			  MAC_FLOW_CONTROL_RFE);
		ctrl &= ~MAC_CONTROL_DM;
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */
	} else {		/* Full Duplex */
#ifdef CONFIG_DRIVER_NETSTMAC
		ctrl |= MAC_CONTROL_F;
		ctrl &= ~MAC_CONTROL_DRO;
		flow |= MAC_FLOW_CONTROL_FCE | MAC_FLOW_CONTROL_PCF;
#endif	/* CONFIG_DRIVER_NETSTMAC */
#ifdef CONFIG_DRIVER_NET_STM_GMAC
		ctrl |= MAC_CONTROL_DM;
		flow |= MAC_FLOW_CONTROL_TFE | MAC_FLOW_CONTROL_RFE;
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */
		flow |= (MAX_PAUSE_TIME << MAC_FLOW_CONTROL_PT_SHIFT);
	}

#ifdef CONFIG_DRIVER_NETSTMAC
	/* use MII */
	ctrl &= ~MAC_CONTROL_PS;
#endif	/* CONFIG_DRIVER_NETSTMAC */

#ifdef CONFIG_DRIVER_NET_STM_GMAC
	switch (speed) {
	case 1000:		/* Gigabit */
		ctrl &= ~MAC_CONTROL_PS;
		break;
	case 100:		/* 100Mbps */
		ctrl |= MAC_CONTROL_PS | MAC_CONTROL_FES;
		break;
	case 10:		/* 10Mbps */
		ctrl |= MAC_CONTROL_PS;
		ctrl &= ~MAC_CONTROL_FES;
		break;
	}
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

	STMAC_WRITE (flow, MAC_FLOW_CONTROL);
	STMAC_WRITE (ctrl, MAC_CONTROL);
	
	/* ensure the SoC knows the correct speed */
	stmac_set_mac_speed (speed);

	return;
}

/* This function provides the initial setup of the MAC controller */
static void stmac_mac_core_init (void)
{
	unsigned int value;

	/* Set the MAC control register with our default value */
	value = (unsigned int) STMAC_READ (MAC_CONTROL);
	value |= MAC_CORE_INIT;
	STMAC_WRITE (value, MAC_CONTROL);

#ifdef CONFIG_DRIVER_NET_STM_GMAC
	/* STBus Bridge Configuration */
	STMAC_WRITE(STBUS_BRIDGE_MAGIC, STBUS_BRIDGE_OFFSET);

	/* Freeze MMC counters */
	STMAC_WRITE(MMC_COUNTER_FREEZE, MMC_CONTROL);

	/* Mask all interrupts */
	STMAC_WRITE(~0u, MAC_INT_MASK);
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

	return;
}

/* ----------------------------------------------------------------------------
 *  			DESCRIPTORS functions
 * ---------------------------------------------------------------------------*/

#ifdef DEBUG
static void display_dma_desc_ring (volatile const stmac_dma_des * p, int size)
{
	int i;
	for (i = 0; i < size; i++)
		printf ("\t%d [0x%x]: "
			"desc0=0x%08x, desc1=0x%08x, buffer1=0x%08x\n",
			i, (unsigned int) &p[i].des01.u.des0,
			p[i].des01.u.des0, p[i].des01.u.des1, p[i].des2);
}
#endif	/* DEBUG */

static void init_rx_desc (volatile stmac_dma_des * p,
	unsigned int ring_size, void **buffers)
{
	int i;

	for (i = 0; i < ring_size; i++) {
		p->des01.u.des0 = p->des01.u.des1 = 0;
		p->des01.rx.own = 1;
		p->des01.rx.buffer1_size = PKTSIZE_ALIGN;
		p->des01.rx.disable_ic = 1;
		if (i == ring_size - 1)
			p->des01.rx.end_ring = 1;
		p->des2 = ((void *) (PHYSADDR (buffers[i])));
		p->des3 = NULL;
		p++;
	}
	return;
}

static void init_tx_desc (volatile stmac_dma_des * p, unsigned int ring_size)
{
	int i;

	for (i = 0; i < ring_size; i++) {
		p->des01.u.des0 = p->des01.u.des1 = 0;
		if (i == ring_size - 1)
			p->des01.tx.end_ring = 1;
		p->des2 = NULL;
		p->des3 = NULL;
		p++;
	}
	return;
}

/* Allocate and init the TX and RX descriptors rings.
 * The driver uses the 'implicit' scheme for implementing the TX/RX DMA
 * linked lists. */

static void init_dma_desc_rings (void)
{
	int i;

	PRINTK (STMAC "allocate and init the DMA RX/TX lists\n");

	/* Clean out uncached buffers */
	flush_cache ((unsigned long)&dma, sizeof (struct dma_t));

	/* Allocate memory for the DMA RX/TX buffer descriptors */
	dma_rx = (volatile stmac_dma_des *) P2SEGADDR (&dma.desc_rx[0]);
	dma_tx = (volatile stmac_dma_des *) P2SEGADDR (&dma.desc_tx[0]);

	cur_rx = 0;

	if ((dma_rx == NULL) || (dma_tx == NULL) ||
	    (((u32)dma_rx % L1_CACHE_BYTES) != 0) ||
	    (((u32)dma_tx % L1_CACHE_BYTES) != 0)) {
		printf (STMAC "ERROR allocating the DMA Tx/Rx desc\n");
		return;
	}

	for (i = 0; i < CONFIG_DMA_RX_SIZE; i++)
		rx_packets[i] = (void *) P2SEGADDR (dma.rx_buff + (PKTSIZE_ALIGN * i));

	/* Initialize the contents of the DMA buffers */
	init_rx_desc (dma_rx, CONFIG_DMA_RX_SIZE, rx_packets);
	init_tx_desc (dma_tx, CONFIG_DMA_TX_SIZE);

#ifdef DEBUG
	printf (STMAC "RX descriptor ring:\n");
	display_dma_desc_ring (dma_rx, CONFIG_DMA_RX_SIZE);
	printf (STMAC "TX descriptor ring:\n");
	display_dma_desc_ring (dma_tx, CONFIG_DMA_TX_SIZE);
#endif

	return;
}

/* Release and free the descriptor resources. */
static void free_dma_desc_resources (void)
{
	dma_tx = NULL;
	dma_rx = NULL;
	return;
}

/* ----------------------------------------------------------------------------
				DMA FUNCTIONS
 * ---------------------------------------------------------------------------*/

/* DMA SW reset.
 *  NOTE1: the MII_TxClk and the MII_RxClk must be active before this
 *	   SW reset otherwise the MAC core won't exit the reset state.
 *  NOTE2: after a SW reset all interrupts are disabled */

static void stmac_dma_reset (void)
{
	unsigned int value;

	value = (unsigned int) STMAC_READ (DMA_BUS_MODE);
	value |= DMA_BUS_MODE_SFT_RESET;

	STMAC_WRITE (value, DMA_BUS_MODE);

	while ((STMAC_READ (DMA_BUS_MODE) & DMA_BUS_MODE_SFT_RESET)) {
	}

	return;
}

/* START/STOP the DMA TX/RX processes */
static void stmac_dma_start_tx (void)
{
	unsigned int value;

	value = (unsigned int) STMAC_READ (DMA_CONTROL);
	value |= DMA_CONTROL_ST;
	STMAC_WRITE (value, DMA_CONTROL);

	return;
}

static void stmac_dma_stop_tx (void)
{
	unsigned int value;

	value = (unsigned int) STMAC_READ (DMA_CONTROL);
	value &= ~DMA_CONTROL_ST;
	STMAC_WRITE (value, DMA_CONTROL);

	return;
}
static void stmac_dma_start_rx (void)
{
	unsigned int value;

	value = (unsigned int) STMAC_READ (DMA_CONTROL);
	value |= DMA_CONTROL_SR;
	STMAC_WRITE (value, DMA_CONTROL);

	return;
}

static void stmac_dma_stop_rx (void)
{
	unsigned int value;

	value = (unsigned int) STMAC_READ (DMA_CONTROL);
	value &= ~DMA_CONTROL_SR;
	STMAC_WRITE (value, DMA_CONTROL);

	return;
}

static void stmac_eth_stop_tx (void)
{

	stmac_dma_stop_tx ();

	return;
}

/* The DMA init function performs:
 * - the DMA RX/TX SW descriptors initialization
 * - the DMA HW controller initialization
 * NOTE: the DMA TX/RX processes will be started in the 'open' method. */

static int stmac_dma_init (void)
{

	PRINTK (STMAC "DMA Core setup\n");

	/* Enable Application Access by writing to DMA CSR0 */
	STMAC_WRITE (DMA_BUS_MODE_DEFAULT |
		     (stmac_default_pbl () << DMA_BUS_MODE_PBL_SHIFT),
		     DMA_BUS_MODE);

	/* Disable interrupts */
	STMAC_WRITE (0, DMA_INTR_ENA);

	/* The base address of the RX/TX descriptor */
	STMAC_WRITE (PHYSADDR (dma_tx), DMA_TX_BASE_ADDR);
	STMAC_WRITE (PHYSADDR (dma_rx), DMA_RCV_BASE_ADDR);

	return (0);
}

static int check_tx_error_summary (const stmac_dma_des * const p)
{
	int ret = 0;	/* assume there are no errors */

	if (unlikely(p->des01.tx.error_summary)) {
		if (unlikely(p->des01.tx.loss_carrier)) {
			PRINTK(STMAC "TX: loss_carrier error\n");
		}
		if (unlikely(p->des01.tx.no_carrier)) {
			PRINTK(STMAC "TX: no_carrier error\n");
		}
		if (unlikely(p->des01.tx.late_collision)) {
			PRINTK(STMAC "TX: late_collision error\n");
		}
		if (unlikely(p->des01.tx.excessive_collisions)) {
			PRINTK(STMAC "TX: excessive_collisions\n");
		}
		if (unlikely(p->des01.tx.excessive_deferral)) {
			PRINTK(STMAC "TX: excessive_deferral\n");
		}
		if (unlikely(p->des01.tx.underflow_error)) {
			PRINTK(STMAC "TX: underflow error\n");
		}
#ifdef CONFIG_DRIVER_NET_STM_GMAC
		if (unlikely(p->des01.tx.jabber_timeout)) {
			PRINTK(STMAC "TX: jabber_timeout error\n");
		}
		if (unlikely(p->des01.tx.frame_flushed)) {
			PRINTK(STMAC "TX: frame_flushed error\n");
		}
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */
		ret = -1;
	}

	if (unlikely(p->des01.tx.deferred)) {
		PRINTK(STMAC "TX: deferred\n");
		ret = -1;
	}
#ifdef CONFIG_DRIVER_NETSTMAC
	if (unlikely(p->des01.tx.heartbeat_fail)) {
		PRINTK(STMAC "TX: heartbeat_fail\n");
		ret = -1;
	}
#endif	/* CONFIG_DRIVER_NETSTMAC */
#ifdef CONFIG_DRIVER_NET_STM_GMAC
	if (unlikely(p->des01.tx.payload_error)) {
		PRINTK(STMAC "TX Addr/Payload csum error\n");
		ret = -1;
	}
	if (unlikely(p->des01.tx.ip_header_error)) {
		PRINTK(STMAC "TX IP header csum error\n");
		ret = -1;
	}
	if (p->des01.tx.vlan_frame) {
		PRINTK(STMAC "TX: VLAN frame\n");
	}
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

#ifdef DEBUG
	if (ret != 0) {
		printf(STMAC "%s() returning %d\n", __FUNCTION__, ret);
	}
#endif

	return (ret);
}

static int check_rx_error_summary (const stmac_dma_des * const p)
{
	int ret = 0;	/* assume there are no errors */

	if (unlikely(p->des01.rx.error_summary)) {
		if (unlikely(p->des01.rx.descriptor_error)) {
			/* frame doesn't fit within the current descriptor. */
			PRINTK(STMAC "RX: descriptor error\n");
		}
		if (unlikely(p->des01.rx.crc_error)) {
			PRINTK(STMAC "RX: CRC error\n");
		}
#ifdef CONFIG_DRIVER_NETSTMAC
		if (unlikely(p->des01.rx.partial_frame_error)) {
			PRINTK(STMAC "RX: partial_frame_error\n");
		}
		if (unlikely(p->des01.rx.runt_frame)) {
			PRINTK(STMAC "RX: runt_frame\n");
		}
		if (unlikely(p->des01.rx.frame_too_long)) {
			PRINTK(STMAC "RX: frame_too_long\n");
		}
		if (unlikely(p->des01.rx.collision)) {
			PRINTK(STMAC "RX: collision\n");
		}
#endif	/* CONFIG_DRIVER_NETSTMAC */
#ifdef CONFIG_DRIVER_NET_STM_GMAC
		if (unlikely(p->des01.rx.overflow_error)) {
			PRINTK(STMAC "RX: Overflow error\n");
		}
		if (unlikely(p->des01.rx.late_collision)) {
			PRINTK(STMAC "RX: late_collision\n");
		}
		if (unlikely(p->des01.rx.receive_watchdog)) {
			PRINTK(STMAC "RX: receive_watchdog error\n");
		}
		if (unlikely(p->des01.rx.error_gmii)) {
			PRINTK(STMAC "RX: GMII error\n");
		}
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */
		ret = -1;
	}

	if (unlikely(p->des01.rx.length_error)) {
		PRINTK(STMAC "RX: length_error error\n");
		ret = -1;
	}
	if (unlikely(p->des01.rx.dribbling)) {
		PRINTK(STMAC "RX: dribbling error\n");
		ret = -1;
	}
#ifdef CONFIG_DRIVER_NET_STM_GMAC
	if (unlikely(p->des01.rx.filtering_fail)) {
		PRINTK(STMAC "RX: filtering_fail error\n");
		ret = -1;
	}
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */
#ifdef CONFIG_DRIVER_NETSTMAC
	if (unlikely(p->des01.rx.last_descriptor == 0)) {
		PRINTK(STMAC "RX: Oversized Ethernet "
			"frame spanned multiple buffers\n");
		ret = -1;
	}
	if (unlikely(p->des01.rx.mii_error)) {
		PRINTK(STMAC "RX: MII error\n");
		ret = -1;
	}
#endif	/* CONFIG_DRIVER_NETSTMAC */

#ifdef DEBUG
	if (ret != 0) {
		printf(STMAC "%s() returning %d\n", __FUNCTION__, ret);
	}
#endif

	return (ret);
}

static int stmac_eth_tx (volatile uchar * data, int len)
{
	volatile stmac_dma_des *p = dma_tx;
	uint now = get_timer (0);
	uint status = 0;
	u32 end_ring;

	while (p->des01.tx.own
	       && (get_timer (now) < CONFIG_STMAC_TX_TIMEOUT)) {
		;
	}

	if (p->des01.tx.own) {
		printf (STMAC "tx timeout - no desc available\n");
		return -1;
	}

	/* Make sure data is in real memory */
	flush_cache ((ulong) data, len);
	p->des2 = (void *) PHYSADDR (data);

	/* Clean and set the TX descriptor */
	end_ring = p->des01.tx.end_ring;
	p->des01.u.des0 = p->des01.u.des1 = 0;
	p->des01.tx.interrupt = 1;
	p->des01.tx.first_segment = 1;
	p->des01.tx.last_segment = 1;
	p->des01.tx.end_ring = end_ring;
	p->des01.tx.buffer1_size = len;
	p->des01.tx.own = 1;

#ifdef DEBUG
	PRINTK ("\n" STMAC "TX %s(data=0x%08x, len=%d)\n", __FUNCTION__, data, len);
	display_dma_desc_ring (dma_tx, CONFIG_DMA_TX_SIZE);
#endif

	/* CSR1 enables the transmit DMA to check for new descriptor */
	STMAC_WRITE (DMA_STATUS_TI, DMA_STATUS);
	STMAC_WRITE (1, DMA_XMT_POLL_DEMAND);

	now = get_timer (0);
	while (get_timer (now) < CONFIG_STMAC_TX_TIMEOUT) {
		status = STMAC_READ (DMA_STATUS);
		if (status & DMA_STATUS_TI)
			break;
	}
	if (!(status & DMA_STATUS_TI)) {
		printf (STMAC "tx timeout\n");
	}

	return check_tx_error_summary ((stmac_dma_des *)p);
}

/* Receive function */
static void stmac_eth_rx (void)
{
	int frame_len = 0;
	volatile stmac_dma_des *drx;

	/* select the RX descriptor to use */
	drx = dma_rx + cur_rx;

	if ((cur_rx < 0) || (cur_rx >= CONFIG_DMA_RX_SIZE)) {
		printf (STMAC "%s: [dma drx = 0x%x, cur_rx=%d]\n", __FUNCTION__,
			(unsigned int) drx, cur_rx);
#ifdef DEBUG
		display_dma_desc_ring (dma_rx, CONFIG_DMA_RX_SIZE);
#endif	/* DEBUG */
	}

	if (!(drx->des01.rx.own) && (drx->des01.rx.last_descriptor)) {
#ifdef DEBUG
		PRINTK (STMAC "RX descriptor ring:\n");
		display_dma_desc_ring (dma_rx, CONFIG_DMA_RX_SIZE);
#endif

		/* Check if the frame was not successfully received */
		if (check_rx_error_summary ((stmac_dma_des *)drx) < 0) {
			drx->des01.rx.own = 1;
		} else if (drx->des01.rx.first_descriptor
			   && drx->des01.rx.last_descriptor) {

			/* FL (frame length) indicates the length in byte including
			 * the CRC */
			frame_len = drx->des01.rx.frame_length;
			if ((frame_len >= 0) && (frame_len <= PKTSIZE_ALIGN)) {
#if defined(DEBUG) || defined(CONFIG_PHY_LOOPBACK) || defined(DUMP_ENCAPSULATION_HEADER)
				const unsigned char *p = rx_packets[cur_rx];
				printf("\nRX[%d]:  0x%08x ", cur_rx, p);
				printf("DA=%02x:%02x:%02x:%02x:%02x:%02x",
					p[0], p[1], p[2], p[3], p[4], p[5]);
				p+=6;
				printf(" SA=%02x:%02x:%02x:%02x:%02x:%02x",
					p[0], p[1], p[2], p[3], p[4], p[5]);
				p+=6;
				printf(" Type=%04x\n", p[0]<<8|p[1]);
#endif
				memcpy ((void*)NetRxPackets[0], rx_packets[cur_rx],
					frame_len);
				NetReceive (NetRxPackets[0], frame_len);
			} else {
				printf (STMAC "%s: Framelen %d too long\n",
					__FUNCTION__, frame_len);
			}
			drx->des01.rx.own = 1;
#ifdef DEBUG
			PRINTK (STMAC "%s: frame received \n", __FUNCTION__);
#endif
		} else {
			printf (STMAC "%s: very long frame received\n",
				__FUNCTION__);
		}

		/* advance to the next RX descriptor (for next time) */
		if (drx->des01.rx.end_ring)
			cur_rx = 0;	/* wrap, to first */
		else
			cur_rx++;	/* advance to next */

	} else {
		STMAC_WRITE (1, DMA_RCV_POLL_DEMAND);	/* request input */
	}
	return;
}

static int stmac_get_ethaddr (bd_t * bd)
{
	int env_size, rom_valid, env_present = 0, reg;
	char *s = NULL, *e, es[] = "11:22:33:44:55:66";
	char s_env_mac[64];
	uchar v_env_mac[6], v_rom_mac[6], *v_mac;

	env_size = getenv_r ("ethaddr", s_env_mac, sizeof (s_env_mac));
	if ((env_size > 0) && (env_size < sizeof (es))) {	/* exit if env is bad */
		printf ("\n*** ERROR: ethaddr is not set properly!!\n");
		return (-1);
	}

	if (env_size > 0) {
		env_present = 1;
		s = s_env_mac;
	}

	for (reg = 0; reg < 6; ++reg) {	/* turn string into mac value */
		v_env_mac[reg] = s ? simple_strtoul (s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}

	rom_valid = stmac_get_mac_addr (v_rom_mac);	/* get ROM mac value if any */

	if (!env_present) {	/* if NO env */
		if (rom_valid) {	/* but ROM is valid */
			v_mac = v_rom_mac;
			sprintf (s_env_mac, "%02X:%02X:%02X:%02X:%02X:%02X",
				 v_mac[0], v_mac[1], v_mac[2], v_mac[3],
				 v_mac[4], v_mac[5]);
			setenv ("ethaddr", s_env_mac);
		} else {	/* no env, bad ROM */
			printf ("\n*** ERROR: ethaddr is NOT set !!\n");
			return (-1);
		}
	} else {		/* good env, don't care ROM */
		v_mac = v_env_mac;	/* always use a good env over a ROM */
	}

	if (env_present && rom_valid) {	/* if both env and ROM are good */
		if (memcmp (v_env_mac, v_rom_mac, 6) != 0) {
			printf ("\nWarning: MAC addresses don't match:\n");
			printf ("\tHW MAC address:  "
				"%02X:%02X:%02X:%02X:%02X:%02X\n",
				v_rom_mac[0], v_rom_mac[1],
				v_rom_mac[2], v_rom_mac[3],
				v_rom_mac[4], v_rom_mac[5]);
			printf ("\t\"ethaddr\" value: "
				"%02X:%02X:%02X:%02X:%02X:%02X\n",
				v_env_mac[0], v_env_mac[1],
				v_env_mac[2], v_env_mac[3],
				v_env_mac[4], v_env_mac[5]);
		}
	}
	memcpy (bd->bi_enetaddr, v_mac, 6);	/* update global address to match env (allows env changing) */
	stmac_set_mac_addr (v_mac);	/* use old function to update default */
	printf ("Using MAC Address %02X:%02X:%02X:%02X:%02X:%02X\n", v_mac[0],
		v_mac[1], v_mac[2], v_mac[3], v_mac[4], v_mac[5]);
	return (0);
}

static int stmac_reset_eth (bd_t * bd)
{
	int err;

	/* MAC Software reset */
	stmac_dma_reset ();		/* Must be done early  */

	/* set smc_mac_addr, and sync it with u-boot globals */
	err = stmac_get_ethaddr (bd);

	if (err < 0) {
		/* hack to make error stick! upper code will abort if not set */
		memset (bd->bi_enetaddr, 0, 6);
		/* upper code ignores return value, but NOT bi_enetaddr */
		return (-1);
	}

	if (stmac_phy_init () < 0) {
		printf (STMAC "ERROR: no PHY detected\n");
		return -1;
	}

	init_dma_desc_rings ();

	stmac_mac_core_init ();
	stmac_dma_init ();

	stmac_set_rx_mode ();

	stmac_mac_enable ();

	stmac_dma_start_rx ();
	stmac_dma_start_tx ();

#ifdef DEBUG
	gmac_dump_regs ();
#endif

	STMAC_WRITE (1, DMA_RCV_POLL_DEMAND);	/* request input */

	return (0);
}

extern int eth_init (bd_t * bd)
{
	PRINTK (STMAC "entering %s()\n", __FUNCTION__);
	return stmac_reset_eth (bd);
}

extern void eth_halt (void)
{
	PRINTK (STMAC "entering %s()\n", __FUNCTION__);

	/* Reset the TX/RX processes */
	stmac_dma_stop_rx ();
	stmac_eth_stop_tx ();

	/* Disable the MAC core */
	stmac_mac_disable ();

	/* Free buffers */
	free_dma_desc_resources ();
}

/* Get a data block via Ethernet */
extern int eth_rx (void)
{
	stmac_eth_rx ();
	return 1;
}

/* Send a data block via Ethernet. */
extern int eth_send (volatile void *packet, int length)
{
	PRINTK (STMAC "entering %s()\n", __FUNCTION__);
#if defined(DEBUG) || defined(CONFIG_PHY_LOOPBACK) || defined(DUMP_ENCAPSULATION_HEADER)
	const unsigned char * p = (const unsigned char*)packet;
	printf("TX   :  0x%08x ", p);
	printf("DA=%02x:%02x:%02x:%02x:%02x:%02x",
		p[0], p[1], p[2], p[3], p[4], p[5]);
	p+=6;
	printf(" SA=%02x:%02x:%02x:%02x:%02x:%02x",
		p[0], p[1], p[2], p[3], p[4], p[5]);
	p+=6;
	printf(" Type=%04x\n", p[0]<<8|p[1]);
#endif

	return stmac_eth_tx (packet, length);
}

#endif /* CONFIG_CMD_NET */

extern int stmac_miiphy_initialize(bd_t *bis)
{
	PRINTK (STMAC "entering %s()\n", __FUNCTION__);
#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	miiphy_register(miidevice, stmac_miiphy_read, stmac_miiphy_write);
#endif
	return 0;
}

#endif /* CONFIG_DRIVER_NETSTMAC || CONFIG_DRIVER_NET_STM_GMAC */

