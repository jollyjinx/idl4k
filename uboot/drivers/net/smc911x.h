/*------------------------------------------------------------------------
 . smc911x.h - macros for the LAN911X Ethernet Driver
 .
 . (C) Copyright 2002
 . Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 . Rolf Offermanns <rof@sysgo.de>
 . Copyright (C) 2001 Standard Microsystems Corporation (SMSC)
 .       Developed by Simple Network Magic Corporation (SNMC)
 . Copyright (C) 1996 by Erik Stahlman (ES)
 .
 . This program is free software; you can redistribute it and/or modify
 . it under the terms of the GNU General Public License as published by
 . the Free Software Foundation; either version 2 of the License, or
 . (at your option) any later version.
 .
 . This program is distributed in the hope that it will be useful,
 . but WITHOUT ANY WARRANTY; without even the implied warranty of
 . MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 . GNU General Public License for more details.
 .
 . You should have received a copy of the GNU General Public License
 . along with this program; if not, write to the Free Software
 . Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 .
 . This file contains register information and access macros for
 . the LAN91C111 single chip ethernet controller.  It is a modified
 . version of the smc9194.h file.
 .
 . Information contained in this file was obtained from the LAN91C111
 . manual from SMC.  To get a copy, if you really want one, you can find
 . information under www.smsc.com.
 .
 . Authors
 . 	Erik Stahlman				( erik@vt.edu )
 .	Daris A Nevil				( dnevil@snmc.com )
 .
 . History
 . 03/16/01		Daris A Nevil	Modified for use with LAN91C111 device
 .
 ---------------------------------------------------------------------------*/
#ifndef _SMC911X_H_
#define _SMC911X_H_

#include <asm/types.h>
#include <config.h>

/*
 * This function may be called by the board specific initialisation code
 * in order to override the default mac address.
 */

void smc_set_mac_addr(const char *addr);


/* I want some simple types */

typedef unsigned char			byte;
typedef unsigned short			word;
typedef unsigned long int 		dword;

/*
 . DEBUGGING LEVELS
 .
 . 0 for normal operation
 . 1 for slightly more details
 . >2 for various levels of increasingly useless information
 .    2 for interrupt tracking, status flags
 .    3 for packet info
 .    4 for complete packet dumps
*/
/*#define SMC_DEBUG 0 */


typedef unsigned char BOOLEAN;

#define TRUE	((BOOLEAN)1)
#define FALSE	((BOOLEAN)0)

#define HIBYTE(w)  ((byte)(((word)(w))>>8))
#define LOBYTE(w)  ((byte)(((word)(w))&0x00FFU))
#define HIWORD(dW) ((word)(((dword)(dW))>>16))
#define LOWORD(dW) ((word)(((dword)(dW))&0x0000FFFFUL))



#define Lan_GetRegDW(dwOffset) \
	((*(volatile dword *)(SMC_BASE_ADDRESS+dwOffset)))
#define Lan_SetRegDW(dwOffset,dwVal) \
	((*(volatile dword *)(SMC_BASE_ADDRESS+dwOffset))=(dwVal))
#define Lan_ClrBitsDW(dwOffset,dwBits) \
	((*(volatile dword *)(SMC_BASE_ADDRESS+dwOffset))&=(~dwBits))
#define Lan_SetBitsDW(dwOffset,dwBits) \
	((*(volatile dword *)(SMC_BASE_ADDRESS+dwOffset))|=(dwBits))


//Below are the register offsets and bit definitions
//  of the Lan911x memory space
#define RX_DATA_FIFO	    (0x00UL)

#define TX_DATA_FIFO        (0x20UL)
#define		TX_CMD_A_ON_COMP_			(0x80000000UL)
#define		TX_CMD_A_BUF_END_ALGN_		(0x03000000UL)
#define		TX_CMD_A_4_BYTE_ALGN_		(0x00000000UL)
#define		TX_CMD_A_16_BYTE_ALGN_		(0x01000000UL)
#define		TX_CMD_A_32_BYTE_ALGN_		(0x02000000UL)
#define		TX_CMD_A_DATA_OFFSET_		(0x001F0000UL)
#define		TX_CMD_A_FIRST_SEG_			(0x00002000UL)
#define		TX_CMD_A_LAST_SEG_			(0x00001000UL)
#define		TX_CMD_A_BUF_SIZE_			(0x000007FFUL)
#define		TX_CMD_B_PKT_TAG_			(0xFFFF0000UL)
#define		TX_CMD_B_ADD_CRC_DISABLE_	(0x00002000UL)
#define		TX_CMD_B_DISABLE_PADDING_	(0x00001000UL)
#define		TX_CMD_B_PKT_BYTE_LENGTH_	(0x000007FFUL)

