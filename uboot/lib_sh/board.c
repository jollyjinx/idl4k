/*
 * (C) Copyright 2004-2010 STMicroelectronics.
 *
 * Andy Sturges <andy.sturges@st.com>
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

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <version.h>
#include <devices.h>
#include <version.h>
#include <net.h>
#include <environment.h>
#if defined(CONFIG_CMD_NAND)
#include <nand.h>
#endif
#if defined(CONFIG_SPI)
#include <spi.h>
#endif
#include <asm/socregs.h>
#include <asm/st40reg.h>

/*
 * Currently, there are several macros which define where SDRAM
 * starts, how big it is, and where various things in RAM are located.
 * Unfortunately, it is possible to define these different
 * macros such that the overall set is mutually inconsistent!
 * In the future, it should be a goal to define only TWO of these
 * macros, and derive all the others automatically. To this end
 * the following code will issue compile-time diagnostics, if
 * the proposed derivations would fail!
 *
 * The two essential macros to be defined:
 *	CFG_SDRAM_BASE, CFG_SDRAM_SIZE
 *
 * Derived Macros:
 * 	CFG_LOAD_ADDR       = CFG_SDRAM_BASE
 *	CFG_MEMTEST_START   = CFG_SDRAM_BASE
 *	CFG_MEMTEST_END     = CFG_SDRAM_BASE + CFG_SDRAM_SIZE - 3MiB
 *	TEXT_BASE           = CFG_SDRAM_BASE + CFG_SDRAM_SIZE - 1MiB
 *	CFG_SE_SDRAM_WINDOW = CFG_SDRAM_SIZE - 1
 *
 *	Note: The 3 MiB figure above should be confirmed!
 *
 * The "mtest" command will totally trash the system, if the address
 * U-Boot is running from (starting at TEXT_BASE) is included the
 * range of memory we are testing. We ensure here that the "default"
 * range that "mtest" uses is not stupid!
 * This is done only as a compile-time test.
 */
#if (TEXT_BASE >= CFG_MEMTEST_START) && (TEXT_BASE < CFG_MEMTEST_END)
#	warning "mtest" will fail when CFG_MEMTEST_START < TEXT_BASE < CFG_MEMTEST_END!
#endif

#if CFG_LOAD_ADDR != CFG_SDRAM_BASE
#	warning CFG_LOAD_ADDR != CFG_SDRAM_BASE
#endif

#if !defined(CONFIG_SH_SE_MODE)	/* only in 32-bit mode */
#	if CFG_MEMTEST_START != CFG_SDRAM_BASE
#		warning CFG_MEMTEST_START != CFG_SDRAM_BASE
#	endif
#else
#	if CFG_MEMTEST_START != CFG_SE_PHYSICAL_BASE
#		warning CFG_MEMTEST_START != CFG_SE_PHYSICAL_BASE
#	endif
#endif

#if CFG_MEMTEST_END != (CFG_MEMTEST_START + CFG_SDRAM_SIZE - (3 << 20))
#	warning CFG_MEMTEST_END != CFG_SDRAM_BASE + CFG_SDRAM_SIZE - 3MiB
#endif

#if TEXT_BASE != (CFG_SDRAM_BASE + CFG_SDRAM_SIZE - (1 << 20))
#	warning TEXT_BASE != CFG_SDRAM_BASE + CFG_SDRAM_SIZE - 1MiB
#endif

#if defined(CONFIG_SH_SE_MODE)	/* only in 32-bit mode */
#   if !defined(CFG_SE_SDRAM_WINDOW)
#	warning CFG_SE_SDRAM_WINDOW is not defined in 32-bit mode
#   elif CFG_SE_SDRAM_WINDOW != (CFG_SDRAM_SIZE - 1)
#	warning CFG_SE_SDRAM_WINDOW != CFG_SDRAM_SIZE - 1
#   endif
#endif	/* CONFIG_SH_SE_MODE */

#if defined(CONFIG_USE_FTA_LIBRARY)
#include "fta/definition.h"
#endif

extern ulong _uboot_end_data;
extern ulong _uboot_end;

ulong monitor_flash_len;

#ifndef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING ""
#endif

const char version_string[] =
	U_BOOT_VERSION" (" __DATE__ " - " __TIME__ ") - " CONFIG_IDENT_STRING ;

/*
 * Begin and End of memory area for malloc(), and current "brk"
 */

#define	TOTAL_MALLOC_LEN	CFG_MALLOC_LEN

static ulong mem_malloc_start;
static ulong mem_malloc_end;
static ulong mem_malloc_brk;

