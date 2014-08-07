/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2009-2011 STMicroelectronics.
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

/*
 * Boot support
 */
#include <common.h>
#include <command.h>
#include <net.h>		/* for print_IPaddr */

DECLARE_GLOBAL_DATA_PTR;

static void print_num(const char *, ulong);
#if defined(CONFIG_SH4)
static void print_mem(const char *, ulong);
static void print_mhz(const char *name, ulong value);
#endif

#ifndef CONFIG_ARM	/* PowerPC and other */

#ifdef CONFIG_PPC
static void print_str(const char *, const char *);

int do_bdinfo ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	bd_t *bd = gd->bd;
	char buf[32];

#ifdef DEBUG
	print_num ("bd address",    (ulong)bd		);
#endif
	print_num ("memstart",	    bd->bi_memstart	);
	print_num ("memsize",	    bd->bi_memsize	);
	print_num ("flashstart",    bd->bi_flashstart	);
	print_num ("flashsize",	    bd->bi_flashsize	);
	print_num ("flashoffset",   bd->bi_flashoffset	);
	print_num ("sramstart",	    bd->bi_sramstart	);
	print_num ("sramsize",	    bd->bi_sramsize	);
#if defined(CONFIG_5xx)  || defined(CONFIG_8xx) || \
    defined(CONFIG_8260) || defined(CONFIG_E500)
	print_num ("immr_base",	    bd->bi_immr_base	);
#endif
	print_num ("bootflags",	    bd->bi_bootflags	);
#if defined(CONFIG_405GP) || defined(CONFIG_405CR) || \
    defined(CONFIG_405EP) || defined(CONFIG_XILINX_ML300) || \
    defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) ||	\
    defined(CONFIG_440SP) || defined(CONFIG_440SPE)
	print_str ("procfreq",	    strmhz(buf, bd->bi_procfreq));
	print_str ("plb_busfreq",   strmhz(buf, bd->bi_plb_busfreq));
#if defined(CONFIG_405GP) || defined(CONFIG_405EP) || defined(CONFIG_XILINX_ML300) || \
    defined(CONFIG_440EP) || defined(CONFIG_440GR) || defined(CONFIG_440SPE) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
	print_str ("pci_busfreq",   strmhz(buf, bd->bi_pci_busfreq));
#endif
#else	/* ! CONFIG_405GP, CONFIG_405CR, CONFIG_405EP, CONFIG_XILINX_ML300, CONFIG_440EP CONFIG_440GR */
#if defined(CONFIG_CPM2)
	print_str ("vco",	    strmhz(buf, bd->bi_vco));
	print_str ("sccfreq",	    strmhz(buf, bd->bi_sccfreq));
	print_str ("brgfreq",	    strmhz(buf, bd->bi_brgfreq));
#endif
	print_str ("intfreq",	    strmhz(buf, bd->bi_intfreq));
#if defined(CONFIG_CPM2)
	print_str ("cpmfreq",	    strmhz(buf, bd->bi_cpmfreq));
#endif
	print_str ("busfreq",	    strmhz(buf, bd->bi_busfreq));
#endif /* CONFIG_405GP, CONFIG_405CR, CONFIG_405EP, CONFIG_XILINX_ML300, CONFIG_440EP CONFIG_440GR */
#if defined(CONFIG_MPC8220)
	print_str ("inpfreq",	    strmhz(buf, bd->bi_inpfreq));
	print_str ("flbfreq",	    strmhz(buf, bd->bi_flbfreq));
	print_str ("pcifreq",	    strmhz(buf, bd->bi_pcifreq));
	print_str ("vcofreq",	    strmhz(buf, bd->bi_vcofreq));
	print_str ("pevfreq",	    strmhz(buf, bd->bi_pevfreq));
#endif

	puts ("ethaddr     =");
	for (i=0; i<6; ++i) {
		printf ("%c%02X", i ? ':' : ' ', bd->bi_enetaddr[i]);
	}

#if defined(CONFIG_HAS_ETH1)
	puts ("\neth1addr    =");
	for (i=0; i<6; ++i) {
		printf ("%c%02X", i ? ':' : ' ', bd->bi_enet1addr[i]);
	}
#endif

