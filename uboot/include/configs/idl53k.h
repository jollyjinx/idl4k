/*
 * (C) Copyright 2008-2011 STMicroelectronics.
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

/* #define CONFIG_USE_FTA_SSA */

#define CONFIG_SH4	1				/* This is an SH4 CPU */
#define CONFIG_CPU_SUBTYPE_SH4_3XX	/* it is an SH4-300   */

#define CONFIG_PLATFORM_SETTINGS_INIT

/*-----------------------------------------------------------------------
 * Are we booting directly from a NAND Flash device ?
 * If so, then define the "CFG_BOOT_FROM_NAND" macro,
 * otherwise (e.g. NOR/SPI Flash booting), do not define it.
 */
#undef CFG_BOOT_FROM_NAND			/* define to build a NAND-bootable image */


/*-----------------------------------------------------------------------
 * Are we booting directly from a SPI Serial Flash device ?
 * If so, then define the "CFG_BOOT_FROM_SPI" macro,
 * otherwise (e.g. for NOR/NAND Flash booting), do not define it.
 */
#undef CFG_BOOT_FROM_SPI			/* define to build a SPI-bootable image */


/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * Assume we run out of uncached memory for the moment
 */

#if defined(CFG_BOOT_FROM_SPI)		/* we are booting from SPI, so *DO* swap CSA and CSC with JE2 */
#define CFG_EMI_SPI_BASE		0xA0000000	/* CSA: SPI Flash,  Physical 0x00000000 (64MiB) */
#define CFG_EMI_NOR_BASE		0xA4000000	/* CSB: NOR Flash,  Physical 0x04000000 (32MiB) */
#define CFG_EMI_NAND_BASE		0xA6000000	/* CSC: NAND Flash, Physical 0x06000000 (8MiB) */
#define CFG_NAND_FLEX_CSn_MAP	{ 0 }		/* NAND is on Chip Select CSA */
#elif defined(CFG_BOOT_FROM_NAND)	/* we are booting from NAND, so *DO* swap CSA and CSC with JE2 */
#define CFG_EMI_NAND_BASE		0xA0000000	/* CSA: NAND Flash, Physical 0x00000000 (64MiB) */
#define CFG_EMI_NOR_BASE		0xA4000000	/* CSB: NOR Flash,  Physical 0x04000000 (8MiB) */
#define CFG_NAND_FLEX_CSn_MAP	{ 0 }		/* NAND is on Chip Select CSA */
#else								/* else, do *NOT* swap CSA and CSB in EPLD */
#define CFG_EMI_NOR_BASE		0xA0000000	/* CSA: NOR Flash,  Physical 0x00000000 (64MiB) */
#define CFG_EMI_NAND_BASE		0xA4000000	/* CSB: NAND Flash, Physical 0x04000000 (8MiB) */
#define CFG_NAND_FLEX_CSn_MAP	{ 1 }		/* NAND is on Chip Select CSB */
#endif /* CFG_BOOT_FROM_NAND */

#ifdef CONFIG_SH_SE_MODE
#define CFG_FLASH_BASE			CFG_EMI_NOR_BASE	/* NOR FLASH (uncached) via PMB */
#define CFG_SE_PHYSICAL_BASE	0x40000000			/* LMI Physical Address */
#define CFG_SDRAM_BASE			0x80000000			/* LMI    Cached addr via PMB */
#define CFG_SE_UNACHED_BASE     0x90000000          /* LMI UN-cached addr via PMB */
#define CFG_SE_SDRAM_WINDOW		(CFG_SDRAM_SIZE-1)
#else
#define CFG_FLASH_BASE			CFG_EMI_NOR_BASE	/* NOR FLASH in P2 region */
#define CFG_SDRAM_BASE			0x8C000000			/* SDRAM in P1 region */
#endif

#define CFG_SDRAM_SIZE          0x10000000          /* 256 MiB of LMI SDRAM */

#define CFG_MONITOR_LEN			0x00050000			/* Reserve 320 KiB for Monitor */
#define CFG_MONITOR_BASE		0xA0000000
#define CFG_MALLOC_LEN			(1 << 20)			/* Reserve 1 MiB for malloc */
#define CFG_BOOTPARAMS_LEN		(64<< 10)			/* 64 KiB */
#define CFG_GBL_DATA_SIZE		1024				/* Global data structures */
#define CFG_LINUX_KERNEL_BASE	0xA0080000			// 0x60000 - monitor + 0x20000 - env+rom
#define CFG_LINUX_LOADER_BASE   0xA02C0000			// KernelBase + KernelSize