extern int soc_init (void); 	/* Detect/set SOC settings  */
extern int board_init (void);   /* Set up board             */
extern int timer_init (void);
extern int checkboard (void);   /* Give info about board    */
#if defined(CONFIG_SPI) && defined(CFG_ENV_IS_IN_EEPROM) && !defined(CFG_BOOT_FROM_SPI)
extern int env_init_after_spi_done (void);
#endif
#ifdef CONFIG_PLATFORM_SETTINGS_INIT
extern void platformSettingsInit(void);
#endif
extern void stmac_set_mac_addr(unsigned char *Addr);

static void mem_malloc_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	ulong dest_addr = TEXT_BASE + gd->reloc_off;

	mem_malloc_end = dest_addr;
	mem_malloc_start = dest_addr - TOTAL_MALLOC_LEN;
	mem_malloc_brk = mem_malloc_start;

	memset ((void *) mem_malloc_start,
		0, mem_malloc_end - mem_malloc_start);
}

void *sbrk (ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	if ((new < mem_malloc_start) || (new > mem_malloc_end)) {
		return (NULL);
	}
	mem_malloc_brk = new;
	return ((void *) old);
}

static int init_func_ram (void)
{
	DECLARE_GLOBAL_DATA_PTR;

#ifdef	CONFIG_BOARD_TYPES
	int board_type = gd->board_type;
#endif

	gd->ram_size = CFG_SDRAM_SIZE;
	puts ("DRAM:  ");
	print_size (gd->ram_size, "\n");

	return (0);
}

static int display_banner (void)
{

	printf ("\n\n%s\n\n", version_string);
	return (0);
}

#ifndef CFG_NO_FLASH
static void display_flash_config (ulong size)
{
	puts ("NOR:   ");
	print_size (size, "\n");
}
#endif /* CFG_NO_FLASH */


static int init_baudrate (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	char tmp[64];		/* long enough for environment variables */
	int i = getenv_r ("baudrate", tmp, sizeof (tmp));

	gd->baudrate = (i > 0)
		? (int) simple_strtoul (tmp, NULL, 10)
		: CONFIG_BAUDRATE;

	return (0);
}

void flashWriteEnable(void);

/*
 * All attempts to come up with a "common" initialization sequence
 * that works for all boards and architectures failed: some of the
 * requirements are just _too_ different. To get rid of the resulting
 * mess of board dependend #ifdef'ed code we now make the whole
 * initialization sequence configurable to the user.
 *
 * The requirements for any new initalization function is simple: it
 * receives a pointer to the "global data" structure as it's only
 * argument, and returns an integer return code, where 0 means
 * "continue" and != 0 means "fatal error, hang the system".
 */
typedef int (init_fnc_t) (void);

init_fnc_t *init_sequence[] = {
	soc_init,
	timer_init,
	board_init,
	env_init,		/* initialize environment */
	init_baudrate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	console_init_f,		/* Initial console             */
	checkboard,
	display_banner,		/* say that we are here */
	init_func_ram,
	NULL,
};


void start_sh4boot (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	bd_t *bd;
	ulong addr;
	init_fnc_t **init_fnc_ptr;
#ifndef CFG_NO_FLASH
	ulong size;
#endif /* CFG_NO_FLASH */

	char *s, *e;
	int i;

	addr = TEXT_BASE;
	/* Reserve memory for malloc() arena. */
	addr -= TOTAL_MALLOC_LEN;
	/* (permanently) allocate a Board Info struct
	 * and a permanent copy of the "global" data
	 */
	addr -= sizeof (gd_t);
	gd = (gd_t *) addr;
	memset ((void *) gd, 0, sizeof (gd_t));
	addr -= sizeof (bd_t);
	bd = (bd_t *) addr;
	gd->bd = bd;

	/* Reserve memory for boot params.
	 */

	addr -= CFG_BOOTPARAMS_LEN;
	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr) () != 0) {
			hang ();
		}
	}

	gd->reloc_off = 0;
	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */

	monitor_flash_len = (ulong) & _uboot_end_data - TEXT_BASE;

	/* configure available FLASH banks */
	flashWriteEnable();
#ifndef CFG_NO_FLASH
	size = flash_init ();
	display_flash_config (size);
#endif /* CFG_NO_FLASH */

	bd = gd->bd;
	bd->bi_boot_params = addr;
	bd->bi_memstart = CFG_SDRAM_BASE;	/* start of  DRAM memory */
	bd->bi_memsize = gd->ram_size;	/* size  of  DRAM memory in bytes */
	bd->bi_baudrate = gd->baudrate;	/* Console Baudrate */