#define RX_STATUS_FIFO      (0x40UL)
#define		RX_STS_ES_			(0x00008000UL)
#define		RX_STS_MCAST_		(0x00000400UL)
#define RX_STATUS_FIFO_PEEK (0x44UL)
#define TX_STATUS_FIFO		(0x48UL)
#define TX_STATUS_FIFO_PEEK (0x4CUL)
#define ID_REV              (0x50UL)
#define		ID_REV_CHIP_ID_		(0xFFFF0000UL)	// RO
#define		ID_REV_REV_ID_		(0x0000FFFFUL)	// RO

#define INT_CFG				(0x54UL)
#define		INT_CFG_INT_DEAS_		(0xFF000000UL)	// R/W
#define     INT_CFG_INT_DEAS_CLR_	(0x00004000UL)  // SC
#define     INT_CFG_INT_DEAS_STS_	(0x00002000UL)  // SC
#define		INT_CFG_IRQ_INT_		(0x00001000UL)	// RO
#define		INT_CFG_IRQ_EN_			(0x00000100UL)	// R/W
#define		INT_CFG_IRQ_POL_		(0x00000010UL)	// R/W Not Affected by SW Reset
#define		INT_CFG_IRQ_TYPE_		(0x00000001UL)	// R/W Not Affected by SW Reset

#define INT_STS				(0x58UL)
#define		INT_STS_SW_INT_		(0x80000000UL)	// R/WC
#define		INT_STS_TXSTOP_INT_	(0x02000000UL)	// R/WC
#define		INT_STS_RXSTOP_INT_	(0x01000000UL)	// R/WC
#define		INT_STS_RXDFH_INT_	(0x00800000UL)	// R/WC
#define		INT_STS_RXDF_INT_	(0x00400000UL)	// R/WC
#define		INT_STS_TX_IOC_		(0x00200000UL)	// R/WC
#define		INT_STS_RXD_INT_	(0x00100000UL)	// R/WC
#define		INT_STS_GPT_INT_	(0x00080000UL)	// R/WC
#define		INT_STS_PHY_INT_	(0x00040000UL)	// RO
#define		INT_STS_PME_INT_	(0x00020000UL)	// R/WC
#define		INT_STS_TXSO_		(0x00010000UL)	// R/WC
#define		INT_STS_RWT_		(0x00008000UL)	// R/WC
#define		INT_STS_RXE_		(0x00004000UL)	// R/WC
#define		INT_STS_TXE_		(0x00002000UL)	// R/WC
#define		INT_STS_TDFU_		(0x00000800UL)	// R/WC
#define		INT_STS_TDFO_		(0x00000400UL)	// R/WC
#define		INT_STS_TDFA_		(0x00000200UL)	// R/WC
#define		INT_STS_TSFF_		(0x00000100UL)	// R/WC
#define		INT_STS_TSFL_		(0x00000080UL)	// R/WC
#define		INT_STS_RXDF_		(0x00000040UL)	// R/WC
#define		INT_STS_RDFL_		(0x00000020UL)	// R/WC
#define		INT_STS_RSFF_		(0x00000010UL)	// R/WC
#define		INT_STS_RSFL_		(0x00000008UL)	// R/WC
#define		INT_STS_GPIO2_INT_	(0x00000004UL)	// R/WC
#define		INT_STS_GPIO1_INT_	(0x00000002UL)	// R/WC
#define		INT_STS_GPIO0_INT_	(0x00000001UL)	// R/WC

