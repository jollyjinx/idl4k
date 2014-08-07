/*
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


#ifndef __STM_NAND_H_
#define __STM_NAND_H_


struct nand_chip;	/* defined elsewhere */
struct mtd_info;	/* defined elsewhere */


extern void stm_default_board_nand_init(
	struct nand_chip * const nand,
	void (*hwcontrol)(struct mtd_info *mtdinfo, int cmd),
	int (*dev_ready)(struct mtd_info *mtd));

extern void stm_flex_init_nand(
	struct mtd_info * const mtd,
	struct nand_chip * const nand);

extern void stm_nand_chip_init(
	struct mtd_info * const mtd,
	const int nand_maf_id,
	const int nand_dev_id);


#endif /* __STM_NAND_H_ */