#ifdef CONFIG_SH_SE_MODE
#define CFG_MEMTEST_START		CFG_SE_PHYSICAL_BASE
#else
#define CFG_MEMTEST_START		CFG_SDRAM_BASE
#endif
#define CFG_MEMTEST_END			(CFG_MEMTEST_START + CFG_SDRAM_SIZE - (3 << 20))

#define CONFIG_BAUDRATE			115200
#define CFG_BAUDRATE_TABLE		{ 9600, 19200, 38400, 57600, 115200 }

#define XSTR(s) STR(s)
#define STR(s) #s

#define CONFIG_EXTRA_ENV_SETTINGS \
    "load_addr=" XSTR(CFG_LOAD_ADDR) "\0" \
    "updateub=" \
      "usb start;" \
      "protect off 0xA0000000 +0x40000;" \
      "fatload usb 0 $load_addr u-boot.bin;" \
      "erase 0xA0000000 +$filesize;" \
      "cp.b $load_addr 0xA0000000 $filesize;" \
      "protect on 0xA0000000 +0x40000\0" \
    "updateib=" \
      "usb start;" \
      "protect off 0xA0000000 +0x40000;" \
      "fatload usb 0 $load_addr i-boot.bin;" \
      "erase 0xA0000000 +$filesize;" \
      "cp.b $load_addr 0xA0000000 $filesize;" \
      "protect on 0xA0000000 +0x40000\0" \
    "updatek=" \
      "usb start;" \
      "protect off 0xA0080000 +0x240000;" \
      "fatload usb 0 $load_addr vmlinux.ub;" \
      "erase 0xA0080000 +$filesize;" \
      "cp.b $load_addr 0xA0080000 $filesize;" \
      "protect on 0xA0080000 +0x240000\0" \
    "updatel=" \
      "usb start;" \
      "protect off 0xA02C0000 +0x540000;" \
      "fatload usb 0 $load_addr module_4.lbs;" \
      "erase 0xA02C0000 +$filesize;" \
      "cp.b $load_addr 0xA02C0000 $filesize;" \
      "protect on 0xA02C0000 +0x540000\0" \
    "updatefs=" \
      "usb start;" \
      "fatload usb 0 $load_addr idl52k.jffs2;" \
      "nand erase 0x00000000 0x03000000;" \
      "nand write.jffs2 $load_addr 0x00000000 $filesize\0" \
    "updatedb=" \
      "usb start;" \
      "fatload usb 0 $load_addr idl52k.db;" \
      "nand erase 0x03000000 0x05000000;" \
      "nand write.jffs2 $load_addr 0x03000000 $filesize\0"

/*--------------------------------------------------------------
 * Command line configuration.
 */
#define CONFIG_CMD_BDI		/* bdinfo			*/
#define CONFIG_CMD_ECHO		/* echo arguments		*/
#define CONFIG_CMD_ENV		/* saveenv			*/
#define CONFIG_CMD_FLASH	/* flinfo, erase, protect	*/
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_RUN		/* run command in env variable	*/

#undef CONFIG_CMD_NET		/* bootp, tftpboot, rarpboot	*/
#undef CONFIG_CMD_NFS		/* NFS support			*/
#undef CONFIG_CMD_PING
#undef CONFIG_CMD_DHCP
#undef CONFIG_CMD_MII

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
# define CFG_STM_ASC_BASE	ST40_ASC1_REGS_BASE  /* UART #1 */

/*---------------------------------------------------------------
 * Ethernet driver config
 */

/* are we using the internal ST GMAC device ? */
#define CONFIG_DRIVER_NET_STM_GMAC

/*
 * Select the appropriate base address for the GMAC.
 * Also, choose which PHY to use.
 */
#ifdef CONFIG_DRIVER_NET_STM_GMAC
#	define CFG_STM_STMAC_BASE	0xfd110000ul	/* MAC = STM GMAC0 */
#	define CONFIG_STMAC_KSZ8041					/* PHY = Micrel KSZ8041NL */
#else
#	undef CONFIG_CMD_NET		/* undefine if no networking at all */
#endif	/* CONFIG_DRIVER_NET_STM_GMAC */

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
#	define CFG_USB0_BASE			0xfe100000	/* upper USB */
#	define CFG_USB1_BASE			0xfea00000	/* lower USB */
#	define CFG_USB_BASE				CFG_USB1_BASE
#	define CONFIG_SH_STX_STX7105_USB_PORT0		/* enable Port #0 */
#	define CONFIG_SH_STX_STX7105_USB_PORT1		/* enable Port #1 */
#	define CONFIG_SH_STX_STX7105_USB_OC	1		/* use overcurrent */
#	define CONFIG_SH_STX_STX7105_USB_PW	1		/* use power control */
#	define CFG_USB_OHCI_REGS_BASE		(CFG_USB_BASE+0xffc00)
#	define CFG_USB_OHCI_SLOT_NAME		"ohci"
#	define CFG_USB_OHCI_MAX_ROOT_PORTS	1
#	define LITTLEENDIAN
#endif	/* ifdef CONFIG_SH_STB7100_USB */