#define INT_EN				(0x5CUL)
#define		INT_EN_SW_INT_EN_		(0x80000000UL)	// R/W
#define		INT_EN_TXSTOP_INT_EN_	(0x02000000UL)	// R/W
#define		INT_EN_RXSTOP_INT_EN_	(0x01000000UL)	// R/W
#define		INT_EN_RXDFH_INT_EN_	(0x00800000UL)	// R/W
#define		INT_EN_TIOC_INT_EN_		(0x00200000UL)	// R/W
#define		INT_EN_RXD_INT_EN_		(0x00100000UL)	// R/W
#define		INT_EN_GPT_INT_EN_		(0x00080000UL)	// R/W
#define		INT_EN_PHY_INT_EN_		(0x00040000UL)	// R/W
#define		INT_EN_PME_INT_EN_		(0x00020000UL)	// R/W
#define		INT_EN_TXSO_EN_			(0x00010000UL)	// R/W
#define		INT_EN_RWT_EN_			(0x00008000UL)	// R/W
#define		INT_EN_RXE_EN_			(0x00004000UL)	// R/W
#define		INT_EN_TXE_EN_			(0x00002000UL)	// R/W
#define		INT_EN_TDFU_EN_			(0x00000800UL)	// R/W
#define		INT_EN_TDFO_EN_			(0x00000400UL)	// R/W
#define		INT_EN_TDFA_EN_			(0x00000200UL)	// R/W
#define		INT_EN_TSFF_EN_			(0x00000100UL)	// R/W
#define		INT_EN_TSFL_EN_			(0x00000080UL)	// R/W
#define		INT_EN_RXDF_EN_			(0x00000040UL)	// R/W
#define		INT_EN_RDFL_EN_			(0x00000020UL)	// R/W
#define		INT_EN_RSFF_EN_			(0x00000010UL)	// R/W
#define		INT_EN_RSFL_EN_			(0x00000008UL)	// R/W
#define		INT_EN_GPIO2_INT_		(0x00000004UL)	// R/W
#define		INT_EN_GPIO1_INT_		(0x00000002UL)	// R/W
#define		INT_EN_GPIO0_INT_		(0x00000001UL)	// R/W

#define BYTE_TEST				(0x64UL)
#define FIFO_INT				(0x68UL)
#define		FIFO_INT_TX_AVAIL_LEVEL_	(0xFF000000UL)	// R/W
#define		FIFO_INT_TX_STS_LEVEL_		(0x00FF0000UL)	// R/W
#define		FIFO_INT_RX_AVAIL_LEVEL_	(0x0000FF00UL)	// R/W
#define		FIFO_INT_RX_STS_LEVEL_		(0x000000FFUL)	// R/W

#define RX_CFG					(0x6CUL)
#define		RX_CFG_RX_END_ALGN_		(0xC0000000UL)	// R/W
#define			RX_CFG_RX_END_ALGN4_		(0x00000000UL)	// R/W
#define			RX_CFG_RX_END_ALGN16_		(0x40000000UL)	// R/W
#define			RX_CFG_RX_END_ALGN32_		(0x80000000UL)	// R/W
#define		RX_CFG_RX_DMA_CNT_		(0x0FFF0000UL)	// R/W
#define		RX_CFG_RX_DUMP_			(0x00008000UL)	// R/W
#define		RX_CFG_RXDOFF_			(0x00001F00UL)	// R/W

#define TX_CFG					(0x70UL)
#define		TX_CFG_TXS_DUMP_		(0x00008000UL)	// Self Clearing
#define		TX_CFG_TXD_DUMP_		(0x00004000UL)	// Self Clearing
#define		TX_CFG_TXSAO_			(0x00000004UL)	// R/W
#define		TX_CFG_TX_ON_			(0x00000002UL)	// R/W
#define		TX_CFG_STOP_TX_			(0x00000001UL)	// Self Clearing

