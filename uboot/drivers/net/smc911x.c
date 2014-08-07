/*------------------------------------------------------------------------
 . smc9111x.c
 . This is a driver for SMSC's LAN911X single-chip Ethernet device.
 . based on the SMC91111 driver from U-boot, sim911x.c of smsc and
 . datsheet.

 . (C) Copyright 2005
 . Andy Sturges, STMicrolectronics <andy.sturges@st.com>

 . (C) Copyright 2002
 . Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 . Rolf Offermanns <rof@sysgo.de>
 .
 . Copyright (C) 2001 Standard Microsystems Corporation (SMSC)
 .	 Developed by Simple Network Magic Corporation (SNMC)
 . Copyright (C) 1996 by Erik Stahlman (ES)
 .
 . This program is free software; you can redistribute it and/or modify
 . it under the terms of the GNU General Public License as published by
 . the Free Software Foundation; either version 2 of the License, or
 . (at your option) any later version.
 .
 . This program is distributed in the hope that it will be useful,
 . but WITHOUT ANY WARRANTY; without even the implied warranty of
 . MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 . GNU General Public License for more details.
 .
 . You should have received a copy of the GNU General Public License
 . along with this program; if not, write to the Free Software
 . Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307	 USA
 .
 . Information contained in this file was obtained from the LAN9111x
 . manual from SMC.  To get a copy, if you really want one, you can find
 . information under www.smsc.com.
 .

  ----------------------------------------------------------------------------*/

#include <common.h>
#include <command.h>
#include <config.h>
#include "smc911x.h"
#include <net.h>

#ifdef CONFIG_DRIVER_SMC911X

#define SMC_DEBUG 0

#if SMC_DEBUG > 1

#define USE_TRACE 1
#define USE_WARNING 1

#endif

