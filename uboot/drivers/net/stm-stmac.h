/*
 *  Copyright (c) 2006-2008  STMicroelectronics Limited
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
 *            Giuseppe Cavallaro <peppe.cavallaro@st.com>
 */


/*----------------------------------------------------------------------------
 *			CORE MAC Block
 *---------------------------------------------------------------------------*/

/* STMAC Control & Status Register (CSR) offsets */
#ifdef CONFIG_DRIVER_NETSTMAC
#define MAC_CONTROL			0x00000000	/* MAC Control */
#define MAC_ADDR_HIGH			0x00000004	/* MAC Address High */
#define MAC_ADDR_LOW			0x00000008	/* MAC Address Low */
#define MAC_HASH_HIGH			0x0000000c	/* Multicast Hash Table High */
#define MAC_HASH_LOW			0x00000010	/* Multicast Hash Table Low */
#define MAC_MII_ADDR			0x00000014	/* MII Address */
#define MAC_MII_DATA			0x00000018	/* MII Data */
#define MAC_FLOW_CONTROL		0x0000001c	/* Flow Control */
#endif	/* CONFIG_DRIVER_NETSTMAC */
/* GMAC Control & Status Register (CSR) offsets */
#ifdef CONFIG_DRIVER_NET_STM_GMAC
#define MAC_CONTROL			0x00000000	/* MAC Configuration */
//#define MAC_FRAME_FILTER		0x00000004	/* Frame Filter */
#define MAC_HASH_HIGH			0x00000008	/* Multicast Hash Table High */
#define MAC_HASH_LOW			0x0000000c	/* Multicast Hash Table Low */
#define MAC_MII_ADDR			0x00000010	/* MII Address */
#define MAC_MII_DATA			0x00000014	/* MII Data */
#define MAC_FLOW_CONTROL		0x00000018	/* Flow Control */
#define MAC_INT_MASK			0x0000003c	/* Interrupt Mask Register */
#define MAC_ADDR_HIGH			0x00000040	/* MAC Address 0 High */
#define MAC_ADDR_LOW			0x00000044	/* MAC Address 0 Low */
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

