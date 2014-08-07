/*
 * Copyright (C) 2009 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 * Copyright (C) 2010 STMicroelectronics.
 *	Sean McGoogan STMicroelectronics, <Sean.McGoogan@st.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef __ASM_SH_CLK_H__
#define __ASM_SH_CLK_H__

/*
 * Note that there are two distinct clocks we need to deal with:
 * The "comms" clock and the "peripheral" clock.
 *
 *	The "comms" clock is used by: ASC, SSC
 *	The "peripheral" clock is used by: TMU, SCIF, Watchdog
 *
 * Unfortunately, due to various historical circumstances and SoC
 * evolution, the U-boot code is now a bit "confused" about which one
 * to use!  Originally, the peripheral clock was used for both the TMU
 * and the SCIF (UART). However, today (March 2010), the ASC has
 * largely replaced the SCIF, and instead uses the "comms" clock.
 *
 * The ASC driver was based on the SCIF one, and unfortunately the
 * clocking nomenclature was unchanged - hence the U-boot code for
 * the SCIF and the ASC both claim to use the "peripheral" clock,
 * which is correct for the former, and incorrect for the latter.
 *
 * In summary, the clocking infrastructure needs to be overhauled and
 * brought up to date, as the peripheral clock often runs at a
 * different frequency to the comms clock.
 *
 *				Sean McGoogan March 2010.
 */

static inline unsigned long get_peripheral_clk_rate(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	const bd_t * const bd = gd->bd;

	/* Return Peripheral Clock in Hz. */
	return bd->bi_emifrq * 1000000;
}

static inline unsigned long get_tmu0_clk_rate(void)
{
	/* Return the TMU's Clock Frequency (in Hz). */
#if defined(CONFIG_SH_STX5197)
	return 200ul * 1000000ul;	/* BODGE: Peripheral Clock = 200 MHz */
#else
	return get_peripheral_clk_rate();
#endif
}

#if 0	/* QQQ - TO DO */
static inline unsigned long get_comms_clk_rate(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	const bd_t * const bd = gd->bd;

	/* Return Comms Clock Frequency (in Hz). */
	return bd->bi_commsfrq * 1000000ul;
}
#endif	/* QQQ - TO DO */

#endif /* __ASM_SH_CLK_H__ */