#ifdef USE_TRACE
#ifndef USE_WARNING
#define USE_WARNING
#endif
#	define SMSC_TRACE(msg,args...)			\
		printf("SMSC: " msg "\n", ## args);
#else
#	define SMSC_TRACE(msg,args...)
#endif

#ifdef USE_WARNING
#define SMSC_WARNING(msg, args...)				\
		printf("SMSC_WARNING: " msg "\n",## args);
#else
#	define SMSC_WARNING(msg, args...)
#endif

/* Autonegotiation timeout in seconds */
#ifndef CONFIG_SMC_AUTONEG_TIMEOUT
#define CONFIG_SMC_AUTONEG_TIMEOUT 10
#endif

#if (SMC_DEBUG > 2 )
#define PRINTK3(args...) printf(args)
#else
#define PRINTK3(args...)
#endif

#if SMC_DEBUG > 1
#define PRINTK2(args...) printf(args)
#else
#define PRINTK2(args...)
#endif

#ifdef SMC_DEBUG
#define PRINTK(args...) printf(args)
#else
#define PRINTK(args...)
#endif


/*------------------------------------------------------------------------
 .
 . The internal workings of the driver.	 If you are changing anything
 . here with the SMC stuff, you should have the datasheet and know
 . what you are doing.
 .
 -------------------------------------------------------------------------*/

static dword dwIdRev;
static dword dwPhyAddress;
static dword dwLinkSpeed;
static dword dwLinkSettings;

#define CARDNAME "LAN911X"

#ifndef CONFIG_SMC911X_BASE
#error Must define memory base address fpr SMC911X
#endif

#define SMC_BASE_ADDRESS CONFIG_SMC911X_BASE

#define SMC_DEV_NAME "SMC911X"
#define SMC_PHY_ADDR 0xFFFFFFFFUL

#define SMC_TX_TIMEOUT 30

#define ETH_ZLEN 60

/* static functions */

static void Phy_SetLink(void);
static word Phy_GetRegW(dword dwRegIndex);
static void Phy_SetRegW(dword dwRegIndex, word wVal);
static void Phy_UpdateLinkMode(void);
static void Phy_GetLinkMode(void);
static void Phy_CheckLink(void);

static void Tx_WriteFifo(dword *pdwBuf, dword dwDwordCount);
static dword Tx_GetTxStatusCount(void);
static dword Tx_CompleteTx(void);
static void Tx_UpdateTxCounters(void);

static void Rx_ReadFifo(dword *pdwBuf,dword dwDwordCount);
static dword Rx_PopRxStatus(void);
static void Rx_FastForward(dword dwDwordCount);

/*-----------------------------------------------------------------
 .
 .  The driver can be entered at any of the following entry points.
 .
 .------------------------------------------------------------------  */

extern int eth_init(bd_t *bd);
extern void eth_halt(void);
extern int eth_rx(void);
extern int eth_send(volatile void *packet, int length);

static BOOLEAN MacNotBusy(void)
{
	int i=0;

	/*  wait for MAC not busy, w/ timeout */
	for(i=0;i<40;i++)
	{
		if((Lan_GetRegDW(MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY_)==(0UL)) {
			return TRUE;
		}
	}
	SMSC_WARNING("timeout waiting for MAC not BUSY. MAC_CSR_CMD = 0x%08lX",
		Lan_GetRegDW(MAC_CSR_CMD));
	return FALSE;
}

/* Gets a mac register value */
dword Mac_GetRegDW(dword dwRegOffset)
{
	dword result=0xFFFFFFFFUL;
	dword dwTemp=0;

	/*  wait until not busy */
	if (Lan_GetRegDW(MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY_)
	{
		SMSC_WARNING("Mac_GetRegDW() failed, MAC already busy at entry");
		goto DONE;
	}

	/*  send the MAC Cmd w/ offset */
	Lan_SetRegDW(MAC_CSR_CMD,
		((dwRegOffset & 0x000000FFUL) | MAC_CSR_CMD_CSR_BUSY_ | MAC_CSR_CMD_R_NOT_W_));
	dwTemp=Lan_GetRegDW(BYTE_TEST);/* to flush previous write */
	dwTemp=dwTemp;

	/*  wait for the read to happen, w/ timeout */
	if (!MacNotBusy())
	{
		SMSC_WARNING("Mac_GetRegDW() failed, waiting for MAC not busy after read");
		goto DONE;
	} else {
		/*  finally, return the read data */
		result=Lan_GetRegDW(MAC_CSR_DATA);
	}
DONE:
	return result;
}

/* Sets a Mac register */
void Mac_SetRegDW(dword dwRegOffset,dword dwVal)
{
	dword dwTemp=0;

	if (Lan_GetRegDW(MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY_)
	{
		SMSC_WARNING("Mac_SetRegDW() failed, MAC already busy at entry");
		goto DONE;
	}

	/*  send the data to write */
	Lan_SetRegDW(MAC_CSR_DATA,dwVal);

	/*  do the actual write */
	Lan_SetRegDW(MAC_CSR_CMD,((dwRegOffset & 0x000000FFUL) | MAC_CSR_CMD_CSR_BUSY_));
	dwTemp=Lan_GetRegDW(BYTE_TEST);/* force flush of previous write */
	dwTemp=dwTemp;

	/*  wait for the write to complete, w/ timeout */
	if (!MacNotBusy())
	{
		SMSC_WARNING("Mac_SetRegDW() failed, waiting for MAC not busy after write");
	}
DONE:
	return;
}

/* Gets a phy register */
word Phy_GetRegW(
	dword dwRegIndex)
{
	dword dwAddr=0;
	int i=0;
	word result=0xFFFFU;

	/*  confirm MII not busy */
	if ((Mac_GetRegDW(MII_ACC) & MII_ACC_MII_BUSY_) != 0UL)
	{
		SMSC_WARNING("MII is busy in Phy_GetRegW???");
		result=0;
		goto DONE;
	}

	/*  set the address, index & direction (read from PHY) */
	dwAddr = (((dwPhyAddress) & 0x1FUL)<<11) | ((dwRegIndex & 0x1FUL)<<6);
	Mac_SetRegDW(MII_ACC, dwAddr);

	/*  wait for read to complete w/ timeout */
	for(i=0;i<100;i++) {
		/*  see if MII is finished yet */
		if ((Mac_GetRegDW(MII_ACC) & MII_ACC_MII_BUSY_) == 0UL)
		{
			/*  get the read data from the MAC & return i */
			result=((word)Mac_GetRegDW(MII_DATA));
			goto DONE;
		}
	}
	SMSC_WARNING("timeout waiting for MII write to finish");

DONE:
	return result;
}

/* Sets a phy register */
void Phy_SetRegW(
	dword dwRegIndex,word wVal)
{
	dword dwAddr=0;
	int i=0;

	/*  confirm MII not busy */
	if ((Mac_GetRegDW(MII_ACC) & MII_ACC_MII_BUSY_) != 0UL)
	{
		SMSC_WARNING("MII is busy in Phy_SetRegW???");
		goto DONE;
	}

	/*  put the data to write in the MAC */
	Mac_SetRegDW(MII_DATA, (dword)wVal);

	/*  set the address, index & direction (write to PHY) */
	dwAddr = (((dwPhyAddress) & 0x1FUL)<<11) | ((dwRegIndex & 0x1FUL)<<6) | MII_ACC_MII_WRITE_;
	Mac_SetRegDW(MII_ACC, dwAddr);

	/*  wait for write to complete w/ timeout */
	for(i=0;i<100;i++) {
		/*  see if MII is finished yet */
		if ((Mac_GetRegDW(MII_ACC) & MII_ACC_MII_BUSY_) == 0UL)
		{
			goto DONE;
		}
	}
	SMSC_WARNING("timeout waiting for MII write to finish");
DONE:
	return;
}

/* Update link mode if any thing has changed */
void Phy_UpdateLinkMode()
{
	dword dwOldLinkSpeed=dwLinkSpeed;

	SMSC_TRACE("Update Link mode");

	Phy_GetLinkMode();

	switch(dwLinkSpeed) {
	case LINK_OFF:
	  SMSC_TRACE("Link is now down");
	  break;
	case LINK_SPEED_10HD:
	  SMSC_TRACE("Link is now UP at 10Mbps HD");
	  break;
	case LINK_SPEED_10FD:
	  SMSC_TRACE("Link is now UP at 10Mbps FD");
	  break;
	case LINK_SPEED_100HD:
	  SMSC_TRACE("Link is now UP at 100Mbps HD");
	  break;
	case LINK_SPEED_100FD:
	  SMSC_TRACE("Link is now UP at 100Mbps FD");
	  break;
	default:
	  SMSC_WARNING("Link is now UP at Unknown Link Speed, dwLinkSpeed=0x%08lX",
					dwLinkSpeed);
	  break;
	}

	if(dwOldLinkSpeed!=(dwLinkSpeed)) {
		if(dwLinkSpeed!=LINK_OFF) {
			dword dwRegVal=0;
			switch(dwLinkSpeed) {
			case LINK_SPEED_10HD:
				SMSC_TRACE("Link is now UP at 10Mbps HD");
				break;
			case LINK_SPEED_10FD:
				SMSC_TRACE("Link is now UP at 10Mbps FD");
				break;
			case LINK_SPEED_100HD:
				SMSC_TRACE("Link is now UP at 100Mbps HD");
				break;
			case LINK_SPEED_100FD:
				SMSC_TRACE("Link is now UP at 100Mbps FD");
				break;
			default:
				SMSC_WARNING("Link is now UP at Unknown Link Speed, dwLinkSpeed=0x%08lX",
					dwLinkSpeed);
				break;
			}

			dwRegVal=Mac_GetRegDW(MAC_CR);
			dwRegVal&=~(MAC_CR_FDPX_|MAC_CR_RCVOWN_);
			switch(dwLinkSpeed) {
			case LINK_SPEED_10HD:
			case LINK_SPEED_100HD:
				dwRegVal|=MAC_CR_RCVOWN_;
				break;
			case LINK_SPEED_10FD:
			case LINK_SPEED_100FD:
				dwRegVal|=MAC_CR_FDPX_;
				break;
			default:
				SMSC_WARNING("Unknown Link Speed, dwLinkSpeed=0x%08lX",
					dwLinkSpeed);
				break;
			}

			Mac_SetRegDW(
				MAC_CR,dwRegVal);

			if(dwLinkSettings&LINK_AUTO_NEGOTIATE) {
				word linkPartner=0;
				word localLink=0;
				localLink=Phy_GetRegW(4);
				linkPartner=Phy_GetRegW(5);
				switch(dwLinkSpeed) {
				case LINK_SPEED_10FD:
				case LINK_SPEED_100FD:
					if(((localLink&linkPartner)&((word)0x0400U)) != ((word)0U)) {
						/* Enable PAUSE receive and transmit */
						Mac_SetRegDW(FLOW,0xFFFF0002UL);
						Lan_SetBitsDW(AFC_CFG,0x0000000FUL);
					} else if(((localLink&((word)0x0C00U))==((word)0x0C00U)) &&
							((linkPartner&((word)0x0C00U))==((word)0x0800U)))
					{
						/* Enable PAUSE receive, disable PAUSE transmit */
						Mac_SetRegDW(FLOW,0xFFFF0002UL);
						Lan_ClrBitsDW(AFC_CFG,0x0000000FUL);
					} else {
						/* Disable PAUSE receive and transmit */
						Mac_SetRegDW(FLOW,0UL);
						Lan_ClrBitsDW(AFC_CFG,0x0000000FUL);
					};break;
				case LINK_SPEED_10HD:
				case LINK_SPEED_100HD:
					Mac_SetRegDW(FLOW,0UL);
					Lan_SetBitsDW(AFC_CFG,0x0000000FUL);
					break;
				default:
					SMSC_WARNING("Unknown Link Speed, dwLinkSpeed=0x%08lX\n",dwLinkSpeed);
					break;
				}
				SMSC_TRACE("LAN911x: %s,%s,%s,%s,%s,%s",
					(localLink&PHY_ANEG_ADV_ASYMP_)?"ASYMP":"     ",
					(localLink&PHY_ANEG_ADV_SYMP_)?"SYMP ":"     ",
					(localLink&PHY_ANEG_ADV_100F_)?"100FD":"     ",
					(localLink&PHY_ANEG_ADV_100H_)?"100HD":"     ",
					(localLink&PHY_ANEG_ADV_10F_)?"10FD ":"     ",
					(localLink&PHY_ANEG_ADV_10H_)?"10HD ":"     ");

				SMSC_TRACE("Partner: %s,%s,%s,%s,%s,%s",
					(linkPartner&PHY_ANEG_LPA_ASYMP_)?"ASYMP":"     ",
					(linkPartner&PHY_ANEG_LPA_SYMP_)?"SYMP ":"     ",
					(linkPartner&PHY_ANEG_LPA_100FDX_)?"100FD":"     ",
					(linkPartner&PHY_ANEG_LPA_100HDX_)?"100HD":"     ",
					(linkPartner&PHY_ANEG_LPA_10FDX_)?"10FD ":"     ",
					(linkPartner&PHY_ANEG_LPA_10HDX_)?"10HD ":"     ");
			} else {
				switch(dwLinkSpeed) {
				case LINK_SPEED_10HD:
				case LINK_SPEED_100HD:
					Mac_SetRegDW(FLOW,0x0UL);
					Lan_SetBitsDW(AFC_CFG,0x0000000FUL);
					break;
				default:
					Mac_SetRegDW(FLOW,0x0UL);
					Lan_ClrBitsDW(AFC_CFG,0x0000000FUL);
					break;
				}
			}
		} else {
			SMSC_TRACE("Link is now DOWN");
			Mac_SetRegDW(FLOW,0UL);
			Lan_ClrBitsDW(AFC_CFG,0x0000000FUL);
		}
	}
}

/* entry point for the link poller */
void Phy_CheckLink()
{
	/* must call this twice */
	Phy_UpdateLinkMode();
	Phy_UpdateLinkMode();

}

/* gets the current link mode */
void Phy_GetLinkMode()
{
	dword result=LINK_OFF;
	word wRegVal=0;
	word wRegBSR=0;

	/* Assuming MacPhyAccessLock has already been acquired */

	wRegBSR=Phy_GetRegW(PHY_BSR);
	dwLinkSettings=LINK_OFF;

	if(wRegBSR&PHY_BSR_LINK_STATUS_) {
		wRegVal=Phy_GetRegW(PHY_BCR);
		if(wRegVal&PHY_BCR_AUTO_NEG_ENABLE_) {
			dword linkSettings=LINK_AUTO_NEGOTIATE;
			word wRegADV=Phy_GetRegW(PHY_ANEG_ADV);
			word wRegLPA=Phy_GetRegW(PHY_ANEG_LPA);

			if(wRegADV&PHY_ANEG_ADV_ASYMP_) {
				linkSettings|=LINK_ASYMMETRIC_PAUSE;
			}
			if(wRegADV&PHY_ANEG_ADV_SYMP_) {
				linkSettings|=LINK_SYMMETRIC_PAUSE;
			}
			if(wRegADV&PHY_ANEG_LPA_100FDX_) {
				linkSettings|=LINK_SPEED_100FD;
			}
			if(wRegADV&PHY_ANEG_LPA_100HDX_) {
				linkSettings|=LINK_SPEED_100HD;
			}
			if(wRegADV&PHY_ANEG_LPA_10FDX_) {
				linkSettings|=LINK_SPEED_10FD;
			}
			if(wRegADV&PHY_ANEG_LPA_10HDX_) {
				linkSettings|=LINK_SPEED_10HD;
			}
			dwLinkSettings=linkSettings;
			wRegLPA&=wRegADV;
			if(wRegLPA&PHY_ANEG_LPA_100FDX_) {
				result=LINK_SPEED_100FD;
			} else if(wRegLPA&PHY_ANEG_LPA_100HDX_) {
				result=LINK_SPEED_100HD;
			} else if(wRegLPA&PHY_ANEG_LPA_10FDX_) {
				result=LINK_SPEED_10FD;
			} else if(wRegLPA&PHY_ANEG_LPA_10HDX_) {
				result=LINK_SPEED_10HD;
			}
		} else {
			if(wRegVal&PHY_BCR_SPEED_SELECT_) {
				if(wRegVal&PHY_BCR_DUPLEX_MODE_) {
					dwLinkSettings=result=LINK_SPEED_100FD;
				} else {
					dwLinkSettings=result=LINK_SPEED_100HD;
				}
			} else {
				if(wRegVal&PHY_BCR_DUPLEX_MODE_) {
					dwLinkSettings=result=LINK_SPEED_10FD;
				} else {
					dwLinkSettings=result=LINK_SPEED_10HD;
				}
			}
		}
	}
	dwLinkSpeed=result;
}

int smc_init(void);
void smc_destructor(void);
static int smc_open(bd_t *bd);
static int smc_close(void);

/*
 . Configures the PHY through the MII Management interface
*/
#ifndef CONFIG_SMC911X_EXT_PHY
static void smc_phy_configure(void);
#endif /* !CONFIG_SMC91111_EXT_PHY */

/*
 . This is a separate procedure to handle the receipt of a packet, to
 . leave the interrupt code looking slightly cleaner
*/
static int smc_rcv(void);

/* See if a MAC address is defined in the current environment. If so use it. If not
 . print a warning and set the environment and other globals with the default.
 . If an EEPROM is present it really should be consulted.
*/

int smc_get_ethaddr(bd_t *bd);
static int get_rom_mac(unsigned char *v_rom_mac);

/*
 ------------------------------------------------------------
 .
 . Internal routines
 .
 ------------------------------------------------------------
*/

static char unsigned smc_mac_addr[6] = {0x02, 0x80, 0xad, 0x20, 0x31, 0xb8};

/*
 * This function must be called before smc_open() if you want to override
 * the default mac address.
 */

void smc_set_mac_addr(const char *addr) {
	int i;

	for (i=0; i < sizeof(smc_mac_addr); i++){
		smc_mac_addr[i] = addr[i];
	}
}

/*
 * smc_get_macaddr is no longer used. If you want to override the default
 * mac address, call smc_get_mac_addr as a part of the board initialization.
 */

/*
 . A rather simple routine to print out a packet for debugging purposes.
*/

#if SMC_DEBUG > 2
static void print_packet( byte *, int );
#endif

/* this does a soft reset on the device */
static void smc_reset( void );

/* Enable Interrupts, Receive, and Transmit */
static void smc_enable( void );

/* this puts the device in an inactive state */
static void smc_shutdown( void );

/*
 . Function: smc_reset
 . Method:
 .      Init the device
*/

static void smc_reset (void)
{
	dword dwTimeOut=0;
	dword dwTemp=0;

	PRINTK2 ("%s: smc_reset\n", SMC_DEV_NAME);

	/* Reset the LAN911x */
	Lan_SetRegDW(HW_CFG,HW_CFG_SRST_);
	dwTimeOut=10;
	do {
		udelay(10);
		dwTemp=Lan_GetRegDW(HW_CFG);
		dwTimeOut--;
	} while((dwTimeOut>0)&&(dwTemp&HW_CFG_SRST_));
	if(dwTemp&HW_CFG_SRST_) {
		SMSC_WARNING("  Failed to complete reset.");
		goto DONE;
	}

	Lan_SetRegDW(HW_CFG,0x00050000UL);
	Lan_SetRegDW(AFC_CFG,0x006E3740UL);

	/* make sure EEPROM has finished loading before setting GPIO_CFG */
	dwTimeOut=50;
	while((dwTimeOut>0)&&(Lan_GetRegDW(E2P_CMD)&E2P_CMD_EPC_BUSY_)) {
		udelay(10);
		dwTimeOut--;
	}
	if(dwTimeOut==0) {
		SMSC_WARNING("Lan_Initialize: Timed out waiting for EEPROM busy bit to clear\n");
	}

	Lan_SetRegDW(GPIO_CFG,0x70070000UL);

	/* initialize interrupts */
	Lan_SetRegDW(INT_EN,0);
	Lan_SetRegDW(INT_STS,0xFFFFFFFFUL);
	Lan_SetRegDW(INT_CFG,0);

DONE:
	SMSC_TRACE("<--Lan_Initialize");
}

/*
 . Function: smc_enable
 . Purpose: let the chip talk to the outside work
 . Method:
 .	Enable the transmitter
 .	Enable the receiver
 .	Enable interrupts
*/

static void smc_enable()
{
	dword dwRegVal=0;

	/* Init Tx */

	dwRegVal=Lan_GetRegDW(HW_CFG);
	dwRegVal&=HW_CFG_TX_FIF_SZ_;
	dwRegVal|=HW_CFG_SF_;
	Lan_SetRegDW(HW_CFG,dwRegVal);

	Lan_SetBitsDW(FIFO_INT,0xFF000000UL);

	{
	  dword dwMacCr=Mac_GetRegDW(MAC_CR);
	  dwMacCr|=(MAC_CR_TXEN_|MAC_CR_HBDIS_);
	  Mac_SetRegDW(MAC_CR,dwMacCr);
	  Lan_SetRegDW(TX_CFG,TX_CFG_TX_ON_);
	}

	/* Init Rx */

	Lan_SetRegDW(RX_CFG,0x00000200UL);

	{
	  dword dwMacCr=Mac_GetRegDW(MAC_CR);
	  dwMacCr|=MAC_CR_RXEN_;
	  Mac_SetRegDW(MAC_CR,dwMacCr);
	}

	Lan_ClrBitsDW(FIFO_INT,0x000000FFUL);

	/* Disable all interrupts */

	Lan_SetBitsDW(INT_EN, 0);
}

/*
 . Function: smc_shutdown
 . Purpose:  closes down the SMC91xxx chip.
 . Method:
 .	1. zero the interrupt mask
 .	2. clear the enable receive flag
 .	3. clear the enable xmit flags
 .
 . TODO:
 .   (1) maybe utilize power down mode.
 .	Why not yet?  Because while the chip will go into power down mode,
 .	the manual says that it will wake up in response to any I/O requests
 .	in the register space.	 Empirical results do not show this working.
*/
static void smc_shutdown()
{
	PRINTK2(CARDNAME ": smc_shutdown\n");
}

/* Writes a packet to the TX_DATA_FIFO */
static void Tx_WriteFifo(
	dword *pdwBuf,
	dword dwDwordCount)
{
	volatile dword *pdwReg;
	pdwReg = (volatile dword *)(SMC_BASE_ADDRESS+TX_DATA_FIFO);
	while(dwDwordCount)
	{
		*pdwReg = *pdwBuf++;
		dwDwordCount--;
	}
}

/* Gets the number of Tx Statuses in the fifo */
static dword Tx_GetTxStatusCount()
{
	dword result=0;
	result=Lan_GetRegDW(TX_FIFO_INF);
	result&=TX_FIFO_INF_TSUSED_;
	result>>=16;
	return result;
}

/* gets a tx status out of the status fifo */
static dword Tx_CompleteTx()
{
	dword result=0;
	result=Lan_GetRegDW(TX_FIFO_INF);

	result&=TX_FIFO_INF_TSUSED_;
	if(result!=0x00000000UL) {
		result=Lan_GetRegDW(TX_STATUS_FIFO);
	} else {
		result=0;
	}

	return result;
}

/* reads tx statuses and increments counters where necessary */
void Tx_UpdateTxCounters()
{
	dword dwTxStatus=0;
	while((dwTxStatus=Tx_CompleteTx())!=0)
	{
		if(dwTxStatus&0x80000000UL) {
			SMSC_WARNING("Packet tag reserved bit is high");
			/* In this driver the packet tag is used as the packet */
			/*   length. Since a packet length can never reach */
			/*   the size of 0x8000, I made this bit reserved */
			/*   so if I ever decided to use packet tracking  */
			/*   tags then those tracking tags would set the  */
			/*   reserved bit. And I would use this control path */
			/*   to look up the packet and perhaps free it. */
			/*   As you can see I never persued this idea. */
			/*   because it never provided any benefit in this */
			/*   linux environment. */
			/* But it is worth noting that the "reserved bit" */
			/*   in the warning above does not reference a */
			/*   hardware defined reserved bit but rather a  */
			/*   driver defined reserved bit.  */
		} else {
			if(dwTxStatus&0x00008000UL) {
			  SMSC_WARNING("Tx error sending packet");
			}
		}
	}
}

/*
 .	This sends the actual packet to the SMC911x chip.
*/

static int smc_send_packet (volatile void *packet, int packet_length)
{
	int length;
	dword dwFreeSpace=0;
	dword dwTxCmdA=0;
	dword dwTxCmdB=0;

	PRINTK3 ("%s: smc_hardware_send_packet\n", SMC_DEV_NAME);

	length = ETH_ZLEN < packet_length ? packet_length : ETH_ZLEN;

	/* I can send the packet now.. */

#if SMC_DEBUG > 2
	printf ("Transmitting Packet\n");
	print_packet ((byte*)packet, length);
#endif

	dwFreeSpace=Lan_GetRegDW(TX_FIFO_INF);
	dwFreeSpace&=TX_FIFO_INF_TDFREE_;
	if(dwFreeSpace<TX_FIFO_LOW_THRESHOLD) {
		SMSC_WARNING("Tx Data Fifo Low, space available = %ld",dwFreeSpace);
	}
	dwTxCmdA=
		((((dword)(packet))&0x03UL)<<16) | /* dword alignment adjustment */
		TX_CMD_A_FIRST_SEG_ | TX_CMD_A_LAST_SEG_ |
		((dword)(length));
	dwTxCmdB=
		(((dword)(length))<<16) |
		((dword)(length));
	Lan_SetRegDW(TX_DATA_FIFO,dwTxCmdA);
	Lan_SetRegDW(TX_DATA_FIFO,dwTxCmdB);
	Tx_WriteFifo(
		(dword *)(((dword)(packet))&0xFFFFFFFCUL),
		(((dword)(length))+3+
		(((dword)(packet))&0x03UL))>>2);

	dwFreeSpace-=(length+32);

	if(Tx_GetTxStatusCount()>=30)
	{
		Tx_UpdateTxCounters();
	}

	if(dwFreeSpace<TX_FIFO_LOW_THRESHOLD) {
		dword temp=Lan_GetRegDW(FIFO_INT);
		temp&=0x00FFFFFFUL;
		temp|=0x32000000UL;
		Lan_SetRegDW(FIFO_INT,temp);
	}

	return length;

}

/*-------------------------------------------------------------------------
 |
 | smc_destructor( struct net_device * dev )
 |   Input parameters:
 |	dev, pointer to the device structure
 |
 |   Output:
 |	None.
 |
 ---------------------------------------------------------------------------
*/
void smc_destructor()
{
	PRINTK2(CARDNAME ": smc_destructor\n");
}


/*
 * Open and Initialize the board
 *
 * Set up everything, reset the card, etc ..
 *
 */
static int smc_open (bd_t * bd)
{
	int err;

	err = 0;

	PRINTK2 ("%s: smc_open\n", SMC_DEV_NAME);

	/* Detect if smc_chip type */

	dwIdRev=Lan_GetRegDW(ID_REV);
	if(HIWORD(dwIdRev)==LOWORD(dwIdRev)) {
		/* this may mean the chip is set for 32 bit  */
		/*   while the bus is reading as 16 bit */
UNKNOWN_CHIP:
		SMSC_WARNING("LAN911x NOT Identified, dwIdRev==0x%08lX",dwIdRev);
		err = 1;
		goto DONE;
	}
	switch(dwIdRev&0xFFFF0000UL) {
	case 0x01180000UL:
		SMSC_TRACE("LAN9118 identified, dwIdRev==0x%08lX",dwIdRev);break;
	case 0x01170000UL:
		SMSC_TRACE("LAN9117 identified, dwIdRev==0x%08lX",dwIdRev);break;
	case 0x01160000UL:
		SMSC_TRACE("LAN9116 identified, dwIdRev==0x%08lX",dwIdRev);break;
	case 0x01150000UL:
		SMSC_TRACE("LAN9115 identified, dwIdRev==0x%08lX",dwIdRev);break;
	default:
		goto UNKNOWN_CHIP;
	}

	/* reset the hardware */
	smc_reset ();
	smc_enable();

	/* Configure the PHY */
#ifndef CONFIG_SMC911X_EXT_PHY
	smc_phy_configure ();
#endif

	err = smc_get_ethaddr (bd);	/* set smc_mac_addr, and sync it with u-boot globals */

	if (err < 0) {
		memset (bd->bi_enetaddr, 0, 6); /* hack to make error stick! upper code will abort if not set */
		return (-1);	/* upper code ignores this, but NOT bi_enetaddr */
	}

	/* Set Mac address */
	{
	  dword dwHigh16, dwLow32;

	  dwLow32  = smc_mac_addr[0]|(smc_mac_addr[1] << 8)|(smc_mac_addr[2] << 16)|(smc_mac_addr[3] << 24);
	  dwHigh16 = smc_mac_addr[4]|(smc_mac_addr[5] << 8);

	  Mac_SetRegDW(ADDRH,dwHigh16);
	  Mac_SetRegDW(ADDRL,dwLow32);
	}

	Phy_CheckLink();

DONE:
	return err;
}

static void Rx_ReadFifo(
	dword *pdwBuf,
	dword dwDwordCount)
{
	const volatile dword * const pdwReg =
		(const volatile dword * const)(SMC_BASE_ADDRESS+RX_DATA_FIFO);

	while (dwDwordCount)
	{
		*pdwBuf++ = *pdwReg;
		dwDwordCount--;
	}
}

/* Gets the next rx status */
static dword Rx_PopRxStatus()
{
	dword result=Lan_GetRegDW(RX_FIFO_INF);
	if(result&0x00FF0000UL) {
		/* Rx status is available, read it */
		result=Lan_GetRegDW(RX_STATUS_FIFO);
	} else {
		result=0;
	}
	return result;
}

/* This function is used to quickly dump bad packets */
void Rx_FastForward(dword dwDwordCount)
{
	if(dwDwordCount>=4)
	{
		dword dwTimeOut=500;
		Lan_SetRegDW(RX_DP_CTRL,RX_DP_CTRL_RX_FFWD_);
		while((dwTimeOut)&&(Lan_GetRegDW(RX_DP_CTRL)&RX_DP_CTRL_RX_FFWD_))
		{
			udelay(1);
			dwTimeOut--;
		}
		if(dwTimeOut==0) {
			SMSC_WARNING("timed out waiting for RX FFWD to finish, RX_DP_CTRL=0x%08lX",
				Lan_GetRegDW(RX_DP_CTRL));
		}
	} else {
		while(dwDwordCount) {
			dword dwTemp=Lan_GetRegDW(RX_DATA_FIFO);
			dwTemp=dwTemp;
			dwDwordCount--;
		}
	}
}

/*-------------------------------------------------------------
 .
 . smc_rcv -  receive a packet from the card
 .
 . There is ( at least ) a packet waiting to be read from
 . chip-memory.
 .
 . o Read the status
 . o If an error, record it
 . o otherwise, read in the packet
 --------------------------------------------------------------
*/

static int smc_rcv()
{
	dword dwRxStatus=0;

	if ((dwRxStatus=Rx_PopRxStatus())!=0)
	{
		dword dwPacketLength=((dwRxStatus&0x3FFF0000UL)>>16);
		if((dwRxStatus&RX_STS_ES_)==0) {
			Rx_ReadFifo(
			  ((dword *)NetRxPackets[0]),
			  (dwPacketLength+2+3)>>2);

#if	SMC_DEBUG > 2
			printf("Receiving Packet\n");
			print_packet( NetRxPackets[0], dwPacketLength );
#endif
			NetReceive(NetRxPackets[0]+2, dwPacketLength-2);
			return dwPacketLength;
		}

		/* if we get here then the packet is to be read */
		/*   out of the fifo and discarded */
		dwPacketLength+=(2+3);
		dwPacketLength>>=2;
		Rx_FastForward(dwPacketLength);
	}

	return 0;
}


/*----------------------------------------------------
 . smc_close
 .
 . this makes the board clean up everything that it can
 . and not talk to the outside world.	Caused by
 . an 'ifconfig ethX down'
 .
 -----------------------------------------------------*/
static int smc_close()
{
	PRINTK2("%s: smc_close\n", SMC_DEV_NAME);

	/* clear everything */
	smc_shutdown();

	return 0;
}

/*------------------------------------------------------------
 . Configures the specified PHY using Autonegotiation. Calls
 . smc_phy_fixed() if the user has requested a certain config.
 .-------------------------------------------------------------*/

#ifndef CONFIG_SMC91111_EXT_PHY

static void Phy_SetLink()
{
	word wTemp;
	word status;
	int timeout;

	SMSC_TRACE("-->Phy_SetLink");

	/* Because this is part of the single threaded initialization */
	/*   path there is no need to acquire the MacPhyAccessLock */

	wTemp=Phy_GetRegW(PHY_ANEG_ADV);
	/* Advertise all speeds and pause capabilities */
	wTemp|=(PHY_ANEG_ADV_PAUSE_|PHY_ANEG_ADV_SPEED_);
	Phy_SetRegW(PHY_ANEG_ADV,wTemp);

	/*  begin to establish link */
	Phy_SetRegW(PHY_BCR,
		PHY_BCR_AUTO_NEG_ENABLE_|
		PHY_BCR_RESTART_AUTO_NEG_);

	/* Wait for autoneg to complete */
	timeout = CONFIG_SMC_AUTONEG_TIMEOUT * 100;
	do {

		status = Phy_GetRegW(PHY_BSR);
		if (status & PHY_BSR_AUTO_NEG_COMP_) {
	SMSC_TRACE("-->Phy_SetLink autoneg complete");
			/* auto-negotiate complete */
			break;
		}

		udelay(5000);	/* wait 500 millisecs */

		/* Restart auto-negotiation if remote fault */
		if (status & PHY_BSR_REMOTE_FAULT_) {
			printf ("%s: PHY remote fault detected\n",
				SMC_DEV_NAME);

			/* Restart auto-negotiation */
			printf ("%s: PHY restarting auto-negotiation\n",SMC_DEV_NAME);
			Phy_SetRegW(PHY_BCR,
				    PHY_BCR_AUTO_NEG_ENABLE_|
				    PHY_BCR_RESTART_AUTO_NEG_);
		}
	} while (timeout--);

	if (timeout < 1) {
		printf ("%s: PHY auto-negotiate timed out\n", SMC_DEV_NAME);
	}

	if (status & PHY_BSR_REMOTE_FAULT_) {
		printf ("%s: PHY remote fault detected\n", SMC_DEV_NAME);
	}
}

static void smc_phy_configure ()
{
	word wTemp=0;
	word wPhyId1=0;
	dword wPhyId2=0;
	dword dwLoopCount=0;
	dword dwPhyAddr = SMC_PHY_ADDR;

	SMSC_TRACE("-->Phy_Initialize");

	if(dwPhyAddr!=0xFFFFFFFFUL) {
		switch(dwIdRev&0xFFFF0000) {
		case 0x01170000UL:
		case 0x01150000UL:
			{
				dword dwHwCfg=Lan_GetRegDW(HW_CFG);
				if(dwHwCfg&HW_CFG_EXT_PHY_DET_) {
		    /* External phy is requested, supported, and detected */
					/* Attempt to switch */
					/* NOTE: Assuming Rx and Tx are stopped */
					/*   because Phy_Initialize is called before  */
					/*   Rx_Initialize and Tx_Initialize */

					/* Disable phy clocks to the mac */
					dwHwCfg&= (~HW_CFG_PHY_CLK_SEL_);
					dwHwCfg|= HW_CFG_PHY_CLK_SEL_CLK_DIS_;
					Lan_SetRegDW(HW_CFG,dwHwCfg);
					udelay(10);/* wait for clocks to acutally stop */

					/* switch to external phy */
					dwHwCfg|=HW_CFG_EXT_PHY_EN_;
					Lan_SetRegDW(HW_CFG,dwHwCfg);

					/* Enable phy clocks to the mac */
					dwHwCfg&= (~HW_CFG_PHY_CLK_SEL_);
					dwHwCfg|= HW_CFG_PHY_CLK_SEL_EXT_PHY_;
					Lan_SetRegDW(HW_CFG,dwHwCfg);
					udelay(10);/* wait for clocks to actually start */

					dwHwCfg|=HW_CFG_SMI_SEL_;
					Lan_SetRegDW(HW_CFG,dwHwCfg);

					/* Because this is part of the single threaded initialization */
					/*   path there is no need to acquire the MacPhyAccessLock */
					if(dwPhyAddr<=31) {
						/* only check the phy address specified */
						dwPhyAddress=dwPhyAddr;
						wPhyId1=Phy_GetRegW(PHY_ID_1);
						wPhyId2=Phy_GetRegW(PHY_ID_2);
					} else {
						/* auto detect phy */
						dword address=0;
						for(address=0;address<=31;address++) {
							dwPhyAddress=address;
							wPhyId1=Phy_GetRegW(PHY_ID_1);
							wPhyId2=Phy_GetRegW(PHY_ID_2);
							if((wPhyId1!=0xFFFFU)||(wPhyId2!=0xFFFFU)) {
								SMSC_TRACE("Detected Phy at address = 0x%02lX = %ld",
									address,address);
								break;
							}
						}
						if(address>=32) {
							SMSC_WARNING("Failed to auto detect external phy");
						}
					}
					if((wPhyId1==0xFFFFU)&&(wPhyId2==0xFFFFU)) {
						SMSC_WARNING("External Phy is not accessable");
						SMSC_WARNING("  using internal phy instead");
						/* revert back to interal phy settings. */

						/* Disable phy clocks to the mac */
						dwHwCfg&= (~HW_CFG_PHY_CLK_SEL_);
						dwHwCfg|= HW_CFG_PHY_CLK_SEL_CLK_DIS_;
						Lan_SetRegDW(HW_CFG,dwHwCfg);
						udelay(10);/* wait for clocks to actually stop */

						/* switch to internal phy */
						dwHwCfg&=(~HW_CFG_EXT_PHY_EN_);
						Lan_SetRegDW(HW_CFG,dwHwCfg);

						/* Enable phy clocks to the mac */
						dwHwCfg&= (~HW_CFG_PHY_CLK_SEL_);
						dwHwCfg|= HW_CFG_PHY_CLK_SEL_INT_PHY_;
						Lan_SetRegDW(HW_CFG,dwHwCfg);
						udelay(10);/* wait for clocks to actually start */

						dwHwCfg&=(~HW_CFG_SMI_SEL_);
						Lan_SetRegDW(HW_CFG,dwHwCfg);
						goto USE_INTERNAL_PHY;
					} else {
						SMSC_TRACE("Successfully switched to external phy");
					}
				} else {
					SMSC_WARNING("No External Phy Detected");
					SMSC_WARNING("  using internal phy instead");
					goto USE_INTERNAL_PHY;
				}
			};break;
		default:
			SMSC_WARNING("External Phy is not supported");
			SMSC_WARNING("  using internal phy instead");
			goto USE_INTERNAL_PHY;
		}
	} else {
USE_INTERNAL_PHY:
		SMSC_TRACE("Using internal phy");
		dwPhyAddress=1;
	}

	wPhyId1=Phy_GetRegW(PHY_ID_1);
	wPhyId2=Phy_GetRegW(PHY_ID_2);
	if((wPhyId1==0xFFFFU)&&(wPhyId2==0xFFFFU)) {
		SMSC_WARNING("Phy Not detected");
		goto DONE;
	}

	dwLinkSpeed=LINK_OFF;
	dwLinkSettings=LINK_OFF;
	/* reset the PHY */
	Phy_SetRegW(PHY_BCR,PHY_BCR_RESET_);
	dwLoopCount=100000;
	do {
		udelay(10);
		wTemp=Phy_GetRegW(PHY_BCR);
		dwLoopCount--;
	} while((dwLoopCount>0) && (wTemp&PHY_BCR_RESET_));
	if(wTemp&PHY_BCR_RESET_) {
		SMSC_WARNING("PHY reset failed to complete.");
		goto DONE;
	}

	Phy_SetLink();

	/*
	init_timer(&(LinkPollingTimer));
	LinkPollingTimer.function=Phy_CheckLink;
	LinkPollingTimer.data=(unsigned long)privateData;
	LinkPollingTimer.expires=jiffies+HZ;
	add_timer(&(LinkPollingTimer));
	*/


DONE:
	SMSC_TRACE("<--Phy_Initialize");
}
#endif /* !CONFIG_SMC91111_EXT_PHY */


#if SMC_DEBUG > 2
static void print_packet( byte * buf, int length )
{
	int i;
	int remainder;
	int lines;

	printf("Packet of length %d \n", length );

#if SMC_DEBUG > 3
	lines = length / 16;
	remainder = length % 16;

	for ( i = 0; i < lines ; i ++ ) {
		int cur;

		for ( cur = 0; cur < 8; cur ++ ) {
			byte a, b;

			a = *(buf ++ );
			b = *(buf ++ );
			printf("%02x%02x ", a, b );
		}
		printf("\n");
	}
	for ( i = 0; i < remainder/2 ; i++ ) {
		byte a, b;

		a = *(buf ++ );
		b = *(buf ++ );
		printf("%02x%02x ", a, b );
	}
	printf("\n");
#endif
}
#endif

int eth_init(bd_t *bd) {
	return (smc_open(bd));
}

void eth_halt() {
	smc_close();
}

int eth_rx() {
	return smc_rcv();
}

int eth_send(volatile void *packet, int length) {
	return smc_send_packet(packet, length);
}

int smc_get_ethaddr (bd_t * bd)
{
	int env_size, rom_valid, env_present = 0, reg;
	char *s = NULL, *e,  es[] = "11:22:33:44:55:66";
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

	for (reg = 0; reg < 6; ++reg) { /* turn string into mac value */
		v_env_mac[reg] = s ? simple_strtoul (s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}

	rom_valid = get_rom_mac (v_rom_mac);	/* get ROM mac value if any */

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

	if (env_present && rom_valid) { /* if both env and ROM are good */
		if (memcmp (v_env_mac, v_rom_mac, 6) != 0) {
			printf ("\nWarning: MAC addresses don't match:\n");
			printf ("\tHW MAC address:  "
				"%02X:%02X:%02X:%02X:%02X:%02X\n",
				v_rom_mac[0], v_rom_mac[1],
				v_rom_mac[2], v_rom_mac[3],
				v_rom_mac[4], v_rom_mac[5] );
			printf ("\t\"ethaddr\" value: "
				"%02X:%02X:%02X:%02X:%02X:%02X\n",
				v_env_mac[0], v_env_mac[1],
				v_env_mac[2], v_env_mac[3],
				v_env_mac[4], v_env_mac[5]) ;
			debug ("### Set MAC addr from environment\n");
		}
	}
	memcpy (bd->bi_enetaddr, v_mac, 6);	/* update global address to match env (allows env changing) */
	smc_set_mac_addr ((char*)v_mac);        /* use old function to update smc default */
	PRINTK("Using MAC Address %02X:%02X:%02X:%02X:%02X:%02X\n", v_mac[0], v_mac[1],
		v_mac[2], v_mac[3], v_mac[4], v_mac[5]);
	return (0);
}

static int get_rom_mac (unsigned char *v_rom_mac)
{
	dword dwHigh16=0;
	dword dwLow32=0;

	dwHigh16=Mac_GetRegDW(ADDRH);
	dwLow32=Mac_GetRegDW(ADDRL);

	if((dwHigh16==0x0000FFFFUL)&&(dwLow32==0xFFFFFFFF))
	  return 0;

	v_rom_mac[0]=LOBYTE(LOWORD(dwLow32));
	v_rom_mac[1]=HIBYTE(LOWORD(dwLow32));
	v_rom_mac[2]=LOBYTE(HIWORD(dwLow32));
	v_rom_mac[3]=HIBYTE(HIWORD(dwLow32));
	v_rom_mac[4]=LOBYTE(LOWORD(dwHigh16));
	v_rom_mac[5]=HIBYTE(LOWORD(dwHigh16));

	return 1;
}
#endif /* CONFIG_DRIVER_SMC911X */
