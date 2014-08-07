/*
 * (C) Copyright 2006
 * Pierre Morel, WYPLAY, pmorel@wyplay.com
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

#define STATS_ON

int memory_test(unsigned long *block_start, unsigned long block_length);

int do_st_memory_test(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned long *start;
	unsigned long length;
	int ret;

	start = (unsigned long *)simple_strtoul(argv[1], NULL, 10);
	length = simple_strtoul(argv[2], NULL, 10);

	printf("\ndo_st_memory_test: Start: 0x%08x - Length: 0x%08x\n",
	       start, length);
	ret = memory_test(start, length);
	return(ret);
}

#define CONTROL_BITS(x) ((unsigned long)(x) & 0xFFF80000ul)
#define ADDRESS_BITS(x) ((unsigned long)(x) & 0x00F7FFFFul)

#define ALL_ZERO 0x00000000ul
#define ALL_FIVE 0x55555555ul
#define ALL_AAAA 0xAAAAAAAAul
#define ALL_ONES 0xFFFFFFFFul

#ifdef STATS_ON
static int stats_run_fails;
static int stats_run_pattern_fails;
static int stats_run_no_write_fails;
static int stats_run_write_00000000_fails;
static int stats_run_write_FFFFFFFF_fails;
static int stats_run_write_AAAAAAAA_fails;
static int stats_run_wrote_00000000_fails;
static int stats_run_move_00000000_fails;
static int stats_run_move_FFFFFFFF_fails;

static int stats_adj_move_00000000_fails;
static int stats_adj_move_FFFFFFFF_fails;

static int stats_total_fails;
static int stats_total_pattern_fails;
static int stats_total_no_write_fails;
static int stats_total_write_00000000_fails;
static int stats_total_write_FFFFFFFF_fails;
static int stats_total_write_AAAAAAAA_fails;
static int stats_total_wrote_00000000_fails;
static int stats_total_move_00000000_fails;
static int stats_total_move_FFFFFFFF_fails;
static int stats_total_adj_move_00000000_fails;
static int stats_total_adj_move_FFFFFFFF_fails;
static int stats_runs;

static int stats_first_test = 0;
#endif

/*{{{ void Report(*Address, Pattern, Value, *Message)*/
void report(volatile unsigned long *address, unsigned long pattern,
	    unsigned long value, char *message)
{
	printf("\r@ 0x%08x  W 0x%08x  R 0x%08x  D 0x%08x   %s\n",
	       address, pattern, value, (pattern ^ value), message);
}
/*}}}  */

/*{{{  int TestPattern(unsigned long *BlockStart, unsigned long BlockLength, unsigned long Pattern, char *Message)*/
int test_pattern(unsigned long *block_start, unsigned long block_length,
		 unsigned long pattern, char *message)
{
	volatile unsigned long *address;
	volatile unsigned long *start_address;
	volatile unsigned long *end_address;
	unsigned long value;
	int error = 0;

	start_address = block_start;
	end_address = start_address + (block_length / sizeof (unsigned int));

	printf("Data Bus Pattern test (0x%08lx) Address from 0x%08x to 0x%08x\n",
		pattern, start_address, end_address);

	/* Write Pattern */
	for (address = start_address; address < end_address - 1; address++)
		*address = pattern;

	*(address) = ~pattern ;	/* To change value on the bus */
	/* Verify Pattern */
	for (address = start_address; address < end_address-1; address++) {
		value = *address;
		if (value != pattern) {
			if(error < 5)
				report(address, pattern, value, message);
			error++;
			/*{{{  stats*/
#ifdef STATS_ON
			stats_run_fails++;
			stats_run_pattern_fails++;
#endif
			/*}}}  */
		}
	}
	printf ("Found %X errors for Pattern %x\n", error, pattern);
	return error;
}
/*}}}  */