#ifndef CFG_NO_FLASH
	bd->bi_flashstart = CFG_FLASH_BASE;
	bd->bi_flashsize = size;
#if CFG_MONITOR_BASE == CFG_FLASH_BASE
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for U-Boot */
#else
	bd->bi_flashoffset = 0;
#endif
#endif /* CFG_NO_FLASH */

	/* initialize malloc() area */
	mem_malloc_init ();

#if defined(CONFIG_CMD_NAND)
	puts ("NAND:  ");
	nand_init ();		/* go init the NAND */
#endif

#if defined(CONFIG_SPI)
	puts ("SPI:  ");
	spi_init ();		/* go init the SPI */
#if defined(CFG_ENV_IS_IN_EEPROM) && !defined(CFG_BOOT_FROM_SPI)
	env_init_after_spi_done ();
#endif
#endif	/* defined(CONFIG_SPI) */

	/* Allocate environment function pointers etc. */
	env_relocate ();

#ifdef CONFIG_PLATFORM_SETTINGS_INIT
	platformSettingsInit();
#endif

	/* board MAC address */
	s = getenv ("ethaddr");
	for (i = 0; i < 6; ++i) {
		if(s) {
			bd->bi_enetaddr[i] = simple_strtoul (s, &e, 16) & 0xff;
			s = (*e) ? e + 1 : e;
		}
	}

#ifdef CONFIG_DRIVER_NET_STM_GMAC
	stmac_set_mac_addr(bd->bi_enetaddr);
#endif

	/* IP Address */
	bd->bi_ip_addr = getenv_IPaddr ("ipaddr");

#if defined(CONFIG_PCI)
	/*
	 * Do pci configuration
	 */
	pci_init ();
#endif

/** leave this here (after malloc(), environment and PCI are working) **/
	/* Initialize devices */
	devices_init ();

	jumptable_init ();

	/* Initialize the console (after the relocation and devices init) */
	console_init_r ();

/** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **/

	/* Initialize from environment */
	if ((s = getenv ("loadaddr")) != NULL) {
		load_addr = simple_strtoul (s, NULL, 16);
	}
#if defined(CONFIG_CMD_NET)
	if ((s = getenv ("bootfile")) != NULL) {
		copy_filename (BootFile, s, sizeof (BootFile));
	}
#endif /* CONFIG_CMD_NET */

#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r ();
#endif

#if defined(CONFIG_CMD_NET)
#if defined(CONFIG_NET_MULTI)
	puts ("Net:   ");
#endif
	eth_initialize(gd->bd);
#endif

#if defined(CONFIG_USE_FTA_LIBRARY)
	initFtaLib();
#endif

	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;) {
		main_loop ();
	}

	/* NOTREACHED - no way out of command loop except booting */
}


void hang (void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;);
}


static void sh_reset (void) __attribute__ ((noreturn));
static void sh_reset (void)
{
#if defined(CONFIG_CPU_SUBTYPE_SH4_2XX)		/* SH4-200 series */
	/*
	 * We will use the on-chip watchdog timer to force a
	 * power-on-reset of the device.
	 * A power-on-reset is required to guarantee all SH4-200 cores
	 * will reset back into 29-bit mode, if they were in SE mode.
	 */
		/* WTCNT          = FF	counter to overflow next tick */
	*ST40_CPG_WTCNT = 0x5AFF;

		/* WTCSR2.CKS[3]  = 0	use legacy clock dividers */
	*ST40_CPG_WTCSR2 = 0xAA00;

		/* WTCSR.TME      = 1	enable up-count counter */
		/* WTCSR.WT       = 1	watchdog timer mode */
		/* WTCSR.RSTS     = 0	enable power-on reset */
		/* WTCSR.CKS[2:0] = 2	clock division ratio 1:128 */
		/* NOTE: we need CKS to be big enough to allow
		 * U-Boot to disable the watchdog, AFTER the reset,
		 * otherwise, we enter an infinite-loop of resetting! */
	*ST40_CPG_WTCSR = 0xA5C2;
#elif defined(CONFIG_CPU_SUBTYPE_SH4_3XX)	/* SH4-300 series */
	/*
	 * However, on SH4-300 series parts, issuing a TRAP instruction
	 * with SR.BL=1 should always be sufficient.
	 */
	ulong sr;
	asm ("stc sr, %0":"=r" (sr));
	sr |= (1 << 28);	/* set block bit, SR.BL=1 */
	asm ("ldc %0, sr": :"r" (sr));
	asm volatile ("trapa #0");
#endif

	/* wait for H/W reset to kick in ... */
	for (;;);
}


extern int do_reset (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	sh_reset();
	/*NOTREACHED*/ return (0);
}