/*---------------------------------------------------------------
 * SATA driver config
 */

/* SATA works on cut 3.x of the STx7105 (just one port) */
/* Choose if we want to use a SATA HDD */
//#define CONFIG_SH_STM_SATA

#ifdef CONFIG_SH_STM_SATA
#	define CONFIG_CMD_IDE				/* enable "ide" command set */
#	define CFG_ATA_BASE_ADDR	0xfe209000	/* E-SATA connector */
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
#	undef CONFIG_CMD_EXT2
#endif

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 */

#define CFG_HUSH_PARSER		1
#define CONFIG_AUTO_COMPLETE	1
#define CFG_LONGHELP		1		/* undef to save memory		*/
#define CFG_PROMPT		"idl53k> "	/* Monitor Command Prompt	*/
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
 * FLASH organization
 */

/* Choose if we want FLASH Support (NAND &/or NOR devices)
 * Note: by default CONFIG_CMD_FLASH is defined in config_cmd_default.h
 */
#define CONFIG_CMD_FLASH	/* define for NOR flash */
#define CONFIG_CMD_NAND		/* define for NAND flash */

/*-----------------------------------------------------------------------
 * NOR FLASH organization
 */

/* PC28Fxxx: 128MiB 1024 blocks, 128KiB block size */
#ifdef CONFIG_CMD_FLASH				/* NOR flash present ? */
#	define CFG_FLASH_CFI_DRIVER
#	define CFG_FLASH_CFI
#	define CONFIG_FLASH_PROTECT_SINGLE_CELL
#	define CFG_FLASH_PROTECTION	1	/* use hardware flash protection	*/
#	define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#	define CFG_MAX_FLASH_SECT	1024	/* max number of sectors on one chip	*/
#	define CFG_FLASH_EMPTY_INFO		/* test if each sector is empty		*/
#	define MTDPARTS_NOR						\
	"physmap-flash:"  /* First NOR flash device */    \
	"320k(U-Boot)"    /* first partition */   \
	",64k(Environment)"          \
	",64k(nps-a)"          \
	",64k(nps-b)"          \
	",2304k(Kernel)"           \
	",-(Loader)"  /* last partition */
#	define MTDIDS_NOR						\
	"nor0=physmap-flash"	/* First NOR flash device */
#else
#	undef CONFIG_CMD_IMLS			/* NOR-flash specific */
#	define CFG_NO_FLASH			/* NOR-flash specific */
#endif	/* CONFIG_CMD_FLASH */

/*-----------------------------------------------------------------------
 * NAND FLASH organization
 */

/* NAND01GW3B: 1GiB  8-bit, large page (128KiB) */
#ifdef CONFIG_CMD_NAND				/* NAND flash present ? */
#	define CFG_MAX_NAND_DEVICE	1
#	define NAND_MAX_CHIPS		CFG_MAX_NAND_DEVICE
#	define CFG_NAND0_BASE		CFG_EMI_NAND_BASE
#	define CFG_NAND_BASE_LIST	{ CFG_NAND0_BASE }
#	define MTDPARTS_NAND						\
	"gen_nand.1:"		/* First NAND flash device */		\
    "48M(root-nand0)"         \
    ",-(RestOfNand0)" /* last partition */
#	define MTDIDS_NAND						\
	"nand0=gen_nand.1"	/* First NAND flash device */

	/*
	 * Currently, there are 2 main modes to read/write from/to
	 * NAND devices on STM SoCs:
	 *	a) "bit-banging" (can NOT be used in boot-from-NAND)
	 *	b) FLEX-mode (only supported means for boot-from-NAND)
	 * If CFG_NAND_FLEX_MODE is defined, then FLEX-mode will be
	 * used, otherwise, "bit-banging" will be used.
	 */
#	define CFG_NAND_FLEX_MODE	/* define to use NAND FLEX-MODE */

	/*
	 * Do we want to read/write NAND Flash compatible with the ST40's
	 * NAND Controller H/W IP block for "boot-mode"? If we want
	 * to read/write NAND flash that is meant to support booting
	 * from NAND, then we need to use 3 bytes of ECC per 128 byte
	 * record.  If so, then define the "CFG_NAND_ECC_HW3_128" macro.
	 */
