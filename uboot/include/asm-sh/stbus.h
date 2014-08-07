/*
 * Copyright (C) 2007-2008 STMicroelectronics Limited
 * David McKay <David.McKay@st.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __STBUS_H__
#define __STBUS_H__

/*
 * This file attempts to support all the various flavours of USB wrappers,
 * thus some of the registers appear to overlap.
 *
 * Some of these register are described in ADCS 7518758 and 7618754
 */


/* Defines for the USB controller register offsets. */
#define AHB2STBUS_WRAPPER_GLUE_BASE	(CFG_USB_BASE + 0x00000000)
#define AHB2STBUS_OHCI_BASE		(CFG_USB_BASE + 0x000ffc00)
#define AHB2STBUS_EHCI_BASE		(CFG_USB_BASE + 0x000ffe00)
#define AHB2STBUS_PROTOCOL_BASE		(CFG_USB_BASE + 0x000fff00)


/* Protocol converter registers (separate registers) */

/* The transaction opcode is programmed in this register */
#define AHB2STBUS_STBUS_OPC		(AHB2STBUS_PROTOCOL_BASE + 0x00)
#define AHB2STBUS_STBUS_OPC_4BIT	0x00
#define AHB2STBUS_STBUS_OPC_8BIT	0x01
#define AHB2STBUS_STBUS_OPC_16BIT	0x02
#define AHB2STBUS_STBUS_OPC_32BIT	0x03
#define AHB2STBUS_STBUS_OPC_64BIT	0x04

/* The message length in number of packets is programmed in this register. */
#define AHB2STBUS_MSGSIZE		(AHB2STBUS_PROTOCOL_BASE + 0x04)
#define AHB2STBUS_MSGSIZE_DISABLE	0x0
#define AHB2STBUS_MSGSIZE_2		0x1
#define AHB2STBUS_MSGSIZE_4		0x2
#define AHB2STBUS_MSGSIZE_8		0x3
#define AHB2STBUS_MSGSIZE_16		0x4
#define AHB2STBUS_MSGSIZE_32		0x5
#define AHB2STBUS_MSGSIZE_64		0x6

/* The chunk size in number of packets is programmed in this register */
#define AHB2STBUS_CHUNKSIZE		(AHB2STBUS_PROTOCOL_BASE + 0x08)
#define AHB2STBUS_CHUNKSIZE_DISABLE	0x0
#define AHB2STBUS_CHUNKSIZE_2		0x1
#define AHB2STBUS_CHUNKSIZE_4		0x2
#define AHB2STBUS_CHUNKSIZE_8		0x3
#define AHB2STBUS_CHUNKSIZE_16		0x4
#define AHB2STBUS_CHUNKSIZE_32		0x5
#define AHB2STBUS_CHUNKSIZE_64		0x6


/* Protocol converter registers (combined register) */

#define AHB2STBUS_STBUS_CONFIG		(AHB2STBUS_PROTOCOL_BASE + 0x04)


/* Wrapper Glue registers */

#define AHB2STBUS_STRAP			(AHB2STBUS_WRAPPER_GLUE_BASE + 0x14)
#define AHB2STBUS_STRAP_PLL		0x08	/* PLL_PWR_DWN */
#define AHB2STBUS_STRAP_8_BIT		0x00	/* ss_word_if */
#define AHB2STBUS_STRAP_16_BIT		0x04	/* ss_word_if */


/* QQQ move the following to somewhere more sensible */
#define USB_FLAGS_STRAP_8BIT			(1<<0)
#define USB_FLAGS_STRAP_16BIT			(2<<0)
#define USB_FLAGS_STRAP_PLL			(1<<2)
#define USB_FLAGS_OPC_MSGSIZE_CHUNKSIZE		(1<<3)
#define USB_FLAGS_STBUS_CONFIG_THRESHOLD128	(1<<4)
#define USB_FLAGS_STBUS_CONFIG_THRESHOLD256	(2<<4)


/* function to start the USB Host Controller Wrapper */
extern int ST40_start_host_control(unsigned int flags);

#endif	/* __STBUS_H__ */