/* STMAC Control Register defines */
#ifdef CONFIG_DRIVER_NETSTMAC
#define MAC_CONTROL_RA			0x80000000	/* Receive All Mode */
//#define MAC_CONTROL_BLE		0x40000000	/* Endian Mode */
#define MAC_CONTROL_HBD			0x10000000	/* Heartbeat Disable */
#define MAC_CONTROL_PS			0x08000000	/* Port Select 0:MIII, 1:ENDEC */
#define MAC_CONTROL_DRO			0x00800000	/* Disable Receive Own */
//#define MAC_CONTROL_EXT_LOOPBACK	0x00400000	/* Reserved (ext loopback?) */
//#define MAC_CONTROL_OM		0x00200000	/* Loopback Operating Mode */
#define MAC_CONTROL_F			0x00100000	/* Full Duplex Mode */
#define MAC_CONTROL_PM			0x00080000	/* Pass All Multicast */
#define MAC_CONTROL_PR			0x00040000	/* Promiscuous Mode */
#define MAC_CONTROL_IF			0x00020000	/* Inverse Filtering */
//#define MAC_CONTROL_PB		0x00010000	/* Pass Bad Frames */
#define MAC_CONTROL_HO			0x00008000	/* Hash Only Filtering Mode */
#define MAC_CONTROL_HP			0x00002000	/* Hash/Perfect Filtering Mode */
//#define MAC_CONTROL_LCC		0x00001000	/* Late Collision Control */
//#define MAC_CONTROL_DBF		0x00000800	/* Disable Broadcast Frames */
//#define MAC_CONTROL_DRTY		0x00000400	/* Disable Retry */
//#define MAC_CONTROL_ASTP		0x00000100	/* Automatic Pad Stripping */
//#define MAC_CONTROL_BOLMT_10		0x00000000	/* Back Off Limit 10 */
//#define MAC_CONTROL_BOLMT_8		0x00000040	/* Back Off Limit 8 */
//#define MAC_CONTROL_BOLMT_4		0x00000080	/* Back Off Limit 4 */
//#define MAC_CONTROL_BOLMT_1		0x000000c0	/* Back Off Limit 1 */
//#define MAC_CONTROL_DC		0x00000020	/* Deferral Check */
#define MAC_CONTROL_TE			0x00000008	/* Transmitter Enable */
#define MAC_CONTROL_RE			0x00000004	/* Receiver Enable */
#define MAC_CORE_INIT			(MAC_CONTROL_HBD)
#endif	/* CONFIG_DRIVER_NETSTMAC */
/* GMAC Control Register defines */
#ifdef CONFIG_DRIVER_NET_STM_GMAC
//#define MAC_CONTROL_TC		0x01000000	/* Transmit Conf. in RGMII/SGMII */
//#define MAC_CONTROL_WD		0x00800000	/* Disable Watchdog on Receive */
#define MAC_CONTROL_JD			0x00400000	/* Jabber disable */
//#define MAC_CONTROL_BE		0x00200000	/* Frame Burst Enable */
//#define MAC_CONTROL_JE		0x00100000	/* Jumbo Frame */
//#define MAC_CONTROL_DCRS		0x00010000	/* Disable Carrier Sense During TX */
#define MAC_CONTROL_PS			0x00008000	/* Port Select 0:GMI, 1:MII */
#define MAC_CONTROL_FES			0x00004000	/* Speed 0:10, 1:100 */
//#define MAC_CONTROL_DO		0x00002000	/* Disable RX Own */
//#define MAC_CONTROL_LM		0x00001000	/* Loop-back Mode */
#define MAC_CONTROL_DM			0x00000800	/* Duplex Mode */
#define MAC_CONTROL_IPC			0x00000400	/* Checksum Offload */
//#define MAC_CONTROL_DR		0x00000200	/* Disable Retry */
//#define MAC_CONTROL_LUD		0x00000100	/* Link Up/Down */
#define MAC_CONTROL_ACS			0x00000080	/* Automatic Pad Stripping */
//#define MAC_CONTROL_DC		0x00000010	/* Deferral Check */
#define MAC_CONTROL_TE			0x00000008	/* Transmitter Enable */
#define MAC_CONTROL_RE			0x00000004	/* Receiver Enable */
#define MAC_CORE_INIT	(MAC_CONTROL_JD | MAC_CONTROL_PS | MAC_CONTROL_ACS | MAC_CONTROL_IPC)
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

/* MAC Flow Control Register defines */
#define MAC_FLOW_CONTROL_PT_MASK	0xffff0000	/* Pause Time Mask */
#define MAC_FLOW_CONTROL_PT_SHIFT	16		/* Pause Time Shift */
//#define GMAC_FLOW_CTRL_FCB_BPA	0x00000001	/* Flow Control Busy ... */
/* STMAC Flow Control Register defines */
#ifdef CONFIG_DRIVER_NETSTMAC
#define MAC_FLOW_CONTROL_PCF		0x00000004	/* Pass Control Frames */
#define MAC_FLOW_CONTROL_FCE		0x00000002	/* Flow Control Enable */
#endif	/* CONFIG_DRIVER_NETSTMAC */
/* GMAC Flow Control Register defines */
#ifdef CONFIG_DRIVER_NET_STM_GMAC
#define MAC_FLOW_CONTROL_RFE		0x00000004	/* RX Flow Control Enable */
#define MAC_FLOW_CONTROL_TFE		0x00000002	/* TX Flow Control Enable */
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

/* MII Address defines */
#define MAC_MII_ADDR_PHY_MASK		0x0000001f	/* MII PHY Address Mask */
#define MAC_MII_ADDR_PHY_SHIFT		11		/* MII PHY Address Shift */
#define MAC_MII_ADDR_REG_MASK		0x0000001f	/* MII Register Mask */
#define MAC_MII_ADDR_REG_SHIFT		6		/* MII Register Shift */
#define MAC_MII_ADDR_WRITE		0x00000002	/* MII Write */
#define MAC_MII_ADDR_BUSY		0x00000001	/* MII Busy */