#if defined(CONFIG_HAS_ETH2)
       puts ("\neth2addr    =");
       for (i=0; i<6; ++i) {
		printf ("%c%02X", i ? ':' : ' ', bd->bi_enet2addr[i]);
	}
#endif

#if defined(CONFIG_HAS_ETH3)
       puts ("\neth3addr    =");
       for (i=0; i<6; ++i) {
		printf ("%c%02X", i ? ':' : ' ', bd->bi_enet3addr[i]);
	}
#endif

#ifdef CONFIG_HERMES
	print_str ("ethspeed",	    strmhz(buf, bd->bi_ethspeed));
#endif
	puts ("\nIP addr     = ");	print_IPaddr (bd->bi_ip_addr);
	printf ("\nbaudrate    = %6ld bps\n", bd->bi_baudrate   );
	return 0;
}

#elif defined(CONFIG_NIOS) /* NIOS*/

int do_bdinfo ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	bd_t *bd = gd->bd;

	print_num ("memstart",		(ulong)bd->bi_memstart);
	print_num ("memsize",		(ulong)bd->bi_memsize);
	print_num ("flashstart",	(ulong)bd->bi_flashstart);
	print_num ("flashsize",		(ulong)bd->bi_flashsize);
	print_num ("flashoffset",	(ulong)bd->bi_flashoffset);

	puts ("ethaddr     =");
	for (i=0; i<6; ++i) {
		printf ("%c%02X", i ? ':' : ' ', bd->bi_enetaddr[i]);
	}
	puts ("\nip_addr     = ");
	print_IPaddr (bd->bi_ip_addr);
	printf ("\nbaudrate    = %ld bps\n", bd->bi_baudrate);

	return 0;
}

#elif defined(CONFIG_NIOS2) /* Nios-II */

int do_bdinfo ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	bd_t *bd = gd->bd;

	print_num ("mem start",		(ulong)bd->bi_memstart);
	print_num ("mem size",		(ulong)bd->bi_memsize);
	print_num ("flash start",	(ulong)bd->bi_flashstart);
	print_num ("flash size",	(ulong)bd->bi_flashsize);
	print_num ("flash offset",	(ulong)bd->bi_flashoffset);

#if defined(CFG_SRAM_BASE)
	print_num ("sram start",	(ulong)bd->bi_sramstart);
	print_num ("sram size",		(ulong)bd->bi_sramsize);
#endif

#if defined(CONFIG_CMD_NET)
	puts ("ethaddr     =");
	for (i=0; i<6; ++i) {
		printf ("%c%02X", i ? ':' : ' ', bd->bi_enetaddr[i]);
	}
	puts ("\nip_addr     = ");
	print_IPaddr (bd->bi_ip_addr);
#endif

	printf ("\nbaudrate    = %ld bps\n", bd->bi_baudrate);

	return 0;
}
#elif defined(CONFIG_MICROBLAZE) /* ! PPC, which leaves Microblaze */

int do_bdinfo ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	bd_t *bd = gd->bd;
	print_num ("mem start      ",	(ulong)bd->bi_memstart);
	print_num ("mem size       ",	(ulong)bd->bi_memsize);
	print_num ("flash start    ",	(ulong)bd->bi_flashstart);
	print_num ("flash size     ",	(ulong)bd->bi_flashsize);
	print_num ("flash offset   ",	(ulong)bd->bi_flashoffset);
#if defined(CFG_SRAM_BASE)
	print_num ("sram start     ",	(ulong)bd->bi_sramstart);
	print_num ("sram size      ",	(ulong)bd->bi_sramsize);
#endif
#if defined(CONFIG_CMD_NET)
	puts ("ethaddr     =");
	for (i=0; i<6; ++i) {
		printf ("%c%02X", i ? ':' : ' ', bd->bi_enetaddr[i]);
	}
	puts ("\nip_addr     = ");
	print_IPaddr (bd->bi_ip_addr);
#endif
	printf ("\nbaudrate    = %d bps\n", (ulong)bd->bi_baudrate);
	return 0;
}

#elif defined(CONFIG_M68K) /* M68K */
static void print_str(const char *, const char *);

