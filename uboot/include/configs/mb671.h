/*
 * (C) Copyright 2007-2009 STMicroelectronics.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_SH4	1		/* This is an SH4 CPU		*/
#define CONFIG_CPU_SUBTYPE_SH4_3XX	/* it is an SH4-300		*/


/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * Assume we run out of uncached memory for the moment
 */

#ifdef CONFIG_SH_SE_MODE
#define CFG_FLASH_BASE		0xA0000000	/* FLASH (uncached) via PMB */
#define CFG_SDRAM_BASE		0x80000000      /* LMI0 via PMB */
#define CFG_SE_PHYSICAL_BASE	0x40000000	/* LMI0 Physical Address */
#define CFG_SE_UNACHED_BASE	0x90000000	/* LMI0 un-cached addr via PMB */
#define CFG_SE_SDRAM_WINDOW	(CFG_SDRAM_SIZE-1)
#else
#define CFG_FLASH_BASE		0xA0000000	/* FLASH in P2 region */
#define CFG_SDRAM_BASE		0x88000000      /* SDRAM in P1 region */
#endif

#define CFG_SDRAM_SIZE		0x10000000	/* 256 MiB of LMI0 SDRAM */

#define CFG_MONITOR_LEN		0x00020000	/* Reserve 128 KiB for Monitor */
#define CFG_MONITOR_BASE        CFG_FLASH_BASE
#define CFG_MALLOC_LEN		(1 << 20)	/* Reserve 1 MiB for malloc */
#define CFG_BOOTPARAMS_LEN	(128 << 10)	/* 128 KiB */
#define CFG_GBL_DATA_SIZE	1024		/* Global data structures */

#define CFG_MEMTEST_START	CFG_SDRAM_BASE
#define CFG_MEMTEST_END		(CFG_SDRAM_BASE + CFG_SDRAM_SIZE - (3 << 20))

#define CONFIG_BAUDRATE		115200
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define XSTR(s) STR(s)
#define STR(s) #s

#define BOARD mb671

#if CFG_MONITOR_LEN == 0x00020000		/* 128 KiB */
#	define MONITOR_SECTORS	"1:0"		/* 1 sector */
#elif CFG_MONITOR_LEN == 0x00040000		/* 256 KiB */
#	define MONITOR_SECTORS	"1:0-1"		/* 2 sectors */
#else						/* unknown */
#	error "Unable to determine sectors for monitor"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
		"board=" XSTR(BOARD) "\0" \
		"monitor_base=" XSTR(CFG_MONITOR_BASE) "\0" \
		"monitor_len=" XSTR(CFG_MONITOR_LEN) "\0" \
		"monitor_sec=" MONITOR_SECTORS "\0" \
		"load_addr=" XSTR(CFG_LOAD_ADDR) "\0" \
		"unprot=" \
		  "protect off $monitor_sec\0" \
		"update=" \
		  "erase $monitor_sec;" \
		  "cp.b $load_addr $monitor_base $monitor_len;" \
		  "protect on $monitor_sec\0"

/*--------------------------------------------------------------
 * Command line configuration.
 */

#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII

/*--------------------------------------------------------------
 * Serial console info
 */

/*
 * We can use one of two methods for the "serial" console.
 * We can either use the (normal hardware) internal ST ASC UART;
 * OR we can use STMicroelectronics' DTF (Data Transfer Format)
 * mechanism over a JTAG link to a remote GDB debugger.
 */
#if 1
#	define CONFIG_STM_ASC_SERIAL	/* use a ST ASC UART */
#else
#	define CONFIG_STM_DTF_SERIAL	/* use DTF over JTAG */
#endif

/* choose which ST ASC UART to use */
#define CFG_STM_ASC_BASE	0xfd032000ul	/* UART2 (lower) */
//#define CFG_STM_ASC_BASE	0xfd033000ul	/* UART3 (upper) */

/*---------------------------------------------------------------
 * Ethernet driver config
 */

/*
 * There are 2 options for ethernet, both use the on-chip ST-MAC.
 * The choice in PHYs is:
 *    1) The on-board SMSC LAN8700 PHY.
 *    2) External PHY connected via the MII off-board connector (MB539B).
 */

/* are we using the internal ST MAC device ? */
#define CONFIG_DRIVER_NETSTMAC

/* choose which internal MAC to use */
#if 1
#	define CONFIG_STMAC_MAC0			/* Use MAC #0 */
#else
#	define CONFIG_STMAC_MAC1			/* Use MAC #1 */
#endif