#	define CFG_NAND_ECC_HW3_128	/* define for "boot-from-NAND" compatibility */

	/*
	 * Do we want to use STMicroelectronics' proprietary AFM4 (4+3/512)
	 * ECC format, instead of Linux's traditional S/W 3/256 ECC?
	 * Note: This does *not* enable H/W AFM - you can use either
	 * "bit-banging" or STM's "FLEX-mode", it is simply the addition
	 * of the AFM4 ECC algorithm+layout that is being supported here.
	 * Note: We *can* use this H/W AFM4 (4+3/512) ECC in addition to
	 * the H/W "boot-mode" (3+1/128) ECC, on the same NAND device,
	 * to partition it, set CFG_NAND_STM_BOOT_MODE_BOUNDARY appropriately.
	 */
#	undef CFG_NAND_ECC_AFM4		/* define for AFM4 (4+3/512) ECC compatibility */

	/*
	 * If using CFG_NAND_ECC_HW3_128, then we must also define
	 * where the (high watermark) boundary is. That is, the
	 * NAND offset, below which we are in "boot-mode", and
	 * must use 3 bytes of ECC for each 128 byte record.
	 * For this offset (and above) we can use any supported
	 * ECC configuration (e.g 3/256 S/W, or 3/512 H/W).
	 */
#	define CFG_NAND_STM_BOOT_MODE_BOUNDARY (0ul)	/* 0 MiB */

	/*
	 * If we want to store the U-boot environment variables in
	 * the NAND device, then we also need to specify *where* the
	 * environment variables will be stored. Typically this
	 * would be immediately after the U-boot monitor itself.
	 * However, that *may* be a bad block. Define the following
	 * to place the environment in an appropriate good block.
	 */
#	define CFG_NAND_ENV_OFFSET (CFG_MONITOR_LEN + 0x0)	/* immediately after u-boot.bin */
#endif	/* CONFIG_CMD_NAND */


/*-----------------------------------------------------------------------
 * Address, size, & location of U-boot's Environment Sector
 */

#define CFG_ENV_SIZE				0x4000			/* 16 KiB of environment data */

#if 1 && defined(CONFIG_CMD_FLASH)					/* NOR flash present ? */
#	define CFG_ENV_IS_IN_FLASH						/* environment in NOR flash */
#	define CFG_ENV_OFFSET			CFG_MONITOR_LEN	/* immediately after u-boot.bin */
#	define CFG_ENV_SECT_SIZE		0x10000			/* 64 KiB Sector size */
#elif 1 && defined(CONFIG_CMD_NAND)					/* NAND flash present ? */
#	define CFG_ENV_IS_IN_NAND						/* environment in NAND flash */
#	define CFG_ENV_OFFSET			CFG_NAND_ENV_OFFSET
#else
#	define CFG_ENV_IS_NOWHERE						/* ENV is stored in volatile RAM */
#	undef CONFIG_CMD_ENV							/* no need for "saveenv" */
#endif	/* CONFIG_CMD_NAND */

/*----------------------------------------------------------------------
 * JFFS2 + MTD Partition support
 */

#if 0 && (defined(CONFIG_CMD_FLASH) || defined(CONFIG_CMD_NAND))
#	define CONFIG_CMD_JFFS2			/* enable JFFS2 support */
#endif

#if defined(CONFIG_CMD_JFFS2)
#	define CONFIG_JFFS2_CMDLINE		/* mtdparts command line support */
#	define CONFIG_JFFS2_NAND		/* JFFS2 support on NAND Flash */
#	if defined(CONFIG_CMD_FLASH) && defined(CONFIG_CMD_NAND) /* Both NOR + NAND */
#		define MTDPARTS_DEFAULT						\
		"mtdparts="							\
			MTDPARTS_NOR	/* NOR flash devices */			\
			";"		/* delimiter */				\
			MTDPARTS_NAND	/* NAND flash devices */
#		define MTDIDS_DEFAULT						\
			MTDIDS_NOR	/* NOR flash devices */			\
			","		/* delimiter */				\
			MTDIDS_NAND	/* NAND flash devices */
#	elif defined(CONFIG_CMD_FLASH)		/* Only NOR flash devices */
#		define MTDPARTS_DEFAULT	"mtdparts=" MTDPARTS_NOR
#		define MTDIDS_DEFAULT	MTDIDS_NOR
#	elif defined(CONFIG_CMD_NAND)		/* Only NAND flash devices */
#		define MTDPARTS_DEFAULT	"mtdparts=" MTDPARTS_NAND
#		define MTDIDS_DEFAULT	MTDIDS_NAND
#	endif	/* defined(CONFIG_CMD_FLASH) && defined(CONFIG_CMD_NAND) */
#endif	/* CONFIG_CMD_JFFS2 */