int do_bdinfo ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	bd_t *bd = gd->bd;
	char buf[32];

	print_num ("memstart",		(ulong)bd->bi_memstart);
	print_num ("memsize",		(ulong)bd->bi_memsize);
	print_num ("flashstart",	(ulong)bd->bi_flashstart);
	print_num ("flashsize",		(ulong)bd->bi_flashsize);
	print_num ("flashoffset",	(ulong)bd->bi_flashoffset);
#if defined(CFG_INIT_RAM_ADDR)
	print_num ("sramstart",		(ulong)bd->bi_sramstart);
	print_num ("sramsize",		(ulong)bd->bi_sramsize);
#endif
#if defined(CFG_MBAR)
	print_num ("mbar",		bd->bi_mbar_base);
#endif
	print_str ("busfreq",		strmhz(buf, bd->bi_busfreq));
#ifdef CONFIG_PCI
	print_str ("pcifreq",		strmhz(buf, bd->bi_pcifreq));
#endif
#ifdef CONFIG_EXTRA_CLOCK
	print_str ("flbfreq",		strmhz(buf, bd->bi_flbfreq));
	print_str ("inpfreq",		strmhz(buf, bd->bi_inpfreq));
	print_str ("vcofreq",		strmhz(buf, bd->bi_vcofreq));
#endif
#if defined(CONFIG_CMD_NET)
	puts ("ethaddr     =");
	for (i=0; i<6; ++i) {
		printf ("%c%02X", i ? ':' : ' ', bd->bi_enetaddr[i]);
	}

#if defined(CONFIG_HAS_ETH1)
	puts ("\neth1addr    =");
	for (i=0; i<6; ++i) {
		printf ("%c%02X", i ? ':' : ' ', bd->bi_enet1addr[i]);
	}
#endif

#if defined(CONFIG_HAS_ETH2)
	puts ("\neth2addr    =");
	for (i=0; i<6; ++i) {
		printf ("%c%02X", i ? ':' : ' ', bd->bi_enet2addr[i]);
	}
#endif

#if defined(CONFIG_HAS_ETH3)
	puts ("\neth3addr    =");
	for (i=0; i<6; ++i) {
		printf ("%c%02X", i ? ':' : ' ', bd->bi_enet3addr[i]);
	}
#endif

	puts ("\nip_addr     = ");
	print_IPaddr (bd->bi_ip_addr);
#endif
	printf ("\nbaudrate    = %d bps\n", bd->bi_baudrate);

	return 0;
}

#elif defined(CONFIG_SH4)

#include "asm/socregs.h"

#if !defined(CONFIG_CMD_BDI_DUMP_EMI_BANKS)
#define CONFIG_CMD_BDI_DUMP_EMI_BANKS 1
#endif

int do_bdinfo ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	DECLARE_GLOBAL_DATA_PTR;
#if CONFIG_CMD_BDI_DUMP_EMI_BANKS
	#if !defined(ST40_EMI_SIZE)
	#define ST40_EMI_SIZE	(128 << 20)	/* EMI is usually 128 MiB */
	#endif	/* ST40_EMI_SIZE */
	#define MAX_EMI_BANKS	6	/* Maximum of 6 EMI Banks */
	const u32 emi_base = 0xa0000000u;
	u32 base[MAX_EMI_BANKS+1];	/* Base address for each bank */
	u32 enabled;			/* number of enabled EMI banks */
#endif	/* CONFIG_CMD_BDI_DUMP_EMI_BANKS */
#if defined(CONFIG_CMD_NET) || CONFIG_CMD_BDI_DUMP_EMI_BANKS
	unsigned int i;
#endif
	bd_t *bd = gd->bd;

	print_num ("boot_params",	(ulong)bd->bi_boot_params);
	print_num ("memstart",		(ulong)bd->bi_memstart);
	print_mem ("memsize",		(ulong)bd->bi_memsize);
#ifndef CFG_NO_FLASH
	print_num ("flashstart",	(ulong)bd->bi_flashstart);
	print_mem ("flashsize",		(ulong)bd->bi_flashsize);
	print_num ("flashoffset",	(ulong)bd->bi_flashoffset);
#endif /* CFG_NO_FLASH */

#if defined(CONFIG_CMD_NET)
	puts ("ethaddr     =");
	for (i=0; i<6; ++i) {
		printf ("%c%02X", i ? ':' : ' ', bd->bi_enetaddr[i]);
	}
	puts ("\nip_addr     = ");
	print_IPaddr (bd->bi_ip_addr);