/*{{{  int SlidingTest(unsigned long *BlockStart, int Buswidth)*/
int sliding_test(unsigned long *block_start, int buswidth)
{
	volatile unsigned long *address;
	unsigned long value;
	unsigned long pattern;
	int error = 0;
	int i;

	address = block_start;
	/*Sliding One*/
	printf("Sliding one test\n");
	pattern = 0x1;
	for (i = 0; i < buswidth; i++) {
		*address = pattern;
		*(address + 1) = 0;
		value = *address;
		if (pattern != value) {
			if(error < 20)
				report(address, pattern, value, "sliding one");
			error++;
		}
		pattern = pattern << 1;
	}

	/*Sliding Zero*/
	printf("Sliding zero test\n");
	pattern = 0xFFFFFFFE;
	for (i = 0; i < buswidth; i++) {
		*address = pattern;
		*(address +1) = ~pattern;
		value = *address;
		if (pattern != value) {
			if(error < 20)
				report(address, pattern, value, "sliding zero");
			error++;
		/*{{{  stats*/
#ifdef STATS_ON
			stats_run_fails++;
			stats_run_pattern_fails++;
#endif
		/*}}}  */
		}
		pattern = (pattern << 1) | 1;
	}
	return error;
}
/*}}}  */


/*{{{  int AddressTests(unsigned long *BlockStart, unsigned long BlockLength)*/
int address_tests(unsigned long *block_start, unsigned long block_length)
{
	volatile unsigned long *address;
	volatile unsigned long *start_address;
	volatile unsigned long *end_address;
	unsigned long memory_size;
	unsigned long value, pattern;
	int error = 0, error_verify =0;
	int dot_count;

	/* Work out start and end addresses */
	start_address = block_start;
	memory_size = block_length;
	end_address = start_address + (memory_size / sizeof (unsigned int));
	printf("Testing memory address bus from 0x%08x to 0x%08x\n",
	       start_address, end_address);

	/*Alternate location */
	printf("Alternate address test\n");
	*start_address = ALL_FIVE;
	*(start_address + 1) = ALL_AAAA;
	value = *start_address;
	if (value != ALL_FIVE) {
		if(error < 20)
			report(start_address, ALL_FIVE, value, "Alternate address test");
	/*{{{  stats*/
#ifdef STATS_ON
		stats_run_fails++;
		stats_run_pattern_fails++;
#endif
	/*}}}  */
		error++;
	}
	value = *(start_address + 1);
	if (value != ALL_AAAA) {
		if(error < 20)
			report((start_address + 1), ALL_AAAA, value, "Alternate address test");
	/*{{{  stats*/
#ifdef STATS_ON
		stats_run_fails++;
		stats_run_pattern_fails++;
#endif
	/*}}}  */
		error++;
	}
#ifdef FAST_TEST

	/*Fast Address Test */
	printf("Fast address test\n");

	address = start_address +1 ;

	for (address = start_address + 1; address < end_address;
	     address = (unsigned long *)(start_address + (ADDRESS_BITS(Address) << 2))) {
/*	     address = (unsigned long *)(CONTROL_BITS(address) | (ADDRESS_BITS(address) << 2))) {*/
		*address = ~(unsigned long)address;
		printf("checking address 0x%X\n", Address);
	}
	for (address = start_address + 1; address < end_address;
	     address = (unsigned long *)(start_address + (ADDRESS_BITS(address) << 2))) {
/*	     address = (unsigned long *)(CONTROL_BITS(address) | (ADDRESS_BITS(address) << 2))) {*/
		pattern = ~((unsigned long)address);
		value = *address;
		if (pattern != value) {
			if(error < 20)
				report(address, pattern, value, "Fast address test");
		/*{{{  stats*/
#ifdef STATS_ON
			stats_run_fails++;
			stats_run_pattern_fails++;
#endif
		/*}}}  */
			error++;
		}
	}
#endif

	/* Address Test */
	printf("Address test ... may take a while ...\n");
	dot_count = 0;
	for (address = start_address; address < end_address; address += 0x20) {
		*address = ~((unsigned long)address);
		pattern = ~((unsigned long)address);
		value = *address ;
		if (value != pattern) {
			if(error_verify < 20)
				report(address, pattern, value, "Address test");
			error_verify++;
		}
		dot_count++;
		if (dot_count >= 0x4000) {
			dot_count = 0;
			printf("w");
		}
	}
	printf("\n");
	if (error_verify !=0)
		printf("Immediate verify after write show %x error(s)\n", error_verify) ;

	dot_count = 0;
	for (address = start_address; address < end_address; address += 0x20) {
		pattern = ~((unsigned long)address);
		value = *address;
		if (pattern != value) {
			if(error < 20)
				report(address, pattern, value, "Address test");
			error++;
		}
		dot_count++;
		if (dot_count >= 0x4000) {
			dot_count = 0;
			printf("r");
		}
	}

	return error;
}
/*}}}  */

