/*
 * (C) Copyright 2006 DENX Software Engineering
 * (C) Copyright 2008-2011 STMicroelectronics, Sean McGoogan <Sean.McGoogan@st.com>
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
#include <nand.h>
#include <asm/io.h>
#include <asm/pio.h>
#include <asm/socregs.h>
#include <asm/stm-nand.h>

/*
 * hardware specific access to control-lines for "bit-banging".
 *	CL -> Emi_Addr(17)
 *	AL -> Emi_Addr(18)
 *	nCE is handled by EMI (not s/w controllable)
 */
#ifndef CFG_NAND_FLEX_MODE	/* for "bit-banging" (c.f. STM "flex-mode")  */
static void fldb_hwcontrol(struct mtd_info *mtdinfo, int cmd)
{
	struct nand_chip* this = (struct nand_chip *)(mtdinfo->priv);

	switch(cmd) {

	case NAND_CTL_SETCLE:
		this->IO_ADDR_W = (void *)((unsigned int)this->IO_ADDR_W | (1u << 17));
		break;

	case NAND_CTL_CLRCLE:
		this->IO_ADDR_W = (void *)((unsigned int)this->IO_ADDR_W & ~(1u << 17));
		break;

	case NAND_CTL_SETALE:
		this->IO_ADDR_W = (void *)((unsigned int)this->IO_ADDR_W | (1u << 18));
		break;

	case NAND_CTL_CLRALE:
		this->IO_ADDR_W = (void *)((unsigned int)this->IO_ADDR_W & ~(1u << 18));
		break;
	}
}
#endif /* CFG_NAND_FLEX_MODE */


/*
 * hardware specific access to the Ready/not_Busy signal.
 * Signal is routed through the EMI NAND Controller block.
 */
#ifndef CFG_NAND_FLEX_MODE	/* for "bit-banging" (c.f. STM "flex-mode")  */
static int fldb_device_ready(struct mtd_info *mtd)
{
	/* extract bit 1: status of RBn pin on boot bank */
	return ((*ST40_EMI_NAND_RBN_STA) & (1ul<<1)) ? 1 : 0;
}
#endif /* CFG_NAND_FLEX_MODE */


/*
 * Board-specific NAND initialization.
 * We use a "generic" STM function stm_default_board_nand_init() to do it.
 * However, we can easily override anything locally, if required.
 */
extern int board_nand_init(struct nand_chip * const nand)
{
#if defined(CFG_NAND_FLEX_MODE)	/* for STM "flex-mode" */
	stm_default_board_nand_init(nand, NULL, NULL);
#else				/* for "bit-banging" */
	stm_default_board_nand_init(nand, fldb_hwcontrol, fldb_device_ready);
#endif

	/*
	 * Only enable the following to use a (volatile) RAM-based
	 * (not NAND-resident) Bad Block Table (BBT).
	 * This is *not* recommended! A NAND-resident BBT is recommended.
	 */
#if 0
	nand->options &= ~NAND_USE_FLASH_BBT;
#endif

	return 0;
}