#endif
	printf ("\nbaudrate    = %d bps\n", bd->bi_baudrate);

#if defined(CONFIG_SH_STB7100)
	if (STB7100_DEVICEID_7109(bd->bi_devid))
		printf ("\nSTb7109 version %ld.x", STB7100_DEVICEID_CUT(bd->bi_devid));
	else if (STB7100_DEVICEID_7100(bd->bi_devid))
		printf ("\nSTb7100 version %ld.x", STB7100_DEVICEID_CUT(bd->bi_devid));
#elif defined(CONFIG_SH_STX5197)
	if (STX5197_DEVICEID_5197(bd->bi_devid))
		printf ("\nSTx5197 version %ld.x", STX5197_DEVICEID_CUT(bd->bi_devid));
#elif defined(CONFIG_SH_STX5206)
	if (STX5206_DEVICEID_5206(bd->bi_devid))
		printf ("\nSTx5206/STx5289 version %ld.x", STX5206_DEVICEID_CUT(bd->bi_devid));
#elif defined(CONFIG_SH_STX7105)
	if (STX7105_DEVICEID_7105(bd->bi_devid))
		printf ("\nSTx7105 version %ld.x", STX7105_DEVICEID_CUT(bd->bi_devid));
	else if (STX7105_DEVICEID_7106(bd->bi_devid))
		printf ("\nSTx7106 version %ld.x", STX7105_DEVICEID_CUT(bd->bi_devid));
#elif defined(CONFIG_SH_STX7108)
	if (STX7108_DEVICEID_7108(bd->bi_devid))
		printf ("\nSTx7108 version %ld.x", STX7108_DEVICEID_CUT(bd->bi_devid));
#elif defined(CONFIG_SH_STX7111)
	if (STX7111_DEVICEID_7111(bd->bi_devid))
		printf ("\nSTx7111 version %ld.x", STX7111_DEVICEID_CUT(bd->bi_devid));
#elif defined(CONFIG_SH_STX7141)
	if (STX7141_DEVICEID_7141(bd->bi_devid))
		printf ("\nSTx7141 version %ld.x", STX7141_DEVICEID_CUT(bd->bi_devid));
#elif defined(CONFIG_SH_STX7200)
	if (STX7200_DEVICEID_7200(bd->bi_devid))
		printf ("\nSTx7200 version %ld.x", STX7200_DEVICEID_CUT(bd->bi_devid));
#elif defined(CONFIG_SH_FLI7510)
	if (FLI7510_DEVICEID_7510(bd->bi_devid))
		printf ("\nFLI7510 version %ld.x", FLI7510_DEVICEID_CUT(bd->bi_devid));
#elif defined(CONFIG_SH_FLI7540)
	if (FLI7540_DEVICEID_7540(bd->bi_devid))
		printf ("\nFLI7540 version %ld.x", FLI7540_DEVICEID_CUT(bd->bi_devid));
#else
#error Missing Device Definitions!
#endif
	else
		printf ("\nUnknown device! (id=0x%08lx)", bd->bi_devid);

#ifdef CONFIG_SH_SE_MODE
	printf ("  [32-bit mode]\n");
#else
	printf ("  [29-bit mode]\n");
#endif

#ifdef CONFIG_SH_STB7100
	print_mhz ("PLL0",		bd->bi_pll0frq);
	print_mhz ("PLL1",		bd->bi_pll1frq);
	print_mhz ("ST40  CPU",		bd->bi_st40cpufrq);
	print_mhz ("ST40  BUS",		bd->bi_st40busfrq);
	print_mhz ("ST40  PER",		bd->bi_st40perfrq);
	print_mhz ("ST231 CPU",		bd->bi_st231frq);
	print_mhz ("ST BUS",		bd->bi_stbusfrq);
	print_mhz ("EMI",		bd->bi_emifrq);
	print_mhz ("LMI",		bd->bi_lmifrq);
#else
	print_mhz ("EMI",		bd->bi_emifrq);
#endif	/* CONFIG_SH_STB7100 */