#define HW_CFG					(0x74UL)
#define		HW_CFG_TTM_				(0x00200000UL)	// R/W
#define		HW_CFG_SF_				(0x00100000UL)	// R/W
#define		HW_CFG_TX_FIF_SZ_		(0x000F0000UL)	// R/W
#define		HW_CFG_TR_				(0x00003000UL)	// R/W
#define     HW_CFG_PHY_CLK_SEL_		(0x00000060UL)  // R/W //only available on 115/117
#define         HW_CFG_PHY_CLK_SEL_INT_PHY_	(0x00000000UL) //R/W //only available on 115/117
#define         HW_CFG_PHY_CLK_SEL_EXT_PHY_	(0x00000020UL) //R/W //only available on 115/117
#define         HW_CFG_PHY_CLK_SEL_CLK_DIS_	(0x00000040UL) //R/W //only available on 115/117
#define     HW_CFG_SMI_SEL_			(0x00000010UL)  // R/W //only available on 115/117
#define     HW_CFG_EXT_PHY_DET_		(0x00000008UL)  // RO  //only available on 115/117
#define     HW_CFG_EXT_PHY_EN_		(0x00000004UL)  // R/W //only available on 115/117
#define		HW_CFG_32_16_BIT_MODE_	(0x00000004UL)	// RO  //only available on 116/118
#define     HW_CFG_SRST_TO_			(0x00000002UL)  // RO  //only available on 115/117
#define		HW_CFG_SRST_			(0x00000001UL)	// Self Clearing

#define RX_DP_CTRL				(0x78UL)
#define		RX_DP_CTRL_RX_FFWD_		(0x80000000UL)	// RO

#define RX_FIFO_INF				(0x7CUL)
#define		RX_FIFO_INF_RXSUSED_	(0x00FF0000UL)	// RO
#define		RX_FIFO_INF_RXDUSED_	(0x0000FFFFUL)	// RO

#define TX_FIFO_INF				(0x80UL)
#define		TX_FIFO_INF_TSUSED_		(0x00FF0000UL)  // RO
#define		TX_FIFO_INF_TDFREE_		(0x0000FFFFUL)	// RO

#define PMT_CTRL				(0x84UL)
#define		PMT_CTRL_PM_MODE_			(0x00003000UL)	// Self Clearing
#define	        PMT_CTRL_PM_MODE_D0_	(0x00000000UL)  // Self Clearing
#define         PMT_CTRL_PM_MODE_D1_	(0x00001000UL)  // Self Clearing
#define         PMT_CTRL_PM_MODE_D2_	(0x00002000UL)  // Self Clearing
#define         PMT_CTRL_PM_MODE_D3_	(0x00003000UL)  // Self Clearing
#define		PMT_CTRL_PHY_RST_			(0x00000400UL)	// Self Clearing
#define		PMT_CTRL_WOL_EN_			(0x00000200UL)	// R/W
#define		PMT_CTRL_ED_EN_				(0x00000100UL)	// R/W
#define		PMT_CTRL_PME_TYPE_			(0x00000040UL)	// R/W Not Affected by SW Reset
#define		PMT_CTRL_WUPS_				(0x00000030UL)	// R/WC
#define			PMT_CTRL_WUPS_NOWAKE_		(0x00000000UL)	// R/WC
#define			PMT_CTRL_WUPS_ED_			(0x00000010UL)	// R/WC
#define			PMT_CTRL_WUPS_WOL_			(0x00000020UL)	// R/WC
#define			PMT_CTRL_WUPS_MULTI_		(0x00000030UL)	// R/WC
#define		PMT_CTRL_PME_IND_		(0x00000008UL)	// R/W
#define		PMT_CTRL_PME_POL_		(0x00000004UL)	// R/W
#define		PMT_CTRL_PME_EN_		(0x00000002UL)	// R/W Not Affected by SW Reset
#define		PMT_CTRL_READY_			(0x00000001UL)	// RO

