/*
 * (C) Copyright 2009 STMicroelectronics.
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


#if defined(CFG_NAND_SKIP_BAD_BLOCKS_ON_RELOCATING) && defined(CONFIG_CMD_NAND)


#include <nand.h>


#define NAND_SIGNATURE_OFFSET		0x80u
#define NAND_SIGNATURE_LENGTH		0x40u
#define NAND_BB_SKIP_OFFSET		(NAND_SIGNATURE_OFFSET + NAND_SIGNATURE_LENGTH)


typedef struct skip_bb			/* see cpu/sh/start.S for more details */
{
	uchar	pattern[8];
	u32	block_size;
	u32	num_blocks;
	u32	array[0];
} skip_bb;


static int check_skip_bb(const ulong addr)
{
	const uchar * const signature = (uchar*)(addr+NAND_SIGNATURE_OFFSET);
	const skip_bb * const ptr = (skip_bb*)(addr+NAND_BB_SKIP_OFFSET);
	const uchar pattern[] = "SKIP_BBs";
	size_t i;

	/*
	 * ensure the (64-byte) NAND magic signature is correct
	 */
	for(i=0; i<NAND_SIGNATURE_LENGTH; i++)
	{
		if (signature[i] != i)
		{
			printf ("Error: "
				"Invalid NAND signature at 0x%08x (%x != %x)\n",
				&signature[i],
				signature[i],
				i);
			return (1);	/* image is INVALID */
		}
	}

	/*
	 * ensure the (8-byte) "skip_bb" pattern is correct.
	 */
	for(i=0; i<sizeof(ptr->pattern); i++)
	{
		if (ptr->pattern[i] != pattern[i])
		{
			printf ("Error: "
				"Invalid SKIP_BB pattern at 0x%08x (%x != %x)\n",
				ptr->pattern,
				ptr->pattern[i],
				pattern[i]);
			return (1);	/* image is INVALID */
		}
	}

	/*
	 * ensure the address provided is 4-byte aligned!
	 */
	if (addr % 4ul != 0ul)
	{
		printf ("Error: "
			"address (0x%08lx) must be 4-byte aligned\n",
			addr);
		return (1);		/* 'addr' is INVALID */
	}

	/*
	 * ensure the physical NAND device has been initialized & selected.
	 */
	if ((nand_curr_device < 0) || (nand_curr_device >= CFG_MAX_NAND_DEVICE))
	{
		printf ("Error: "
			"Invalid NAND device (%d) currently selected\n",
			nand_curr_device);
		return (1);		/* NAND device is INVALID */
	}

	/*
	 * ensure the Block size agrees with physical NAND device.
	 */
	if (ptr->block_size != nand_info[nand_curr_device].erasesize)
	{
		printf ("Error: "
			"Block Size (%uKiB) does not agree with physical NAND (%uKiB)\n",
			ptr->block_size >> 10,
			nand_info[nand_curr_device].erasesize >> 10);
		return (1);		/* Block Size does NOT AGREE */
	}

	/*
	 * test to see if physical block #0 is tagged as being "BAD"
	 */
	if (ptr->array[0])
	{
		printf ("Warning: "
			"Block #0 is marked as being BAD - are you sure?\n");
	}

	return (0);			/* image is okay */
}


static int do_info(const ulong addr)
{
	const skip_bb * const ptr = (skip_bb*)(addr+NAND_BB_SKIP_OFFSET);
	size_t i;
	size_t skipped;

	if (check_skip_bb(addr))	/* is image invalid ? */
		return (1);		/* bad image */

	printf( "info: The skip_bb structure at 0x%x looks okay,\n"
		"info: There is space for %u bad blocks (each of %u KiB).\n",
		addr + NAND_BB_SKIP_OFFSET,
		ptr->num_blocks,
		ptr->block_size >> 10);

	/* how many blocks are to be skipped ? */
	for(i=skipped=0; i<ptr->num_blocks; i++)
	{
		if (ptr->array[i])
			skipped++;
	}

	if (skipped == 0)
	{
		printf("info: There are NO blocks marked to be skipped\n");
	}
	else
	{
		printf("info: There are %u blocks marked to be skipped:\n",
			skipped);
		for(i=0; i<ptr->num_blocks; i++)
		{
			if (ptr->array[i])
			{
				printf("\tblock #%u is marked to be skipped (at 0x%x)\n",
				i,
				i*ptr->block_size);
			}
		}
	}

	return (0);			/* okay */
}


static int do_copy(const ulong addr)
{
	skip_bb * const ptr = (skip_bb*)(addr+NAND_BB_SKIP_OFFSET);
	ulong start;
	size_t i;

	if (check_skip_bb(addr))	/* is image invalid ? */
		return (1);		/* bad image */

	for(i=start=0; i<ptr->num_blocks; i++,start+=ptr->block_size)
	{
		ulong read;
		nand_info_t * const nand = &nand_info[nand_curr_device];
		const ulong skip = nand_block_isbad(nand, start) ? 1 : 0;
		if (skip)	/* do we skip it ? */
		{
			printf( "info: "
				"tagging block #%u as bad (at 0x%x)\n",
				i,
				start);
		}

		/* update the array in the skip_bb structure */
		*(volatile ulong*)(&ptr->array[i]) = skip;

		/* did we succeed ? */
		read = *(volatile ulong*)(&ptr->array[i]);
		if (read != skip)
		{
			printf( "Error: "
				"For block #%u, poke to 0x%08x failed! (%u != %u)\n",
				i,
				&ptr->array[i],
				skip,
				read);
			return (1);		/* poke FAILED */
		}
	}

	return (0);			/* okay */
}


extern int do_copybbt (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if ( (argc==3) && (strcmp (argv[1], "info")==0))
	{
		const ulong addr = simple_strtoul (argv[2], NULL, 16);
		return do_info(addr);
	}
	else if ( (argc==3) && (strcmp (argv[1], "copy")==0))
	{
		const ulong addr = simple_strtoul (argv[2], NULL, 16);
		return do_copy(addr);
	}
	else
	{
		printf ("Usage:\n%s\n", cmdtp->usage);
		return (1);		/* bad command usage */
	}

	return (0);
}


U_BOOT_CMD(
	copybbt, 3, 0, do_copybbt,
	"copybbt - copies abridged version of NAND BBT\n",
	"info [addr] - shows status of copied BBT in RAM\n"
	"copybbt copy [addr] - copies BBT into RAM copy of u-boot.bin\n"
	"\t\t      this may be done prior to burning to NAND\n"
);


#endif /* CFG_NAND_SKIP_BAD_BLOCKS_ON_RELOCATING */