/*{{{  int BlockMoveTests(unsigned long *BlockStart, unsigned long BlockLength)*/
int block_move_tests(unsigned long *block_start, unsigned long block_length)
{
	volatile unsigned long *address;
	volatile unsigned long *start_address;
	volatile unsigned long *end_address;
	volatile unsigned long *src_start;
	volatile unsigned long *src_end;
	volatile unsigned long *dst_start;
	volatile unsigned long *dst_end;
	unsigned long memory_size, pattern, value;
	int error = 0;
	int totalerrors = 0;
	int dot_count;
	/*int i;
	int seed=5;*/

	/*Block Move Test */
	printf("\nBlock Move test ---------------------------------------------\n");
	/*Set up Block address pointers*/
	start_address = block_start;
	end_address = (start_address + (block_length / sizeof (unsigned int))-1);
	memory_size = block_length / 2 / sizeof(long);
	src_start = start_address;
	src_end = (start_address + memory_size) - 1;
	dst_start = (src_end + 1);
	dst_end = end_address;
	printf("Source Block from 0x%08x to 0x%08x\n", src_start, src_end);
	printf("Destination Block from 0x%08x to 0x%08x\n", dst_start, dst_end);

	/*{{{	Initialise Memory*/
	/*Initialise Memory*/
	printf("Fill Memory with 0x01020304\n");
	dot_count = 0;
	for (address = start_address; address < end_address; address++) {
		*address = 0x01020304;
		dot_count++;
		if (dot_count >= 0x40000) {
			dot_count = 0;
			printf("w");
		}
	}
	/*}}}	*/
	/*{{{	Verify Memory*/
	printf("\n");
	dot_count = 0;
	for (address = start_address; address < end_address; address++) {
		pattern = 0x01020304;
		value = *address;
		if (pattern != value) {
			if(error < 20)
				report(address, pattern, value, "Block Move test init 0x01020304");
			/*{{{	stats*/
#ifdef STATS_ON
			stats_run_fails++;
			if (value == 0)
				stats_run_write_00000000_fails++;
#endif
			/*}}}	*/
			error++;
		}
		dot_count++;
		if (dot_count >= 0x40000) {
			dot_count = 0;
			printf("r");
		}
	}
	/*}}}	*/

	/*{{{	Fill source block with alternating F's and 0's*/
	/*Fill Source Block*/
	printf("\nFill Source Block of Memory with F's and 0's\n");
	dot_count = 0;
	for (address = src_start; address < src_end;) {
		*address++ = 0x0;
		*address++ = 0x0;
		*address++ = 0xFFFFFFFF;
		*address++ = 0xFFFFFFFF;
		dot_count+=4;
		if (dot_count >= 0x40000) {
			dot_count = 0;
			printf("w");
		}
	}
	/*}}}	*/

	/*{{{	Fill destination block with A's*/
	/*Fill Destination Block*/
	printf("\nFill Destination Block of Memory with A's\n");
	dot_count = 0;
	for (address = dst_start; address < dst_end;) {
		*address++ = 0xAAAAAAAA;
		dot_count++;
		if (dot_count >= 0x40000) {
			dot_count = 0;
			printf("w");
		}
	}
	/*}}}	*/

	printf("\n");

	/*{{{	Verify source block with alternating F's and 0's*/
	/*Verify Source block*/
	printf("Verifying source block F's and 0's\n");
	dot_count = 0;
	for (address = src_start; address < src_end;) {
		pattern = 0x0;
		value = *address++;
		if (pattern != value) {
			if(error < 20)
				report(address, pattern, value, "Source block initialisation error");
			/*{{{	stats*/
#ifdef STATS_ON
			stats_run_fails++;
			stats_run_write_00000000_fails++;
			if (value == 0x01020304)
				stats_run_no_write_fails++;
#endif
			/*}}}	*/
			error++;
		}
		value = *address++;
		if (pattern != value) {
			if(error < 20)
				report(address, pattern, value, "Source block initialisation error");
			/*{{{	stats*/
#ifdef STATS_ON
			stats_run_fails++;
			stats_run_write_00000000_fails++;
			if (value == 0x01020304)
				stats_run_no_write_fails++;
#endif
			/*}}}	*/
			error++;
		}
		pattern = 0xFFFFFFFF;
		value = *address++;
		if (pattern != value) {
			if(error < 20)
				report(address, pattern, value, "Source block initialisation error");
			/*{{{	stats*/
#ifdef STATS_ON
			stats_run_fails++;
			stats_run_write_FFFFFFFF_fails++;
			if (value == 0)
				stats_run_wrote_00000000_fails++;
			if (value == 0x01020304)
				stats_run_no_write_fails++;
#endif
			/*}}}	*/
			error++;
		}
		value = *address++;
		if (pattern != value) {
			if(error < 20)
				report(address, pattern, value, "Source block initialisation error");
			/*{{{	stats*/
#ifdef STATS_ON
			stats_run_fails++;
			stats_run_write_FFFFFFFF_fails++;
			if (value == 0)
				stats_run_wrote_00000000_fails++;
			if (value == 0x01020304)
				stats_run_no_write_fails++;
#endif
			/*}}}	*/
			error++;
		}
		dot_count += 4;
		if (dot_count >= 0x40000) {
			dot_count = 0;
			printf("r");
		}
	}
	totalerrors = error;
	error = 0;
	/*}}}	*/

	/*{{{	Verify destination block with A's*/
	/*Verify destination block*/
	printf("\n");
	printf("Verifying Destination block with A's\n");
	dot_count = 0;
	for (address = dst_start; address < dst_end;) {
		pattern = 0xAAAAAAAA;
		value = *address++;
		if (pattern != value) {
			if(error < 20)
				report(address, pattern, value, "Destination block initialisation error");
			/*{{{	stats*/
#ifdef STATS_ON
			stats_run_fails++;
			stats_run_write_AAAAAAAA_fails++;
			if (value == 0)
				stats_run_wrote_00000000_fails++;
			if (value == 0x01020304)
				stats_run_no_write_fails++;
#endif
			/*}}}	*/
			error++;
		}
		dot_count++;
		if (dot_count >= 0x40000) {
			dot_count = 0;
			printf("r");
		}
	}
	totalerrors += error;
	error = 0;
	/*}}}	*/


	/*{{{	move source block to destination block*/
	/*Move Block*/
	printf("\nMove Block of Memory\n");
	memcpy((unsigned long *)dst_start, (unsigned long *)src_start, (int)block_length / 2);
	/*}}}	*/

	/*{{{	Verify destination block with F's and 0's*/
	/*Verify Destination Block*/
	printf("Verify Destination Block F's and 0's\n");
	dot_count = 0;
	for (address = dst_start; address < dst_end;) {
		pattern = 0x0;
		value = *address++;
		if (pattern != value) {
			if(error < 20 )
				report(address, pattern, value, "Block Move test F's and 0's fill");
			/*{{{	stats*/
#ifdef STATS_ON
			stats_run_fails++;
			stats_run_move_00000000_fails++;
			if (value == 0x01020304)
				stats_run_no_write_fails++;
			if (value == 0xAAAAAAAA)
				stats_run_no_write_fails++;
#endif
			/*}}}	*/
			error++;
		}
		value = *address++;
		if (pattern != value) {
			if(error < 20 )
				report(address, pattern, value, "Block Move test F's and 0's fill");
			/*{{{	stats*/
#ifdef STATS_ON
			stats_run_fails++;
			stats_run_move_00000000_fails++;
			if (value == 0x01020304)
				stats_run_no_write_fails++;
			if (value == 0xAAAAAAAA)
				stats_run_no_write_fails++;
#endif
			/*}}}	*/
			error++;
		}
		pattern = 0xFFFFFFFF;
		value = *address++;
		if (pattern != value) {
			if(error < 20 )
				report(address, pattern, value, "Block Move test F's and 0's fill");
			/*{{{	stats*/
#ifdef STATS_ON
			stats_run_fails++;
			stats_run_move_FFFFFFFF_fails++;
			if (value == 0)
				stats_run_wrote_00000000_fails++;
			if (value == 0x01020304)
				stats_run_no_write_fails++;
			if (value == 0xAAAAAAAA)
				stats_run_no_write_fails++;
#endif
			/*}}}	*/
			error++;
		}
		value = *address++;
		if (pattern != value) {
			if(error < 20 )
				report(address, pattern, value, "Block Move test F's and 0's fill");
			/*{{{	stats*/
#ifdef STATS_ON
			stats_run_fails++;
			stats_run_move_FFFFFFFF_fails++;
			if (value == 0)
				stats_run_wrote_00000000_fails++;
			if (value == 0x01020304)
				stats_run_no_write_fails++;
			if (value == 0xAAAAAAAA)
				stats_run_no_write_fails++;
#endif
			/*}}}	*/
			error++;
		}
		dot_count += 4;
		if (dot_count >= 0x40000) {
			dot_count = 0;
			printf("r");
		}
	}
	totalerrors += error;
	error = 0;
	/*}}}	*/

	return totalerrors;
}