#define GPIO_CFG				(0x88UL)
#define		GPIO_CFG_LED3_EN_		(0x40000000UL)	// R/W
#define		GPIO_CFG_LED2_EN_		(0x20000000UL)	// R/W
#define		GPIO_CFG_LED1_EN_		(0x10000000UL)	// R/W
#define		GPIO_CFG_GPIO2_INT_POL_	(0x04000000UL)	// R/W
#define		GPIO_CFG_GPIO1_INT_POL_	(0x02000000UL)	// R/W
#define		GPIO_CFG_GPIO0_INT_POL_	(0x01000000UL)	// R/W
#define		GPIO_CFG_EEPR_EN_		(0x00700000UL)	// R/W
#define		GPIO_CFG_GPIOBUF2_		(0x00040000UL)	// R/W
#define		GPIO_CFG_GPIOBUF1_		(0x00020000UL)	// R/W
#define		GPIO_CFG_GPIOBUF0_		(0x00010000UL)	// R/W
#define		GPIO_CFG_GPIODIR2_		(0x00000400UL)	// R/W
#define		GPIO_CFG_GPIODIR1_		(0x00000200UL)	// R/W
#define		GPIO_CFG_GPIODIR0_		(0x00000100UL)	// R/W
#define		GPIO_CFG_GPIOD4_		(0x00000020UL)	// R/W
#define		GPIO_CFG_GPIOD3_		(0x00000010UL)	// R/W
#define		GPIO_CFG_GPIOD2_		(0x00000004UL)	// R/W
#define		GPIO_CFG_GPIOD1_		(0x00000002UL)	// R/W
#define		GPIO_CFG_GPIOD0_		(0x00000001UL)	// R/W

#define GPT_CFG					(0x8CUL)
#define		GPT_CFG_TIMER_EN_		(0x20000000UL)	// R/W
#define		GPT_CFG_GPT_LOAD_		(0x0000FFFFUL)	// R/W

#define GPT_CNT					(0x90UL)
#define		GPT_CNT_GPT_CNT_		(0x0000FFFFUL)	// RO

#define ENDIAN					(0x98UL)
#define FREE_RUN				(0x9CUL)
#define RX_DROP					(0xA0UL)
#define MAC_CSR_CMD				(0xA4UL)
#define		MAC_CSR_CMD_CSR_BUSY_	(0x80000000UL)	// Self Clearing
#define		MAC_CSR_CMD_R_NOT_W_	(0x40000000UL)	// R/W
#define		MAC_CSR_CMD_CSR_ADDR_	(0x000000FFUL)	// R/W

#define MAC_CSR_DATA			(0xA8UL)
#define AFC_CFG					(0xACUL)
#define		AFC_CFG_AFC_HI_			(0x00FF0000UL)	// R/W
#define		AFC_CFG_AFC_LO_			(0x0000FF00UL)	// R/W
#define		AFC_CFG_BACK_DUR_		(0x000000F0UL)	// R/W
#define		AFC_CFG_FCMULT_			(0x00000008UL)	// R/W
#define		AFC_CFG_FCBRD_			(0x00000004UL)	// R/W
#define		AFC_CFG_FCADD_			(0x00000002UL)	// R/W
#define		AFC_CFG_FCANY_			(0x00000001UL)	// R/W

#define E2P_CMD					(0xB0UL)
#define		E2P_CMD_EPC_BUSY_		(0x80000000UL)	// Self Clearing
#define		E2P_CMD_EPC_CMD_		(0x70000000UL)	// R/W
#define			E2P_CMD_EPC_CMD_READ_	(0x00000000UL)	// R/W
#define			E2P_CMD_EPC_CMD_EWDS_	(0x10000000UL)	// R/W
#define			E2P_CMD_EPC_CMD_EWEN_	(0x20000000UL)	// R/W
#define			E2P_CMD_EPC_CMD_WRITE_	(0x30000000UL)	// R/W
#define			E2P_CMD_EPC_CMD_WRAL_	(0x40000000UL)	// R/W
#define			E2P_CMD_EPC_CMD_ERASE_	(0x50000000UL)	// R/W
#define			E2P_CMD_EPC_CMD_ERAL_	(0x60000000UL)	// R/W
#define			E2P_CMD_EPC_CMD_RELOAD_	(0x70000000UL)  // R/W
#define		E2P_CMD_EPC_TIMEOUT_	(0x00000200UL)	// R
#define		E2P_CMD_MAC_ADDR_LOADED_	(0x00000100UL)	// RO
#define		E2P_CMD_EPC_ADDR_		(0x000000FFUL)	// R/W

#define E2P_DATA				(0xB4UL)
#define		E2P_DATA_EEPROM_DATA_	(0x000000FFUL)	// R/W
//end of lan register offsets and bit definitions
#define LAN_REGISTER_EXTENT		(0x00000100UL)

