/*
 * (C) Copyright 2007,2009-2010 STMicroelectronics.
 *
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


#if defined(CONFIG_SPI)


/**********************************************************************/


#if defined(CONFIG_SPI_FLASH_ATMEL)
/* For Atmel AT45DB321D Serial Flash */
#define CFG_STM_SPI_MODE	SPI_MODE_3
#if !defined(CFG_STM_SPI_FREQUENCY)
#  define CFG_STM_SPI_FREQUENCY	(5*1000*1000)	/* 5 MHz */
#endif	/* CFG_STM_SPI_FREQUENCY */
#define CFG_STM_SPI_DEVICE_MASK	0x3cu		/* Mask Bits [5:2] */
#define CFG_STM_SPI_DEVICE_VAL	0x34u		/* Binary xx1101xx */

#define OP_READ_STATUS		0xd7u		/* Status Register Read */
#define OP_READ_DEVID		0x9fu		/* Manufacturer & Device ID Read */
//#define OP_READ_ARRAY		0xe8u		/* Continuous Array Read */
//#define OP_READ_ARRAY		0x0bu		/* Continuous Array Read */
#define OP_READ_ARRAY		0x03u		/* Continuous Array Read */
#define OP_WRITE_VIA_BUFFER1	0x82u		/* Main Memory Page Program via Buffer 1 */
#define OP_WRITE_VIA_BUFFER2	0x85u		/* Main Memory Page Program via Buffer 2 */
#define OP_PAGE_TO_BUFFER1	0x53u		/* Main Memory Page to Buffer 1 Transfer */
#define OP_PAGE_TO_BUFFER2	0x55u		/* Main Memory Page to Buffer 2 Transfer */

#define SR_READY		(1u<<7)		/* Status Register Read/nBusy bit */


#elif defined(CONFIG_SPI_FLASH_ST)	/******************************/


/* For ST M25Pxx Serial Flash */
#define CFG_STM_SPI_MODE	SPI_MODE_3
#if !defined(CFG_STM_SPI_FREQUENCY)
#  define CFG_STM_SPI_FREQUENCY	(5*1000*1000)	/* 5 MHz */
#endif	/* CFG_STM_SPI_FREQUENCY */
#define CFG_STM_SPI_DEVICE_MASK	0x60u		/* Mask Bits [6:5] */
#define CFG_STM_SPI_DEVICE_VAL	0x00u		/* Binary x00xxxxx */

#define OP_READ_STATUS		0x05u		/* Read Status Register */
#define OP_WRITE_STATUS		0x01u		/* Write Status Register */
#define OP_READ_DEVID		0x9fu		/* Read ID */
#define OP_READ_ARRAY		0x03u		/* Read Data Bytes */
#define OP_WREN			0x06u		/* Write Enable */
#define OP_SE			0xD8u		/* Sector Erase */
#define OP_SSE			0x20u		/* Sub-Sector Erase, for M25PXxx */
#define OP_PP			0x02u		/* Page Program */

#define SR_WIP			(1u<<0)		/* Status Register Write In Progress bit */
#define SR_BP_MASK		0x1c		/* Block Protect Bits (BP[2:0]) */


#elif defined(CONFIG_SPI_FLASH_MXIC)	/******************************/


/* For Macronix MX25Lxxx Serial Flash */
#define CFG_STM_SPI_MODE	SPI_MODE_3
#if !defined(CFG_STM_SPI_FREQUENCY)
#  define CFG_STM_SPI_FREQUENCY	(5*1000*1000)	/* 5 MHz */
#endif	/* CFG_STM_SPI_FREQUENCY */
#define CFG_STM_SPI_DEVICE_MASK	0x00u		/* Mask Bits */
#define CFG_STM_SPI_DEVICE_VAL	0x00u		/* Binary xxxxxxxx */

#define OP_READ_STATUS		0x05u		/* Read Status Register */
#define OP_WRITE_STATUS		0x01u		/* Write Status Register */
#define OP_READ_DEVID		0x9fu		/* Read ID */
#define OP_READ_ARRAY		0x03u		/* Read Data Bytes */
#define OP_WREN			0x06u		/* Write Enable */
#define OP_SE			0x20u		/* Sector Erase */
#define OP_PP			0x02u		/* Page Program */

#define SR_WIP			(1u<<0)		/* Status Register Write In Progress bit */
#define SR_BP_MASK		0x3c		/* Block Protect Bits (BP[3:0]) */


#elif defined(CONFIG_SPI_FLASH_WINBOND)	/******************************/


/* For Winbond W25Q32V Serial Flash */
#define CFG_STM_SPI_MODE	SPI_MODE_3
#if !defined(CFG_STM_SPI_FREQUENCY)
#  define CFG_STM_SPI_FREQUENCY	(5*1000*1000)	/* 5 MHz */
#endif	/* CFG_STM_SPI_FREQUENCY */
#define CFG_STM_SPI_DEVICE_MASK	0x00u		/* Mask Bits */
#define CFG_STM_SPI_DEVICE_VAL	0x00u		/* Binary xxxxxxxx */

#define OP_READ_STATUS		0x05u		/* Read Status Register */
#define OP_WRITE_STATUS		0x01u		/* Write Status Register */
#define OP_READ_DEVID		0x9fu		/* Read ID */
#define OP_READ_ARRAY		0x03u		/* Read Data Bytes */
#define OP_WREN			0x06u		/* Write Enable */
#define OP_SE			0x20u		/* Sector Erase */
#define OP_PP			0x02u		/* Page Program */

#define SR_WIP			(1u<<0)		/* Status Register Write In Progress bit */
#define SR_BP_MASK		0x1c		/* Block Protect Bits (BP[2:0]) */


#else					/******************************/

#error Please specify which SPI Serial Flash is being used

#endif	/* defined(CONFIG_STM_SPI_xxxxxx) */


/**********************************************************************/


#endif	/* defined(CONFIG_SPI) */


