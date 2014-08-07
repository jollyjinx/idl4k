/*
 * (C) Copyright 2007-2008 STMicroelectronics.
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

#define CFG_MONITOR_LEN		0x00040000	/* Reserve 256 KiB for Monitor */
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

#define BOARD cb102

#if CFG_MONITOR_LEN == 0x00020000		/* 128 KiB */
#	define MONITOR_SECTORS	"1:0-8"		/* 9 sectors */
#elif CFG_MONITOR_LEN == 0x00040000		/* 256 KiB */
#	define MONITOR_SECTORS	"1:0-10"	/* 11 sectors */
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
//#define CONFIG_CMD_JFFS2	/* QQQ: is this needed ? */

/*--------------------------------------------------------------
 * Serial console info
 */

/* we are using the internal ST ASC UART */
#define CONFIG_STM_ASC_SERIAL	1

/* choose which UART to use */
#define CFG_STM_ASC_BASE	0xfd033000ul	/* TERM UART3 (J8001) */

/*---------------------------------------------------------------
 * Ethernet driver config
 */

/* are we using the internal ST MAC device ? */
#define CONFIG_DRIVER_NETSTMAC
#define CONFIG_STMAC_MAC0			/* Use MAC #0 */
#define CFG_STM_STMAC_BASE	 0xfd500000ul	/* MAC #0 */

/* we use the on-board SMSC PHY */
#define CONFIG_STMAC_LAN8700			/* SMSC LAN8700 */

/*  If this board does not have eeprom for ethernet address so allow the user
 *  to set it in the environment
 */
#define CONFIG_ENV_OVERWRITE


/*---------------------------------------------------------------
 * USB driver config
 */

/* Choose if we want USB Mass-Storage Support */
#define CONFIG_SH_STB7100_USB

#ifdef CONFIG_SH_STB7100_USB
#	define CONFIG_CMD_USB
#	define CONFIG_CMD_FAT
#	define CONFIG_USB_OHCI_NEW
#	define CONFIG_USB_STORAGE
#	define CFG_USB_OHCI_CPU_INIT
#	define CFG_USB0_BASE			0xfd200000	/* ST72681 + NAND08GW3B */
#	define CFG_USB1_BASE			0xfd300000	/* Front (near RJ-45) */
#	define CFG_USB2_BASE			0xfd400000	/* Rear (J9002) */
#	define CFG_USB_BASE			CFG_USB2_BASE
#	undef CFG_USB_BASE					/* QQQ - DELETE */
#	define CFG_USB_BASE			CFG_USB2_BASE	/* QQQ - DELETE */
#	define CFG_USB_OHCI_REGS_BASE		(CFG_USB_BASE+0xffc00)
#	define CFG_USB_OHCI_SLOT_NAME		"ohci"
#	define CFG_USB_OHCI_MAX_ROOT_PORTS	1
#	define LITTLEENDIAN
#endif	/* ifdef CONFIG_SH_STB7100_USB */

/*---------------------------------------------------------------
 * IDE driver config
 */

#if defined(CONFIG_SH_STB7100_SATA) ||	\
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
#define CFG_PROMPT		"CB102> "	/* Monitor Command Prompt	*/
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

#if 1
/* M29W640GB: 8MiB, 127*64 KiB plus 8*8 KiB, a total of 135 blocks */
#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CONFIG_FLASH_PROTECT_SINGLE_CELL
#define CFG_FLASH_PROTECTION	1	/* use hardware flash protection	*/
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	135	/* max number of sectors on one chip	*/
#define CFG_FLASH_EMPTY_INFO		/* test if each sector is empty		*/
#elif 0
/* M58LT256GT: 32MiB 259 blocks, 128KiB block size plus parameter block */
#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CONFIG_FLASH_PROTECT_SINGLE_CELL
#define CFG_FLASH_PROTECTION	1	/* use hardware flash protection	*/
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	259	/* max number of sectors on one chip	*/
#define CFG_FLASH_EMPTY_INFO		/* test if each sector is empty		*/
#endif

/*-----------------------------------------------------------------------
 * NAND FLASH organization
 */

/* NAND flash is NOT visible via the EMI, but only through OHCI */

/*-----------------------------------------------------------------------
 * Address, size, & location of U-boot's Environment Sector
 */

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_OFFSET		CFG_MONITOR_LEN
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + CFG_ENV_OFFSET)
#define CFG_ENV_SIZE		0x10000		/* 64 KiB */
#define CFG_ENV_SECT_SIZE	0x10000		/* 64 KiB */

/*----------------------------------------------------------------------
 * JFFS2 support
 */

#if defined(CONFIG_CMD_JFFS2)
#define CONFIG_JFFS2_CMDLINE	/* mtdparts command line support */
#define MTDPARTS_DEFAULT						\
	"mtdparts="							\
	"physmap-flash:"	/* First NOR flash device */		\
		"256k(U-Boot)"		/* first partition */		\
		",128k(Environment)"					\
		",-(RestOfNor0)"	/* last partition */
#define MTDIDS_DEFAULT							\
	"nor0=physmap-flash"	/* First NOR flash device */
#endif	/* CONFIG_CMD_JFFS2 */

#endif	/* __CONFIG_H */

