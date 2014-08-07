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


#ifndef _PMB_H_
#define _PMB_H_

	/*
	 * The PMB contains 16 entries, and supports the following
	 * 4 page sizes: 16MB, 64MB, 128MB, and 512MB.
	 *
	 * Although the PMB has a total of 16 entries, but we will
	 * pretend that there are only 14, by grouping #0 and #1
	 * logically together, we will also group #2 and #3 logically
	 * together. Each of these two new groups will be used to map
	 * up to 256MB of main memory (LMI0). One group ([2:3]) will
	 * always map main memory as UN-cached. The other group ([0:1])
	 * will usually be cached, but will sometimes be un-cached for
	 * special needs. Both the new groups will reference the *same*
	 * physical memory, and hence they are virtual aliases for each other.
	 *
	 * Thus, PMB[0:1] is used as main memory (LMI), and will mostly
	 * be used as cached, except on initialization, and when passing
	 * control to the linux kernel, where it will be un-cached.
	 * The cacheability of PMB[0:1] may be toggled by calling
	 * the function sh_toggle_pmb_cacheability().
	 *
	 * PMB[2:3] is an alias for PMB[0:1], except it is guaranteed
	 * to always be UN-cached.
	 *
	 * PMB[0:1]  will map 0x80000000 .. 0x8fffffff.	(256MB)
	 * PMB[2:3]  will map 0x90000000 .. 0x9fffffff.	(256MB)
	 * PMB[4:15] will map 0xa0000000 .. 0xbfffffff.	(512MB)
	 *
	 * If sizeof(LMI) <= 128MB, then PMB[1] and PMB[3] are *unused*,
	 * i.e. PMB[1].V == PMB[3].V == 0.
	 *
	 * Note: PMB[0:3] should all be initialized as UN-cached,
	 * and then sh_toggle_pmb_cacheability() should be called
	 * to enable the caching of PMB[0:1].
	 */

	 /*
	 * If the main LMI memory is 256MB, then we need to have
	 * two PMB entries to represent this amount of memory.
	 * The following predicate will yield TRUE if U-boot
	 * requires 2 PMB entries (#0, and #1) for main memory
	 * (plus two more (#2 and #3) for its un-cached alias),
	 * and FALSE if only a single entry (#0) is required
	 * (plus one more (#2) for its un-cached alias).
	 */
#define CFG_SH_LMI_NEEDS_2_PMB_ENTRIES	\
	( (CFG_SDRAM_SIZE) == (256*1024*1024) )


#endif	/* _PMB_H_ */

