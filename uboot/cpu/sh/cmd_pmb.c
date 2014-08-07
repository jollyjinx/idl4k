/*
 * (C) Copyright 2008 STMicroelectronics.
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

#include <common.h>
#include <command.h>
#include <asm/pmb.h>
#include <asm/addrspace.h>


//#define DEBUG

#if defined(CONFIG_SH_SE_MODE)


#define PMB_ADDR(i)	((volatile unsigned long*)(P4SEG_PMB_ADDR+((i)<<8)))
#define PMB_DATA(i)	((volatile unsigned long*)(P4SEG_PMB_DATA+((i)<<8)))


#ifdef DEBUG
#define SHOULD_BE_VALID(pmb)							\
do {										\
	if ((valid & (1u<<(pmb))) == 0u)					\
		printf ("ERROR: PMB[%u].V == 0, but it should be   Valid\n",	\
			(pmb));							\
} while(0)
#define SHOULD_BE_INVALID(pmb)							\
do {										\
	if ((valid & (1u<<(pmb))) != 0u)					\
		printf ("ERROR: PMB[%u].V == 1, but it should be INvalid\n",	\
			(pmb));							\
} while(0)
#endif	/* DEBUG */


static unsigned pmb_page_size (const unsigned long data)
{
	const unsigned long size = data & 0x90ul;

	return	(size==0x00ul) ?  16 :
		(size==0x10ul) ?  64 :
		(size==0x80ul) ? 128 :
				 512;
}

static void display_pmb (void)
{
	size_t i;
#ifdef DEBUG
	unsigned valid = 0;	/* assume all PMBs are invalid */
#endif	/* DEBUG */

	printf ("\ninfo: sizeof(LMI) = 0x%08x = %uMB (uses %u PMB slot%s)\n\n",
		CFG_SDRAM_SIZE, CFG_SDRAM_SIZE/1024/1024,
		(CFG_SH_LMI_NEEDS_2_PMB_ENTRIES) ? 2 : 1,
		(CFG_SH_LMI_NEEDS_2_PMB_ENTRIES) ? "s" : "");

	for (i=0; i<16; i++)
	{
		const unsigned long data = *PMB_DATA(i);
		const unsigned long addr = *PMB_ADDR(i);
		const unsigned long mb   = pmb_page_size(data);
		const unsigned long vpn  = addr >> 24;
		const unsigned long ppn  = data >> 24;

#ifdef DEBUG
		if ((data & 0x100) != (addr & 0x100))
			printf ("ERROR: PMB[%u].V disagrees!\n", i);
#endif	/* DEBUG */

		if (!(data & 0x100))		/* V == 0 ? */
			continue;		/* skip to next if not valid */

		/* print entry if it is valid */
		printf ("PMB[%2u]  =  VPN:%02x...%02x  PPN:%02x...%02x  %2s  %1s  %2s  %3uMB\n",
			i,
			vpn,				/* VPN (first page) */
			vpn + (mb >> 4) - 1,		/* VPN (last page) */
			ppn,				/* PPN (first page) */
			ppn + (mb >> 4) - 1,		/* PPN (last page) */
			(data & (1<<0)) ? "WT" : "CB",	/* write-through / copy-back */
			(data & (1<<3)) ? "C" : "",	/* cacheability */
			(data & (1<<9)) ? "UB" : "",	/* Unbuffered */
			mb);				/* SZ */

#ifdef DEBUG
		valid |= 1u<<i;		/* set bit 'i', in the mask set */
		switch (i)
		{
			case 0:
				if (vpn != 0x80)
					printf ("ERROR: PMB[0].VPN != 0x80\n");
				break;

			case 1:
				if (vpn != 0x88)
					printf ("ERROR: PMB[1].VPN != 0x88\n");
				if (ppn != ((*PMB_DATA(0)>>24)+0x08))
					printf ("ERROR: PMB[1].PPN != PMB[0].PPN+0x08\n");
				break;

			case 2:
				if (vpn != 0x90)
					printf ("ERROR: PMB[2].VPN != 0x90\n");
				if (ppn != (*PMB_DATA(0)>>24))
					printf ("ERROR: PMB[2].PPN != PMB[0].PPN\n");
				if ((data&0x90) != (*PMB_DATA(0)&0x90))
					printf ("ERROR: PMB[2].SZ != PMB[0].SZ\n");
				break;

			case 3:
				if (vpn != 0x98)
					printf ("ERROR: PMB[3].VPN != 0x98\n");
				if (ppn != (*PMB_DATA(1)>>24))
					printf ("ERROR: PMB[3].PPN != PMB[1].PPN\n");
				if (ppn != ((*PMB_DATA(2)>>24)+0x08))
					printf ("ERROR: PMB[3].PPN != PMB[2].PPN+0x08\n");
				if ((data&0x90) != (*PMB_DATA(1)&0x90))
					printf ("ERROR: PMB[3].SZ != PMB[1].SZ\n");
				break;

			default:
				if ( (vpn < 0xa0) || (vpn > 0xbf) )
					printf ("ERROR: PMB[%u].VPN not in range 0xa0..0xbf\n",i);
		}
#endif	/* DEBUG */
	}

#ifdef DEBUG
	/* perform additional checks on validity */
	SHOULD_BE_VALID(0);
#if CFG_SH_LMI_NEEDS_2_PMB_ENTRIES
	SHOULD_BE_VALID(1);
#else
	SHOULD_BE_INVALID(1);
#endif	/* CFG_SH_LMI_NEEDS_2_PMB_ENTRIES */
	SHOULD_BE_VALID(2);
#if CFG_SH_LMI_NEEDS_2_PMB_ENTRIES
	SHOULD_BE_VALID(3);
#else
	SHOULD_BE_INVALID(3);
#endif	/* CFG_SH_LMI_NEEDS_2_PMB_ENTRIES */
	SHOULD_BE_INVALID(15);
#endif	/* DEBUG */
}

extern int do_pmb (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	display_pmb ();
	return (0);
}


U_BOOT_CMD(
	pmb, 1, 0, do_pmb,
	"pmb     - displays the contents of the PMB\n",
	"- displays the contents of the PMB (when V==1)\n"
);

#endif	/* CONFIG_SH_SE_MODE */