#if CONFIG_CMD_BDI_DUMP_EMI_BANKS
	enabled = *ST40_EMI_BANK_ENABLE;
	printf("#EMI Banks  = %u\n", enabled);
	if (enabled > MAX_EMI_BANKS)
	{
		printf("Error: Maximum Number of Enabled Banks should be %u\n", MAX_EMI_BANKS);
		enabled = MAX_EMI_BANKS;
	}

	/*
	 * EmiBaseAddress[5:0] == Address[27:22] (Multiple of 4MiB)
	 *
	 * Retreive all the configured EMI bank bases into base[].
	 */
	for(i=0; i<enabled; i++)
	{
		const u32 start = *ST40_EMI_BASEADDRESS(i) & 0x3fu;
		base[i] = emi_base + (start << (22));
	}
	/* last valid bank occupies all remaining space */
	base[i] = emi_base + ST40_EMI_SIZE;	/* total size of EMI is usually 128MiB */

	/*
	 * Print out the ranges of each bank.
	 */
	for(i=0; i<enabled; i++)
	{
		const u32 lower = base[i];
		const u32 upper = base[i+1];
		printf ("EMI #%u CS%c  = 0x%08X ... 0x%08X (",
			i,
			'A' + i,
			lower,
			upper-1u);
		print_size (upper-lower, ")\n");
	}
#endif	/* CONFIG_CMD_BDI_DUMP_EMI_BANKS */

	return 0;
}

#else /* ! SH4 || PPC, which leaves MIPS */

int do_bdinfo ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	bd_t *bd = gd->bd;

	print_num ("boot_params",	(ulong)bd->bi_boot_params);
	print_num ("memstart",		(ulong)bd->bi_memstart);
	print_num ("memsize",		(ulong)bd->bi_memsize);
	print_num ("flashstart",	(ulong)bd->bi_flashstart);
	print_num ("flashsize",		(ulong)bd->bi_flashsize);
	print_num ("flashoffset",	(ulong)bd->bi_flashoffset);

	puts ("ethaddr     =");
	for (i=0; i<6; ++i) {
		printf ("%c%02X", i ? ':' : ' ', bd->bi_enetaddr[i]);
	}
	puts ("\nip_addr     = ");
	print_IPaddr (bd->bi_ip_addr);
	printf ("\nbaudrate    = %d bps\n", bd->bi_baudrate);

	return 0;
}
#endif  /* MIPS */

#else	/* ARM */

int do_bdinfo ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	bd_t *bd = gd->bd;

	print_num ("arch_number",	bd->bi_arch_number);
	print_num ("env_t",		(ulong)bd->bi_env);
	print_num ("boot_params",	(ulong)bd->bi_boot_params);

	for (i=0; i<CONFIG_NR_DRAM_BANKS; ++i) {
		print_num("DRAM bank",	i);
		print_num("-> start",	bd->bi_dram[i].start);
		print_num("-> size",	bd->bi_dram[i].size);
	}

	puts ("ethaddr     =");
	for (i=0; i<6; ++i) {
		printf ("%c%02X", i ? ':' : ' ', bd->bi_enetaddr[i]);
	}
	puts  ( "\n"
		"ip_addr     = ");
	print_IPaddr (bd->bi_ip_addr);
	printf ("\n"
		"baudrate    = %d bps\n", bd->bi_baudrate);

	return 0;
}

#endif /* CONFIG_ARM XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */

static void print_num(const char *name, ulong value)
{
	printf ("%-12s= 0x%08lX\n", name, value);
}

#if defined(CONFIG_SH4)
static void print_mem(const char *name, ulong value)
{
	printf ("%-12s= 0x%08lX\t(", name, value);
	print_size (value, ")\n");
}
static void print_mhz(const char *name, ulong value)
{
	printf ("%-12s= %3lu MHz\n", name, value);
}
#endif

#if defined(CONFIG_PPC) || defined(CONFIG_M68K)
static void print_str(const char *name, const char *str)
{
	printf ("%-12s= %6s MHz\n", name, str);
}
#endif	/* CONFIG_PPC */


/* -------------------------------------------------------------------- */

U_BOOT_CMD(
	bdinfo,	1,	1,	do_bdinfo,
	"bdinfo  - print Board Info structure\n",
	NULL
);