#define LINK_OFF				(0x00UL)
#define LINK_SPEED_10HD			(0x01UL)
#define LINK_SPEED_10FD			(0x02UL)
#define LINK_SPEED_100HD		(0x04UL)
#define LINK_SPEED_100FD		(0x08UL)
#define LINK_SYMMETRIC_PAUSE	(0x10UL)
#define LINK_ASYMMETRIC_PAUSE	(0x20UL)
#define LINK_AUTO_NEGOTIATE		(0x40UL)



/*
 ****************************************************************************
 ****************************************************************************
 *	MAC Control and Status Register (Indirect Address)
 *	Offset (through the MAC_CSR CMD and DATA port)
 ****************************************************************************
 ****************************************************************************
 *
 */
#define MAC_CR				(0x01UL)	// R/W

	/* MAC_CR - MAC Control Register */
	#define MAC_CR_RXALL_		(0x80000000UL)
	#define MAC_CR_HBDIS_		(0x10000000UL)
	#define MAC_CR_RCVOWN_		(0x00800000UL)
	#define MAC_CR_LOOPBK_		(0x00200000UL)
	#define MAC_CR_FDPX_		(0x00100000UL)
	#define MAC_CR_MCPAS_		(0x00080000UL)
	#define MAC_CR_PRMS_		(0x00040000UL)
	#define MAC_CR_INVFILT_		(0x00020000UL)
	#define MAC_CR_PASSBAD_		(0x00010000UL)
	#define MAC_CR_HFILT_		(0x00008000UL)
	#define MAC_CR_HPFILT_		(0x00002000UL)
	#define MAC_CR_LCOLL_		(0x00001000UL)
	#define MAC_CR_BCAST_		(0x00000800UL)
	#define MAC_CR_DISRTY_		(0x00000400UL)
	#define MAC_CR_PADSTR_		(0x00000100UL)
	#define MAC_CR_BOLMT_MASK_	(0x000000C0UL)
	#define MAC_CR_DFCHK_		(0x00000020UL)
	#define MAC_CR_TXEN_		(0x00000008UL)
	#define MAC_CR_RXEN_		(0x00000004UL)

#define ADDRH				(0x02UL)	// R/W mask 0x0000FFFFUL
#define ADDRL				(0x03UL)	// R/W mask 0xFFFFFFFFUL
#define HASHH				(0x04UL)	// R/W
#define HASHL				(0x05UL)	// R/W

#define MII_ACC				(0x06UL)	// R/W
	#define MII_ACC_PHY_ADDR_	(0x0000F800UL)
	#define MII_ACC_MIIRINDA_	(0x000007C0UL)
	#define MII_ACC_MII_WRITE_	(0x00000002UL)
	#define MII_ACC_MII_BUSY_	(0x00000001UL)

#define MII_DATA			(0x07UL)	// R/W mask 0x0000FFFFUL

#define FLOW				(0x08UL)	// R/W
	#define FLOW_FCPT_			(0xFFFF0000UL)
	#define FLOW_FCPASS_		(0x00000004UL)
	#define FLOW_FCEN_			(0x00000002UL)
	#define FLOW_FCBSY_			(0x00000001UL)

#define VLAN1				(0x09UL)	// R/W mask 0x0000FFFFUL
#define VLAN2				(0x0AUL)	// R/W mask 0x0000FFFFUL

#define WUFF				(0x0BUL)	// WO

#define WUCSR				(0x0CUL)	// R/W
	#define WUCSR_GUE_			(0x00000200UL)
	#define WUCSR_WUFR_			(0x00000040UL)
	#define WUCSR_MPR_			(0x00000020UL)
	#define WUCSR_WAKE_EN_		(0x00000004UL)
	#define WUCSR_MPEN_			(0x00000002UL)


/*
 ****************************************************************************
 *	Chip Specific MII Defines
 ****************************************************************************
 *
 *	Phy register offsets and bit definitions
 *
 */
#define LAN9118_PHY_ID	(0x00C0001C)

#define PHY_BCR		((dword)0U)
#define PHY_BCR_RESET_					((word)0x8000U)
#define PHY_BCR_SPEED_SELECT_		((word)0x2000U)
#define PHY_BCR_AUTO_NEG_ENABLE_	((word)0x1000U)
#define PHY_BCR_RESTART_AUTO_NEG_	((word)0x0200U)
#define PHY_BCR_DUPLEX_MODE_		((word)0x0100U)