/*----------------------------------------------------------------------------
 *			MAC Management Counters (MMC) Block
 *---------------------------------------------------------------------------*/

#ifdef CONFIG_DRIVER_NET_STM_GMAC
/* MAC Management Counters Register offsets */
#define MMC_CONTROL			0x00000100	/* MMC Control */

/* MAC Management Counters Register defines */
#define MMC_COUNTER_FREEZE		0x00000008	/* Freeze the Counters */
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */


/*----------------------------------------------------------------------------
 *			DMA Block
 *---------------------------------------------------------------------------*/

/* DMA Control & Status Register (CSR) offsets */
#define DMA_BUS_MODE			0x00001000	/* Bus Mode */
#define DMA_XMT_POLL_DEMAND		0x00001004	/* Transmit Poll Demand */
#define DMA_RCV_POLL_DEMAND		0x00001008	/* Receive Poll Demand */
#define DMA_RCV_BASE_ADDR		0x0000100c	/* Receive List Base Address */
#define DMA_TX_BASE_ADDR		0x00001010	/* Transmit List Base Address */
#define DMA_STATUS			0x00001014	/* Status Register */
#define DMA_CONTROL			0x00001018	/* Control (Operational Mode) */
#define DMA_INTR_ENA			0x0000101c	/* Interrupt Enable */
//#define DMA_CUR_TX_BUF_ADDR		0x00001050	/* Current Host Transmit Buffer */
//#define DMA_CUR_RX_BUF_ADDR		0x00001054	/* Current Host Receive Buffer */

/* DMA Bus Mode Register defines */
//#define DMA_BUS_MODE_DBO		0x00100000	/* Descriptor Byte Ordering */
//#define DMA_BUS_MODE_PBL_MASK		0x00003f00	/* Programmable Burst Length Mask */
#define DMA_BUS_MODE_PBL_SHIFT		8		/* Programmable Burst Length Shift */
//#define DMA_BUS_MODE_BLE		0x00000080	/* Big Endian/Little Endian */
//#define DMA_BUS_MODE_DSL_MASK		0x0000007c	/* Descriptor Skip Length Mask */
//#define DMA_BUS_MODE_DSL_SHIFT	2		/*       (in DWORDS)           */
//#define DMA_BUS_MODE_BAR_BUS		0x00000002	/* Bar-Bus Arbitration */
#define DMA_BUS_MODE_SFT_RESET		0x00000001	/* Software Reset */
#define DMA_BUS_MODE_DEFAULT		0x00000000

/* DMA Status Register defines */
//#define DMA_STATUS_EB_MASK		0x00380000	/* Error Bits Mask */
//#define DMA_STATUS_EB_TX_ABORT	0x00080000	/* Error Bits - TX Abort */
//#define DMA_STATUS_EB_RX_ABORT	0x00100000	/* Error Bits - RX Abort */
//#define DMA_STATUS_TS_MASK		0x00700000	/* Transmit Process State */
//#define DMA_STATUS_TS_SHIFT		20
//#define DMA_STATUS_RS_MASK		0x000e0000	/* Receive Process State */
//#define DMA_STATUS_RS_SHIFT		17
//#define DMA_STATUS_NIS		0x00010000	/* Normal Interrupt Summary */
//#define DMA_STATUS_AIS		0x00008000	/* Abnormal Interrupt Summary */
//#define DMA_STATUS_ERI		0x00004000	/* Early Receive Interrupt */
//#define DMA_STATUS_FBI		0x00002000	/* Fatal Bus Error Interrupt */
//#define DMA_STATUS_ETI		0x00000400	/* Early Transmit Interrupt */
//#define DMA_STATUS_RWT		0x00000200	/* Receive Watchdog Timeout */
//#define DMA_STATUS_RPS		0x00000100	/* Receive Process Stopped */
//#define DMA_STATUS_RU			0x00000080	/* Receive Buffer Unavailable */
//#define DMA_STATUS_RI			0x00000040	/* Receive Interrupt */
//#define DMA_STATUS_UNF		0x00000020	/* Transmit Underflow */
//#define DMA_STATUS_OVF		0x00000010	/* Receive Overflow */
//#define DMA_STATUS_TJT		0x00000008	/* Transmit Jabber Timeout */
//#define DMA_STATUS_TU			0x00000004	/* Transmit Buffer Unavailable */
//#define DMA_STATUS_TPS		0x00000002	/* Transmit Process Stopped */
#define DMA_STATUS_TI			0x00000001	/* Transmit Interrupt */