/*
 * Select the appropriate base address for the selected MAC.
 * Also, choose which external PHY type to use.
 */
#if defined(CONFIG_STMAC_MAC0)
#	define CFG_STM_STMAC_BASE	 0xfd500000ul	/* MAC #0 */
#	define CONFIG_STMAC_LAN8700			/* SMSC LAN8700 */
#elif defined(CONFIG_STMAC_MAC1)
#	define CFG_STM_STMAC_BASE	 0xfd510000ul	/* MAC #1 */
#	define CONFIG_STMAC_LAN8700			/* SMSC LAN8700 */
#endif

/*  If this board does not have eeprom for ethernet address so allow the user
 *  to set it in the environment
 */
#define CONFIG_ENV_OVERWRITE


/*---------------------------------------------------------------
 * USB driver config
 */

/* Choose if we want USB Mass-Storage Support */
//#define CONFIG_SH_STB7100_USB

#ifdef CONFIG_SH_STB7100_USB
#	define CONFIG_CMD_USB
#	define CONFIG_CMD_FAT
#	define CONFIG_USB_OHCI_NEW
#	define CONFIG_USB_STORAGE
#	define CFG_USB_OHCI_CPU_INIT
#	define CFG_USB0_BASE			0xfd200000	/* middle */
#	define CFG_USB1_BASE			0xfd300000	/* right */
#	define CFG_USB2_BASE			0xfd400000	/* left */
#	define CFG_USB_BASE			CFG_USB0_BASE
#	define CFG_USB_OHCI_REGS_BASE		(CFG_USB_BASE+0xffc00)
#	define CFG_USB_OHCI_SLOT_NAME		"ohci"
#	define CFG_USB_OHCI_MAX_ROOT_PORTS	1
#	define LITTLEENDIAN
#endif	/* ifdef CONFIG_SH_STB7100_USB */

/*---------------------------------------------------------------
 * SATA driver config
 */

/* SATA works on cut 3.x of the STx7200 (just one port) */
/* Choose if we want to use a SATA HDD */
//#define CONFIG_SH_STM_SATA

#ifdef CONFIG_SH_STM_SATA
#	define CONFIG_CMD_IDE				/* enable "ide" command set */
#	define CFG_ATA_BASE_ADDR	0xfd520000	/* SATA #0 */
#	define CFG_ATA_IDE0_OFFSET	0x800		/* Host Controller */
#	define CFG_ATA_REG_OFFSET	0x0
#	define CFG_ATA_DATA_OFFSET	0x0
#	define CFG_ATA_STRIDE		0x4
#	define CFG_IDE_MAXBUS		1
#	define CFG_IDE_MAXDEVICE	1
#endif	/* CONFIG_SH_STM_SATA */

#if defined(CONFIG_SH_STM_SATA) ||	\
    defined(CONFIG_SH_STB7100_USB)
#	define CFG_64BIT_LBA
#	define CONFIG_LBA48
#	define CONFIG_DOS_PARTITION
#	define CONFIG_CMD_EXT2
#endif

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 */

#define CFG_HUSH_PARSER		1
#define CONFIG_AUTO_COMPLETE	1
#define CFG_LONGHELP		1		/* undef to save memory		*/
#define CFG_PROMPT		"MB671> "	/* Monitor Command Prompt	*/
#define CFG_PROMPT_HUSH_PS2	"> "
#define CFG_CBSIZE		1024
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size	*/
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_HZ			1000		/* HZ for timer ticks	*/
#define CFG_LOAD_ADDR		CFG_SDRAM_BASE	/* default load address		*/
#define CONFIG_BOOTDELAY	10		/* default delay before executing bootcmd */
#define CONFIG_ZERO_BOOTDELAY_CHECK

#define CONFIG_CMDLINE_EDITING

/*-----------------------------------------------------------------------
 * NOR FLASH organization
 */

/* M58LT256GT: 32MiB 259 blocks, 128 KiB block size plus parameter block */
#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CONFIG_FLASH_PROTECT_SINGLE_CELL
#define CFG_FLASH_PROTECTION	1	/* use hardware flash protection	*/
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	259	/* max number of sectors on one chip	*/
#define CFG_FLASH_EMPTY_INFO		/* test if each sector is empty		*/

/*-----------------------------------------------------------------------
 * Addresss, size, & location of U-boot's Environment Sector
 */

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_OFFSET		CFG_MONITOR_LEN
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + CFG_ENV_OFFSET)
#define CFG_ENV_SIZE		0x10000
#define CFG_ENV_SECT_SIZE	0x20000

#endif	/* __CONFIG_H */