/*}}}	*/

/*{{{	int MemoryTest(unsigned long *BlockStart, unsigned long BlockLength)*/
int memory_test(unsigned long *block_start, unsigned long block_length)
{
	unsigned long error = 0;
	unsigned long *start_address;
	unsigned long *end_address;
	unsigned long memory_size;
	int buswidth;


	printf("\nRunning memory tester ... V1.1\n\n");

#ifdef STATS_ON
	/*{{{	init stats*/
	if (stats_first_test == 0) {
		printf("STATS: Initialising stat counters ... \n\n");
		stats_first_test = 1;
		stats_runs = 0;

		stats_total_fails = 0;
		stats_total_pattern_fails = 0;
		stats_total_no_write_fails = 0;
		stats_total_write_00000000_fails = 0;
		stats_total_write_FFFFFFFF_fails = 0;
		stats_total_write_AAAAAAAA_fails = 0;
		stats_total_wrote_00000000_fails = 0;
		stats_total_move_00000000_fails = 0;
		stats_total_move_FFFFFFFF_fails = 0;

	}
	stats_run_fails = 0;
	stats_run_pattern_fails = 0;
	stats_run_no_write_fails = 0;
	stats_run_write_00000000_fails = 0;
	stats_run_write_FFFFFFFF_fails = 0;
	stats_run_write_AAAAAAAA_fails = 0;
	stats_run_wrote_00000000_fails = 0;
	stats_run_move_00000000_fails = 0;
	stats_run_move_FFFFFFFF_fails = 0;
	/*}}}	*/
#endif

/*{{{	data bus tests*/
#ifndef NO_PATTERN_TEST
	/* Work out start and end addresses */
	start_address = block_start;
	memory_size = 1024;
	end_address = start_address + (memory_size / sizeof (unsigned int));

	/* Pattern Tests */
	error += test_pattern(start_address, memory_size, 0x00000000, "Test pattern (00)");
	error += test_pattern(start_address, memory_size, 0xFFFFFFFF, "Test pattern (FF)");
	error += test_pattern(start_address, memory_size, 0x55555555, "Test pattern (55)");
	error += test_pattern(start_address, memory_size, 0xAAAAAAAA, "Test pattern (AA)");
#endif
/*}}}	*/

#ifdef MANUFTEST
	if (error !=0)
		return ((int) (error=1)) ;
#endif

/*{{{	Sliding tests*/
#ifndef NO_SLIDING_TEST
	start_address = block_start;
	buswidth = 32;
	error+=sliding_test(start_address, buswidth);
#endif
/*}}}	*/

#ifdef MANUFTEST
	if (error !=0)
		return ((int)(error=1)) ;
#endif

/*{{{	address tests*/
#ifndef NO_ADDRESS_TEST
	start_address = block_start;
	memory_size = block_length;
	error += address_tests(start_address, memory_size);
#endif
/*}}}	*/

#ifdef MANUFTEST
	printf("\nMemory test completed with [%ld] errors!\n\n",error);
	if (error !=0)
		return ((int)(error=1)) ;
	else return ((int)(error=0)) ;
#endif

/*{{{	Block move tests*/
#ifndef NO_BLOCK_MOVE_TEST
	start_address = block_start;
	memory_size = block_length;
	error += block_move_tests(start_address, memory_size);
#endif
/*}}}	*/

	printf("\nMemory test completed with [%ld] errors!\n\n",error);
	/*{{{	print stats*/
	#ifdef STATS_ON
	stats_runs++;
	stats_total_fails += stats_run_fails;
	stats_total_pattern_fails += stats_run_pattern_fails;
	stats_total_no_write_fails += stats_run_no_write_fails;
	stats_total_write_00000000_fails += stats_run_write_00000000_fails;
	stats_total_write_FFFFFFFF_fails += stats_run_write_FFFFFFFF_fails;
	stats_total_write_AAAAAAAA_fails += stats_run_write_AAAAAAAA_fails;
	stats_total_wrote_00000000_fails += stats_run_wrote_00000000_fails;
	stats_total_move_00000000_fails	+= stats_run_move_00000000_fails;
	stats_total_move_FFFFFFFF_fails	+= stats_run_move_FFFFFFFF_fails;

	stats_adj_move_00000000_fails =	stats_run_move_00000000_fails - stats_run_write_00000000_fails;
	stats_adj_move_FFFFFFFF_fails =	stats_run_move_FFFFFFFF_fails - stats_run_write_FFFFFFFF_fails;
	stats_total_adj_move_00000000_fails += stats_adj_move_00000000_fails;
	stats_total_adj_move_FFFFFFFF_fails += stats_adj_move_FFFFFFFF_fails;


	printf("============================================================================================\n");
	printf("STATS:                        \tLast\tTotal\tAverage(over %d runs)\n", stats_runs);
	printf("STATS:                        \t    \t     \tNumber\t%%\n");
	printf("STATS: Total fails:           \t%d\t%d\t%d\n", stats_run_fails, stats_total_fails, (stats_total_fails / stats_runs));
	if (stats_total_fails != 0)	/* ensure we avoid divide by zero! */
	{
		printf("STATS: Pattern fails:         \t%d\t%d\t%d\t%d\n", stats_run_pattern_fails, stats_total_pattern_fails, (stats_total_pattern_fails / stats_runs), ((stats_total_pattern_fails * 100) / stats_total_fails));
		printf("STATS: No write fails:        \t%d\t%d\t%d\t%d\n", stats_run_no_write_fails, stats_total_no_write_fails, (stats_total_no_write_fails / stats_runs), ((stats_total_no_write_fails * 100) / stats_total_fails));
		printf("STATS: Fail writing 0x00000000\t%d\t%d\t%d\t%d\n", stats_run_write_00000000_fails, stats_total_write_00000000_fails, (stats_total_write_00000000_fails / stats_runs), ((stats_total_write_00000000_fails * 100) / stats_total_fails));
		printf("STATS: Fail writing 0xFFFFFFFF\t%d\t%d\t%d\t%d\n", stats_run_write_FFFFFFFF_fails, stats_total_write_FFFFFFFF_fails, (stats_total_write_FFFFFFFF_fails / stats_runs), ((stats_total_write_FFFFFFFF_fails * 100) / stats_total_fails));
		printf("STATS: Fail writing 0xAAAAAAAA\t%d\t%d\t%d\t%d\n", stats_run_write_AAAAAAAA_fails, stats_total_write_AAAAAAAA_fails, (stats_total_write_AAAAAAAA_fails / stats_runs), ((stats_total_write_AAAAAAAA_fails * 100) / stats_total_fails));
		printf("STATS: Wrote 0x00000000       \t%d\t%d\t%d\t%d\n", stats_run_wrote_00000000_fails, stats_total_wrote_00000000_fails, (stats_total_wrote_00000000_fails / stats_runs), ((stats_total_wrote_00000000_fails * 100) / stats_total_fails));
		printf("STATS: Fail moving  0x00000000\t%d\t%d\t%d\t%d\n", stats_run_move_00000000_fails, stats_total_move_00000000_fails, (stats_total_move_00000000_fails / stats_runs), ((stats_total_move_00000000_fails * 100) / stats_total_fails) );
		printf("STATS: Fail moving  0xFFFFFFFF\t%d\t%d\t%d\t%d\n", stats_run_move_FFFFFFFF_fails, stats_total_move_FFFFFFFF_fails, (stats_total_move_FFFFFFFF_fails / stats_runs), ((stats_total_move_FFFFFFFF_fails * 100) / stats_total_fails) );
		printf("STATS: Adj  moving  0x00000000\t%d\t%d\t%d\t%d\n", stats_adj_move_00000000_fails, stats_total_adj_move_00000000_fails, (stats_total_adj_move_00000000_fails / stats_runs), ((stats_total_adj_move_00000000_fails * 100) / stats_total_fails) );
		printf("STATS: Adj  moving  0xFFFFFFFF\t%d\t%d\t%d\t%d\n", stats_adj_move_FFFFFFFF_fails, stats_total_adj_move_FFFFFFFF_fails, (stats_total_adj_move_FFFFFFFF_fails / stats_runs), ((stats_total_adj_move_FFFFFFFF_fails * 100) / stats_total_fails) );
	}
	printf("============================================================================================\n");
	#endif
	/*}}}	*/

	if (error >= 1)
		return 1;
	else
		return 0;
}

/*}}}	*/