/* DMA Control (Operation Mode) Register defines */
#define DMA_CONTROL_ST			0x00002000	/* Start/Stop Transmission */
#define DMA_CONTROL_SR			0x00000002	/* Start/Stop Receive */


/*----------------------------------------------------------------------------
 *			MII defines
 *---------------------------------------------------------------------------*/

/* MII Register Offsets */
#define MII_BMCR			0x00		/* Basic Mode Control Register */
#define MII_BMSR			0x01		/* Basic Mode Status Register */
#define MII_PHYSID1			0x02		/* PHY Identifier #1 */
#define MII_PHYSID2			0x03		/* PHY Identifier #2 */
#define MII_ADVERTISE			0x04		/* AN Advertisement Control Register */
#define MII_LPA				0x05		/* AN Link Partner Ability Register */
//#define MII_EXPANSION			0x06		/* AN Expansion Register */
#define MII_GBCR			0x09		/* 1000BASE-T Control Register */
//#define MII_GBSR			0x0A		/* 1000BASE-T Status Register */

/* Basic Mode Control Register defines */
//#define BMCR_CTST			0x0080		/* Collision Test */
#define BMCR_FULLDPLX			0x0100		/* Full Duplex */
#define BMCR_ANRESTART			0x0200		/* Auto-Negotiation Restart */
//#define BMCR_ISOLATE			0x0400		/* Disconnect from the MII */
//#define BMCR_PDOWN			0x0800		/* Power-down */
#define BMCR_ANENABLE			0x1000		/* Enable Auto-Negotiation */
#define BMCR_LOOPBACK			0x4000		/* Enable Loop-back Mode */
//#define BMCR_RESET			0x8000		/* Software Reset */
#define BMCR_SPEED1000			0x0040		/* Select 1000Mbps */
#define BMCR_SPEED100			0x2000		/* Select 100Mbps */
#define BMCR_SPEED10			0x0000		/* Select 10Mbps */
#define BMCR_SPEED_MASK			(BMCR_SPEED100|BMCR_SPEED1000)

/* Basic Mode Status Register defines */
//#define BMSR_ERCAP			0x0001		/* Extended Capabilities Registers */
//#define BMSR_JCD			0x0002		/* Jabber Detected */
#define BMSR_LSTATUS			0x0004		/* Link Status */
//#define BMSR_ANEGCAPABLE		0x0008		/* Able to do Auto-Negotiation */
#define BMSR_RFAULT			0x0010		/* Remote Fault Detected */
#define BMSR_ANEGCOMPLETE		0x0020		/* Auto-Negotiation Complete */
#define BMSR_10HALF			0x0800		/* Can do 10Mbps, Half-Duplex */
#define BMSR_10FULL			0x1000		/* Can do 10Mbps, Full-Duplex */
#define BMSR_100HALF			0x2000		/* Can do 100Mbps, Half-Duplex */
#define BMSR_100FULL			0x4000		/* Can do 100Mbps, Full-Duplex */
#define BMSR_100BASE4			0x8000		/* Can do 100Mbps, 4k Packets */

/* Auto-Negotiate Advertisement Control Register defines */
//#define ADVERTISE_SLCT		0x001f		/* Selector Bits */
//#define ADVERTISE_CSMA		0x0001		/* Only Selector Supported */
#define ADVERTISE_10HALF		0x0020		/* Try for 10Mbps Half-Duplex */
#define ADVERTISE_10FULL		0x0040		/* Try for 10Mbps Full-Duplex */
#define ADVERTISE_100HALF		0x0080		/* Try for 100Mbps Half-Duplex */
#define ADVERTISE_100FULL		0x0100		/* Try for 100Mbps Full-Duplex */
#define ADVERTISE_100BASE4		0x0200		/* Try for 100Mbps 4k Packets */
//#define ADVERTISE_RFAULT		0x2000		/* Say we can Detect Faults */
//#define ADVERTISE_LPACK		0x4000		/* Ack Link Partners Response */
//#define ADVERTISE_NPAGE		0x8000		/* Next Page Capable */
//#define ADVERTISE_FULL (ADVERTISE_100FULL | ADVERTISE_10FULL | ADVERTISE_CSMA)
//#define ADVERTISE_ALL (ADVERTISE_10HALF | ADVERTISE_10FULL | ADVERTISE_100HALF | ADVERTISE_100FULL)