#define PHY_BSR		((dword)1U)
	#define PHY_BSR_LINK_STATUS_	((word)0x0004U)
	#define PHY_BSR_REMOTE_FAULT_	((word)0x0010U)
	#define PHY_BSR_AUTO_NEG_COMP_	((word)0x0020U)

#define PHY_ID_1	((dword)2U)
#define PHY_ID_2	((dword)3U)

#define PHY_ANEG_ADV    ((dword)4U)
#define PHY_ANEG_ADV_PAUSE_ ((word)0x0C00)
#define PHY_ANEG_ADV_ASYMP_	((word)0x0800)
#define PHY_ANEG_ADV_SYMP_	((word)0x0400)
#define PHY_ANEG_ADV_10H_	((word)0x020)
#define PHY_ANEG_ADV_10F_	((word)0x040)
#define PHY_ANEG_ADV_100H_	((word)0x080)
#define PHY_ANEG_ADV_100F_	((word)0x100)
#define PHY_ANEG_ADV_SPEED_	((word)0x1E0)

#define PHY_ANEG_LPA	((dword)5U)
#define PHY_ANEG_LPA_ASYMP_		((word)0x0800)
#define PHY_ANEG_LPA_SYMP_		((word)0x0400)
#define PHY_ANEG_LPA_100FDX_	((word)0x0100)
#define PHY_ANEG_LPA_100HDX_	((word)0x0080)
#define PHY_ANEG_LPA_10FDX_		((word)0x0040)
#define PHY_ANEG_LPA_10HDX_		((word)0x0020)

#define PHY_MODE_CTRL_STS		((dword)17)	// Mode Control/Status Register
	#define MODE_CTRL_STS_EDPWRDOWN_	((word)0x2000U)
	#define MODE_CTRL_STS_ENERGYON_		((word)0x0002U)

#define PHY_INT_SRC			((dword)29)
#define PHY_INT_SRC_ENERGY_ON_			((word)0x0080U)
#define PHY_INT_SRC_ANEG_COMP_			((word)0x0040U)
#define PHY_INT_SRC_REMOTE_FAULT_		((word)0x0020U)
#define PHY_INT_SRC_LINK_DOWN_			((word)0x0010U)

#define PHY_INT_MASK		((dword)30)
#define PHY_INT_MASK_ENERGY_ON_		((word)0x0080U)
#define PHY_INT_MASK_ANEG_COMP_		((word)0x0040U)
#define PHY_INT_MASK_REMOTE_FAULT_	((word)0x0020U)
#define PHY_INT_MASK_LINK_DOWN_		((word)0x0010U)

#define PHY_SPECIAL			((dword)31)
#define PHY_SPECIAL_SPD_	((word)0x001CU)
#define PHY_SPECIAL_SPD_10HALF_		((word)0x0004U)
#define PHY_SPECIAL_SPD_10FULL_		((word)0x0014U)
#define PHY_SPECIAL_SPD_100HALF_	((word)0x0008U)
#define PHY_SPECIAL_SPD_100FULL_	((word)0x0018U)

#define LINK_OFF				(0x00UL)
#define LINK_SPEED_10HD			(0x01UL)
#define LINK_SPEED_10FD			(0x02UL)
#define LINK_SPEED_100HD		(0x04UL)
#define LINK_SPEED_100FD		(0x08UL)
#define LINK_SYMMETRIC_PAUSE	(0x10UL)
#define LINK_ASYMMETRIC_PAUSE	(0x20UL)
#define LINK_AUTO_NEGOTIATE		(0x40UL)

/*-------------------------------------------------------------------------
 .  I define some macros to make it easier to do somewhat common
 . or slightly complicated, repeated tasks.
 --------------------------------------------------------------------------*/

/* this enables an interrupt in the interrupt mask register */
#define SMC_ENABLE_INT(x) {;}

/* this disables an interrupt from the interrupt mask register */

#define SMC_DISABLE_INT(x) {;}

#define TX_FIFO_LOW_THRESHOLD	(1600)

#endif  /* _SMC_911X_H_ */