#if 1 && defined(CONFIG_USE_FTA_LIBRARY)
#	define USE_DISPLAY
#		define RAM_DISPLAY_NODE_ADDRESS		0x88500000
#		define RAM_DISPLAY_ADDRESS			0x88504000
#	define CONFIG_CMD_EEPROM
#	define CONFIG_CMD_I2C
#	define CONFIG_SELF_TEST
#	define CFG_64BIT_VSPRINTF
#endif

#if defined(CONFIG_SELF_TEST)
#	define CONFIG_SELF_TEST_I2C
#	define CONFIG_SELF_TEST_EEPROM
#	define CONFIG_SELF_TEST_TUNER
#endif

#if defined(CONFIG_CMD_EEPROM)
#	define CFG_I2C_EEPROM_ADDR       (0xA0>>1)
#	define CFG_I2C_EEPROM_ADDR_LEN   2
#endif

#if defined(CONFIG_CMD_I2C)
#	define CONFIG_I2C_BUS_B				/* Use I2C Bus "B" */
#	define CONFIG_I2C_CMD_TREE			/* use a "i2c" root command */
#	define CFG_I2C_SLAVE		0x7F	/* I2C slave address - Bogus: master-only in U-Boot */
#	define CONFIG_SOFT_I2C				/* I2C with S/W bit-banging	*/
#	define I2C_ACTIVE					/* open-drain, nothing to do */
#	define I2C_TRISTATE					/* open-drain, nothing to do */
#	define I2C_SCL(val)			do { stx7105_i2c_scl((val)); } while (0)
#	define I2C_SDA(val)			do { stx7105_i2c_sda((val)); } while (0)
#	define I2C_READ				stx7105_i2c_read()

	/*
	 * The "BOGOS" for NDELAY() may be calibrated using the
	 * following code fragment, and measuring (using an oscilloscope)
	 * the frequency of the I2C SCL pin, and adjusting
	 * NDELAY_BOGOS, until the SCL is approximately 100 kHz.
	 * (100kHz has a period of 5us + 5us).
	 *
	 *	printf("just toggling I2C SCL (100kHz frequency) ...\n");
	 *	while (1)
	 *	{
	 *		I2C_SCL(1); NDELAY(5000);
	 *		I2C_SCL(0); NDELAY(5000);
	 *	}
	 */
#	define NDELAY_BOGOS		17	/* Empirical measurement for 1ns */
#	define NDELAY(ns)									\
		do {											\
			const unsigned n_bogo = NDELAY_BOGOS;		\
			const unsigned n_ticks = 					\
				((ns)<n_bogo) ? 1u : (ns)/n_bogo;		\
			volatile unsigned n_count;					\
			for(n_count=0; n_count<n_ticks; n_count++)	\
				;	/* do nothing */					\
		} while(0)

	/*
	 * Note there are 4 * I2C_DELAY per I2C clock cycle
	 * So, 100 kHz requires an I2C delay of 2500 ns.
	 * However, this calculation only works if the S/W
	 * overhead in I2C bit-banging is negligible - which it is not!
	 * So, in practice, either I2C_DELAY or CFG_I2C_SPEED will be lower.
	 * The higher the clock frequency, the greater the difference.
	 * Empirical measurement/adjustment is recommended.
	 */
#	define CFG_I2C_SPEED	100000				/* I2C speed (Hz) */
#	define I2C_DELAY	do { NDELAY(2500); } while (0)	/* 2500 ns */
#endif	/* CONFIG_CMD_I2C */

#define CONFIG_SOFT_SPI

#if defined(CONFIG_SOFT_SPI)
#	define SPI_SCL(val)		do { stx7105_spi_scl((val)); } while (0)
#	define SPI_SDA(val)		do { stx7105_spi_sda((val)); } while (0)
#	define SPI_DELAY		do { udelay(20); } while (0)
#	define SPI_READ			stx7105_spi_read()
#	define SPI_CS(val)		do { stx7105_spi_cs ((val)); } while (0)
#	define SPI_SET_DATA_IN	do { stx7105_spi_data(1); } while (0)
#	define SPI_SET_DATA_OUT	do { stx7105_spi_data(0); } while (0)
#endif	/* CONFIG_SPI */

#endif	/* __CONFIG_H */