/* Auto-Negotiate Link Partner Ability Register defines */
//#define LPA_SLCT			0x001f		/* Same as Advertise Selector */
//#define LPA_10HALF			0x0020		/* Can do 10Mbps Half-Duplex */
#define LPA_10FULL			0x0040		/* Can do 10Mbps Full-Duplex */
#define LPA_100HALF			0x0080		/* Can do 100Mbps Half-Duplex */
#define LPA_100FULL			0x0100		/* Can do 100Mbps Full-Duplex */
//#define LPA_100BASE4			0x0200		/* Can do 100Mbps 4k Packets */
//#define LPA_RFAULT			0x2000		/* Link Partner Faulted */
//#define LPA_LPACK			0x4000		/* Link Partner Acknowledged */
//#define LPA_NPAGE			0x8000		/* Next Page Capable */
//#define LPA_DUPLEX			(LPA_10FULL | LPA_100FULL)
//#define LPA_100			(LPA_100FULL | LPA_100HALF | LPA_100BASE4)

/* 1000BASE-T Control Register defines */
#define GBCR_1000HALF			0x0100		/* Try for 1000Mbps half-duplex */
#define GBCR_1000FULL			0x0200		/* Try for 1000Mbps full-duplex */


/*----------------------------------------------------------------------------
 *			Descriptor Structure
 *---------------------------------------------------------------------------*/

/* This structure is common for both receive and transmit DMA descriptors.
 * A descriptor should not be used for storing more than one frame.
 *
 * NOTE: the 'enhanced' structures are used the GMAC, whereas
 * the 10/100 STMAC uses the older (non-enhanced) structures.
 * */
struct stmac_dma_des_t
{
	/* Receive descriptor */
	union {
		struct {
			u32 des0;	/* RDES0 or TDES0 */
			u32 des1;	/* RDES1 or TDES1 */
		} u;
#ifdef CONFIG_DRIVER_NETSTMAC
		struct {
			/* RDES0 */
			u32 reserved1:1;
			u32 crc_error:1;
			u32 dribbling:1;
			u32 mii_error:1;
			u32 receive_watchdog:1;
			u32 frame_type:1;
			u32 collision:1;
			u32 frame_too_long:1;
			u32 last_descriptor:1;
			u32 first_descriptor:1;
			u32 multicast_frame:1;
			u32 runt_frame:1;
			u32 length_error:1;
			u32 partial_frame_error:1;
			u32 descriptor_error:1;
			u32 error_summary:1;
			u32 frame_length:14;
			u32 filtering_fail:1;
			u32 own:1;
			/* RDES1 */
			u32 buffer1_size:11;
			u32 buffer2_size:11;
			u32 reserved2:2;
			u32 second_address_chained:1;
			u32 end_ring:1;
			u32 reserved3:5;
			u32 disable_ic:1;
		} rx;
#endif	/* CONFIG_DRIVER_NETSTMAC */
#ifdef CONFIG_DRIVER_NET_STM_GMAC
		struct {
			/* RDES0 */
			u32 payload_csum_error:1;
			u32 crc_error:1;
			u32 dribbling:1;
			u32 error_gmii:1;
			u32 receive_watchdog:1;
			u32 frame_type:1;
			u32 late_collision:1;
			u32 ipc_csum_error:1;
			u32 last_descriptor:1;
			u32 first_descriptor:1;
			u32 vlan_tag:1;
			u32 overflow_error:1;
			u32 length_error:1;
			u32 source_filter_fail:1;
			u32 descriptor_error:1;
			u32 error_summary:1;
			u32 frame_length:14;
			u32 filtering_fail:1;
			u32 own:1;
			/* RDES1 */
			u32 buffer1_size:13;
			u32 reserved1:1;
			u32 second_address_chained:1;
			u32 end_ring:1;
			u32 buffer2_size:13;
			u32 reserved2:2;
			u32 disable_ic:1;
		} rx;		/* -- enhanced -- */
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */
#ifdef CONFIG_DRIVER_NETSTMAC
		struct {
			/* TDES0 */
			u32 deferred:1;
			u32 underflow_error:1;
			u32 excessive_deferral:1;
			u32 collision_count:4;
			u32 heartbeat_fail:1;
			u32 excessive_collisions:1;
			u32 late_collision:1;
			u32 no_carrier:1;
			u32 loss_carrier:1;
			u32 reserved1:3;
			u32 error_summary:1;
			u32 reserved2:15;
			u32 own:1;
			/* TDES1 */
			u32 buffer1_size:11;
			u32 buffer2_size:11;
			u32 reserved3:1;
			u32 disable_padding:1;
			u32 second_address_chained:1;
			u32 end_ring:1;
			u32 crc_disable:1;
			u32 reserved4:2;
			u32 first_segment:1;
			u32 last_segment:1;
			u32 interrupt:1;
		} tx;
#endif	/* CONFIG_DRIVER_NETSTMAC */
#ifdef CONFIG_DRIVER_NET_STM_GMAC
		struct {
			/* TDES0 */
			u32 deferred:1;
			u32 underflow_error:1;
			u32 excessive_deferral:1;
			u32 collision_count:4;
			u32 vlan_frame:1;
			u32 excessive_collisions:1;
			u32 late_collision:1;
			u32 no_carrier:1;
			u32 loss_carrier:1;
			u32 payload_error:1;
			u32 frame_flushed:1;
			u32 jabber_timeout:1;
			u32 error_summary:1;
			u32 ip_header_error:1;
			u32 time_stamp_status:1;
			u32 reserved1:2;
			u32 second_address_chained:1;
			u32 end_ring:1;
			u32 checksum_insertion:2;
			u32 reserved2:1;
			u32 time_stamp_enable:1;
			u32 disable_padding:1;
			u32 crc_disable:1;
			u32 first_segment:1;
			u32 last_segment:1;
			u32 interrupt:1;
			u32 own:1;
			/* TDES1 */
			u32 buffer1_size:13;
			u32 reserved3:3;
			u32 buffer2_size:13;
			u32 reserved4:3;
		} tx;		/* -- enhanced -- */
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */
	} des01;
	void * des2;
	void * des3;
};

typedef struct stmac_dma_des_t stmac_dma_des;


/*----------------------------------------------------------------------------
 *			Miscellaneous defines
 *---------------------------------------------------------------------------*/

/* MAC Register Accessors */
#define STMAC_READ(REG)		readl(CFG_STM_STMAC_BASE+(REG))
#define STMAC_WRITE(V, REG)	writel((V), CFG_STM_STMAC_BASE+(REG))

/* max delay to wait after performing a MII Register read/write */
#ifndef CONFIG_STMAC_MII_POLL_BUSY_DELAY
#define CONFIG_STMAC_MII_POLL_BUSY_DELAY 1000		/* ticks */
#endif

/* max delay to wait for a PHY Auto-Negotiate to take */
#ifndef CONFIG_STMAC_AUTONEG_TIMEOUT
#define CONFIG_STMAC_AUTONEG_TIMEOUT	(10*CFG_HZ)	/* 10 seconds */
#endif

/* max delay to wait for a TX to take */
#ifndef CONFIG_STMAC_TX_TIMEOUT
#define CONFIG_STMAC_TX_TIMEOUT		(5*CFG_HZ)	/* 5 seconds */
#endif

/* Recommended STBus Bridge Values for GMAC */
#ifdef CONFIG_DRIVER_NET_STM_GMAC
#define STBUS_BRIDGE_OFFSET		0x00007000
#define STBUS_BRIDGE_MAGIC		0x25C608	/* from validation */
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */


